#include "phys_chunk.h"

// TODO:
//	- Add functionality for access flags

int init_shareable_device_phys_mem_handle(Device * device, Phys_Chunk * phys_chunk){

	DeviceType device_type = device -> device_type;

	int err;

	// sets phys_chunk -> sharable_phys_mem_handle
	switch(device_type){
		case AMD_GPU:
			err = hsa_init_shareable_device_phys_mem_handle(device, phys_chunk);
			break;

		default:
			fprintf(stderr, "Error: device type not support for initializing sharable phys mem handle\n");
			err = -1;
	}

	if (err != 0){
		fprintf(stderr, "Error: could not initialize sharable device phys memory handle\n");
		return err;
	}

	return 0;
}

int init_shareable_host_phys_mem_handle(Host * host, Phys_Chunk * phys_chunk){

	OsType os_type = host -> os_type;

	int err;

	switch(os_type){
		case POSIX:
			err = posix_init_shareable_host_phys_mem_handle(host, phys_chunk);
			break;

		default:
			fprintf(stderr, "Error: device type not support for initializing sharable phys mem handle\n");
			err = -1;
	}

	if (err != 0){
		fprintf(stderr, "Error: could not initialize sharable device phys memory handle\n");
		return err;
	}

	return 0;

}

// ASSUMES EVERY ID WILL BE UNIQUE
//	- this id is the index into chunks -> phys_chunks and chunks -> chunk_sizes
Phys_Chunk * init_phys_chunk(Location location, void * home, uint64_t id, uint64_t size, uint64_t access_flags){

	Phys_Chunk * phys_chunk = (Phys_Chunk *) malloc(sizeof(Phys_Chunk));

	if (phys_chunk == NULL){
		fprintf(stderr, "Error: malloc failed in init phys chunk\n");
		// TODO: cleanup here
		return NULL;
	}

	phys_chunk -> location = location;
	phys_chunk -> home = home;
	phys_chunk -> id = id;
	phys_chunk -> size = size;
	phys_chunk -> ref_cnt = 0;

	// TODO:
	//	- look into mutex attributes (not sure if NULL is what we want)
	pthread_mutex_init(&(phys_chunk -> ref_cnt_lock), NULL);

	uint64_t min_granularity;
	if (location == DEVICE){
		min_granularity = ((Device *) home) -> min_granularity;
	}
	if (location == HOST){
		min_granularity = ((Host *) home) -> min_granularity;
	}

	uint64_t num_min_chunks = ceil((float) size / (float) min_granularity);

	phys_chunk -> num_min_chunks = num_min_chunks;

	// FOR NOW UNUSED
	phys_chunk -> access_flags = access_flags;

	int ret;

	switch (location){
		case DEVICE:
			ret = init_shareable_device_phys_mem_handle((Device *) home, phys_chunk);
			break;
		case HOST:
			ret = init_shareable_host_phys_mem_handle((Host *) home, phys_chunk);
			break;

		default:
			fprintf(stderr, "Error: location not yet supported for allocating physical chunk\n");
			ret = -1;
			break;
	}
	
	if (ret != 0){
		fprintf(stderr, "Error: could not init phys_chunk\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	return phys_chunk;
}

void destroy_phys_chunk(Phys_Chunk * chunk){

	fprintf(stderr, "Destroy Phys_Chunk: Unimplemented Error\n");
}



// occurs during system_init()
// Because this is called at initialization time assume no races (don't need locks)
int insert_free_chunk(Location location, void * home, Phys_Chunk * phys_chunk){

	// upon initiailization for all min granularity chunks set id to be current count
	//	- TODO:
	//		- handling chunk_id's for variable sized chunks?
	//		- set index to be total accumulated minimum sized chunks?

	Chunks * chunks;

	switch (location){
		case DEVICE:
			chunks = ((Device *) home) -> chunks;
			break;
		case HOST:
			chunks = ((Host *) home) -> chunks;
			break;

		default:
			fprintf(stderr, "Error: location not supported for inserting free chunks\n");
			return -1;
	}

	if (chunks == NULL){
		fprintf(stderr, "Error: cannot insert chunk. The chunks container has not been initialized\n");
		return -1;
	}

	Phys_Chunk ** phys_chunks = chunks -> phys_chunks;
	uint64_t * chunk_sizes = chunks -> chunk_sizes;
	uint64_t max_chunks = chunks -> max_chunks;

	// Assume this was set prior
	uint64_t phys_chunk_id = phys_chunk -> id;

	// shoudln't ever happen
	if (phys_chunk_id >= max_chunks){
		fprintf(stderr, "Error: Cannot insert phys_chunk_id = %ld into chunks. Exceeds max_chunks which is set to %ld\n", phys_chunk_id, max_chunks);
		return -1;
	}
	if (phys_chunks[phys_chunk_id] != NULL) {
		fprintf(stderr, "Error: Cannot insert phys_chunk_id = %ld into phys_chunks. Spot taken\n", phys_chunk_id);
		return -1;
	}
	if (chunk_sizes[phys_chunk_id] != 0) {
		fprintf(stderr, "Error: Cannot insert phys_chunk_id = %ld into chunks_sizes. Spot taken\n", phys_chunk_id);
		return -1;
	}
	
	phys_chunks[phys_chunk_id] = phys_chunk;
	// setting size in bytes
	chunk_sizes[phys_chunk_id] = phys_chunk -> size;

	uint64_t num_min_chunks = phys_chunk -> num_min_chunks;
	
	chunks -> total_cnt += 1;
	chunks -> free_min_sized_chunks += num_min_chunks;

	// insert into free list
	Deque * free_chunk_ids = chunks -> free_chunk_ids;
	enqueue(free_chunk_ids, phys_chunk_id);

	return 0;
}