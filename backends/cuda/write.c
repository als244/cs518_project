#include "cuda_backend.h"



// FOR NOW ASSUMES THAT CHUNK IS ON DEVICE MEMORY
//	- the host as own functions for reading/writing

// TODO:
//	- where are contents? (other device? host? remote? server?)
//	- For now assumes contents are on host!
//	- BULK WRITE? Just like read is bulk read?

// CALLED FROM push() in obj.c
int cuda_write_contents_to_chunk(uint64_t write_size, void * contents, Phys_Chunk * phys_chunk){

	CUresult result;
	const char * err;

	// need to:
	// 		1. Set context for device where phys_chunk lives
	//		2. Import shared handle (created at system init time)
	//		3. Ensure the write size is a multiple of page granularity
	//		3. Reserve virtual address
	// 		4. Create mapping to phys handle
	//		5. Set access flags so all devices (intra-node) can access
	//		6. Write contents to virtual address
	//		7. Pop context

	Device * device = (Device *) phys_chunk -> home;
	uint64_t device_id = device -> local_device_id;

	// 1. Setting context for phys_chunk's device
    Cuda_Context * backend_context = (Cuda_Context *) (device -> backend_context);
    CUcontext ctx = backend_context -> context;
    result = cuCtxPushCurrent(ctx);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not push device context: %s\n", err);
    	return -1;
    }

    // 2. retrieving shared handle created at system init time
    CUmemGenericAllocationHandle phys_mem_handle; 
	// HARDCODING for posix here
	CUmemAllocationHandleType handle_type = CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR;
    result = cuMemImportFromShareableHandle(&phys_mem_handle, phys_chunk -> shareable_phys_mem_handle, handle_type);
	if (result != CUDA_SUCCESS){
		cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not retrieve sharable phys mem handle: %s\n", err);
    	// cleanup
    	cuCtxPopCurrent(&ctx);
    	return -1;
	}

	// 3. Ensuring size aligns with phys_chunk (physical page in arch) size
	//		- should've been specified in it
	uint64_t size = phys_chunk -> size;


	// 4. Reserving virtual address
	CUdeviceptr dptr;
	result = cuMemAddressReserve(&dptr, size, 0, 0, 0);
	if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not allocate device range: %s\n", err);
    	// cleanup
    	cuCtxPopCurrent(&ctx);
    	return -1;
    }

    // 5. Doing memory mapping
    result = cuMemMap(dptr, size, 0, phys_mem_handle, 0);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not do memory mapping: %s\n", err);
    	// cleanup
    	// free virtual address reservation
    	cuMemAddressFree(dptr, size);
    	// pop device context
    	cuCtxPopCurrent(&ctx);
    	return -1;
    }

    // 6. Setting access flags
    //		- only need to set for this one-device because other consumers will have different virt addr space
   	CUmemAccessDesc accessDescriptor;
    accessDescriptor.location.id = device_id;
    accessDescriptor.location.type = CU_MEM_ALLOCATION_TYPE_PINNED;
    // ENSURE IMMUTABLE OBJECTS BECAUSE RELYING ON SHARED MEMORY AND DEDUPED IDs!
    accessDescriptor.flags = CU_MEM_ACCESS_FLAGS_PROT_READWRITE;
    result = cuMemSetAccess(dptr, size, &accessDescriptor, 1);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not copy contents to device: %s\n", err);
    	// cleanup
    	// unmap the backing phys_chunk
    	cuMemUnmap(dptr, size);
    	// free virtual address reservation
    	cuMemAddressFree(dptr, size);
    	// pop device context
    	cuCtxPopCurrent(&ctx);
    	return -1;
    }

    // 7. Writing contents (from cpu memory) to device virtual address
    //		- now only use write_size in case this is last phys_chunk with fragmentation
    result = cuMemcpyHtoD(dptr, contents, write_size);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not copy contents from host to device in cuda_write: %s\n", err);
    	// cleanup
    	// unmap the backing phys_chunk
    	cuMemUnmap(dptr, size);
    	// free virtual address reservation
    	cuMemAddressFree(dptr, size);
    	// pop device context
    	cuCtxPopCurrent(&ctx);
    	return -1;
    }

    // 7. Removing this device's context from current cpu thread
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