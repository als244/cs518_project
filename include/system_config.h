#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "top_level.h"
#include "memory_layer_structs.h"
#include "node_structs.h"
#include "deque.h"


// TODO:
//	- Deal with network interfaces!
//	- Deal with filesystem

// EXPORT:

// Structures to help with initialization

// what else is missing from host?
typedef struct host_init_config {
	uint64_t global_host_id;
	// should these be in here? or server?
	char * ipaddr;
	char * hostname;
	OsType os_type;
	uint64_t max_chunks;
	uint64_t min_granularity;
	uint64_t * init_chunk_sizes_list;
} Host_Init_Config;

// IMPORTANT TODO:
//	- support slabs!
//		- should have better way of doing sizes list...
//		- tradeoff between driver overhead for doing small mappings vs. fragmentation/implementation challenges 
typedef struct device_init_config {
	uint64_t global_device_id;
	uint64_t local_device_id;
	DeviceType device_type;
	uint64_t max_chunks;
	uint64_t * init_chunk_sizes_list;
} Device_Init_Config;

typedef struct obj_table_init_config {
	uint64_t min_size;
	uint64_t max_size;
	float load_factor;
	float shrink_factor;
} Obj_Table_Init_Config;

// DEFINING A 1-1 relationship between node <--> host
//	- host also contains other information
//		- os things
//			- TODO: network/filesystem stuff
//	- system memory usage
typedef struct node_init_config {
	uint64_t global_node_id;
	DriverType driver_type;
	Host_Init_Config * host_init_config;
	int n_devices;
	Device_Init_Config ** device_init_configs;
	Obj_Table_Init_Config * obj_table_init_config;
} Node_Init_Config;

// TODO:
//	- work on letting the master be distributed
//	- need server like communication with it!

typedef struct master_init_config {
	uint64_t n_devices;
	uint64_t n_nodes;
} Master_Init_Config;


// Functions 
Device_Init_Config * create_device_init_config(uint64_t global_device_id, uint64_t local_device_id, DeviceType device_type, uint64_t max_chunks, uint64_t * init_chunk_sizes_list);
Host_Init_Config * create_host_init_config(uint64_t global_host_id, char * ipaddr, char * hostname, OsType os_type, uint64_t max_chunks, uint64_t min_granularity, uint64_t * init_chunk_sizes_list);
Obj_Table_Init_Config * create_obj_table_init_config(uint64_t min_size, uint64_t max_size, float load_factor, float shrink_factor);
Node_Init_Config * create_node_init_config(uint64_t global_node_id, DriverType driver_type, Host_Init_Config * host_init_config, int n_devices, Device_Init_Config ** device_init_configs, Obj_Table_Init_Config * obj_table_init_config);
Master_Init_Config * create_master_init_config(uint64_t n_devices, uint64_t n_nodes);

#endif