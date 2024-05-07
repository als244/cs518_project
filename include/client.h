#ifndef CLIENT_H
#define CLIENT_H

#include "top_level.h"
#include "client_structs.h"
// only included for now while testing
#include "system_config.h"
#include "system_init.h"
#include "server_structs.h"
// include backends but also only for now while testing
#include "hsa_plugins.h"


// IMPORT
//	- for now because testing
//	- instead of sending request, directly call handle_request
// From request_handlers.c
Response * handle_request(Server * server, Request * request);


// EXPORT

Client * init_client(uint64_t client_id, uint64_t request_id);

Push_Request * init_push_request(uint64_t obj_size, void * obj_contents, uint64_t dest_global_device_id);
Pull_Request * init_pull_request(uint64_t obj_id, uint64_t dest_global_device_id);
Reserve_Request * init_reserve_request(uint64_t obj_id, uint64_t dest_global_device_id);
Request * init_request(Client * client, OperationType request_type, void * generic_request);

Response * send_request(Server * server, Request * request);

int client_push(Client * client, Server * server, uint64_t obj_size, void * obj_contents, uint64_t dest_global_device_id, uint64_t * ret_obj_id);
int client_pull(Client * client, Server * server, uint64_t obj_id, uint64_t dest_global_device_id, uint64_t * ret_obj_size, void ** ret_obj_contents);

#endif