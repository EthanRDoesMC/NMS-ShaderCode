#ifndef D_COMPUTE_CULL_COMMON_H
#define D_COMPUTE_CULL_COMMON_H

#include "Common/Defines.shader.h"

// NOTE(sal): this needs to match the struct in CommonUniforms.shader.h
struct sPerInstanceData
{
    vec4 mkTransformMat4R0;
    vec4 mkTransformMat4R1;
    vec4 mkTransformMat4R2;
    vec4 mkTransformMat4R3;
    vec4 mkTransformMat4R4;
    vec4 mkTransformMat4R5;
};

struct sIndexedIndirectBuffer
{
    uint    muIndexCount;
#if defined ( D_PLATFORM_METAL )	
    atomic_int muInstanceCount;
#else
    uint    muInstanceCount;
#endif
    uint    muFirstIndex;
    int     miVertexOffset;
    uint    muFirstInstance;
    uint    maPadding[ 3 ];
};

#define GLOBALS( NAME )  NAME

#if !defined ( D_PLATFORM_ORBIS ) && !defined ( D_PLATFORM_METAL )
REGULARBUFFER(   sPerInstanceData,          gaSrcInstanceData, 0);
RW_REGULARBUFFER(sPerInstanceData,          gaDstInstanceData, 1);
RW_REGULARBUFFER(sIndexedIndirectBuffer,    gaIndirectCmdData, 2);
REGULARBUFFER(   uint,                      gaFlgInstanceData, 3);
REGULARBUFFER(   uint,                      gaIndirectIndices, 4);
#endif

struct PerDispatchUniforms
{
	uint guInstBaseIndex;
	uint guInstCount;

	#if defined( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_METAL )
	REGULARBUFFER(   sPerInstanceData,          gaSrcInstanceData, 0);
	RW_REGULARBUFFER(sPerInstanceData,          gaDstInstanceData, 1);
	RW_REGULARBUFFER(sIndexedIndirectBuffer,    gaIndirectCmdData, 2);
	REGULARBUFFER(   uint,                      gaFlgInstanceData, 3);
	REGULARBUFFER(   uint,                      gaIndirectIndices, 4);
	#endif

};

DECLARE_UNIFORMS
    DECLARE_PTR(PerDispatchUniforms, mpPerDispatchUniforms)
DECLARE_UNIFORMS_END

#endif