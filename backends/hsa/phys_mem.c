#include "hsa_backend.h"


int hip_set_min_chunk_granularity(Device * device){

    hipError_t result;
    const char * err;

    // Need to set device's context and query driver
    Hip_Context * hip_context = (Hip_Context *) device -> backend_context;
    int device_id = hip_context -> device_id;
    result = hipSetDevice(device_id);
    if (result != hipSuccess){
        err = hipGetErrorString(result);
        fprintf(stderr, "Error: Could not set hip device: %s\n", err);
        return -1;
    }
    

    uint64_t min_granularity = 0;
    hipMemGenericAllocationHandle_t allocHandle;
    hipMemAllocationProp prop = {};
    prop.type = hipMemAllocationTypePinned;
    prop.location.type = hipMemLocationTypeDevice;
    prop.location.id = device_id;

    hipMemAllocationGranularity_flags min = hipMemAllocationGranularityMinimum;

    result = hipMemGetAllocationGranularity(&min_granularity, &prop, min);

    if (result != hipSuccess){
        err = hipGetErrorString(result);
        fprintf(stderr, "Could not get granularity: %s\n", err);
        return -1;
    }

    // MY 7900 XT has minimum granularity of 4KB
    //  - HOWEVER -- the ROCm 6.1 Virtual Memory Management has bugs for suballocation and \
    //              setting access for multiple chunks within a given range
    //device -> min_granularity = min_granularity;

    // TEMPORARY
    // manually setting to 64KB because that is default virtual page size
    device -> min_granularity = 1 << 21;

    

    return 0;

}

int hip_get_mem_info(Device * device, uint64_t * ret_free, uint64_t * ret_total){

    hipError_t result;
    const char * err;
    int ret;

    // 1. set context for device
    Hip_Context * hip_context = (Hip_Context *) device -> backend_context;
    int device_id = hip_context -> device_id;
    result = hipSetDevice(device_id);
    if (result != hipSuccess){
        err = hipGetErrorString(result);
        fprintf(stderr, "Error: Could not set hip device: %s\n", err);
        return -1;
    }

    // 2. Retrieve available memory
    //      - Could also query for alloc granularity here for future portability
    //          - For now assuming 2^21 chunk size
    uint64_t free, total;
    result = hipMemGetInfo(&free, &total);
    if (result != hipSuccess){
        err = hipGetErrorString(result);
        fprintf(stderr, "Error: Could not get mem info: %s\n", err);
        // TODO:
        //  - cleanup
        return -1;
    }

    // set values
    *ret_free = free;
    *ret_total = total;

    return 0;
}



// VERY IMPORTANT
// WHY DOES THE ROCM-SMI REPORT DOUBLE THE AMOUNT OF MEMORY THAN WE ALLOCATE WITH CREATE????

// ASSUMES phys_chunk intialized with correct ids, sizes, except for sharable_phys_mem_handle
// Responsibilities:
//  1. Set device context
//  2. Create chunk
//  3. Export to sharable handle and set phys_chunk -> sharable_phys_mem_handle
//  4. Pop device context
int hsa_init_shareable_device_phys_mem_handle(Device * device, Phys_Chunk * phys_chunk){

	hipError_t result;

    hsa_status_t hsa_status;
	const char * err;
	int ret;

    uint64_t min_granularity = device -> min_granularity;
    uint64_t num_min_chunks = phys_chunk -> num_min_chunks;
    uint64_t alloc_size = min_granularity * num_min_chunks;
    
    // 1. Create phys mem allocation
    Hip_Context * hip_context = (Hip_Context *) device -> backend_context;
    hsa_agent_t gpu_agent = hip_context -> agent;
    hsa_amd_memory_pool_t gpu_mem_pool = hip_context -> mem_pool;

    hsa_amd_memory_type_t mem_type = MEMORY_TYPE_PINNED;
    // CURRENTLY UNSUPPORTED
    uint64_t create_flags = 0;

    hsa_amd_vmem_alloc_handle_t phys_mem_handle;

    hsa_status = hsa_amd_vmem_handle_create(gpu_mem_pool, alloc_size, mem_type, create_flags, &phys_mem_handle);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error iterating agents: %s\n", err);
    }

    // 2. Export shared handle
    //		- Passing in address of sharable handle that's within chunk we just initialized
    //		- Need to be careful about open file limits!
    
    int dmabuf_fd;
    // CURRENTLY UNSUPPORTED
    uint64_t export_flags = 0;
    hsa_status = hsa_amd_vmem_export_shareable_handle(&dmabuf_fd, phys_mem_handle, export_flags);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error exporting shareable handle: %s\n", err);
    }

    phys_chunk -> shareable_phys_mem_handle = (void *) dmabuf_fd;

    return 0;
}
