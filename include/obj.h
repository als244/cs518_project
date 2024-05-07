#ifndef OBJ_H
#define OBJ_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "obj_table.h"
#include "chunks.h"
#include "deque.h"
#include "hsa_plugins.h"

// DEFINING INTERFACE FUNCTIONS FOR BUILD PROCESS

// EXPORT
uint64_t dedup_hash(void * obj_contents);
int push(uint64_t obj_size, void * obj_contents, Device * dest_device, Node * src_node, uint64_t * ret_obj_id);
int pull(uint64_t obj_id, Device * dest_device, Node * src_node, uint64_t * ret_obj_size, void ** ret_obj_contents);

#endif