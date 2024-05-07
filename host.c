#include "host.h"

#define TO_PRINT 1

Host * init_host(Node * node, Host_Init_Config * host_init_config) {

	if (TO_PRINT){
		printf("Initializing host with global id: %ld\n", host_init_config -> global_host_id);
	}

	Host * host = (Host *) malloc(sizeof(Host));

	if (host == NULL){
		fprintf(stderr, "Error: malloc failed in init host\n");
		return NULL;
	}

	host -> node = node;
	host -> global_host_id = host_init_config -> global_host_id;
	host -> ipaddr = host_init_config -> ipaddr;
	host -> hostname = host_init_config -> hostname;
	host -> os_type = host_init_config -> os_type;
	uint64_t max_chunks = host_init_config -> max_chunks;
	host -> max_chunks = max_chunks;
	host -> min_granularity = host_init_config -> min_granularity;

	Location location = HOST;
	void * home = (void *) host;

	int ret = init_chunks_container(location, home, max_chunks);
	if (ret != 0){
		fprintf(stderr, "Error: could not initialize host chunks\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	uint64_t * init_chunk_sizes_list = host_init_config -> init_chunk_sizes_list;
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
		// Could be error if host OOM
		if (phys_chunk == NULL){
			fprintf(stderr, "Error: could not initialize chunk with id: %ld (in host memory). Will not initialize/insert any more chunks\n", phys_chunk_id);
			break;
		}

		ret = insert_free_chunk(location, home, phys_chunk);
		// shoudn't ever happen here
		if (ret != 0){
			fprintf(stderr, "Error: couldn not insert free chunk with id: %ld (in host memory). Will not initialize/insert any more chunks\n", phys_chunk_id);
			break;
		}

	}

	return host;
}

void destroy_host(Host * host){
	fprintf(stderr, "Destroy Host: Unimplemented Error\n");
}


int posix_init_shareable_host_phys_mem_handle(Host * host, Phys_Chunk * phys_chunk) {
	fprintf(stderr, "Posix init sharable host phys mem handle: Unimplemented Error\n");
	return -1;
}