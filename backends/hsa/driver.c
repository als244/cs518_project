#include "hsa_backend.h"

int hip_init_driver(unsigned int init_flags){

	hipError_t result;
	const char * err;

    result = hipInit(init_flags);

    if (result != hipSuccess){
    	err = hipGetErrorString(result);
    	fprintf(stderr, "Could not init hip: %s\n", err);
    	return -1;
    }

    return 0;

}