#ifndef SERVER_H
#define SERVER_H

#include "top_level.h"
#include "node_structs.h"
#include "server_structs.h"
#include "master.h"

// IMPORT:

// From request_handlers.c
Response * handle_request(Server * server, Request * request);

// EXPORT:
//	- used in system_init.c

Server * init_server(Master * master, Node * node);
void destroy_server(Server * server);

int start_server(Server * server);
int shutdown_server(Server * server);

#endif

