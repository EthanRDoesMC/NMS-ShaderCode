#define D_PARTICLE_UNIFORMS

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif
#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "Custom/TrailCommon.h"


//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec4,  mfTexCoord_Diffuse_Distortion, TEXCOORD1 )
    INPUT( vec4,  mfTexCoord_Alpha,              TEXCOORD2 )
    INPUT( float, mfFade,                        TEXCOORD3 )

DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lCol;
    vec2 lDiffuseTexCoords = IN( mfTexCoord_Diffuse_Distortion ).xy;

#if defined( _F31_DISPLACEMENT )
    vec2 lDisplacementVec2 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDistortionMap ), IN( mfTexCoord_Diffuse_Distortion.zw ) ).xy;
    lDisplacementVec2 *= lUniforms.mpCustomPerMaterial.gDistortionScale_Unused.x * 4.0;
    lDiffuseTexCoords += lDisplacementVec2;
#endif

    lCol        = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lDiffuseTexCoords );
    lCol.w     *= IN( mfFade );

#if !defined( D_GLOW )
    lCol.rgb   *= 1.0 + lUniforms.mpCustomPerMaterial.gMinPixelSize_Glow.y;
#else
    lCol.a     *= 1.0 + lUniforms.mpCustomPerMaterial.gMinPixelSize_Glow.y;
#endif

#if defined( _F14_UVSCROLL )
    // Additional alpha texture (mainly used for fading near emitter, independently of diffuse alpha).
    vec2  lAlphaTexCoords = IN( mfTexCoord_Alpha.xy );
    float lfAlphaMultiplier = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gAlphaMap ), lAlphaTexCoords.xy ).r;

    lCol.a *= lfAlphaMultiplier;
#endif

    FRAGMENT_COLOUR = lCol;
}