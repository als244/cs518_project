#include "request_handlers.h"

// HANDLERS!


// Should push object somewhere in memory
// Return the object_id within push response
Push_Response * push_handler(Server * server, Push_Request * push_request){

	Master * master = server -> master;
	
	uint64_t obj_size = push_request -> obj_size;
	void * obj_contents = push_request -> obj_contents;

	uint64_t dest_global_device_id = push_request -> dest_global_device_id;
	Device * dest_device = get_device_from_global_id(master, dest_global_device_id);

	if (dest_device == NULL){
		fprintf(stderr, "Error: could not retrieve device from global id\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	// ACTUALLY CALL PUSH
	// 	- maybe reserve obj id = 0 for NULL object?
	uint64_t ret_obj_id = 0;
	int errno = push(obj_size, obj_contents, dest_device, server -> node, &ret_obj_id);


	if (errno != 0){
		errno = 1;
		fprintf(stderr, "Error in pushting object\n");
	}

	Push_Response * push_response = (Push_Response *) malloc(sizeof(Push_Response));

	if (push_response == NULL){
		fprintf(stderr, "Error: malloc failed in push handler\n");
		return NULL;
	}

	push_response -> errno = errno;
	push_response -> obj_id = ret_obj_id;

	return push_response;
}


// Should locate the object and determine which chunks hold it
// After pull() call in object.c we have
//	- obj size
//	- # handles 
//	- array of sharable handles

// Now we should create
Pull_Response * pull_handler(Server * server, Pull_Request * pull_request){

	Node * node = server -> node;
	Obj_Table * obj_table = server -> node -> obj_table;
	Master * master = server -> master;


	uint64_t obj_id = pull_request -> obj_id;
	uint64_t dest_global_device_id = pull_request -> dest_global_device_id;
	Device * dest_device = get_device_from_global_id(master, dest_global_device_id);

	if (dest_device == NULL){
		fprintf(stderr, "Error: could not retrieve device from global id\n");
		// TODO:
		//	- cleanup
		return NULL;
	}

	// ACTUALLY CALL PULL
	uint64_t ret_obj_size;
	void * ret_obj_contents;
	int errno = pull(obj_id, dest_device, server -> node, &ret_obj_size, &ret_obj_contents);

	// Return response

	// Convert obj to pull_response format
	Pull_Response * pull_response = (Pull_Response *) malloc(sizeof(Pull_Response));

	if (errno != 0){
		errno = 1;
		fprintf(stderr, "Error in pushting object\n");
	}

	if (pull_response == NULL){
		fprintf(stderr, "Error: malloc failed in pull handler\n");
		return NULL;
	}

	// populate the response
	pull_response -> errno = errno;
	pull_response -> obj_size = ret_obj_size;
	pull_response -> obj_contents = ret_obj_contents;

	return pull_response;
}


// Should reserve the object from table and release it's memory
// Return if successful within reserve response
Reserve_Response * reserve_handler(Server * server, Reserve_Request * reserve_request){

	fprintf(stderr, "Pull: Unimplemented Error\n");
	return NULL;
}



Response * handle_request(Server * server, Request * request){

	printf("\nIn Request Handler...\n");

	// Now process request

	// mark start time of handling request
	struct timespec start_time;
	clock_gettime(CLOCK_REALTIME, &start_time);
	uint64_t timestamp_start = start_time.tv_sec * 1e9 + start_time.tv_nsec;

	// 1. TODO: authenticate request
	// 2. Pass to handler

	// 1. TODO: authenticate and verify integreity of request
	Client * client = request -> client;
	uint64_t client_id = client -> client_id;
	uint64_t request_id = client -> request_id;

	// 2. Passing to Handler
	OperationType req_type = request -> request_type;


	// build partial-response struct
	Response * response = (Response *) malloc(sizeof(Response));
	
	response -> client = client;
	response -> timestamp_start = timestamp_start;
	response -> response_type = req_type;

	
	void * generic_response = NULL;

	int system_errno = 0;
	bool is_shutdown = false;
	char * req_type_str;
	int req_err;
	switch(req_type){
		case PUSH:
			req_type_str = "PUSH";
			Push_Response * push_response = push_handler(server, (Push_Request *) request -> generic_request);
			req_err = push_response -> errno;
			generic_response = (void *) push_response;
			break;
		case PULL:
			req_type_str = "PULL";
			Pull_Response * pull_response = pull_handler(server, (Pull_Request *) request -> generic_request);
			req_err = pull_response -> errno;
			generic_response = (void *) pull_response;
			break;
		case RESERVE:
			req_type_str = "RESERVE";
			Reserve_Response * reserve_response = reserve_handler(server, (Reserve_Request *) request -> generic_request);
			req_err = reserve_response -> errno;
			generic_response = (void *) reserve_response;
			break;
		case SHUTDOWN:
			is_shutdown = true;
			req_type_str = "SHUTDOWN";
			// TODO:
			//	- implement shutdown functionality
			// Shutdown_Response * shutdown_response = shutdown_handler(server, (Shutdown_Request *) request -> generic_request);
			// req_eror = shutdown_response -> errno;
			// generic_response = (void *) shutdown_response;
			break;

		default:
			fprintf(stderr, "Request Type: %d not supported\n", req_type);
			system_errno = 3;
			break;
	}

	

	struct timespec complete_time;
	clock_gettime(CLOCK_REALTIME, &complete_time);
	uint64_t timestamp_complete = complete_time.tv_sec * 1e9 + complete_time.tv_nsec;


	response -> timestamp_complete = timestamp_complete;
	response -> generic_response = generic_response;
	


	uint64_t elapsed_time_ns = timestamp_complete - timestamp_start;


	char * req_status;

	// IMPORTANT TODO:
	//	- handle errors better. Need to have enum and error numbers everywhere!

	// if one of the underlying functions failed
	if (req_err != 0){
		system_errno = 1;
		req_status = "FAILED (function)";
	}	
	// if not a shutdown and one of the handlers themselves (not underlying function) failed
	else if (!is_shutdown && generic_response == NULL){
		system_errno = 2;
		req_status = "FAILED (system)";
	}
	else if (system_errno != 0){
		req_status = "UNSUPPORTED";
	}
	else {
		req_status = "COMPLETED";
	}
	

	printf("\nRequest %s:\n\tClient ID = %lu\n\tRequest ID = %lu\n\tType = %s\n\tStart: %lu\n\tCompleted: %lu\n\tElapsed Time (System): %lu\n\n", 
				req_status,
				client_id,
				request_id,
				req_type_str,
				timestamp_start,
				timestamp_complete,
				elapsed_time_ns);

	response -> system_errno = system_errno;

	return response;
}
