#ifndef NODE_H
#define NODE_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "system_config.h"
#include "master.h"
#include "host.h"
#include "device.h"
#include "hsa_plugins.h"

// TODO:
//	- modularize node.c / node.h / node_structs.h
//	- instead of node: create device and host


// IMPORT:
// From obj_table.c
Obj_Table * init_obj_table(Obj_Table_Init_Config * obj_table_init_config);

// EXPORT:

// Functions to export
Node * init_node(Master * master, Node_Init_Config * node_init_config);
void destroy_node();

#endif