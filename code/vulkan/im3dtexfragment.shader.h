////////////////////////////////////////////////////////////////////////////////
///
///     @file       Im3dFragment.shader.h
///     @author     
///     @date       
///
///     @brief      Immediate Mode Fragment
///
///     Copyright (c) 2009 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////
#ifndef D_FRAGMENT
#define D_FRAGMENT
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
               
    INPUT( vec2, UV             , TEXCOORD0 )
    INPUT( vec4, Color          , COLOR )
#if defined( D_USING_LOGDEPTH )
    INPUT( vec2, mDepthVal      , TEXCOORD1 )
#endif
DECLARE_INPUT_END



//StructuredBuffer<UniformBuffer> lUniforms : register(t0, space1);  

#if defined( D_USING_LOGDEPTH )
FRAGMENT_MAIN_COLOUR_DEPTH_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{                                                  
    vec4 lAlbedo;

    if ( lUniforms.mpCommonPerMesh.gImmediateRenderParams.x >= 0.0f )
    {
        lAlbedo = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCommonPerMesh, gColourMap ), vec2( IN( UV ).x, 1.0 - IN( UV ).y ), lUniforms.mpCommonPerMesh.gImmediateRenderParams.x );
    }
    else
    {
        lAlbedo = texture2D( SAMPLER_GETMAP( lUniforms.mpCommonPerMesh, gColourMap ), vec2( IN( UV ).x, 1.0 - IN( UV ).y ) );
    }

    lAlbedo *= IN( Color ); 

    FRAGMENT_COLOUR = lAlbedo;
#if defined( D_USING_LOGDEPTH )
    FRAGMENT_DEPTH  = log2( In.mDepthVal.x ) * IN( mDepthVal ).y;
#endif
}      

