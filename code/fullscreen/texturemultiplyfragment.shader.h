////////////////////////////////////////////////////////////////////////////////
///
///     @file       TextureMultiplyFragment.h
///     @author     User
///     @date       
///
///     @brief      TextureMultiplyFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif
#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Fullscreen/RecolourCommon.shader.h"

#include "Common/Common.shader.h"

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
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//    Functions

#ifdef D_MATCHTERRAIN
    #define RECOLOUR RecolourToMatchTerrain
#else
    #define RECOLOUR Recolour
#endif

#if defined ( D_HASALPHACHANNEL ) && defined ( D_RECOLOUR )

#define COMBINE_FN( controlId, layerId, arrayId ) \
    float lfLayer##layerId##Alpha = 0.0; \
    if ( lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ] > 0.0001 ) \
    { \
        vec4 lLayerColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gSource##layerId##Map ), lTexCoordsVec2, 0.0 ); \
        float lfAlpha = lLayerColourVec4.a; \
        \
        lfAlpha = mix( lfAlpha, 1.0, lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] ); \
        lfAlpha *= lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ]; \
        lLayerColourVec4.rgb = RECOLOUR( lLayerColourVec4.rgb, lUniforms.mpCustomPerMesh.gAverageColour##layerId##Vec4.rgb, lUniforms.mpCustomPerMesh.gRecolour##layerId##Vec4.rgb, lUniforms.mpCustomPerMesh.gRecolour##layerId##Vec4.a ); \
        lFragmentColourVec4 = mix( lFragmentColourVec4, lLayerColourVec4, lfAlpha ); \
        lfLayer##layerId##Alpha = lLayerColourVec4.a; \
    } \
    else if ( lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] > 0.0001 ) \
    { \
        lfLayer##layerId##Alpha = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gSource##layerId##Map ), lTexCoordsVec2, 0.0 ).a; \
    }

#elif defined ( D_HASALPHACHANNEL )

#define COMBINE_FN( controlId, layerId, arrayId ) \
    float lfLayer##layerId##Alpha = 0.0; \
    if ( lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ] > 0.0001 ) \
    { \
        vec4 lLayerColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gSource##layerId##Map ), lTexCoordsVec2, 0.0 ); \
        float lfAlpha = lLayerColourVec4.a; \
        \
        lfAlpha = mix( lfAlpha, 1.0, lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] ); \
        lfAlpha *= lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ]; \
        lFragmentColourVec4 = mix( lFragmentColourVec4, lLayerColourVec4, lfAlpha ); \
        lfLayer##layerId##Alpha = lLayerColourVec4.a; \
    } \
    else if ( lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] > 0.0001 ) \
    { \
        lfLayer##layerId##Alpha = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gSource##layerId##Map ), lTexCoordsVec2, 0.0 ).a; \
    }

#elif defined ( D_RECOLOUR )

#define COMBINE_FN( controlId, layerId, arrayId ) \
    float lfLayer##layerId##Alpha = 0.0; \
    if ( lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ] > 0.0001 ) \
    { \
        vec4 lLayerColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gSource##layerId##Map ), lTexCoordsVec2, 0.0 ); \
        float lfAlpha = lLayerColourVec4.a; \
        \
        lfAlpha = mix( 1.0, texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gAlpha##layerId##Map ), lTexCoordsVec2, 0.0 ).a, lUniforms.mpCustomPerMesh.gAlphaLayersUsed##controlId##Vec4[ arrayId ] ); \
        lfAlpha = mix( lfAlpha, 1.0, lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] ); \
        lfAlpha *= lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ]; \
        lLayerColourVec4.rgb = RECOLOUR( lLayerColourVec4.rgb, lUniforms.mpCustomPerMesh.gAverageColour##layerId##Vec4.rgb, lUniforms.mpCustomPerMesh.gRecolour##layerId##Vec4.rgb, lUniforms.mpCustomPerMesh.gRecolour##layerId##Vec4.a ); \
        lFragmentColourVec4 = mix( lFragmentColourVec4, lLayerColourVec4, lfAlpha ); \
        lfLayer##layerId##Alpha = lLayerColourVec4.a; \
    }

#else

#define COMBINE_FN( controlId, layerId, arrayId ) \
    float lfLayer##layerId##Alpha = 0.0; \
    float lfLayer##layerId##Mask = 0.0; \
    if ( lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ] > 0.0001 ) \
    { \
        vec4 lLayerColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gSource##layerId##Map ), lTexCoordsVec2, 0.0 ); \
        float lfAlpha = lLayerColourVec4.a; \
        \
        lfAlpha = mix( 1.0, texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gAlpha##layerId##Map ), lTexCoordsVec2, 0.0 ).a, lUniforms.mpCustomPerMesh.gAlphaLayersUsed##controlId##Vec4[ arrayId ] ); \
        lfAlpha = mix( lfAlpha, 1.0, lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] ); \
        lfAlpha *= lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ]; \
        lFragmentColourVec4 = mix( lFragmentColourVec4, lLayerColourVec4, lfAlpha ); \
        lfLayer##layerId##Alpha = lfAlpha; \
        lfLayer##layerId##Mask = lFragmentColourVec4.r; \
    }

#endif

#define COMBINE_MASK_FN( controlId, layerId, arrayId ) \
        if ( lUniforms.mpCustomPerMesh.gLayersUsed##controlId##Vec4[ arrayId ] > 0.0001 ) \
        { \
            lFragmentColourVec4.r = max( lFragmentColourVec4.r, ( 1.0 - lfLayer##layerId##Mask ) * lfLayer##layerId##Alpha ); \
        }

#define COMBINE_ALPHA_FN( controlId, layerId, arrayId ) \
        if ( lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] > 0.0001 ) \
        { \
            lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer##layerId##Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer##controlId##Vec4[ arrayId ] ); \
        }


vec3
Recolour(
    vec3  lOriginalColourVec3,
    vec3  lAverageColourVec3,
    vec3  lRecolourVec3,
    float lfMix )
{
    vec3 lOriginalHSVVec3 = RGBToHSV( lOriginalColourVec3 );
    vec3 lAverageHSVVec3  = RGBToHSV( lAverageColourVec3 );
    vec3 lRecolourHSVVec3 = RGBToHSV( lRecolourVec3 );

    lOriginalHSVVec3.r = fract( (  lOriginalHSVVec3.r - lAverageHSVVec3.r ) + lRecolourHSVVec3.r );
    lOriginalHSVVec3.g = min(      lOriginalHSVVec3.g, lRecolourHSVVec3.g );
    //lOriginalHSVVec3.b = saturate( lOriginalHSVVec3.b + sin( 3.14 * lOriginalHSVVec3.b ) * ( lRecolourHSVVec3.b - lAverageHSVVec3.b ) );
    lOriginalHSVVec3.b = pow(10.0, (-10.0 * (lOriginalHSVVec3.b-0.5)*(lOriginalHSVVec3.b-0.5) ) ) * (0.5 * ( lRecolourHSVVec3.b - lAverageHSVVec3.b )) + lOriginalHSVVec3.b;

    lOriginalHSVVec3 = GammaCorrectInput( saturate( HSVToRGB( lOriginalHSVVec3 ) ) );

    lOriginalHSVVec3 = mix( GammaCorrectInput( lOriginalColourVec3 ), lOriginalHSVVec3, lfMix );
     
    return lOriginalHSVVec3;
}

vec3
RecolourToMatchTerrain(
    vec3  lOriginalColourVec3,
    vec3  lAverageColourVec3,
    vec3  lRecolourVec3,
    float lfMix )
{
    vec3 lOriginalHSVVec3 = RGBToHSV( lOriginalColourVec3 );
    vec3 lAverageHSVVec3  = RGBToHSV( lAverageColourVec3 );
    vec3 lRecolourHSVVec3 = RGBToHSV( lRecolourVec3 );

    lOriginalHSVVec3.r = fract( ( lOriginalHSVVec3.r - lAverageHSVVec3.r ) + lRecolourHSVVec3.r );
    lOriginalHSVVec3.g = min( lRecolourHSVVec3.g, lOriginalHSVVec3.g );
    lOriginalHSVVec3.b = saturate( lOriginalHSVVec3.b - lAverageHSVVec3.b + lRecolourHSVVec3.b );
     
    lOriginalHSVVec3 = GammaCorrectInput( saturate( HSVToRGB( lOriginalHSVVec3 ) ) );

    lOriginalHSVVec3 = mix( GammaCorrectInput( lOriginalColourVec3 ), lOriginalHSVVec3, lfMix );

    return lOriginalHSVVec3;
}

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
FRAGMENT_MAIN_COLOUR_SRT
{   
    vec2 lTexCoordsVec2;
    lTexCoordsVec2 = IN( mTexCoordsVec2 );

    // Read layer from the textures.
    // Copy alpha from each layer so we can use it for blending
    // Set the lowest alpha layer to be fully opaque
    // Set the alpha for any layer which is not used to 0
    // Recolour diffuse textures
    // Blend layers together, based on alpha 

    vec4 lFragmentColourVec4 = vec4(1.0, 1.0, 1.0, 0.0);

    COMBINE_FN( 1, 1, 0 );
    COMBINE_FN( 1, 2, 1 );
    COMBINE_FN( 1, 3, 2 );
    COMBINE_FN( 1, 4, 3 );
    COMBINE_FN( 2, 5, 0 );
    COMBINE_FN( 2, 6, 1 );
    COMBINE_FN( 2, 7, 2 );
    COMBINE_FN( 2, 8, 3 );

#if defined ( D_COMBINE_MASKS )

    lFragmentColourVec4.r = 0.0;

    COMBINE_MASK_FN( 1, 1, 0 );
    COMBINE_MASK_FN( 1, 2, 1 );
    COMBINE_MASK_FN( 1, 3, 2 );
    COMBINE_MASK_FN( 1, 4, 3 );
    COMBINE_MASK_FN( 2, 5, 0 );
    COMBINE_MASK_FN( 2, 6, 1 );
    COMBINE_MASK_FN( 2, 7, 2 );
    COMBINE_MASK_FN( 2, 8, 3 );

    // Red is AO - invert it to get proper results
    lFragmentColourVec4.r = 1.0 - lFragmentColourVec4.r;

#else

    // Set final alpha to match based layer
    #ifdef D_HASALPHACHANNEL

        // We can save another two VGPRs by using COMBINE_ALPHA_FN... but would need more back to make this worthwhile.
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer1Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer1Vec4.r );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer2Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer1Vec4.g );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer3Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer1Vec4.b );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer4Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer1Vec4.a );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer5Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer2Vec4.r );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer6Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer2Vec4.g );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer7Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer2Vec4.b );
        lFragmentColourVec4.a = mix( lFragmentColourVec4.a, lfLayer8Alpha, lUniforms.mpCustomPerMesh.gBaseAlphaLayer2Vec4.a );
        
    #endif

#endif

    FRAGMENT_COLOUR = lFragmentColourVec4;
}
