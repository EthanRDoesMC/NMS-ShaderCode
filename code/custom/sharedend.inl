
//make sure generate definitions is cleared if it was defined
#ifdef TKGPU_GENERATE_DEFINITIONS
#undef TKGPU_GENERATE_DEFINITIONS
#endif 

//un-include the cpu2gpu defines/functions
#ifdef __cplusplus
}
#include <Graphics/Compute/TkCPU2GPUEnd.inl>
#endif

