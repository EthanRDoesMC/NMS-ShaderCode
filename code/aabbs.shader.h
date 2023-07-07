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

#if defined( D_INSTANCE )
#if defined( D_VERTEX )  && !defined ( D_PLATFORM_ORBIS ) && !defined ( D_PLATFORM_METAL )
REGULARBUFFER(sPerInstanceData, gaPerInstanceData, 0);
#endif

#if defined( D_INSTANCE_BASE_INDEX) && !defined ( D_PLATFORM_ORBIS ) && !defined ( D_PLATFORM_METAL )
RW_REGULARBUFFER(uint,             gaPerInstanceCull, 1);
#endif

DECLARE_UNIFORMS
    DECLARE_PTR( PerFrameUniforms,          mpPerFrame )       /*: PER_MATERIAL*/
    DECLARE_PTR( CommonPerMeshUniforms,     mpCommonPerMesh )  /*: PER_MESH*/
DECLARE_UNIFORMS_END


#ifdef D_VERTEX

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonDepth.shader.h"
#include "Common/CommonVertex.shader.h"


//-----------------------------------------------------------------------------
//      Global Data
STATIC_CONST ivec3 kaBBoxMinMaxIdx[ 8 ] =
{
    ivec3( 0, 0, 1 ),
    ivec3( 1, 0, 1 ),
    ivec3( 1, 1, 1 ),
    ivec3( 0, 1, 1 ),
    ivec3( 0, 0, 0 ),
    ivec3( 1, 0, 0 ),
    ivec3( 1, 1, 0 ),
    ivec3( 0, 1, 0 ),
};

STATIC_CONST float kfDistMargin = 1.0;

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

 

DECLARE_INPUT_END


DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION

    flat OUTPUT( int, miInstanceIdx, TEXCOORD0 )
DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
//    Functions

#if defined ( D_PLATFORM_METAL )
VERTEX_MAIN_INSTANCED_ID_SRT
#else
VERTEX_MAIN_INSTANCED_SRT
#endif
{
    mat4  lWorldMat4;
    vec4  lLocalPositionVec4;
    vec4  lWorldPositionVec4;
    vec4  lScreenPositionVec4;
    vec3  laBBoxVec4[ 2 ];
    ivec3 lBBoxIdxVec3;


    int              liInstanceIdx = int( lUniforms.mpCommonPerMesh.guInstBaseIndex ) + instanceID;
    sPerInstanceData lInstanceData = GETBUFFERDATA( lUniforms.mpCommonPerMesh, gaPerInstanceData, liInstanceIdx );

    lWorldMat4[ 0 ] = vec4( lInstanceData.mkTransformMat4R0.xyz, 0.0 );
    lWorldMat4[ 1 ] = vec4( lInstanceData.mkTransformMat4R1.xyz, 0.0 );
    lWorldMat4[ 2 ] = vec4( lInstanceData.mkTransformMat4R2.xyz, 0.0 );
    lWorldMat4[ 3 ] = vec4( lInstanceData.mkTransformMat4R3.xyz, 1.0 );

    lWorldMat4 = PLATFORM_TRANSPOSE( lWorldMat4 );

#if !defined ( D_PLATFORM_METAL ) && !defined ( D_PLATFORM_GLSL )
    lBBoxIdxVec3            = kaBBoxMinMaxIdx[ gl_VertexIndex ];
#else
    lBBoxIdxVec3            = kaBBoxMinMaxIdx[ vertexID ];
#endif

    laBBoxVec4[ 0 ]         = lUniforms.mpCommonPerMesh.gAabbMinVec4.xyz;
    laBBoxVec4[ 1 ]         = lUniforms.mpCommonPerMesh.gAabbMaxVec4.xyz;

    lLocalPositionVec4.x    = laBBoxVec4[ lBBoxIdxVec3.x ].x;
    lLocalPositionVec4.y    = laBBoxVec4[ lBBoxIdxVec3.y ].y;
    lLocalPositionVec4.z    = laBBoxVec4[ lBBoxIdxVec3.z ].z;
    lLocalPositionVec4.w    = 1.0;

    lWorldPositionVec4      = CalcWorldPosInstanced ( lUniforms.mpCommonPerMesh.gWorldMat4, lWorldMat4, lLocalPositionVec4 );
    lScreenPositionVec4     = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );

    if ( lScreenPositionVec4.w < kfDistMargin )
    {
        // The box is very close to the camera, and the camera might be inside it
        // when that happens no fragment shader is invoked and the box can be mistakenly
        // classified as 'not seen'. To avoid this, we always classify instances very close
        // to the camera as 'seen/not occluded'.
        GETBUFFERDATA( lUniforms.mpCommonPerMesh, gaPerInstanceCull, liInstanceIdx ) = 1;
    }

    OUT( miInstanceIdx )    = liInstanceIdx;

    SCREEN_POSITION         = lScreenPositionVec4;

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

    flat INPUT( int, miInstanceIdx, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions

VOID_MAIN_EARLYZ_SRT
{
    int liInstanceIdx = IN( miInstanceIdx );

    GETBUFFERDATA( lUniforms.mpCommonPerMesh, gaPerInstanceCull, liInstanceIdx ) = 1;

    //FRAGMENT_COLOUR = vec4( 1.0, 0.0, 0.0, 0.05 );
}

#endif

#endif // D_INSTANCE
