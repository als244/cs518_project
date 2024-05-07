#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cuda.h>

uint64_t round_up_mult_pow_two(uint64_t orig, uint64_t multiple){
	return (orig + multiple - 1) & (~(multiple - 1));
}

int init_driver(uint64_t init_flags){
	// Intitialize User-space Driver
	CUresult result;
	const char * err;

    result = cuInit(init_flags);

    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Could not init cuda: %s\n", err);
    	return -1;
    }
    return 0;
}

int init_and_push_context(int device_id) {
	CUresult result;
	const char * err;
	CUcontext ctx;
    unsigned int ctx_create_flags = 0;
    CUdevice dev_id = device_id;
    result = cuCtxCreate(&ctx, ctx_create_flags, dev_id);

    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Could not create context: %s\n", err);
    	return -1;
    }

    result = cuCtxPushCurrent(ctx);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Could not push context: %s\n", err);
    	return -1;
    }

    return 0;

}

CUmemAllocationProp set_default_alloc_prop(int device_id){
	
	CUmemAllocationProp prop = {};
	prop.type = CU_MEM_ALLOCATION_TYPE_PINNED;
	prop.location.type = CU_MEM_LOCATION_TYPE_DEVICE;
	prop.location.id = device_id;
	return prop;

}

// ASSUMES CONTEXT FOR DEVICE WAS SET
// Using default allocation properties
int get_min_chunk_granularity(int device_id, uint64_t * granularity, CUmemAllocationProp * prop){
	const char * err;
	CUmemAllocationGranularity_flags min = CU_MEM_ALLOC_GRANULARITY_MINIMUM;
	CUresult result = cuMemGetAllocationGranularity(granularity, prop, min);
	if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Could not get granularity: %s\n", err);
    	return -1;
    }

    return 0;
}

// takes in size and address of allocHandle to set
// will be default pinned settings on device
int alloc_phys_chunk(uint64_t alloc_size, CUmemGenericAllocationHandle * generic_handle, CUmemAllocationProp * prop){

	const char * err;
	// Assumes other properties (device type, device_id, pinned) alreay set
    prop -> requestedHandleTypes = CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR;
    CUresult result = cuMemCreate(generic_handle, alloc_size, prop, 0);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not get alloc handle: %s\n", err);
    	// TODO:
    	//	- more cleanup needed here...
    	return -1;
    }

    return 0;
}


int export_to_shareable_handle(CUmemGenericAllocationHandle phys_mem_handle, void ** shareable_phys_mem_handle){

	const char * err;
    CUresult result = cuMemExportToShareableHandle((void *) shareable_phys_mem_handle, phys_mem_handle, CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR, 0);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not export to sharable handle: %s\n", err);
    	return -1;
    }
    return 0;
}

int import_from_shareable_handle(void * shareable_handle, CUmemGenericAllocationHandle * handle) {

	const char * err;
	CUresult result = cuMemImportFromShareableHandle(handle, shareable_handle, CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR);
	if (result != CUDA_SUCCESS){
		cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not import from sharable handle: %s\n", err);
    	return -1;
	}
	return 0;
}


int reserve_va_space(uint64_t va_range_size, void ** ret_va_ptr){
	const char * err;
	CUresult result = cuMemAddressReserve((CUdeviceptr *) ret_va_ptr, va_range_size, 0, 0, 0);
	if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not allocate device range: %s\n", err);
    	return -1;
    }
    return 0;
}


int mem_map(uint64_t map_size, void * va_ptr, CUmemGenericAllocationHandle phys_mem_handle){
	const char * err;
	CUresult result = cuMemMap((CUdeviceptr) va_ptr, map_size, 0, phys_mem_handle, 0);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not do memory mapping: %s\n", err);
    	return -1;
    }

}

int set_access(int device_id, uint64_t va_range_size, void * va_ptr){
	const char * err;
	CUmemAccessDesc accessDescriptor;
    accessDescriptor.location.id = device_id;
    accessDescriptor.location.type = CU_MEM_ALLOCATION_TYPE_PINNED;
    accessDescriptor.flags = CU_MEM_ACCESS_FLAGS_PROT_READ;

    CUresult result = cuMemSetAccess((CUdeviceptr) va_ptr, va_range_size, &accessDescriptor, 1);
    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not set access: %s\n", err);
    	return -1;
    }
}


int main(int argc, char *argv[]) {


	int ret;

	// Initialize Cuda Driver
	unsigned int driver_flags;
	ret = init_driver(driver_flags);
	if (ret != 0){
		exit(1);
	}

    // Initialize Device Context
    int device_id = 0;
    ret = init_and_push_context(device_id);
    if (ret != 0){
    	exit(1);
    }

    // Setting default properties
  	CUmemAllocationProp prop = set_default_alloc_prop(device_id);

    // Get min chunk granularity   
	uint64_t granularity = 0;
	ret = get_min_chunk_granularity(device_id, &granularity, &prop);
	if (ret != 0){
		exit(1);
	}

	// Perform phys mem allocation

	// Setting alloc size to 1 gpu page
	uint64_t raw_alloc_size = (uint64_t) 1 * (1 << 21);
	


	// assumes granularity is power of 2...
	uint64_t alloc_size = round_up_mult_pow_two(raw_alloc_size, granularity);

	printf("DOING TIMINGS WITH ALLOC SIZE = %ld\n\n", alloc_size);
	

	struct timespec start, stop;
	uint64_t timestamp_start, timestamp_stop, elapsed;


	// 1. Allocating Physical Memory
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	CUmemGenericAllocationHandle phys_mem_handle;
	ret = alloc_phys_chunk(alloc_size, &phys_mem_handle, &prop);
	if (ret != 0){
		exit(1);
	}

	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("\tAlloc Phys Chunk Took: %ld\n", elapsed);


	// 2. Exporting to Shareable Handle
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	void * shareable_phys_mem_handle;
	ret = export_to_shareable_handle(phys_mem_handle, &shareable_phys_mem_handle);
	if (ret != 0){
		exit(1);
	}

	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("\tExport to Shareable Handle Took: %ld\n", elapsed);


	// 3. Importing from Shareable Handle
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	CUmemGenericAllocationHandle imported_handle;
	ret = import_from_shareable_handle(shareable_phys_mem_handle, &imported_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("\tImporting From Shareable Handle Took: %ld\n", elapsed);


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
	printf("\tReserving VA Space Took: %ld\n", elapsed);


	// 5. Doing memory mapping
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	uint64_t map_size = alloc_size;
	ret = mem_map(map_size, va_ptr, imported_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("\tMemory Mapping Took: %ld\n", elapsed);


	// 6. Setting Access
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	
	ret = set_access(device_id, va_range_size, va_ptr);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("\tSetting Access Took: %ld\n\n", elapsed);

	return 0;

}


// CONCLUSIONS:
//	- Allocating physical memory slightly scales
//		- 30 mircoseconds for 1 page and 75 mics for 10000 pages
//		- Noticed very large spikes, however. Beware of this variance
//			- Spikes up to a few hundred microseconds
//	- Export to sharable handle is constant
//		- 85-90 microseconds
//		- Also noticed some jitter here (up to around 120 mics)
//	- Impoting from handle is constant
//		- 35-40 microseconds
//	- Reserving VA Space slightly scales
//		- 1-2 mics for 1 page and 8 mics for 10000 pages
//	- Mem Map is constant
//		- 2-3 mics
//	- Setting access is expensive when scaling
//		- 45 mics for 1 page, 210 mics for 1000 pages, 750 mics for 10000 pages