#include "node.h"

#define TO_PRINT 1

int init_driver(DriverType driver_type){

	int err;
	// not sure what flags might be needed here...
	unsigned int init_flags = 0;
	switch (driver_type){
		case HIP_DRIVER:
			err = hip_init_driver(init_flags);
			break;

		default:
			fprintf(stderr, "Error: driver type is not supported yet\n");
			err = -1;
			break;
	}

	return err;
}

// THE MAIN INIT FUNCTION THAT IS EXPORTED
//	- will init everything else!
//		- returns node struct that has all core data structures

// TODO:
//	- CONSIDER HOST AS A DEVICE

// Responsible for:
//	1. creating node container
//	2. Initializing hoost
//	3. Initializing driver 
//	4. Intiializing devices
//	5. Initializing object table

Node * init_node(Master * master, Node_Init_Config * node_init_config) {

	int err;

	if (TO_PRINT){
		printf("Initializing node with id: %ld\n", node_init_config -> global_node_id);
	}
	// 1. create container
	Node * node = (Node *) malloc(sizeof(Node));

	if (node == NULL){
		fprintf(stderr, "Error: malloc failed in init_node\n");
		return NULL;
	}

	node -> global_node_id = node_init_config -> global_node_id;
		
	Host * host = init_host(node, node_init_config -> host_init_config);
	if (host == NULL){
		fprintf(stderr, "Error: could not initialize host\n");
		// TODO:
		//	- cleanup
		return NULL;
	}
	node -> host = host;

	// 3. Initialize cuda driver
	if (TO_PRINT){
		printf("Initializing driver\n");
	}

	DriverType driver_type = node_init_config -> driver_type;
	node -> driver_type = driver_type;

	err = init_driver(driver_type);
	if (err != 0){
		fprintf(stderr, "Error initializing driver\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	// 4. create devices
	
	if (TO_PRINT){
		printf("Initializing devices\n");
	}
	int n_devices = node_init_config -> n_devices;
	node -> n_devices = n_devices;

	Device ** devices = (Device **) malloc(n_devices * sizeof(Device *));
	if (devices == NULL){
		fprintf(stderr, "Error: malloc failed in init_node\n");
		return NULL;
	}
	Device * device;
	Device_Init_Config ** device_init_configs = node_init_config -> device_init_configs;
	for (int i = 0; i < n_devices; i++){
		device = init_device(node, device_init_configs[i]);
		if (device == NULL){
			fprintf(stderr, "Error initializing device #%d\n", i);
			// TODO:
			//	- cleanup
			return NULL;
		}
		devices[i] = device;
		err = master_insert_device(master, device);
		if (err != 0){
			fprintf(stderr, "Error inserting device to master\n");
			// TODO:
			//	- cleanup
			return NULL;
		}
	}
	node -> devices = devices;

	// 5. create object table
	if (TO_PRINT){
		printf("Intializing object table\n");
	}

	Obj_Table_Init_Config * obj_table_init_config = node_init_config -> obj_table_init_config;	
	Obj_Table * obj_table = init_obj_table(obj_table_init_config);
	if (obj_table == NULL){
		fprintf(stderr, "Error initializing object table\n");
		// TODO:
			//	- cleanup
		return NULL;
	}
	node -> obj_table = obj_table;

	err = master_insert_node(master, node);
	if (err != 0){
		fprintf(stderr, "Error inserting device to master\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	return node;
}


void destroy_node(Node * node) {
	fprintf(stderr, "Destroy Node: Unimplemented Error\n");

}

