#include "chunks.h"


// Initializing an destroying collection of chunks

// Called from init_device() in node.c (will be moving to device.c)
// Responsibilities:
//	1. initialized empty container of the maximum possible chunks
int init_chunks_container(Location location, void * home, uint64_t max_chunks){

	if (max_chunks == 0){
		fprintf(stderr, "Heads up: Max chunks successfully set to 0. Setting home -> chunks = Null\n");
		if (location == DEVICE){
			((Device *) home) -> chunks = NULL;
		}
		if (location == HOST){
			((Host *) home) -> chunks = NULL;
		}
		return 0;
	}

	Chunks * chunks = (Chunks *) malloc(sizeof(Chunks));

	if (chunks == NULL){
		fprintf(stderr, "Error: malloc failed in init chunks\n");
		// TODO: cleanup here
		return -1;
	}
	chunks -> max_chunks = max_chunks;
	chunks -> total_cnt = 0;
	chunks -> free_min_sized_chunks = 0;

	chunks -> location = location;
	chunks -> home = home;
	uint64_t min_granularity;
	if (location == DEVICE){
		min_granularity = ((Device *) home) -> min_granularity;
	}
	if (location == HOST){
		min_granularity = ((Host *) home) -> min_granularity;
	}
	chunks -> min_granularity = min_granularity;
	

	// TODO:
	//	- look into mutex attributes (not sure if NULL is what we want)
	pthread_mutex_init(&(chunks -> cnt_lock), NULL);


	Deque * free_chunk_ids = init_deque();
	if (free_chunk_ids == NULL){
		fprintf(stderr, "Error: init_deque failed in init chunks\n");
		// TODO: cleanup here
		return -1;
	}

	chunks -> free_chunk_ids = free_chunk_ids;
	pthread_mutex_init(&(chunks -> free_list_lock), NULL);
	
	Phys_Chunk ** phys_chunks = (Phys_Chunk **) malloc(max_chunks * sizeof(Phys_Chunk *));
	if (phys_chunks == NULL){
		fprintf(stderr, "Error: malloc failed in init chunks\n");
		// TODO: cleanup here
		return -1;
	}

	uint64_t * chunk_sizes = (uint64_t *) malloc(max_chunks * sizeof(uint64_t));

	if (chunk_sizes == NULL){
		fprintf(stderr, "Error: malloc failed in init chunks\n");
		// TODO: cleanup here
		return -1;
	}

	for (uint64_t i = 0; i < max_chunks; i++){
		phys_chunks[i] = NULL;
		chunk_sizes[i] = 0;
	}


	chunks -> phys_chunks = phys_chunks;
	chunks -> chunk_sizes = chunk_sizes;


	// setting chunks

	if (location == DEVICE){
		((Device *) home) -> chunks = chunks;
	}
	if (location == HOST){
		((Host *) home) -> chunks = chunks;
	}

	return 0;
}

void destroy_chunks(Chunks * chunks){

	fprintf(stderr, "Destroy Chunks: Unimplemented Error\n");
}