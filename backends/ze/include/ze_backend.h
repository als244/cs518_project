// Included in all of the backend source files
#ifndef ZE_BACKEND_H
#define ZE_BACKEND_H

// For standard lib / system headers
#include "top_level.h"

// structures needed from memory layer in order to build backend
#include "memory_layer_structs.h"
#include "node_structs.h"

// general utilities
#include "utils.h"


// backend-specific includes
// ze header location part of makefile
#include <ze_api.h>
#include "ze_plugins.h"


// BACKEND-SPECIFIC Structures 

// USED AS THE BACKEND->context in node_structs (moving to host_structs.h)
// Iis located: Device -> generic_context -> backend_context

// THIS IS ALL INITIALIZED IN context.c
typedef struct ze_context {
	int device_id;
	void * drv_handle;
	void * dev_handle;
	void * ctx_handle;
	void * cmd_queue_handle;
} Ze_Context;


#endif