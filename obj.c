#include "obj.h"

// IMPORTANT TODO:

//	 - convert push/pull to be location agnostic
//		- change Device * argument to location + void * home


// RESERVE OBJ ID = 0 for NULL OBJECT!
uint64_t obj_counter = 1;

uint64_t dedup_hash(void * obj_contents){
	// TODO
	// for now...
	uint64_t obj_id = obj_counter;
	obj_counter += 1;
	return obj_id;
}



// TODO:
//	- generalize!

// FOR NOW ASSUME ONLY DEVICES
//	- BUT NOT TRUE!
int write_contents_to_chunk(uint64_t size, void * contents, Phys_Chunk * chunk){

	Device * device = ((Device *) chunk -> home);
	DeviceType dev_type = device -> device_type;

	int err;

	switch (dev_type){
		case AMD_GPU:
			err = hsa_write_contents_to_chunk(size, contents, chunk);
			break;

		default:
			fprintf(stderr, "Error: Writing to specified device type not supported\n");
			err = -1;
	}

	if (err != 0){
		fprintf(stderr, "Error: could not write contents to chunk\n");
		return -1;
	}

	return 0;

}


// IMPORTANT TODO:
//	- only pushing the minimum granularity!
//	- need to make call to some other smart function to decide what sizes to grab!

// ASSUMES CALLER IS SMART WHEN IT COMES TO LOCAL/REMOTE
// Responsibilities:
//	- Get obj id
//		- TODO:
//			- DEDUP SCHEME
//	- Looks up object in dest dev obj_table
//		- if exists on destination device already then no need
//	- Determine if local or remote
//	- TODO:
//		- If remote deal with remote push (RDMA)
//	- If local then:
//		- TODO:
//			- decide if we want to modularize better with contacting backend (in bulk instead of per chunk?)
//		- Reserve phys chunks of dest device
//		- Make writes
//			- Calls backend write
//	- Populate returned object id
//	- If successful return 0, otherwise error
int push (uint64_t obj_size, void * obj_contents, Device * dest_device, Node * src_node, uint64_t * ret_obj_id) {

	// IMPORTANT TODO:
	//	- deal with network transfers
	//	- assumes dest device is on same node calling this program


	int err;

	// Retrieve core data structures
	Obj_Table * obj_table = src_node -> obj_table;
	Chunks * chunks = dest_device -> chunks;


	// first check if we already have it
	uint64_t obj_id = dedup_hash(obj_contents);
	Obj * obj = find_obj(obj_table, obj_id);
	
	// fast-path: object already exists
	if (obj != NULL){
		*ret_obj_id = obj_id;
		return 0;
	}

	// we didn't have it so lets see if we can allocate memory for it
	uint64_t min_granularity = dest_device -> min_granularity;
	
	// IMPORTANT TODO:
	//	- have a smart algorithm to decide chunk and chunk sizes
	// make sure we have enough chunks, this is where internal fragmentation occurs
	//	- only for last chunk of object
	//	- problem for very small objects because min granularity is 2^21 bytes = 2MB
	uint64_t obj_chunk_cnt = ceil((double) obj_size / (double) min_granularity);

	// need to acquire free list lock because might be concurrent writers
	pthread_mutex_lock(&(chunks -> cnt_lock));
	uint64_t free_cnt = chunks -> free_min_sized_chunks;
	// check if enough memory!
	if (obj_chunk_cnt > free_cnt){
		pthread_mutex_unlock(&(chunks -> cnt_lock));
		fprintf(stderr, "Not enough memory. Needed %lu chunks, but only have %lu available\n", obj_chunk_cnt, free_cnt);
		return 2;
	} 
	chunks -> free_min_sized_chunks -= free_cnt;
	pthread_mutex_unlock(&(chunks -> cnt_lock));

	// we have enough memory so we can init object
	obj = (Obj *) malloc(sizeof(Obj));
	obj -> obj_id = obj_id;
	obj -> obj_size = obj_size;
	obj -> obj_chunk_cnt = obj_chunk_cnt;
	obj -> access_flags = 0;
	uint64_t * obj_chunks = (uint64_t *) malloc(obj_chunk_cnt * sizeof(uint64_t));


	// so we need to grab free chunks and add to obj chunk list
	Deque * free_chunk_ids = chunks -> free_chunk_ids;
	// the physical memory handles
	Phys_Chunk ** phys_chunks = chunks -> phys_chunks;

	uint64_t chunk_id;
	Phys_Chunk * chunk;
	uint64_t write_size = min_granularity;
	for (uint64_t c = 0; c < obj_chunk_cnt; c++){
		// pop from head of free chunk list
		pthread_mutex_lock(&(chunks -> free_list_lock));
		err = dequeue(free_chunk_ids, &chunk_id);
		pthread_mutex_unlock(&(chunks -> free_list_lock));
		
		if (err != 0){
			fprintf(stderr, "Error: could not dequeue from free chunk ids\n");
			// TODO:
			//	- cleanup
			return err;
		}	

		// add to tail of obj chunk list
		obj_chunks[c] = chunk_id;

		// get phys chunk
		chunk = phys_chunks[chunk_id];

		// write contents of object to the allocated chunk!
		// if we are the last chunk we need to ensure we pass in the correct size of remaining object and pad
		if (c == obj_chunk_cnt - 1){
			write_size = obj_size - c * min_granularity;
		}

		err = write_contents_to_chunk(write_size, obj_contents + c * min_granularity, chunk);
		
		// could not write object so need to cleanup
		if (err != 0){
			fprintf(stderr, "Error writing content to chunks\n");
			// TODO:
			//	- cleanup
			return err;
		}
	}

	obj -> obj_chunks = obj_chunks;
	obj -> home = (void *) dest_device;
	obj -> location = DEVICE;

	pthread_mutex_lock(&(obj_table -> table_lock));
	err = insert_obj(obj_table, obj);
	pthread_mutex_unlock(&(obj_table -> table_lock));
	
	if (err != 0){
		fprintf(stderr, "Error inserting object to table\n");
		// TODO:
		//	- cleanup
		return err;
	}

	// set the returned object id
	*ret_obj_id = obj_id;

	return 0;
}


// ASSUMES CALLER IS SMART WHEN IT COMES TO LOCAL/REMOTE
// Responsibilities:
//	- Looks up object in dest dev obj_table
//		- if exists on destination device already then no need
//	- Determine if local or remote
//	- TODO:
//		- If remote deal with remote pull (RDMA)
//	- If local then:
//		- Call backend read
//	- Populates return object size and return objec contents on the dest device
//	- If successful return 0, otherwise error
int pull(uint64_t obj_id, Device * dest_device, Node * src_node, uint64_t * ret_obj_size, void ** ret_obj_contents){

	// IMPORTANT TODO:
	//	- deal with network transfers
	//	- FOR NOW, assumes dest device is on same node calling this program

	Obj_Table * obj_table = src_node -> obj_table;

	// Null if not found
	Obj * obj = find_obj(obj_table, obj_id);

	// handle if object did not exsit
	if (obj == NULL){
		fprintf(stderr, "Error: could not find object in pull\n");
		return -1;
	}

	// pull size and chunk count
	uint64_t obj_size = obj -> obj_size;
	uint64_t chunk_cnt = obj -> obj_chunk_cnt;
	uint64_t * obj_chunks = obj -> obj_chunks;

	void ** chunk_handles = (void **) malloc(chunk_cnt * sizeof(void *));

	if (chunk_handles == NULL){
		fprintf(stderr, "Error: malloc failed in pull\n");
		// TODO:
		//	- cleanup
		return -1;
	}

	// retrieve shared chunk handles
	Phys_Chunk ** phys_chunks = dest_device -> chunks -> phys_chunks;

	// CALL BACKEND READ
	DeviceType device_type = dest_device -> device_type;

	int err;

	// WILL POPULATE ret_obj_contents
	switch(device_type){
		case AMD_GPU:
			err = hsa_read_contents_from_chunks(dest_device, obj_size, chunk_cnt, phys_chunks, ret_obj_contents);
			break;

		default:
			fprintf(stderr, "Error: Dest device type not supported for pulling to\n");
			err = -1;
	}

	if (err != 0){
		fprintf(stderr, "Error. Could not pull object to dest device\n");
		// TODO:
		//	- cleanup
		return -1;
	}

	// set the returned object size (ret_obj_contents already set)
	*ret_obj_size = obj_size;

	return 0;

}