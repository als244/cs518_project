#ifndef PHYS_CHUNK_H
#define PHYS_CHUNK_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "deque.h"
#include "host.h"
#include "hsa_plugins.h"

// EXPORT

//	- All used in init_device() in node.c (will be moving to device.c)
//		- Called by init_node() (which will move to hsot.c)
//			- Called by system_init()
Phys_Chunk * init_phys_chunk(Location location, void * home, uint64_t id, uint64_t size, uint64_t access_flags);
int insert_free_chunk(Location location, void * home, Phys_Chunk * phys_chunk);

#endif