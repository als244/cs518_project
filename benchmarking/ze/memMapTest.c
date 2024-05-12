#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <ze_api.h>

typedef struct ze_context {
	int device_id;
	void * drv_handle;
	void * dev_handle;
	void * ctx_handle;
	void * cmd_queue_handle;
	uint64_t min_granularity;
} Ze_Context;


uint64_t get_aligned_size(uint64_t obj_size, uint64_t page_size){
	int remainder = obj_size % page_size;
    if (remainder == 0)
        return obj_size;

    return obj_size + page_size - remainder;
}

// HARDCODE TO 2 MB
// Intel has 2 different min granualrities (64K and 2MB) depending on alloc size
uint64_t get_min_alloc_granularity(Ze_Context * ze_context) {
	return 1 << 21;
}


int ze_initialize(int device_id, Ze_Context ** ret_ze_context){

	// 1.) Init Driver
	ze_result_t result;

	// unused init flags
	unsigned long init_flags = 0;
    result = zeInit(init_flags);

    if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Could not init ze: %d\n", result);
    	return -1;
    }

    // 2.) Init Context
	Ze_Context * ze_context = (Ze_Context *) malloc(sizeof(Ze_Context));

	if (ze_context == NULL){
		fprintf(stderr, "Error: malloc failed in init_ze_context\n");
		return -1;
	}

	// a.) Set devcie id
	ze_context -> device_id = device_id;

	// b.) Set driver_handle
	ze_driver_handle_t drv_handle;
	uint32_t dev_count = 1;
	result = zeDriverGet(&dev_count, &drv_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not get driver handle: %d\n", result);
    	return -1;
	}
	ze_context -> drv_handle = (void *) drv_handle;

	// c.) Set device handle
	ze_device_handle_t dev_handle;
	result = zeDeviceGet(drv_handle, &dev_count, &dev_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not get device handle: %d\n", result);
    	return -1;
	}
	ze_context -> dev_handle = dev_handle;

	// 4.) Set context handle
	ze_context_desc_t ctxDesc = {
   		ZE_STRUCTURE_TYPE_CONTEXT_DESC,
   		NULL,
   		0
	};
	ze_context_handle_t ctx_handle;
	result = zeContextCreate(drv_handle, &ctxDesc, &ctx_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not create context: %d\n", result);
    	return -1;
	}
	ze_context -> ctx_handle = (void *) ctx_handle;

	// 5.) Create command queue
	ze_command_queue_handle_t cmd_queue_handle;

	ze_command_queue_desc_t cmd_queue_desc = {
		ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
		NULL,
		0,
		0,
		ZE_COMMAND_QUEUE_MODE_SYNCHRONOUS,
		ZE_COMMAND_QUEUE_PRIORITY_NORMAL
	};

	result = zeCommandQueueCreate(ctx_handle, dev_handle, &cmd_queue_desc, &cmd_queue_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not create command queue: %d\n", result);
    	return -1;
	}

	ze_context -> cmd_queue_handle = cmd_queue_handle;

	ze_context -> min_granularity = get_min_alloc_granularity(ze_context);

	*ret_ze_context = ze_context;

	return 0;
}



// takes in size and address of allocHandle to set
// will be default pinned settings on device
int ze_allocate_phys(Ze_Context * ze_context, uint64_t alloc_size, ze_physical_mem_handle_t * ret_phys_mem_handle){

	ze_result_t result;
	const char * err;

	// 1. get handles
    ze_device_handle_t dev_handle = ze_context -> dev_handle;
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;
    uint64_t min_granularity = ze_context -> min_granularity;

    // 2. Create device mem allocation exportable
    ze_physical_mem_desc_t pmemDesc = {
        ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC,
        NULL,
        0, // flags
        alloc_size // size
    };

    // ensure allocation is exportable
    
    // Needed to pass to zeMemAllocDevice upon alloc to make the allocation exportable
    // ze_external_memory_export_desc_t export_desc = {
    // 	ZE_STRUCTURE_TYPE_EXTERNAL_MEMORY_EXPORT_DESC,
    // 	NULL,
    // 	ZE_EXTERNAL_MEMORY_TYPE_FLAG_DMA_BUF
    // }
	// pmemDesc.next = &export_desc;

    // Link the request into the allocation descriptor and allocate
    ze_physical_mem_handle_t phys_mem_handle;
    result = zePhysicalMemCreate(ctx_handle, dev_handle, &pmemDesc, &phys_mem_handle);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not allocate exportable memory: %d\n", result);
        return -1;
    }

    *ret_phys_mem_handle = phys_mem_handle;

    return 0;
}


int ze_export_to_dmabuf(Ze_Context * ze_context, ze_physical_mem_handle_t phys_mem_handle, int * ret_dmabuf_fd){
    
    // Think thi is ONLY supported for zeMemAllocDevice and zeMemAllocHost
	// and NOT zePhysicalMemCreate + zeVirtualMemReservev + zeVirtualMemMap


	// Needed to pass to zeMemGetAllocProperties in order to retrieve dmabuf fd
	
	// ze_external_memory_export_fd_t export_fd = {
	// 	ZE_STRUCTURE_TYPE_EXTERNAL_MEMORY_EXPORT_FD,
	// 	NULL,
	// 	ZE_EXTERNAL_MEMORY_TYPE_FLAG_OPAQUE_FD,
	// 	0 // [out] fd
	// };
	// zeMemGetAllocProperties(ctx_handle, ptr, &alloc_props, NULL);

	fprintf(stderr, "Unimplemented err: ze_import_from_dmabuf not implemented\n");
}


int ze_import_from_dmabuf(int dmabuf_fd, ze_physical_mem_handle_t * ret_handle) {

	// Think thi is ONLY supported for zeMemAllocDevice and zeMemAllocHost
	// and NOT zePhysicalMemCreate + zeVirtualMemReservev + zeVirtualMemMap

	// ze_external_memory_import_fd_t import_fd = {
	// 	ZE_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMPORT_FD,
	// 	NULL,
	// 	ZE_EXTERNAL_MEMORY_TYPE_FLAG_DMA_BUF,
	// 	fd // [in] dma_buf fd
	// }

	// alloc_desc.pNext = &import_fd;
	// zeMemAllocDevice(ctx_handle, &alloc_desc, size, alignment, dev_handle, &ptr);

	fprintf(stderr, "Unimplemented err: ze_import_from_dmabuf not implemented\n");
}


int reserve_va_space(Ze_Context * ze_context, uint64_t va_range_size, void ** ret_va_ptr){
	
	ze_result_t result;
	const char * err;

	// 1.) get handle
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;
    uint64_t min_granularity = ze_context -> min_granularity;

	// 2.) Reserva va
	void * d_ptr;
	const void * start_addr = NULL;
	uint64_t aligned_size = get_aligned_size(va_range_size, min_granularity);
	result = zeVirtualMemReserve(ctx_handle, start_addr, aligned_size, &d_ptr);
	if (result != ZE_RESULT_SUCCESS){
		fprintf(stderr, "Error: could not reserve va space: %d\n", result);
        return -1;
	}

	*ret_va_ptr = d_ptr;

	return 0;

}


int mem_map(Ze_Context * ze_context, uint64_t map_size, void * va_ptr, ze_physical_mem_handle_t phys_mem_handle){
	
	ze_result_t result;
	const char * err;

	// 1.) get handle
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;
    uint64_t min_granularity = ze_context -> min_granularity;

	// 2.) do mapping
	uint64_t aligned_size = get_aligned_size(map_size, min_granularity);
	result = zeVirtualMemMap(ctx_handle, va_ptr, aligned_size, phys_mem_handle, 0, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE);
	if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not do memory mapping: %d\n", result);
        return -1;
    }

    return 0;

}


int set_access(Ze_Context * ze_context, uint64_t va_range_size, void * va_ptr){
	
	ze_result_t result;
	const char * err;

	// 1.) get handle
	ze_context_handle_t ctx_handle = ze_context -> ctx_handle;
    uint64_t min_granularity = ze_context -> min_granularity;

    // 2.) set access
    uint64_t aligned_size = get_aligned_size(va_range_size, min_granularity);
    result = zeVirtualMemSetAccessAttribute(ctx_handle, va_ptr, aligned_size, ZE_MEMORY_ACCESS_ATTRIBUTE_READWRITE);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not set access attribute: %d\n", result);
        return -1;
    }

    return 0;
}

int mem_unmap(Ze_Context * ze_context, void * va_ptr, uint64_t va_range_size){
	
	ze_result_t result;
	const char * err;

	// 1.) get handle
	ze_context_handle_t ctx_handle = ze_context -> ctx_handle;

	// 2.) unmap
    result = zeVirtualMemUnmap(ctx_handle, va_ptr, va_range_size);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not do unmapping: %d\n", result);
        return -1;
    }
    return 0;
}

// Freeing before unmapping also does unmapping (but doesn't destroy physical allocations)
int free_va_space(Ze_Context * ze_context, void * va_ptr, uint64_t va_range_size) {
	ze_result_t result;
	const char * err;

	// 1.) get handle
	ze_context_handle_t ctx_handle = ze_context -> ctx_handle;

	// 2.) free va
    result = zeVirtualMemFree(ctx_handle, va_ptr, va_range_size);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not free virtual range: %d\n", result);
        return -1;
    }
    return 0;
}

int release_phys_mem(Ze_Context * ze_context, ze_physical_mem_handle_t phys_mem_handle){
	ze_result_t result;
	const char * err;

	// 1.) get handle
	ze_context_handle_t ctx_handle = ze_context -> ctx_handle;

	// 2.) free physical memory
    result = zePhysicalMemDestroy(ctx_handle, phys_mem_handle);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not release/destroy physical memory: %d\n", result);
        return -1;
    }
    return 0;
}


int close_shareable_handle(int dmabuf_fd) {

	// Think thi is ONLY supported for zeMemAllocDevice and zeMemAllocHost
	// and NOT zePhysicalMemCreate + zeVirtualMemReservev + zeVirtualMemMap

	fprintf(stderr, "Unimplemented err: close_shareable_handle not implemented\n");

}





int main(int argc, char *argv[]) {


	int ret;

	// Initialize HSA driver + agents + mem pools
	Ze_Context * ze_context;
	int device_id = 0;
	ret = ze_initialize(device_id, &ze_context);

	// Perform phys mem allocation

	// Setting alloc size to input argument
	uint64_t raw_alloc_size = (uint64_t) atol(argv[1]);
	
	// assumes granularity is power of 2...

	// hardcoding to 1 << 21
	uint64_t min_granularity = get_min_alloc_granularity(ze_context);
	uint64_t alloc_size = get_aligned_size(raw_alloc_size, min_granularity);

	// Print alloc size
	printf("%ld,", alloc_size);

	struct timespec start, stop;
	uint64_t timestamp_start, timestamp_stop, elapsed;

	// 1. Allocating Physical Memory
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	ze_physical_mem_handle_t phys_mem_handle;
	ret = ze_allocate_phys(ze_context, alloc_size, &phys_mem_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 2. UNSUPPORTED: Exporting to Shareable Handle
	// clock_gettime(CLOCK_REALTIME, &start);
	// timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	// int dmabuf_fd;
	// ret = hsa_export_to_dmabuf(phys_mem_handle, &dmabuf_fd);
	// if (ret != 0){
	// 	exit(1);
	// }

	// clock_gettime(CLOCK_REALTIME, &stop);
	// timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	// elapsed = timestamp_stop - timestamp_start;
	// printf("%ld,", elapsed);


	// 3.  UNSUPPORTED: Importing from Shareable Handle
	// clock_gettime(CLOCK_REALTIME, &start);
	// timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	// hsa_amd_vmem_alloc_handle_t imported_handle;
	// ret = import_from_dmabuf(dmabuf_fd, &imported_handle);
	// if (ret != 0){
	// 	exit(1);
	// }
	// clock_gettime(CLOCK_REALTIME, &stop);
	// timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	// elapsed = timestamp_stop - timestamp_start;
	// printf("%ld,", elapsed);


	// 4. Reserving VA Space
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	void * va_ptr;
	uint64_t va_range_size = alloc_size;
	ret = reserve_va_space(ze_context, va_range_size, &va_ptr);
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
	ret = mem_map(ze_context, map_size, va_ptr, phys_mem_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);


	// 6. Setting Access
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;

	ret = set_access(ze_context, va_range_size, va_ptr);
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
	ret = mem_unmap(ze_context, va_ptr, va_range_size);
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
	ret = free_va_space(ze_context, va_ptr, va_range_size);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld,", elapsed);

	// NOTE
	//	- physical memory is only freed upon explicit zePhysicalMemDestroy
	//	- unclear when using the regualr zeMemAllocDevice + exporting to dmabuf / importing...

	// 9. UNSUPPORTED: Close the imported dmabuf
	// clock_gettime(CLOCK_REALTIME, &start);
	// timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	// ret = close_shareable_handle(dmabuf_fd);
	// if (ret != 0){
	// 	exit(1);
	// }
	// clock_gettime(CLOCK_REALTIME, &stop);
	// timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	// elapsed = timestamp_stop - timestamp_start;
	// printf("%ld,", elapsed);

	// 10. UNSUPPORTED: Release imported physical memory
	// clock_gettime(CLOCK_REALTIME, &start);
	// timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	// ret = release_phys_mem(imported_handle);
	// if (ret != 0){
	// 	exit(1);
	// }
	// clock_gettime(CLOCK_REALTIME, &stop);
	// timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	// elapsed = timestamp_stop - timestamp_start;
	// printf("%ld,", elapsed);

	
	// 11. Finally release the physical allocation which creates free page frames
	clock_gettime(CLOCK_REALTIME, &start);
	timestamp_start = start.tv_sec * 1e9 + start.tv_nsec;
	ret = release_phys_mem(ze_context, phys_mem_handle);
	if (ret != 0){
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &stop);
	timestamp_stop = stop.tv_sec * 1e9 + stop.tv_nsec;
	elapsed = timestamp_stop - timestamp_start;
	printf("%ld\n,", elapsed);

	return 0;

}