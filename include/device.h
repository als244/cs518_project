#ifndef DEVICE_H
#define DEVICE_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "system_config.h"
#include "master.h"
#include "hsa_plugins.h"


// TODO:
//	- modularize node.c / node.h / node_structs.h
//	- instead of node: create device and host


// IMPORT:

// From chunks.c
//	- this takes care of initializing & inserting device -> chunks
//	- all used in init_device()
int init_chunks_container(Location location, void * home, uint64_t max_chunks);
// From phys_chunk.c
Phys_Chunk * init_phys_chunk(Location location, void * home, uint64_t id, uint64_t size, uint64_t access_flags);
int insert_free_chunk(Location location, void * home, Phys_Chunk * phys_chunk);

// From obj_table.c
Obj_Table * init_obj_table(Obj_Table_Init_Config * obj_table_init_config);


// EXPORT:
Device * init_device(Node * host_node, Device_Init_Config * device_init_config);
void destroy_device(Device * device);

#endif