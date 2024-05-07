#include "system_config.h"

Device_Init_Config * create_device_init_config(uint64_t global_device_id, uint64_t local_device_id, DeviceType device_type, uint64_t max_chunks, uint64_t * init_chunk_sizes_list) {

	Device_Init_Config * dev_config = (Device_Init_Config *) malloc(sizeof(Device_Init_Config));

	if (dev_config == NULL){
		fprintf(stderr, "Error: malloc failed in create dev config\n");
		return NULL;
	}

	dev_config -> global_device_id = global_device_id;
	dev_config -> local_device_id = local_device_id;
	dev_config -> device_type = device_type;
	dev_config -> max_chunks = max_chunks;
	dev_config -> init_chunk_sizes_list = init_chunk_sizes_list;

	return dev_config;
}

Host_Init_Config * create_host_init_config(uint64_t global_host_id, char * ipaddr, char * hostname, OsType os_type, uint64_t max_chunks, uint64_t min_granularity, uint64_t * init_chunk_sizes_list){

	Host_Init_Config * host_config = (Host_Init_Config *) malloc(sizeof(Host_Init_Config));

	if (host_config == NULL){
		fprintf(stderr, "Error: malloc failed in create host_config\n");
		return NULL;
	}

	host_config -> global_host_id = global_host_id;
	host_config -> ipaddr = ipaddr;
	host_config -> hostname = hostname;
	host_config -> os_type = os_type;
	host_config -> max_chunks = max_chunks;
	host_config -> min_granularity = min_granularity;
	host_config -> init_chunk_sizes_list = init_chunk_sizes_list;
}

Obj_Table_Init_Config * create_obj_table_init_config(uint64_t min_size, uint64_t max_size, float load_factor, float shrink_factor) {

	Obj_Table_Init_Config * tab_config = (Obj_Table_Init_Config *) malloc(sizeof(Obj_Table_Init_Config));

	if (tab_config == NULL){
		fprintf(stderr, "Error: malloc failed in create obj table config\n");
		return NULL;
	}

	tab_config -> min_size = min_size;
	tab_config -> max_size = max_size;
	tab_config -> load_factor = load_factor;
	tab_config -> shrink_factor = shrink_factor;

	return tab_config;
}

Node_Init_Config * create_node_init_config(uint64_t global_node_id, DriverType driver_type, Host_Init_Config * host_init_config, 
											int n_devices, Device_Init_Config ** device_init_configs, Obj_Table_Init_Config * obj_table_init_config) {

	Node_Init_Config * node_config = (Node_Init_Config *) malloc(sizeof(Node_Init_Config));

	if (node_config == NULL){
		fprintf(stderr, "Error: malloc failed in create node config\n");
		return NULL;
	}

	node_config -> global_node_id = global_node_id;
	node_config -> driver_type = driver_type;
	node_config -> host_init_config = host_init_config;
	node_config -> n_devices = n_devices;
	node_config -> device_init_configs = device_init_configs;
	node_config -> obj_table_init_config = obj_table_init_config;

	return node_config;
}

Master_Init_Config * create_master_init_config(uint64_t n_devices, uint64_t n_nodes) {

	Master_Init_Config * master_config = (Master_Init_Config *) malloc(sizeof(Master_Init_Config));

	if (master_config == NULL){
		fprintf(stderr, "Error: malloc failed in create master config\n");
		return NULL;
	}

	master_config -> n_devices = n_devices;
	master_config -> n_nodes = n_nodes;

	return master_config;

}