// Included in all of the backend source files
#ifndef HIP_BACKEND_H
#define HIP_BACKEND_H

// For standard lib / system headers
#include "top_level.h"

// structures needed from memory layer in order to build backend
#include "memory_layer_structs.h"
#include "node_structs.h"

// general utilities
#include "utils.h"


// backend-specific includes
// hip header location part of makefile
#include <hip/hip_runtime_api.h>

// FOR THE DMA BUF
#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include "hsa_plugins.h"


// BACKEND-SPECIFIC Structures 

// USED AS THE BACKEND->context in node_structs (moving to host_structs.h)
// Iis located: Device -> generic_context -> backend_context
typedef struct hip_context {
	int device_id;
	hsa_agent_t agent;
	hsa_agent_t cpu_agent;
	hsa_amd_memory_pool_t mem_pool;
	hsa_amd_memory_pool_t cpu_mem_pool;
} Hip_Context;

typedef struct agents {
	int n_agents;
	hsa_agent_t * agents;
} Agents;

typedef struct mem_pools {
	int n_mem_pools;
	hsa_amd_memory_pool_t * mem_pools;
} MemPools;


#endif