#ifndef CLIENT_STRUCTS_H
#define CLIENT_STRUCTS_H

#include "top_level.h"


typedef struct device Device;

// Details about client

// TODO:
//	- ip address, port
//	- some sort of authentication
//	- some sort of information about client process

// resuest id should be unique for every request sent by a given client
typedef struct client {
	uint64_t client_id;
	uint64_t request_id;
} Client;



typedef enum operation_type {
	PUSH = 1,
	PULL = 2,
	RESERVE = 3,
	SHUTDOWN = 4
} OperationType;

// Each specific request struct will be passed to appropriate handler

// REQUIRING SPECIFYING DESTINATION DEVICE
//	- assumption is that this decision will be handled smartly be higher layer
typedef struct push_request {
	// size in bytes
	uint64_t obj_size;
	void * obj_contents;
	uint64_t dest_global_device_id;
} Push_Request;

typedef struct pull_request {
	uint64_t obj_id;
	uint64_t dest_global_device_id;
} Pull_Request;

typedef struct reserve_request {
	uint64_t obj_size;
	uint64_t obj_id;
	uint64_t dest_global_device_id;
} Reserve_Request;


// Request struct is received from client lib

// Request is of type Push_Request, Reserve_Request, or Pull_Request and is casted depending on request type

// Only system admin can send (actionably) send shutdown type 
typedef struct request{
	Client * client;
	uint64_t timestamp_sent;
	OperationType request_type;
	void * generic_request;
} Request;



// TODO:
//	- work on being more descriptive with error numbers
//		- for now, just using 0 = success, 1 = error

typedef struct push_response {
	int errno; 
	uint64_t obj_id;
} Push_Response;

typedef struct pull_response {
	int errno;
	uint64_t obj_size;
	void * obj_contents;
} Pull_Response;

typedef struct reserve_response {
	int errno;
	void * empty_obj_contents;
} Reserve_Response;



typedef struct shutdown_message {

}Shutdown_Message;


// Response is of type Push_Response, Reserve_Response, or Pull_Response and is casted depending on response type

// Client lib should read from pointer assoicated with response_type
// System errno occurs if the handler's themselves fail
typedef struct response {
	Client * client;
	uint64_t timestamp_start;
	uint64_t timestamp_complete;
	OperationType response_type;
	void * generic_response;
	int system_errno;
} Response;

#endif