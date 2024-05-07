#include "cuda_backend.h"

int cuda_init_driver(unsigned int init_flags){

	CUresult result;
	const char * err;

    result = cuInit(init_flags);

    if (result != CUDA_SUCCESS){
    	cuGetErrorString(result, &err);
    	fprintf(stderr, "Could not init cuda: %s\n", err);
    	return -1;
    }

    return 0;

}