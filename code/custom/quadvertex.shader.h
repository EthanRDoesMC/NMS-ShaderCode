////////////////////////////////////////////////////////////////////////////////
///
///     @file       QuadVertex.h
///     @author     User
///     @date       
///
///     @brief      TerrainVertexShader
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
#include "Common/CommonDepth.shader.h"
#include "Common/CommonVertex.shader.h"

//
// SRT Buffer
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )       /*: PER_MATERIAL*/
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )           /*: PER_MESH*/
DECLARE_UNIFORMS_END

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
    INPUT( vec4, mkLocalNormalVec4,   NORMAL0       )
    INPUT( vec4, mkCustom1Vec4,       BLENDINDICES  )

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
    OUTPUT( vec4,   mWorldPositionVec4,         TEXCOORD1 )
    OUTPUT( vec3,   mWorldNormalVec3,           TEXCOORD2 )
    OUTPUT( vec2,   mTexCoordsVec2,             TEXCOORD3 )

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
    vec4 lScreenSpacePositionVec4;
    vec4 lWorldPositionVec4;
    vec4 lLocalPositionVec4;
	vec3 lLocalNormalVec3;
    vec3 lWorldNormalVec3;
    
	lLocalNormalVec3            = IN( mkLocalNormalVec4 ).xyz;
    lWorldNormalVec3            = IN( mkLocalNormalVec4 ).xyz;
    lLocalPositionVec4          = vec4( IN( mkLocalPositionVec4.xyz ), 1.0 );
    lWorldPositionVec4          = MUL( lUniforms.mpCommonPerMesh.gWorldMat4, lLocalPositionVec4 );

    OUT( mTexCoordsVec2 )       = IN( mkCustom1Vec4 ).zw;
    OUT( mWorldNormalVec3 )     = lWorldNormalVec3;
    OUT( mWorldPositionVec4 )   = lWorldPositionVec4;

    lScreenSpacePositionVec4        = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );

    SCREEN_POSITION          = lScreenSpacePositionVec4;
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
}