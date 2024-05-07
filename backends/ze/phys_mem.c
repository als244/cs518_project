#include "ze_backend.h"
// TO QUERY MEM INFO
#include <zes_api.h>


int ze_set_min_chunk_granularity(Device * device){

    ze_result_t result;
    const char * err;

    // Need to get device's context info
    Ze_Context * ze_context = (Ze_Context *) device -> backend_context;
    ze_driver_handle_t drv_handle = ze_context -> drv_handle;
    ze_device_handle_t dev_handle = ze_context -> dev_handle;
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;

    // set unaligned minimum allocation of 1 bytes
    size_t min_size = 1;
    size_t min_granularity;
    result = zeVirtualMemQueryPageSize(ctx_handle, dev_handle, min_size, &min_granularity);
    if (result != ZE_RESULT_SUCCESS){
        zeDriverGetLastErrorDescription(drv_handle, &err);
        fprintf(stderr, "Error: could not query minimum page size: %s\n", err);
        return -1;
    }

    device -> min_granularity = min_granularity;

    return 0;

}


// NORMAL ZE DRIVER DOESN'T HAVE ACCESS
//  - need to use ZES (Sysman driver)
int zes_get_mem_info(Device * device, uint64_t * ret_free, uint64_t * ret_total){

    ze_result_t result;
    const char * err;
    int ret;

    // 1. Init ZES Driver

    #ifndef ZES_INIT_FUNC
    #define ZES_INIT_FUNC
    result = zesInit(0);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Could not init ze sysman: %d\n", result);
        return -1;
    }
    #endif

    // 2. Get Sysman driver handle
    // HARDCODED FOR NOW: UGLY!
    // THIS SHOULD HAPPEN AT INIT TIME AND PASS 0 DEV COUNT FOR INPUT TO RETRIEVE ALL HANDLES
    uint32_t dev_count = 1;
    zes_driver_handle_t zes_drv_handle;
    result = zesDriverGet(&dev_count, &zes_drv_handle);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not get sysman driver handle: %d\n", result);
        return -1;
    }

    // 3. Get System dev handles
    zes_device_handle_t zes_dev_handle;
    result = zesDeviceGet(zes_drv_handle, &dev_count, &zes_dev_handle);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not get sysman device handle: %d\n", result);
        return -1;
    }

    // 4. Retrieve available memory
    zes_mem_handle_t dev_mem_handle;
    result = zesDeviceEnumMemoryModules(zes_dev_handle, &dev_count, &dev_mem_handle);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not get memory handle: %s\n", err);
        return -1;
    }

    // 5. Get memory state
    zes_mem_state_t mem_state;
    result = zesMemoryGetState(dev_mem_handle, &mem_state);
    if (result != ZE_RESULT_SUCCESS){
        fprintf(stderr, "Error: could not get memory state: %s\n", err);
        return -1;
    }

    // set values
    *ret_free = mem_state.free;
    *ret_total = mem_state.size;

    return 0;
}

// TODO:
//	- modularize better!
//	- this function is too big and not just doing ze specific things!


// ASSUMES phys_chunk intialized with correct ids, sizes, except for sharable_phys_mem_handle
// Responsibilities:
//  1. Create exportable memory region with zeMemAllocDevice 
//  2. Export DMA_BUF FD as sharable handle and set phys_chunk -> sharable_phys_mem_handle
int ze_init_shareable_device_phys_mem_handle(Device * device, Phys_Chunk * phys_chunk){

	ze_result_t result;
	const char * err;
	int ret;

	// 1. get handles
	Ze_Context * ze_context = (Ze_Context *) device -> backend_context;
    ze_driver_handle_t drv_handle = ze_context -> drv_handle;
    ze_device_handle_t dev_handle = ze_context -> dev_handle;
    ze_context_handle_t ctx_handle = ze_context -> ctx_handle;


    // 2. Create device mem allocation exportable

    uint64_t min_granularity = device -> min_granularity;
    uint64_t num_min_chunks = phys_chunk -> num_min_chunks;
    uint64_t alloc_size = min_granularity * num_min_chunks;

    ze_physical_mem_desc_t pmemDesc = {
        ZE_STRUCTURE_TYPE_PHYSICAL_MEM_DESC,
        NULL,
        0, // flags
        alloc_size // size
    };

    // Link the request into the allocation descriptor and allocate
    ze_physical_mem_handle_t phys_mem_handle;
    result = zePhysicalMemCreate(ctx_handle, dev_handle, &pmemDesc, &phys_mem_handle);
    if (result != ZE_RESULT_SUCCESS){
        zeDriverGetLastErrorDescription(drv_handle, &err);
        fprintf(stderr, "Error: could not allocate exportable memory: %s\n", err);
        return -1;
    }

    phys_chunk -> shareable_phys_mem_handle = (void *) phys_mem_handle;

    return 0;
}
