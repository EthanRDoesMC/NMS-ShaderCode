////////////////////////////////////////////////////////////////////////////////
///
///     @file       RefractionFragment.h
///     @author     strgiu
///     @date       
///
///     @brief      RefractionFragmentShader
///
///     Copyright (c) 2021 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Fullscreen/PostCommon.h"
#include "Common/Common.shader.h"
#include "Common/CommonPostProcess.shader.h"
#include "Common/CommonDepth.shader.h"

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_REFRACT
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_REFRACT

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//STATIC_CONST float  REFRACT_THICKNESS   = 0.0001;

vec4
GetUVCoordsFromWorld(
    in PerFrameUniforms lPerFrameUniforms,
    in vec3             lPosWorldVec3 )
{
    vec4    lUVsVec4    = vec4( lPosWorldVec3, 1.0 );
    lUVsVec4            = MUL( lPerFrameUniforms.gViewProjectionMat4, lUVsVec4 );
    lUVsVec4.xyz       /= lUVsVec4.w;
    lUVsVec4.xy         = SCREENSPACE_AS_RENDERTARGET_UVS( lUVsVec4.xy * 0.5 + 0.5 );

    return lUVsVec4;
}

FRAGMENT_MAIN_COLOUR012_SRT
{
    vec2  lUVsVec2      = TEX_COORDS.xy;
    
    float lfAlpha       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsVec2, 0.0 ).r;

    vec3  lRefrDirVec3  = DecodeVec3FromR11G11B10( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2, 0.0 ).rgb );
    float lfRefrScale   = dot( lRefrDirVec3.xy, lRefrDirVec3.xy );
    float lfDepthStart  = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsVec2, 0.0 ) );
    vec3  lPosStartVec3 = RecreatePositionFromRevZDepth( lfDepthStart, lUVsVec2, lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpPerFrame.gInverseViewProjectionMat4  );
    vec3  lPosEndVec3   = lPosStartVec3 + lRefrDirVec3;
    vec3  lUVsStartVec3 = vec3( lUVsVec2, lfDepthStart );
    vec3  lUVsEndVec3   = GetUVCoordsFromWorld( DEREF_PTR( lUniforms.mpPerFrame ), lPosEndVec3 ).xyz;
    float lfDepthEnd    = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lUVsEndVec3.xy, 0.0 ) );
    
    float lfRefrBlend   = 1.0;
    lfRefrBlend         = lUVsEndVec3.z < lfDepthEnd ? 0.0 : 1.0;
    lfRefrBlend        *= 1.0 - smoothstep(0.0, 0.0525, max(abs(lUVsEndVec3.x - 0.5) - 0.45, 0.0));
    lfRefrBlend        *= 1.0 - smoothstep(0.0, 0.0525, max(abs(lUVsEndVec3.y - 0.5) - 0.45, 0.0));
    lfRefrBlend        *= smoothstep( 0.0, 0.001, lfRefrScale );
    lfRefrBlend         = min( lfRefrBlend, 0.85 );

    vec3  lColourStart  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2,       0.0 ).rgb;
    vec3  lColourEnd    = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsEndVec3.xy, 0.0 ).rgb;

    vec3  lRefrColour   = mix( lColourStart, lColourEnd, lfRefrBlend );
   
    vec3  lEncUVsVec2   = EncodeVec2ToR11G11B10( lUVsEndVec3.xy );

    WRITE_FRAGMENT_COLOUR0( vec4( lRefrColour,    1.0 - lfAlpha ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lEncUVsVec2,    1.0 ) );
    WRITE_FRAGMENT_COLOUR2( vec4( lfRefrBlend,    0.0,  0.0, 1.0 ) );
}
#endif


// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_REFRACT_BEHIND
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_REFRACT_BEHIND

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lUVsVec2      = TEX_COORDS.xy;
    
    vec3  lSrcColVec3   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2, 0.0 ).rgb;
    float lfSrcAlpha    = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsVec2, 0.0 ).r;
    vec3  lSrcColBVec3  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2, 0.0 ).rgb;
    float lfSrcAlphaB   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsVec2, 0.0 ).r;
    
    vec3  lColFinal     = ( lSrcColBVec3 - lfSrcAlpha * lSrcColVec3 ) / max( 1.0 - lfSrcAlpha, 0.0000001 );
    float lfAlphaFinal  = ( lfSrcAlphaB  - lfSrcAlpha )               / max( 1.0 - lfSrcAlpha, 0.0000001 );

    WRITE_FRAGMENT_COLOUR( vec4( lColFinal, saturate( lfAlphaFinal ) ) );
}
#endif

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_REFRACT_DISTORT
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_REFRACT_DISTORT

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lUVsVec2          = TEX_COORDS.xy;    
    vec3  lUVsEndVec3       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsVec2,    0.0 ).rgb;
    vec2  lUVsDecVec2       = DecodeVec2FromR11G11B10( lUVsEndVec3 );
    float lfBlend           = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2,    0.0 ).r;

    vec3  lColourVec3       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2,    0.0 ).rgb;
    vec3  lColourDistVec3   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsDecVec2, 0.0 ).rgb;
    
    vec3  lColFinal         = mix( lColourVec3, lColourDistVec3, lfBlend );

    #if defined( D_BLEND_ADD ) && defined( D_COMPUTE )
    WRITE_FRAGMENT_COLOUR( vec4( FRAGMENT_COLOUR + lColFinal, 1.0 ) );
    #else
    WRITE_FRAGMENT_COLOUR( vec4( lColFinal, 1.0 ) );
    #endif
}
#endif