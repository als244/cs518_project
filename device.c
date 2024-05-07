#include "device.h"

#define TO_PRINT 1

// wrapper around backend for retrieving minimum phys chunk granularity
// ASSUMES CONTEXT HAS BEEN SET ALEADY!
int set_min_chunk_granularity(Device * device){

	DeviceType device_type = device -> device_type;

	int err;
	switch(device_type){
		case AMD_GPU:
			err = hip_set_min_chunk_granularity(device);
			break;

		default:
			fprintf(stderr, "Error: setting chunk size for device type not supported\n");
			err = -1;
			break;
	}

	if (err != 0){
		fprintf(stderr, "Error: could not set min chunk size\n");
		return err;
	}

	return 0;
}



// WRAPPER around backend-specfic contexts
void * init_context(Device * device){

	DeviceType device_type = device -> device_type;

	void * backend_context;
	switch(device_type){
		case AMD_GPU:
			backend_context = (void *) hsa_init_context(device -> local_device_id);
			break;

		default:
			fprintf(stderr, "Error backend context not support for device type\n");
			// TODO:
			//	- cleanup
			return NULL;
	}

	if (backend_context == NULL){
		fprintf(stderr, "Error: failed to get backend_context\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	return backend_context;
}


// Responsible for:
//	1. Creating device container
//	2. Create context
//	3. Retrieve / set device minimum chunk size
//	4. Create chunks container
//	5. Iterate through initial chunk sizes list, for item in list:
//		- initialize phys chunk
//		- insert free chunk
Device * init_device(Node * host_node, Device_Init_Config * device_init_config) {

	int ret;

	if (TO_PRINT){
		printf("Initializing device with global id: %ld\n", device_init_config -> global_device_id);
	}

	// 1. Create device container
	Device * device = (Device *) malloc(sizeof(Device));
	if (device == NULL){
		fprintf(stderr, "Error: malloc failed in init_device\n");
		// TODO:
		//	- cleanup
		return NULL;
	}
	device -> host_node = host_node;
	device -> global_device_id = device_init_config -> global_device_id;
	device -> local_device_id = device_init_config -> local_device_id;
	DeviceType device_type = device_init_config -> device_type;
	device -> device_type = device_type;

	// 2. Create context
	void * backend_context = init_context(device);
	if (backend_context == NULL){
		fprintf(stderr, "Error: could not initialize device context\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	device -> backend_context = backend_context;

	// 3. Set min granularity
	ret = set_min_chunk_granularity(device);
	if (ret != 0){
		fprintf(stderr, "Error: could not retrieve minimum chunk granularity\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	Location location = DEVICE;
	void * home = (void *) device;

	// 4. Create chunks container
	uint64_t max_chunks = device_init_config -> max_chunks;
	ret = init_chunks_container(location, home, max_chunks);
	if (ret != 0){
		fprintf(stderr, "Error: could not initialize device chunks container\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	// 5. Iterate over chunks and insert
	uint64_t * init_chunk_sizes_list = device_init_config -> init_chunk_sizes_list;
	// for each
			// 1. init
			// 2. insert

	uint64_t phys_chunk_cnt = 0;
	// TODO:
	//	- think about how to deal with access flags
	uint64_t access_flags = 0;
	uint64_t phys_chunk_size;
	Phys_Chunk * phys_chunk;
	for (uint64_t phys_chunk_id = 0; phys_chunk_id < max_chunks; phys_chunk_id++){
		// Get chunk size!
		phys_chunk_size = init_chunk_sizes_list[phys_chunk_id];

		// Assume the chunks are packed tightly at beginning of list 
		// if some have larger than min granularity size
		//	- this means that we will be break upon first zero
		if (phys_chunk_size == 0){
			break;
		}

		// Will be responsible for:
		//	- computing & setting num_min_chunks
		//	- allocating phys mem on device & setting sharable_phys_mem_handle
		phys_chunk = init_phys_chunk(location, home, phys_chunk_id, phys_chunk_size, access_flags);
		// Could be error if device OOM
		if (phys_chunk == NULL){
			fprintf(stderr, "Error: could not initialize chunk with id: %ld (in device memory). Will not initialize/insert any more chunks\n", phys_chunk_id);
			break;
		}

		ret = insert_free_chunk(location, home, phys_chunk);
		// shoudn't ever happen here
		if (ret != 0){
			fprintf(stderr, "Error: couldn not insert free chunk with id: %ld (in device memory). Will not initialize/insert any more chunks\n", phys_chunk_id);
			break;
		}

	}

	return device;
}

void destroy_device(Device * device) {

	fprintf(stderr, "Destroy Device: Unimplemented Error\n");

}