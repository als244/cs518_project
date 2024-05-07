#include "ze_backend.h"

int ze_init_driver(unsigned int init_flags){

	ze_result_t result;

    result = zeInit(init_flags);

    if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Could not init ze: %d\n", result);
    	return -1;
    }

    return 0;

}