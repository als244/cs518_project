#include "cuda_backend.h"


int cuda_set_min_chunk_granularity(Device * device){

    CUresult result;
    const char * err;

    // Need to set device's context and query driver

    Cuda_Context * cuda_context = (Cuda_Context *) device -> backend_context;
    CUcontext ctx = cuda_context -> context;

    result = cuCtxPushCurrent(ctx);
    if (result != CUDA_SUCCESS){
        cuGetErrorString(result, &err);
        fprintf(stderr, "Error: Could not push device context: %s\n", err);
        return -1;
    }

    uint64_t min_granularity = 0;
    CUmemGenericAllocationHandle allocHandle;
    CUmemAllocationProp prop = {};
    prop.type = CU_MEM_ALLOCATION_TYPE_PINNED;
    prop.location.type = CU_MEM_LOCATION_TYPE_DEVICE;
    prop.location.id = device -> local_device_id;

    CUmemAllocationGranularity_flags min = CU_MEM_ALLOC_GRANULARITY_MINIMUM;

    result = cuMemGetAllocationGranularity(&min_granularity, &prop, min);

    if (result != CUDA_SUCCESS){
        cuGetErrorString(result, &err);
        fprintf(stderr, "Could not get granularity: %s\n", err);
        return -1;
    }

    result = cuCtxPopCurrent(&ctx);
    if (result != CUDA_SUCCESS){
        cuGetErrorString(result, &err);
        fprintf(stderr, "Error: Could not pop context: %s\n", err);
        return -1;
    }

    device -> min_granularity = min_granularity;

    return 0;

}

int cuda_get_mem_info(Device * device, uint64_t * ret_free, uint64_t * ret_total){

    CUresult result;
    const char * err;
    int ret;

    // 1. set context for device
    uint64_t device_id = device -> local_device_id;
    Cuda_Context * backend_context = (Cuda_Context *) (device -> backend_context);
    CUcontext ctx = backend_context -> context;
    result = cuCtxPushCurrent(ctx);
    if (result != CUDA_SUCCESS){
        cuGetErrorString(result, &err);
        fprintf(stderr, "Error: Could not push device context in mem info: %s\n", err);
        // TODO:
        //  - cleanup
        return -1;
    }

    // 2. Retrieve available memory
    //      - Could also query for alloc granularity here for future portability
    //          - For now assuming 2^21 chunk size
    uint64_t free, total;
    result = cuMemGetInfo(&free, &total);
    if (result != CUDA_SUCCESS){
        cuGetErrorString(result, &err);
        fprintf(stderr, "Error: Could not get mem info: %s\n", err);
        // TODO:
        //  - cleanup
        return -1;
    }

    // 4. Pop context
    result = cuCtxPopCurrent(&ctx);
    if (result != CUDA_SUCCESS){
        cuGetErrorString(result, &err);
        fprintf(stderr, "Error: Could not pop context in mem info: %s\n", err);
        // TODO:
        //  - more cleanup needed here...
        return -1;
    }

    // set values
    *ret_free = free;
    *ret_total = total;

    return 0;
}

// TODO:
//	- modularize better!
//	- this function is too big and not just doing cuda specific things!


// ASSUMES phys_chunk intialized with correct ids, sizes, except for sharable_phys_mem_handle
// Responsibilities:
//  1. Set device context
//  2. Create chunk
//  3. Export to sharable handle and set phys_chunk -> sharable_phys_mem_handle
//  4. Pop device context
int cuda_init_shareable_device_phys_mem_handle(Device * device, Phys_Chunk * phys_chunk){

	CUresult result;
	const char * err;
	int ret;

	// 1. set context for device
	uint64_t device_id = device -> local_device_id;
    Cuda_Context * backend_context = (Cuda_Context *) (device -> backend_context);
    CUcontext ctx = backend_context -> context;
    result = cuCtxPushCurrent(ctx);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not push device context in init_phys_chunk: %s\n", err);
        // TODO:
        //  - cleanup
    	return -1;
    }


    // 2. Create phys mem allocation (generic handle)k
    CUmemGenericAllocationHandle generic_handle;
    CUmemAllocationProp prop = {};
    prop.type = CU_MEM_ALLOCATION_TYPE_PINNED;
    prop.location.type = CU_MEM_LOCATION_TYPE_DEVICE;
    prop.location.id = device_id;
    prop.requestedHandleTypes = CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR;
    CUmemAllocationHandleType handle_type = CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR;
    
    uint64_t min_granularity = device -> min_granularity;
    uint64_t num_min_chunks = phys_chunk -> num_min_chunks;

    uint64_t alloc_size = min_granularity * num_min_chunks;

    result = cuMemCreate(&generic_handle, alloc_size, &prop, 0);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not get alloc handle: %s\n", err);
    	// TODO:
    	//	- more cleanup needed here...
    	return -1;
    }

    // 3. Export shared handle
    //		- Passing in address of sharable handle that's within chunk we just initialized
    //		- Need to be careful about open file limits!
    result = cuMemExportToShareableHandle((void *)&(phys_chunk -> shareable_phys_mem_handle), generic_handle, handle_type, 0);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not export to sharable handle: %s\n", err);
    	// TODO:
    	//	- more cleanup needed here...
    	return -1;
    }

    // 4. Pop context
    result = cuCtxPopCurrent(&ctx);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not pop context: %s\n", err);
    	// TODO:
    	//	- more cleanup needed here...
    	return -1;
    }

    return 0;
}
