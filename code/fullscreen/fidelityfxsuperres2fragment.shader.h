////////////////////////////////////////////////////////////////////////////////
///
///     @file       FidelityFXSuperRes2Fragment.shader.h
///     @author     Griff
///     @date       
///
///     @brief      
///
///     Copyright (c) 2022 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#if defined( D_PLATFORM_SWITCH )
    #extension GL_KHR_vulkan_glsl : enable

    #pragma optionNV(unroll all)
    #pragma optionNV(fastmath on)

    #if defined ( D_FSR2_REDUCE_LUMINANCE )
    #extension GL_KHR_shader_subgroup_arithmetic : require
    #endif

    #if defined ( D_FSR2_MOMENTS_PASS )
    #extension GL_KHR_shader_subgroup_clustered  : require
    #endif
#endif

#if defined( D_PLATFORM_MACOS ) || defined( D_PLATFORM_IOS )
    #include "Common/Defines.shader.h"
#endif

#define FFX_GPU

#if defined( D_PLATFORM_PC ) || defined( D_PLATFORM_SWITCH ) || defined ( D_PLATFORM_METAL )
#define FFX_GLSL (1)
#else
#define FFX_HLSL (1)
#endif

#if defined( D_PLATFORM_SCARLETT )

#ifndef SPD_NO_WAVE_OPERATIONS
#define FFX_HALF (1)
#endif
#define FFX_HLSL_6_2 (1)

#define FFX_FSR2_EMBED_ROOTSIG
#define CS main

#elif defined( D_PLATFORM_XBOXGDK ) || defined( D_PLATFORM_DX12 )

#define FFX_FSR2_EMBED_ROOTSIG
#define CS main

#ifndef SPD_NO_WAVE_OPERATIONS
//#define FFX_HALF (1)
#endif

#endif

#if defined( D_PLATFORM_PROSPERO ) 
#pragma argument(realtypes)
//#pragma PSSL_target_output_format(default FMT_32_ABGR)
#pragma argument(wavemode=wave32)
#ifndef SPD_NO_WAVE_OPERATIONS
#define FFX_HALF (1)
#endif
#endif
#if defined( D_PLATFORM_PROSPERO ) ||  defined( D_PLATFORM_ORBIS )

#pragma argument (O4; fastmath)
#pragma argument(unrollallloops)

#if FFX_HALF
#pragma warning (disable: 6922)
#pragma warning (disable: 6928)
#endif

#define RWTexture2D RW_Texture2D        
#define cbuffer ConstantBuffer
#define uvec2 uint2
#define uvec3 uint3
#define uvec4 uint4
//#define min16int2 short2
#define SampleLevel SampleLOD
#define SV_GroupID S_GROUP_ID              
#define SV_GroupIndex S_GROUP_INDEX
#define SV_GroupThreadID S_GROUP_THREAD_ID       
#define SV_DispatchThreadID S_DISPATCH_THREAD_ID
#define groupshared thread_group_memory
#define InterlockedAdd AtomicAdd
#define InterlockedMax AtomicMax
#define CS main
#define numthreads num_threads
//#define globallycoherent __kBuffer_GLC
#define globallycoherent
#define unorm
#define  GroupMemoryBarrierWithGroupSync ThreadGroupMemoryBarrierSync
#define float32_t float
#define float32_t2 float2
#define float32_t3 float3
#define float32_t4 float4
#define uint32_t uint
#define uint32_t2 uint2
#define uint32_t3 uint3
#define uint32_t4 uint4
#define int32_t int
#define int32_t2 int2
#define int32_t3 int3
#define int32_t4 int4
#define uint16_t  ushort
#define uint16_t2 ushort2
#define uint16_t3 ushort3
#define uint16_t4 ushort4
#define int16_t  short
#define int16_t2 short2
#define int16_t3 short3
#define int16_t4 short4
#define float16_t half
#define float16_t2 half2
#define float16_t3 half3
#define float16_t4 half4
#define float16_t half
#define float16_t2 half2
#define float16_t3 half3
#define float16_t4 half4
typedef half min16float;
typedef half2 min16float2;
typedef half3 min16float3;
typedef half4 min16float4;
typedef ushort min16uint;
typedef ushort2 min16uint2;
typedef ushort3 min16uint3;
typedef ushort4 min16uint4;
typedef short min16int;
typedef short2 min16int2;
typedef short3 min16int3;
typedef short4 min16int4;

#endif


// FSR2 options
#if defined( D_FSR2_USE_OPT_SETTINGS )
#define FFX_FSR2_OPTION_USE_LANCZOS_LUT                         0
#define FFX_FSR2_OPTION_UPSAMPLE_USE_LANCZOS_TYPE               0
#define FFX_FSR2_OPTION_HDR_COLOR_INPUT                         1
#define FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS           1
#define FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS                 0
#define FFX_FSR2_OPTION_INVERTED_DEPTH                          1
#define FFX_FSR2_OPTION_APPLY_SHARPENING                        0
#else
#define FFX_FSR2_OPTION_USE_LANCZOS_LUT                         1
#define FFX_FSR2_OPTION_UPSAMPLE_USE_LANCZOS_TYPE               1
#define FFX_FSR2_OPTION_HDR_COLOR_INPUT                         1
#define FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS           1
#define FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS                 0
#define FFX_FSR2_OPTION_INVERTED_DEPTH                          1
#define FFX_FSR2_OPTION_APPLY_SHARPENING                        0
#endif

// FSR2 custom options
#if defined( D_FSR2_USE_OPT_SETTINGS )
#define FFX_FSR2_CUSTOM_OPTION_INPUT_COLOR_ALREADY_PREPARED     1
#define FFX_FSR2_CUSTOM_OPTION_DILATED_BUFFERS_ALREADY_PREPARED 1
#define FFX_FSR2_CUSTOM_OPTION_NO_CLEARS_ON_COLOR_PREPARATION   1
#define FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOUR_LUMA_BUFFERS        0
#define FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT   1
#define FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOR_OUTPUT               0
#define FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION         1
#define FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS                1
#define FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_DEPTH_CLIP           1
#define FFX_FSR2_CUSTOM_OPTION_LOCKS_LUMA_BIAS                  1
#define FFX_FSR2_CUSTOM_OPTION_DEPTH_MIN_BIAS                   1
#define FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS          1
#define FFX_FSR2_CUSTOM_OPTION_COMPACT_LOCKS                    1
#define FFX_FSR2_CUSTOM_OPTION_COMPACT_HISTORY_WEIGHT           1
#define FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE                      1
#define FFX_FSR2_CUSTOM_OPTION_NO_PRE_EXPOSURE                  1
#define FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING                   0
#define FFX_FSR2_CUSTOM_OPTION_NO_ACCUMULATION_MASK             1
#define FFX_FSR2_CUSTOM_OPTION_SATURATE_OUTPUT                  1
#else
#define FFX_FSR2_CUSTOM_OPTION_INPUT_COLOR_ALREADY_PREPARED     0
#define FFX_FSR2_CUSTOM_OPTION_DILATED_BUFFERS_ALREADY_PREPARED 0
#define FFX_FSR2_CUSTOM_OPTION_NO_CLEARS_ON_COLOR_PREPARATION   0
#define FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOUR_LUMA_BUFFERS        0
#define FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT   0
#define FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOR_OUTPUT               1
#define FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION         0
#define FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS                0
#define FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_DEPTH_CLIP           0
#define FFX_FSR2_CUSTOM_OPTION_LOCKS_LUMA_BIAS                  1
#define FFX_FSR2_CUSTOM_OPTION_DEPTH_MIN_BIAS                   0
#define FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS          1
#define FFX_FSR2_CUSTOM_OPTION_COMPACT_LOCKS                    0
#define FFX_FSR2_CUSTOM_OPTION_COMPACT_HISTORY_WEIGHT           0
#define FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE                      1
#define FFX_FSR2_CUSTOM_OPTION_NO_PRE_EXPOSURE                  1
#define FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING                   0
#define FFX_FSR2_CUSTOM_OPTION_NO_ACCUMULATION_MASK             1
#define FFX_FSR2_CUSTOM_OPTION_SATURATE_OUTPUT                  1
#endif

// FSR2 per-pass options overrides
#if defined( D_FSR2_ACCUMULATE_SHARPEN_PASS_COMPUTE )
#undef  FFX_FSR2_OPTION_APPLY_SHARPENING
#define FFX_FSR2_OPTION_APPLY_SHARPENING                        1
#endif

#if defined ( D_PLATFORM_METAL )

typedef half  min16float;
typedef half2 min16float2;
typedef half3 min16float3;
typedef half4 min16float4;
typedef ushort  min16uint;
typedef ushort2 min16uint2;
typedef ushort3 min16uint3;
typedef ushort4 min16uint4;
typedef short  min16int;
typedef short2 min16int2;
typedef short3 min16int3;
typedef short4 min16int4;

typedef half  float16_t;
typedef half2 f16vec2;
typedef half3 f16vec3;
typedef half4 f16vec4;
typedef ushort  min16uint;
typedef ushort2 min16uint2;
typedef ushort3 min16uint3;
typedef ushort4 min16uint4;
typedef short  min16int;
typedef short2 i16vec2;
typedef short3 i16vec3;
typedef short4 i16vec4;
typedef ushort  min16uint;
typedef ushort2 u16vec2;
typedef ushort3 u16vec3;
typedef ushort4 u16vec4;

//#define rcp(X) (1.0 / (X))

uint f32tof16(min16float2 v)
{
    return pack_half_to_unorm2x16(v);
}
uint packHalf2x16(min16float2 toPack)
{
    return pack_half_to_unorm2x16(toPack);
}
uint packHalf2x16(float2 toPack)
{
    return pack_float_to_unorm2x16(toPack);
}
uint packFloat2x16(f16vec2 toPack)
{
    return pack_half_to_unorm2x16(toPack);
}
uint packFloat2x16(float2 toPack)
{
    return pack_float_to_unorm2x16(toPack);
}

f16vec2 unpackFloat2x16(uint toUnpack)
{
    return unpack_unorm2x16_to_half(toUnpack);
}
float16_t uint16BitsToHalf(uint16_t toConvert)
{
    return as_type<float16_t>(toConvert);
}
f16vec2 uint16BitsToHalf(u16vec2 toConvert)
{
    return as_type<f16vec2>(toConvert);
}
f16vec3 uint16BitsToHalf(u16vec3 toConvert)
{
    return as_type<f16vec3>(toConvert);
}
f16vec4 uint16BitsToHalf(u16vec4 toConvert)
{
    return as_type<f16vec4>(toConvert);
}

u16vec4 halfBitsToUint16(f16vec4  conv)
{
    return as_type<u16vec4>(conv);
}
u16vec3 halfBitsToUint16(f16vec3  conv)
{
    return as_type<u16vec3>(conv);
}
u16vec2 halfBitsToUint16(f16vec2  conv)
{
    return as_type<u16vec2>(conv);
}
uint16_t halfBitsToUint16(float16_t  conv)
{
    return as_type<uint16_t>(conv);
}
u16vec2 unpackUint2x16(uint tounpack)
{
    return as_type<u16vec2>(tounpack);
}
uint packUint2x16(u16vec2 tounpack)
{
    return as_type<uint>(tounpack);
}
#endif

#if defined ( D_PLATFORM_METAL ) || ( !defined ( D_PLATFORM_PROSPERO ) && defined ( D_PLATFORM_ORBIS ) )
    // Commented out atm...
    #include "Common/CommonUniforms.shader.h"
    #include "Fullscreen/PostCommon.h"

    #if defined ( D_FSR2_QUAD )
        // Vertex shader.
        DECLARE_INPUT
        DECLARE_INPUT_END

        DECLARE_OUTPUT
        OUTPUT_SCREEN_POSITION
        DECLARE_OUTPUT_END

        DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
        OUTPUT_SCREEN_POSITION_REDECLARED
        DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

        VERTEX_MAIN_INSTANCED_SRT
        {
            SCREEN_POSITION = vec4(0.0, 0.0, 0.0, 1.0);
        }
    #else
        DECLARE_INPUT
        INPUT_SCREEN_POSITION
        INPUT_SCREEN_SLICE

        INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
        DECLARE_INPUT_END

        FRAGMENT_MAIN_COLOUR_SRT
        {
            WRITE_FRAGMENT_COLOUR(vec4(1.0, 0.0, 0.0, 1.0));
        }
    #endif

#else

#if defined( D_PLATFORM_SWITCH ) && defined( D_FSR2_ACCUMULATE_PASS_FRAGMENT )
    #define FFX_HALF (1)
#endif

// FSR2 option combinations checks
#if FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT
    #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION == 0
    #error Split internal weight is only implemented in the lightweight accumulation pass.
    #endif
#endif

#if defined( D_FSR2_COMPUTELUMINANCEPYRAMID )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_compute_luminance_pyramid_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_compute_luminance_pyramid_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_REDUCE_LUMINANCE )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_reduce_luminance_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_reduce_luminance_pass.glsl"
#endif
#endif

#if defined( D_FSR2_PREPARE_INPUT_COLOR_PASS )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_prepare_input_color_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_prepare_input_color_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_STABILITY_PASS_FRAGMENT )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_stability_pass_frag.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_stability_pass_frag.glsl"
#endif
#endif

#if defined( D_FSR2_STABILITY_PASS_COMPUTE )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_stability_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_stability_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_RECONSTRUCT_PREVIOUS_DEPTH_PASS )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_reconstruct_previous_depth_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_reconstruct_previous_depth_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_DEPTH_CLIP_PASS_FRAGMENT )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_depth_clip_pass_frag.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_depth_clip_pass_frag.glsl"
#endif
#endif

#if defined( D_FSR2_DEPTH_CLIP_PASS_COMPUTE )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_depth_clip_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_depth_clip_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_LOCK_PASS )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_lock_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_lock_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_ACCUMULATE_PASS_FRAGMENT )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate_pass_frag.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate_pass_frag.hlsl"
#endif
#endif

#if defined( D_FSR2_ACCUMULATE_PASS_COMPUTE )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_ACCUMULATE_SHARPEN_PASS_COMPUTE )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_RCAS_PASS )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_rcas_pass.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_rcas_pass.hlsl"
#endif
#endif

#if defined( D_FSR2_QUAD )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_quad_pass_vert.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_quad_pass_vert.hlsl"
#endif
#endif

#if defined( D_FSR2_QUAD_TEST_FRAG )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_quad_pass_frag.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_quad_pass_frag.glsl"
#endif
#endif

#if defined( D_FSR2_DEBUG )
#if defined( FFX_GLSL )
#include "Fullscreen/FidelityFX/ffx_fsr2_debug_pass_frag.glsl"
#else
#include "Fullscreen/FidelityFX/ffx_fsr2_debug_pass_frag.glsl"
#endif
#endif

#endif // defined ( D_PLATFORM_METAL )
