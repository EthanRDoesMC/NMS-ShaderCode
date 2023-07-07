
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


#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"


struct CommonPerMeshUniforms
{
    vec4 gImmediateRenderParams MTL_ID(0);
    mat4 gWorldViewProjectionMat4;

BEGIN_SAMPLERBLOCK

	SAMPLER2D( gColourMap );

END_SAMPLERBLOCK

DECLARE_UNIFORMS
     DECLARE_PTR( CommonPerMeshUniforms, mpCommonPerMesh )       /*: PER_MESH*/
DECLARE_UNIFORMS_END

                                                    
DECLARE_INPUT

    INPUT( vec4, mkLocalPositionVec4 , POSITION0 )            // Object space position      
    INPUT( vec2, mkTexCoordsVec4     , TEXCOORD0 )            // UVs
    INPUT( vec4, mkColourVec4        , NORMAL0   )            // Vertex color     

DECLARE_INPUT_END
	
DECLARE_OUTPUT
    OUTPUT( vec4, Position       , REG_POSITION )
    OUTPUT( vec2, UV             , TEXCOORD0 )
    OUTPUT( vec4, Color          , NORMAL0   )
#if defined( D_USING_LOGDEPTH )
    OUTPUT( vec2, mDepthVal      , TEXCOORD1 )
#endif
DECLARE_OUTPUT_END                                       

VERTEX_MAIN_SRT
{

    vec4 pos = IN( mkLocalPositionVec4 );                         
    pos.w = 1.0;       

    vec4 lScreenSpacePositionVec4 = MUL( lUniforms.mpCommonPerMesh.gWorldViewProjectionMat4, pos ); 

    OUT( Position ) = lScreenSpacePositionVec4; 
    OUT( Color )    = IN( mkColourVec4 ); 
    OUT( UV )       = IN( mkTexCoordsVec4 );                                    
#if defined( D_USING_LOGDEPTH )
    const float lfFarClipPlane = 5000000.0;
    Out.mDepthVal.x = lScreenSpacePositionVec4.z+ lUniforms.mpCommonPerMesh.gImmediateRenderParams.y;  
    Out.mDepthVal.y = 1.0 / log2( lfFarClipPlane + 1.0 );
#endif
 }                                                 


