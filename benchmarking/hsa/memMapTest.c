#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>

typedef struct mem_pools {
	int n_mem_pools;
	hsa_amd_memory_pool_t * mem_pools;
} MemPools;

typedef struct agent {
	hsa_agent_t agent_handle;
	MemPools * mem_pools;
} Agent;

typedef struct agents {
	int n_agents;
	Agent ** agents;
} Agents;

MemPools * init_mem_pools(int max_mem_pools){
	MemPools * mem_pools = (MemPools *) malloc(sizeof(MemPools));
	mem_pools -> n_mem_pools = 0;
	mem_pools -> mem_pools = (hsa_amd_memory_pool_t *) malloc(max_mem_pools * sizeof(hsa_amd_memory_pool_t));
	return mem_pools;
}

Agents * init_agents(int max_agents, int max_mem_pools){
    Agents * agents = (Agents *) malloc(sizeof(Agents));
    agents -> n_agents = 0;
    Agent ** agent_arr = (Agent **) malloc(max_agents * sizeof(Agent *));
    for (int i = 0; i < max_agents; i++){
    	agent_arr[i] = malloc(sizeof(Agent));
    	agent_arr[i] -> mem_pools = init_mem_pools(max_mem_pools);
    }
    agents -> agents = agent_arr;
    return agents;
}


hsa_status_t my_mem_pool_callback(hsa_amd_memory_pool_t mem_pool, void * data){
	MemPools * mem_pools = (MemPools *) data;
	(mem_pools -> mem_pools)[mem_pools -> n_mem_pools] = mem_pool;
	mem_pools -> n_mem_pools++;
	return HSA_STATUS_SUCCESS;
}


hsa_status_t my_agent_callback(hsa_agent_t agent, void * data){
    Agents * agents = (Agents *) data;
    (agents -> agents)[agents -> n_agents] -> agent_handle = agent;
    agents -> n_agents++;
    return HSA_STATUS_SUCCESS;
}

hsa_agent_t get_agent_handle(Agents * agents, int agent_id){
	Agent ** agent_handles = agents -> agents;
	Agent * agent_handle = agent_handles[agent_id];
	return agent_handle -> agent_handle;
}

hsa_amd_memory_pool_t get_mem_pool_handle(Agents * agents, int agent_id, int mem_pool_id) {
	Agent ** agent_handles = agents -> agents;
	Agent * agent_handle = agent_handles[agent_id];
	MemPools * mem_pools = agent_handle -> mem_pools;
	hsa_amd_memory_pool_t * mem_pool_handles = mem_pools -> mem_pools;
	return mem_pool_handles[mem_pool_id];
}

Agents * hsa_initialize(int max_agents, int max_mem_pools) {

	hsa_status_t hsa_status;
	const char * err;

	// Init hsa runtime
	hsa_status = hsa_init();
	if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error iterating agents: %s\n", err);
    }


	Agents * agents = init_agents(max_agents, max_mem_pools);

	hsa_status = hsa_iterate_agents(&my_agent_callback, (void *) agents);
	if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error iterating agents: %s\n", err);
    }

    int n_agents = agents -> n_agents;

    hsa_agent_t agent_handle;
    MemPools * agent_mem_pools;

   	for (int i = 0; i < n_agents; i++){
   		agent_handle = (agents -> agents)[i] -> agent_handle;
   		agent_mem_pools = (agents -> agents)[i] -> mem_pools;
   		hsa_status = hsa_amd_agent_iterate_memory_pools(agent_handle, &my_mem_pool_callback, (void *) agent_mem_pools);
    	if (hsa_status != HSA_STATUS_SUCCESS){
        	hsa_status_string(hsa_status, &err);
        	fprintf(stderr, "Error iterating memory pools: %s\n", err);
    	}
   	}

   	return agents;
}



uint64_t get_aligned_size(uint64_t obj_size, uint64_t page_size){
	int remainder = obj_size % page_size;
    if (remainder == 0)
        return obj_size;

    return obj_size + page_size - remainder;
}

uint64_t get_mem_pool_alloc_granularity(hsa_amd_memory_pool_t dest_pool) {
	return 1 << 21;
}




// takes in size and address of allocHandle to set
// will be default pinned settings on device
int hsa_allocate_phys(Agents * agents, int agent_id, int pool_id, uint64_t alloc_size, hsa_amd_vmem_alloc_handle_t * ret_phys_mem_handle){

	hsa_status_t hsa_status;
	const char * err;

	Agent ** agent_arr = agents -> agents;
	Agent * agent = agent_arr[agent_id];
	MemPools * agent_mem_pools = agent -> mem_pools;
	hsa_amd_memory_pool_t mem_pool_handle = agent_mem_pools -> mem_pools[pool_id];

    hsa_amd_memory_type_t mem_type = MEMORY_TYPE_PINNED;
    // CURRENTLY UNSUPPORTED
    uint64_t create_flags = 0;
    hsa_amd_vmem_alloc_handle_t phys_mem_handle;
    hsa_status = hsa_amd_vmem_handle_create(mem_pool_handle, alloc_size, mem_type, create_flags, &phys_mem_handle);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error iterating agents: %s\n", err);
        return -1;
    }

    *ret_phys_mem_handle = phys_mem_handle;

    return 0;
}


int hsa_export_to_dmabuf(hsa_amd_vmem_alloc_handle_t phys_mem_handle, int * ret_dmabuf_fd){
    
    hsa_status_t hsa_status;
    const char * err;

    int dmabuf_fd;
    // CURRENTLY UNSUPPORTED
    uint64_t export_flags = 0;
    hsa_status = hsa_amd_vmem_export_shareable_handle(&dmabuf_fd, phys_mem_handle, export_flags);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error exporting shareable handle: %s\n", err);
        return -1;
    }

    *ret_dmabuf_fd = dmabuf_fd;

    return 0;
}


int import_from_dmabuf(int dmabuf_fd, hsa_amd_vmem_alloc_handle_t * handle) {

	hsa_status_t hsa_status;
    const char * err;

	hsa_status = hsa_amd_vmem_import_shareable_handle(dmabuf_fd, handle);
	if (hsa_status != HSA_STATUS_SUCCESS){
		hsa_status_string(hsa_status, &err);
    	fprintf(stderr, "Error: Could not import from sharable handle: %s\n", err);
    	return -1;
	}
	return 0;
}


int reserve_va_space(uint64_t va_range_size, void ** ret_va_ptr){
	hsa_status_t hsa_status;
	const char * err;
	uint64_t addr_req = 0;
    uint64_t va_req_flags = 0;

    hsa_status = hsa_amd_vmem_address_reserve(ret_va_ptr, va_range_size, addr_req, va_req_flags);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error reserving VA: %s\n", err);
        return -1;
    }
    return 0;
}


int mem_map(uint64_t map_size, void * va_ptr, hsa_amd_vmem_alloc_handle_t phys_mem_handle){
	hsa_status_t hsa_status;
	const char * err;
	hsa_status = hsa_amd_vmem_map(va_ptr, map_size, 0, phys_mem_handle, 0);
    if (hsa_status != HSA_STATUS_SUCCESS){
    	hsa_status_string(hsa_status, &err);
    	fprintf(stderr, "Error: Could not do memory mapping: %s\n", err);
    	return -1;
    }
    return 0;

}


int set_access(uint64_t va_range_size, void * va_ptr, uint64_t desc_cnt, hsa_amd_memory_access_desc_t * accessDescriptors){
	
	hsa_status_t hsa_status;
	const char * err;

    hsa_status = hsa_amd_vmem_set_access(va_ptr, va_range_size, accessDescriptors, desc_cnt);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error setting access: %s\n", err);
        return -1;
    }
    return 0;
}

int mem_unmap(void * va_ptr, uint64_t va_range_size){
	hsa_status_t hsa_status;
	const char * err;

    hsa_status = hsa_amd_vmem_unmap(va_ptr, va_range_size);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error unmapping: %s\n", err);
        return -1;
    }
    return 0;
}


int free_va_space(void * va_ptr, uint64_t va_range_size) {
	hsa_status_t hsa_status;
	const char * err;

    hsa_status = hsa_amd_vmem_address_free(va_ptr, va_range_size);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error freeing va space: %s\n", err);
        return -1;
    }
    return 0;
}

int release_phys_mem(hsa_amd_vmem_alloc_handle_t phys_mem_handle){
	hsa_status_t hsa_status;
	const char * err;

    hsa_status = hsa_amd_vmem_handle_release(phys_mem_handle);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error releasing phys mem handle: %s\n", err);
        return -1;
    }
    return 0;
}


int close_shareable_handle(int dmabuf_fd) {
	hsa_status_t hsa_status;
	const char * err;
    hsa_status = hsa_amd_portable_close_dmabuf(dmabuf_fd);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error closing shareable handle: %s\n", err);
        return -1;
    }
    return 0;
}





int main(int argc, char *argv[]) {


	int ret;

	// Initialize HSA driver + agents + mem pools
	int max_agents = 2;
	int max_mem_pools = 3;
	Agents * agents = hsa_initialize(max_agents, max_mem_pools);

	// 
	int cpu_agent_id = 0;
	int gpu_agent_id = 1;
	int main_pool_id = 0;

	hsa_amd_memory_pool_t dev_mempool = get_mem_pool_handle(agents, gpu_agent_id, 0);


    // Get min chunk granularity   
	uint64_t granularity = get_mem_pool_alloc_granularity(dev_mempool);
	

	// Perform phys mem allocation

	// Setting alloc size to input argument
	uint64_t raw_alloc_size = (uint64_t) atol(argv[1]);
	


	// assumes granularity is power of 2...
	uint64_t alloc_size = get_aligned_size(raw_alloc_size, granularity);

	// Print alloc size
	printf("%ld,", alloc_size);
	

	struct timespec start, stop;
	uint64_t timestamp_start, timestamp_stop, elapsed;


	// 1. Allocating Physical Memory
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	hsa_amd_vmem_alloc_handle_t phys_mem_handle;
	ret = hsa_allocate_phys(agents, gpu_agent_id, main_pool_id, alloc_size, &phys_mem_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 2. Exporting to Shareable Handle
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	int dmabuf_fd;
	ret = hsa_export_to_dmabuf(phys_mem_handle, &dmabuf_fd);
	if (ret != 0){
		exit(1);
	}

	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 3. Importing from Shareable Handle
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	hsa_amd_vmem_alloc_handle_t imported_handle;
	ret = import_from_dmabuf(dmabuf_fd, &imported_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 4. Reserving VA Space
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	void * va_ptr;
	uint64_t va_range_size = alloc_size;
	ret = reserve_va_space(va_range_size, &va_ptr);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 5. Doing memory mapping
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	uint64_t map_size = alloc_size;
	ret = mem_map(map_size, va_ptr, phys_mem_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 6. Setting Access
	
	int desc_cnt = 2;
	hsa_amd_memory_access_desc_t * accessDescriptors = malloc(desc_cnt * sizeof(hsa_amd_memory_access_desc_t));
    
    for (int i = 0; i < desc_cnt; i++){
    	accessDescriptors[i].permissions = HSA_ACCESS_PERMISSION_RW;
    	accessDescriptors[i].agent_handle = get_agent_handle(agents, i);
    }


	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	ret = set_access(va_range_size, va_ptr, desc_cnt, accessDescriptors);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 7. Unmapping
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	ret = mem_unmap(va_ptr, va_range_size);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 8. Freeing VA space
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	ret = free_va_space(va_ptr, va_range_size);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);

	// NOTE
	//	- physical memory is only freed after all dmabuf_fd's are closed, all imported handles are released, and the original handle is released

	// 9. Close the imported dmabuf
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	ret = close_shareable_handle(dmabuf_fd);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);

	// 10. Release imported physical memory
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	ret = release_phys_mem(imported_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);

	
	// 11. Finally release the physical allocation which creates free page frames
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	ret = release_phys_mem(phys_mem_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld\n,", elapsed);

	printf("Sleeping for a while to sit and analyze if memory is truly released...");
	sleep(5 * 60);

	return 0;

}