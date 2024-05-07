// INCLUDE GUARD BECAUSE ALL FILES IN MEMORY LAYER WANT THESE
#ifndef MEMORY_LAYER_STRUCTS_H
#define MEMORY_LAYER_STRUCTS_H

#include "top_level.h"

// DEFINING INTERFACE FUNCTIONS FOR BUILD PROCESS

// self referencing within struct def, so putting typedef here for readability
typedef struct deque_item Deque_Item;

typedef struct device Device;

struct deque_item {
	uint64_t id;
	Deque_Item * prev;
	Deque_Item * next;
};

typedef struct deque {
	uint64_t cnt;
	Deque_Item * head;
	Deque_Item * tail;
} Deque;


typedef enum location {
	DEVICE = 0,
	HOST = 1,
	SSD = 2,
	DISK = 3,
	REMOTE_NODE = 4,
	CLOUD = 5,
	OTHER_LOCATION = 6
} Location;

//	- Entirely initialized at system_init()
//		- Metadata for a chunk of physical memory
//			- Where "chunk" be located on any device in system
//		-  Sharable Phys Mem handle actually contains reference to phys chunk
// 			- For CUDA backend:
//				- Initially created from cuMemExportSharableHandle() in system_init()
//				- Handle was created from cuMemCreate(), with props:
//					- CUmemAllocationType = CU_MEM_ALLOCATION_TYPE_PINNED = 3
//					- CUmemLocation:
//						- id = device_id where chunk resides
//						- CUmemLocationType = CU_MEM_LOCATION_TYPE_DEVICE = 1
//			- POSIX File Descriptor
//			- Can only be read from node that holds the chunk's assoicated device
//			- For CUDA backend:
//				- Use cuMemImportFromShareableHandle() to retrieve allocation
//	- Not written to after system_init()
//	- Read from client's get()
// 		- Retrieves sharable handle in order to import allocation
//	- Destroyed at system_shutdown()
typedef struct phys_chunk {
	Location location;
	// (for now: either of type Device or type Host)
	void * home;
	// globally (across devices/nodes) unique
	uint64_t id;
	// Size in bytes
	// 	- For now, set to the minimum allocation granularity
	//		- Around 2MB (on my 3090)
	//			- Precicely 2,097,152 bytes = 2^21
	// UNUSED so far
	// 	- Could allow flexible-sized chunks
	// 		- Larger chunks => less system overhead
	uint64_t size;
	uint64_t num_min_chunks;
	// BACKEND + OS SPECIFIC
	// for linux + cuda:
		// this value is a POSIX FD
		// created from cuMemExportToShareableHandle at system_init() time
		// can be retrieved through cuMemImportFromSharableHandle
	void * shareable_phys_mem_handle;
	// UNUSED FOR NOW
	// how many processes/contexts have this phys_chunk mapped into their space
	// AKA how many time a pull() call has mapped this phys chunk
	// TODO:
	//	- deal with deletion?
	//	- need the scheduling/process/context layer for that!
	uint64_t ref_cnt;
	pthread_mutex_t ref_cnt_lock;
	// UNUSED SO FAR
	uint64_t access_flags;

} Phys_Chunk;


//	- Initialized at system_init()
//		- Aggregates all of the chunks in the system
//		- Free chunks initially entire system
//	- Modified upon put() operation
//		- IDs are removed from free chunks list + free cnt decreases
//	- Modified upon remove() operation
//		- IDs are added to free chunks list + free cnt increases
//	- Read from client's get()
// 		- Retrieve chunk from phys_chunks array 
//	- Destroyed at system_shutdown()
typedef struct chunks {
	Location location;
	// (for now: either of type Device or type Host)
	void * home;
	// if every chunk was of minimum granularity
	uint64_t max_chunks;
	uint64_t min_granularity;
	// total number of chunks (non_null) in phys_chunks/chunk_sizes
	uint64_t total_cnt;
	// number of free minimum sized chunks
	//	- max chunks - free_min_sized_chunks sets chunk_id upon insert
	uint64_t free_min_sized_chunks;
	pthread_mutex_t cnt_lock;
	// LinkedList of free chunk id's (indicies into Chunk * chunks array)
	// probably could be more efficient (bitmasks?) and suitable for parallelism
	// order matters here because of defragmentation and wrap around
	Deque * free_chunk_ids;
	pthread_mutex_t free_list_lock;
	// contains the handles to physical device memory
	Phys_Chunk ** phys_chunks;
	uint64_t * chunk_sizes;
} Chunks;


//	- Initialized upon first instance of put() operation for the exact contents
//		- Obj id created based on dedup schemes
//		- Obj chunks is an ordered-list of Chunk ID's that hold physical contents
//	- Created in put()
//	- Read from client's get()
//		- Retrieves size for va range allocation
// 		- Retrieves list of chunk id's that the client will map into its space
//	- Destroyed in remove()
typedef struct obj {
	Location location;
	// (for now: either of type Device or type Host)
	void * home;
	// dedpued obj id
	uint64_t obj_id;
	// size in bytes
	uint64_t obj_size;
	// numer of chunks it takes up
	uint64_t obj_chunk_cnt;
	// Ordered list of chunk_ids which hold the obj
	// Indices into chunks -> phys_chunks array
	uint64_t * obj_chunks;
	// UNUSED SO FAR
	uint64_t access_flags;
} Obj;

//	- Initialized at system_init()
//		- THE CRITICAL data structure!
//			- Table keeping track of all the object id -> [phys chunk id] mappings
//			- Equivalent of page table, but for specific objects instead of arbitrary virtual pages
//				- (Uses open addressing, but worth looking into performant hash table implementations)
//	- Modified upon put()
//		- Obj created, inserted into table, count updated
//		- May "grow" (more slots in table) depending on load
//	- Modified upon remove()
//		- Obj removed from table, destroyed, count updated
//		- May "shrink" (less slots in table) if it gets too empty
//	- Read in client's get()
//		- Client looks up obj_id in table to retrieve obj's size and list of chunks
typedef struct obj_table {
	// number of objects in table
	uint64_t cnt;
	// amount of allocated space in table
	uint64_t size;
	// never shrink smaller than min_size
	uint64_t min_size;
	// setting an upper limit on size to prevent out of memory growing
	uint64_t max_size;
	// when cnt > size * load_factor
	// set new size to be size * (1 / load_factor) and grow table
	float load_factor;
	// when cnt < size * shrink_factor
	// set new size to be size * (1 - shrink_factor) and shrink table 
	float shrink_factor;
	// array of pointers to objects
	Obj ** table;
	pthread_mutex_t table_lock;
} Obj_Table;



#endif
