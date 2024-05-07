#include "system_init.h"


// SHOULD TAKE CONFIG FILE AS INPUT
Server * system_init(){

	// TODO:
	//	- Load in configuration file
	//	- Should replace this section!

	// 1. Get system config and intialize:
	//		a. Master
	//			- UGLY: shouldn't happen here
	//				- just for testing
	//			- this function feels like a per-node initialization, not global...
	//			- this should be a global "head" node that stores information
	//		b. Host
	//		c. Devices
	//		d. Obj Table
	//		e. Node

	

	// a. Master

	uint64_t n_global_devices = 1;
	uint64_t n_global_nodes = 1;
	Master_Init_Config * master_config = create_master_init_config(n_global_devices, n_global_nodes);
	if (master_config == NULL){
		fprintf(stderr, "Error creating master config\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	Master * master = init_master(master_config);

	if (master == NULL){
		fprintf(stderr, "Error initializing master\n");
		// TODO:
		//	- cleaup
		return NULL;
	}

	// b. Host

	// hardcoding this for now
	uint64_t global_host_id = 0;
	char * ipaddr = "127.0.0.1:8080";
	char * hostname = "tubingen";
	OsType os_type = POSIX;
	// reading/writing from shared host memory not ready yet
	uint64_t host_max_chunks = 0;
	uint64_t host_min_granularity = sysconf(_SC_PAGESIZE);
	uint64_t * host_init_chunk_sizes_list = NULL;

	Host_Init_Config * host_config = create_host_init_config(global_host_id, ipaddr, hostname, os_type, host_max_chunks, 
																host_min_granularity, host_init_chunk_sizes_list);
	if (host_config == NULL){
		fprintf(stderr, "Error creating host config\n");
		// TODO:
		//	- cleanup
		return NULL;
	}


	//	c. Devices

	// hardcoding this stuff for testing purposes
	int n_devices = 1;
	Device_Init_Config ** dev_configs = (Device_Init_Config **) malloc(n_devices * sizeof(Device_Init_Config *));
	if (dev_configs == NULL){
		fprintf(stderr, "Error: malloc failed in system init\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	// SETTING DIRECTLY FOR ONLY DEVICE
	//	- (again should be read in from config file)
	uint64_t global_device_id = 0;
	uint64_t local_device_id = 0;
	DeviceType device_type = AMD_GPU;
	// hardcoding total memory usage for this
	// 5 gb
	uint64_t total_memory = 5 * 1e9;
	uint64_t min_granularity = 1 << 21;
	uint64_t max_chunks = total_memory / min_granularity;

	uint64_t * init_chunk_sizes_list = (uint64_t *) malloc(max_chunks * sizeof(uint64_t));

	// FOR NOW SET EVERY CHUNK TO BE MIN GRANULARITY
	for (uint64_t phys_chunk_id = 0; phys_chunk_id < max_chunks; phys_chunk_id++){
		init_chunk_sizes_list[phys_chunk_id] = min_granularity;
	}
	
	printf("Creating %ld chunks of size %ld\n", max_chunks, min_granularity);

	Device_Init_Config * dev_config = create_device_init_config(global_device_id, local_device_id, device_type, max_chunks, init_chunk_sizes_list);
	if (dev_config == NULL){
		fprintf(stderr, "Error creating device config\n");
		// TODO:
		//	- cleanup
		return NULL;
	}
	dev_configs[0] = dev_config;

	// d. Obj Table

	uint64_t min_size = 256;
	uint64_t max_size = 65536;
	float load_factor = .5;
	float shrink_factor = .1;
	Obj_Table_Init_Config * tab_config = create_obj_table_init_config(min_size, max_size, load_factor, shrink_factor);
	if (tab_config == NULL){
		fprintf(stderr, "Error creating tab config\n");
		// TODO:
		//	- cleanup
		return NULL;
	}


	// e. Node

	uint64_t node_id = 0;
	DriverType driver_type = HIP_DRIVER;
	Node_Init_Config * node_config = create_node_init_config(node_id, driver_type, host_config, n_devices, dev_configs, tab_config);
	if (node_config == NULL){
		fprintf(stderr, "Error creating node config\n");
		// TODO:
		//	- cleanup
		return NULL;
	}


	// 2. Initialize node

	// Initialization steps:
	//	- Initializing Devices
	//		- this involves initializing contexts
	//	- Ingesting device memory into system
	//		- this ivolves:
	//			- determining number of chunks on device
	//			- initializing chunks container
	//			- intiializing chunk and inserting into chunks
	//
	//	- Intializing Object Table

	Node * node = init_node(master, node_config);

	if (node == NULL){
		fprintf(stderr, "Error initializing node\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	// 3. Initiailze server(s)

	// TODO:
	//	- Create realistic server(s) and accept connections

	Server * server = init_server(master, node);

	if (server == NULL){
		fprintf(stderr, "Error intializing server\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	return server;
}


// FOR NOW ONLY HAVING CLIENT RUN MAIN FUNCTION
int system_init_main(int argc, char *argv[]) {

	int ret;

	printf("Initializing system...\n\n");

	// CORE SYSTEM DATA STRUCTURES

	// parameter should really be configuration file
	Server * server = system_init();

	if (server == NULL){
		fprintf(stderr, "Error in system init. Exiting...\n");
		exit(1);
	}

	printf("Starting system server...\n");

	// should only return on shutdown request
	ret = start_server(server);


	if (ret != 0){
		fprintf(stderr, "Error in server. Exiting...\n");
	}

	ret = shutdown_server(server);

	if (ret != 0){
		fprintf(stderr, "Error in shutting dow server. Exiting...\n");
	}

	return 0;
}




// Structures that need system intialization:
	
	// 1. Per-Device Cuda Contexts
	// 2. Chunks + exporting Cuda Allocation handles
	// 3. Obj Table


// System server handles:
	
	// 1. Put
	// 		- Requests from clients to allocate memory for object
	// 		- Client has no responsibility after server call

	// 2. Remove
	// 		- Requests from clients to free object from memory
	//		- Client has no responsibility after server call

	// 3. Get (partially)
	// 		- Requests from clients to retrieve objects
	//		- System will return:
	//			- If exists:
	//				- Object size in bytes
	//				- An allocated, ordered-list of sharable handles referencing physical memory containing object
	//			- Otherwise:
	//				- Error


// Assumed handling in client lib

	// 1. Get functionality in system's client library (not application). 
	// 		- Assume client application "knows" the object-id 
	//			- Can perform dedup hash on their side
	// 		- Pseudocode:
	// 			- Input: 
	//				- Object id
	//				- Destination cuda_device id, or -1 for CPU
	//					- TODO:
	//						- When we go distributed this will be a global id, not cuda id
	//						- Allow user to specify if pinned memory (if they choose a CPU as dest)
	// 				- pointer to unintialized device pointer
	//				- pointer to uint64_t size (IPC)
	// 				- pointer to unallocated void ** shared handle array (IPC) 
	//			- Output:
	//				- Fill in device pointer
	//					- This is a virtual address pointing to a contiguous space that is 
	// 					  mapped to physical memory chunks containing object
	//					- NULL if object does not exist or dest OOM
	//						- Return error code specifying either issue
	//				- Fill in size
	//					- Object size in bytes
	// 						- So client can reserve virtual address space and do mapping
	//				- Fill in ordered shared-handle array
	//					- Each entry in array contains a shared handle to a physical allocation
	//					- Needed to do memory mapping!
	//					- Each handle can be imported and used in mapping
	// 			
	// 			1. Send get(object_id) request to server lib
	//				- Retrieves:
	//					- Object size
	//					- Ordered-list of sharable phys mem handles
	// 				- If error (object doesn't exist):
	//					- Return and pass error up to application
	// 			2. Ensure context for specified device is side
	//				- Otherwise create context and push to CPU-thread
	//			3. Reserve virtual memory address range based on object size -> returns unmapped device ptr
	//			4. For each handle in sharable handle array:
	// 				- Import allocation from handle
	//				- Read size of allocation
	//				- Perform memMap to map physical memory from handle into reserved virt add space
	//				- Update:
	//					- Virtual address pointer where next memMap should map handle to
	//					- Remaining size to be mapped
	//						- Assume all handles are full (conents take up entire allocation size)
	// 						  except the last handle which has int. fragmentation => 
	//							- Need remaining size for mapping the last handle in list correctly
	//
