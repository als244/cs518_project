#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "top_level.h"
#include "system_config.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "master.h"
#include "server_structs.h"

// IMPORT:

// From node.c (but will move to host.c)
Node * init_node(Master * master, Node_Init_Config * node_init_config);
void destroy_node();

// From server.c
Server * init_server(Master * master, Node * node);
int start_server(Server * server);
int shutdown_server(Server * server);


// EXPORT:
//	- for now used in example_client.c
Server * system_init();

#endif