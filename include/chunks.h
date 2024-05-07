#ifndef CHUNKS_H
#define CHUNKS_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "deque.h"

// EXPORT

//	- Used in init_device() in node.c (will be moving to device.c)
//		- Called by init_node() (which will move to hsot.c)
//			- Called by system_init()
int init_device_chunks_container(Device * device, uint64_t max_chunks);


#endif