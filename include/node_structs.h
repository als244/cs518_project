#ifndef NODE_STRUCTS_H
#define NODE_STRUCTS_H

#include "top_level.h"
#include "memory_layer_structs.h"

typedef struct node Node;

typedef enum os_type {
	POSIX = 0,
	WINDOWS = 1,
	OTHER_OS = 2
} OsType;

// TODO
//	- deal with network interfaces (RDMA)
//	- deal with filesystem
typedef struct host {
	uint64_t global_host_id;
	char * ipaddr;
	char * hostname;
	OsType os_type;
	uint64_t max_chunks;
	uint64_t min_granularity;
	Chunks * chunks;
	Node * node;
} Host;

typedef enum device_type {
	CPU = 0,
	NVIDIA_GPU = 1,
	AMD_GPU = 2,
	INTEL_GPU = 3,
	XILINX_FPGA = 4,
	ALTERA_FPGA = 5,
	GROQ_DEVICE = 6,
	SAMBANOVA_DEVICE = 7,
	CEREBRAS_DEVICE = 8,
	OTHER_DEVICE = 9
} DeviceType;

typedef struct device {
	uint64_t global_device_id;
	int local_device_id;
	DeviceType device_type;
	// the main device context we create at init time
	void * backend_context;
	// chunk size u
	uint64_t min_granularity;
	// pointer to struct that holds all device memory
	Chunks * chunks;
	Node * host_node;
} Device;

typedef enum driver_type {
	NONE = 0,
	CUDA_DRIVER = 1,
	HIP_DRIVER = 2,
	ZE_DRIVER = 3,
	OTHER_DRIVER = 4
} DriverType;

typedef struct node {
	uint64_t global_node_id;
	DriverType driver_type;
	Host * host;
	int n_devices;
	Device ** devices;
	// core data structure
	Obj_Table * obj_table;
} Node;


#endif