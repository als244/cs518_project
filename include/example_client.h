#ifndef EXAMPLE_CLIENT_H
#define EXAMPLE_CLIENT_H

#include "client.h"

// INCLUDING VENDOR BACKEND FOR NOW
// 	- shouldn't be here
//	- used in example client for:
//		- init context
//		- D to H memory transfer after pull()
#include "hsa_backend.h"


// IMPORT FUNCTIONS
//	- from client.c

Client * init_client(uint64_t client_id, uint64_t request_id);

Push_Request * init_push_request(uint64_t obj_size, void * obj_contents, uint64_t dest_global_device_id);
Pull_Request * init_pull_request(uint64_t obj_id, uint64_t dest_global_device_id);
Reserve_Request * init_reserve_request(uint64_t obj_id, uint64_t dest_global_device_id);
Request * init_request(Client * client, OperationType request_type, void * generic_request);

Response * send_request(Server * server, Request * request);

int client_push(Client * client, Server * server, uint64_t obj_size, void * obj_contents, uint64_t dest_global_device_id, uint64_t * ret_obj_id);
int client_pull(Client * client, Server * server, uint64_t obj_id, uint64_t dest_global_device_id, uint64_t * ret_obj_size, void ** ret_obj_contents);

#endif