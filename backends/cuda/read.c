#include "cuda_backend.h"

// FOR NOW ASSUMES THAT CHUNK_HANDLES ARE ON DEVICE MEMORY
//	- the host as own functions for reading/writing

// CALLED FROM PULL() in obj.c
int cuda_read_contents_from_chunks(Device * device, uint64_t obj_size, uint64_t chunk_cnt, Phys_Chunk ** phys_chunks, void ** ret_obj_contents){

	// cast ret_obj_contents (which is a pointer to the pointer we want to hold contents)
	// to CUdevice ptr to make nvcc happy
	CUdeviceptr * cuda_ptr = (CUdeviceptr *) ret_obj_contents; 

	CUresult result;
	const char * err;

	
	// Need to round obj size up to multiple of chunk size
	uint64_t min_granularity = device -> min_granularity;
	uint64_t aligned_obj_size = round_up_mult_pow_two(obj_size, min_granularity);
	

	// TODO: Need to push device context!

	// 1. Setting context for chunk's device
    Cuda_Context * backend_context = (Cuda_Context *) (device -> backend_context);
    CUcontext ctx = backend_context -> context;
    result = cuCtxPushCurrent(ctx);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not push device context: %s\n", err);
    	return -1;
    }

	
	// 2. Reserve virtual address space
	result = cuMemAddressReserve(cuda_ptr, aligned_obj_size, 0, 0, 0);
	if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not allocate device range: %s\n", err);
    	return -1;
    }

	
	CUmemGenericAllocationHandle phys_mem_handle;
	void * shareable_phys_mem_handle;
	CUmemAllocationHandleType handle_type = CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR;

	Phys_Chunk * phys_chunk;
	uint64_t map_size;
	uint64_t offset = 0;

	// For all but last chunk we know they are full
	for (uint64_t i = 0; i < chunk_cnt; i++){

		phys_chunk = phys_chunks[i];
		map_size = phys_chunk -> size;

		// Determining next chunk to map
		shareable_phys_mem_handle = phys_chunk -> shareable_phys_mem_handle;

		// Importing from shared handle
		result = cuMemImportFromShareableHandle(&phys_mem_handle, shareable_phys_mem_handle, handle_type);
		if (result != CUDA_SUCCESS){
    		cuGetErrorString(result, &err);
    		fprintf(stderr, "Error: Could not import sharable handle: %s\n", err);
    		// cleanup
    		// free virtual address reservation
    		cuMemAddressFree(*cuda_ptr, obj_size);
    		return -1;
    	}

		// if we are the last chunk then modify size to map
		// only last chunk will have internal fragmentation
    	result = cuMemMap(*cuda_ptr + offset, map_size, 0, phys_mem_handle, 0);
    	if (result != CUDA_SUCCESS){
    		cuGetErrorString(result, &err);
    		fprintf(stderr, "Error: Could not do memory mapping: %s\n", err);
    		// cleanup
    		// free virtual address reservation
    		cuMemAddressFree(*cuda_ptr, obj_size);
    		return -1;
    	}

    	offset += map_size;
    }

    // *ret_obj_contents should be fully mapped now


    // 3. Setting access for newly allocated and mapped virtual address range
    CUmemAccessDesc accessDescriptor;

    // UGLY: HARDCODED HERE
    // TODO:
    //	- security concerns
    accessDescriptor.location.id = 0;
    accessDescriptor.location.type = CU_MEM_LOCATION_TYPE_DEVICE;
    // ENSURE IMMUTABLE OBJECTS BECAUSE RELYING ON SHARED MEMORY AND DEDUPED IDs!
    accessDescriptor.flags = CU_MEM_ACCESS_FLAGS_PROT_READ;

    result = cuMemSetAccess(*cuda_ptr, aligned_obj_size, &accessDescriptor, 1);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not set access: %s\n", err);
    	// cleanup
    	// TODO
    	//	- unmapping
    	// free virtual address reservation
    	cuMemAddressFree(*cuda_ptr, obj_size);
    	return -1;
    }


    // 4. Popping context
    result = cuCtxPopCurrent(&ctx);
    if (result != CUDA_SUCCESS){
		cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not pop current context in cuda_write: %s\n", err);
    	// TODO:
    	//	- cleanup
    	return -1;
	}

    return 0;
}
