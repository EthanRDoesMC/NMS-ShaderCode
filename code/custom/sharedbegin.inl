//include the cpu2gpu defines/functions (allow hlsl code to be roughly compatible with cpp)
#ifdef __cplusplus
#include <Graphics/Compute/TkCPU2GPUBegin.inl>
namespace GPU
{
#else

#ifndef D_PLATFORM_GLSL
#define int64 long
#define uint64 ulong
#define mat4x4 float4x4
#endif

#define TKPRINTS(x)
#define TKPRINTVAL(x)
#define TKPRINTVALANDHEX(x)
#define TKASSERT(x)

#define TK_ENABLE_OPTIMIZATION
#define TK_DISABLE_OPTIMIZATION

#define TKINLINE
#define TKFORCEINLINE

#define FALSE 0
#define TRUE 1

#endif
