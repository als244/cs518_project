// Included in all of the backend source files
#ifndef CUDA_BACKEND_H
#define CUDA_BACKEND_H

// For standard lib / system headers
#include "top_level.h"

// structures needed from memory layer in order to build backend
#include "memory_layer_structs.h"
#include "node_structs.h"

// general utilities
#include "utils.h"


// backend-specific includes
// cuda header location part of makefile
#include <cuda.h>
#include "cuda_plugins.h"


// BACKEND-SPECIFIC Structures 

// USED AS THE BACKEND->context in node_structs (moving to host_structs.h)
// Iis located: Device -> generic_context -> backend_context
typedef struct cuda_context {
	int device_id;
	void * context;
} Cuda_Context;


#endif