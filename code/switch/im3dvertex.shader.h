////////////////////////////////////////////////////////////////////////////////
///
///     @file       Im3dVertex.shader.h
///     @author     
///     @date       
///
///     @brief      Immediate Mode Vertex
///
///     Copyright (c) 2009 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#version 440 core
#extension GL_NV_gpu_shader5 : require
#extension GL_ARB_separate_shader_objects : require

#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"

struct CommonPerMeshUniforms
{
    vec4 gImmediateRenderParams;
    mat4 gWorldViewProjectionMat4;

BEGIN_SAMPLERBLOCK
	SAMPLER2D( gColourMap );
END_SAMPLERBLOCK

DECLARE_UNIFORMS
     DECLARE_PTR( CommonPerMeshUniforms, mpCommonPerMesh )       /*: PER_MESH*/
DECLARE_UNIFORMS_END
                                                    
DECLARE_INPUT
    INPUT( vec4, mkLocalPositionVec4 , 0 )            // Object space position      
    INPUT( vec2, mkTexCoordsVec4     , 1 )            // UVs
    INPUT( vec4, mkColourVec4        , 2 )            // Vertex color
DECLARE_INPUT_END
	
DECLARE_OUTPUT
    OUTPUT( vec2, UV             , TEXCOORD0 )
    OUTPUT( vec4, Color          , COLOR )
#if defined( D_USING_LOGDEPTH )
    OUTPUT( vec2, mDepthVal      , TEXCOORD1 )
#endif
DECLARE_OUTPUT_END                                       

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
    OUTPUT_SCREEN_POSITION_REDECLARED
DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

VERTEX_MAIN_SRT
{        
    float lfFarClipPlane = 5000000.0;
     
    vec4 pos = IN( mkLocalPositionVec4 );                         
    pos.w = 1.0;       

    vec4 lScreenSpacePositionVec4 = MUL( lUniforms.mpCommonPerMesh.gWorldViewProjectionMat4, pos );

    SCREEN_POSITION = lScreenSpacePositionVec4; 
    OUT( Color )   = IN( mkColourVec4 ); 
    OUT( UV )      = IN( mkTexCoordsVec4 );                                    
#if defined( D_USING_LOGDEPTH )
    Out.mDepthVal.x = lScreenSpacePositionVec4.z+ lUniforms.mpCommonPerMesh.gImmediateRenderParams.y;  
    Out.mDepthVal.y = 1.0 / log2( lfFarClipPlane + 1.0 );
#endif
}                                                 
