#include "ze_backend.h"

Ze_Context * ze_init_context(int device_id){

	ze_result_t result;

	Ze_Context * ze_context = (Ze_Context *) malloc(sizeof(Ze_Context));

	if (ze_context == NULL){
		fprintf(stderr, "Error: malloc failed in init_ze_context\n");
		return NULL;
	}

	// 1.) Set devcie id
	ze_context -> device_id = device_id;

	// 2.) Set driver_handle
	ze_driver_handle_t drv_handle;
	uint32_t dev_count = 1;
	result = zeDriverGet(&dev_count, &drv_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not get driver handle: %d\n", result);
    	return NULL;
	}
	ze_context -> drv_handle = (void *) drv_handle;

	// 3.) Set device handle
	ze_device_handle_t dev_handle;
	result = zeDeviceGet(drv_handle, &dev_count, &dev_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not get device handle: %d\n", result);
    	return NULL;
	}
	ze_context -> dev_handle = dev_handle;

	// 4.) Set context handle
	ze_context_desc_t ctxDesc = {
   		ZE_STRUCTURE_TYPE_CONTEXT_DESC,
   		NULL,
   		0
	};
	ze_context_handle_t ctx_handle;
	result = zeContextCreate(drv_handle, &ctxDesc, &ctx_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not create context: %d\n", result);
    	return NULL;
	}
	ze_context -> ctx_handle = (void *) ctx_handle;

	// 5.) Create command queue
	ze_command_queue_handle_t cmd_queue_handle;

	ze_command_queue_desc_t cmd_queue_desc = {
		ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
		NULL,
		0,
		0,
		ZE_COMMAND_QUEUE_MODE_SYNCHRONOUS,
		ZE_COMMAND_QUEUE_PRIORITY_NORMAL
	};

	result = zeCommandQueueCreate(ctx_handle, dev_handle, &cmd_queue_desc, &cmd_queue_handle);
	if (result != ZE_RESULT_SUCCESS){
    	fprintf(stderr, "Error: Could not create command queue: %d\n", result);
    	return NULL;
	}

	ze_context -> cmd_queue_handle = cmd_queue_handle;

	return ze_context;
}