#ifndef HIP_PLUGINS_H
#define HIP_PLUGINS_H

// For standard lib / system headers
#include "top_level.h"

// STRUCTURES LINKING THE MEMORY LAYER TO BACKEND
// Easier to deal with forward declaring rather than circular depends
// whereever this header is included it must be that memory layer / node structs already or will be decalared
typedef struct hip_context Hip_Context;
typedef struct device Device;
typedef struct phys_chunk Phys_Chunk;
typedef struct chunks Chunks;

// EXPORTED:

// CALLED FROM init_driver() from init-node() within node.c
//	- will be movinig to init_host next version
int hip_init_driver(unsigned int init_flags);

// CALLED FROM init_node within node.c
//	- will be moving to init_host next version
Hip_Context * hsa_init_context(int device_id);


// CALLED FROM insert_device_memory in chunks.c, 
// which is called from init_device() in node.c (moving to device.c)
// whihc is called from init_node() in node.c (moving to host.h)
// which is called from system_init() in system.init.c
int hip_set_min_chunk_granularity(Device * device);
int hip_get_mem_info(Device * device, uint64_t * ret_free, uint64_t * ret_total);


int hsa_init_shareable_device_phys_mem_handle(Device * device, Phys_Chunk * phys_chunk);

// for now called from write_contents_to_chunk() from push() in obj.c
//		- will be moving to push_hanlder in request_handlers()
//	will change to take int an array of phys chunks to write to
int hsa_write_contents_to_chunk(uint64_t write_size, void * contents, Phys_Chunk * phys_chunk);

// Called from read_contents_from_chunks() from get() in obj.c
int hsa_read_contents_from_chunks(Device * device, uint64_t obj_size, uint64_t chunk_cnt, Phys_Chunk ** phys_chunks, void ** ret_obj_contents);

#endif