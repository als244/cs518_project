#include "server.h"

// Will be responsible for handling:
//	- Put requests from client lib
//	- Remove requests from client lib
//	- Sending obj size and shared phys mem handles to client lib's get request
//	- Interpreting a shutdown message and returning


// Always running, only shuts down upon shutdown message
//	- start_server spawns threads to handle each request
//		- TODO: Functionality
//			- Handle client requests with some socket-like mechanism
//		- TODO: Correctness
//			- Handle concurrent requests
//				- Currently no locks in memory allocator!
//					- Do want objects thinking they own chunk when in reality it was assigned to mulitple
//						- Need atomic free lists
//					- Do want objects to get() object when it is only partially written
//						- Need sync signal for when writes complete
//		- TODO: Optimization
//			- Utilize thread pool to remove spawn overhead
//			- Deal with async requests
//			- Seperate "streams" per permissions space to enable more lockless concurrency
//				- When it is guaranteed clients' requests will not conflict


Server * init_server(Master * master, Node * node){

	Server * server = (Server *) malloc(sizeof(Server));

	server -> master = master;
	server -> node = node;

	// TODO:
	//	- ACTUALLY IMPLEMENT SERVER STUFF
	//		- connections
	//		- ip address
	//		- hostname
	//		- thread pool
	//		- etc.

	return server;
}

void destroy_server(Server * server) {

	fprintf(stderr, "Shutdown Server: Unimplemented Error\n");
}


int start_server(Server * server){

	// TODO: Handle incoming client requests with socket/rpc mechanism
	//		- i.e. Request * request = read from client
	Request * request;


	// received request from client
	// process in request handler
	Response * response = handle_request(server, request);

	return -1;
	

}


// Cleaing up
// Erasing all memory contents and releasing back to devices
// Freeing all system meta data
int shutdown_server(Server * server){

	fprintf(stderr, "Shutdown Server: Unimplemented Error\n");
	return -1;
}
