typedef enum _ze_result_t
{
    ZE_RESULT_SUCCESS = 0,                                                  ///< [Core] success
    ZE_RESULT_NOT_READY = 1,                                                ///< [Core] synchronization primitive not signaled
    ZE_RESULT_ERROR_DEVICE_LOST = 0x70000001,                               ///< [Core] device hung, reset, was removed, or driver update occurred
    ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY = 0x70000002,                        ///< [Core] insufficient host memory to satisfy call
    ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY = 0x70000003,                      ///< [Core] insufficient device memory to satisfy call
    ZE_RESULT_ERROR_MODULE_BUILD_FAILURE = 0x70000004,                      ///< [Core] error occurred when building module, see build log for details
    ZE_RESULT_ERROR_MODULE_LINK_FAILURE = 0x70000005,                       ///< [Core] error occurred when linking modules, see build log for details
    ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET = 0x70000006,                     ///< [Core] device requires a reset
    ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE = 0x70000007,                 ///< [Core] device currently in low power state
    ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX = 0x7ff00001,                  ///< [Core, Experimental] device is not represented by a fabric vertex
    ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE = 0x7ff00002,                  ///< [Core, Experimental] fabric vertex does not represent a device
    ZE_RESULT_EXP_ERROR_REMOTE_DEVICE = 0x7ff00003,                         ///< [Core, Experimental] fabric vertex represents a remote device or
                                                                            ///< subdevice
    ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE = 0x7ff00004,                 ///< [Core, Experimental] operands of comparison are not compatible
    ZE_RESULT_EXP_RTAS_BUILD_RETRY = 0x7ff00005,                            ///< [Core, Experimental] ray tracing acceleration structure build
                                                                            ///< operation failed due to insufficient resources, retry with a larger
                                                                            ///< acceleration structure buffer allocation
    ZE_RESULT_EXP_RTAS_BUILD_DEFERRED = 0x7ff00006,                         ///< [Core, Experimental] ray tracing acceleration structure build
                                                                            ///< operation deferred to parallel operation join
    ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS = 0x70010000,                  ///< [Sysman] access denied due to permission level
    ZE_RESULT_ERROR_NOT_AVAILABLE = 0x70010001,                             ///< [Sysman] resource already in use and simultaneous access not allowed
                                                                            ///< or resource was removed
    ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE = 0x70020000,                    ///< [Common] external required dependency is unavailable or missing
    ZE_RESULT_WARNING_DROPPED_DATA = 0x70020001,                            ///< [Tools] data may have been dropped
    ZE_RESULT_ERROR_UNINITIALIZED = 0x78000001,                             ///< [Validation] driver is not initialized
    ZE_RESULT_ERROR_UNSUPPORTED_VERSION = 0x78000002,                       ///< [Validation] generic error code for unsupported versions
    ZE_RESULT_ERROR_UNSUPPORTED_FEATURE = 0x78000003,                       ///< [Validation] generic error code for unsupported features
    ZE_RESULT_ERROR_INVALID_ARGUMENT = 0x78000004,                          ///< [Validation] generic error code for invalid arguments
    ZE_RESULT_ERROR_INVALID_NULL_HANDLE = 0x78000005,                       ///< [Validation] handle argument is not valid
    ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE = 0x78000006,                      ///< [Validation] object pointed to by handle still in-use by device
    ZE_RESULT_ERROR_INVALID_NULL_POINTER = 0x78000007,                      ///< [Validation] pointer argument may not be nullptr
    ZE_RESULT_ERROR_INVALID_SIZE = 0x78000008,                              ///< [Validation] size argument is invalid (e.g., must not be zero)
    ZE_RESULT_ERROR_UNSUPPORTED_SIZE = 0x78000009,                          ///< [Validation] size argument is not supported by the device (e.g., too
                                                                            ///< large)
    ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT = 0x7800000a,                     ///< [Validation] alignment argument is not supported by the device (e.g.,
                                                                            ///< too small)
    ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT = 0x7800000b,            ///< [Validation] synchronization object in invalid state
    ZE_RESULT_ERROR_INVALID_ENUMERATION = 0x7800000c,                       ///< [Validation] enumerator argument is not valid
    ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION = 0x7800000d,                   ///< [Validation] enumerator argument is not supported by the device
    ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT = 0x7800000e,                  ///< [Validation] image format is not supported by the device
    ZE_RESULT_ERROR_INVALID_NATIVE_BINARY = 0x7800000f,                     ///< [Validation] native binary is not supported by the device
    ZE_RESULT_ERROR_INVALID_GLOBAL_NAME = 0x78000010,                       ///< [Validation] global variable is not found in the module
    ZE_RESULT_ERROR_INVALID_KERNEL_NAME = 0x78000011,                       ///< [Validation] kernel name is not found in the module
    ZE_RESULT_ERROR_INVALID_FUNCTION_NAME = 0x78000012,                     ///< [Validation] function name is not found in the module
    ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION = 0x78000013,              ///< [Validation] group size dimension is not valid for the kernel or
                                                                            ///< device
    ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION = 0x78000014,            ///< [Validation] global width dimension is not valid for the kernel or
                                                                            ///< device
    ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX = 0x78000015,             ///< [Validation] kernel argument index is not valid for kernel
    ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE = 0x78000016,              ///< [Validation] kernel argument size does not match kernel
    ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE = 0x78000017,            ///< [Validation] value of kernel attribute is not valid for the kernel or
                                                                            ///< device
    ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED = 0x78000018,                   ///< [Validation] module with imports needs to be linked before kernels can
                                                                            ///< be created from it.
    ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE = 0x78000019,                 ///< [Validation] command list type does not match command queue type
    ZE_RESULT_ERROR_OVERLAPPING_REGIONS = 0x7800001a,                       ///< [Validation] copy operations do not support overlapping regions of
                                                                            ///< memory
    ZE_RESULT_WARNING_ACTION_REQUIRED = 0x7800001b,                         ///< [Sysman] an action is required to complete the desired operation
    ZE_RESULT_ERROR_UNKNOWN = 0x7ffffffe,                                   ///< [Core] unknown or internal error
    ZE_RESULT_FORCE_UINT32 = 0x7fffffff

} ze_result_t;