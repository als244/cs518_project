#include "ze_backend.h"

// FOR NOW ASSUMES THAT CHUNK_HANDLES ARE ON DEVICE MEMORY
//	- the host as own functions for reading/writing


// CALLED FROM PULL() in obj.c
int ze_read_contents_from_chunks(Device * device, uint64_t obj_size, uint64_t chunk_cnt, Phys_Chunk ** phys_chunks, void ** ret_obj_contents){

	ze_result_t result;
	const char * err;

	// 1. get handles
	Ze_Context * ze_context = (Ze_Context *) device -> backend_context;
    ze_driver_handle_t drv_handle = ze_context -> drv_handle;
    ze_device_handle_t dev_handle = ze_context -> dev_handle;
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;
    ze_command_queue_handle_t cmd_queue_handle = ze_context -> cmd_queue_handle;
    uint64_t min_granularity = device -> min_granularity;

	// 2. Reserve VA Space

    void * d_ptr;
    const void * start_addr = NULL;
    uint64_t page_size;
    result = zeVirtualMemQueryPageSize(ctx_handle, dev_handle, obj_size, &page_size);
    if (result != ZE_RESULT_SUCCESS){
		fprintf(stderr, "Error: could not query page size: %d\n", result);
        return -1;
	}

	// from utils.c
	uint64_t aligned_size = get_aligned_size(obj_size, page_size);

	result = zeVirtualMemReserve(ctx_handle, start_addr, aligned_size, &d_ptr);
	if (result != ZE_RESULT_SUCCESS){
		fprintf(stderr, "Error: could not reserve va space: %d\n", result);
        return -1;
	}

	// 3. Set Access
    result = zeVirtualMemSetAccessAttribute(ctx_handle, d_ptr, aligned_size, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not set access attribute: %d\n", result);
        return -1;
    }

	// 4. Do Memory Mappings from physical memory handles
	
	uint64_t num_min_chunks;
	uint64_t alloc_size;
	ze_physical_mem_handle_t phys_mem_handle;
	// Link the request into the allocation descriptor and allocate
	uint64_t offset = 0;
	for (uint64_t i = 0; i < chunk_cnt; i++){
     	num_min_chunks = phys_chunks[i] -> num_min_chunks;
    	alloc_size = min_granularity * num_min_chunks;
		phys_mem_handle = (ze_physical_mem_handle_t) (phys_chunks[i] -> shareable_phys_mem_handle);

		result = zeVirtualMemQueryPageSize(ctx_handle, dev_handle, alloc_size, &page_size);
		if (result != ZE_RESULT_SUCCESS){
			fprintf(stderr, "Error: could not query page size: %d\n", result);
			return -1;
		}
		aligned_size = get_aligned_size(alloc_size, page_size);
		result = zeVirtualMemMap(ctx_handle, d_ptr + offset, aligned_size, phys_mem_handle, 0, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE);
		if (result != ZE_RESULT_SUCCESS){
        	fprintf(stderr, "Error: could not do memory mapping: %d\n", result);
        	return -1;
    	}
    	offset += aligned_size;
	}

	*ret_obj_contents = d_ptr;

	return 0;
}