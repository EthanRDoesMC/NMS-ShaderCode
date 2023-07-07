////////////////////////////////////////////////////////////////////////////////
///
///     @file       
///     @author     User
///     @date       
///
///     @brief      
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

DECLARE_UNIFORMS
    DECLARE_PTR( PerFrameUniforms,          mpPerFrame )       /*: PER_MATERIAL*/
    DECLARE_PTR( CommonPerMeshUniforms,     mpCommonPerMesh )           /*: PER_MESH*/
DECLARE_UNIFORMS_END


#ifdef D_VERTEX

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

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
    INPUT(  vec3, mkLocalPositionVec3, POSITION0 )
DECLARE_INPUT_END

DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE

    OUTPUT( vec4, mSceenPosVec4, TEXCOORD0 )
DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END


//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------


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
    vec4 lWorldPositionVec4       = MUL( lUniforms.mpCommonPerMesh.gWorldMat4, vec4( IN( mkLocalPositionVec3 ), 1.0) );
    vec4 lScreenSpacePositionVec4 = MUL( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );

    OUT(mSceenPosVec4)       = lScreenSpacePositionVec4;

    SCREEN_POSITION   = lScreenSpacePositionVec4;
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
}


#endif

#ifdef D_FRAGMENT

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec4, mSceenPosVec4, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
           
FRAGMENT_MAIN_COLOUR_SRT
{
    FRAGMENT_COLOUR = vec4( 1.0, 0.0, 0.0, 1.0 );
}

#endif


