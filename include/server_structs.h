#ifndef SERVER_STRUCTS_H
#define SERVER_STRUCTS_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "client_structs.h"
#include "node_structs.h"
#include "master.h"

// TODO
//	- see what else is needed for these structs

typedef struct connection {
	Client * client;
	uint64_t connection_id;
} Connection;

// Aggregating all the structures needed by the main server
typedef struct server {
	Master * master;
	Node * node;
	uint64_t server_id;
	char * hostname;
	char * ipaddr;
	int num_connections;
	Connection * connections;
} Server;

#endif
