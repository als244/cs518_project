#include "client.h"


Client * init_client(uint64_t client_id, uint64_t request_id){

	Client * client = (Client *) malloc(sizeof(Client));

	if (client == NULL){
		fprintf(stderr, "Error: malloc failed in init_client\n");
		return NULL;
	}

	client -> client_id = client_id;
	client -> request_id = request_id;

	return client;

}


Push_Request * init_push_request(uint64_t obj_size, void * obj_contents, uint64_t dest_global_device_id){

	Push_Request * push_request = (Push_Request *) malloc(sizeof(Push_Request));

	if (push_request == NULL){
		fprintf(stderr, "Error: malloc failed in init_push_request\n");
		return NULL;
	}

	push_request -> obj_size = obj_size;
	push_request -> obj_contents = obj_contents;
	push_request -> dest_global_device_id = dest_global_device_id;

	return push_request;

}

Pull_Request * init_pull_request(uint64_t obj_id, uint64_t dest_global_device_id) {

	Pull_Request * pull_request = (Pull_Request *) malloc(sizeof(Pull_Request));

	if (pull_request == NULL){
		fprintf(stderr, "Error: malloc failed in init_pull_request\n");
		return NULL;
	}

	pull_request -> obj_id = obj_id;
	pull_request -> dest_global_device_id = dest_global_device_id;

	return pull_request;

}

Reserve_Request * init_reserve_request(uint64_t obj_id, uint64_t dest_global_device_id) {
	
	Reserve_Request * reserve_request = (Reserve_Request *) malloc(sizeof(Reserve_Request));

	if (reserve_request == NULL){
		fprintf(stderr, "Error: malloc failed in init_reserve_request\n");
		return NULL;
	}

	reserve_request -> obj_id = obj_id;
	reserve_request -> dest_global_device_id = dest_global_device_id;

	return reserve_request;

}

Request * init_request(Client * client, OperationType request_type, void * generic_request) {

	Request * request = (Request *) malloc(sizeof(Request));

	if (request == NULL){
		fprintf(stderr, "Error: malloc failed in init_request\n");
		return NULL;
	}

	request -> client = client;
	request -> request_type = request_type;
	request -> generic_request = generic_request;

	return request;

}

Response * send_request(Server * server, Request * request){

	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	uint64_t timestamp_sent = timestamp.tv_sec * 1e9 + timestamp.tv_nsec;

	request -> timestamp_sent = timestamp_sent;

	// SHOULD ACTUALLY BE SENDING OVER SOME COMMUNICATION CHANNEL IN SEPERATE PROCESS

	// FOR NOW DIRECTLY CALLING HANDLE_REQUEST
	Node * node;
	Response * response = handle_request(server, request);

	return response;
}


// Steps for Client Methods:
//	1. Create typed request
//	2. Create general request
//	3. Send request
//	4. Handle response

// Each returns error code
// Push:
//	- populates obj_id argument
// Pull:
//	- populated obj_size & obj_contents arguments

// TODO:
//	- Asynchronous response return for all client send_requests()!



// TODO:
//	- Incorporate device parameter to "retrieve" to certain device

// FOR NOW:
//	- Assume obj_contents is a pointer to host memory
int client_push(Client * client, Server * server, uint64_t obj_size, void * obj_contents, uint64_t dest_global_device_id, uint64_t * ret_obj_id) {

	Push_Request * push_request = init_push_request(obj_size, obj_contents, dest_global_device_id);

	if (push_request == NULL){
		fprintf(stderr, "Error in creating push request\n");
		return -1;
	}

	Request * request = init_request(client, PUSH, (void *) push_request);

	if (request == NULL){
		fprintf(stderr, "Error in creating generic request\n");
		return -1;
	}

	Response * response = send_request(server, request);

	// MARK WHEN WE RECEIVED RESPONSE
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	uint64_t timestamp_recv = timestamp.tv_sec * 1e9 + timestamp.tv_nsec;

	// RETRIEVE RESPONSE TIMESTAMP METADATA
	uint64_t timestamp_start = response -> timestamp_start;
	uint64_t timestamp_complete = response -> timestamp_complete;
	
	// HANDLE PUSH RESPONSE
	Push_Response * push_response = (Push_Response *) (response -> generic_response);

	// Check for errors
	int errno = push_response -> errno;

	if (errno != 0){
		fprintf(stderr, "Error in system's push handler. Errno: %d\n", errno);
		return -1;
	}

	int system_errno = response -> system_errno;

	if (system_errno != 0){
		fprintf(stderr, "Error in system's handler. Errno: %d\n", system_errno);
		return -1;
	}

	// SET THE OBJ ID
	*ret_obj_id = push_response -> obj_id;

	return 0;
}

// TODO:
//	- Modularize better (seperate files?)
//		- seperate phys mem retrieval and returned object destination
//			- retrieving from vendor a's device and expecting on vendor b's device
// FOR NOW:
//	- ASSUMING BOTH OBJECT IS STORED ON CUDA DEVCICE AND WE ARE RETURNING TO CUDA (or CPU) DEVICE


// Return object size + pointer to object contents on destination device
int client_pull(Client * client, Server * server, uint64_t obj_id, uint64_t dest_global_device_id, uint64_t * ret_obj_size, void ** ret_obj_contents) {

	Pull_Request * pull_request = init_pull_request(obj_id, dest_global_device_id);

	if (pull_request == NULL){
		fprintf(stderr, "Error in creating pull request\n");
		return -1;
	}

	Request * request = init_request(client, PULL, (void *) pull_request);

	if (request == NULL){
		fprintf(stderr, "Error in creating generic request\n");
		return -1;
	}

	Response * response = send_request(server, request);

	// MARK WHEN WE RECEIVED RESPONSE
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME, &timestamp);
	uint64_t timestamp_recv = timestamp.tv_sec * 1e9 + timestamp.tv_nsec;

	// RETRIEVE RESPONSE TIMESTAMP METADATA
	uint64_t timestamp_start = response -> timestamp_start;
	uint64_t timestamp_complete = response -> timestamp_complete;
	
	// HANDLE PULL RESPONSE
	Pull_Response * pull_response = (Pull_Response *) (response -> generic_response);

	// Check for errors
	int errno = pull_response -> errno;

	if (errno != 0){
		fprintf(stderr, "Error in system's push handler. Errno: %d\n", errno);
		return -1;
	}

	int system_errno = response -> system_errno;

	if (system_errno != 0){
		fprintf(stderr, "Error in system's handler. Errno: %d\n", system_errno);
		return -1;
	}

	uint64_t obj_size = pull_response -> obj_size;
	void * obj_contents = pull_response -> obj_contents;

	*ret_obj_size = obj_size;
	// SET THE OBJ CONTENTS RETURN POINTER
	//	 - on destination device
	*ret_obj_contents = obj_contents;

	return 0;
}

