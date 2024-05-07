#include "hsa_backend.h"

// FOR NOW ASSUMES THAT CHUNK_HANDLES ARE ON DEVICE MEMORY
//	- the host as own functions for reading/writing

int hsa_read_contents_from_chunks(Device * device, uint64_t obj_size, uint64_t chunk_cnt, Phys_Chunk ** phys_chunks, void ** ret_obj_contents){
	

	hsa_status_t hsa_status;
	const char * err;

	
	// Need to round obj size up to multiple of chunk size
	uint64_t min_granularity = device -> min_granularity;
	uint64_t aligned_obj_size = get_aligned_size(obj_size, min_granularity);

	// 1. Setting context for chunk's device
   	// Need to set device's context and query driver
    Hip_Context * hip_context = (Hip_Context *) device -> backend_context;

    hsa_agent_t agent = hip_context -> agent;
    hsa_amd_memory_pool_t mem_pool = hip_context -> mem_pool;
    hsa_agent_t cpu_agent = hip_context -> cpu_agent;
    hsa_amd_memory_pool_t cpu_mem_pool = hip_context -> cpu_mem_pool;


	// 2. Reserve virtual address space for object
	void * dptr;
    uint64_t addr_req = 0;
    uint64_t va_req_flags = 0;
    hsa_status = hsa_amd_vmem_address_reserve(&dptr, aligned_obj_size, addr_req, va_req_flags);
	

	int dmabuf_fd;
	hsa_amd_vmem_alloc_handle_t phys_mem_handle;
	Phys_Chunk * phys_chunk;
	uint64_t map_size;
	uint64_t offset = 0;

	// 3. Map phys chunks into the object va space

	// For all but last chunk we know they are full
	for (uint64_t i = 0; i < chunk_cnt; i++){

		phys_chunk = phys_chunks[i];
		map_size = phys_chunk -> size;

		// Determining next chunk to map
		dmabuf_fd = (int) (phys_chunk -> shareable_phys_mem_handle);

		hsa_status = hsa_amd_vmem_import_shareable_handle(dmabuf_fd, &phys_mem_handle);
    	if (hsa_status != HSA_STATUS_SUCCESS){
        	hsa_status_string(hsa_status, &err);
        	fprintf(stderr, "Error importing handle: %s\n", err);
        	// TODO:
        	//	- cleanup
        	return -1;
    	}

    	hsa_status = hsa_amd_vmem_map(dptr + offset, map_size, 0, phys_mem_handle, 0);
    	if (hsa_status != HSA_STATUS_SUCCESS){
        	hsa_status_string(hsa_status, &err);
        	fprintf(stderr, "Error doing memory mapping: %s\n", err);
        	// TODO:
        	//	- cleanup
        	return -1;
    	}

    	offset += map_size;
    }

    // 4. Set access flags for reading
    hsa_amd_memory_access_desc_t accessDescriptorSrc;

    //HSA_ACCESS_PERMISSION_NONE, HSA_ACCESS_PERMISSION_RO, HSA_ACCESS_PERMISSION_WO, HSA_ACCESS_PERMISSION_RW
    accessDescriptorSrc.permissions = HSA_ACCESS_PERMISSION_RW;
    accessDescriptorSrc.agent_handle = cpu_agent;
    hsa_amd_memory_access_desc_t accessDescriptorDest;

    //HSA_ACCESS_PERMISSION_NONE, HSA_ACCESS_PERMISSION_RO, HSA_ACCESS_PERMISSION_WO, HSA_ACCESS_PERMISSION_RW

    accessDescriptorDest.permissions = HSA_ACCESS_PERMISSION_RW;
    accessDescriptorDest.agent_handle = agent;


    hsa_amd_memory_access_desc_t accessDescriptors[2] = {accessDescriptorSrc, accessDescriptorDest};

    uint64_t desc_cnt = 2;

    hsa_status = hsa_amd_vmem_set_access(dptr, aligned_obj_size, accessDescriptors, desc_cnt);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error setting access: %s\n", err);
        // TODO:
        //	- cleanup
        return -1;
    }

    // ACTUALLY TRANSFER TO DESTINATION NOW
    // (if src != dest but on same node)
    //	1. create signal to indicate completation of memcpy
    //	2. allocate memory on dest pool
    //	3. call memcpy
    //	4. wait for signal to change

    // 1. create signal
    hsa_signal_t signal;

    // THIS WILL GET DECREMENTED ON COMPLETETION OF MEMCPY ASYNC
    // OR SET TO NEGATIVE IF ERROR
    hsa_signal_value_t signal_initial_value = 1;
    // means anyone can consume
    uint32_t num_consumers = 0;
    hsa_agent_t * consumers = NULL;

    hsa_status = hsa_signal_create(signal_initial_value, num_consumers, consumers, &signal);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error unmapping: %s\n", err);
        return -1;
    }


    // 2. allocate memory on dest pool

    // TEMPORARY: FOR NOW ASSUMING ALL READS ARE ON CPU MEM
    void * dest_ptr = malloc(obj_size);

    // set agents to NULL to allow everyone access
    void * hsa_dest_ptr;
    hsa_status = hsa_amd_memory_lock(dest_ptr, obj_size, NULL, 0, &hsa_dest_ptr);
    if (hsa_status != HSA_STATUS_SUCCESS){
    	hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error calling memory lock: %s\n", err);
        return -1;
    }

    // Allocating on host and then allowing access returned error when there are more than one chunks mapped to a single va region!
    //	- However no mapping is even being done at this point just that the a region is being allocated? 
    //		- why would this work with a single chunk and not multiple? 
    //		- seems like there is a bug with some intermediate state being carried along incorrectly...
    //	- Issue because allocation was on host specifically? 

    /*
    hsa_amd_memory_pool_t dest_mem_pool = cpu_mem_pool; 
    void * dest_ptr;
    hsa_status = hsa_amd_memory_pool_allocate(dest_mem_pool, aligned_obj_size, HSA_AMD_MEMORY_POOL_STANDARD_FLAG, &dest_ptr);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error allocating on dest: %s\n", err);
        return -1;
    }


    int num_agents = 2;
    hsa_agent_t agents_access[2] = {cpu_agent, agent};
    // Reserved and needs to be none
    uint32_t * access_flags = NULL;
    hsa_status = hsa_amd_agents_allow_access(num_agents, agents_access, access_flags, dest_ptr);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error allowing access to dest pointer: %s\n", err);
        return -1;
    }
    */

    
    // 3. Now perform memcpy
    //		- Now both src and dest buffers are accessible by src and dest agents
    uint64_t num_dep_signals = 0;
    hsa_signal_t * dep_signals = NULL;
    hsa_signal_t completition_signal = signal;

    // TEMPORARY: HARDCODED HERE BUT SHOULDN'T BE
    hsa_agent_t src_agent = agent;
    hsa_agent_t dest_agent = cpu_agent;
    // PERFORM MEMCPY
    //	- need to use the hsa_dest_ptr because gpu needs to have access 
    hsa_status = hsa_amd_memory_async_copy(hsa_dest_ptr, cpu_agent, dptr, agent, aligned_obj_size, num_dep_signals, dep_signals, completition_signal);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error calling memory_async_copy: %s\n", err);
        return -1;
    }

    // // WAIT UNTIL COMPLETETION SIGNAL CHANGES
    hsa_signal_value_t sig_val = signal_initial_value;
    while (sig_val == signal_initial_value){
        sig_val = hsa_signal_load_scacquire(completition_signal);
    }

    // set to negative if error
    if (sig_val < 0){
        fprintf(stderr, "Error: could not do memory copy, negative signal returned.\n");
        return -1;
    }

    *ret_obj_contents = dest_ptr;

    return 0;
}