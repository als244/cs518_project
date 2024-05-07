#include "hsa_backend.h"

// FOR NOW ASSUMES THAT CHUNK IS ON DEVICE MEMORY
//  - the host as own functions for reading/writing

// TODO:
//  - where are contents? (other device? host? remote? server?)
//  - For now assumes contents are on host!
//  - BULK WRITE? Just like read is bulk read?

// CALLED FROM push() in obj.c
int hsa_write_contents_to_chunk(uint64_t write_size, void * contents, Phys_Chunk * phys_chunk){

    hsa_status_t hsa_status;
    const char * err;

    // need to:
    //      1. Set context for device where phys_chunk lives
    //      2. Import shared handle (created at system init time)
    //      3. Ensure the write size is a multiple of page granularity
    //      3. Reserve virtual address
    //      4. Create mapping to phys handle
    //      5. Set access flags so all devices (intra-node) can access
    //      6. Write contents to virtual address
    //      7. Pop context

    Device * device = (Device *) phys_chunk -> home;
    Hip_Context * hip_context = (Hip_Context *) device -> backend_context;

    hsa_agent_t agent = hip_context -> agent;
    hsa_agent_t cpu_agent = hip_context -> cpu_agent;
    hsa_amd_memory_pool_t cpu_mem_pool = hip_context -> cpu_mem_pool;


    // 2. retrieving shared handle created at system init time
   
    // TEMPORARY
    int dmabuf_fd = (int) (phys_chunk -> shareable_phys_mem_handle);
    hsa_amd_vmem_alloc_handle_t phys_mem_handle;
    hsa_status = hsa_amd_vmem_import_shareable_handle(dmabuf_fd, &phys_mem_handle);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error importing handle: %s\n", err);
        return -1;
    }

    // 3. Reserving VA
    //      - Ensuring size aligns with phys_chunk (physical page in arch) size
    //      - should've been specified in it
    uint64_t size = phys_chunk -> size;

    void * dptr;
    uint64_t addr_req = 0;
    uint64_t va_req_flags = 0;
    hsa_status = hsa_amd_vmem_address_reserve(&dptr, size, addr_req, va_req_flags);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error reserving VA: %s\n", err);
        return -1;
    }


    // 4. Doing memory mapping

    // UNSUPPORTED
    uint64_t map_offset = 0;
    uint64_t map_flags = 0;
    hsa_status = hsa_amd_vmem_map(dptr, size, map_offset, phys_mem_handle, map_flags);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error doing memory mapping: %s\n", err);
        return -1;
    }

    // 6. Setting access flags
    //      - only need to set for both src and dest because doing memcpy to actually write

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

    hsa_status = hsa_amd_vmem_set_access(dptr, size, accessDescriptors, desc_cnt);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error setting access: %s\n", err);
        return -1;
    }

    // TEMPORARY SOLUTIONS

    // CREATE SIGNAL
    // NEEDED FOR MEMCPY ASYNC 
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

    // 7. Make src contents from normal OS memory accessible to src/dest agents
    //      - all memory will be allocated within mem pools in better version so this will not need to happen
    // give access to all agents
    int num_agents = 0;
    hsa_agent_t * access_agents = NULL;
    void * hsa_contents;
    hsa_status = hsa_amd_memory_lock(contents, write_size, access_agents, num_agents, &hsa_contents);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error locking memory and giving agents access to contents: %s\n", err);
        return -1;
    }

    
    // 8. Writing contents (from cpu memory) to device virtual address
    //      - now only use write_size in case this is last phys_chunk with fragmentation

    uint64_t num_dep_signals = 0;
    hsa_signal_t * dep_signals = NULL;
    hsa_signal_t completition_signal = signal;

    // PERFORM MEMCPY
    hsa_status = hsa_amd_memory_async_copy(dptr, agent, hsa_contents, cpu_agent, write_size, num_dep_signals, dep_signals, completition_signal);
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

    // Now sig val should equal zero


    // 7. Now unmapping virtual address
    hsa_status = hsa_amd_vmem_unmap(dptr, size);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error unmapping: %s\n", err);
        return -1;
    }

    // 8. Now freeing virtual address
    hsa_status = hsa_amd_vmem_address_free(dptr, size);
    if (hsa_status != HSA_STATUS_SUCCESS){
        hsa_status_string(hsa_status, &err);
        fprintf(stderr, "Error freeing va space: %s\n", err);
        return -1;
    }

    

    return 0;
}