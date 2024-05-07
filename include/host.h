#ifndef HOST_H
#define HOST_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "system_config.h"
#include "master.h"


// IMPORT

// From chunks.c
//	- this takes care of initializing & inserting device -> chunks
//	- all used in init_device()
int init_chunks_container(Location location, void * home, uint64_t max_chunks);
// From phys_chunk.c
Phys_Chunk * init_phys_chunk(Location location, void * home, uint64_t id, uint64_t size, uint64_t access_flags);
int insert_free_chunk(Location location, void * home, Phys_Chunk * phys_chunk);

// EXPORT
Host * init_host(Node * node, Host_Init_Config * host_init_config);
void destroy_host(Host * host);

// SHOULDN'T BE HERE
//	- need to have os-specific "backends"
//	- ok, for now...
int posix_init_shareable_host_phys_mem_handle(Host * host, Phys_Chunk * phys_chunk);

#endif