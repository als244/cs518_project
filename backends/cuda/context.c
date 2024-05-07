#include "cuda_backend.h"

Cuda_Context * cuda_init_context(int device_id){

	Cuda_Context * cuda_context = (Cuda_Context *) malloc(sizeof(Cuda_Context));

	if (cuda_context == NULL){
		fprintf(stderr, "Error: malloc failed in init_cuda_context\n");
		return NULL;
	}

	cuda_context -> device_id = device_id;

	// I don't think any special flags are necessary?
		// https://docs.nvidia.com/cuda/cuda-driver-api/group__CUDA__CTX.html
	unsigned int flags = 0;

	CUresult result;
	const char * err;

	CUcontext ctx;
	result = cuCtxCreate(&ctx, 0, device_id);
	if (result != CUDA_SUCCESS){
		cuGetErrorString(result, &err);
    	fprintf(stderr, "Error: Could not create context: %s\n", err);
    	// TODO:
    	//	- more cleanup needed here...
    	return NULL;
	}

	cuda_context -> context = ctx;

	return cuda_context;
}