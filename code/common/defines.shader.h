// =================================================================================================
   
#ifndef D_DEFINES
#define D_DEFINES

#if defined(D_PLATFORM_MACOS) || defined(D_PLATFORM_IOS)
#ifndef D_PLATFORM_METAL
#define D_PLATFORM_METAL
#endif
#endif

#if defined ( D_PLATFORM_SWITCH ) || defined ( D_PLATFORM_SWITCH_COMPUTE )
// Switch doesn't have the bandwidth to support tiled lights at the moment, those large buffers will kill the platform.
#ifdef D_TILED_LIGHTS
#undef D_TILED_LIGHTS
#endif
#endif

#define MOTION_VECTORS_FUNC_CONSTANT_INDEX  0
#define USE_FRAG_DISCARD_FN_CONSTANT_INDEX  1
#define MAX_LIGHT_COUNT_FN_CONSTANT_INDEX   2
#define USE_SPOTLIGHT_COOKIES_FN_CONSTANT   3

#if defined(D_PLATFORM_METAL)
  //used as a hint for preprocess stages.
  //avoid preprocessing the metal includes
	_#include <metal_stdlib>
	using namespace metal;
	_#include <metal_atomic>
	_#include <metal_simdgroup>
	_#include <metal_quadgroup>
  #define MTL_ID(X) [[id(X)]]
  
  constant bool HAS_MOTION_VECTORS    [[ function_constant(MOTION_VECTORS_FUNC_CONSTANT_INDEX) ]];
  constant int MTL_MAX_LIGHT_COUNT    [[ function_constant(MAX_LIGHT_COUNT_FN_CONSTANT_INDEX) ]];
  constant bool USE_SPOTLIGHT_COOKIES [[ function_constant(USE_SPOTLIGHT_COOKIES_FN_CONSTANT) ]];
#else
  #define MTL_ID(X)
  #define HAS_MOTION_VECTORS  true
  #define INPUT_VARIANT(TYPE, NAME, REG, VARIANT) INPUT(TYPE, NAME, REG)
  #define OUTPUT_VARIANT(TYPE, NAME, REG, VARIANT) OUTPUT(TYPE, NAME, REG)
  #define FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(VARIANT) FRAGMENT_MAIN_COLOUR012_SRT
  #define FRAGMENT_MAIN_COLOUR012_DEPTH_SRT_VARIANT(VARIANT) FRAGMENT_MAIN_COLOUR012_DEPTH_SRT
#endif


#if defined D_PLATFORM_XBOXGDK  && !defined D_PLATFORM_XBOXONE
#define D_PLATFORM_XBOXONE
#endif

#if defined D_PLATFORM_SCARLETT && !defined D_PLATFORM_XBOXONE
#define D_PLATFORM_XBOXONE
#endif

#if defined __XBOX_ONE && !defined D_PLATFORM_XBOXONE
#define D_PLATFORM_XBOXONE
#endif
#if defined( D_PLATFORM_XBOXONE )
// spilling to LDS is an error
#pragma warning( error:4803 )
#endif

#if defined D_PLATFORM_PC && !defined D_PLATFORM_DX12

#define D_PLATFORM_GLSL

#ifdef VULKAN
#define D_PLATFORM_VULKAN
#else
#define D_PLATFORM_OPENGL
#endif

#elif defined D_PLATFORM_XBOXONE && !defined D_PLATFORM_DX12

#define D_PLATFORM_DX12

#elif defined D_PLATFORM_PROSPERO

#define D_PLATFORM_ORBIS

//#define D_COMPUTE_DISABLEWAVE32
#if !defined D_COMPUTE_DISABLEWAVE32
#pragma argument(wavemode=wave32)
#endif

#pragma argument(realtypes)
//#pragma argument(hardwarerevision=proto3)

#elif defined D_PLATFORM_SWITCH

#define D_PLATFORM_GLSL
#extension GL_NV_desktop_lowp_mediump : enable

#endif

// =================================================================================================
// Platform defines
// =================================================================================================
#if defined D_PLATFORM_GLSL

    #define D_ENABLE_REVERSEZ_PROJECTION  (1)

    #pragma optionNV(strict on)
    #extension GL_ARB_gpu_shader5 : enable
    #extension GL_ARB_fragment_coord_conventions : enable
	#extension GL_ARB_derivative_control : enable

	//TF_BEGIN
	#if defined(GL_ARB_shader_stencil_export)
		#extension GL_ARB_shader_stencil_export : enable
	#endif
	//TF_END
    #if defined(GL_SPIRV) || defined( D_PLATFORM_VULKAN )
        #extension GL_ARB_gpu_shader_int64 : enable
    #endif

    #if defined( GL_NV_gpu_shader5 )
        #extension GL_NV_gpu_shader5 : enable
    #endif

    #if defined( GL_AMD_gpu_shader_half_float )
        #extension GL_AMD_gpu_shader_half_float : enable
    #endif

    #if defined( GL_ARB_shader_ballot )
        #extension GL_ARB_shader_ballot : enable
    #endif

    #if defined ( D_PLATFORM_SWITCH ) //&& defined ( GL_ARB_bindless_texture )
        #extension GL_ARB_bindless_texture : enable
        #extension GL_NV_shader_buffer_load : enable
        #extension GL_KHR_vulkan_glsl : enable
        //#extension GL_ARB_shader_stencil_export : enable
        //#extension GL_ARB_shader_image_load_store : enable
        //#extension GL_ARB_shader_image_size : enable
        //#extension GL_NV_shader_atomic_float : enable
        //#extension GL_NV_shader_atomic_int64 : enable
        //#extension GL_NV_shader_atomic_fp16_vector : enable
        #extension GL_NV_extended_pointer_atomics : enable
        #extension GL_ARB_shader_storage_buffer_object : enable
        #extension GL_EXT_shader_image_load_formatted : enable
        #extension GL_EXT_shader_image_load_store : enable
    #endif
    
    #ifdef D_USE_CROSS_LANE
        #if defined( GL_KHR_shader_subgroup_basic )
            #extension GL_KHR_shader_subgroup_basic : enable
        #endif

        #if defined( GL_KHR_shader_subgroup_ballot )
            #extension GL_KHR_shader_subgroup_ballot : enable
        #endif

        #if defined( GL_KHR_shader_subgroup_arithmetic )
            #extension GL_KHR_shader_subgroup_arithmetic : enable
        #endif

        #if defined( GL_KHR_shader_subgroup_shuffle )
            #extension GL_KHR_shader_subgroup_shuffle : enable
        #endif
    #endif

    #if defined( D_FRAGMENT ) && defined( _F64_ )
        layout(early_fragment_tests) in;
    #endif

    #if defined ( D_COMPUTE ) && !defined ( D_PLATFORM_SWITCH )
        #define D_PLATFORM_PC_COMPUTE
    #elif defined ( D_COMPUTE ) && defined ( D_PLATFORM_SWITCH )
        #define D_PLATFORM_SWITCH_COMPUTE
    #endif

    #if defined ( D_PLATFORM_VULKAN ) || defined ( D_PLATFORM_SWITCH )
        #define D_UNIFORMS_ARE_GLOBAL
    #else
    #endif

    #if defined ( D_PLATFORM_VULKAN )
        #define D_USE_UBO
    #endif

    #ifdef D_PLATFORM_PC_LOWEND 
        #undef D_PLATFORM_PC_LOWEND
        #define D_PLATFORM_PC_LOWEND true
        #define D_PLATFORM_NOT_LOWEND false
    #endif

    #ifndef D_PLATFORM_PC_LOWEND 
        #if defined ( D_PLATFORM_OPENGL )
            #define D_PLATFORM_PC_LOWEND false
            #define D_PLATFORM_NOT_LOWEND true
        #elif defined ( D_PLATFORM_SWITCH )
            #define D_PLATFORM_PC_LOWEND false
            #define D_PLATFORM_NOT_LOWEND true
        #else
            layout (constant_id = 0) const int gIsLowEnd = 0;
            #define D_PLATFORM_PC_LOWEND  (gIsLowEnd == 1)
            #define D_PLATFORM_NOT_LOWEND (gIsLowEnd == 0)
        #endif
    #endif

    #if defined( D_PLATFORM_VULKAN ) && defined( D_VERTEX ) && defined( D_INSTANCE ) && !defined( D_SHADOW )
        invariant gl_Position;
    #endif

#elif defined D_PLATFORM_DX12

	#define D_ENABLE_REVERSEZ_PROJECTION  (1)
    #define __XBOX_CONTROL_NONIEEE 0

    #ifndef D_SAMPLERS_ARE_GLOBAL
      #define D_SAMPLERS_ARE_GLOBAL
    #endif

    #define D_PLATFORM_PC_LOWEND false
    #define D_PLATFORM_NOT_LOWEND true

#elif defined( D_PLATFORM_ORBIS )

	#define D_SRT
    #define D_ENABLE_REVERSEZ_PROJECTION  (1)

    // use this with sdk 2.0 compiler 
   // #pragma argument (allow-scratch-buffer-spill)

    //define these flags so they don't get ignored in build process and in the comb mask
    //this is because materials, vertex layouts and shaders need to be synced on 360 to avoid patching
    #ifdef _F27_VBTANGENT
    #endif
    #ifdef _F28_VBSKINNED
    #endif
    #ifdef _F29_VBCOLOUR
    #endif
    #ifdef _F21_VERTEXCOLOUR
    #endif
    #ifdef _F02_SKINNED
    #endif
    #ifdef _F03_NORMALMAP
    #endif
    #if defined( _F01_DIFFUSEMAP ) || defined( D_LOD0 ) || defined( D_LOD1 ) || defined( D_LOD2 ) || defined( D_LOD3) || defined( D_LOD4 ) 
    #endif
    #ifdef _F01_DIFFUSEMAP
    #endif
    #ifdef _F09_TRANSPARENT
    #endif
    #ifdef _F10_NORECEIVESHADOW
    #endif

    // disable warnings for unused parameters. This happens a lot because of defining different things.
    #pragma warning (disable: 5203)

    #ifdef __PSSL_CS__
    #define D_PLATFORM_ORBIS_COMPUTE
    #define D_COMPUTE
    #endif

    #ifdef __PSSL_HS__
    #define D_HULL
    #endif

    #ifdef __PSSL_DS__
    #define D_DOMAIN
    #endif

    #ifdef __PSSL_VS__
    #define D_VERTEX
    #endif

    #ifdef __PSSL_VS_LS__
    // this is a vertex shader that's part of a tessellation chain
    #define D_VERTEX
    #endif

    #ifdef __PSSL_GS__
    #define D_GEOMETRY
    #endif

    #ifdef __PSSL_VS_ES__
    // off chip Geometry
    #define D_VERTEX
    #define D_PLATFORM_ORBIS_GEOMETRY
    #endif

    #define D_PLATFORM_PC_LOWEND false
    #define D_PLATFORM_NOT_LOWEND true

#elif defined(D_PLATFORM_METAL)
//TF_BEGIN
	constant bool use_discard_fragment [[function_constant(USE_FRAG_DISCARD_FN_CONSTANT_INDEX)]];
	#define discard do { if(use_discard_fragment) discard_fragment(); } while(0)
	#define D_ENABLE_REVERSEZ_PROJECTION  (1)
	#define __XBOX_CONTROL_NONIEEE 0
	#define D_PLATFORM_PC_LOWEND false
	#define D_PLATFORM_NOT_LOWEND true
//TF_END
#endif

// =================================================================================================
// Basic Types
// =================================================================================================
#ifdef D_PLATFORM_GLSL
    //#define CONST         const
    #define STATIC_CONST  const
    #define lerp          mix
    //#define half4 f16vec4
    //#define half3 f16vec3
    //#define half2 f16vec2
    //#define half float16_t

    #if defined ( D_PLATFORM_SWITCH )
    #define half            float16_t
    #define half2           f16vec2
    #define half3           f16vec3
    #define half4           f16vec4
    #else
    #define half            float
    #define half2           vec2
    #define half3           vec3
    #define half4           vec4
    #endif

    #define long int64_t    
    #define long2 i64vec2
    #define long3 i64vec3
    #define long4 i64vec4
    #define ulong uint64_t    
    #define ulong2 u64vec2    
    #define ulong3 u64vec3    
    #define ulong4 u64vec4    
    #define int64 int64_t
    #define uint64 uint64_t
    #define float2vec2(_a) vec2(_a)
    #define float2vec3(_a) vec3(_a)
    #define float2vec4(_a) vec4(_a)
	#define float2half2( _a ) half2( _a )
	#define float2half3( _a ) half3( _a )
	#define float2half4( _a ) half4( _a )
    #define cast2mat3(_m)  mat3(_m)
	#define groupID				gl_WorkGroupID
	#define groupThreadID		gl_LocalInvocationID
	#define dispatchThreadID	gl_GlobalInvocationID
    #define instanceID          gl_InstanceIndex
	#if defined( D_PLATFORM_VULKAN )
	#define vertexID			gl_VertexIndex
	#else
	#define vertexID			gl_VertexID
	#endif
	#define THREADGROUP_LOCAL   shared 
	#define THREADGROUP_BARRIER groupMemoryBarrier()
    #define THREADGROUP_BARRIER_SYNC groupMemoryBarrier()

    #ifdef D_USE_CROSS_LANE
    ulong ballot( bool b )
    {
        uvec4 vals = subgroupBallot(b);
        return vals.x | (vals.y << 32);
    }

    #define CrossLaneOr subgroupOr 
    #define ReadFirstLane subgroupBroadcastFirst
    #define ReadLane subgroupBroadcast
    #define ReadLaneDynamic subgroupShuffle
    #endif

#elif defined(D_PLATFORM_DX12)

    //#if !defined(D_PLATFORM_SCARLETT)
    #define half            float
    #define half2           float2
    #define half3           float3
    #define half4           float4
    //#endif
    #define vec2            float2
    #define vec3            float3
    #define vec4            float4
    #define ivec2           int2
    #define ivec3           int3
    #define ivec4           int4
    #define uvec2           uint2
    #define uvec3           uint3
    #define uvec4           uint4
    #define bvec2           bool2
    #define bvec3           bool3
    #define bvec4           bool4
    

    #define r8i             int
    #define r8ui            uint
    #define rgba32f         float4
    #define r32f            float
    #define r32i            int
    #define r32ui           uint
    #define rgba32ui        uint4
    #define rg32ui          uint2
    // NOTE:
    // operator[] accesses rows, not columns 
    // matrix constructors interpret the passed vectors as row vectors, not column vectors
    #define mat2            float2x2
    #define mat3            float3x3
    #define mat4            float4x4

    #define Texture2D_Array     Texture2DArray
    #define RW_Texture2D        RWTexture2D
    #define RW_Texture3D        RWTexture3D
    #define RW_DataBuffer       RWBuffer
    #define DataBuffer          Buffer
    #define RW_RegularBuffer    RWStructuredBuffer
    #define RegularBuffer       StructuredBuffer
    #define SampleLOD           SampleLevel
    #define SampleCmpLOD0       SampleCmpLevelZero
    #define SampleGradient      SampleGrad

    #define THREADGROUP_LOCAL       groupshared
    #define THREADGROUP_BARRIER     GroupMemoryBarrier()
    #define THREADGROUP_BARRIER_SYNC GroupMemoryBarrier()
    #define S_GROUP_ID              SV_GroupID
    #define S_GROUP_THREAD_ID       SV_GroupThreadID
    #define S_DISPATCH_THREAD_ID    SV_DispatchThreadID

    #define SRT(STRUCTURE, NAME)    NAME

    #define STATIC_CONST    static const

    #define float2vec2( _a ) _a
    #define float2vec3( _a ) _a
    #define float2vec4( _a ) _a
    #define float2half2( _a ) _a
    #define float2half3( _a ) _a
    #define float2half4( _a ) _a
    #define cast2mat3( _m )  (mat3) _m
    //#define lerp          mix

    #define gl_SubgroupSize WaveGetLaneCount()
    #define gl_SubgroupInvocationID WaveGetLaneIndex()
    #define ballot WaveBallot
#ifdef D_PLATFORM_XBOXONE
    #define CrossLaneOr __XB_WaveOR 
#else
    #define CrossLaneOr WaveActiveBitOr 
#endif
    #define ReadFirstLane WaveReadLaneFirst
    #define ReadLane WaveReadLaneAt
    #define ReadLaneDynamic WaveReadLaneAt

#elif defined D_PLATFORM_ORBIS

    #ifndef D_PLATFORM_PROSPERO
    #define half            float
    #define half2           float2
    #define half3           float3
    #define half4           float4
    #endif
    #define vec2            float2
    #define vec3            float3
    #define vec4            float4
    #define ivec2           int2
    #define ivec3           int3
    #define ivec4           int4
    #define uvec2           uint2
    #define uvec3           uint3
    #define uvec4           uint4
    #define bvec2           bool2
    #define bvec3           bool3
    #define bvec4           bool4

    #define r8i             int
    #define r8ui            uint
    #define rgba32f         float4
    #define r32f            float
    #define r32i            int
    #define r32ui           uint
    #define rgba32ui        uint4
    #define rg32ui          uint2
    // NOTE: 
    // operator[] accesses rows, not columns 
    // matrix constructors interpret the passed vectors as row vectors, not column vectors
    #define mat2            row_major float2x2
    #define mat3            row_major float3x3
    #define mat4            row_major float4x4

	#define SRT(STRUCTURE, NAME)    STRUCTURE.NAME
    #define STATIC_CONST    static const
   // #define const           ERROR, DON'T USE CONST FOR PS4. USE STATIC_CONST INSTEAD FOR A COMPILED IN CONSTANT. OTHERWISE IT TRIES TO PUT IT IN A CONSTANT BUFFER AND FOR SOME REASON IT DOESN'T WORK.

	#define float2vec2( _a ) vec2( _a )
	#define float2vec3( _a ) vec3( _a )
	#define float2vec4( _a ) vec4( _a )
	#define float2half2( _a ) half2( _a )
	#define float2half3( _a ) half3( _a )
	#define float2half4( _a ) half4( _a )
    #define cast2mat3( _m )  (mat3) _m

    #define THREADGROUP_LOCAL    thread_group_memory
    #define THREADGROUP_BARRIER  ThreadGroupMemoryBarrier()
    #define THREADGROUP_BARRIER_SYNC  ThreadGroupMemoryBarrierSync()

    #define gl_SubgroupInvocationID ExclusivePrefixSum( 1 )
    #define ReadLaneDynamic ReadLane
        
    #if defined( D_PLATFORM_SWITCH )  
    #define gl_SubgroupSize 32
    #elif (!defined( D_PLATFORM_PROSPERO )) || defined( D_COMPUTE_DISABLEWAVE32 )
    #define gl_SubgroupSize 64
    #else
    #define gl_SubgroupSize 32
    #endif

#elif defined(D_PLATFORM_METAL)
//TF_BEGIN
#define inout(Type, Name) thread Type& Name      
#define out(Type, Name) thread Type& Name  
#define inoutPtr(Type, Name) thread Type * Name
#define outPtr(Type, Name) thread Type * Name     
//#define in
//#define in(T)     thread T     

#define half            half
#define half2           half2
#define half3           half3
#define half4           half4
#define vec2            float2
#define vec3            float3
#define vec4            float4
#define ivec2           int2
#define ivec3           int3
#define ivec4           int4
#define uvec2           uint2
#define uvec3           uint3
#define uvec4           uint4
#define bvec2           bool2
#define bvec3           bool3
#define bvec4           bool4

#define rgba32f         float4
#define r32f            float
#define r32i            int
#define rgba32ui        uint4
#define rg32ui          uint2
// NOTE: 
// operator[] accesses rows, not columns 
// matrix constructors interpret the passed vectors as row vectors, not column vectors
#define mat2            float2x2
#define mat3            float3x3
#define mat4            float4x4

#define Texture2D_Array		texture2d_array
#define RW_Texture2D		  texture2d
#define RW_Texture3D      texture3d
#define RW_DataBuffer( TYPE, NAME )  device TYPE * NAME	  
#define DataBuffer			
#define RW_RegularBuffer	
#define RegularBuffer		
#define SampleLOD			sample
#define SampleCmpLOD0		sample
#define SampleGradient		sample

#define THREADGROUP_LOCAL       threadgroup
#define THREADGROUP_BARRIER     threadgroup_barrier(mem_flags::mem_threadgroup)
#define THREADGROUP_BARRIER_SYNC threadgroup_barrier(mem_flags::mem_threadgroup)
#define S_GROUP_ID              [[threadgroup_position_in_grid]]
#define S_GROUP_THREAD_ID       [[thread_position_in_threadgroup]]
#define S_DISPATCH_THREAD_ID    [[thread_position_in_grid]]

#define SRT(STRUCTURE, NAME)	NAME

#define STATIC_CONST    constant

#define float2vec2( _a ) vec2(_a)
#define float2vec3( _a ) vec3(_a)
#define float2vec4( _a ) vec4(_a)
#define float2half2( _a ) half2( _a )
#define float2half3( _a ) half3( _a )
#define float2half4( _a ) half4( _a )    
#define cast2mat3( _m )  mat3 (_m[0].xyz, _m[1].xyz, _m[2].xyz)
#define lerp          mix

#define gl_SubgroupSize WaveGetLaneCount()
#define gl_SubgroupInvocationID simd_lane_id
#define ballot uint4
#define CrossLaneOr  

#define ReadFirstLane simd_broadcast_first
#define ReadLane simd_shuffle
#define ReadLaneDynamic simd_shuffle


mat4 operator/(mat4 a, float b)
{
  return a * ( 1.0f / b);
}
mat4 operator/(const constant mat4 &a, float b)
{
  return a * ( 1.0f / b);
}

int sign(int x)
{
  return (int)sign(float(x));
}

float log2(int x)
{
  return log2(float(x));
}

float log10(constant int& x)
{
  return log10(float(x));
}

float length(float x)
{
  return abs(x);  
}

template<typename T>
T clamp(T x, int minVal, int maxVal)
{
  return clamp(x, T(minVal), T(maxVal));
}

//TF_END
#endif

#if defined(D_PLATFORM_DX12)


#if defined( D_TESS_SHADERS_PRESENT )

#define ROOT_FLAGS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),"

#else

#define ROOT_FLAGS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS),"
#define ROOT_FLAGS_GS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS),"

#endif

    #define ROOT_SIG_VS_PS\
        ROOT_FLAGS                                            \
        "CBV(b0, space = 0,visibility=SHADER_VISIBILITY_ALL),"\
        "DescriptorTable( SRV(t0, numDescriptors=25, space = 0), UAV(u0, numDescriptors=8),  visibility=SHADER_VISIBILITY_ALL)," \
        "DescriptorTable( Sampler(s0, numDescriptors=25, space = 0),  visibility=SHADER_VISIBILITY_ALL)"


    #define ROOT_SIG_GS\
        ROOT_FLAGS_GS                                         \
        "CBV(b0, space = 0,visibility=SHADER_VISIBILITY_ALL),"\
        "DescriptorTable( SRV(t0, numDescriptors=25, space = 0), UAV(u0, numDescriptors=8),  visibility=SHADER_VISIBILITY_ALL)," \
        "DescriptorTable( Sampler(s0, numDescriptors=25, space = 0),  visibility=SHADER_VISIBILITY_ALL)"


    #define ROOT_SIG_CS\
        ROOT_FLAGS                                            \
        "CBV(b0, space = 0,visibility=SHADER_VISIBILITY_ALL),"\
        "DescriptorTable( SRV(t0, numDescriptors=25, space = 0),  UAV(u0, numDescriptors=8),  visibility=SHADER_VISIBILITY_ALL)," \
        "DescriptorTable( Sampler(s0, numDescriptors=25, space = 0),  visibility=SHADER_VISIBILITY_ALL), " \
        "StaticSampler(s0,space = 1, filter = FILTER_MIN_MAG_MIP_POINT, " \
                              "addressU = TEXTURE_ADDRESS_CLAMP, " \
                              "addressV = TEXTURE_ADDRESS_CLAMP, " \
                              "addressW = TEXTURE_ADDRESS_CLAMP, " \
                              "comparisonFunc = COMPARISON_NEVER, " \
                              "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK), " \
        "StaticSampler(s1, space = 1, filter = FILTER_MIN_MAG_MIP_LINEAR, " \
                              "addressU = TEXTURE_ADDRESS_CLAMP, " \
                              "addressV = TEXTURE_ADDRESS_CLAMP, " \
                              "addressW = TEXTURE_ADDRESS_CLAMP, " \
                              "comparisonFunc = COMPARISON_NEVER, " \
                              "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK) "  


	#define ROOT_SIG_VS			ROOT_SIG_VS_PS
	#define ROOT_SIG_PS			ROOT_SIG_VS_PS
	//#define ROOT_SIG_GS			ROOT_SIG_VS_PS
	#define ROOT_SIG_HS			ROOT_SIG_VS_PS
	#define ROOT_SIG_DS			ROOT_SIG_VS_PS

    #define S_POSITION  		 SV_Position        
    #define S_TARGET_OUTPUT0     SV_Target0
    #define S_TARGET_OUTPUT1     SV_Target1
    #define S_TARGET_OUTPUT2     SV_Target2
    #define S_TARGET_OUTPUT3     SV_Target3
    #define S_TARGET_OUTPUT4     SV_Target4
    #define S_TARGET_OUTPUT5     SV_Target5
    #define S_TARGET_OUTPUT6     SV_Target6
    #define S_TARGET_OUTPUT7     SV_Target7
    #define S_TARGET_OUTPUT      SV_Target
    #define S_DEPTH_OUTPUT       SV_Depth
    #define S_DEPTH_GE_OUTPUT    SV_DepthGreater
    #define S_DEPTH_LE_OUTPUT    SV_DepthLessEqual
    #define S_FRONT_FACE         SV_IsFrontFace
    #define S_INSTANCE_ID        SV_InstanceID
    #define S_VERTEX_ID          SV_VertexID
    #define S_STENCIL_OP         SV_StencilRef // probably not right but i just wnt it to compile fornow!

    #define  floatToIntBits( _X )  asuint( _X )
    #define  pow( _X, _Y )         pow( max( _X, 0.0 ) , _Y )

#if defined( D_PLATFORM_XBOXONE )
    bool  ballot(uint _X)
    {
        uint2 luVal32Vec2 = __XB_Ballot64(_X);
        return (luVal32Vec2[0] | luVal32Vec2[1]) ? true : false;
    }

    float LaneSwizzle(float x, const uint andMask, const uint orMask, const uint xorMask)
    {
        return __XB_LaneSwizzle(x, ( ( andMask & 0x1f ) | ( orMask << 5 & 0xfe0 ) | ( xorMask << 10 ) ) & 0x7FFF );
    }

    uint __v_cndmask_b32(uint vsrc0, uint vsrc1, unsigned int ssrc1, unsigned int ssrc2)
    {
        return __XB_MBCNT64(uint2(ssrc1, ssrc2));
    }
#endif

#elif defined(D_PLATFORM_METAL)

#define ROOT_SIG_VS			
#define ROOT_SIG_PS			
//#define ROOT_SIG_GS		
#define ROOT_SIG_HS			
#define ROOT_SIG_DS			

#define flat [[flat]]
#define S_POSITION  		 [[position]]     
#define S_RENDER_TARGET_INDEX    
#define S_TARGET_OUTPUT0     [[color(0)]]
#define S_TARGET_OUTPUT1     [[color(1)]]
#define S_TARGET_OUTPUT2     [[color(2)]]
#define S_TARGET_OUTPUT3     [[color(3)]]
#define S_TARGET_OUTPUT4     [[color(4)]]
#define S_TARGET_OUTPUT5     [[color(5)]]
#define S_TARGET_OUTPUT6     [[color(6)]]
#define S_TARGET_OUTPUT7     [[color(7)]]
#define S_TARGET_OUTPUT      [[color(0)]]
#define S_DEPTH_OUTPUT       [[depth(any)]]
#define S_DEPTH_GE_OUTPUT    [[depth(greater)]]
#define S_DEPTH_LE_OUTPUT    [[depth(less)]]
#define S_FRONT_FACE         [[front_facing]]
#define S_INSTANCE_ID        [[instance_id]]
#define S_VERTEX_ID			 [[vertex_id]]
#define S_STENCIL_OP         [[stencil]] // probably not right but i just wnt it to compile fornow!


//TF_BEGIN todo
// ballot ballot(uint _X)
// {
// 	simd_vote activeLaneMask = simd_ballot(expr);
// 	// simd_ballot() returns a 64-bit integer-like object, but
// 	// SPIR-V callers expect a uint4. We must convert.
// 	// FIXME: This won't include higher bits if Apple ever supports
// 	// 128 lanes in an SIMD-group.
// 	return uint4(
// 		(uint)(((simd_vote::vote_t)activeLaneMask) & 0xFFFFFFFF),
// 		(uint)(((simd_vote::vote_t)activeLaneMask >> 32) & 0xFFFFFFFF),
// 		uint2(0));
// }

float LaneSwizzle(float x, const uint andMask, const uint orMask, const uint xorMask)
{
  //TF_BEGIN todo
	return 0;// __XB_LaneSwizzle(x, ((andMask & 0x1f) | (orMask << 5 & 0xfe0) | (xorMask << 10)) & 0x7FFF);
}

uint __v_cndmask_b32(uint vsrc0, uint vsrc1, unsigned int ssrc1, unsigned int ssrc2)
{
  //TF_BEGIN todo
	return 0;//__XB_MBCNT64(uint2(ssrc1, ssrc2));
}
//TF_END
#endif

// =================================================================================================
// Functions
// =================================================================================================

#ifdef D_PLATFORM_GLSL

    #define saturate( V )  clamp(V, 0.0, 1.0)
    #define atan2( Y, X )  atan( Y, X )
    #define invsqrt( X )   inversesqrt( X )
    #define ldexp( _X,_EXP ) (_X * pow(2, _EXP) )
    vec4 f16tof32(uvec4 v) { return vec4(unpackHalf2x16(v.x).x,unpackHalf2x16(v.y).x,unpackHalf2x16(v.z).x,unpackHalf2x16(v.w).x); }

#ifndef D_CAST_FUNTIONS
#define D_CAST_FUNTIONS
    #define asint( X )     floatBitsToInt( X )
    #define asuint( X )    floatBitsToUint( X )
    #define asfloat( X )   intBitsToFloat( X )
    #define asfloatu( X )  uintBitsToFloat( X )
#endif

#ifdef D_COMPUTE

    #define groupID           gl_WorkGroupID
    #define groupThreadID     gl_LocalInvocationID
    #define dispatchThreadID  gl_GlobalInvocationID

    shared float gDerivBufferf[8][8];
    shared vec2  gDerivBuffer2[8][8];
    shared vec3  gDerivBuffer3[8][8];
    shared vec4  gDerivBuffer4[8][8];

    float comp_dFdx( float var ) 
    { 
        gDerivBufferf[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        float delta = var - gDerivBufferf[groupThreadID.x ^ 1][groupThreadID.y]; 
        delta = (groupThreadID.x & 1) == 1? -delta : delta;
        return delta;
    }

    vec2 comp_dFdx( vec2 var ) 
    { 
        gDerivBuffer2[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        vec2 delta = var - gDerivBuffer2[groupThreadID.x ^ 1][groupThreadID.y]; 
        delta = (groupThreadID.x & 1) == 1? -delta : delta;
        return delta;
    }

    vec3 comp_dFdx( vec3 var ) 
    { 
        gDerivBuffer3[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        vec3 delta = var - gDerivBuffer3[groupThreadID.x ^ 1][groupThreadID.y]; 
        delta = (groupThreadID.x & 1) == 1? -delta : delta;
        return delta;
    }

    vec4 comp_dFdx( vec4 var ) 
    { 
        gDerivBuffer4[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        vec4 delta = var - gDerivBuffer4[groupThreadID.x ^ 1][groupThreadID.y]; 
        delta = (groupThreadID.x & 1) == 1? -delta : delta;
        return delta;
    }

    float comp_dFdy( float var ) 
    { 
        gDerivBufferf[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        float delta = var - gDerivBufferf[groupThreadID.x][groupThreadID.y ^ 1]; 
        delta = (groupThreadID.y & 1) == 1? -delta : delta;
        return delta;
    }

    vec2 comp_dFdy( vec2 var ) 
    { 
        gDerivBuffer2[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        vec2 delta = var - gDerivBuffer2[groupThreadID.x][groupThreadID.y ^ 1]; 
        delta = (groupThreadID.y & 1) == 1? -delta : delta;
        return delta;
    }

    vec3 comp_dFdy( vec3 var ) 
    { 
        gDerivBuffer3[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        vec3 delta = var - gDerivBuffer3[groupThreadID.x][groupThreadID.y ^ 1]; 
        delta = (groupThreadID.y & 1) == 1? -delta : delta;
        return delta;
    }

    vec4 comp_dFdy( vec4 var ) 
    { 
        gDerivBuffer4[groupThreadID.x][groupThreadID.y] = var; 
        groupMemoryBarrier(); 
        vec4 delta = var - gDerivBuffer4[groupThreadID.x][groupThreadID.y ^ 1]; 
        delta = (groupThreadID.y & 1) == 1? -delta : delta;
        return delta;
    }

    #define dFdxFine        comp_dFdx
    #define dFdyFine        comp_dFdy
    #define dFdx            comp_dFdx
    #define dFdy            comp_dFdy

#endif

#else
//TF_BEGIN
#ifdef D_PLATFORM_METAL
    #define mix             mix
    #define fract           fract
    #define mod				      fmod
    //#define saturate( V )   ( min( max( (float)V, 0.0f) , 1.0f) )
    #define invsqrt( X )    rsqrt( X )
  

#ifndef D_CAST_FUNTIONS
    #define D_CAST_FUNTIONS
    #define asint( X )     as_int(X)
    #define asuint( X )    as_uint(X)
    #define asfloat( X )   as_float(X)
    #define asfloatu( X )  as_float(X)
#endif

//TF_END
#else
    #define mix             lerp
    #define fract           frac
    #define mod				fmod
    #define saturate( V )   ( min( max( V, 0.0) , 1.0) )
    #define invsqrt( X )    rsqrt( X )
#endif 

#ifndef D_CAST_FUNTIONS
#define D_CAST_FUNTIONS
    #define asfloatu( X )  asfloat( X )
#endif

#if defined(D_COMPUTE) && defined(D_PLATFORM_PROSPERO) && !defined D_COMPUTE_DISABLEWAVE32

    float dFdx( float var ) { float delta = var - LaneSwizzle( var, 0x1f, 0, 1 ); return wave32::__v_cndmask_b32(0, 1, 0xAAAAAAAAU ) ? delta : -delta;  }
    float dFdy( float var ) { float delta = var - LaneSwizzle( var, 0x1f, 0, 8 ); return wave32::__v_cndmask_b32(0, 1, 0xFF00FF00U ) ? delta : -delta;  }

    vec2 dFdx( vec2 var ) { vec2 delta = vec2( var.x - LaneSwizzle( var.x, 0x1f, 0, 1 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 1 ) ); return wave32::__v_cndmask_b32(0, 1, 0xAAAAAAAAU) ? delta : -delta;  }
    vec2 dFdy( vec2 var ) { vec2 delta = vec2( var.x - LaneSwizzle( var.x, 0x1f, 0, 8 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 8 ) ); return wave32::__v_cndmask_b32(0, 1, 0xFF00FF00U) ? delta : -delta;  }

    vec3 dFdx( vec3 var ) { vec3 delta = vec3( var.x - LaneSwizzle( var.x, 0x1f, 0, 1 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 1 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 1 ) ); return wave32::__v_cndmask_b32(0, 1, 0xAAAAAAAAU) ? delta : -delta;  }
    vec3 dFdy( vec3 var ) { vec3 delta = vec3( var.x - LaneSwizzle( var.x, 0x1f, 0, 8 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 8 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 8 ) ); return wave32::__v_cndmask_b32(0, 1, 0xFF00FF00U) ? delta : -delta;  }

    vec4 dFdx( vec4 var ) { vec4 delta = vec4( var.x - LaneSwizzle( var.x, 0x1f, 0, 1 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 1 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 1 ), var.z - LaneSwizzle( var.w, 0x1f, 0, 1 ) ); return wave32::__v_cndmask_b32(0, 1, 0xAAAAAAAAU) ? delta : -delta;  }
    vec4 dFdy( vec4 var ) { vec4 delta = vec4( var.x - LaneSwizzle( var.x, 0x1f, 0, 8 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 8 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 8 ), var.z - LaneSwizzle( var.w, 0x1f, 0, 8 ) ); return wave32::__v_cndmask_b32(0, 1, 0xFF00FF00U) ? delta : -delta;  }


    #define dFdxFine        dFdx
    #define dFdyFine        dFdy

#elif defined(D_COMPUTE) && defined(D_PLATFORM_ORBIS)

    float dFdx( float var ) { float delta = var - LaneSwizzle( var, 0x1f, 0, 1 ); return __v_cndmask_b32(0, 1, 0xAAAAAAAAAAAAAAAAUL ) ? delta : -delta;  }
    float dFdy( float var ) { float delta = var - LaneSwizzle( var, 0x1f, 0, 8 ); return __v_cndmask_b32(0, 1, 0xFF00FF00FF00FF00UL ) ? delta : -delta;  }

    vec2 dFdx( vec2 var ) { vec2 delta = vec2( var.x - LaneSwizzle( var.x, 0x1f, 0, 1 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 1 ) ); return __v_cndmask_b32(0, 1, 0xAAAAAAAAAAAAAAAAUL ) ? delta : -delta;  }
    vec2 dFdy( vec2 var ) { vec2 delta = vec2( var.x - LaneSwizzle( var.x, 0x1f, 0, 8 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 8 ) ); return __v_cndmask_b32(0, 1, 0xFF00FF00FF00FF00UL ) ? delta : -delta;  }

    vec3 dFdx( vec3 var ) { vec3 delta = vec3( var.x - LaneSwizzle( var.x, 0x1f, 0, 1 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 1 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 1 ) ); return __v_cndmask_b32(0, 1, 0xAAAAAAAAAAAAAAAAUL ) ? delta : -delta;  }
    vec3 dFdy( vec3 var ) { vec3 delta = vec3( var.x - LaneSwizzle( var.x, 0x1f, 0, 8 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 8 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 8 ) ); return __v_cndmask_b32(0, 1, 0xFF00FF00FF00FF00UL ) ? delta : -delta;  }

    vec4 dFdx( vec4 var ) { vec4 delta = vec4( var.x - LaneSwizzle( var.x, 0x1f, 0, 1 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 1 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 1 ), var.z - LaneSwizzle( var.w, 0x1f, 0, 1 ) ); return __v_cndmask_b32(0, 1, 0xAAAAAAAAAAAAAAAAUL ) ? delta : -delta;  }
    vec4 dFdy( vec4 var ) { vec4 delta = vec4( var.x - LaneSwizzle( var.x, 0x1f, 0, 8 ), var.y - LaneSwizzle( var.y, 0x1f, 0, 8 ), var.z - LaneSwizzle( var.z, 0x1f, 0, 8 ), var.z - LaneSwizzle( var.w, 0x1f, 0, 8 ) ); return __v_cndmask_b32(0, 1, 0xFF00FF00FF00FF00UL ) ? delta : -delta;  }


    #define dFdxFine        dFdx
    #define dFdyFine        dFdy

#elif defined(D_COMPUTE) && defined(D_PLATFORM_XBOXONE)

#if __XBOX_ENABLE_WAVE32

    float dFdx(float var) { float delta = var - LaneSwizzle(var, 0x1f, 0, 1); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0) ? delta : -delta; }
    float dFdy(float var) { float delta = var - LaneSwizzle(var, 0x1f, 0, 8); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0) ? delta : -delta; }

    vec2 dFdx(vec2 var) { vec2 delta = vec2(var.x - LaneSwizzle(var.x, 0x1f, 0, 1), var.y - LaneSwizzle(var.y, 0x1f, 0, 1)); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0) ? delta : -delta; }
    vec2 dFdy(vec2 var) { vec2 delta = vec2(var.x - LaneSwizzle(var.x, 0x1f, 0, 8), var.y - LaneSwizzle(var.y, 0x1f, 0, 8)); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0) ? delta : -delta; }

    vec3 dFdx(vec3 var) { vec3 delta = vec3(var.x - LaneSwizzle(var.x, 0x1f, 0, 1), var.y - LaneSwizzle(var.y, 0x1f, 0, 1), var.z - LaneSwizzle(var.z, 0x1f, 0, 1)); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0) ? delta : -delta; }
    vec3 dFdy(vec3 var) { vec3 delta = vec3(var.x - LaneSwizzle(var.x, 0x1f, 0, 8), var.y - LaneSwizzle(var.y, 0x1f, 0, 8), var.z - LaneSwizzle(var.z, 0x1f, 0, 8)); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0) ? delta : -delta; }

    vec4 dFdx(vec4 var) { vec4 delta = vec4(var.x - LaneSwizzle(var.x, 0x1f, 0, 1), var.y - LaneSwizzle(var.y, 0x1f, 0, 1), var.z - LaneSwizzle(var.z, 0x1f, 0, 1), var.z - LaneSwizzle(var.w, 0x1f, 0, 1)); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0) ? delta : -delta; }
    vec4 dFdy(vec4 var) { vec4 delta = vec4(var.x - LaneSwizzle(var.x, 0x1f, 0, 8), var.y - LaneSwizzle(var.y, 0x1f, 0, 8), var.z - LaneSwizzle(var.z, 0x1f, 0, 8), var.z - LaneSwizzle(var.w, 0x1f, 0, 8)); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0) ? delta : -delta; }

#else

    float dFdx(float var) { float delta = var - LaneSwizzle(var, 0x1f, 0, 1); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0xAAAAAAAA) ? delta : -delta; }
    float dFdy(float var) { float delta = var - LaneSwizzle(var, 0x1f, 0, 8); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0xFF00FF00) ? delta : -delta; }

    vec2 dFdx(vec2 var) { vec2 delta = vec2(var.x - LaneSwizzle(var.x, 0x1f, 0, 1), var.y - LaneSwizzle(var.y, 0x1f, 0, 1)); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0xAAAAAAAA) ? delta : -delta; }
    vec2 dFdy(vec2 var) { vec2 delta = vec2(var.x - LaneSwizzle(var.x, 0x1f, 0, 8), var.y - LaneSwizzle(var.y, 0x1f, 0, 8)); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0xFF00FF00) ? delta : -delta; }

    vec3 dFdx(vec3 var) { vec3 delta = vec3(var.x - LaneSwizzle(var.x, 0x1f, 0, 1), var.y - LaneSwizzle(var.y, 0x1f, 0, 1), var.z - LaneSwizzle(var.z, 0x1f, 0, 1)); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0xAAAAAAAA) ? delta : -delta; }
    vec3 dFdy(vec3 var) { vec3 delta = vec3(var.x - LaneSwizzle(var.x, 0x1f, 0, 8), var.y - LaneSwizzle(var.y, 0x1f, 0, 8), var.z - LaneSwizzle(var.z, 0x1f, 0, 8)); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0xFF00FF00) ? delta : -delta; }

    vec4 dFdx(vec4 var) { vec4 delta = vec4(var.x - LaneSwizzle(var.x, 0x1f, 0, 1), var.y - LaneSwizzle(var.y, 0x1f, 0, 1), var.z - LaneSwizzle(var.z, 0x1f, 0, 1), var.z - LaneSwizzle(var.w, 0x1f, 0, 1)); return __v_cndmask_b32(0, 1, 0xAAAAAAAA, 0xAAAAAAAA) ? delta : -delta; }
    vec4 dFdy(vec4 var) { vec4 delta = vec4(var.x - LaneSwizzle(var.x, 0x1f, 0, 8), var.y - LaneSwizzle(var.y, 0x1f, 0, 8), var.z - LaneSwizzle(var.z, 0x1f, 0, 8), var.z - LaneSwizzle(var.w, 0x1f, 0, 8)); return __v_cndmask_b32(0, 1, 0xFF00FF00, 0xFF00FF00) ? delta : -delta; }

#endif

    #define dFdxFine        dFdx
    #define dFdyFine        dFdy

#elif defined(D_COMPUTE) && defined(D_PLATFORM_DX12)
    float dFdx( float var ) { return 0;  }
    float dFdy( float var ) { return 0;  }

	vec2 dFdx( vec2 var ) { return vec2(0, 0); }
    vec2 dFdy( vec2 var ) { return vec2(0, 0); }

    vec3 dFdx( vec3 var ) { return vec3(0, 0, 0); }
    vec3 dFdy( vec3 var ) { return vec3(0, 0, 0); }

    vec4 dFdx( vec4 var ) { return vec4(0, 0, 0, 0); }
    vec4 dFdy( vec4 var ) { return vec4(0, 0, 0, 0); }

    #define dFdxFine        dFdx
    #define dFdyFine        dFdy

#elif defined(D_COMPUTE) && defined(D_PLATFORM_METAL)
//todo
  float dFdx( float var ) { return 0;  }
  float dFdy( float var ) { return 0;  }

  vec2 dFdx( vec2 var ) { return vec2(0, 0); }
  vec2 dFdy( vec2 var ) { return vec2(0, 0); }

  vec3 dFdx( vec3 var ) { return vec3(0, 0, 0); }
  vec3 dFdy( vec3 var ) { return vec3(0, 0, 0); }

  vec4 dFdx( vec4 var ) { return vec4(0, 0, 0, 0); }
  vec4 dFdy( vec4 var ) { return vec4(0, 0, 0, 0); }
	#define dFdxFine		dFdx
	#define dFdyFine		dFdy
#elif defined(D_PLATFORM_METAL)
	#define dFdx			dfdx
	#define dFdy			dfdy
	#define dFdxFine		dfdx
	#define dFdyFine		dfdy
#else

    #define dFdx            ddx
    #define dFdy            ddy
    #define dFdxFine        ddx_fine
    #define dFdyFine        ddy_fine

#endif

#endif

// =================================================================================================
// Samplers and textures
// =================================================================================================

#ifdef D_PLATFORM_GLSL
    #if (defined( D_USE_UBO ) && !defined( D_PLATFORM_SWITCH )) || \
        (defined( D_PLATFORM_SWITCH ) && defined(D_SWITCH_NO_BINDLESS_SAMPLERS))
        #ifndef D_SAMPLERS_ARE_GLOBAL
            #define D_SAMPLERS_ARE_GLOBAL
        #endif
    #endif

        #if !(defined(D_DOMAIN) || defined(D_HULL))
            #define texture2D( T, C )               texture( T, C )
            #define texture2DOffset( T, C, O )      texture( T, C, O )
            #define texture2DArray(  T, C )         texture( T, C )
            #define texture2DComputeGrad( T, C )    texture( T, C )

            #define texture2DI( T, C )              texture( T, C )
            #define texture2DIOffset( T, C, O )     texture( T, C, O )
            #define texture2DIArray(  T, C )        texture( T, C )
            #define texture2DIComputeGrad( T, C )   texture( T, C )

            #define texture2DU( T, C )              texture( T, C )
            #define texture2DUOffset( T, C, O )     texture( T, C, O )
            #define texture2DUArray(  T, C )        texture( T, C )
            #define texture2DUComputeGrad( T, C )   texture( T, C )
        #else
            #define texture2D( T, C )               textureLod( T, C, 0 )
            #define texture2DOffset( T, C, O )      textureLod( T, C, 0, O )
            #define texture2DArray(  T, C )         textureLod( T, C, 0 )
            #define texture2DComputeGrad(  T, C )   textureLod( T, C, 0 )

            #define texture2DI( T, C )              textureLod( T, C, 0 )
            #define texture2DIOffset( T, C, O )     textureLod( T, C, 0, O )
            #define texture2DIArray(  T, C )        textureLod( T, C, 0 )
            #define texture2DIComputeGrad(  T, C )  textureLod( T, C, 0 )

            #define texture2DU( T, C )              textureLod( T, C, 0 )
            #define texture2DUOffset( T, C, O )     textureLod( T, C, 0, O )
            #define texture2DUArray(  T, C )        textureLod( T, C, 0 )
            #define texture2DUComputeGrad(  T, C )  textureLod( T, C, 0 )
        #endif


        #define texture2DBias( T, C, B )                texture( T, C, B )
        #define texture2DLod( S, UV, LOD )              textureLod( S, UV, LOD )
        #define texture2DLodOffset( S, UV, LOD, O )     textureLodOffset( S, UV, LOD, O )
        #define texture2DArrayLod( S, UV, LOD )         textureLod( S, UV, LOD )
        #define texture3DLod( S, UV, LOD )              textureLod( S, UV, LOD )

        #define texture2DIBias( T, C, B )               texture( T, C, B )
        #define texture2DILod( S, UV, LOD )             textureLod( S, UV, LOD )
        #define texture2DILodOffset( S, UV, LOD, O )    textureLodOffset( S, UV, LOD, O )
        #define texture2DIArrayLod( S, UV, LOD )        textureLod( S, UV, LOD )
        #define texture3DILod( S, UV, LOD )             textureLod( S, UV, LOD )

        #define texture2DUBias( T, C, B )               texture( T, C, B )
        #define texture2DULod( S, UV, LOD )             textureLod( S, UV, LOD )
        #define texture2DULodOffset( S, UV, LOD, O )    textureLodOffset( S, UV, LOD, O )
        #define texture2DUArrayLod( S, UV, LOD )        textureLod( S, UV, LOD )
        #define texture3DULod( S, UV, LOD )             textureLod( S, UV, LOD )

        #define textureLoadF                            texelFetch
        #define textureLoadI                            texelFetch
        #define textureLoadU                            texelFetch
        #define textureLoadMsF                          texelFetch
        #define textureLoadMsI                          texelFetch
        #define textureLoadMsU                          texelFetch
        #define textureLoadArrayF                       texelFetch
        #define textureLoadArrayI                       texelFetch
        #define textureLoadArrayU                       texelFetch

        #define textureLoadOffsetF                      texelFetchOffset
        #define textureLoadOffsetI                      texelFetchOffset
        #define textureLoadOffsetU                      texelFetchOffset
        #define textureLoadOffsetArrayF                 texelFetchOffset
        #define textureLoadOffsetArrayI                 texelFetchOffset
        #define textureLoadOffsetArrayU                 texelFetchOffset

        #define textureGatherRed(    lTex, lSamp )      textureGather( lTex, lSamp, 0 )
        #define textureGatherGreen(  lTex, lSamp )      textureGather( lTex, lSamp, 1 )
        #define textureGatherBlue(   lTex, lSamp )      textureGather( lTex, lSamp, 2 )
        #define textureGatherAlpha(  lTex, lSamp )      textureGather( lTex, lSamp, 3 )

        #define textureIGatherRed(   lTex, lSamp )      textureGather( lTex, lSamp, 0 )
        #define textureIGatherGreen( lTex, lSamp )      textureGather( lTex, lSamp, 1 )
        #define textureIGatherBlue(  lTex, lSamp )      textureGather( lTex, lSamp, 2 )
        #define textureIGatherAlpha( lTex, lSamp )      textureGather( lTex, lSamp, 3 )

        #define textureUGatherRed(   lTex, lSamp )      textureGather( lTex, lSamp, 0 )
        #define textureUGatherGreen( lTex, lSamp )      textureGather( lTex, lSamp, 1 )
        #define textureUGatherBlue(  lTex, lSamp )      textureGather( lTex, lSamp, 2 )
        #define textureUGatherAlpha( lTex, lSamp )      textureGather( lTex, lSamp, 3 )

        #define textureCube( T, C )                     texture( T, C )
        #define textureCubeLod( T, C, N )               textureLod( T, C, N )

        #define shadow2D( S, UV )                       texture( S, UV )

        #define AtomicAdd(_MEM, V)                      atomicAdd(_MEM, V)
        #define AtomicMin(_MEM, V)                      atomicMin(_MEM, V)
        #define AtomicMax(_MEM, V)                      atomicMax(_MEM, V)
        #define AtomicAnd(_MEM, V)                      atomicAnd(_MEM, V)
        #define AtomicOr( _MEM, V)                      atomicOr( _MEM, V)
        #define AtomicXor(_MEM, V)                      atomicXor(_MEM, V)

        #define AtomicAddOut(_MEM, V, O)                O = atomicAdd(_MEM, V)
        #define AtomicMinOut(_MEM, V, O)                O = atomicMin(_MEM, V)
        #define AtomicMaxOut(_MEM, V, O)                O = atomicMax(_MEM, V)
        #define AtomicAndOut(_MEM, V, O)                O = atomicAnd(_MEM, V)
        #define AtomicOrOut (_MEM, V, O)                O = atomicOr( _MEM, V)
        #define AtomicXorOut(_MEM, V, O)                O = atomicXor(_MEM, V)

        #define imageAtomicAddOut(  T, C, V, O )        O = imageAtomicAdd( T, C, V )
        #define imageAtomicMinOut(  T, C, V, O )        O = imageAtomicMin( T, C, V )
        #define imageAtomicMaxOut(  T, C, V, O )        O = imageAtomicMax( T, C, V )
        #define imageAtomicAndOut(  T, C, V, O )        O = imageAtomicAnd( T, C, V )
        #define imageAtomicOrOut(   T, C, V, O )        O = imageAtomicOr(  T, C, V )
        #define imageAtomicXorOut(  T, C, V, O )        O = imageAtomicXor( T, C, V )

        #define imageAtomicExchangeOut( T, C, V, O )    O = imageAtomicExchange( T, C, V )
        #define imageAtomicCompSwapOut( T, C, I, V, O ) O = imageAtomicCompSwap( T, C, I, V )

        #define IMAGE_GETMAP( _S, _V )          _V
        #define IMAGE_GETLOCAL( _S, _V )        _V

    #if defined( D_TEXTURE_FEEDBACK )

        #define texture2D( T, C )               Tex2dFeedback( T, T##FB, C )

        #define texture2DLod( T, C, N )         Tex2dLodFeedback( T, T##FB, C, N )  
        #define texture2DArray(  T, C )         Tex2dArrayFeedback( T, T##FB, C )

        #define texture3D( S, UV )              Tex3dFeedback( S, S##FB, UV )
        #define texture3DLod( S, UV, LOD )      Tex3dLodFeedback( S, S##FB, UV, LOD )

        #define SAMPLER2DARG( NAME )            in sampler2D NAME, in int NAME##FB
        #define SAMPLER2DPARAM( NAME )          NAME, NAME##FB

        #define SAMPLER2DARRAYARG( NAME )       in sampler2DArray NAME, in int NAME##FB
        #define SAMPLER2DARRAYPARAM( NAME )     NAME, NAME##FB

        #define SAMPLERCUBEARG( NAME )          in samplerCube NAME, in int NAME##FB
        #define SAMPLERCUBEPARAM( NAME )        NAME, NAME##FB

        #define SAMPLER2D( NAME )               sampler2D       NAME; int NAME##FB
        #define SAMPLER2DARRAY( NAME )          sampler2DArray  NAME; int NAME##FB
        #define SAMPLER3D( NAME )               sampler3D       NAME; int NAME##FB
        #define SAMPLER2DSHADOW( NAME )         sampler2DShadow NAME; int NAME##FB

    #else

        #define SAMPLER2DARG(  NAME )           in  sampler2D NAME
        #define SAMPLER2DIARG( NAME )           in isampler2D NAME
        #define SAMPLER2DUARG( NAME )           in usampler2D NAME

        #define SAMPLER2DMSARG(  NAME )         in  sampler2DMS NAME
        #define SAMPLER2DMSIARG( NAME )         in isampler2DMS NAME
        #define SAMPLER2DMSUARG( NAME )         in usampler2DMS NAME

        #define SAMPLER3DARG(  NAME )           in  sampler3D NAME
        #define SAMPLER3DIARG( NAME )           in isampler3D NAME
        #define SAMPLER3DUARG( NAME )           in usampler3D NAME

        #define SAMPLERCUBEARG(  NAME )         in  samplerCube NAME
        #define SAMPLERCUBEIARG( NAME )         in isamplerCube NAME
        #define SAMPLERCUBEUARG( NAME )         in usamplerCube NAME

        #define SAMPLER2DARRAYARG(  NAME )      in  sampler2DArray NAME
        #define SAMPLER2DARRAYIARG( NAME )      in isampler2DArray NAME
        #define SAMPLER2DARRAYUARG( NAME )      in usampler2DArray NAME

        #define SAMPLER2DSHADOWARG( NAME )      in sampler2DShadow NAME

        #define SAMPLERCUBEPARAM( NAME )        NAME

        #define SAMPLER2DPARAM( NAME )          NAME
        #define SAMPLER2DPARAM_SRT( S, NAME )   SAMPLER_GETMAP( S, NAME )

        #define SAMPLERCUBEPARAM( NAME )        NAME
        #define SAMPLERCUBEPARAM_SRT( S, NAME ) SAMPLER_GETMAP( S, NAME )

        #define SAMPLER3DPARAM( NAME )          NAME
        #define SAMPLER3DPARAM_SRT( S, NAME )   SAMPLER_GETMAP( S, NAME )

        #define SAMPLER3DPARAM( NAME )          NAME
        #define SAMPLER3DPARAM_SRT( S, NAME )   SAMPLER_GETMAP( S, NAME )

        #define SAMPLER2DARRAYPARAM( NAME )         NAME
        #define SAMPLER2DARRAYPARAM_SRT( S, NAME )  SAMPLER_GETMAP( S, NAME )

        #define RWIMAGE2DARG( TYPE, NAME )           image2D NAME
        #define RWINTIMAGE2DARG( TYPE, NAME )       iimage2D NAME
        #define RWUINTIMAGE2DARG( TYPE, NAME )      uimage2D NAME

        #if defined ( D_SAMPLERS_ARE_GLOBAL )

            #define BEGIN_SAMPLERBLOCK                  };
            #define END_SAMPLERBLOCK
            #define BEGIN_IMAGEBLOCK
            #define END_IMAGEBLOCK
            #define SAMPLER_GETMAP( _S, _V )            _V
            #define SAMPLER_GETLOCAL( _S, _V )          _V
            #define SAMPLER2DPARAM_LOCAL( S, NAME )     NAME

            #if defined( D_PLATFORM_VULKAN )
            #define LAYOUT(   S )                       layout(set=S)
            #define LAYOUT_B( S, B )                    layout(set=S, binding=B)
            #define LAYOUT_T( S, T )                    layout(set=S, binding=B)
            #else
            #define LAYOUT( S )
            #define LAYOUT_B( S, B )                    layout(binding=B)
            #define LAYOUT_T( S, T )                    layout(T)
            #endif

            #define SAMPLER2D(  NAME )                  LAYOUT(1)           uniform  sampler2D       NAME
            #define SAMPLER2DI( NAME )                  LAYOUT(1)           uniform isampler2D       NAME
            #define SAMPLER2DU( NAME )                  LAYOUT(1)           uniform usampler2D       NAME

            #define SAMPLER2DMS(  NAME )                LAYOUT(1)           uniform  sampler2DMS     NAME
            #define SAMPLER2DMSI( NAME )                LAYOUT(1)           uniform isampler2DMS     NAME
            #define SAMPLER2DMSU( NAME )                LAYOUT(1)           uniform usampler2DMS     NAME

            #define SAMPLER3D(  NAME )                  LAYOUT(1)           uniform  sampler3D       NAME
            #define SAMPLER3DI( NAME )                  LAYOUT(1)           uniform isampler3D       NAME
            #define SAMPLER3DU( NAME )                  LAYOUT(1)           uniform usampler3D       NAME

            #define SAMPLERCUBE(  NAME )                LAYOUT(1)           uniform  samplerCube     NAME
            #define SAMPLERCUBEI( NAME )                LAYOUT(1)           uniform isamplerCube     NAME
            #define SAMPLERCUBEU( NAME )                LAYOUT(1)           uniform usamplerCube     NAME

            #define SAMPLER2DARRAY(  NAME )             LAYOUT(1)           uniform  sampler2DArray  NAME
            #define SAMPLER2DIARRAY( NAME )             LAYOUT(1)           uniform isampler2DArray  NAME
            #define SAMPLER2DUARRAY( NAME )             LAYOUT(1)           uniform usampler2DArray  NAME

            #define SAMPLER2DSHADOW( NAME )             LAYOUT(1)           uniform  sampler2DShadow NAME

            #define SAMPLER2DREG(  NAME, REG )          LAYOUT_B(1, REG)    uniform  sampler2D       NAME
            #define SAMPLER2DIREG( NAME, REG )          LAYOUT_B(1, REG)    uniform isampler2D       NAME
            #define SAMPLER2DUREG( NAME, REG )          LAYOUT_B(1, REG)    uniform usampler2D       NAME

            #define SAMPLER2DMSREG(  NAME, REG )        LAYOUT_B(1, REG)    uniform  sampler2DMS     NAME
            #define SAMPLER2DMSIREG( NAME, REG )        LAYOUT_B(1, REG)    uniform isampler2DMS     NAME
            #define SAMPLER2DMSUREG( NAME, REG )        LAYOUT_B(1, REG)    uniform usampler2DMS     NAME

            #define SAMPLER3DREG(  NAME, REG )          LAYOUT_B(1, REG)    uniform  sampler3D       NAME
            #define SAMPLER3DIREG( NAME, REG )          LAYOUT_B(1, REG)    uniform isampler3D       NAME
            #define SAMPLER3DUREG( NAME, REG )          LAYOUT_B(1, REG)    uniform usampler3D       NAME

            #define SAMPLERCUBEREG(  NAME, REG )        LAYOUT_B(1, REG)    uniform  samplerCube     NAME
            #define SAMPLERCUBEIREG( NAME, REG )        LAYOUT_B(1, REG)    uniform isamplerCube     NAME
            #define SAMPLERCUBEUREG( NAME, REG )        LAYOUT_B(1, REG)    uniform usamplerCube     NAME

            #define SAMPLER2DARRAYREG(  NAME, REG )     LAYOUT_B(1, REG)    uniform  sampler2DArray  NAME
            #define SAMPLER2DIARRAYREG( NAME, REG )     LAYOUT_B(1, REG)    uniform isampler2DArray  NAME
            #define SAMPLER2DUARRAYREG( NAME, REG )     LAYOUT_B(1, REG)    uniform usampler2DArray  NAME

            #define SAMPLER2DSHADOWREG( NAME, REG )     LAYOUT_B(1, REG)    uniform  sampler2DShadow NAME
        #else
            #define BEGIN_SAMPLERBLOCK
            #define END_SAMPLERBLOCK                    };
            #define BEGIN_IMAGEBLOCK                    };
            #define END_IMAGEBLOCK
            #define SAMPLER_GETMAP( _S, _V )            _S._V
            #define SAMPLER_GETLOCAL( _S, _V )          _S._V
            #define SAMPLER2DPARAM_LOCAL( S, NAME )     S.NAME

            #define SAMPLER2D(  NAME )                   sampler2D       NAME
            #define SAMPLER2DI( NAME )                  isampler2D       NAME
            #define SAMPLER2DU( NAME )                  usampler2D       NAME

            #define SAMPLER2DMS(  NAME )                 sampler2DMS     NAME
            #define SAMPLER2DMSI( NAME )                isampler2DMS     NAME
            #define SAMPLER2DMSU( NAME )                usampler2DMS     NAME

            #define SAMPLER3D(  NAME )                   sampler3D       NAME
            #define SAMPLER3DI( NAME )                  isampler3D       NAME
            #define SAMPLER3DU( NAME )                  usampler3D       NAME

            #define SAMPLERCUBE(  NAME )                 samplerCube     NAME
            #define SAMPLERCUBEI( NAME )                isamplerCube     NAME
            #define SAMPLERCUBEU( NAME )                usamplerCube     NAME

            #define SAMPLER2DARRAY(  NAME )              sampler2DArray  NAME
            #define SAMPLER2DIARRAY( NAME )             isampler2DArray  NAME
            #define SAMPLER2DUARRAY( NAME )             usampler2DArray  NAME

            #define SAMPLER2DSHADOW( NAME )              sampler2DShadow NAME

            #define SAMPLER2DREG(  NAME, REG )           sampler2D       NAME
            #define SAMPLER2DIREG( NAME, REG )          isampler2D       NAME
            #define SAMPLER2DUREG( NAME, REG )          usampler2D       NAME

            #define SAMPLER2DMSREG(  NAME, REG )         sampler2DMS     NAME
            #define SAMPLER2DMSIREG( NAME, REG )        isampler2DMS     NAME
            #define SAMPLER2DMSUREG( NAME, REG )        usampler2DMS     NAME

            #define SAMPLER3DREG(  NAME, REG )           sampler3D       NAME
            #define SAMPLER3DIREG( NAME, REG )          isampler3D       NAME
            #define SAMPLER3DUREG( NAME, REG )          usampler3D       NAME

            #define SAMPLERCUBEREG(  NAME, REG )         samplerCube     NAME
            #define SAMPLERCUBEIREG( NAME, REG )        isamplerCube     NAME
            #define SAMPLERCUBEUREG( NAME, REG )        usamplerCube     NAME

            #define SAMPLER2DARRAYREG(  NAME, REG )      sampler2DArray  NAME
            #define SAMPLER2DIARRAYREG( NAME, REG )     isampler2DArray  NAME
            #define SAMPLER2DUARRAYREG( NAME, REG )     usampler2DArray  NAME

            #define SAMPLER2DSHADOWREG( NAME, REG )      sampler2DShadow NAME

        #endif // defined ( D_PLATFORM_SWITCH )

        #if !defined ( D_PLATFORM_SWITCH )
            #define RWIMAGE2D( TYPE, NAME )             uniform layout( set=2, TYPE )  image2D NAME
            #define RWINTIMAGE2D( TYPE, NAME )          uniform layout( set=2, TYPE ) iimage2D NAME
            #define RWUINTIMAGE2D( TYPE, NAME )         uniform layout( set=2, TYPE ) uimage2D NAME
            #define DATABUFFER( TYPE, NAME, REG )       layout( binding = REG, set=3, std430 ) buffer NAME ## _dec  { TYPE data[]; } NAME
            #define RW_DATABUFFER( TYPE, NAME, REG )    DATABUFFER( TYPE, NAME ,REG )
            #define REGULARBUFFER( TYPE, NAME, REG )    layout( binding = REG, set=3, std430 ) buffer NAME ## _dec  { TYPE data[]; } NAME
            #define RW_REGULARBUFFER( TYPE, NAME, REG ) REGULARBUFFER( TYPE, NAME,REG )
            #define GETBUFFERDATA( S, NAME, IDX )       NAME.data[IDX]
        #else
            #if defined(D_SWITCH_NO_BINDLESS_SAMPLERS)
            #define RWIMAGE2D( TYPE, NAME )             layout(TYPE)                  image2D NAME
            #define RWINTIMAGE2D( TYPE, NAME )          layout(TYPE)                 iimage2D NAME
            #define RWUINTIMAGE2D( TYPE, NAME )         layout(TYPE)                 uimage2D NAME
            #else
            #define RWIMAGE2D( TYPE, NAME )             layout(bindless_image, TYPE)  image2D NAME
            #define RWINTIMAGE2D( TYPE, NAME )          layout(bindless_image, TYPE) iimage2D NAME
            #define RWUINTIMAGE2D( TYPE, NAME )         layout(bindless_image, TYPE) uimage2D NAME
            #endif
            #define DATABUFFER( TYPE, NAME, REG )       layout( std430, binding = REG ) buffer NAME ## _dec  { TYPE NAME ## []; }
            #define RW_DATABUFFER( TYPE, NAME, REG )    DATABUFFER( TYPE, NAME ,REG )
            #define REGULARBUFFER( TYPE, NAME, REG )    layout( std430, binding = REG ) buffer NAME ## _dec  { TYPE NAME ## []; }
            #define RW_REGULARBUFFER( TYPE, NAME, REG ) REGULARBUFFER( TYPE, NAME,REG )
            #define GETBUFFERDATA( S, NAME, IDX )       NAME ## [IDX]
        #endif // !defined ( D_PLATFORM_SWITCH )


    #endif // defined( D_TEXTURE_FEEDBACK )




//!D_PLATFORM_GLSL
#else

#if defined(D_PLATFORM_METAL)

inline float4 as_float(uint4 X) { return as_type<float4>(X); }
inline float3 as_float(uint3 X) { return as_type<float3>(X); }
inline float2 as_float(uint2 X) { return as_type<float2>(X); }
inline float as_float(uint X) { return as_type<float>(X); }
inline uint as_uint(float X) { return as_type<uint>(X); }
inline uint2 as_uint(float2 X) { return as_type<uint2>(X); }
inline uint3 as_uint(float3 X) { return as_type<uint3>(X); }
inline uint4 as_uint(float4 X) { return as_type<uint4>(X); }
inline int as_int(float X) { return as_type<int>(X); }
inline int2 as_int(float2 X) { return as_type<int2>(X); }
inline int3 as_int(float3 X) { return as_type<int3>(X); }
inline int4 as_int(float4 X) { return as_type<int4>(X); }

#define bitfieldExtract extract_bits
#define bitfieldInsert insert_bits
#define floatToIntBits( _X )  as_int( _X )
#define f16tof32(_X)  as_float(_X)
#define floatBitsToUint as_uint
#define uintBitsToFloat as_float
#define ReverseBits reverse_bits

#define METAL_half   half
#define METAL_float  float
#define METAL_float2 float
#define METAL_float3 float
#define METAL_float4 float
#define METAL_uint   uint
#define METAL_uint2  uint
#define METAL_uint3  uint
#define METAL_uint4  uint
#define METAL_int    int
#define METAL_int2   int
#define METAL_int3   int
#define METAL_int4   int
#define METAL_r32ui  uint
#define METAL_r16ui  uint16_t
#define METAL_T(ELEM_TYPE) METAL_##ELEM_TYPE

#define BEGIN_SAMPLERBLOCK
#define END_SAMPLERBLOCK				};
#define BEGIN_IMAGEBLOCK                
#define END_IMAGEBLOCK                  };
#define SAMPLER_GETMAP( _S, _V )		_S._V
#define SAMPLER_GETLOCAL( _S, _V )      _S._V
#define IMAGE_GETMAP( _S, _V )          _V
//#define IMAGE_GETMAP( _S, _V )        _S._V
#define IMAGE_GETLOCAL( _S, _V )        _S._V
#define SAMPLER2DPARAM_LOCAL( STRUCTURE, NAME )   SAMPLER2DPARAM( STRUCTURE.NAME )
#define TEX_SS_STRINGIFY(T) T##SS
#define TEX_MIP_LOD_BIAS lfMipLodBias

#if defined(D_DOMAIN)
#define texture2D( T, C )               T.sample( TEX_SS_STRINGIFY(T), C, 0)
#define texture2DOffset( T, C, O )      T.sample( TEX_SS_STRINGIFY(T), C, 0, O)
#define texture2DArray(  T, C )         T.sample( TEX_SS_STRINGIFY(T), C.xy, C.z)
#define texture2DArrayLod(  T, C, L )   T.sample( TEX_SS_STRINGIFY(T), C.xy, C.z, level(L))
#define texture2DComputeGrad(  T, C )   T.sample( TEX_SS_STRINGIFY(T), C, 0)
#elif defined(D_COMPUTE)
#define texture2D( T, C )		            T.sample( TEX_SS_STRINGIFY(T), C, 0)
#define texture2DOffset( T, C, O )      T.sample( TEX_SS_STRINGIFY(T), C, 0, O)
#define texture2DArray(  T, C )         T.sample( TEX_SS_STRINGIFY(T), C.xy, C.z)
#define textureCube( T, C )		          T.sample( TEX_SS_STRINGIFY(T), C )
#define texture2DComputeGrad( T, C )    T.sample( TEX_SS_STRINGIFY(T), C, gradient2d(vec2( dFdx( C ) ), vec2( dFdy( C ) )) )
#define shadow2D( T, C )                float4(T.sample_compare(TEX_SS_STRINGIFY(T), C.xy, C.z, level(0)))
#else
#define texture2D( T, C )		            T.sample( TEX_SS_STRINGIFY(T), C, bias(TEX_MIP_LOD_BIAS) )
#define texture2DOffset( T, C, O )      T.sample( TEX_SS_STRINGIFY(T), C, bias(TEX_MIP_LOD_BIAS), O )
#define texture2DBias( T, C, B )        T.sample( TEX_SS_STRINGIFY(T), C, bias(TEX_MIP_LOD_BIAS + B) )
#define texture2DArray(  T, C )         T.sample( TEX_SS_STRINGIFY(T), C.xy, C.z, bias(TEX_MIP_LOD_BIAS) )
#define texture2DArrayLod(  T, C, L )   T.sample( TEX_SS_STRINGIFY(T), C.xy, C.z, level(L) )
#define texture2DArrayGrad(  T, C, DDX, DDY ) T.sample( TEX_SS_STRINGIFY(T), C.xy, C.z, gradient2d(DDX, DDY) )
#define texture2DComputeGrad( T, C )    T.sample( TEX_SS_STRINGIFY(T), C )
#define shadow2D( T, C )                float4(T.sample_compare(TEX_SS_STRINGIFY(T), C.xy, C.z, level(0) ))
#if defined(D_VERTEX)    
#define textureCube( T, C )		          T.sample( TEX_SS_STRINGIFY(T), C, level(0) )
#else
#define textureCube( T, C )		          T.sample( TEX_SS_STRINGIFY(T), C )
#endif
#endif

#define texture2DLod( T, C, N )             T.sample(TEX_SS_STRINGIFY(T), C, level(N) )
#define texture2DLodOffset( T, C, N, O )    T.sample(TEX_SS_STRINGIFY(T), C, level(N), O )
#define texture3DLod( T, C, N )             T.sample(TEX_SS_STRINGIFY(T), C, level(N) )
#define textureCubeLod( T, C, N )	        T.sample(TEX_SS_STRINGIFY(T), C, level(N) )
#define textureGrad( T, C, DDX, DDY )       T.sample(TEX_SS_STRINGIFY(T), C, gradient2d(DDX, DDY) )
#define texture2DMS( T, UV, S )             texelFetch( T, UV, S )

#define textureGatherRed( T, C )            T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::x )
#define textureGatherGreen( T, C )          T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::y )
#define textureGatherBlue( T, C )           T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::z )
#define textureGatherAlpha( T, C )          T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::w )

#define textureIGatherRed( T, C )           T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::x )
#define textureIGatherGreen( T, C )         T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::y )
#define textureIGatherBlue( T, C )          T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::z )
#define textureIGatherAlpha( T, C )         T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::w )

#define textureUGatherRed( T, C )           T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::x )
#define textureUGatherGreen( T, C )         T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::y )
#define textureUGatherBlue( T, C )          T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::z )
#define textureUGatherAlpha( T, C )         T.gather( TEX_SS_STRINGIFY(T), C, int2(0), component::w )

#define IMAGE2D_INPUT(TYPE, NAME, ACCESS, NUM)     RW_Texture2D< METAL_T(TYPE), ACCESS > NAME [[texture(NUM)]];
#define RWIMAGE2D_INPUT( TYPE, NAME, NUM )          IMAGE2D_INPUT(TYPE, NAME, access::read_write, NUM)

#define RWIMAGE2D( TYPE, NAME )           RW_Texture2D< METAL_T(TYPE), access::read_write > NAME; sampler NAME##SS
#define RWINTIMAGE2D( TYPE, NAME )        RW_Texture2D< METAL_T(TYPE), access::read_write> NAME; sampler NAME##SS
#define RWUINTIMAGE2D( TYPE, NAME )       RW_Texture2D< METAL_T(TYPE), access::read_write> NAME; sampler NAME##SS

#define RW_DATABUFFER( TYPE, NAME, IDX )      device TYPE * NAME [[buffer(IDX)]]
#define DATABUFFER( TYPE, NAME, IDX )         const RW_DATABUFFER( TYPE, NAME, IDX )
#define RW_REGULARBUFFER( TYPE, NAME, IDX )   device TYPE * NAME [[buffer(IDX)]]
#define REGULARBUFFER( TYPE, NAME, IDX )      const RW_REGULARBUFFER( TYPE, NAME, IDX )

#define GETBUFFERDATA( S, NAME, IDX )         NAME ## [IDX]

/*
#define DATABUFFER( TYPE, NAME )              constant TYPE * NAME
#define RW_DATABUFFER( TYPE, NAME, IDX )      device TYPE * NAME [[buffer(IDX)]]
#define REGULARBUFFER( TYPE, NAME )           constant TYPE * NAME
#define RW_REGULARBUFFER( TYPE, NAME, IDX )   device TYPE * NAME [[buffer(IDX)]]
*/

int min(int x, float y)
{
  return min(x, int(y));
}
uint min(uint x, float y)
{
  return min(x, uint(y));
}
float normalize(float x)
{
  return 1;
}


template<typename T, typename V>
void SampleReadAtomicAdd(T tex, int2 coords, V value, thread V& output)
{
  output = tex.read((uint2)coords);
  tex.write(output+value, (uint2)coords);
}

template<typename T, typename V>
void SampleReadAtomicAdd(T tex, int4 coords, V value, thread V& output)
{
  output = tex.read((uint4)coords);
  tex.write(output+value, (uint4)coords);
}

template<typename T>
void SampleReadAtomicAdd(T tex, int2 coords, int value, thread int& output)
{
  output = tex.read((uint2)coords).x;
  tex.write(output+value, (uint2)coords);
}

template<typename T>
void SampleReadAtomicAdd(T tex, int4 coords, int value, thread int& output)
{
  output = tex.read((uint4)coords).x;
  tex.write(output+value, (uint4)coords);
}

#define imageAtomicAdd( T, C, V )         T.write( T.read(uint2(C)) + V, uint2(C) )
#define imageAtomicMin(  T, C, V, O )     T.write( min( T.read(uint2(C)), V ), uint2(C) )
#define imageAtomicMax(  T, C, V, O )     T.write( max( T.read(uint2(C)), V ), uint2(C) )

// #define imageAtomicAddOut( T, C, V, O )   O = T.read(uint2(C)); T.write(O + V, uint2(C))
#define imageAtomicAddOut( T, C, V, O )   SampleReadAtomicAdd(T, C, V, O)
#define imageLoad( T, C )				          T.read(uint2(C))
#define imageStore( T, C, V )             T.write(V, uint2(C))

//returns previous value
#define atomicAdd(B, V)        atomic_fetch_add_explicit(B, V, memory_order::memory_order_relaxed)
#define atomicAddOut(B, V, O)  O = atomic_fetch_add_explicit(B, V, memory_order::memory_order_relaxed)
#define atomicLoad( B )				 atomic_load_explicit(B, memory_order::memory_order_relaxed)
#define atomicStore( B, V )     atomic_store_explicit(B, V, memory_order::memory_order_relaxed)

#define bufferAtomicAdd(B, C, V)        atomic_fetch_add_explicit(&B[C], V, memory_order::memory_order_relaxed)
#define bufferAtomicAddOut(B, C, V, O)  O = atomic_fetch_add_explicit(&B[C], V, memory_order::memory_order_relaxed)
#define bufferAtomicLoad( B, C )				 atomic_load_explicit(&B[C], memory_order::memory_order_relaxed)
#define bufferAtomicStore( B, C, V )     atomic_store_explicit(&B[C], V, memory_order::memory_order_relaxed)

#define SAMPLER3D( NAME )							  texture3d<float, access::sample> NAME; sampler NAME##SS;
#define SAMPLERCUBE( NAME)							texturecube<float, access::sample> NAME; sampler NAME##SS;
#define SAMPLER3DARG( NAME)							texture3d<float, access::sample> NAME, sampler NAME##SS
#define SAMPLER3DPARAM( NAME )						NAME, TEX_SS_STRINGIFY(NAME)
#define SAMPLER_EVALUATE(S, N) SAMPLER_GETMAP(S,N)
#define SAMPLER3DPARAM_SRT( STRUCTURE, NAME )		SAMPLER3DPARAM(SAMPLER_EVALUATE(STRUCTURE,NAME))

#define SAMPLER2D( NAME )							    texture2d<float, access::sample> NAME; sampler NAME##SS;
#define SAMPLER2DARRAY( NAME )						texture2d_array<float, access::sample> NAME; sampler NAME##SS;
#define SAMPLER2DREG( NAME, REG)					SAMPLER2D(NAME)
#define SAMPLER3DREG( NAME, REG)					SAMPLER3D(NAME)
#define SAMPLER2DARRAYREG( NAME, REG)				SAMPLER2DARRAY(NAME)
#define SAMPLERCUBEREG( NAME, REG)					SAMPLERCUBE(NAME)

//TF_BEGIN
#define SAMPLER2DMS( NAME )             texture2d_ms<float, access::read> NAME; sampler NAME##SS;
#define SAMPLER2DMSREG( NAME, REG )     texture2d_ms<float, access::read> NAME; sampler NAME##SS;
#define SAMPLER2DMSIREG(TYPE, NAME )		texture2d_ms<METAL_T(TYPE), access::read> NAME; sampler NAME##SS;
//TF_END

#define SAMPLER2DI( NAME )              texture2d<int, access::sample> NAME; sampler NAME##SS;
#define SAMPLER2DU( NAME )              texture2d<uint, access::sample> NAME; sampler NAME##SS;

#define SAMPLER2DARG( NAME )						    texture2d<float, access::sample> NAME, sampler NAME##SS
#define SAMPLER2DSHADOWARG( NAME )					depth2d<float, access::sample> NAME, sampler NAME##SS

#define SAMPLER2DPARAM( NAME )						NAME, TEX_SS_STRINGIFY(NAME)
#define SAMPLER2DPARAM_SRT( STRUCTURE, NAME )		SAMPLER2DPARAM(SAMPLER_EVALUATE(STRUCTURE,NAME))
#define SAMPLER2DSHADOW( NAME )					depth2d<float, access::sample> NAME; sampler NAME##SS;

#define SAMPLER2DARRAYARG( NAME )         texture2d_array<float, access::sample> NAME, sampler NAME##SS
#define SAMPLER2DARRAYPARAM( NAME )		  NAME, TEX_SS_STRINGIFY(NAME)
#define SAMPLER2DARRAYPARAM_SRT( STRUCTURE, NAME )	SAMPLER2DPARAM(SAMPLER_EVALUATE(STRUCTURE,NAME))

#define ISAMPLER2D( TYPE, NAME )          texture2d<METAL_T(TYPE), access::sample> NAME; sampler NAME##SS;
#define ISAMPLER2DARG( TYPE, NAME )       texture2d<METAL_T(TYPE), access::sample> NAME, sampler NAME##SS

#define SAMPLERCUBEARG( NAME )            texturecube<float, access::sample> NAME, sampler NAME##SS
#define SAMPLERCUBEPARAM( NAME )          NAME, TEX_SS_STRINGIFY(NAME)
#define SAMPLERCUBEPARAM_SRT( STRUCTURE, NAME )  SAMPLERCUBEPARAM(SAMPLER_EVALUATE(STRUCTURE,NAME))

#define RWIMAGE2DARG( TYPE, NAME )        RW_Texture2D< METAL_T(TYPE) > NAME
#define RWINTIMAGE2DARG( TYPE, NAME )     RW_Texture2D< METAL_T(TYPE) > NAME
#define RWIMAGE2DACCESSARG( TYPE, NAME, ACCESS )        RW_Texture2D< METAL_T(TYPE), ACCESS > NAME

#define textureQueryLod( lTex, lCoords )  vec2( lTex.calculate_unclamped_lod( lTex##SS, lCoords ), 0.0 )

#define textureLoadF                    texelFetch
#define textureLoadI                    texelFetch
#define textureLoadU                    texelFetch
#define textureLoadMsF                  texelFetch
#define textureLoadMsI                  texelFetch
#define textureLoadMsU                  texelFetch
#define textureLoadArrayF               texelFetch
#define textureLoadArrayI               texelFetch
#define textureLoadArrayU               texelFetch

#define textureLoadOffsetF              texelFetchOffset
#define textureLoadOffsetI              texelFetchOffset
#define textureLoadOffsetU              texelFetchOffset
#define textureLoadOffsetArrayF         texelFetchOffset
#define textureLoadOffsetArrayI         texelFetchOffset
#define textureLoadOffsetArrayU         texelFetchOffset

#define texelFetch( lTex, lSamp, lLod ) lTex.read(uint2(lSamp.xy), lLod) //needs to be fixed to take into account vec3 as coords.
#define texelFetchOffset( lTex, lSamp, lLod, lOffset ) lTex.read( uint2( lSamp.xy + lOffset.xy ), lLod ) //needs to be fixed to take into account vec3 as coords.

#else //!D_PLATFORM_METAL
#if defined(D_PLATFORM_DX12)
    #ifndef D_SAMPLERS_ARE_GLOBAL
      #define D_SAMPLERS_ARE_GLOBAL
    #endif

  	#define BEGIN_SAMPLERBLOCK  };
  	#define END_SAMPLERBLOCK
    #define BEGIN_IMAGEBLOCK           
    #define END_IMAGEBLOCK
    #define SAMPLER_GETMAP(_S, _V)          _V
    #define SAMPLER_GETLOCAL( _S, _V )      _V
    #define IMAGE_GETMAP(_S, _V)            _V
    #define IMAGE_GETLOCAL( _S, _V )        _V
    #define SAMPLER2DPARAM_LOCAL( S, NAME ) SAMPLER2DPARAM( NAME )
#else
    #define BEGIN_SAMPLERBLOCK
    #define END_SAMPLERBLOCK                };
    #define BEGIN_IMAGEBLOCK                
    #define END_IMAGEBLOCK                  };
    #define SAMPLER_GETMAP( _S, _V )        _S._V
    #define SAMPLER_GETLOCAL( _S, _V )      _S._V
    #define IMAGE_GETMAP( _S, _V )          _S._V
    #define IMAGE_GETLOCAL( _S, _V )        _S._V
    #define SAMPLER2DPARAM_LOCAL( S, NAME ) SAMPLER2DPARAM( S.NAME )
#endif

    #if defined(D_DOMAIN)
        #define texture2D( T, C )               T.SampleLOD( T##SS, C, 0)
        #define texture2DOffset( T, C, O )      T.SampleLOD( T##SS, C, 0, O)
        #define texture2DArray(  T, C )         T.SampleLOD( T##SS, C, 0)
        #define texture2DArrayLod(  T, C, L )   T.SampleLOD( T##SS, C, L)
        #define texture2DComputeGrad(  T, C )   T.SampleLOD( T##SS, C, 0)

        #define texture2DI( T, C )              T.SampleLOD( T##SS, C, 0)
        #define texture2DIOffset( T, C, O )     T.SampleLOD( T##SS, C, 0, O)
        #define texture2DIArray(  T, C )        T.SampleLOD( T##SS, C, 0)
        #define texture2DIArrayLod(  T, C, L )  T.SampleLOD( T##SS, C, L)
        #define texture2DIComputeGrad(  T, C )  T.SampleLOD( T##SS, C, 0)

        #define texture2DU( T, C )              T.SampleLOD( T##SS, C, 0)
        #define texture2DUOffset( T, C, O )     T.SampleLOD( T##SS, C, 0, O)
        #define texture2DUArray(  T, C )        T.SampleLOD( T##SS, C, 0)
        #define texture2DUArrayLod(  T, C, L )  T.SampleLOD( T##SS, C, L )
        #define texture2DUComputeGrad(  T, C )  T.SampleLOD( T##SS, C, 0)

    #elif defined(D_COMPUTE)
        #define texture2D( T, C )               T.SampleLOD( T##SS, C, 0)
        #define texture2DOffset( T, C, O )      T.SampleLOD( T##SS, C, 0, O)
        #define texture2DArray(  T, C )         T.SampleLOD( T##SS, C, 0)
        #define texture2DComputeGrad( T, C )    T.SampleGradient(T##SS, C, vec2( dFdx( C ) ), vec2( dFdy( C ) ) )

        #define texture2DI( T, C )              T.SampleLOD( T##SS, C, 0)
        #define texture2DIOffset( T, C, O )     T.SampleLOD( T##SS, C, 0, O)
        #define texture2DIArray(  T, C )        T.SampleLOD( T##SS, C, 0)
        #define texture2DIComputeGrad( T, C )   T.SampleGradient(T##SS, C, vec2( dFdx( C ) ), vec2( dFdy( C ) ) )

        #define texture2DU( T, C )              T.SampleLOD( T##SS, C, 0)
        #define texture2DUOffset( T, C, O )     T.SampleLOD( T##SS, C, 0, O)
        #define texture2DUArray(  T, C )        T.SampleLOD( T##SS, C, 0)
        #define texture2DUComputeGrad( T, C )   T.SampleGradient(T##SS, C, vec2( dFdx( C ) ), vec2( dFdy( C ) ) )

        #define shadow2D( T, C )                T.SampleCmpLOD0(T##SS, C.xy, C.z)
        #define textureCube( T, C )             T.SampleLOD( T##SS, C )
    #else
        #define texture2D( T, C )                  T.Sample( T##SS, C )
        #define texture2DOffset( T, C, O )         T.Sample( T##SS, C, O )
        #define texture2DBias( T, C, B )           T.SampleBias( T##SS, C, B )
        #define texture2DArray(  T, C )            T.Sample( T##SS, C )
        #define texture2DArrayLod(  T, C, L )      T.SampleLOD( T##SS, C, L )
        #define texture2DComputeGrad( T, C )       T.Sample(T##SS, C )

        #define texture2DI( T, C )                 T.Sample( T##SS, C )
        #define texture2DIOffset( T, C, O )        T.Sample( T##SS, C, O )
        #define texture2DIBias( T, C, B )          T.SampleBias( T##SS, C, B )
        #define texture2DIArray(  T, C )           T.Sample( T##SS, C )
        #define texture2DIArrayLod(  T, C, L )     T.SampleLOD( T##SS, C, L )
        #define texture2DIComputeGrad( T, C )      T.Sample(T##SS, C )

        #define texture2DU( T, C )                 T.Sample( T##SS, C )
        #define texture2DUOffset( T, C, O )        T.Sample( T##SS, C, O )
        #define texture2DUBias( T, C, B )          T.SampleBias( T##SS, C, B )
        #define texture2DUArray(  T, C )           T.Sample( T##SS, C )
        #define texture2DUArrayLod(  T, C, L )     T.SampleLOD( T##SS, C, L )
        #define texture2DUComputeGrad( T, C )      T.Sample(T##SS, C )

        #define shadow2D( T, C )                   T.SampleCmp(T##SS, C.xy, C.z )
        #if defined(D_VERTEX)
          #define textureCube( T, C )              T.SampleLOD( T##SS, C, 0 )
        #else
          #define textureCube( T, C )              T.Sample( T##SS, C )
        #endif
    #endif

    #define texture2DLod( T, C, N )           T.SampleLOD( T##SS, C, N )
    #define texture2DLodOffset( T, C, N, O )  T.SampleLOD( T##SS, C, N, O )
    #define texture3DLod( T, C, N )           T.SampleLOD( T##SS, C, N )

    #define texture2DILod( T, C, N )          T.SampleLOD( T##SS, C, N )
    #define texture2DILodOffset( T, C, N, O ) T.SampleLOD( T##SS, C, N, O )
    #define texture3DILod( T, C, N )          T.SampleLOD( T##SS, C, N )

    #define texture2DULod( T, C, N )          T.SampleLOD( T##SS, C, N )
    #define texture2DULodOffset( T, C, N, O ) T.SampleLOD( T##SS, C, N, O )
    #define texture3DULod( T, C, N )          T.SampleLOD( T##SS, C, N )

    #define textureCubeLod( T, C, N )         T.SampleLOD( T##SS, C, N )
    #define textureGrad( T, C, DDX, DDY )     T.SampleGradient(T##SS, C, DDX, DDY )

    #define textureGatherRed(    lTex, lSamp ) lTex.GatherRed  ( lTex##SS, lSamp )
    #define textureGatherGreen(  lTex, lSamp ) lTex.GatherGreen( lTex##SS, lSamp )
    #define textureGatherBlue(   lTex, lSamp ) lTex.GatherBlue ( lTex##SS, lSamp )
    #define textureGatherAlpha(  lTex, lSamp ) lTex.GatherAlpha( lTex##SS, lSamp )

    #define textureIGatherRed(   lTex, lSamp ) lTex.GatherRed  ( lTex##SS, lSamp )
    #define textureIGatherGreen( lTex, lSamp ) lTex.GatherGreen( lTex##SS, lSamp )
    #define textureIGatherBlue(  lTex, lSamp ) lTex.GatherBlue ( lTex##SS, lSamp )
    #define textureIGatherAlpha( lTex, lSamp ) lTex.GatherAlpha( lTex##SS, lSamp )

    #define textureUGatherRed(   lTex, lSamp ) lTex.GatherRed  ( lTex##SS, lSamp )
    #define textureUGatherGreen( lTex, lSamp ) lTex.GatherGreen( lTex##SS, lSamp )
    #define textureUGatherBlue(  lTex, lSamp ) lTex.GatherBlue ( lTex##SS, lSamp )
    #define textureUGatherAlpha( lTex, lSamp ) lTex.GatherAlpha( lTex##SS, lSamp )

    #define RWIMAGE2D( TYPE, NAME )           RW_Texture2D< TYPE > NAME; SamplerState NAME##SS
    #define RWINTIMAGE2D( TYPE, NAME )        RW_Texture2D< TYPE > NAME; SamplerState NAME##SS
    #define RWUINTIMAGE2D( TYPE, NAME )       RW_Texture2D< TYPE > NAME; SamplerState NAME##SS
    
    #define imageLoad( T, C )                       ( T[C] )
    #define imageStore( T, C, V )                     T[C] = V

    #ifdef D_PLATFORM_ORBIS

    #define DATABUFFER( TYPE, NAME,REG )            DataBuffer< TYPE >          NAME
    #define RW_DATABUFFER( TYPE, NAME, REG )        RW_DataBuffer< TYPE >       NAME
    #define REGULARBUFFER( TYPE, NAME, REG )        RegularBuffer< TYPE >       NAME
    #define RW_REGULARBUFFER( TYPE, NAME,REG )      RW_RegularBuffer< TYPE >    NAME

    #define AtomicAddOut( T,V, O )                  AtomicAdd( T, V, O )
    #define AtomicMinOut( T,V, O )                  AtomicMin( T, V, O )
    #define AtomicMaxOut( T,V, O )                  AtomicMax( T, V, O )
    #define AtomicAndOut( T,V, O )                  AtomicAnd( T, V, O )
    #define AtomicOrOut(  T,V, O )                  AtomicOr(  T, V, O )
    #define AtomicXorOut( T,V, O )                  AtomicXor( T, V, O )

    #define imageAtomicAdd( T, C, V )               AtomicAdd( T[ C ], V )
    #define imageAtomicMin( T, C, V )               AtomicMin( T[ C ], V )
    #define imageAtomicMax( T, C, V )               AtomicMax( T[ C ], V )
    #define imageAtomicAnd( T, C, V )               AtomicAnd( T[ C ], V )
    #define imageAtomicOr(  T, C, V )               AtomicOr(  T[ C ], V )
    #define imageAtomicXor( T, C, V )               AtomicXor( T[ C ], V )

    #define imageAtomicAddOut( T, C, V, O )         AtomicAdd( T[ C ], V, O )
    #define imageAtomicMinOut( T, C, V, O )         AtomicMin( T[ C ], V, O )
    #define imageAtomicMaxOut( T, C, V, O )         AtomicMax( T[ C ], V, O )
    #define imageAtomicAndOut( T, C, V, O )         AtomicAnd( T[ C ], V, O )
    #define imageAtomicOrOut(  T, C, V, O )         AtomicOr(  T[ C ], V, O )
    #define imageAtomicXorOut( T, C, V, O )         AtomicXor( T[ C ], V, O )

    #define imageAtomicExchangeOut( T, C, V, O )    AtomicExchange(    T[ C ], V, O )
    #define imageAtomicCompSwapOut( T, C, I, V, O ) AtomicCmpExchange( T[ C ], I, V, O )
    #else
    
    #define DATABUFFER( TYPE, NAME,REG )            DataBuffer< TYPE >          NAME : register( t2##REG )
    #define RW_DATABUFFER( TYPE, NAME, REG )        RW_DataBuffer< TYPE >       NAME : register( u##REG )
    #define REGULARBUFFER( TYPE, NAME, REG )        RegularBuffer< TYPE >       NAME : register( t2##REG )
    #define RW_REGULARBUFFER( TYPE, NAME,REG )      RW_RegularBuffer< TYPE >    NAME : register( u##REG )

    #define AtomicAdd(_MEM, _DATA)                  InterlockedAdd(_MEM, _DATA)
    #define AtomicMin(_MEM, _DATA)                  InterlockedMin(_MEM, _DATA)
    #define AtomicMax(_MEM, _DATA)                  InterlockedMax(_MEM, _DATA)
    #define AtomicAnd(_MEM, _DATA)                  InterlockedAnd(_MEM, _DATA)
    #define AtomicOr( _MEM, _DATA)                  InterlockedOr( _MEM, _DATA)
    #define AtomicXor(_MEM, _DATA)                  InterlockedXor(_MEM, _DATA)

    #define AtomicAddOut(_MEM, _DATA, _ORIG)        InterlockedAdd(_MEM, _DATA, _ORIG)
    #define AtomicMinOut(_MEM, _DATA, _ORIG)        InterlockedMin(_MEM, _DATA, _ORIG)
    #define AtomicMaxOut(_MEM, _DATA, _ORIG)        InterlockedMax(_MEM, _DATA, _ORIG)
    #define AtomicAndOut(_MEM, _DATA, _ORIG)        InterlockedAnd(_MEM, _DATA, _ORIG)
    #define AtomicOrOut (_MEM, _DATA, _ORIG)        InterlockedOr( _MEM, _DATA, _ORIG)
    #define AtomicXorOut(_MEM, _DATA, _ORIG)        InterlockedXor(_MEM, _DATA, _ORIG)

    #define imageAtomicAdd( T, C, V )               InterlockedAdd( T[ C ], V )
    #define imageAtomicMin( T, C, V )               InterlockedMin( T[ C ], V )
    #define imageAtomicMax( T, C, V )               InterlockedMax( T[ C ], V )
    #define imageAtomicAnd( T, C, V )               InterlockedAnd( T[ C ], V )
    #define imageAtomicOr(  T, C, V )               InterlockedOr(  T[ C ], V )
    #define imageAtomicXor( T, C, V )               InterlockedXor( T[ C ], V )

    #define imageAtomicAddOut( T, C, V, O )         InterlockedAdd( T[ C ], V, O )
    #define imageAtomicAddOut( T, C, V, O )         InterlockedAdd( T[ C ], V, O )
    #define imageAtomicMinOut( T, C, V, O )         InterlockedMin( T[ C ], V, O )
    #define imageAtomicMaxOut( T, C, V, O )         InterlockedMax( T[ C ], V, O )
    #define imageAtomicAndOut( T, C, V, O )         InterlockedAnd( T[ C ], V, O )
    #define imageAtomicOrOut(  T, C, V, O )         InterlockedOr(  T[ C ], V, O )
    #define imageAtomicXorOut( T, C, V, O )         InterlockedXor( T[ C ], V, O )

    #define imageAtomicExchangeOut( T, C, V, O )    InterlockedExchange(         T[ C ], V, O )
    #define imageAtomicCompSwapOut( T, C, I, V, O ) InterlockedCompareExchange ( T[ C ], I, V, O )
    #endif

    #ifdef D_PLATFORM_ORBIS
    #define GETBUFFERDATA( S, NAME, IDX )   S.NAME ## [IDX]
    #else
    #define GETBUFFERDATA( S, NAME, IDX )   NAME ## [IDX]
    #endif

    #define SAMPLER2D(  NAME )              Texture2D               NAME; SamplerState NAME##SS
    #define SAMPLER2DI( NAME )              Texture2D<int4>         NAME; SamplerState NAME##SS
    #define SAMPLER2DU( NAME )              Texture2D<uint4>        NAME; SamplerState NAME##SS

    #if defined( D_PLATFORM_ORBIS )
    #define SAMPLER2DMS(  NAME )            MS_Texture2D<float4>    NAME; SamplerState NAME##SS
    #define SAMPLER2DMSI( NAME )            MS_Texture2D<int4>      NAME; SamplerState NAME##SS
    #define SAMPLER2DMSU( NAME )            MS_Texture2D<uint4>     NAME; SamplerState NAME##SS
    #else
    #define SAMPLER2DMS(  NAME )            Texture2DMS<float4>     NAME; SamplerState NAME##SS
    #define SAMPLER2DMSI( NAME )            Texture2DMS<int4>       NAME; SamplerState NAME##SS
    #define SAMPLER2DMSU( NAME )            Texture2DMS<uint4>      NAME; SamplerState NAME##SS
    #endif

    #define SAMPLER3D(  NAME )              Texture3D               NAME; SamplerState NAME##SS
    #define SAMPLER3DI( NAME )              Texture3D<int4>         NAME; SamplerState NAME##SS
    #define SAMPLER3DU( NAME )              Texture3D<uint4>        NAME; SamplerState NAME##SS

    #define SAMPLERCUBE(  NAME )            TextureCube             NAME; SamplerState NAME##SS
    #define SAMPLERCUBEI( NAME )            TextureCube<int4>       NAME; SamplerState NAME##SS
    #define SAMPLERCUBEU( NAME )            TextureCube<uint4>      NAME; SamplerState NAME##SS

    #define SAMPLER2DARRAY(  NAME )         Texture2D_Array         NAME; SamplerState NAME##SS
    #define SAMPLER2DARRAYI( NAME )         Texture2D_Array<int4>   NAME; SamplerState NAME##SS
    #define SAMPLER2DARRAYU( NAME )         Texture2D_Array<uint4>  NAME; SamplerState NAME##SS

    #define SAMPLER2DARG(  NAME )           Texture2D               NAME, SamplerState NAME##SS
    #define SAMPLER2DIARG( NAME )           Texture2D<int4>         NAME, SamplerState NAME##SS
    #define SAMPLER2DUARG( NAME )           Texture2D<uint4>        NAME, SamplerState NAME##SS

    #define SAMPLERCUBEARG(  NAME )         TextureCube             NAME, SamplerState NAME##SS
    #define SAMPLERCUBEIARG( NAME )         TextureCube<int4>       NAME, SamplerState NAME##SS
    #define SAMPLERCUBEUARG( NAME )         TextureCube<uint4>      NAME, SamplerState NAME##SS

    #define SAMPLER3DARG(  NAME )           Texture3D               NAME, SamplerState NAME##SS
    #define SAMPLER3DIARG( NAME )           Texture3D<int4>         NAME, SamplerState NAME##SS
    #define SAMPLER3DUARG( NAME )           Texture3D<uint4>        NAME, SamplerState NAME##SS

    #define SAMPLER2DARRAYARG( NAME )       Texture2D_Array         NAME, SamplerState NAME##SS
    #define SAMPLER2DARRAYIARG( NAME )      Texture2D_Array<int4>   NAME, SamplerState NAME##SS
    #define SAMPLER2DARRAYUARG( NAME )      Texture2D_Array<uint4>  NAME, SamplerState NAME##SS

    #define SAMPLER2DSHADOWARG( NAME )      Texture2D               NAME, SamplerComparisonState NAME##SS
    #define SAMPLER2DSHADOW( NAME )         Texture2D               NAME; SamplerComparisonState NAME##SS

    #define SAMPLER2DPARAM( NAME )              NAME, NAME##SS
    #define SAMPLER2DPARAM_SRT( S, NAME )       SAMPLER2DPARAM( SAMPLER_GETMAP( S, NAME ) )

    #define SAMPLER3DPARAM( NAME )              NAME, NAME##SS
    #define SAMPLER3DPARAM_SRT( S, NAME )       SAMPLER3DPARAM( SAMPLER_GETMAP( S, NAME ) )

    #define SAMPLERCUBEPARAM( NAME )            NAME, NAME##SS
    #define SAMPLERCUBEPARAM_SRT( S, NAME )     SAMPLERCUBEPARAM( SAMPLER_GETMAP( S, NAME ) )

    #define SAMPLER2DARRAYPARAM( NAME )         NAME, NAME##SS
    #define SAMPLER2DARRAYPARAM_SRT( S, NAME )  SAMPLER2DPARAM( SAMPLER_GETMAP( S, NAME ) )

#if defined(D_PLATFORM_DX12)
    #define SAMPLER2DREG(  NAME, REG )          Texture2D               NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER2DIREG( NAME, REG )          Texture2D<int4>         NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER2DUREG( NAME, REG )          Texture2D<uint4>        NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)

    #define SAMPLER2DMSREG(  NAME, REG )        Texture2DMS             NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER2DMSIREG( NAME, REG )        Texture2DMS<int4>       NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER2DMSUREG( NAME, REG )        Texture2DMS<uint4>      NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)

    #define SAMPLER3DREG(  NAME, REG )          Texture3D               NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER3DIREG( NAME, REG )          Texture3D<int4>         NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER3DUREG( NAME, REG )          Texture3D<uint4>        NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)

    #define SAMPLERCUBEREG(  NAME, REG )        TextureCube             NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLERCUBEIREG( NAME, REG )        TextureCube<int4>       NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLERCUBEUREG( NAME, REG )        TextureCube<uint4>      NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)

    #define SAMPLER2DARRAYREG(  NAME, REG )     Texture2D_Array         NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER2DARRAYIREG( NAME, REG )     Texture2D_Array<int4>   NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)
    #define SAMPLER2DARRAYUREG( NAME, REG )     Texture2D_Array<uint4>  NAME :  register(t##REG); SamplerState NAME##SS : register(s##REG)

#else
    #define SAMPLER2DREG(  NAME, REG )          Texture2D               NAME; SamplerState NAME##SS
    #define SAMPLER2DIREG( NAME, REG )          Texture2D<int4>         NAME; SamplerState NAME##SS
    #define SAMPLER2DUREG( NAME, REG )          Texture2D<uint4>        NAME; SamplerState NAME##SS

    #define SAMPLER2DMSREG(  NAME, REG )        MS_Texture2D            NAME; SamplerState NAME##SS
    #define SAMPLER2DMSIREG( NAME, REG )        MS_Texture2D<int4>      NAME; SamplerState NAME##SS
    #define SAMPLER2DMSUREG( NAME, REG )        MS_Texture2D<uint4>     NAME; SamplerState NAME##SS

    #define SAMPLER3DREG(  NAME, REG )          Texture3D               NAME; SamplerState NAME##SS
    #define SAMPLER3DIREG( NAME, REG )          Texture3D<int4>         NAME; SamplerState NAME##SS
    #define SAMPLER3DUREG( NAME, REG )          Texture3D<uint4>        NAME; SamplerState NAME##SS

    #define SAMPLERCUBEREG(  NAME, REG )        TextureCube             NAME; SamplerState NAME##SS
    #define SAMPLERCUBEIREG( NAME, REG )        TextureCube<int4>       NAME; SamplerState NAME##SS
    #define SAMPLERCUBEUREG( NAME, REG )        TextureCube<uint4>      NAME; SamplerState NAME##SS

    #define SAMPLER2DARRAYREG(  NAME, REG )     Texture2D_Array         NAME; SamplerState NAME##SS
    #define SAMPLER2DARRAYIREG( NAME, REG )     Texture2D_Array<int4>   NAME; SamplerState NAME##SS
    #define SAMPLER2DARRAYUREG( NAME, REG )     Texture2D_Array<uint4>  NAME; SamplerState NAME##SS

#endif

    #define RWIMAGE2DARG( TYPE, NAME )          RW_Texture2D< TYPE > NAME
    #define RWINTIMAGE2DARG( TYPE, NAME )       RW_Texture2D< TYPE > NAME

  #if defined(D_PLATFORM_DX12)
    #define textureQueryLod( lTex, lCoords )                    vec2( lTex.CalculateLevelOfDetailUnclamped( lTex##SS, lCoords ), 0.0 )
    #define textureLoadF( lTex, lSamp, lLod )                   lTex.Load( int3((lSamp), (lLod)) )
    #define textureLoadI( lTex, lSamp, lLod )                   lTex.Load( int3((lSamp), (lLod)) )
    #define textureLoadU( lTex, lSamp, lLod )                   lTex.Load( int3((lSamp), (lLod)) )
    #define textureLoadMsF( lTex, lSamp, lSmp )                 lTex.Load( int3((lSamp), (0)), (lSmp) )
    #define textureLoadMsI( lTex, lSamp, lSmp )                 lTex.Load( int3((lSamp), (0)), (lSmp) )
    #define textureLoadMsU( lTex, lSamp, lSmp )                 lTex.Load( int3((lSamp), (0)), (lSmp) )
    #define textureLoadArrayF( lTex, lSamp, lLod )              lTex.Load( int4((lSamp), (lLod)) )
    #define textureLoadArrayI( lTex, lSamp, lLod )              lTex.Load( int4((lSamp), (lLod)) )
    #define textureLoadArrayU( lTex, lSamp, lLod )              lTex.Load( int4((lSamp), (lLod)) )

    #define textureLoadOffsetF( lTex, lSamp, lLod, lOff )       lTex.Load( int3((lSamp), (lLod)), (lOff) )
    #define textureLoadOffsetI( lTex, lSamp, lLod, lOff )       lTex.Load( int3((lSamp), (lLod)), (lOff) )
    #define textureLoadOffsetU( lTex, lSamp, lLod, lOff )       lTex.Load( int3((lSamp), (lLod)), (lOff) )
    #define textureLoadOffsetArrayF( lTex, lSamp, lLod, lOff )  lTex.Load( int4((lSamp), (lLod)), (lOff) )
    #define textureLoadOffsetArrayI( lTex, lSamp, lLod, lOff )  lTex.Load( int4((lSamp), (lLod)), (lOff) )
    #define textureLoadOffsetArrayU( lTex, lSamp, lLod, lOff )  lTex.Load( int4((lSamp), (lLod)), (lOff) )
  #else
    #define textureQueryLod( lTex, lCoords )                    vec2( lTex.GetLODUnclamped( lTex##SS, lCoords ), 0.0 )
    #define textureLoadF( lTex, lSamp, lLod )                   lTex.MipMaps((lLod), (lSamp))
    #define textureLoadI( lTex, lSamp, lLod )                   lTex.MipMaps((lLod), (lSamp))
    #define textureLoadU( lTex, lSamp, lLod )                   lTex.MipMaps((lLod), (lSamp))
    #define textureLoadMsF( lTex, lSamp, lSmp )                 lTex.Load(  (lSamp), (lSmp) )
    #define textureLoadMsI( lTex, lSamp, lSmp )                 lTex.Load(  (lSamp), (lSmp) )
    #define textureLoadMsU( lTex, lSamp, lSmp )                 lTex.Load(  (lSamp), (lSmp) )
    #define textureLoadArrayF( lTex, lSamp, lLod )              lTex.MipMaps((lLod), (lSamp))
    #define textureLoadArrayI( lTex, lSamp, lLod )              lTex.MipMaps((lLod), (lSamp))
    #define textureLoadArrayU( lTex, lSamp, lLod )              lTex.MipMaps((lLod), (lSamp))

    #define textureLoadOffsetF( lTex, lSamp, lLod, lOff )       lTex.MipMaps((lLod), (lSamp) + (lOff) )
    #define textureLoadOffsetI( lTex, lSamp, lLod, lOff )       lTex.MipMaps((lLod), (lSamp) + (lOff) )
    #define textureLoadOffsetU( lTex, lSamp, lLod, lOff )       lTex.MipMaps((lLod), (lSamp) + (lOff) )
    #define textureLoadOffsetArrayF( lTex, lSamp, lLod, lOff )  lTex.MipMaps((lLod), (lSamp) + (lOff) )
    #define textureLoadOffsetArrayI( lTex, lSamp, lLod, lOff )  lTex.MipMaps((lLod), (lSamp) + (lOff) )
    #define textureLoadOffsetArrayU( lTex, lSamp, lLod, lOff )  lTex.MipMaps((lLod), (lSamp) + (lOff) )
  #endif

#endif // defined ( D_PLATFORM_GLSL )
#endif

// =================================================================================================
// Matrices
// =================================================================================================
#if defined(D_PLATFORM_GLSL) || defined(D_PLATFORM_METAL)

    #define MUL( INPUT_A, INPUT_B )         (INPUT_A * INPUT_B)
    #define PLATFORM_TRANSPOSE
    #define MAT4_SET_POS( M, P )            M[ 3 ] = P
    #define MAT4_SET_TRANSLATION( M, T )    M[ 3 ].xyz = T
    #define MAT4_GET_COLUMN( M, C )         M[ C ].xyz
    #define MAT3_GET_COLUMN( M, C )         M[ C ]
    #define MAT4_GET_COLUMN_VEC4( M, C )    M[ C ]

    #define MAT3_SET_COLUMN( M, C, V )      M[ C ] = V;
    #define MAT4_SET_COLUMN( M, C, V )      M[ C ] = V;

#else

    #define MUL( INPUT_A, INPUT_B )         mul( INPUT_B, INPUT_A )
    #define PLATFORM_TRANSPOSE
    #define MAT4_SET_POS( M, P )            M[ 3 ] = P
    #define MAT4_SET_TRANSLATION( M, T )    M[ 3 ].xyz = T
    #define MAT4_GET_COLUMN( M, C )         M[ C ].xyz
    #define MAT3_GET_COLUMN( M, C )         M[ C ]
    #define MAT4_GET_COLUMN_VEC4( M, C )    M[ C ]

    #define MAT3_SET_COLUMN( M, C, V )      M[ C ] = V;
    #define MAT4_SET_COLUMN( M, C, V )      M[ C ] = V;

#endif

// =================================================================================================
// Arrays (workaround AMD shader compiler issues by making arrays have global scope)
// =================================================================================================

#if defined(D_PLATFORM_GLSL)

#ifdef D_UNIFORMS_ARE_GLOBAL
#define ARRAY_LOOKUP_FS( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   lUniforms._BUFFER._ELEMENT[_INDEX]
#define ARRAY_LOOKUP_FP( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   lUniforms._BUFFER._ELEMENT[_INDEX]
//#elif defined( D_ARRAYS_ARE_GLOBAL )
//#define ARRAY_LOOKUP_FS( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _ELEMENT[_INDEX]
//#define ARRAY_LOOKUP_FP( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _ELEMENT[_INDEX]
#else
#define ARRAY_LOOKUP_FS( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]
#define ARRAY_LOOKUP_FP( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]
#endif

#define BUFFER_LOOKUP_FS( _UNIFORMS, _ELEMENT, _INDEX ) _ELEMENT[_INDEX]
#define BUFFER_LOOKUP_FP( _UNIFORMS, _ELEMENT, _INDEX ) _ELEMENT[_INDEX]

#elif defined(D_PLATFORM_DX12)

#define ARRAY_LOOKUP_FS( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]
#define ARRAY_LOOKUP_FP( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]

#define BUFFER_LOOKUP_FS( _UNIFORMS, _ELEMENT, _INDEX ) _ELEMENT[_INDEX]
#define BUFFER_LOOKUP_FP( _UNIFORMS, _ELEMENT, _INDEX ) _ELEMENT[_INDEX]

#elif defined(D_PLATFORM_ORBIS)

#define ARRAY_LOOKUP_FS( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]
#define ARRAY_LOOKUP_FP( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]

#define BUFFER_LOOKUP_FS( _UNIFORMS, _ELEMENT, _INDEX ) _UNIFORMS._ELEMENT[_INDEX]
#define BUFFER_LOOKUP_FP( _UNIFORMS, _ELEMENT, _INDEX ) _UNIFORMS._ELEMENT[_INDEX]

#elif defined(D_PLATFORM_METAL)

#define ARRAY_LOOKUP_FS( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]
#define ARRAY_LOOKUP_FP( _UNIFORMS, _BUFFER, _ELEMENT, _INDEX)   _UNIFORMS._ELEMENT[_INDEX]

#define BUFFER_LOOKUP_FS( _UNIFORMS, _ELEMENT, _INDEX ) _UNIFORMS._ELEMENT[_INDEX]
#define BUFFER_LOOKUP_FP( _UNIFORMS, _ELEMENT, _INDEX ) _UNIFORMS._ELEMENT[_INDEX]

#endif


// =================================================================================================
// Input and Output
// =================================================================================================

//interpolation:
//centroid    flat    smooth    noperspective 

#if defined( D_PLATFORM_PC_COMPUTE )

    #define DECLARE_INPUT
    #define DECLARE_INPUT_END

    #define flat
    #define smooth

    #define DECLARE_UNIFORMS                struct UniformBuffer {
    #define DECLARE_PTR( TYPE, NAME )       TYPE NAME;
    #define UNIFORM( INST, VAR )            lUniforms.INST.VAR

    #define DEREF_UNIFORM( VAR )            lUniforms.VAR

  #ifdef D_UNIFORMS_ARE_GLOBAL
    #define DECLARE_UNIFORMS_END            }; uniform lUniforms_BLK { UniformBuffer lUniforms; };
    #define DECLARE_PACKED_UNIFORMS_END     }; uniform lUniforms_BLK { UniformBuffer lUniforms; };
  #else
    #define DECLARE_UNIFORMS_END            };
    #define DECLARE_PACKED_UNIFORMS_END     };
  #endif

    #define INPUT_SCREEN_POSITION
    #define INPUT_SCREEN_SLICE
    #define INPUT(   TYPE, NAME, REG )              
    #define INPUT_NOINTERP(   TYPE, NAME, REG )     
    #define INPUT_NOPERSP(   TYPE, NAME, REG )     

    #define IN_SCREEN_POSITION              

    #define WRITE_FRAGMENT_COLOUR( VAL )   imageStore( gOutTexture0, ivec2( dispatchThreadID ).xy, VAL ) 
    #define WRITE_FRAGMENT_COLOUR0( VAL )  imageStore( gOutTexture0, ivec2( dispatchThreadID ).xy, VAL ) 
    #define WRITE_FRAGMENT_COLOUR1( VAL )  imageStore( gOutTexture1, ivec2( dispatchThreadID ).xy, VAL ) 
    #define WRITE_FRAGMENT_COLOUR2( VAL )  imageStore( gOutTexture2, ivec2( dispatchThreadID ).xy, VAL ) 
    #define WRITE_FRAGMENT_COLOUR3( VAL )  imageStore( gOutTexture3, ivec2( dispatchThreadID ).xy, VAL ) 
    #define WRITE_FRAGMENT_COLOUR4( VAL )  imageStore( gOutTexture4, ivec2( dispatchThreadID ).xy, VAL ) 
    #define WRITE_FRAGMENT_DEPTH( VAL )    imageStore( gOutTextureDepth, ivec2( dispatchThreadID ).xy, vec4( VAL ) ) 
    #define WRITE_FRAGMENT_STENCIL( VAL )  imageStore( gOutTextureStencil, ivec2( dispatchThreadID ).xy, vec4( VAL ) ) 


    #define FRAGMENT_COLOUR   imageLoad( SAMPLER_GETMAP( mpCmpOutPerMesh, gOutTexture0 ), ivec2( dispatchThreadID ).xy ) 
    #define FRAGMENT_COLOUR0  imageLoad( SAMPLER_GETMAP( mpCmpOutPerMesh, gOutTexture0 ), ivec2( dispatchThreadID ).xy ) 
    #define FRAGMENT_COLOUR1  imageLoad( SAMPLER_GETMAP( mpCmpOutPerMesh, gOutTexture1 ), ivec2( dispatchThreadID ).xy ) 
    #define FRAGMENT_COLOUR2  imageLoad( SAMPLER_GETMAP( mpCmpOutPerMesh, gOutTexture2 ), ivec2( dispatchThreadID ).xy ) 
    #define FRAGMENT_COLOUR3  imageLoad( SAMPLER_GETMAP( mpCmpOutPerMesh, gOutTexture3 ), ivec2( dispatchThreadID ).xy ) 
    #define FRAGMENT_COLOUR4  imageLoad( SAMPLER_GETMAP( mpCmpOutPerMesh, gOutTexture4 ), ivec2( dispatchThreadID ).xy ) 

    #define DEREF_PTR( VAR )                VAR

#elif defined ( D_PLATFORM_SWITCH_COMPUTE )

#define DECLARE_INPUT                  
#define DECLARE_INPUT_END                

#define flat            
#define smooth      

#define DECLARE_PTR( TYPE, NAME )       TYPE NAME;

#define UNIFORM( INST, VAR )            lUniforms.INST.VAR
#define DEREF_UNIFORM( VAR )            lUniforms.VAR

#define DECLARE_UNIFORMS                struct UniformBuffer {
#define DECLARE_UNIFORMS_END            }; uniform lUniforms_BLK { UniformBuffer lUniforms; };
#define DECLARE_PACKED_UNIFORMS_END     }; uniform lUniforms_BLK { UniformBuffer lUniforms; };
#define INPUT_SCREEN_POSITION
#define INPUT_SCREEN_SLICE
#define INPUT(   TYPE, NAME, REG )              
#define INPUT_NOINTERP(   TYPE, NAME, REG )     
#define INPUT_NOPERSP(   TYPE, NAME, REG )  

#define IN_SCREEN_POSITION              

#define WRITE_FRAGMENT_COLOUR( VAL )   imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), ivec2( dispatchThreadID ).xy, VAL ) 
#define WRITE_FRAGMENT_COLOUR0( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), ivec2( dispatchThreadID ).xy, VAL ) 
#define WRITE_FRAGMENT_COLOUR1( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture1 ), ivec2( dispatchThreadID ).xy, VAL ) 
#define WRITE_FRAGMENT_COLOUR2( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture2 ), ivec2( dispatchThreadID ).xy, VAL ) 
#define WRITE_FRAGMENT_COLOUR3( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture3 ), ivec2( dispatchThreadID ).xy, VAL ) 
#define WRITE_FRAGMENT_COLOUR4( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture4 ), ivec2( dispatchThreadID ).xy, VAL ) 
#define WRITE_FRAGMENT_DEPTH( VAL )    imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTextureDepth ), ivec2( dispatchThreadID ).xy, vec4( VAL ) ) 
#define WRITE_FRAGMENT_STENCIL( VAL )  

#define FRAGMENT_COLOUR   imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), ivec2( dispatchThreadID ).xy ) 
#define FRAGMENT_COLOUR0  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), ivec2( dispatchThreadID ).xy ) 
#define FRAGMENT_COLOUR1  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture1 ), ivec2( dispatchThreadID ).xy ) 
#define FRAGMENT_COLOUR2  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture2 ), ivec2( dispatchThreadID ).xy ) 
#define FRAGMENT_COLOUR3  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture3 ), ivec2( dispatchThreadID ).xy ) 
#define FRAGMENT_COLOUR4  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture4 ), ivec2( dispatchThreadID ).xy ) 

#define DEREF_PTR( VAR )                VAR

#elif defined(D_PLATFORM_METAL)
//TF_BEGIN
    #define DECLARE_OUTPUT                  struct cOutput {
    #define DECLARE_INPUT                   struct cInput  {
    #define DECLARE_INPUT_END               };
    #define DECLARE_OUTPUT_END              };

    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
    #define INPUT_SCREEN_POSITION_REDECLARED

    #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
    #define OUTPUT_SCREEN_POSITION_REDECLARED

    #define DECLARE_UNIFORMS                struct UniformBuffer {
    #define DECLARE_PTR( TYPE, NAME )       constant TYPE * NAME;

  #ifdef D_UNIFORMS_ARE_GLOBAL
    #define DECLARE_UNIFORMS_END            }; lUniforms_BLK { UniformBuffer lUniforms; };
  #else
    #define DECLARE_UNIFORMS_END            };
  #endif

    #define D_LIGHT_CLUSTER_BUFFER_ID       9
    #define D_PER_INSTANCE_BUFFER_ID        10
    #define D_PER_INSTANCE_CULL_BUFFER_ID   11
    #define D_BILATERAL_REJECT_MAP_BUFFER_ID    8
    #define D_DEPTH_REPRJ_FRWD_BUFFER_ID        8
    #define D_DEPTH_REPRJ_BKWD_BUFFER_ID        8
    #define D_CB_MASK_BUFFER_ID                 8
    #define D_DEPTH_LINEAR_BUFFER_ID            8

    #define UNIFORM( INST, VAR )            lUniforms.INST.VAR
    #define DEREF_UNIFORM( VAR )            lUniforms.VAR
    #define DECLARE_PATCH_INPUT_TRI
    #define DECLARE_PATCH_OUTPUT_TRI
    #define DECLARE_PATCH_INPUT_END
    #define DECLARE_PATCH_OUTPUT_END
    #define IN_PATCH_TRI_TESS_CONSTANTS
    #define OUT_PATCH_TRI_TESS_CONSTANTS
    #define DECLARE_PATCH_INPUT_QUAD
    #define DECLARE_PATCH_OUTPUT_QUAD
    #define IN_PATCH_QUAD_TESS_CONSTANTS
    #define OUT_PATCH_QUAD_TESS_CONSTANTS

    #define REG_POSITION    position
    #define POSITION0    attribute(0)
    #define TEXCOORD0    attribute(1)
    #define COLOR 
    #define NORMAL0      attribute(2)
    #define NORMAL       attribute(2)
    #define TANGENT0     attribute(3)
    #define TANGENT      attribute(4)
    #define BLENDINDICES attribute(5)
    #define BLENDWEIGHT  attribute(6)
    #define TEXCOORD1    attribute(7)
    #define TEXCOORD2    attribute(8)
    #define TEXCOORD3    attribute(9)
    #define TEXCOORD4    attribute(10)
    #define TEXCOORD5    attribute(11)
    #define TEXCOORD6    attribute(12)
    #define TEXCOORD7    attribute(13)
    #define TEXCOORD8    attribute(14)
    #define TEXCOORD9    attribute(15)
    #define TEXCOORD10   attribute(16)
    #define TEXCOORD11   attribute(17)
    #define TEXCOORD12   attribute(18)
    #define TEXCOORD13   attribute(19)
    #define TEXCOORD14   attribute(20)
    #define TEXCOORD15   attribute(21)
    #define TEXCOORD16   attribute(22)
    #define TEXCOORD17   attribute(23)
    #define TEXCOORD18   attribute(24)
    #define TEXCOORD19   attribute(25)
    #define TEXCOORD20   attribute(26)
    #define TEXCOORD21   attribute(27)
    #define TEXCOORD22   attribute(28)
    #define TEXCOORD23   attribute(29)

    #define INPUT_NOPERSP(    TYPE, NAME, REG )               TYPE NAME [[REG]] [[center_no_perspective]];

    #if defined(D_COMPUTE)
      #define INPUT(            TYPE, NAME, REG )               TYPE NAME;
      #define OUTPUT(           TYPE, NAME, REG )               TYPE NAME;
    #else
      #define INPUT(            TYPE, NAME, REG )               TYPE NAME [[REG]];
      #define OUTPUT(           TYPE, NAME, REG )               TYPE NAME [[REG]];
      #define INPUT_VARIANT(    TYPE, NAME, REG, VARIANT )      TYPE NAME [[REG, function_constant(VARIANT)]];
      #define OUTPUT_VARIANT(   TYPE, NAME, REG, VARIANT )      TYPE NAME [[REG, function_constant(VARIANT)]];
    #endif

    #define AtomicAdd(_MEM, V)                      atomic_fetch_add_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicMin(_MEM, V)                      atomic_fetch_min_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicMax(_MEM, V)                      atomic_fetch_max_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicAnd(_MEM, V)                      atomic_fetch_and_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicOr(_MEM, V)                       atomic_fetch_or_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicXor(_MEM, V)                      atomic_fetch_xor_explicit( &_MEM, V, memory_order::memory_order_relaxed )

    #define AtomicAddOut(_MEM, V, O)                O = atomic_fetch_add_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicMinOut(_MEM, V, O)                O = atomic_fetch_min_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicMaxOut(_MEM, V, O)                O = atomic_fetch_max_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicAndOut(_MEM, V, O)                O = atomic_fetch_and_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicOr(_MEM, V, O)                    O = atomic_fetch_or_explicit( &_MEM, V, memory_order::memory_order_relaxed )
    #define AtomicXorOut(_MEM, V, O)                O = atomic_fetch_xor_explicit( &_MEM, V, memory_order::memory_order_relaxed )

#if defined(D_COMPUTE)
  #define FRAGMENT_COLOUR_UVEC4           mColour0
  #define FRAGMENT_COLOUR_UVEC4_DEFINE    uvec4 mColour0 [[texture(0)]]
  #define FRAGMENT_COLOUR_VEC4_DEFINE     RWIMAGE2D_NOSS(rgba32f, gOutTexture0) [[texture(0)]]
	#define FRAGMENT_COLOUR1_VEC4_DEFINE    RWIMAGE2D_NOSS(rgba32f, gOutTexture1) [[texture(1)]]
	#define FRAGMENT_COLOUR2_VEC4_DEFINE    RWIMAGE2D_NOSS(rgba32f, gOutTexture2) [[texture(2)]]
	#define FRAGMENT_COLOUR3_VEC4_DEFINE    RWIMAGE2D_NOSS(rgba32f, gOutTexture3) [[texture(3)]]
	#define FRAGMENT_COLOUR4_VEC4_DEFINE    RWIMAGE2D_NOSS(rgba32f, gOutTexture4) [[texture(4)]]
	#define FRAGMENT_COLOUR5_VEC4_DEFINE    RWIMAGE2D_NOSS(rgba32f, gOutTexture5) [[texture(5)]]
	#define FRAGMENT_DEPTH_VEC4_DEFINE      RWIMAGE2D_NOSS(rgba32f, gOutTextureDepth) [[texture(6)]]
	#define FRAGMENT_STENCIL_VEC4_DEFINE    RWIMAGE2D_NOSS(rgba32f, gOutTextureStencil) [[texture(7)]]
  

  #define WRITE_FRAGMENT_COLOUR( VAL )   imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), uvec2( dispatchThreadID.xy ), VAL ) 
  #define WRITE_FRAGMENT_COLOUR0( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), uvec2( dispatchThreadID.xy ), VAL ) 
  #define WRITE_FRAGMENT_COLOUR1( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture1 ), uvec2( dispatchThreadID.xy ), VAL ) 
  #define WRITE_FRAGMENT_COLOUR2( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture2 ), uvec2( dispatchThreadID.xy ), VAL ) 
  #define WRITE_FRAGMENT_COLOUR3( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture3 ), uvec2( dispatchThreadID.xy ), VAL ) 
  #define WRITE_FRAGMENT_COLOUR4( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture4 ), uvec2( dispatchThreadID.xy ), VAL ) 
  #define WRITE_FRAGMENT_DEPTH( VAL )    imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTextureDepth), uvec2( dispatchThreadID.xy ), vec4( VAL ) ) 
  #define WRITE_FRAGMENT_STENCIL( VAL )  imageStore( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTextureStencil), uvec2( dispatchThreadID.xy ) , uvec4( VAL ) ) 


  #define FRAGMENT_COLOUR   imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), uvec2( dispatchThreadID.xy ) ) 
  #define FRAGMENT_COLOUR0  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture0 ), uvec2( dispatchThreadID.xy ) ) 
  #define FRAGMENT_COLOUR1  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture1 ), uvec2( dispatchThreadID.xy ) ) 
  #define FRAGMENT_COLOUR2  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture2 ), uvec2( dispatchThreadID.xy ) ) 
  #define FRAGMENT_COLOUR3  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture3 ), uvec2( dispatchThreadID.xy ) ) 
  #define FRAGMENT_COLOUR4  imageLoad( SAMPLER_GETMAP( lUniforms.mpCmpOutPerMesh, gOutTexture4 ), uvec2( dispatchThreadID.xy ) ) 

#else
  #define FRAGMENT_COLOUR_UVEC4_DEFINE    uvec4 outu_color0 [[color(0)]]
  #define FRAGMENT_COLOUR_UVEC4           Out.mColour0
	#define FRAGMENT_COLOUR                 Out.mColour0
	#define FRAGMENT_COLOUR0                Out.mColour0
	#define FRAGMENT_COLOUR1                Out.mColour1
	#define FRAGMENT_COLOUR2                Out.mColour2
	#define FRAGMENT_COLOUR3                Out.mColour3
	#define FRAGMENT_COLOUR4                Out.mColour4
	#define FRAGMENT_COLOUR5                Out.mColour5
	#define FRAGMENT_DEPTH                  Out.mDepth
	#define FRAGMENT_STENCIL                Out.mStencil
  

  #define WRITE_FRAGMENT_COLOUR( VAL )   FRAGMENT_COLOUR  = VAL 
  #define WRITE_FRAGMENT_COLOUR0( VAL )  FRAGMENT_COLOUR0 = VAL 
  #define WRITE_FRAGMENT_COLOUR1( VAL )  FRAGMENT_COLOUR1 = VAL 
  #define WRITE_FRAGMENT_COLOUR2( VAL )  FRAGMENT_COLOUR2 = VAL 
  #define WRITE_FRAGMENT_COLOUR3( VAL )  FRAGMENT_COLOUR3 = VAL 
  #define WRITE_FRAGMENT_COLOUR4( VAL )  FRAGMENT_COLOUR4 = VAL 
  #define WRITE_FRAGMENT_COLOUR5( VAL )  FRAGMENT_COLOUR5 = VAL 
  #define WRITE_FRAGMENT_DEPTH( VAL )    FRAGMENT_DEPTH   = VAL
  #define WRITE_FRAGMENT_STENCIL( VAL )  FRAGMENT_STENCIL   = VAL // don't know how you do this on pc yet

#endif

  #define DOMAIN_COORDS float2()
  #define COMMON_FRAG_MAIN_DEF fragment cOutput stageMain(cInput In [[stage_in]], UNIFORM_DECL)
  #define VOID_FRAG_MAIN_DEF fragment void stageMain(cInput In [[stage_in]], UNIFORM_DECL)
  #define COMPUTE_PARAMS(X,Y,Z) // [numthreads(X,Y,Z)]

  //use name of #define as a marker for the preprocess step that is used by TkBuildToolFRAGMENT_MAIN_COLOUR_SRT
  //this will get removed/replaced automaticaly
  #define VOID_MAIN_SRT					      _VOID_MAIN_SRT VOID_FRAG_MAIN_DEF

  #if defined( D_COMPUTE )
	  #define COMPUTE_MAIN_SRT( X, Y, Z )		    	    COMPUTE_PARAMS(X,Y,Z) kernel void stageMain(uint3 groupID S_GROUP_ID, uint3 groupThreadID S_GROUP_THREAD_ID, uint3 dispatchThreadID S_DISPATCH_THREAD_ID, UNIFORM_DECL)                                    
    #define COMPUTE_MAIN_UNIF( X, Y, Z, UNIF_TYPE ) _COMPUTE_MAIN_UNIF UNIFORMS_CB_TYPE( UNIF_TYPE ) COMPUTE_PARAMS(X, Y, Z) void stageMain(uint3 groupID S_GROUP_ID, uint3 groupThreadID S_GROUP_THREAD_ID, uint3 dispatchThreadID S_DISPATCH_THREAD_ID UNIFORMS_SRT_TYPE( UNIF_TYPE ) ) 
	  #define FRAGMENT_MAIN_COLOUR_SRT			          _FRAGMENT_MAIN_COLOUR_SRT COMPUTE_MAIN_SRT( 8,8,1 ) 
	  #define FRAGMENT_MAIN_COLOUR01_SRT			        _FRAGMENT_MAIN_COLOUR01_SRT COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR012_SRT			        _FRAGMENT_MAIN_COLOUR012_SRT COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR0123_SRT		        _FRAGMENT_MAIN_COLOUR0123_SRT COMPUTE_MAIN_SRT( 8,8,1 )
  	#define FRAGMENT_MAIN_COLOUR_DEPTH_SRT	      	_FRAGMENT_MAIN_COLOUR_DEPTH_SRT COMPUTE_MAIN_SRT( 8,8,1 )
  	#define FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT	_FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_UICOLOUR_SRT              _FRAGMENT_MAIN_UICOLOUR_SRT COMPUTE_MAIN_SRT( 8,8,1 )
    #define VOID_MAIN_DEPTH_SRT         		        _VOID_MAIN_DEPTH_SRT COMPUTE_MAIN_SRT( 8,8,1 )
	  #define VOID_MAIN_DEPTH_STENCIL_SRT 		        _VOID_MAIN_DEPTH_STENCIL_SRT COMPUTE_MAIN_SRT( 8,8,1 )

  #elif defined( D_TAA_RENDER_TARGETS )	
    #define FRAGMENT_MAIN_COLOUR_SRT		    _FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; }; COMMON_FRAG_MAIN_DEF
  #elif defined( D_LIT_WITH_MASK )    
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT	_FRAGMENT_MAIN_COLOUR_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]];   vec4 mColour1 [[color(1)]]; float mDepth [[depth(any)]];};	COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR_SRT				_FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4 mColour0 [[color(0)]];   vec4 mColour1 [[color(1)]]; }; COMMON_FRAG_MAIN_DEF
  #elif defined( D_REFRACT ) && !defined( D_REFRACT_HIGH )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT	_FRAGMENT_MAIN_COLOUR_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; float mDepth [[depth(any)]];};	 COMMON_FRAG_MAIN_DEF
  	#define FRAGMENT_MAIN_COLOUR_SRT		    _FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]];}; COMMON_FRAG_MAIN_DEF
  #elif defined( D_REFRACT ) &&  defined( D_REFRACT_HIGH )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT	_FRAGMENT_MAIN_COLOUR_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; vec4 mColour4 [[color(4)]]; vec4 mColour5 [[color(5)]]; float mDepth [[depth(any)]]; }; COMMON_FRAG_MAIN_DEF
	  #define FRAGMENT_MAIN_COLOUR_SRT		  	_FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; vec4 mColour4 [[color(4)]]; vec4 mColour5 [[color(5)]]; }; COMMON_FRAG_MAIN_DEF
  #elif !defined( D_ATTRIBUTES )

    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT		_FRAGMENT_MAIN_COLOUR_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]]; float mDepth [[depth(any)]];    };  COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT _FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT struct cOutput { vec4 mColour0 [[color(0)]]; float mDepth [[depth(any)]];  uint mStencil [[stencil]]; };  COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR_DEPTH_GE_SRT	_FRAGMENT_MAIN_COLOUR_DEPTH_GE_SRT struct cOutput { vec4 mColour0 [[color(0)]]; float mDepth [[depth(greater)]]; };  COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR_DEPTH_LE_SRT	_FRAGMENT_MAIN_COLOUR_DEPTH_LE_SRT struct cOutput { vec4 mColour0 [[color(0)]]; float mDepth [[depth(less)]]; };  COMMON_FRAG_MAIN_DEF
  
  // switched from uint4 to vec4 for dummy reflection pipeline creation
    #define FRAGMENT_MAIN_UICOLOUR_SRT			_FRAGMENT_MAIN_UICOLOUR_SRT struct cOutput { uvec4 mColour0 [[color(0)]]; }; COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR_SRT			_FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4  mColour0 [[color(0)]]; }; COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT		_FRAGMENT_MAIN_COLOUR_EARLYZ_SRT struct cOutput { vec4  mColour0 [[color(0)]]; }; [[early_fragment_tests]] COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_INT		            _FRAGMENT_MAIN_COLOUR_SRT struct cOutput { int  mColour0 [[color(0)]]; }; COMMON_FRAG_MAIN_DEF
    #define VOID_MAIN_EARLYZ_SRT                _VOID_MAIN_EARLYZ_SRT [[early_fragment_tests]] VOID_FRAG_MAIN_DEF
	#define VOID_MAIN_DEPTH_SRT					_VOID_MAIN_DEPTH_SRT struct cOutput {  float mDepth [[depth(any)]];  };  COMMON_FRAG_MAIN_DEF
	#define VOID_MAIN_DEPTH_STENCIL_SRT 	    _VOID_MAIN_DEPTH_STENCIL_SRT struct cOutput { uint mStencil [[stencil]]; float mDepth [[depth(any)]]; };  COMMON_FRAG_MAIN_DEF
  	#define VOID_MAIN_COLOUR_EARLYZ_SRT		    _VOID_MAIN_COLOUR_EARLYZ_SRT [[early_fragment_tests]] VOID_FRAG_MAIN_DEF
	#define FRAGMENT_MAIN_COLOUR01_SRT		    _FRAGMENT_MAIN_COLOUR01_SRT struct cOutput {vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]]; };   COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR012_SRT		    _FRAGMENT_MAIN_COLOUR012_SRT struct cOutput {vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]]; vec4 mColour2 [[color(2)]]; };   COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(VARIANT) _FRAGMENT_MAIN_COLOUR012_SRT struct cOutput {vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]]; vec4 mColour2 [[color(2), function_constant(VARIANT)]]; };   COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR012_DEPTH_SRT_VARIANT(VARIANT) _FRAGMENT_MAIN_COLOUR012_DEPTH_SRT struct cOutput {vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]]; vec4 mColour2 [[color(2), function_constant(VARIANT)]]; float mDepth [[depth(any)]]; };   COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR0123_SRT	    _FRAGMENT_MAIN_COLOUR0123_SRT struct cOutput {vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]]; vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; };   COMMON_FRAG_MAIN_DEF
    #define FRAGMENT_MAIN_COLOUR01_DEPTH_SRT	_FRAGMENT_MAIN_COLOUR01_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];float mDepth [[depth(any)]]; };  COMMON_FRAG_MAIN_DEF
  #else
    #if defined( D_OUTPUT_LINEARDEPTH )
        #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT		_FRAGMENT_MAIN_COLOUR_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; vec4 mColour4 [[color(4)]]; float mDepth [[depth(any)]];  }; COMMON_FRAG_MAIN_DEF
        #define FRAGMENT_MAIN_COLOUR_SRT			_FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; vec4 mColour4 [[color(4)]]; };COMMON_FRAG_MAIN_DEF
        #define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT		_FRAGMENT_MAIN_COLOUR_EARLYZ_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; vec4 mColour4 [[color(4]]; };[[early_fragment_tests]] COMMON_FRAG_MAIN_DEF                                                 
    #else
        #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT		_FRAGMENT_MAIN_COLOUR_DEPTH_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; float mDepth [[depth(any)]];  }; COMMON_FRAG_MAIN_DEF
	    #define FRAGMENT_MAIN_COLOUR_SRT			_FRAGMENT_MAIN_COLOUR_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; };COMMON_FRAG_MAIN_DEF
	    #define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT		_FRAGMENT_MAIN_COLOUR_EARLYZ_SRT struct cOutput { vec4 mColour0 [[color(0)]];  vec4 mColour1 [[color(1)]];  vec4 mColour2 [[color(2)]]; vec4 mColour3 [[color(3)]]; };[[early_fragment_tests]] COMMON_FRAG_MAIN_DEF                                                 
    #endif 
  #endif    

  #define FRAGMENT_OUTPUT_T0              Out.mColour0
  #define FRAGMENT_OUTPUT_T1              Out.mColour1
  #define FRAGMENT_OUTPUT_T2              Out.mColour2
  #define FRAGMENT_OUTPUT_T3              Out.mColour3

  #define FRAGMENT_OUTPUTS_T1( T0 )                  struct cOutput { T0 mColour0 S_TARGET_OUTPUT0; };
  #define FRAGMENT_OUTPUTS_T2( T0, T1 )              struct cOutput { T0 mColour0 S_TARGET_OUTPUT0; T1 mColour1 S_TARGET_OUTPUT1; };
  #define FRAGMENT_OUTPUTS_T3( T0, T1, T2 )          struct cOutput { T0 mColour0 S_TARGET_OUTPUT0; T1 mColour1 S_TARGET_OUTPUT1; T2 mColour2 S_TARGET_OUTPUT2; };
  #define FRAGMENT_OUTPUTS_T4( T0, T1, T2, T3 )      struct cOutput { T0 mColour0 S_TARGET_OUTPUT0; T1 mColour1 S_TARGET_OUTPUT1; T2 mColour2 S_TARGET_OUTPUT2; T3 mColour3 S_TARGET_OUTPUT3; };

  #define FRAGMENT_MAIN_T1_SRT( T0 )                  _FRAGMENT_MAIN_T1_SRT FRAGMENT_OUTPUTS_T1(  T0 )             COMMON_FRAG_MAIN_DEF
  #define FRAGMENT_MAIN_T2_SRT( T0, T1 )              _FRAGMENT_MAIN_T2_SRT FRAGMENT_OUTPUTS_T2(  T0, T1 )         COMMON_FRAG_MAIN_DEF
  #define FRAGMENT_MAIN_T3_SRT( T0, T1, T2 )          _FRAGMENT_MAIN_T3_SRT FRAGMENT_OUTPUTS_T3(  T0, T1, T2 )     COMMON_FRAG_MAIN_DEF
  #define FRAGMENT_MAIN_T4_SRT( T0, T1, T2, T3 )      _FRAGMENT_MAIN_T4_SRT FRAGMENT_OUTPUTS_T4(  T0, T1, T2, T3 ) COMMON_FRAG_MAIN_DEF

  #define FRAGMENT_FRONTFACE              In.mbFrontFacing
  #define INPUT_FRONTFACING               bool mbFrontFacing [[front_facing]];

  #define PRECISE                         
  #define INVARIANT
  #define DEREF_PTR( VAR )                (*VAR)

  #ifndef D_DOMAIN
    #define IN(  VAR )                      In.VAR
    #define OUT( VAR )                      Out.VAR
  #else
  #define IN(  VAR, IND )                   
  #define OUT( VAR )                        
    #define PATCH_IN(  VAR, IND )           
  #endif
  #if defined( D_FRAGMENT )
    #define IN_SCREEN_POSITION               IN( mScreenPositionVec4 )
  #else
    #define IN_SCREEN_POSITION
  #endif

    #ifndef D_FRAGMENT
    #define OUTPUT_SCREEN_POSITION OUTPUT( vec4, mScreenPositionVec4, REG_POSITION )
    #define OUTPUT_SCREEN_SLICE
    #define WRITE_SCREEN_SLICE(_S)
    #endif
    
  #ifdef D_VERTEX
    #define OUTPUT_SCREEN_POSITION      OUTPUT( vec4, mScreenPositionVec4, REG_POSITION )
    #define OUTPUT_SCREEN_SLICE	
    #define SCREEN_POSITION					OUT( mScreenPositionVec4 )
    #define VSPS								OUTPUT
  #else
    #define INPUT_SCREEN_POSITION INPUT( vec4, mScreenPositionVec4, REG_POSITION )      
    #define INPUT_SCREEN_SLICE
    #define SCREEN_POSITION				IN( mScreenPositionVec4 )
    #define VSPS      INPUT
  #endif

    #define UNIFORM_DECL        constant UniformBuffer& lUniforms [[buffer(1)]]
    //#define COMPUTE_OUT_DECL    device struct ComputeOutputUniforms& Out [[buffer(1)]]

    #define VERTEX_MAIN                         _VERTEX_MAIN vertex cOutput stageMain(cInput In [[stage_in]])
    #define VERTEX_MAIN_SRT                     _VERTEX_MAIN_SRT vertex cOutput stageMain(cInput In [[stage_in]], UNIFORM_DECL )
    #define VERTEX_MAIN_INSTANCED_SRT           _VERTEX_MAIN_INSTANCED_SRT vertex cOutput stageMain(uint instanceID [[instance_id]], cInput In [[stage_in]], UNIFORM_DECL )
    #define VERTEX_MAIN_INSTANCED_ID_SRT        _VERTEX_MAIN_INSTANCED_ID_SRT vertex cOutput stageMain(uint instanceID [[instance_id]], uint vertexID [[vertex_id]], cInput In [[stage_in]], UNIFORM_DECL )
    #define VERTEX_MAIN_ID_SRT					_VERTEX_MAIN_ID_SRT vertex cOutput stageMain(uint vertexID [[vertex_id]], cInput In [[stage_in]], UNIFORM_DECL )

    #define HULL_TRI_MAIN_SRT                   void HS_MAIN( cInput In, UNIFORM_DECL )
    #define HULL_QUAD_MAIN_SRT( PATCHCNT )      void HS_MAIN( cInput In, UNIFORM_DECL )
    #define DOMAIN_TRI_MAIN_SRT                 void DS_MAIN( void )
    #define DOMAIN_QUAD_MAIN_SRT                void DS_MAIN( void )

    #define GEOMETRY_MAIN_DECLARE_OUTPUT   
    #define GEOMETRY_MAIN_SRT( MAX_VERTS )     void main( cInput In, UNIFORM_DECL )
    // icky workaround for fxc compiler not supporting output parameters on hull shaders >:[
    #define DECLARE_HULL_OUTPUT 
    #define RETURN_HULL_OUTPUT  
//TF_END
#elif defined( D_PLATFORM_GLSL )

  #if defined( D_VERTEX ) // VERTEX SHADER
    #define DECLARE_INPUT
    #define DECLARE_INPUT_END    
    #define DECLARE_OUTPUT                  out VertexBlock {
    #define DECLARE_OUTPUT_END              } Out;

    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
    #define INPUT_SCREEN_POSITION_REDECLARED

    #if defined ( D_PLATFORM_SWITCH )
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR            out gl_PerVertex{
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END        };
        #define OUTPUT_SCREEN_POSITION_REDECLARED               vec4 gl_Position;
    #else
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
        #define OUTPUT_SCREEN_POSITION_REDECLARED
    #endif

  #elif defined( D_FRAGMENT ) // FRAGMENT SHADER

    #if defined( D_GEOM_SHADER_PRESENT )
      #define DECLARE_INPUT                   in GeomBlock {
      #define DECLARE_INPUT_END               } In;
    #elif defined( D_TESS_SHADERS_PRESENT )
      #define DECLARE_INPUT                   in DomainBlock {
      #define DECLARE_INPUT_END               } In;
    #else
      #define DECLARE_INPUT                   in VertexBlock {
      #define DECLARE_INPUT_END               } In;
    #endif
    #define DECLARE_OUTPUT
    #define DECLARE_OUTPUT_END

    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
    #define INPUT_SCREEN_POSITION_REDECLARED

    #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
    #define OUTPUT_SCREEN_POSITION_REDECLARED

  #elif defined( D_GEOMETRY ) // GEOMETRY SHADER

    #if defined( D_TESS_SHADERS_PRESENT )
      #define DECLARE_INPUT                   in DomainBlock {
      #define DECLARE_INPUT_END               } In[];
    #else
      #define DECLARE_INPUT                   in VertexBlock {
      #define DECLARE_INPUT_END               } In[];
    #endif
    #define DECLARE_OUTPUT                  out GeomBlock {
    #define DECLARE_OUTPUT_END              } Out;

    #if defined ( D_PLATFORM_SWITCH )
      #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR       in gl_PerVertex{
      #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END   } gl_in[];
      #define INPUT_SCREEN_POSITION_REDECLARED          vec4 gl_Position;

      #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR      out gl_PerVertex{
      #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END  };
      #define OUTPUT_SCREEN_POSITION_REDECLARED         vec4 gl_Position;
    #else
      #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
      #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
      #define INPUT_SCREEN_POSITION_REDECLARED

      #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
      #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
      #define OUTPUT_SCREEN_POSITION_REDECLARED
    #endif
    
  #elif defined( D_DOMAIN ) // DOMAIN SHADER

    #define DECLARE_INPUT                   in HullBlock {
    #define DECLARE_INPUT_END               } In[];
    #define DECLARE_OUTPUT                  out DomainBlock {
    #define DECLARE_OUTPUT_END              } Out;

    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
    #define INPUT_SCREEN_POSITION_REDECLARED

    #if defined ( D_PLATFORM_SWITCH )
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR            out gl_PerVertex{
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END        };
        #define OUTPUT_SCREEN_POSITION_REDECLARED               vec4 gl_Position;
    #else
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
        #define OUTPUT_SCREEN_POSITION_REDECLARED
    #endif

  #elif defined( D_HULL ) // HULL SHADER

    #define DECLARE_INPUT                   in VertexBlock {
    #define DECLARE_INPUT_END               } In[];
    #define DECLARE_OUTPUT                  out HullBlock {
    #define DECLARE_OUTPUT_END              } Out[];

    #if defined ( D_PLATFORM_SWITCH )
        #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR            in gl_PerVertex{
        #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END        } gl_in[];
        #define INPUT_SCREEN_POSITION_REDECLARED               vec4 gl_Position;

        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR            out gl_PerVertex{
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END        } gl_out[];
        #define OUTPUT_SCREEN_POSITION_REDECLARED               vec4 gl_Position;
    #else
        #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
        #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
        #define INPUT_SCREEN_POSITION_REDECLARED

        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
        #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
        #define OUTPUT_SCREEN_POSITION_REDECLARED
    #endif
  #endif

  #define DEREF_PTR( VAR )                VAR

  #define DECLARE_UNIFORMS                struct UniformBuffer {

  #ifdef D_UNIFORMS_ARE_GLOBAL
    #define DECLARE_UNIFORMS_END            }; uniform lUniforms_BLK { UniformBuffer lUniforms; };
    #define DECLARE_PACKED_UNIFORMS_END     }; layout(packed) uniform lUniforms_BLK { UniformBuffer lUniforms; };
  #else
    #define DECLARE_UNIFORMS_END            };
    #define DECLARE_PACKED_UNIFORMS_END     };
  #endif

  #define DECLARE_PTR( TYPE, NAME )       TYPE NAME;
  #define UNIFORM( INST, VAR )            lUniforms.INST.VAR

  #define DEREF_UNIFORM( VAR )            lUniforms.VAR

  #ifdef D_USE_UBO

   // #define DECLARE_UNIFORM( TYPE, NAME )   uniform NAME ## _BLK { TYPE NAME; };

  #else

    #define DEREF_UNIFORM( VAR )            lUniforms.VAR

   // #define DECLARE_UNIFORM( TYPE, NAME )   uniform TYPE NAME;

  #endif

    #define DECLARE_PATCH_INPUT_TRI
    #define DECLARE_PATCH_OUTPUT_TRI
    #define DECLARE_PATCH_INPUT_END
    #define DECLARE_PATCH_OUTPUT_END
    #define IN_PATCH_TRI_TESS_CONSTANTS
    #define OUT_PATCH_TRI_TESS_CONSTANTS

    #define DECLARE_PATCH_INPUT_QUAD
    #define DECLARE_PATCH_OUTPUT_QUAD
    #define IN_PATCH_QUAD_TESS_CONSTANTS
    #define OUT_PATCH_QUAD_TESS_CONSTANTS

  #if defined( D_VERTEX ) 

    #define INPUT(            TYPE, NAME, REG )  layout( location = REG ) in TYPE NAME;
    #define OUTPUT(           TYPE, NAME, REG )                              TYPE NAME;

    #define POSITION0    0
    #define TEXCOORD0    1
    #define NORMAL0      2
    #define NORMAL       2
    #define TANGENT0     3
    #define TANGENT      4
    #define BLENDINDICES 5
    #define BLENDWEIGHT  6
    #define TEXCOORD1    7
    #define TEXCOORD2    8
    #define TEXCOORD3    9
    #define TEXCOORD4    10
    #define TEXCOORD5    11
    #define TEXCOORD6    12
    #define TEXCOORD7    13
    #define TEXCOORD8    14
    #define TEXCOORD9    15
    #define TEXCOORD10   16
    #define TEXCOORD11   17
    #define TEXCOORD12   18
    #define TEXCOORD13   19
    #define TEXCOORD14   20
    #define TEXCOORD15   21
    #define TEXCOORD16   22
    #define TEXCOORD17   23
    #define TEXCOORD18   24
    #define TEXCOORD19   25
    #define TEXCOORD20   26
    #define TEXCOORD21   27
    #define TEXCOORD22   28
    #define TEXCOORD23   29

  #elif defined( D_FRAGMENT )

    #define INPUT(            TYPE, NAME, REG )               TYPE NAME;
    #define INPUT_NOPERSP(    TYPE, NAME, REG )               noperspective TYPE NAME;
    #define OUTPUT(           TYPE, NAME, REG )   out         TYPE NAME;

  #else

    #define INPUT(            TYPE, NAME, REG )               TYPE NAME;
    #define INPUT_NOPERSP(    TYPE, NAME, REG )               TYPE NAME;
    #define OUTPUT(           TYPE, NAME, REG )               TYPE NAME;

  #endif

    #define FRAGMENT_COLOUR_UVEC4_DEFINE    layout(location = 0) out uvec4 outu_color0;
    #define FRAGMENT_COLOUR_UVEC4           outu_color0

    #define FRAGMENT_COLOUR                 out_color0
    #define FRAGMENT_COLOUR0                out_color0
    #define FRAGMENT_COLOUR1                out_color1
    #define FRAGMENT_COLOUR2                out_color2
    #define FRAGMENT_COLOUR3                out_color3
    #define FRAGMENT_COLOUR4                out_color4
    #define FRAGMENT_COLOUR5                out_color5
    #define FRAGMENT_OUTPUT_T0              out_var0
    #define FRAGMENT_OUTPUT_T1              out_var1
    #define FRAGMENT_OUTPUT_T2              out_var2
    #define FRAGMENT_OUTPUT_T3              out_var3

    #define FRAGMENT_OUTPUTS_T1(  T0 )              layout(location = 0) out T0 out_var0;
    #define FRAGMENT_OUTPUTS_T2(  T0, T1 )          layout(location = 0) out T0 out_var0; layout(location = 1) out T1 out_var1;
    #define FRAGMENT_OUTPUTS_T3(  T0, T1, T2 )      layout(location = 0) out T0 out_var0; layout(location = 1) out T1 out_var1; layout(location = 2) out T2 out_var2;
    #define FRAGMENT_OUTPUTS_T4(  T0, T1, T2, T3 )  layout(location = 0) out T0 out_var0; layout(location = 1) out T1 out_var1; layout(location = 2) out T2 out_var2; layout(location = 3) out T3 out_var3;

    #if defined( D_TAA_RENDER_TARGETS )
        #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2;
    #elif defined( D_LIT_WITH_MASK )    
        #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;
    #elif defined( D_REFRACT ) && !defined( D_REFRACT_HIGH )
        #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2; layout(location = 3) out vec4 out_color3;
    #elif defined( D_REFRACT ) &&  defined( D_REFRACT_HIGH )
        #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2; layout(location = 3) out vec4 out_color3; layout(location = 4) out vec4 out_color4; layout(location = 5) out vec4 out_color5;
    #elif !defined( D_ATTRIBUTES )
        #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;
    #else
        #if defined( D_OUTPUT_LINEARDEPTH )
            #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2; layout(location = 3) out vec4 out_color3; layout(location = 4) out vec4 out_color4;
        #else   
            #define FRAGMENT_COLOUR_VEC4_DEFINE layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2; layout(location = 3) out vec4 out_color3;
        #endif
    #endif    
    #define FRAGMENT_COLOUR01_VEC4_DEFINE       layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;
    #define FRAGMENT_COLOUR012_VEC4_DEFINE      layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2;
    #define FRAGMENT_COLOUR0123_VEC4_DEFINE     layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2; layout(location = 3) out vec4 out_color3;
    #define FRAGMENT_COLOUR01234_VEC4_DEFINE    layout(location = 0) out vec4 out_color0;  layout(location = 1) out vec4 out_color1;  layout(location = 2) out vec4 out_color2; layout(location = 3) out vec4 out_color3; layout(location = 4) out vec4 out_color4;

    #define FRAGMENT_DEPTH                  gl_FragDepth
#if defined( GL_ARB_shader_stencil_export )
    #define FRAGMENT_STENCIL                gl_FragStencilRefARB
#else
	#define FRAGMENT_STENCIL                gl_FragDepth.a // If extension is unavailable the value will be discarted
#endif
    #define FRAGMENT_FRONTFACE              gl_FrontFacing

    #define WRITE_FRAGMENT_COLOUR( VAL )   FRAGMENT_COLOUR  = VAL 
    #define WRITE_FRAGMENT_COLOUR0( VAL )  FRAGMENT_COLOUR0 = VAL 
    #define WRITE_FRAGMENT_COLOUR1( VAL )  FRAGMENT_COLOUR1 = VAL 
    #define WRITE_FRAGMENT_COLOUR2( VAL )  FRAGMENT_COLOUR2 = VAL 
    #define WRITE_FRAGMENT_COLOUR3( VAL )  FRAGMENT_COLOUR3 = VAL 
    #define WRITE_FRAGMENT_COLOUR4( VAL )  FRAGMENT_COLOUR4 = VAL 
    #define WRITE_FRAGMENT_COLOUR5( VAL )  FRAGMENT_COLOUR5 = VAL 
    #define WRITE_FRAGMENT_DEPTH( VAL )    FRAGMENT_DEPTH   = VAL
    #define WRITE_FRAGMENT_STENCIL( VAL )  FRAGMENT_STENCIL = VAL 


    #define INPUT_FRONTFACING

    #define PRECISE                         precise
    #define INVARIANT       

    #define DEREF_PTR( VAR )                VAR

  #if defined( D_HULL ) 

	#define IN(  VAR )                      In[ gl_InvocationID ].VAR
	#define OUT( VAR )                      Out[ gl_InvocationID ].VAR
    #define PATCH_IN( VAR, IND )            In[ IND ].VAR

    #define OUT_SCREEN_POSITION             gl_out[ gl_InvocationID ].gl_Position
    #define IN_SCREEN_POSITION( IND )       gl_in [ IND ].gl_Position

    #define TESS_LEVEL_EDGE( IND )          gl_TessLevelOuter[ IND ]
    #define TESS_LEVEL_INNER( IND )         gl_TessLevelInner[ IND ]

    #define HULL_TRI_CONSTANTS_SRT          void ConstantsHS(                           \
                                                UniformBuffer lUniforms )
    #define HULL_QUAD_CONSTANTS_SRT( PATCHCNT ) void ConstantsHS(                           \
                                                UniformBuffer lUniforms )

    #define CALL_HULL_CONSTANTS             ConstantsHS( lUniforms );

    #if defined ( D_TESS_ON_AMD )
      #define PATCH_OUTPUT(     TYPE, NAME, REG )   patch out   TYPE NAME;
    #endif // D_TESS_ON_AMD

  #elif defined( D_DOMAIN )

	#define IN(  VAR, IND )                 In[ IND ].VAR
	#define OUT( VAR )                      Out.VAR
	#define PATCH_IN(  VAR, IND )           In[ IND ].VAR

    #define OUT_SCREEN_POSITION             gl_Position
    #define IN_SCREEN_POSITION( IND )       gl_in [ IND ].gl_Position

    #define DOMAIN_COORDS                   gl_TessCoord

  #elif defined( D_GEOMETRY )

	#define IN(  VAR, IND )                 In[ IND ].VAR
	#define OUT( VAR )                      Out.VAR

    #define OUT_SCREEN_POSITION				gl_Position
    #define IN_SCREEN_POSITION(IND)			gl_in [ IND ].gl_Position

    #define EMIT_VERTEX                     EmitVertex()
    #define END_PRIMITIVE                   EndPrimitive()

  #elif defined( D_VERTEX )

    #define IN(  VAR )                      VAR
    #define OUT( VAR )                      Out.VAR

    #define IN_SCREEN_POSITION 

  #else

	#define IN(  VAR )                      In.VAR
	#define OUT( VAR )                      Out.VAR

    #if defined( D_FRAGMENT )
    #define IN_SCREEN_POSITION              gl_FragCoord
    #else
    #define IN_SCREEN_POSITION
    #endif

  #endif

    #ifdef D_VERTEX
      #define OUTPUT_SCREEN_POSITION		
      #define OUTPUT_SCREEN_SLICE   
      #define WRITE_SCREEN_SLICE(_S)   
      #define SCREEN_POSITION					gl_Position
      #define VSPS								OUTPUT
    #elif defined D_FRAGMENT
      #define INPUT_SCREEN_POSITION           
      #define INPUT_SCREEN_SLICE    
      #define SCREEN_POSITION				
      #define VSPS      INPUT
    #elif defined( D_DOMAIN ) || defined( D_HULL ) || defined( D_GEOMETRY )
      #define OUTPUT_SCREEN_POSITION          
      #define OUTPUT_SCREEN_SLICE    	
      #define WRITE_SCREEN_SLICE(_S)   
      #define INPUT_SCREEN_POSITION             
      #define INPUT_SCREEN_SLICE   
    #endif

#else // !D_PLATFORM_GLSL

    #define DECLARE_OUTPUT                  struct cOutput {
    #define DECLARE_INPUT                   struct cInput  {
    #define DECLARE_INPUT_END               };
    #define DECLARE_OUTPUT_END              };

    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END
    #define INPUT_SCREEN_POSITION_REDECLARED

    #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
    #define DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END
    #define OUTPUT_SCREEN_POSITION_REDECLARED

    #define DECLARE_UNIFORMS                struct UniformBuffer {
    #define DECLARE_UNIFORMS_END            };
    #define DECLARE_PACKED_UNIFORMS_END     };

    #define INPUT(TYPE, NAME, REG )         TYPE NAME : REG;
    #define INPUT_NOPERSP(TYPE, NAME, REG ) TYPE NAME : REG;
    #define OUTPUT(TYPE, NAME, REG )		TYPE NAME : REG;    

    #define D_PER_INSTANCE_BUFFER_ID        0
    #define D_PER_INSTANCE_CULL_BUFFER_ID   1

  #ifdef D_PLATFORM_ORBIS

	#define	flat			nointerp
	#define	smooth			linear
	#define	noperspective	nopersp
    #define PRECISE         
    #define INVARIANT       __invariant

    #define DECLARE_PTR( TYPE, NAME )       TYPE* NAME;
	// TODO get rid of this - don't pass struct through functinos, pass members.
	#define DEREF_PTR( VAR )                *VAR
    #define DEREF_UNIFORM( VAR )            *lUniforms.VAR
    #define UNIFORM( INST, VAR )            lUniforms.INST.VAR
  #else
    
    #if defined( D_PLATFORM_DX12 ) && !defined( D_PLATFORM_XBOXONE )
    //these currently causes problems if defined on PC
    #define	flat		    
    #define	smooth			
    #else
    #define	flat			nointerpolation
    #define	smooth			linear
    #endif

    #define PRECISE         precise	
    #define INVARIANT       

    #define DECLARE_PTR( TYPE, NAME )       TYPE NAME;
	  #define DEREF_PTR( VAR )                VAR
    #define DEREF_UNIFORM( VAR )            lUniforms.VAR
    #define UNIFORM( INST, VAR )            lUniforms.INST.VAR
  #endif

 
  #if defined D_COMPUTE
  #if defined D_PLATFORM_ORBIS
	#define FRAGMENT_COLOUR                 lUniforms.mpCmpOutPerMesh.gOutTexture0[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
	#define FRAGMENT_COLOUR0                lUniforms.mpCmpOutPerMesh.gOutTexture0[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
	#define FRAGMENT_COLOUR1                lUniforms.mpCmpOutPerMesh.gOutTexture1[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
	#define FRAGMENT_COLOUR2                lUniforms.mpCmpOutPerMesh.gOutTexture2[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
	#define FRAGMENT_COLOUR3                lUniforms.mpCmpOutPerMesh.gOutTexture3[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
	#define FRAGMENT_COLOUR4                lUniforms.mpCmpOutPerMesh.gOutTexture4[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
    #define FRAGMENT_DEPTH                  lUniforms.mpCmpOutPerMesh.gOutTextureDepth[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
    #define FRAGMENT_STENCIL                lUniforms.mpCmpOutPerMesh.gOutTextureStencil[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)] 
  #elif defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS )
    #define FRAGMENT_COLOUR                 lUniforms.mpCmpOutPerMesh.gOutTexture0[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR0                lUniforms.mpCmpOutPerMesh.gOutTexture0[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR1                lUniforms.mpCmpOutPerMesh.gOutTexture1[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR2                lUniforms.mpCmpOutPerMesh.gOutTexture2[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR3                lUniforms.mpCmpOutPerMesh.gOutTexture3[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR4                lUniforms.mpCmpOutPerMesh.gOutTexture4[dispatchThreadID.xy] 
    #define FRAGMENT_DEPTH                  lUniforms.mpCmpOutPerMesh.gOutTextureDepth[dispatchThreadID.xy] 
    #define FRAGMENT_STENCIL                lUniforms.mpCmpOutPerMesh.gOutTextureStencil[dispatchThreadID.xy] 
  #else
    #define FRAGMENT_COLOUR                 gOutTexture0[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR0                gOutTexture0[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR1                gOutTexture1[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR2                gOutTexture2[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR3                gOutTexture3[dispatchThreadID.xy] 
    #define FRAGMENT_COLOUR4                gOutTexture4[dispatchThreadID.xy] 
    #define FRAGMENT_DEPTH                  gOutTextureDepth[dispatchThreadID.xy] 
    #define FRAGMENT_STENCIL                gOutTextureStencil[dispatchThreadID.xy] 
  #endif
  #else
  #ifdef D_FRAGMENT
    #define INPUT_FRONTFACING				INPUT( bool, mbFrontFacing, S_FRONT_FACE )
    #define FRAGMENT_FRONTFACE              In.mbFrontFacing
    #define FRAGMENT_COLOUR                 Out.mColour
    #define FRAGMENT_COLOUR0                Out.mColour0
    #define FRAGMENT_COLOUR1                Out.mColour1
    #define FRAGMENT_COLOUR2                Out.mColour2
    #define FRAGMENT_COLOUR3                Out.mColour3
    #define FRAGMENT_COLOUR4                Out.mColour4
    #define FRAGMENT_COLOUR5                Out.mColour5
    #define FRAGMENT_DEPTH                  Out.mDepth
    #define FRAGMENT_STENCIL                Out.mStencil
    #define FRAGMENT_COLOUR_UVEC4           Out.mColour
    #define FRAGMENT_OUTPUT_T0              Out.mOut0
    #define FRAGMENT_OUTPUT_T1              Out.mOut1
    #define FRAGMENT_OUTPUT_T2              Out.mOut2
    #define FRAGMENT_OUTPUT_T3              Out.mOut3

  #else
	#define INPUT_FRONTFACING
  #endif
  #endif

    #define WRITE_FRAGMENT_COLOUR( VAL )   FRAGMENT_COLOUR  = VAL 
    #define WRITE_FRAGMENT_COLOUR0( VAL )  FRAGMENT_COLOUR0 = VAL 
    #define WRITE_FRAGMENT_COLOUR1( VAL )  FRAGMENT_COLOUR1 = VAL 
    #define WRITE_FRAGMENT_COLOUR2( VAL )  FRAGMENT_COLOUR2 = VAL 
    #define WRITE_FRAGMENT_COLOUR3( VAL )  FRAGMENT_COLOUR3 = VAL 
    #define WRITE_FRAGMENT_COLOUR4( VAL )  FRAGMENT_COLOUR4 = VAL 
    #define WRITE_FRAGMENT_COLOUR5( VAL )  FRAGMENT_COLOUR5 = VAL 
    #define WRITE_FRAGMENT_DEPTH( VAL )    FRAGMENT_DEPTH   = VAL
    #define WRITE_FRAGMENT_STENCIL( VAL )  FRAGMENT_STENCIL = VAL
    

  #if defined( D_HULL ) 

    #define IN(  VAR )                      In[ uCPID ].VAR
    #define OUT( VAR )                      Out.VAR
    #define PATCH_IN(VAR, IND)              In[ IND ].VAR
    #define PATCH_OUT( VAR )                Out.VAR

    #define IN_SCREEN_POSITION( IND )       In[ IND ].mScreenPositionVec4
    #define OUT_SCREEN_POSITION             OUT( mScreenPositionVec4 )

    #define TESS_LEVEL_EDGE( IND )          Out.edge_ts[ IND ]
    #define TESS_LEVEL_INNER( IND )         Out.insi_ts[ IND ]

    #define gl_InvocationID                 uCPID

  #if defined D_PLATFORM_ORBIS
    #define DECLARE_PATCH_OUTPUT_TRI        [DOMAIN_PATCH_TYPE("tri")]  struct HSConstantOutputData {
    #define OUT_PATCH_TRI_TESS_CONSTANTS    float edge_ts[3]            : S_EDGE_TESS_FACTOR;    \
                                            float insi_ts[1]            : S_INSIDE_TESS_FACTOR;

    #define DECLARE_PATCH_OUTPUT_QUAD       [DOMAIN_PATCH_TYPE("quad")] struct HSConstantOutputData {
    #define OUT_PATCH_QUAD_TESS_CONSTANTS   float edge_ts[4]            : S_EDGE_TESS_FACTOR;    \
                                            float insi_ts[2]            : S_INSIDE_TESS_FACTOR;
   #else                                        
    #define DECLARE_PATCH_OUTPUT_TRI        struct HSConstantOutputData {
    #define OUT_PATCH_TRI_TESS_CONSTANTS    float edge_ts[3]            : SV_TessFactor;    \
                                            float insi_ts[1]            : SV_InsideTessFactor;

    #define DECLARE_PATCH_OUTPUT_QUAD       struct HSConstantOutputData {
    #define OUT_PATCH_QUAD_TESS_CONSTANTS   float edge_ts[4]            : SV_TessFactor;    \
                                            float insi_ts[2]            : SV_InsideTessFactor;
   #endif

    #define DECLARE_PATCH_OUTPUT_END };
    #define PATCH_OUTPUT(TYPE, NAME, REG )  TYPE NAME : REG;

    #define CALL_HULL_CONSTANTS


  #elif defined( D_DOMAIN )

    #define IN(VAR, IND)                    In[ IND ].VAR
    #define OUT(VAR)                        Out.VAR
	#define PATCH_IN(VAR)                   patchIn.VAR

    #define OUT_SCREEN_POSITION	            OUT( mScreenPositionVec4 )
    #define IN_SCREEN_POSITION(IND)         IN( mScreenPositionVec4, IND )

    #define DOMAIN_COORDS                   domainCoordinates

  #if defined D_PLATFORM_ORBIS

    #define DECLARE_PATCH_INPUT_TRI         [DOMAIN_PATCH_TYPE("tri")]  struct HSConstantOutputData {
    #define IN_PATCH_TRI_TESS_CONSTANTS     float edge_ts[3]            : S_EDGE_TESS_FACTOR;    \
                                            float insi_ts[1]            : S_INSIDE_TESS_FACTOR;

    #define DECLARE_PATCH_INPUT_QUAD        [DOMAIN_PATCH_TYPE("quad")] struct HSConstantOutputData {
    #define IN_PATCH_QUAD_TESS_CONSTANTS    float edge_ts[4]            : S_EDGE_TESS_FACTOR;    \
                                            float insi_ts[2]            : S_INSIDE_TESS_FACTOR;
   #else                                        

    #define DECLARE_PATCH_INPUT_TRI         struct HSConstantOutputData {
    #define IN_PATCH_TRI_TESS_CONSTANTS     float edge_ts[3]            : SV_TessFactor;    \
                                            float insi_ts[1]            : SV_InsideTessFactor;

    #define DECLARE_PATCH_INPUT_QUAD        struct HSConstantOutputData {
    #define IN_PATCH_QUAD_TESS_CONSTANTS    float edge_ts[4]            : SV_TessFactor;    \
                                            float insi_ts[2]            : SV_InsideTessFactor;
   #endif

    #define DECLARE_PATCH_INPUT_END };
    #define PATCH_INPUT(TYPE, NAME, REG)    TYPE NAME : REG;

  #elif defined( D_GEOMETRY )

    #define IN(  VAR, IND )                 In[ IND ].VAR
    #define OUT( VAR )                      Out.VAR

    #define OUT_SCREEN_POSITION             OUT( mScreenPositionVec4 )
    #define IN_SCREEN_POSITION( IND )       IN( mScreenPositionVec4, IND )

    #define EMIT_VERTEX                     TriStream.Append( Out )
    #define END_PRIMITIVE                   TriStream.RestartStrip()

  #else

    #define IN(  VAR )                      In.VAR
    #define OUT( VAR )                      Out.VAR

  #if defined( D_FRAGMENT )                  
    #define IN_SCREEN_POSITION              IN( mScreenPositionVec4 )
  #endif

  #endif


    #ifndef D_FRAGMENT
      #define OUTPUT_SCREEN_POSITION            OUTPUT( vec4, mScreenPositionVec4, S_POSITION )
      #if defined D_PLATFORM_ORBIS && !defined D_PLATFORM_ORBIS_GEOMETRY
        #define OUTPUT_SCREEN_SLICE    			OUTPUT( uint, mScreenSliceIndexUint, S_RENDER_TARGET_INDEX )
        #define WRITE_SCREEN_SLICE(_S)         OUT( mScreenSliceIndexUint ) = uint( _S )
      #else
        #define OUTPUT_SCREEN_SLICE 
        #define INPUT_SCREEN_SLICE
        #define WRITE_SCREEN_SLICE(_S)   
      #endif
    #endif
    #ifndef D_VERTEX
      #define INPUT_SCREEN_POSITION             INPUT( vec4, mScreenPositionVec4, S_POSITION )  
      #if defined D_PLATFORM_ORBIS
        #define INPUT_SCREEN_SLICE              INPUT(uint, mScreenSliceIndexUint, S_RENDER_TARGET_INDEX)
      #else   
        #define INPUT_SCREEN_SLICE 
      #endif
    #endif

    #ifdef D_VERTEX
      #define SCREEN_POSITION                   OUT( mScreenPositionVec4 )
      #define VSPS                              OUTPUT
    #elif defined D_FRAGMENT
      #define SCREEN_POSITION					IN( mScreenPositionVec4 )
      #define VSPS      INPUT
    #endif

#endif



// =================================================================================================
// Main
// =================================================================================================
#if defined( D_PLATFORM_PC_COMPUTE ) || defined ( D_PLATFORM_SWITCH_COMPUTE )

  #ifdef D_UNIFORMS_ARE_GLOBAL
    #define UNIFORM_DECL         
  #else
    #ifdef D_USE_UBO
      #define UNIFORM_DECL                        DECLARE_UNIFORM( UniformBuffer, lUniforms )
    #else
      #define UNIFORM_DECL                        uniform UniformBuffer lUniforms;
    #endif
  #endif

    #define COMPUTE_MAIN_SRT( X, Y, Z )         layout (local_size_x = X, local_size_y = Y, local_size_z = Z) in; UNIFORM_DECL void main( void )
    #define COMPUTE_MAIN_UNIF( X, Y, Z, UNIF_TYPE )         layout (local_size_x = X, local_size_y = Y, local_size_z = Z) in; uniform lUniforms_BLK { UNIF_TYPE lUniforms; }; void main( void )

    #define VOID_MAIN_DEPTH_SRT                 COMPUTE_MAIN_SRT( 8,8,1 )
    #define VOID_MAIN_DEPTH_STENCIL_SRT         COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR_SRT            COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_UICOLOUR_SRT          COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR01_SRT          COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR012_SRT         COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR0123_SRT        COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR01234_SRT       COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT      COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR012_DEPTH_SRT   COMPUTE_MAIN_SRT( 8,8,1 )

#elif defined(D_PLATFORM_METAL)

#elif defined( D_PLATFORM_GLSL )

  #ifdef D_UNIFORMS_ARE_GLOBAL
    #define UNIFORM_DECL         
  #else
    #ifdef D_USE_UBO
      #define UNIFORM_DECL                      DECLARE_UNIFORM( UniformBuffer, lUniforms )
    #else
  	  #define UNIFORM_DECL 						uniform UniformBuffer lUniforms;
    #endif
  #endif

    #define VERTEX_MAIN                         void main( void )
    #define VERTEX_MAIN_SRT                     UNIFORM_DECL void main( void )
    #define VERTEX_MAIN_INSTANCED_SRT           UNIFORM_DECL void main( void )
    #define VERTEX_MAIN_ID_SRT					UNIFORM_DECL void main( void )

    #define HULL_TRI_MAIN_SRT                   layout( vertices = 3 ) out; UNIFORM_DECL void main( void )
    #define HULL_QUAD_MAIN_SRT( PATCHCNT )      layout( vertices = 4 ) out; UNIFORM_DECL void main( void )

  #ifdef D_PLATFORM_VULKAN    
    #define DOMAIN_TRI_MAIN_SRT                 layout( triangles, equal_spacing, cw ) in; UNIFORM_DECL void main( void )
    #define DOMAIN_QUAD_MAIN_SRT                layout( quads, equal_spacing, cw ) in; UNIFORM_DECL void main( void )
  #else
    #define DOMAIN_TRI_MAIN_SRT                 layout( triangles, equal_spacing, ccw ) in; UNIFORM_DECL void main( void )
    #define DOMAIN_QUAD_MAIN_SRT                layout( quads, equal_spacing, ccw ) in; UNIFORM_DECL void main( void )
  #endif

    #define FRAGMENT_MAIN_COLOUR                FRAGMENT_COLOUR_VEC4_DEFINE void main( void )
    #define VOID_MAIN_COLOUR                    FRAGMENT_COLOUR_VEC4_DEFINE void main( void )
    #define FRAGMENT_MAIN_COLOUR_DEPTH          FRAGMENT_COLOUR_VEC4_DEFINE void main( void )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_GE_SRT   FRAGMENT_COLOUR_VEC4_DEFINE UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_LE_SRT   FRAGMENT_COLOUR_VEC4_DEFINE UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT     layout(early_fragment_tests) in; FRAGMENT_COLOUR_VEC4_DEFINE UNIFORM_DECL void main( void )


    #define VOID_MAIN_SRT                       UNIFORM_DECL void main( void )
    #define VOID_MAIN_DEPTH_SRT                 UNIFORM_DECL void main( void )
    #define VOID_MAIN_EARLYZ_SRT                layout(early_fragment_tests) in; UNIFORM_DECL void main( void )
    #define VOID_MAIN_DEPTH_STENCIL_SRT         UNIFORM_DECL void main( void )
    #define VOID_MAIN_COLOUR_EARLYZ_SRT         layout(early_fragment_tests) in; UNIFORM_DECL void main( void )

    #define VOID_MAIN_REZ_SRT                   VOID_MAIN_SRT

    #define FRAGMENT_MAIN_COLOUR_SRT            FRAGMENT_COLOUR_VEC4_DEFINE         UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT      FRAGMENT_COLOUR_VEC4_DEFINE         UNIFORM_DECL void main( void )
	#define FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT FRAGMENT_COLOUR_VEC4_DEFINE     UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR01_SRT          FRAGMENT_COLOUR01_VEC4_DEFINE       UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR012_SRT         FRAGMENT_COLOUR012_VEC4_DEFINE      UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR0123_SRT        FRAGMENT_COLOUR0123_VEC4_DEFINE     UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR01234_SRT       FRAGMENT_COLOUR01234_VEC4_DEFINE    UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR01_DEPTH_SRT    FRAGMENT_COLOUR01_VEC4_DEFINE       UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_COLOUR012_DEPTH_SRT   FRAGMENT_COLOUR012_VEC4_DEFINE      UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_UICOLOUR_SRT          FRAGMENT_COLOUR_UVEC4_DEFINE        UNIFORM_DECL void main( void )

    #define FRAGMENT_MAIN_T1_SRT( T0 )                 FRAGMENT_OUTPUTS_T1(  T0 )             UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_T2_SRT( T0, T1 )             FRAGMENT_OUTPUTS_T2(  T0, T1 )         UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_T3_SRT( T0, T1, T2 )         FRAGMENT_OUTPUTS_T3(  T0, T1, T2 )     UNIFORM_DECL void main( void )
    #define FRAGMENT_MAIN_T4_SRT( T0, T1, T2, T3 )     FRAGMENT_OUTPUTS_T4(  T0, T1, T2, T3 ) UNIFORM_DECL void main( void )

    #define GEOMETRY_MAIN_DECLARE_OUTPUT   
    #define GEOMETRY_MAIN_SRT( MAX_VERTS )      layout(triangles) in; layout(triangle_strip, max_vertices = MAX_VERTS) out; UNIFORM_DECL void main( void )


    // icky workaround for fxc compiler not supporting output parameters on hull shaders >:[
    #define DECLARE_HULL_OUTPUT 
    #define RETURN_HULL_OUTPUT     
    
#else

  #ifdef D_PLATFORM_ORBIS
	#define DEF_SRT(S,N)			S N : S_SRT_DATA
	#define DEF_SRTP(S,N)			S *N : S_SRT_DATA
	#define UNIFORMS_CB
	#define UNIFORMS_SRT			, UniformBuffer lUniforms : S_SRT_DATA
    #define UNIFORMS_CB_TYPE( T )
    #define UNIFORMS_SRT_TYPE( T )  , T * lUniforms : S_SRT_DATA
	#define ROOT_SIG(P)
	#define FRAG					[RE_Z]
    #define FRAG_EDS
	#define COMPUTE_PARAMS(X,Y,Z)	[NUM_THREADS(X, Y, Z)]

    #define HULL_PARAMS(domain, partitioning, topology, points, maxtess)    [DOMAIN_PATCH_TYPE(#domain)] [PARTITIONING_TYPE(#partitioning)] [OUTPUT_TOPOLOGY_TYPE(#topology)] [OUTPUT_CONTROL_POINTS(points)] [PATCH_CONSTANT_FUNC("ConstantsHS")] [MAX_TESS_FACTOR(maxtess)]
    #define DOMAIN_PARAMS(domain)                                           [DOMAIN_PATCH_TYPE(#domain)]
    #define GEOMETRY_MAIN_DECLARE_OUTPUT   
    #define GEOMETRY_MAIN_SRT(MAX_VERTS)		UNIFORMS_CB cOutput Out; [MAX_VERTEX_COUNT(MAX_VERTS)]				void main(inout TriangleBuffer<cOutput> TriStream, Triangle cInput In[3] UNIFORMS_SRT)

  #else
	#define DEF_SRT(S,N)			cbuffer _##S : register(b0) { S N; }
	#define DEF_SRTP(S,N)			DEF_SRT(S,N)
	#define UNIFORMS_CB				DEF_SRT(UniformBuffer, lUniforms);
	#define UNIFORMS_SRT
    #define UNIFORMS_CB_TYPE( T )   DEF_SRT( T , lUniforms);
    #define UNIFORMS_SRT_TYPE( T )
	#define ROOT_SIG(P)				[RootSignature(ROOT_SIG_##P)]
	#define FRAG		

    #if defined( D_FRAGMENT ) && defined( _F64_ ) && defined( D_PLATFORM_DX12 ) && !defined( D_PLATFORM_XBOXONE )
    #define FRAG_EDS [earlydepthstencil]
    #else
    #define FRAG_EDS
    #endif

	#define COMPUTE_PARAMS(X,Y,Z)	[numthreads(X, Y, Z)] ROOT_SIG(CS) 
    #define S_OUTPUT_CONTROL_POINT_ID SV_OutputControlPointID
    #define S_DOMAIN_LOCATION           SV_DomainLocation
    #define HULL_PARAMS(dmn, prt, topology, points, maxtess)    ROOT_SIG(HS) [domain(#dmn)] [partitioning(#prt)] [outputtopology(#topology)] [outputcontrolpoints(points)] [patchconstantfunc("ConstantsHS")] [maxtessfactor(maxtess)]
    #define DOMAIN_PARAMS(dmn)                                  ROOT_SIG(DS) [domain(#dmn)]
    #define GEOMETRY_MAIN_DECLARE_OUTPUT        cOutput Out; 
    #define GEOMETRY_MAIN_SRT(MAX_VERTS)		UNIFORMS_CB     ROOT_SIG(GS) [maxvertexcount(MAX_VERTS)]	       void main(triangle cInput In[3],inout TriangleStream<cOutput> TriStream UNIFORMS_SRT)

  #endif

    #define VOID_MAIN_REZ_SRT   				UNIFORMS_CB                                                     	ROOT_SIG(PS) FRAG_EDS FRAG void main(cInput In UNIFORMS_SRT)
    #define VOID_MAIN_SRT    					UNIFORMS_CB                                                     	ROOT_SIG(PS) FRAG_EDS void main(cInput In UNIFORMS_SRT)
	#define VOID_MAIN_COLOUR					UNIFORMS_CB struct cOutput { vec4  mColour : S_TARGET_OUTPUT; };	ROOT_SIG(PS) FRAG_EDS void main(cInput In, out cOutput Out )
	#define VERTEX_MAIN																								ROOT_SIG(VS) void main(cInput In, out cOutput Out )
    #define VERTEX_MAIN_SRT                     UNIFORMS_CB                                                         ROOT_SIG(VS) void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define VERTEX_MAIN_INSTANCED_SRT			UNIFORMS_CB															ROOT_SIG(VS) void main(cInput In, out cOutput Out, uint instanceID : S_INSTANCE_ID, uint gl_VertexIndex : S_VERTEX_ID UNIFORMS_SRT)
    #define VERTEX_MAIN_ID_SRT					UNIFORMS_CB															ROOT_SIG(VS) void main(cInput In, out cOutput Out, int vertexID : SV_VertexID UNIFORMS_SRT)
    #define HULL_TRI_MAIN_SRT					HULL_PARAMS(tri,integer,triangle_cw,3,32.0)		                    cOutput main(InputPatch<cInput, 3> In, uint uCPID : S_OUTPUT_CONTROL_POINT_ID UNIFORMS_SRT)
	#define HULL_QUAD_MAIN_SRT( PATCHCNT )		HULL_PARAMS(quad,integer,triangle_cw,4,32.0)		                cOutput main(InputPatch<cInput, PATCHCNT> In, uint uCPID : S_OUTPUT_CONTROL_POINT_ID UNIFORMS_SRT)
    #define DOMAIN_TRI_MAIN_SRT					UNIFORMS_CB DOMAIN_PARAMS(tri)										void main(HSConstantOutputData patchIn, const OutputPatch<cInput, 3> In, out cOutput Out, float3 domainCoordinates : S_DOMAIN_LOCATION UNIFORMS_SRT)
	#define DOMAIN_QUAD_MAIN_SRT				UNIFORMS_CB DOMAIN_PARAMS(quad)										void main(HSConstantOutputData patchIn, const OutputPatch<cInput, 4> In, out cOutput Out, float2 domainCoordinates : S_DOMAIN_LOCATION UNIFORMS_SRT)
	#define FRAGMENT_MAIN_COLOUR				UNIFORMS_CB struct cOutput { vec4  mColour : S_TARGET_OUTPUT; };									ROOT_SIG(PS) FRAG_EDS FRAG void main(cInput In, out cOutput Out UNIFORMS_SRT)
	#define FRAGMENT_MAIN_COLOUR_DEPTH			UNIFORMS_CB struct cOutput { vec4  mColour : S_TARGET_OUTPUT; float mDepth  : S_DEPTH_OUTPUT; };	ROOT_SIG(PS) FRAG_EDS FRAG void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define HULL_TRI_CONSTANTS_SRT              UNIFORMS_CB void ConstantsHS( InputPatch<cInput, 3> In, out HSConstantOutputData Out UNIFORMS_SRT )
    #define HULL_QUAD_CONSTANTS_SRT( PATCHCNT ) UNIFORMS_CB void ConstantsHS( InputPatch<cInput, PATCHCNT> In, out HSConstantOutputData Out UNIFORMS_SRT )

    // icky workaround for fxc compiler not supporting output parameters on hull shaders >:[
    #define DECLARE_HULL_OUTPUT cOutput Out
    #define RETURN_HULL_OUTPUT  return Out

    #define FRAGMENT_OUTPUTS_T1(  T0 )                  struct cOutput { T0  mOut0 : S_TARGET_OUTPUT0; };
    #define FRAGMENT_OUTPUTS_T2(  T0, T1 )              struct cOutput { T0  mOut0 : S_TARGET_OUTPUT0; T1  mOut1 : S_TARGET_OUTPUT1; };
    #define FRAGMENT_OUTPUTS_T3(  T0, T1, T2 )          struct cOutput { T0  mOut0 : S_TARGET_OUTPUT0; T1  mOut1 : S_TARGET_OUTPUT1; T2 mOut2 : S_TARGET_OUTPUT2; };
    #define FRAGMENT_OUTPUTS_T4(  T0, T1, T2, T3 )      struct cOutput { T0  mOut0 : S_TARGET_OUTPUT0; T1  mOut1 : S_TARGET_OUTPUT1; T2 mOut2 : S_TARGET_OUTPUT2; T3 mOut3 : S_TARGET_OUTPUT3; };

    #define FRAGMENT_MAIN_T1_SRT( T0 )                  UNIFORMS_CB FRAGMENT_OUTPUTS_T1(  T0 )             ROOT_SIG(PS) FRAG_EDS FRAG void main( cInput In, out cOutput Out UNIFORMS_SRT  )
    #define FRAGMENT_MAIN_T2_SRT( T0, T1 )              UNIFORMS_CB FRAGMENT_OUTPUTS_T2(  T0, T1 )         ROOT_SIG(PS) FRAG_EDS FRAG void main( cInput In, out cOutput Out UNIFORMS_SRT  )
    #define FRAGMENT_MAIN_T3_SRT( T0, T1, T2 )          UNIFORMS_CB FRAGMENT_OUTPUTS_T3(  T0, T1, T2 )     ROOT_SIG(PS) FRAG_EDS FRAG void main( cInput In, out cOutput Out UNIFORMS_SRT  )
    #define FRAGMENT_MAIN_T4_SRT( T0, T1, T2, T3 )      UNIFORMS_CB FRAGMENT_OUTPUTS_T4(  T0, T1, T2, T3 ) ROOT_SIG(PS) FRAG_EDS FRAG void main( cInput In, out cOutput Out UNIFORMS_SRT  )

  #if defined D_COMPUTE

    #define COMPUTE_MAIN_SRT( X, Y, Z )             UNIFORMS_CB COMPUTE_PARAMS(X, Y, Z) void main(uint3 groupID : S_GROUP_ID, uint3 groupThreadID : S_GROUP_THREAD_ID, uint3 dispatchThreadID : S_DISPATCH_THREAD_ID UNIFORMS_SRT)                                    
    #define COMPUTE_MAIN_UNIF( X, Y, Z, UNIF_TYPE ) UNIFORMS_CB_TYPE( UNIF_TYPE ) COMPUTE_PARAMS(X, Y, Z) void main(uint3 groupID : S_GROUP_ID, uint3 groupThreadID : S_GROUP_THREAD_ID, uint3 dispatchThreadID : S_DISPATCH_THREAD_ID UNIFORMS_SRT_TYPE( UNIF_TYPE ) ) 
    #define FRAGMENT_MAIN_COLOUR_SRT            COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR01_SRT          COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR012_SRT         COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR0123_SRT        COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR01234_SRT       COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT      COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT     COMPUTE_MAIN_SRT( 8,8,1 )
    #define FRAGMENT_MAIN_UICOLOUR_SRT          COMPUTE_MAIN_SRT( 8,8,1 )
    #define VOID_MAIN_DEPTH_SRT                 COMPUTE_MAIN_SRT( 8,8,1 )
    #define VOID_MAIN_DEPTH_STENCIL_SRT         COMPUTE_MAIN_SRT( 8,8,1 )

  #elif defined( D_TAA_RENDER_TARGETS )		   
											   
	#define FRAGMENT_MAIN_COLOUR_SRT			UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main(cInput In, out cOutput Out UNIFORMS_SRT)

  #elif defined( D_LIT_WITH_MASK )		   
    
    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT		UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; float mDepth : S_DEPTH_OUTPUT;    }; ROOT_SIG(PS) FRAG_EDS FRAG	  void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR_SRT			UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; };                                   ROOT_SIG(PS) FRAG_EDS          void main(cInput In, out cOutput Out UNIFORMS_SRT)
  
  #elif defined( D_REFRACT ) && !defined( D_REFRACT_HIGH )

    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT		UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; float mDepth   : S_DEPTH_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT  )
	#define FRAGMENT_MAIN_COLOUR_SRT			UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT )

  #elif defined( D_REFRACT ) &&  defined( D_REFRACT_HIGH )

    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT      UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; vec4 mColour4 : S_TARGET_OUTPUT4; vec4 mColour5 : S_TARGET_OUTPUT5; float mDepth   : S_DEPTH_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT  )
    #define FRAGMENT_MAIN_COLOUR_SRT            UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; vec4 mColour4 : S_TARGET_OUTPUT4; vec4 mColour5 : S_TARGET_OUTPUT5; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT )

  #elif !defined( D_ATTRIBUTES )

    #define FRAGMENT_MAIN_COLOUR_DEPTH_SRT      UNIFORMS_CB struct cOutput { vec4 mColour : S_TARGET_OUTPUT; float mDepth : S_DEPTH_OUTPUT;    }; ROOT_SIG(PS) FRAG_EDS FRAG	        void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT   UNIFORMS_CB struct cOutput { vec4 mColour : S_TARGET_OUTPUT; float mDepth : S_DEPTH_OUTPUT; uint mStencil : S_STENCIL_OP; }; ROOT_SIG(PS) FRAG_EDS FRAG           void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR_DEPTH_GE_SRT   UNIFORMS_CB struct cOutput { vec4 mColour : S_TARGET_OUTPUT; float mDepth : S_DEPTH_GE_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS FRAG	        void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR_DEPTH_LE_SRT   UNIFORMS_CB struct cOutput { vec4 mColour : S_TARGET_OUTPUT; float mDepth : S_DEPTH_LE_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS FRAG	        void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_UICOLOUR_SRT          UNIFORMS_CB struct cOutput { uint4 mColour : S_TARGET_OUTPUT; };	                              ROOT_SIG(PS) FRAG_EDS              void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR_SRT            UNIFORMS_CB struct cOutput { vec4  mColour : S_TARGET_OUTPUT; };	                              ROOT_SIG(PS) FRAG_EDS              void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT     UNIFORMS_CB struct cOutput { vec4  mColour : S_TARGET_OUTPUT; };                                  ROOT_SIG(PS) FRAG_EDS  [FORCE_EARLY_DEPTH_STENCIL] void main( cInput In, out cOutput Out UNIFORMS_SRT )
    #define VOID_MAIN_EARLYZ_SRT                UNIFORMS_CB struct cOutput {                                                                   }; ROOT_SIG(PS) FRAG_EDS  [FORCE_EARLY_DEPTH_STENCIL] void main( cInput In, out cOutput Out UNIFORMS_SRT )
    #define VOID_MAIN_DEPTH_SRT	                UNIFORMS_CB struct cOutput {                                 float mDepth : S_DEPTH_OUTPUT;    }; ROOT_SIG(PS) FRAG_EDS FRAG 	    void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define VOID_MAIN_DEPTH_STENCIL_SRT         UNIFORMS_CB struct cOutput { uint mStencil : S_STENCIL_OP; float mDepth : S_DEPTH_OUTPUT;    }; ROOT_SIG(PS) FRAG_EDS FRAG 	    void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define VOID_MAIN_COLOUR_EARLYZ_SRT         UNIFORMS_CB 	                                				               	 ROOT_SIG(PS) FRAG_EDS [FORCE_EARLY_DEPTH_STENCIL]   void main(cInput In UNIFORMS_SRT )
    #define FRAGMENT_MAIN_COLOUR01_SRT          UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; };  ROOT_SIG(PS) FRAG_EDS FRAG    void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR012_SRT         UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; };  ROOT_SIG(PS) FRAG_EDS FRAG    void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR0123_SRT        UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; };  ROOT_SIG(PS) FRAG_EDS FRAG    void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR01234_SRT       UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; mColour4 : S_TARGET_OUTPUT4; };  ROOT_SIG(PS) FRAG_EDS FRAG    void main(cInput In, out cOutput Out UNIFORMS_SRT)
    #define FRAGMENT_MAIN_COLOUR01_DEPTH_SRT    UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; float mDepth : S_DEPTH_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS FRAG void main( cInput In, out cOutput Out UNIFORMS_SRT  )
    #define FRAGMENT_MAIN_COLOUR012_DEPTH_SRT   UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; float mDepth : S_DEPTH_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS FRAG void main( cInput In, out cOutput Out UNIFORMS_SRT  )

  #else
        // #pragma PSSL_target_output_format(target 1 FMT_32_AR)
	#define FRAGMENT_MAIN_COLOUR_DEPTH_SRT		UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; float mDepth   : S_DEPTH_OUTPUT; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT  )

    #ifdef D_OUTPUT_LINEARDEPTH
    #ifdef D_PLATFORM_ORBIS
    #pragma PSSL_target_output_format(target 4 FMT_32_AR)
    #endif
    #define FRAGMENT_MAIN_COLOUR_SRT			UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3;  vec4  mColour4 : S_TARGET_OUTPUT4; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT )
    #else
	#define FRAGMENT_MAIN_COLOUR_SRT			UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; }; ROOT_SIG(PS) FRAG_EDS [RE_Z] void main( cInput In, out cOutput Out UNIFORMS_SRT )
    #endif
	#define FRAGMENT_MAIN_COLOUR_EARLYZ_SRT		UNIFORMS_CB struct cOutput { vec4  mColour0 : S_TARGET_OUTPUT0; vec4  mColour1 : S_TARGET_OUTPUT1; vec4  mColour2 : S_TARGET_OUTPUT2; vec4  mColour3 : S_TARGET_OUTPUT3; }; ROOT_SIG(PS) FRAG_EDS  [FORCE_EARLY_DEPTH_STENCIL] void main( cInput In, out cOutput Out UNIFORMS_SRT )                                                    
  #endif


#endif


bool WaveIsFirstLane()
{
#if defined( D_PLATFORM_GLSL) && defined( GL_ARB_shader_ballot ) && !defined( GL_SPIRV )
    return gl_SubGroupInvocationARB == readFirstInvocationARB( gl_SubGroupInvocationARB );
#elif defined( D_PLATFORM_PROSPERO ) && !defined D_COMPUTE_DISABLEWAVE32
    uint exec = wave32::__s_read_exec();
    uint lane_mask = exec ^ (exec&(exec - 1));
    return wave32::__v_cndmask_b32(0, 1, lane_mask);
#elif defined( D_PLATFORM_ORBIS )
    ulong exec = __s_read_exec();
    ulong lane_mask = exec^(exec&(exec-1));
    return __v_cndmask_b32(0, 1, lane_mask);
#elif defined( D_PLATFORM_XBOXONE )
    return __XB_GetLaneID() == __XB_S_FF1_U64(__XB_GetEntryActiveMask64());
#else
    return true;
#endif
}

// =================================================================================================
// Texture resolution
// =================================================================================================

#if defined( D_PLATFORM_GLSL )

    #define GetTexResolution( TEX )                 textureSize( TEX, 0 )
    #define GetTexResolutionLod( TEX, LOD )         textureSize( TEX, LOD )
    #define GetImgResolution( IMG )                 imageSize( IMG )


#elif defined(D_PLATFORM_METAL)

#define GetTexResolution( TEX ) vec2(TEX.get_width(), TEX.get_height())
#define GetTexResolutionLod( TEX, LOD )     vec2(TEX.get_width(LOD), TEX.get_height(LOD))
#define GetImgResolution( IMG ) vec2(IMG.get_width(), IMG.get_height())

#elif defined D_PLATFORM_DX12

uvec2 GetTexResolution(Texture2D lTexture)
{
    uvec2 lResolution;
    lTexture.GetDimensions(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetTexResolution(Texture2D<rgba32ui> lTexture )
{
    uvec2 lResolution;
    lTexture.GetDimensions(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetTexResolutionLod(Texture2D lTexture, int liLod)
{
    uvec2 lResolution;
    uint  luLevels;
    lTexture.GetDimensions(uint(liLod), lResolution.x, lResolution.y, luLevels);
    return lResolution;
}

uvec2 GetTexResolutionLod(Texture2D<rgba32ui> lTexture, int liLod)
{
    uvec2 lResolution;
    uint  luLevels;
    lTexture.GetDimensions(uint(liLod), lResolution.x, lResolution.y, luLevels);
    return lResolution;
}

uvec2 GetImgResolution(RW_Texture2D<float4> lTexture)
{
    uvec2 lResolution;
    lTexture.GetDimensions(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetImgResolution(RW_Texture2D<int> lTexture)
{
    uvec2 lResolution;
    lTexture.GetDimensions(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetImgResolution(RW_Texture2D<uint> lTexture)
{
    uvec2 lResolution;
    lTexture.GetDimensions(lResolution.x, lResolution.y);
    return lResolution;
}

#elif defined D_PLATFORM_ORBIS

uvec2 GetTexResolution(Texture2D lTexture )
{
    uvec2 lResolution;
    lTexture.GetDimensionsFast(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetTexResolution(Texture2D<rgba32ui> lTexture )
{
    uvec2 lResolution;
    lTexture.GetDimensionsFast(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetTexResolutionLod(Texture2D lTexture, int liLod )
{
    uvec2 lResolution;
    uint  luLevels;
    lTexture.GetDimensions(uint(liLod), lResolution.x, lResolution.y, luLevels);
    return lResolution;
}

uvec2 GetTexResolutionLod(Texture2D<r32ui> lTexture, int liLod )
{
    uvec2 lResolution;
    uint  luLevels;
    lTexture.GetDimensions(uint(liLod), lResolution.x, lResolution.y, luLevels);
    return lResolution;
}

uvec2 GetTexResolutionLod(Texture2D<rgba32ui> lTexture, int liLod )
{
    uvec2 lResolution;
    uint  luLevels;
    lTexture.GetDimensions(uint(liLod), lResolution.x, lResolution.y, luLevels);
    return lResolution;
}

uvec2 GetImgResolution(RW_Texture2D<float4> lTexture )
{
    uvec2 lResolution;
    lTexture.GetDimensionsFast(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetImgResolution(RW_Texture2D<int> lTexture )
{
    uvec2 lResolution;
    lTexture.GetDimensionsFast(lResolution.x, lResolution.y);
    return lResolution;
}

uvec2 GetImgResolution(RW_Texture2D<r32ui> lTexture )
{
    uvec2 lResolution;
    lTexture.GetDimensionsFast(lResolution.x, lResolution.y);
    return lResolution;
}

#endif

// =================================================================================================
// Viewport
// =================================================================================================
#ifdef D_PLATFORM_OPENGL

	vec2 SCREENSPACE_AS_RENDERTARGET_UVS(vec2 A) { return A; }
	vec2 DSCREENSPACE_AS_RENDERTARGET_UVS(vec2 A) { return A; }

#else

	vec2 SCREENSPACE_AS_RENDERTARGET_UVS(vec2 A) { return vec2(A.x, 1.0 - A.y); }
	vec2 DSCREENSPACE_AS_RENDERTARGET_UVS(vec2 A) { return vec2(A.x, -A.y); }

#endif

#define D_DEPTH_CLEARVALUE (0.0)

// =================================================================================================
// Texture usage feedback
// =================================================================================================
#if defined( D_TEXTURE_FEEDBACK )
	
#ifdef D_PLATFORM_GLSL

layout(r32i) uniform  iimage2D gTexFeedbackImg;

void WriteTexFeedback( in int liCounter, in float liMip )
{
    if( liCounter != 0 )
    {
        #if defined( GL_ARB_shader_ballot ) && ( GL_ARB_shader_ballot == 1 )
        if( readFirstInvocationARB( gl_SubGroupInvocationARB ) == gl_SubGroupInvocationARB )
        #endif
        {
            int liIntMip = int(floor(liMip));
            //imageStore( gTexFeedbackImg, ivec2( liCounter, liIntMip ), ivec4(1,0,0,0) );
            imageAtomicAdd( gTexFeedbackImg, ivec2( liCounter, liIntMip ), int(1) );
        }
    }
}

vec4 Tex2dFeedback( in sampler2D lSamp, in int liCounter, in vec2 lCoords )
{
    float liLod = textureQueryLOD( lSamp, lCoords ).x;
    WriteTexFeedback( liCounter, liLod );
    return texture( lSamp, lCoords );
}

vec4 Tex2dLodFeedback( in sampler2D lSamp, in int liCounter, in vec2 lCoords, in float liLod )
{
    WriteTexFeedback( liCounter, liLod );
    return textureLod( lSamp, lCoords, liLod );
}

vec4 Tex2dArrayFeedback( in sampler2DArray lSamp, in int liCounter, in vec3 lCoords )
{
    float liLod = textureQueryLOD( lSamp, lCoords.xy ).x;
    WriteTexFeedback( liCounter, liLod );
    return texture( lSamp, lCoords );
}

vec4 Tex3dFeedback( in sampler3D lSamp, in int liCounter, in vec3 lCoords )
{
    float liLod = textureQueryLOD( lSamp, lCoords ).x;
    WriteTexFeedback( liCounter, liLod );
    return texture( lSamp, lCoords );
}

vec4 Tex3dLodFeedback( in sampler3D lSamp, in int liCounter, in vec3 lCoords, in float liLod )
{
    WriteTexFeedback( liCounter, liLod );
    return textureLod( lSamp, lCoords, liLod );
}

#else

RW_Texture2D<uint> gTexFeedbackImg;

void WriteTexFeedback( in int counter, in float mip )
{
    if ( counter != 0 )
		InterlockedAdd(gTexFeedbackImg[int2(counter, int(mip))], 1);
}

vec4 Tex2dFeedback(in Texture2D tex, in SamplerState samp, int counter, in vec2 coords )
{
    WriteTexFeedback( counter, tex.CalculateLevelOfDetail(samp, coords));
    return tex.Sample(samp, coords);
}

vec4 Tex2dLodFeedback(in Texture2D tex, in SamplerState samp, int counter, in vec2 coords, in float lod )
{
    WriteTexFeedback( counter, lod);
    return tex.SampleLOD(samp, coords, lod);
}

vec4 Tex2dArrayFeedback( in Texture2D tex, SamplerState samp, in int counter, in vec3 coords )
{
    WriteTexFeedback(counter, tex.CalculateLevelOfDetail(samp, coords));
    return tex.Sample(samp, coords);
}

vec4 Tex3dFeedback( in Texture3D tex, in SamplerState samp, in int counter, in vec3 coords )
{
    WriteTexFeedback( counter, tex.CalculateLevelOfDetail(samp, coords));
    return tex.Sample(samp, coords);
}

vec4 Tex3dLodFeedback( in Texture3D tex, in SamplerState samp, in int counter, in vec3 coords, in float lod )
{
    WriteTexFeedback( counter, lod);
    return tex.SampleLOD(samp, coords, lod);
}

#endif

#endif

#endif	//D_DEFINES
