////////////////////////////////////////////////////////////////////////////////
///
///     @file       GTAOFragment.shader.h
///     @author     User
///     @date       
///
///     @brief      Ground Truth Ambient Occlusion
///
///     Copyright (c) 2019 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

//-----------------------------------------------------------------------------
//      Include files
#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/Common.shader.h"


//-----------------------------------------------------------------------------
///
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///     Stuff that is only used for these types of meshes.
//-----------------------------------------------------------------------------

#include "Fullscreen/PostCommon.h"

#include "Common/CommonPostProcess.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonGBuffer.shader.h"

//-----------------------------------------------------------------------------
//      Const Data


// =================================================================================================
//
// GTAO Apply
//
// =================================================================================================

#ifdef D_GTAO_APPLY

#include "Common/CommonGTAO.shader.h"

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION

INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#if defined( D_PLATFORM_ORBIS )
#pragma argument(targetoccupancy_atallcosts=90)
#pragma argument(unrollallloops)
#elif defined( D_PLATFORM_DX12 )
//#define __XBOX_REGALLOC_FORCE_VGPR_LIMIT 64
#else
#pragma optionNV(unroll all)
#endif





//-----------------------------------------------------------------------------
//      Functions 


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoordsVec2 = TEX_COORDS;

#ifdef D_PLATFORM_ORBIS
    if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
        if (HmdFovMask_isInvisible(lFragCoordsVec2,
            lUniforms.mpPerFrame.gFrustumTanFovVec4,
            lUniforms.mpPerFrame.gVREyeInfoVec3)
            )
        {
            WRITE_FRAGMENT_COLOUR( vec4(1.0, 0.0, 0.0, 0.0) );
            return;
        }
    }
#endif

    vec4 lFragCol = GTAO(
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer1Map),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer2Map),
        lFragCoordsVec2,
        lUniforms.mpPerFrame.gFoVValuesVec4,
        lUniforms.mpPerFrame.gFrameBufferSizeVec4,
        lUniforms.mpPerFrame.gCameraMat4,
        lUniforms.mpPerFrame.gInverseProjectionSCMat4,
        lUniforms.mpPerFrame.gClipPlanesVec4,
        lUniforms.mpPerFrame.giFrameIndex
    );

    WRITE_FRAGMENT_COLOUR( lFragCol );
}


#endif // D_GTAO_APPLY




// =================================================================================================
//
// GTAO Temporal Filter
//
// =================================================================================================

#ifdef D_GTAO_TEMPORAL_FILTER

#include "Common/CommonGTAOTemporalFilter.shader.h"

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION

INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#if defined( D_PLATFORM_ORBIS )
#pragma argument(targetoccupancy_atallcosts=90)
#pragma argument(unrollallloops)
#elif defined( D_PLATFORM_DX12 )
//#define __XBOX_REGALLOC_FORCE_VGPR_LIMIT 64
#else
//#pragma optionNV(unroll all)
#endif





//-----------------------------------------------------------------------------
//      Functions 


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoordsVec2 = TEX_COORDS;

#ifdef D_PLATFORM_ORBIS
    if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
        if (HmdFovMask_isInvisible(lFragCoordsVec2,
            lUniforms.mpPerFrame.gFrustumTanFovVec4,
            lUniforms.mpPerFrame.gVREyeInfoVec3)
            )
        {
            WRITE_FRAGMENT_COLOUR( vec4(1.0, 0.0, 0.0, 0.0) );
            return;
        }
    }
#endif
#ifdef D_PLATFORM_VULKAN
    if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
        vec3 lNormalVec3 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBufferMap, lFragCoordsVec2).xyz;
        if (lNormalVec3.x == 0.0 && lNormalVec3.y == 0.0 && lNormalVec3.z == 0.0)
        {
#ifdef D_COMPUTE
            return;
#else
            discard;
#endif
        }
    }
#endif

    vec4 lFragCol = GTAOTemporalFilter(
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer1Map),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer2Map),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer3Map),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer4Map),
        lFragCoordsVec2,
        lUniforms.mpPerFrame.gClipPlanesVec4,
        lUniforms.mpPerFrame.gFrameBufferSizeVec4
    );

    WRITE_FRAGMENT_COLOUR( lFragCol );
}


#endif // D_GTAO_TEMPORAL_FILTER


// =================================================================================================
//
// GTAO Spatial Denoise
//
// =================================================================================================

#ifdef D_GTAO_SPATIAL_DENOISE

#include "Common/CommonGTAOSpatialFilter.shader.h"

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION

INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END



//-----------------------------------------------------------------------------
//      Functions 


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoordsVec2 = TEX_COORDS;

    #ifdef D_PLATFORM_ORBIS
    if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
        if (HmdFovMask_isInvisible(lFragCoordsVec2,
            lUniforms.mpPerFrame.gFrustumTanFovVec4,
            lUniforms.mpPerFrame.gVREyeInfoVec3)
            )
        {
            WRITE_FRAGMENT_COLOUR( vec4(1.0, 0.0, 0.0, 0.0) );
            return;
        }
    }
    #endif
#ifdef D_PLATFORM_VULKAN
    if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
        vec3 lNormalVec3 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBufferMap, lFragCoordsVec2).xyz;
        if (lNormalVec3.x == 0.0 && lNormalVec3.y == 0.0 && lNormalVec3.z == 0.0)
        {
#ifdef D_COMPUTE
            return;
#else
            discard;
#endif
        }
    }
#endif

    vec4 lFragCol = GTAOSpatialFilter(
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer1Map),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer2Map),
        lFragCoordsVec2,
        lUniforms.mpPerFrame.gFrameBufferSizeVec4,
        lUniforms.mpPerFrame.gClipPlanesVec4
    );

    WRITE_FRAGMENT_COLOUR( lFragCol );
}


#endif // D_GTAO_SPATIAL_DENOISE