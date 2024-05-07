#ifndef MASTER_H
#define MASTER_H

#include "top_level.h"
#include "system_config.h"
#include "node_structs.h"

// TODO:
//	- THINK ABOUT HOW TO DO THIS IN DISTRIBUTED FASHION!

typedef struct master_init_config Master_Init_Config;

typedef struct master {
	uint64_t n_devices;
	Device ** device_table;
	uint64_t n_nodes;
	Node ** node_table;
} Master;


// FUNCTIONS TO EXPORT

Master * init_master(Master_Init_Config * master_init_config);
void destroy_master(Master * master);

// WITHIN init_node() in node.c each of these will be called!
int master_insert_device(Master * master, Device * device);
int master_insert_node(Master * master, Node * node);

// Needed for handler to know where pull / get!
//	- TODO:
//		- needs to be dsitributed to make sense
Device * get_device_from_global_id(Master * master, uint64_t global_device_id);
Node * get_node_from_global_id(Master * master, uint64_t node_id);

#endif