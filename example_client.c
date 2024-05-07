#include "example_client.h"

// A DEMO CLIENT!
//	- Testing if pushting an object and then pullting it returns the same thing
int example_client(Server * server){

	int ret;
	hipError_t result;
	hsa_status_t hsa_status;
	const char * err;

	Node * node = server -> node;

	// hipInit already happened within system_init

	int device_id = 0;
	result = hipSetDevice(device_id);
	if (result != hipSuccess){
		err = hipGetErrorString(result);
    	fprintf(stderr, "Error: Could not set device: %s\n", err);
    	return -1;
	}



	Client * client = init_client(0, 0);

	// Create object
	printf("Creating Object...\n");
	uint64_t n_chunks = 2;
	uint64_t chunk_size = 1 << 21;
	uint64_t n_ints = (n_chunks * chunk_size) / sizeof(uint64_t);
	printf("N ints: %lu\n", n_ints);
	
	uint64_t obj_size = n_ints * sizeof(uint64_t);
	
	uint64_t * obj_contents = (uint64_t *) calloc(n_ints, sizeof(uint64_t));
	// initialize object as just range from 0, n_ints -1
	for (uint64_t i = 0; i < n_ints; i++){
		obj_contents[i] = i;
	}

	// Do push
	uint64_t obj_id;
	uint64_t dest_global_device_id = 0;
	printf("Pushing object with size: %lu\n", obj_size);
	ret = client_push(client, server, obj_size, (void *) obj_contents, dest_global_device_id, &obj_id);

	if (ret != 0){
		fprintf(stderr, "Error in client push\n");
		return ret;
	}

	printf("Object has id: %lu\n", obj_id);

	printf("Synchronizing so writes complete...\n");

	hipDeviceSynchronize();

	printf("Finished sync. Now pullting object with id: %lu\n", obj_id);

	uint64_t ret_obj_size;
	// will be a device pointer, but was set accessible from host (cpu) agent!
	void * ret_obj_contents;
	ret = client_pull(client, server, obj_id, dest_global_device_id, &ret_obj_size, &ret_obj_contents);
	if (ret != 0){
		fprintf(stderr, "Error in client pull\n");
		return ret;
	}

	printf("Retrieved object has size: %lu\n", ret_obj_size);

    printf("Checking if objects are identical...\n");
    // casting 
    uint64_t * h_ret_obj_contents_casted = (uint64_t *) ret_obj_contents;
    for (uint64_t i = 0; i < n_ints; i++){
    	if (obj_contents[i] != h_ret_obj_contents_casted[i]){
    		fprintf(stderr, "Error. Pull object differs from what was push. At index: %lu\n", i);
    		fprintf(stderr, "Orig: %lu vs. Retrieved: %lu\n\n", obj_contents[i], h_ret_obj_contents_casted[i]);
    		printf("\nTest Failed :(\n\tPull object != Push object\n");
    		return -1;
    	}
    }

    printf("\nTest passed!\n\tPull object == Push object\n\n");

    // printf("\n\n sitting in loop \n\n");
    // uint64_t upper_bound = 1 << 31;
    // uint64_t a = 0;
    // for (uint64_t i = 0; i < upper_bound; i++){
    // 	a += 1;
    // }
}


// TODO:
//	-- Actually implemented connecting to server!

// For now just creating the server ourselvers
Server * connect_to_server() {

	// TODO: Should be connecting to existing server here through some socket!
	fprintf(stderr, "Connect to server: Unimplemented Error\n");
	return NULL;
}

// TEMPORARILY HERE FOR TESTING
Server * client_system_init(){

	// For now, just initializing system in the client
	printf("Initializing system...\n\n");
	// parameter should really be configuration file
	Server * server = system_init();

	if (server == NULL){
		fprintf(stderr, "Error in system init. Exiting...\n");
		return NULL;
	}

	return server;

}


int main(int argc, char *argv[]) {

	int ret;

	Server * server = client_system_init();


	if (server == NULL){
		fprintf(stderr, "Error initializing\n");
		return 1;
	}

	ret = example_client(server);

	return 0;
}