#include "ze_backend.h"

// FOR NOW ASSUMES THAT CHUNK IS ON DEVICE MEMORY
//	- the host as own functions for reading/writing

// TODO:
//	- where are contents? (other device? host? remote? server?)
//	- For now assumes contents are on host!
//	- BULK WRITE? Just like read is bulk read?

// CALLED FROM push() in obj.c
int ze_write_contents_to_chunk(uint64_t write_size, void * contents, Phys_Chunk * phys_chunk){

	ze_result_t result;
	const char * err;

	// 1. get handles
	Device * device = (Device *) phys_chunk -> home;
	Ze_Context * ze_context = (Ze_Context *) device -> backend_context;
    ze_driver_handle_t drv_handle = ze_context -> drv_handle;
    ze_device_handle_t dev_handle = ze_context -> dev_handle;
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;
    ze_command_queue_handle_t cmd_queue_handle = ze_context -> cmd_queue_handle;
    uint64_t min_granularity = device -> min_granularity;

    // 2. Reserve temporary va space needed to write
    void * d_ptr;
    const void * start_addr = NULL;
    
    uint64_t page_size;
    result = zeVirtualMemQueryPageSize(ctx_handle, dev_handle, write_size, &page_size);
    if (result != ZE_RESULT_SUCCESS){
		fprintf(stderr, "Error: could not query page size: %d\n", result);
        return -1;
	}

	// from utils.c
	uint64_t aligned_size = get_aligned_size(write_size, page_size);

	result = zeVirtualMemReserve(ctx_handle, start_addr, aligned_size, &d_ptr);
	if (result != ZE_RESULT_SUCCESS){
		fprintf(stderr, "Error: could not reserve va space: %d\n", result);
        return -1;
	}

	// 3. Map the chunk into va space
	ze_physical_mem_handle_t phys_mem_handle = (ze_physical_mem_handle_t) (phys_chunk -> shareable_phys_mem_handle);
	// ENSURE WE CAN WRITE!
	result = zeVirtualMemMap(ctx_handle, d_ptr, aligned_size, phys_mem_handle, 0, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE);
	if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not do memory mapping: %d\n", result);
        return -1;
    }

    // 4. Set Access
    result = zeVirtualMemSetAccessAttribute(ctx_handle, d_ptr, aligned_size, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not set access attribute: %d\n", result);
        return -1;
    }

    // 4. Do memcpy to transfer contents to chunk!

    ze_command_list_desc_t cmd_list_desc = {
    	ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC,
    	NULL,
    	0,
    	0
	};
	
	ze_command_list_handle_t cmd_list_handle;
	
	result = zeCommandListCreate(ctx_handle, dev_handle, &cmd_list_desc, &cmd_list_handle);
	if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not create command list: %d\n", result);
        return -1;
    }

	// copy contents to memory
	//	- Need to ensure that contents is accessible from device...?
	result = zeCommandListAppendMemoryCopy(cmd_list_handle, d_ptr, contents, write_size, NULL, 0, NULL);
	if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not append memory copy to command list: %d\n", result);
        return -1;
    }

    result = zeCommandListClose(cmd_list_handle);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not close command list: %d\n", result);
        return -1;
    }

    result = zeCommandQueueExecuteCommandLists(cmd_queue_handle, 1, &cmd_list_handle, NULL);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not execute command list to transfer from host to device: %d\n", result);
        return -1;
    }

    result = zeCommandQueueSynchronize(cmd_queue_handle, UINT64_MAX);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not synchronize command queue: %d\n", result);
        return -1;
    }

    // 5. Need to free VA range in order for phys mem to stick
    result = zeVirtualMemUnmap(ctx_handle, d_ptr, aligned_size);
    if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: could not unmap virt memory: %d\n", result);
    	return -1;
    }


    return 0;
}