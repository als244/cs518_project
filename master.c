#include "master.h"


Master * init_master(Master_Init_Config * master_init_config){

	Master * master = (Master *) malloc(sizeof(Master));

	if (master == NULL) {
		fprintf(stderr, "Error: malloc failed in init master\n");
		return NULL;
	}

	uint64_t n_devices = master_init_config -> n_devices;
	uint64_t n_nodes = master_init_config -> n_nodes;

	// Create device table
	master -> n_devices = n_devices;
	Device ** device_table = (Device **) malloc(n_devices * sizeof(Device *));
	
	if (device_table == NULL){
		fprintf(stderr, "Error: malloc failed in init master\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	master -> device_table = device_table;

	// Create node table

	for (uint64_t i = 0; i < n_devices; i++){
		device_table[i] = NULL;
	}
	Node ** node_table = (Node **) malloc(n_nodes * sizeof(Node *));
	
	if (node_table == NULL){
		fprintf(stderr, "Error: malloc failed in init master\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	for (uint64_t i = 0; i < n_nodes; i++){
		node_table[i] = NULL;
	}

	
	master -> node_table = node_table;

	return master;
}

void destroy_master(Master * master) {
	fprintf(stderr, "Destroy Master: Unimplemented Error\n");
}


int master_insert_device(Master * master, Device * device) {

	uint64_t global_device_id = device -> global_device_id;

	Device ** device_table = master -> device_table;

	if (device_table[global_device_id] != NULL){
		fprintf(stderr, "Error inserting device to master. Device already exists\n");
		return -1;
	}

	device_table[global_device_id] = device;

	master -> device_table = device_table;

	return 0;
}

int master_insert_node(Master * master, Node * node) {

	uint64_t global_node_id = node -> global_node_id;

	Node ** node_table = master -> node_table;

	if (node_table[global_node_id] != NULL){
		fprintf(stderr, "Error inserting node to master. Node already exists\n");
		return -1;
	}

	node_table[global_node_id] = node;

	master -> node_table = node_table;

	return 0;

}

Device * get_device_from_global_id(Master * master, uint64_t global_device_id) {
	Device ** device_table = master -> device_table;

	if (global_device_id >= master -> n_devices) {
		fprintf(stderr, "Error: global_device_id is larger than number of devices");
		return NULL;
	}

	return device_table[global_device_id];
}


Node * get_node_from_global_id(Master * master, uint64_t global_node_id) {

	Node ** node_table = master -> node_table;

	if (global_node_id >= master -> n_nodes) {
		fprintf(stderr, "Error: node_id is larger than number of nodes");
		return NULL;
	}

	return node_table[global_node_id];
}