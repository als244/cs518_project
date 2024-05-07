#ifndef REQUEST_HANDLERS_H
#define REQUEST_HANDLERS_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "server_structs.h"
#include "client_structs.h"
#include "hsa_plugins.h"

// IMPORT

// Functions:

// From obj.c
uint64_t dedup_hash(void * obj_contents);
int push(uint64_t obj_size, void * obj_contents, Device * dest_device, Node * src_node, uint64_t * ret_obj_id);
int pull(uint64_t obj_id, Device * dest_device, Node * src_node, uint64_t * ret_obj_size, void ** ret_obj_contents);


// EXPORT:
//	- used in server.c
Response * handle_request(Server * server, Request * request);

#endif