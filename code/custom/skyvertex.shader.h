////////////////////////////////////////////////////////////////////////////////
///
///     @file       SkyVertex.h
///     @author     User
///     @date       
///
///     @brief      SkyVertexShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files


#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Custom/SkyCommon.h"

#include "Common/CommonVertex.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonDepth.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT

    INPUT( vec4, mkLocalPositionVec4, POSITION0 )
    INPUT( vec4, mkTexCoordsVec4,     TEXCOORD0     )
    INPUT( vec4, mkLocalNormalVec4,   TEXCOORD1     )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------
DECLARE_OUTPUT

    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE

    OUTPUT( vec4, mWorldPositionVec4,        TEXCOORD0 )
    OUTPUT( vec4, mLocalPositionVec4,        TEXCOORD1 )
    OUTPUT( vec4, mScreenSpacePositionVec4,  TEXCOORD2 )
#if defined( D_OUTPUT_MOTION_VECTORS )
    OUTPUT( vec4, mPrevScreenSpacePositionVec4, TEXCOORD3 )
#endif	
#if defined ( D_OUTPUT_MOTION_VECTORS ) && defined( D_SKY_CUBE )
    OUTPUT_VARIANT( vec4, mPrevScreenPosition,       TEXCOORD17, HAS_MOTION_VECTORS )
#endif

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Vertex Main
///
///     @brief      Vertex Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
VERTEX_MAIN_SRT
{
    vec3 lDirectionVec3;
    vec4 lWorldPositionVec4;
    vec4 lPrevWorldPositionVec4;
    vec4 lFlatWorldPositionVec4;
    vec3 lWorldNormalVec3;

    lWorldPositionVec4        = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, vec4( IN( mkLocalPositionVec4 ).xyz, 1.0 ) );
#if defined( D_OUTPUT_MOTION_VECTORS )
    lPrevWorldPositionVec4    = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMotionMat4, vec4( IN( mkLocalPositionVec4 ).xyz, 1.0 ));
#endif
    OUT( mLocalPositionVec4 ) = IN( mkLocalPositionVec4 );
    OUT( mWorldPositionVec4 ) = lWorldPositionVec4;
    //OUT( mSunDirectionVec3 )  = normalize( lWorldPositionVec4.xyz - lUniforms.mpPerFrame.gViewPositionVec3 );
    //OUT( mTexCoordsVec2 )     = IN( mkTexCoordsVec4 ).xy;

    #ifdef D_REFLECT
    {
        vec4 lScreenSpacePositionVec4 = CalcDualParaboloidScreenPosition( lUniforms.mpPerFrame.gViewMat4, lWorldPositionVec4, lUniforms.mpPerFrame.gClipPlanesVec4.xy );
        OUT(mScreenSpacePositionVec4) = lScreenSpacePositionVec4;
        SCREEN_POSITION = lScreenSpacePositionVec4;
    }
    #else
    {   
        vec4 lScreenSpacePositionVec4 = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gViewMat4, lUniforms.mpPerFrame.gProjectionNoJitterMat4, lWorldPositionVec4);
        //vec4 lScreenSpacePositionVec4 = MUL(lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4);
        //lScreenSpacePositionVec4.xy -= UNIFORM(mpPerFrame, gDeJitterVec3.xy) * lScreenSpacePositionVec4.w;

        #if defined ( D_OUTPUT_MOTION_VECTORS ) && defined( D_SKY_CUBE )
    #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
    #endif
        {
            OUT(mPrevScreenPosition) = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gPrevViewProjectionMat4, lWorldPositionVec4);
        }
        #endif
        OUT( mScreenSpacePositionVec4 ) = lScreenSpacePositionVec4;
#if defined( D_OUTPUT_MOTION_VECTORS )
        OUT( mPrevScreenSpacePositionVec4) = MUL(lUniforms.mpPerFrame.gPrevViewProjectionNoJitterMat4, lPrevWorldPositionVec4);
        //OUT(mPrevScreenSpacePositionVec4).xy -= UNIFORM(mpPerFrame, gDeJitterVec3.xy) * OUT(mPrevScreenSpacePositionVec4).w;
#endif
        SCREEN_POSITION = lScreenSpacePositionVec4;

        WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
    }
    #endif
}