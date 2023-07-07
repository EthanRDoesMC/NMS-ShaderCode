////////////////////////////////////////////////////////////////////////////////
///
///     @file       PostProcessFragment.h
///     @author     User
///     @date       
///
///     @brief      DepthOfFieldFragmentShader
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
#include "Fullscreen/PostCommon.h"
#include "Common/Common.shader.h"
#include "Common/CommonUtils.shader.h"
#include "Common/CommonPostProcess.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/ACES.shader.h"

#if defined ( D_COMPUTE ) && defined ( D_PLATFORM_ORBIS ) && defined ( D_CONVERT_SRGB_P3 ) && defined ( D_POSTPROCESS_COPY )
// Do not use the optimisation flags... they don't make it any better.
#elif defined ( D_PLATFORM_ORBIS )
#pragma argument (O4; fastmath; scheduler=minpressure)
#pragma argument(unrollallloops)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
#pragma optionNV(fastmath on)
#endif


// =================================================================================================

#if defined( D_GUASS_BLUR_RADIUS_4 )
#if defined( D_PLATFORM_PC ) && defined( D_POSTPROCESS_GUASS_SQUARE )
    // Griff - Radius 4 crashes the AMD driver atm, so uses a slightly wider kernel on PC for now
    #define D_GUASS_BLUR_RADIUS 5
#else
    #define D_GUASS_BLUR_RADIUS 4
#endif
#elif defined( D_GUASS_BLUR_RADIUS_3 )
    #define D_GUASS_BLUR_RADIUS 3
#elif defined( D_GUASS_BLUR_RADIUS_2 )
    #define D_GUASS_BLUR_RADIUS 2
#endif


// =================================================================================================
//
// PARTICLE_BLEND
//
// =================================================================================================

#ifdef D_POSTPROCESS_PARTICLE_BLEND

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    //vec4 lFragmentTexVec4;
    vec4 lResult = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS.xy, 0.0);

    //FRAGMENT_COLOUR = vec4( (lFragmentTexVec4.rgb * lParticleTexVec4.a) + lParticleTexVec4.rgb, lFragmentTexVec4.a ); 
    
    lResult = vec4( lResult.rgb, 1.0 - lResult.a );

    #ifdef D_BLEND_POST_TAA
    vec4 lPostTaaTexVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS.xy, 0.0);
    lResult.rgb = lPostTaaTexVec4.rgb + lResult.rgb;
    #endif

    #ifdef D_COMPUTE
    // manual blending in compute-land
    WRITE_FRAGMENT_COLOUR( lResult + FRAGMENT_COLOUR * ( 1.0 - lResult.a ) );
    #else
    FRAGMENT_COLOUR = lResult;
    #endif
}

#endif

// =================================================================================================
//
// PARTICLE CLEAR
//
// =================================================================================================

#ifdef D_POSTPROCESS_PARTICLE_CLEAR

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    FRAGMENT_COLOUR = vec4(0.0, 0.0, 0.0, 1.0);
}

#endif

// =================================================================================================
//
// PARTICLE CLEAR
//
// =================================================================================================

#ifdef D_POSTPROCESS_CLEAR

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 


FRAGMENT_MAIN_COLOUR_SRT
{
    #if     defined( D_POSTPROCESS_CLEAR_ZERO )
    FRAGMENT_COLOUR = vec4( 0.0, 0.0, 0.0, 0.0 );
    #elif   defined( D_POSTPROCESS_CLEAR_ONE )
    FRAGMENT_COLOUR = vec4( 1.0, 1.0, 1.0, 1.0 );
    #else
    FRAGMENT_COLOUR = lUniforms.mpCustomPerMesh.gBlurParamsVec4;
    #endif
}

#endif


// =================================================================================================
//
// COPY
//
// =================================================================================================

#ifdef D_POSTPROCESS_COPY

#ifdef D_POSTPROCESS_COPY_RED32

#ifdef D_PLATFORM_ORBIS
#pragma PSSL_target_output_format(target 0 FMT_32_AR)
#endif

#endif



//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{			
    //FRAGMENT_COLOUR = vec4( texture2D( lUniforms.mpCustomPerMesh.gBufferMap, IN(mTexCoordsVec2) ).xyz, 1.0 );	
#ifdef D_POSTPROCESS_COPYRGBONLY
    WRITE_FRAGMENT_COLOUR( vec4( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0).rgb,1.0) );
#elif defined( D_POSTPROCESS_COPY_MIP )
    WRITE_FRAGMENT_COLOUR( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w ) );
#elif defined( D_POSTPROCESS_COPY_FLIP )
    WRITE_FRAGMENT_COLOUR( 1.0 - texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0 ) );
#else
    #ifdef D_POSTPROCESS_COPY_ADD
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR + texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0) );
    #elif defined( D_POSTPROCESS_COPY_MULADD )
    vec4 lReadColour = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0);
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR * lReadColour.a + lReadColour );
    #else
    vec4 lColor = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0 );
    #ifdef D_CONVERT_SRGB_P3
    lColor.rgb = MUL( lColor.rgb, sRGB_TO_P3D65 );
    #endif
    WRITE_FRAGMENT_COLOUR( lColor );
    #endif
#endif

}

#endif


// =================================================================================================
//
// Gen Reactivity Map
//
// =================================================================================================

#ifdef D_POSTPROCESS_FSR2_GENREACT

//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float kfThreshold      = 0.15;
STATIC_CONST float kfAlphaThreshold = 0.88;
STATIC_CONST float kfClip           = 0.85;

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    int     liMaterialID        = int( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), TEX_COORDS ).r * 255.0 );
    bool    lWantsColorClipAA   = ( liMaterialID & D_CLAMP_AA ) != 0;
    float   lfReact;

    if ( lWantsColorClipAA )
    {
        lfReact = kfClip;
    }
    else
    {
        float   lfPrtAlpha;

        lfPrtAlpha      = texture2DLod( SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0 ).a;
        lfPrtAlpha      = saturate( kfAlphaThreshold - lfPrtAlpha );
        lfReact         = lfPrtAlpha;

        #if 1 //#ifndef D_PLATFORM_PROSPERO
        if ( lUniforms.mpPerFrame.gFoVValuesVec4.z != 2.0 )
        #endif
        {
            float lfLuminanceOP = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS, 0.0).r;
            float lfLuminanceTR = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), TEX_COORDS, 0.0).r;
            lfReact             = abs( lfLuminanceTR - lfLuminanceOP ) + lfPrtAlpha;
        }
        if ( lfReact > kfThreshold )
        {
            lfReact = sqrt( lfReact );
            lfReact = min(  lfReact, kfClip );
        }
        else
        {
            lfReact = 0.0;
        }
    }

    WRITE_FRAGMENT_COLOUR( vec4( lfReact, 0.0, 0.0, 1.0 ) );
}

#endif

// =================================================================================================
//
// POST BLENDED ABOVE REACTIVITY
//
// =================================================================================================

#ifdef D_POSTPROCESS_REACT_POST_BLEND

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_T1_SRT( float )
{
    int     liMaterialID        = int( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), TEX_COORDS ).r * 255.0 );
    bool    lbWantsColourClipAA = ( liMaterialID & D_CLAMP_AA ) != 0;
    float   lfReact             = 1.0;

    if ( !lbWantsColourClipAA )
    {
        vec3    lvColour        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), TEX_COORDS, 0.0 ).rgb;
        float   lfLuminanceOP   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS, 0.0 ).r;
        float   lfLuminanceTR   = saturate( dot( vec3( 0.2126, 0.7152, 0.0722 ), lvColour ) );
        lfReact                 = sqrt_fast_0( abs( lfLuminanceTR - lfLuminanceOP ) ) * 0.6123724357;
        lfReact                 = min( lfReact, 0.375 );
    }

    FRAGMENT_OUTPUT_T0 = lfReact;
}

#endif

// =================================================================================================
//
// POST PARTICLES REACTIVITY
//
// =================================================================================================

#ifdef D_POSTPROCESS_REACT_POST_PARTICLES

//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float kfThreshold      = 0.10;
STATIC_CONST float kfAlphaThreshold = 0.65;
STATIC_CONST float kfClip           = 0.60;

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_T1_SRT( float )
{
    float lfReact = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0 ).r;

    if ( lfReact == 1.0 )
    {
        FRAGMENT_OUTPUT_T0 = 0.85;
        return;
    }

    if ( lfReact < kfClip )
    {
        float   lfPrtAlpha;

        lfPrtAlpha  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS, 0.0 ).a;
        lfReact    += saturate( kfAlphaThreshold - lfPrtAlpha );
    }

    if( lfReact > kfThreshold )
    {
        lfReact = sqrt_fast_0( lfReact );
        lfReact = min( lfReact, kfClip );
    }
    else
    {
        lfReact = 0.0;
    }

    FRAGMENT_OUTPUT_T0 = lfReact;
}

#endif


// =================================================================================================
//
// D_POSTPROCESS_REPROJECT
//
// =================================================================================================

#ifdef D_POSTPROCESS_REPROJECT

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lUVsVec2       = TEX_COORDS;
    vec3  lEcdMotionVec3 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2, 0.0 ).xyz;
    vec2  lDcdMotionVec2 = DecodeMotion( lEcdMotionVec3.xy );
    //float lfDisoccl      = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsVec2, 0.0 ).x;
    float lfClip         = lEcdMotionVec3.z;
    vec2  lRpjUVsVec2    = GetDejitteredTexCoord( lUVsVec2 + lDcdMotionVec2, lUniforms.mpPerFrame.gDeJitterVec3 );
    vec3  lColourVec3    = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lRpjUVsVec2 , 0.0 ).rgb;

    bool  lbClip         = /*lfDisoccl > 0.0 || */ lfClip > 0.0;
    lbClip               = lbClip || ( lRpjUVsVec2.x <= 0.0 || lRpjUVsVec2.x >= 1.0 );
    lbClip               = lbClip || ( lRpjUVsVec2.y <= 0.0 || lRpjUVsVec2.y >= 1.0 );

    if ( lbClip )
    {
        //lColourVec3     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map  ), lUVsVec2, 0.0 ).rgb;
        lColourVec3     = float2vec3( 0.0 );
    }

    WRITE_FRAGMENT_COLOUR( vec4( lColourVec3, 1.0 ) );
}

#endif

// =================================================================================================
//
// D_POSTPROCESS_TEMPORAL_FILTER
//
// =================================================================================================

#ifdef D_POSTPROCESS_TEMPORAL_FILTER

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    vec3 lColourFinalVec3;
    vec3 laColoursVec3[ 6 ];
    
    vec2 lUVsVec2 = TEX_COORDS;

    laColoursVec3[ 0 ]  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2, 0.0 ).rgb;
    laColoursVec3[ 1 ]  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsVec2, 0.0 ).rgb;
    laColoursVec3[ 2 ]  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2, 0.0 ).rgb;
    laColoursVec3[ 3 ]  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsVec2, 0.0 ).rgb;
    laColoursVec3[ 4 ]  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lUVsVec2, 0.0 ).rgb;
    laColoursVec3[ 5 ]  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), lUVsVec2, 0.0 ).rgb;

    //lColourFinalVec3    = laColoursVec3[ 0 ] * 0.3 + laColoursVec3[ 1 ] * 0.25 + laColoursVec3[ 2 ] * 0.25 + laColoursVec3[ 3 ] * 0.2;
    lColourFinalVec3    = laColoursVec3[ 0 ] + laColoursVec3[ 1 ] + laColoursVec3[ 2 ] + laColoursVec3[ 3 ] + laColoursVec3[ 4 ] + laColoursVec3[ 5 ];
    lColourFinalVec3   /= 6.0;
    //lColourFinalVec3    = laColoursVec3[ 0 ];

    WRITE_FRAGMENT_COLOUR( vec4( lColourFinalVec3, 1.0 ) );
}

#endif

// =================================================================================================
//
// COPY
//
// =================================================================================================

#ifdef D_POSTPROCESS_STAA_COPY

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{           
    //FRAGMENT_COLOUR = vec4( texture2D( lUniforms.mpCustomPerMesh.gBufferMap, IN(mTexCoordsVec2) ).xyz, 1.0 );    
    FRAGMENT_COLOUR0 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBufferMap ),  IN(mTexCoordsVec2) );   
    FRAGMENT_COLOUR1 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer1Map ), IN(mTexCoordsVec2) );   
}

#endif

// =================================================================================================
//
// COPY
//
// =================================================================================================

#ifdef D_POSTPROCESS_COPY_YCGCO_TO_RGB

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{   
    vec2 tc = SCREENSPACE_AS_RENDERTARGET_UVS(IN(mTexCoordsVec2));

    vec3 colIn = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), tc ).rgb;  

    vec4 colOut = vec4(1.0);

    colOut.rgb = YCgCoToRGB( colIn );
    colOut.rgb = saturate( TonemapKodak( colOut.rgb ) / TonemapKodak( float2vec3(1.0) ) );
    colOut.rgb = GammaCorrectOutput( colOut.rgb  );
    FRAGMENT_COLOUR = colOut;
}

#endif

// =================================================================================================
//
// DEJITTER
//
// =================================================================================================

#ifdef D_POSTPROCESS_DEJITTER

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{           
    //FRAGMENT_COLOUR = vec4( texture2D( lUniforms.mpCustomPerMesh.gBufferMap, IN(mTexCoordsVec2) ).xyz, 1.0 );    
    vec2 lDeJitterVec = DSCREENSPACE_AS_RENDERTARGET_UVS(lUniforms.mpPerFrame.gDeJitterVec3.xy);

    vec2 ldejit = IN(mTexCoordsVec2) + lDeJitterVec;

    FRAGMENT_COLOUR = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), ldejit.xy );   
}

#endif


// =================================================================================================
//
// CONVERTHDR
//
// =================================================================================================

#ifdef D_POSTPROCESS_CONVERTHDR

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 



vec3 HdrSimColor( vec3 rgb )
{
    float lum = MUL( rgb, sRGB_TO_XYZ ).y * 80.0;
    float logLum = log10(lum);

    vec4 colorPoints[12] = {
        vec4( 0, 0, 0, log10(1.0) ),
        vec4( 0, 0, 1, log10(2.0) ),
        vec4( 0, 0, 1, log10(50.0) ),
        vec4( 0, 0, 1, log10(120.0) ),
        vec4( 0, 1, 0, log10(125.0) ),
        vec4( 0, 1, 0, log10(200.0) ),
        vec4( 1, 1, 0, log10(210.0) ),
        vec4( 1, 1, 0, log10(320.0) ),
        vec4( 1, 0, 0, log10(400.0) ),
        vec4( 1, 0, 0, log10(1000.0) ),
        vec4( 1, 0, 1, log10(4000.0) ),
        vec4( 1, 1, 1, log10(10000.0) )
    };

    vec3 outCol = vec3(0,0,0);

    for( int ii = 1; ii<12; ++ii )
    {
        if( logLum < colorPoints[ii].w )
        {
            float lerpVal = saturate( (logLum - colorPoints[ii-1].w) / (colorPoints[ii].w - colorPoints[ii-1].w) );
            outCol = mix(colorPoints[ii-1].xyz, colorPoints[ii].xyz, lerpVal);
            break;
        }
    }

    return outCol;
}

//TF_BEGIN
//only used under VULKAN
#ifdef D_PLATFORM_VULKAN
SAMPLER3DREG( gHdrLut, 1 );
#endif
//TF_END

#if defined( D_PLATFORM_PROSPERO )


float
PreciseGammaCorrectInputF(
    in float lfValue)
{
    float lfSign = (lfValue < 0.0) ? -1.0 : 1.0;
    float lfCorrectValue = lfValue * lfSign;

    if (lfCorrectValue > 1.0)
    {
        lfCorrectValue = pow(lfCorrectValue, 2.5);
    }
    else if (lfCorrectValue > 0.0)
    {
        lfCorrectValue = pow(lfCorrectValue, 2.3); // lfCorrectValue * ( lfCorrectValue * ( lfCorrectValue * ( 0.305306011 ) + ( 0.682171111 ) ) + ( 0.012522878 ) );
    }

    lfCorrectValue *= lfSign;

    return lfCorrectValue;
}


vec3
PreciseGammaCorrectInput(
    in vec3 lColourVec3)
{
    vec3 lCorrectColourVec3;

    lCorrectColourVec3.x = PreciseGammaCorrectInputF(lColourVec3.x);
    lCorrectColourVec3.y = PreciseGammaCorrectInputF(lColourVec3.y);
    lCorrectColourVec3.z = PreciseGammaCorrectInputF(lColourVec3.z);

    return lCorrectColourVec3;
}

#endif

FRAGMENT_MAIN_COLOUR_SRT
{      
    vec4 lFragCol = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy, 0.0);
    lFragCol.w = 1.0;
    lFragCol.xyz = GammaCorrectInput(lFragCol.xyz);
    lFragCol.xyz *= 3.0;
    lFragCol.xyz = max(float2vec3(0.0), MUL(lFragCol.xyz, BT709_TO_BT2020));
    lFragCol.xyz = saturate( PQ_OETF_Fast(lFragCol.xyz) );

    #ifdef D_PLATFORM_VULKAN
    lFragCol.xyz *= 64.0 / 63.0;
    lFragCol.xyz += 1.0 / 128.0;
    lFragCol.xyz = texture3DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gHdrLut), lFragCol.xyz, 0.0).xyz;
    
    lFragCol.xyz = PQ_EOTF(lFragCol, 0, 0).xyz;
    lFragCol.xyz = MUL(lFragCol.xyz, BT2020_TO_BT709);
    lFragCol.xyz /= 0.8;
    #endif

    WRITE_FRAGMENT_COLOUR( vec4(lFragCol.xyz, 1.0) );
}

#endif

// =================================================================================================
//
// DEPTHMASK
//
// =================================================================================================

#ifdef D_POSTPROCESS_DEPTHMASK

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

VOID_MAIN_DEPTH_SRT
{
#ifndef D_PLATFORM_PROSPERO
    vec4 lFragCol = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy, 0.0);
    if (lFragCol.r < 1.0) discard;
#endif
    FRAGMENT_DEPTH = 1.0 - D_DEPTH_CLEARVALUE;
}

#endif


// =================================================================================================
//
// COPY_RGB_TO_LUMINANCE_LINEAR/GAMMA
//
// =================================================================================================

#ifdef D_POSTPROCESS_COPY_RGB_TO_LUMINANCE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lFragCol = texture2DLod( SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy, 0.0 );

    #if defined( D_LINEAR )
    vec3 lvLumCoeff = vec3( 0.2126, 0.7152, 0.0722 );
    #elif defined( D_GAMMA )
    vec3 lvLumCoeff = vec3( 0.299, 0.587, 0.114 );
    #else
    #error Unexpected combination
    #endif

    WRITE_FRAGMENT_COLOUR( float2vec4( dot( lFragCol.rgb, lvLumCoeff ) ) );
}

#endif


// =================================================================================================
//
// COPY_DEPTH
//
// =================================================================================================

#ifdef D_POSTPROCESS_COPY_DEPTH


//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

VOID_MAIN_DEPTH_SRT
{
    WRITE_FRAGMENT_DEPTH(  texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS ).x );
}

#endif

// =================================================================================================
//
// DEPTH_CLEAR
//
// =================================================================================================

#ifdef D_POSTPROCESS_DEPTH_CLEAR


//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

VOID_MAIN_DEPTH_SRT
{
    WRITE_FRAGMENT_DEPTH( 0.0 );
}

#endif

// =================================================================================================
//
// COPY_DEPTH_STENCIL
//
// =================================================================================================

#ifdef D_POSTPROCESS_COPY_DEPTH_STENCIL 

 
//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

VOID_MAIN_DEPTH_STENCIL_SRT
{
    //vec4 lFrag = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS );
    vec4 lFrag = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), vec2( 0.5, 0.5 ) );
    WRITE_FRAGMENT_DEPTH( lFrag.b );
    WRITE_FRAGMENT_STENCIL( lFrag.x ); 
    //WRITE_FRAGMENT_DEPTH(  texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS ).b );
    //WRITE_FRAGMENT_STENCIL(  texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS ).x ); 
    //WRITE_FRAGMENT_DEPTH( 0.5 );
    //WRITE_FRAGMENT_STENCIL(  5 );  
}

#endif

// =================================================================================================
//
// DEPTH_REVERSE_TO_LINEAR
//
// =================================================================================================

#ifdef D_POSTPROCESS_REVERSE_TO_LINEAR

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#ifdef D_PLATFORM_ORBIS
#pragma PSSL_target_output_format(target 0 FMT_32_AR)
#endif

FRAGMENT_MAIN_COLOUR_SRT
{
    float lfReverseDepth = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS ).x;
    float lfLinearDepthNorm = ReverseZToLinearDepthNorm( lUniforms.mpPerFrame.gClipPlanesVec4, lfReverseDepth );
    WRITE_FRAGMENT_COLOUR( EncodeDepthToColour( lfLinearDepthNorm ) );
}

#endif



// =================================================================================================
//
// LINEARNORM_TO_REVERSE
//
// =================================================================================================

#ifdef D_POSTPROCESS_LINEARNORM_TO_REVERSE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#ifdef D_PLATFORM_ORBIS
#pragma PSSL_target_output_format(target 0 FMT_32_AR)
#endif

FRAGMENT_MAIN_COLOUR_SRT
{
    float lfDepth           = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS ) );
    float lfReveraseDepth   = LinearNormToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth );
    WRITE_FRAGMENT_COLOUR( EncodeDepthToColour( lfReveraseDepth ) );
}

#endif



// =================================================================================================
//
// DEPTH_REVERSE_TO_LINEAR_LESS
//
// =================================================================================================

#ifdef D_POSTPROCESS_REVERSE_TO_LINEAR_LESS

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#ifdef D_PLATFORM_ORBIS
#pragma PSSL_target_output_format(target 0 FMT_32_AR)
#endif

#if defined ( D_COMPUTE )
COMPUTE_MAIN_SRT( 8, 8, 1 )
#else
FRAGMENT_MAIN_COLOUR_DEPTH_SRT
#endif
{
#if defined ( D_COMPUTE ) && defined ( D_PLATFORM_ORBIS )

    // Optimisation for SPI Wave generation, increase step size, and reduce horizontal thread count in pipeline.
    // Assumes no side-by-side rendering!  We're getting no benefit from increasing the step size, but keeping the code
    //  here for the future.
    const uint kStepOffset = 1;

    for ( uint i = 0; i < kStepOffset; ++i )
    {
        uvec2 lComputeCoords = uvec2( dispatchThreadID.x * kStepOffset + i, dispatchThreadID.y );
        vec2 lTexCoords = ( vec2( dispatchThreadID.x * kStepOffset + i, dispatchThreadID.y ) + vec2( 0.5, 0.5 ) ) * lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw;

        float lfDepth = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lTexCoords ).x;
        float lfReverseDepth = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords ).x;

        if ( lfReverseDepth < lfDepth )
        {
            continue;
        }

        float lfLinearDepthNorm = ReverseZToLinearDepthNorm( lUniforms.mpPerFrame.gClipPlanesVec4, lfReverseDepth );
        lUniforms.mpCmpOutPerMesh.gOutTexture0[lComputeCoords] = EncodeDepthToColour( lfLinearDepthNorm );
    }

#else
    float lfReverseDepth  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS ).x;
    //float lfReverseDepth2 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ).x;
   // lfReverseDepth = lfReverseDepth2;//max( lfReverseDepth, lfReverseDepth2 );

    float lfLinearDepthNorm = ReverseZToLinearDepthNorm( lUniforms.mpPerFrame.gClipPlanesVec4, lfReverseDepth );
    WRITE_FRAGMENT_COLOUR( EncodeDepthToColour( lfLinearDepthNorm ) );
    WRITE_FRAGMENT_DEPTH( lfReverseDepth );
#endif
}

#endif

// =================================================================================================
//
// DOWNSAMPLE_GBUFFERDEPTH
//
// =================================================================================================

#ifdef D_POSTPROCESS_DOWNSAMPLE_GBUFFERDEPTH

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#ifdef D_PLATFORM_ORBIS
#pragma PSSL_target_output_format(target 0 FMT_32_AR)
#endif


FRAGMENT_MAIN_COLOUR_SRT

{   
    vec2 lFlooredTexCoordsVec2 = TEX_COORDS.xy;
    WRITE_FRAGMENT_COLOUR( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lFlooredTexCoordsVec2, 0.0 ) );
}

#endif // defined ( D_POSTPROCESS_DOWNSAMPLE_GBUFFERDEPTH )


// =================================================================================================
//
// DOWNSAMPLE_DEPTH 
//
// =================================================================================================

#ifdef D_POSTPROCESS_DOWNSAMPLE_DEPTH

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#ifdef D_PLATFORM_ORBIS
#pragma PSSL_target_output_format(target 0 FMT_32_AR)
#endif

#ifdef D_PLATFORM_SWITCH
//precision highp sampler;
//precision highp float;
#endif

#ifdef D_COMPUTE
FRAGMENT_MAIN_COLOUR_SRT
#else
FRAGMENT_MAIN_COLOUR_DEPTH_SRT
#endif
{
    vec2 lTexCoordsVec2 = TEX_COORDS;

    float lfDepth   = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoordsVec2 ).x;

    #ifdef D_COMPUTE
    WRITE_FRAGMENT_COLOUR( vec4( lfDepth, 0.0, 0.0, 0.0) );
    #else

    #if defined( D_POSTPROCESS_DOWNSAMPLE_DEPTH_NORM )
    FRAGMENT_COLOUR = vec4( ReverseZToLinearDepthNorm( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth ), 0.0, 0.0, 0.0 );
    #else
    FRAGMENT_COLOUR = vec4( ReverseZToLinearDepth    ( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth ), 0.0, 0.0, 0.0 );
    #endif

    FRAGMENT_DEPTH = lfDepth;
    #endif
}


#endif

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_DEPTH_REPRJ_FRWD
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_DEPTH_REPRJ_FRWD

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

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

#if defined( D_PLATFORM_PC )
FRAGMENT_MAIN_COLOUR_SRT
#else
VOID_MAIN_SRT
#endif
{
    vec2    lUVsVec2    = TEX_COORDS;
    float   lfDepth     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lUVsVec2, 0.0 ).x;

    vec3    lWorldPos   = RecreatePositionFromRevZDepthSC(
                            lfDepth, lUVsVec2,
                            lUniforms.mpPerFrame.gPrevViewPositionVec3,
                            lUniforms.mpPerFrame.gPrevInverseViewProjectionSCMat4 );

    vec3    lUVPos      = GetUVCoordsFromWorld( DEREF_PTR( lUniforms.mpPerFrame ), lWorldPos ).xyz;

    if ( ( lUVPos.x >= 0 && lUVPos.x < 1.0 ) &&
         ( lUVPos.y >= 0 && lUVPos.y < 1.0 ) )
    {
        uint  luDepth = asuint( lUVPos.z );

        #if defined ( D_PLATFORM_METAL )
        ivec2 lUViPos = ivec2( lUVPos.xy * GetImgResolution( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjFrwd ) ) );
        imageAtomicMin( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjFrwd ), lUViPos, luDepth );
        #else
        ivec2 lUViPos = ivec2( lUVPos.xy * GetImgResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjFrwd ) ) );
        imageAtomicMin( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjFrwd ), lUViPos, luDepth );			
        #endif
    }

    #ifdef D_PLATFORM_PC
    WRITE_FRAGMENT_COLOUR( vec4( 1.0 ) );
    #endif
}
#endif

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_DEPTH_U_DOWN
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_DEPTH_U_DOWN

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_UICOLOUR_SRT
{
    uint    luDepth;
    uvec4   laDepths;
    vec2    lUVsVec2 = TEX_COORDS;

    luDepth  = uint( -1 );
    laDepths = textureUGatherRed( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0uMap ), lUVsVec2 );
    for ( int kk = 0; kk < 4; ++kk )
    {
        luDepth = min( luDepth, laDepths[ kk ] );
    }

    FRAGMENT_COLOUR_UVEC4 = uvec4( luDepth, 0, 0, 0 );
}
#endif

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_DEPTH_I2F
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_DEPTH_I2F

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

VOID_MAIN_DEPTH_SRT
{
    vec2    lUVs_fVec2      = TEX_COORDS;
    vec2    lHalfTxlVec2    = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * 0.5;
    
    uint    luDepth;
    float   lfDepth;
    uvec4   laDepths;
    vec2    lCurrUVs;

    luDepth  = uint( -1 );

    for ( int ii = 0; ii <= 1; ++ii )
    {
        for ( int jj = 0; jj <= 1; ++jj )
        {
            lCurrUVs = lUVs_fVec2 + lHalfTxlVec2 * ( vec2( ii, jj ) * 2.0 - 1.0 );
            laDepths = textureUGatherRed( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0uMap ), lCurrUVs );
            for ( int kk = 0; kk < 4; ++kk )
            {
                luDepth = min( luDepth, laDepths[ kk ] );
            }
        }
    }

    lfDepth = luDepth == uint( -1 ) ? 0.0f : asfloatu( luDepth );

    FRAGMENT_DEPTH = lfDepth;
}
#endif




// =================================================================================================
//
// BRIGHTPASS
//
// =================================================================================================

#ifdef D_POSTPROCESS_BRIGHTPASS

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

vec3 
Threshold(
    in vec3 lColour,
    in float lfThreshold,      
    in float lfGain )
{
    vec3 lumcoeff = vec3(0.299,0.587,0.114);
    //vec3 lumcoeff = normalize(vec3(1.0,1.0,1.0));

    float lum = dot(lColour.rgb, lumcoeff);

    float thresh = max((lum-lfThreshold)*lfGain, 0.0);
    return mix( vec3(0.0, 0.0, 0.0), lColour, thresh );
    //return vec3( lum );
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec3 lBrightColourVec3;
    lBrightColourVec3 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBufferMap ), TEX_COORDS, 0.0).xyz;

    #ifdef D_COMPUTE
    // for async, the glow is stored already in the bloom buffer
    float lfGlowAlpha = FRAGMENT_COLOUR.r;
    #else
     float lfGlowAlpha = 1.0 - texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer1Map ), TEX_COORDS, 0.0).a;
    #endif

    lBrightColourVec3.xyz = TonemapKodak(lBrightColourVec3.xyz) / TonemapKodak( vec3(1.0,1.0,1.0) );

    lBrightColourVec3 = GammaCorrectOutput( lBrightColourVec3 );

    lBrightColourVec3 = Threshold(  lBrightColourVec3, 
                                    min( lUniforms.mpCustomPerMesh.gHDRParamsVec4.y, lfGlowAlpha),  // Threshold
                                    lUniforms.mpCustomPerMesh.gHDRParamsVec4.z );// Offset

    lBrightColourVec3 = clamp( lBrightColourVec3, float2vec3(0.0), float2vec3(1.0) );
    WRITE_FRAGMENT_COLOUR( vec4( lBrightColourVec3, 1.0 ) );
    //FRAGMENT_COLOUR = vec4( 0.0, 0.0, 1.0, 1.0 );
}


#endif

// =================================================================================================
//
// BRIGHTPASS_COPY
//
// =================================================================================================

#ifdef D_POSTPROCESS_BRIGHTPASS_COPY

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    float lfGlowAlpha = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBufferMap ), TEX_COORDS ).a;

    FRAGMENT_COLOUR = vec4( vec3( lfGlowAlpha,lfGlowAlpha,lfGlowAlpha ), 1.0 );
}


#endif

// =================================================================================================
//
// BRIGHTPASS
//
// =================================================================================================

#ifdef D_POSTPROCESS_DOWNSAMPLE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR01_SRT
{
    vec2 coord2 = TEX_COORDS; // +vec2(2.0, 2.0) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;

    vec3 lTextureColourVec3;
    lTextureColourVec3.rgb = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), coord2, 0.0).rgb;

    float lfPower = GetDofPower( coord2,
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer1Map),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBlurMask),
        lUniforms.mpCustomPerMesh.gDoFParamsVec4,
        lUniforms.mpPerFrame.gClipPlanesVec4);

    WRITE_FRAGMENT_COLOUR0( vec4( lTextureColourVec3, 1.0 ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lfPower, 0.0, 0.0,  1.0 ) );
}


#endif


// =================================================================================================
//
// D_POSTPROCESS_PREPARE_UPSAMPLE_4TAP_CATMULL_ROM
//
// =================================================================================================

#ifdef D_POSTPROCESS_PREPARE_UPSAMPLE_4TAP_CATMULL_ROM

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    ivec2  lPixVec2     = ivec2( TEX_COORDS * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );
    float  lfSign       = ( lPixVec2.x & 1 ) == ( lPixVec2.y & 1 ) ? 1.0 : -1.0;
    vec3   lvColourIn   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS, 0.0 ).rgb;
    vec4   lvColourOut;

    lvColourOut         = vec4( lvColourIn, 1.0 );
    lvColourOut         = lfSign * lvColourOut + 1.0;
    lvColourOut        *= 511.0 / 1023.0;

    WRITE_FRAGMENT_COLOUR( lvColourOut );
}

#endif

// =================================================================================================
//
// D_POSTPROCESS_UPSAMPLE_4TAP_BICUBIC
//
// =================================================================================================

#ifdef D_POSTPROCESS_UPSAMPLE_4TAP_BICUBIC

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


vec3 BicubicTextureSample4(SAMPLER2DARG(lTextureMap), vec2 lTexCoord)
{
    vec2 lTexSize = GetTexResolution(lTextureMap);
    vec2 lTexelSize = 1.0 / lTexSize;
    float x = (lTexCoord.x * lTexSize.x) + 0.5f;
    float y = (lTexCoord.y * lTexSize.y) + 0.5f;
    // transform the coordinate from [0,extent] to [-0.5, extent-0.5]
    vec2 coord_grid = vec2(x - 0.5, y - 0.5);
    //float2 coord_grid = float2(x, y);
    vec2 index = floor(coord_grid);
    vec2 fraction = coord_grid - index;
    vec2 one_frac = 1.0 - fraction;
    vec2 one_frac2 = one_frac * one_frac;
    vec2 fraction2 = fraction * fraction;
    vec2 w0 = 1.0 / 6.0 * one_frac2 * one_frac;
    vec2 w1 = 2.0 / 3.0 - 0.5 * fraction2 * (2.0 - fraction);
    vec2 w2 = 2.0 / 3.0 - 0.5 * one_frac2 * (2.0 - one_frac);
    vec2 w3 = 1.0 / 6.0 * fraction2 * fraction;
    vec2 g0 = w0 + w1;
    vec2 g1 = w2 + w3;
    // h0 = w1/g0 - 1, move from [-0.5, extent-0.5] to [0, extent]
    vec2 h0 = (w1 / g0) - 0.5 + index;
    vec2 h1 = (w3 / g1) + 1.5 + index;
    // fetch the four linear interpolations
    vec3 tex00 = texture2DLod(lTextureMap, vec2(h0.x, h0.y) * lTexelSize.xy, 0).rgb;
    vec3 tex10 = texture2DLod(lTextureMap, vec2(h1.x, h0.y) * lTexelSize.xy, 0).rgb;
    vec3 tex01 = texture2DLod(lTextureMap, vec2(h0.x, h1.y) * lTexelSize.xy, 0).rgb;
    vec3 tex11 = texture2DLod(lTextureMap, vec2(h1.x, h1.y) * lTexelSize.xy, 0).rgb;
    tex00 = lerp(tex01, tex00, g0.y);
    tex10 = lerp(tex11, tex10, g0.y);
    return lerp(tex10, tex00, g0.x);
}


FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lFragCol;
    lFragCol.rgb = BicubicTextureSample4(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy);
    lFragCol.w = 1.0;
    WRITE_FRAGMENT_COLOUR(lFragCol);
}

#endif

// =================================================================================================
//
// D_POSTPROCESS_UPSAMPLE_5TAP_BICUBIC
//
// =================================================================================================

#ifdef D_POSTPROCESS_UPSAMPLE_5TAP_BICUBIC

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


#ifdef D_PLATFORM_SWITCH

precision mediump sampler;

half3
BicubicTextureSample5Half(
    SAMPLER2DARG(lTextureMap),
    vec2            lTexCoord)
{
    half3 result;

    vec2 lTexSize = GetTexResolution(lTextureMap);
    vec2 lTexelSize = 1.0 / lTexSize;

    vec2 pixel = lTexCoord * lTexSize + 0.5;

    vec2 c_onePixel = lTexelSize;
    vec2 c_twoPixels = 2.0 * lTexelSize;

    vec2 pixel_frac = fract(pixel);

    vec2 P = floor(pixel) - vec2(0.5, 0.5);
    P = P * lTexelSize;

    // 5-tap bicubic sampling (for Hermite/Carmull-Rom filter) -- (approximate from original 9-tap bilinear fetching) 
    // 4-tap is possible but only for B-spline filter, whose quality is too low for TAA history resampling (more blur than single bilinear) 
    half   s = 0.5;	// s is potentially adjustable
    half2    t = pixel_frac;
    half2    t2 = t * t;
    half2    t3 = t2 * t;
    half2    w0 = -s * t3 + 2.0 * s*t2 - s * t;
    half2    w1 = (2.0 - s)*t3 + (s - 3.0)*t2 + 1.0;
    half2    w2 = (s - 2.0)*t3 + (3.0 - 2.0 * s)*t2 + s * t;
    half2    w3 = s * t3 - s * t2;

    half2    s0 = w1 + w2;
    vec2    f0 = w2 / (w1 + w2);
    vec2    m0 = P + (f0) * lTexelSize;

    vec2    tc0 = P + vec2(-c_onePixel.x, -c_onePixel.y);
    vec2    tc3 = P + vec2(c_twoPixels.x, c_twoPixels.y);

    half3    A = texture2DLod(lTextureMap, vec2(m0.x, tc0.y), 0).rgb;
    half3    B = texture2DLod(lTextureMap, vec2(tc0.x, m0.y), 0).rgb;
    half3    C = texture2DLod(lTextureMap, vec2(m0.x, m0.y), 0).rgb;
    half3    D = texture2DLod(lTextureMap, vec2(tc3.x, m0.y), 0).rgb;
    half3    E = texture2DLod(lTextureMap, vec2(m0.x, tc3.y), 0).rgb;

    result = (0.5 * (A + B) * w0.x + A * s0.x + 0.5 * (A + B) * w3.x) * w0.y +
        (B * w0.x + C * s0.x + D * w3.x) * s0.y +
        (0.5 * (B + E) * w0.x + E * s0.x + 0.5 * (D + E) * w3.x) * w3.y;

    return result;
}


#if defined ( D_COMPUTE )
COMPUTE_MAIN_SRT(8, 8, 1)
{
    half4 lFragCol0;
    half4 lFragCol1;
    half4 lFragCol2;
    half4 lFragCol3;

    uvec2 liDispatchThreadID = dispatchThreadID.xy * 2;
    lFragCol0.rgb = BicubicTextureSample5Half(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), ((vec2(liDispatchThreadID.xy              ) + vec2(0.5, 0.5)) * lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw));
    lFragCol1.rgb = BicubicTextureSample5Half(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), ((vec2(liDispatchThreadID.xy + ivec2(1, 0)) + vec2(0.5, 0.5)) * lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw));
    lFragCol2.rgb = BicubicTextureSample5Half(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), ((vec2(liDispatchThreadID.xy + ivec2(0, 1)) + vec2(0.5, 0.5)) * lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw));
    lFragCol3.rgb = BicubicTextureSample5Half(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), ((vec2(liDispatchThreadID.xy + ivec2(1, 1)) + vec2(0.5, 0.5)) * lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw));

    lFragCol0.w = 1.0;
    lFragCol1.w = 1.0;
    lFragCol2.w = 1.0;
    lFragCol3.w = 1.0;

    imageStore(lUniforms.mpCmpOutPerMesh.gOutTexture0, ivec2(liDispatchThreadID).xy, lFragCol0);
    imageStore(lUniforms.mpCmpOutPerMesh.gOutTexture0, ivec2(liDispatchThreadID).xy + ivec2(1, 0), lFragCol1);
    imageStore(lUniforms.mpCmpOutPerMesh.gOutTexture0, ivec2(liDispatchThreadID).xy + ivec2(0, 1), lFragCol2);
    imageStore(lUniforms.mpCmpOutPerMesh.gOutTexture0, ivec2(liDispatchThreadID).xy + ivec2(1, 1), lFragCol3);

}

#else

FRAGMENT_MAIN_COLOUR_SRT
{
    half4 lFragCol;
    lFragCol.rgb = BicubicTextureSample5Half(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy);
    lFragCol.w = 1.0;
    WRITE_FRAGMENT_COLOUR(lFragCol);
}
#endif

#else

vec3
BicubicTextureSample5(
    SAMPLER2DARG(lTextureMap),
    vec2            lTexCoord)
{
    vec3 result;

    vec2 lTexSize = GetTexResolution(lTextureMap);
    vec2 lTexelSize = 1.0 / lTexSize;
    vec2 pixel = lTexCoord * lTexSize + 0.5;

    vec2 c_onePixel = lTexelSize;
    vec2 c_twoPixels = 2.0 * lTexelSize;

    vec2 pixel_frac = fract(pixel);

    vec2 P = floor(pixel) - vec2(0.5, 0.5);
    P = P * lTexelSize;

    // 5-tap bicubic sampling (for Hermite/Carmull-Rom filter) -- (approximate from original 9-tap bilinear fetching) 
    // 4-tap is possible but only for B-spline filter, whose quality is too low for TAA history resampling (more blur than single bilinear) 
    float   s = 0.5;	// s is potentially adjustable
    vec2    t = pixel_frac;
    vec2    t2 = t * t;
    vec2    t3 = t2 * t;
    vec2    w0 = -s * t3 + 2 * s*t2 - s * t;
    vec2    w1 = (2 - s)*t3 + (s - 3)*t2 + 1;
    vec2    w2 = (s - 2)*t3 + (3 - 2 * s)*t2 + s * t;
    vec2    w3 = s * t3 - s * t2;

    vec2    s0 = w1 + w2;
    vec2    f0 = w2 / (w1 + w2);
    vec2    m0 = P + (f0) / lTexSize;

    vec2    tc0 = P + vec2(-c_onePixel.x, -c_onePixel.y);
    vec2    tc3 = P + vec2(c_twoPixels.x, c_twoPixels.y);

    vec3    A = texture2DLod(lTextureMap, vec2(m0.x, tc0.y), 0).rgb;
    vec3    B = texture2DLod(lTextureMap, vec2(tc0.x, m0.y), 0).rgb;
    vec3    C = texture2DLod(lTextureMap, vec2(m0.x, m0.y), 0).rgb;
    vec3    D = texture2DLod(lTextureMap, vec2(tc3.x, m0.y), 0).rgb;
    vec3    E = texture2DLod(lTextureMap, vec2(m0.x, tc3.y), 0).rgb;

    result = (0.5 * (A + B) * w0.x + A * s0.x + 0.5 * (A + B) * w3.x) * w0.y +
        (B * w0.x + C * s0.x + D * w3.x) * s0.y +
        (0.5 * (B + E) * w0.x + E * s0.x + 0.5 * (D + E) * w3.x) * w3.y;

    return result;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lFragCol;
    lFragCol.rgb = BicubicTextureSample5(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy);
    lFragCol.w = 1.0;
    WRITE_FRAGMENT_COLOUR(lFragCol);
}

#endif

#endif

// =================================================================================================
//
// D_POSTPROCESS_UPSAMPLE_4TAP_CATMULL_ROM
//
// =================================================================================================

#ifdef D_POSTPROCESS_UPSAMPLE_4TAP_CATMULL_ROM

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

#if defined( D_PLATFORM_SWITCH )
precision mediump sampler;
#define   type    half
#define   type2   half2
#define   type3   half3
#define   type4   half4
#else
#define   type    float
#define   type2   vec2
#define   type3   vec3
#define   type4   vec4
#endif

type4
SamplingWeights(
    vec2  pixelIdx,
    type2 s01,
    type2 s23,
    type2 f )
{
    type  sf  = ( int( pixelIdx.x + pixelIdx.y ) & 1 ) == 0 ? 1.0 : -1.0;

    s01.y    *= sf;
    s23.y    *= sf;

    #if !defined( D_POSTPROCESS_UPSAMPLE_4TAP_CATMULL_ROM_SHARPER )
    s01      *= 1.0 - f;
    s23      *= f;
    #endif

    type4 s;
    s.x    =  s01.x * s01.y;
    s.y    = -s23.x * s01.y;
    s.z    = -s01.x * s23.y;
    s.w    =  s23.x * s23.y;

    return s;
}

type4
SampleTextureBilinearly(
    SAMPLER2DARG( lTextureMap ),
    vec2 uv,
    type s )
{
    type4  sample_color = texture2DLod( lTextureMap, uv, 0.0 );
    return sample_color * s;
}

type3
DecodeSample(
    type4 smp,
    type4 s )
{
    type ss;

    ss        = s.x + s.y + s.z + s.w;
    smp      *= 1023.0 / 511.0;
    smp      -= ss;
    smp.rgb  /= smp.a;

    return smp.rgb;
}

// based on https://www.decarpentier.nl/2d-catmull-rom-in-4-samples
type3
CatmullRomTextureSample4(
    SAMPLER2DARG( lTextureMap ),
    vec2          lTexCoord )
{
    vec2 texSize   = vec2( GetTexResolution( lTextureMap ) );

    vec2 samplePos = lTexCoord * texSize;
    vec2 pixelIdx  = floor( samplePos - 0.5 );

    // Compute the fractional offset from our starting texel to our original sample location, which we'll
    // feed into the Catmull-Rom spline function to get our filter weights.
    type2 f   = samplePos - pixelIdx - 0.5;

    #if defined( D_POSTPROCESS_UPSAMPLE_4TAP_CATMULL_ROM_SHARPER )
    type2 s01 =  2.0 * f * f * (f - 2.0) + f + 1.0;
    type2 s23 = -2.0 * f * f * (f - 1.0) + f;

    vec2  f1  = -0.7 * f * (f - 1.0);
    vec2  f0  = -f1 + 1.0;
    #else
    type2 s1  =  (-1.5 * f + 1.0) * f + 1.0;   // = w1 / (1 - f)
    type2 s3  = -( 0.5 * f - 0.5) * f;         // =-w3 / (f)
    type2 s01 =  (-2.0 * f + 1.5) * f + 1.0;   // s0 + s1
    type2 s23 =  (-2.0 * f + 2.5) * f + 0.5;   // s2 + s3

    vec2  f0  = s1 / s01;
    vec2  f1  = s3 / s23;
    #endif

    vec2  t0  = ( pixelIdx - 0.5 + f0 ) / texSize;
    vec2  t1  = ( pixelIdx + 1.5 + f1 ) / texSize;

    type4 s;
    type4 smp;

    s         = SamplingWeights( pixelIdx, s01, s23, f );
    smp       = SampleTextureBilinearly( SAMPLER2DPARAM(lTextureMap), vec2( t0.x, t0.y ), s.x );
    smp      += SampleTextureBilinearly( SAMPLER2DPARAM(lTextureMap), vec2( t1.x, t0.y ), s.y );
    smp      += SampleTextureBilinearly( SAMPLER2DPARAM(lTextureMap), vec2( t0.x, t1.y ), s.z );
    smp      += SampleTextureBilinearly( SAMPLER2DPARAM(lTextureMap), vec2( t1.x, t1.y ), s.w );
    smp.rgb   = DecodeSample( smp, s );

    return smp.rgb;
}


FRAGMENT_MAIN_COLOUR_SRT
{
    type4 lFragCol;
    lFragCol.rgb = CatmullRomTextureSample4(SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS.xy );
    lFragCol.w   = 1.0;

    WRITE_FRAGMENT_COLOUR( lFragCol );
}

#if 0 // Unoptimised Reference

vec4
SampleTextureBilinearlyAndUnpack(
    SAMPLER2DARG( lTextureMap ),
    vec2  uv,
    float sf )
{
    vec4 sample_color  = texture2DLod( lTextureMap, uv, 0.0 );
    //sample_color       = 2.0 * sample_color - 1.0;
    sample_color      *= 1023.0;
    sample_color      -= 511.0;
    sample_color      /= 512.0;
    return sample_color * sf;
}

// based on https://www.decarpentier.nl/2d-catmull-rom-in-4-samples
vec3
CatmullRomTextureSample4(
    SAMPLER2DARG( lTextureMap ),
    vec2          lTexCoord )
{
    vec2 texSize   = vec2( GetTexResolution( lTextureMap ) );

    vec2 samplePos = lTexCoord * texSize;
    vec2 pixelPos  = floor( samplePos - 0.5 ) + 0.5;

    // Compute the fractional offset from our starting texel to our original sample location, which we'll
    // feed into the Catmull-Rom spline function to get our filter weights.
    vec2 f  = samplePos - pixelPos;

    vec2 s0	= -( 0.5 * f - 0.5) * f;         // =-w0 / (1 - f)
    vec2 s1 =  (-1.5 * f + 1.0) * f + 1.0;   // = w1 / (1 - f)
    vec2 s2 =  (-1.5 * f + 2.0) * f + 0.5;   // = w2 / (f)
    vec2 s3 = -( 0.5 * f - 0.5) * f;         // =-w3 / (f)

    vec2 f0  = s1 / ( s1 + s0 );
    vec2 f1  = s3 / ( s3 + s2 );

    vec2 t0 = pixelPos - 1.0 + f0;
    vec2 t1 = pixelPos + 1.0 + f1;

    t0 /= texSize;
    t1 /= texSize;

    s0 *= 1.0 - f;
    s1 *= 1.0 - f;
    s2 *= f;
    s3 *= f;

    float sf = ( ivec2( pixelPos ).x & 1 ) == 
               ( ivec2( pixelPos ).y & 1 ) ? 1.0 : -1.0;

    vec4  smp;
    vec4  s;

    s.x    =    ( s0.x + s1.x ) * ( s0.y + s1.y );
    s.y    =   -( s2.x + s3.x ) * ( s0.y + s1.y );
    s.z    =   -( s0.x + s1.x ) * ( s2.y + s3.y );
    s.w    =    ( s2.x + s3.x ) * ( s2.y + s3.y );

    smp    = SampleTextureBilinearlyAndUnpack( SAMPLER2DPARAM(lTextureMap), vec2( t0.x, t0.y ), sf ) * s.x;
    smp   += SampleTextureBilinearlyAndUnpack( SAMPLER2DPARAM(lTextureMap), vec2( t1.x, t0.y ), sf ) * s.y;
    smp   += SampleTextureBilinearlyAndUnpack( SAMPLER2DPARAM(lTextureMap), vec2( t0.x, t1.y ), sf ) * s.z;
    smp   += SampleTextureBilinearlyAndUnpack( SAMPLER2DPARAM(lTextureMap), vec2( t1.x, t1.y ), sf ) * s.w;

    smp   /= smp.a;

    return smp.rgb;
}

#endif

#endif

// =================================================================================================
//
// D_POSTPROCESS_FLIP_AND_PACK
//
// =================================================================================================

#ifdef D_POSTPROCESS_FLIP_AND_PACK

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoordsVec2  = TEX_COORDS.xy;
    vec4  lFragCol         = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lFragCoordsVec2, 0.0 );
    lFragCol.w             = 1.0;

    ivec2 lFragCoordsIVec2 = ivec2( lFragCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );

    lFragCol               = round( min( lFragCol * 512.0, 511.0 ) );

    //lFragCol = vec4( 1.0, 0.0, 0.0, 1.0 );

    if ( ( lFragCoordsIVec2.x & 1 ) !=
         ( lFragCoordsIVec2.y & 1 ) )
    {
        //lFragCol *= -1.0;
        lFragCol = -lFragCol;
    }

    //lFragCol    = lFragCol * 0.5 + 0.5;
    lFragCol   += 511.0;
    lFragCol    = lFragCol / 1023.0;

    WRITE_FRAGMENT_COLOUR( lFragCol );
}


#endif

// =================================================================================================
//
// D_POSTPROCESS_DOWNSAMPLE_FLPY
//
// =================================================================================================

#ifdef D_POSTPROCESS_DOWNSAMPLE_FLPY

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lFragCoordsVec2 = TEX_COORDS.xy;
    lFragCoordsVec2.y = 1.0 - lFragCoordsVec2.y;
    vec4 lFragCol = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), lFragCoordsVec2, 0.0);
    lFragCol.w = 1.0;
    WRITE_FRAGMENT_COLOUR(lFragCol);
}


#endif

// =================================================================================================
//
// BLUR
//
// =================================================================================================

#ifdef D_POSTPROCESS_GUASS

//-----------------------------------------------------------------------------
//      Global Data

//blur params x = bool x-axis, y = bool y-axis, z = Sigma
// The sigma value for the gaussian function: higher value means more blur
// A good value for 9x9 is around 3 to 5
// A good value for 7x7 is around 2.5 to 4
// A good value for 5x5 is around 2 to 3.5
// ... play around with this based on what you need :)



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#if defined( D_PLATFORM_SWITCH )
//precision mediump float;
#endif


//-----------------------------------------------------------------------------
//      Functions 


FRAGMENT_MAIN_COLOUR_SRT
{    
    const float pi = 3.14159265;
    vec3   incrementalGaussian; // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
    vec4   avgValue = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4   lCenter;
    float coefficientSum = 0.0f;
    float sigma;

    vec2 lStepSizeVec2 = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    lStepSizeVec2     *= lUniforms.mpCustomPerMesh.gBlurParamsVec4.xy;
    vec2    lTexCoords = TEX_COORDS.xy;

    lCenter = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );

    // The sigma value for the gaussian function: higher value means more blur
    // A good value for 9x9 is around 3 to 5
    // A good value for 7x7 is around 2.5 to 4
    // A good value for 5x5 is around 2 to 3.5
    // ... play around with this based on what you need :)
    sigma = lUniforms.mpCustomPerMesh.gBlurParamsVec4.z;

    incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * sigma);
    incrementalGaussian.y = exp(-0.5f / (sigma * sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

    // Take the central sample first...
    avgValue                = lCenter * incrementalGaussian.x;
    coefficientSum         += incrementalGaussian.x;
    
#if !defined( D_GUASS_BLUR_HALF )
    // Go through the remaining X directional samples (D_GUASS_BLUR_TAPS on each side of the center)
    for (int i = 1; i <= D_GUASS_BLUR_RADIUS; i++) 
    {
        incrementalGaussian.xy *= incrementalGaussian.yz;

        vec4 lTexPos;
        vec4 lTexNeg;

        lTexPos = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords.xy + float(i) * lStepSizeVec2, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );
        lTexNeg = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords.xy - float(i) * lStepSizeVec2, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );

        avgValue += lTexNeg * incrementalGaussian.x;         
        avgValue += lTexPos * incrementalGaussian.x;
               
        coefficientSum         += 2.0 * incrementalGaussian.x;        
    }
#else    

    for (int i = 1; i <= D_GUASS_BLUR_RADIUS; i+=2 ) 
    {
        vec4 lTexPos;
        vec4 lTexNeg;

        incrementalGaussian.xy *= incrementalGaussian.yz;
        float lfWeight          = incrementalGaussian.x;
        coefficientSum         += 2.0 * incrementalGaussian.x;

        incrementalGaussian.xy *= incrementalGaussian.yz;
        float lfWeightNext      = incrementalGaussian.x;
        coefficientSum         += 2.0 * incrementalGaussian.x;        

        float lfWeightPairSum   = lfWeight + lfWeightNext;
        vec2 lOffsetVec2        = ( float(i) + lfWeightNext / lfWeightPairSum ) * lStepSizeVec2;

        lTexPos = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords.xy + lOffsetVec2, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );
        lTexNeg = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords.xy - lOffsetVec2, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );

        #if defined(D_BROADCAST_G) // broadcast single channel bloom
        lTexPos = lTexPos.gggg;
        lTexNeg = lTexNeg.gggg;
        #endif

        avgValue += lTexNeg * lfWeightPairSum;
        avgValue += lTexPos * lfWeightPairSum;
    }
#endif

    #if defined(D_SKIP_B) // added to avoid blurring DOF mask
    avgValue.b = lCenter.b * coefficientSum;
    #endif

    #ifdef D_GUASS_BLUR_ADD
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR + vec4( avgValue.rgb / coefficientSum, 1.0 ) );
    #else
    WRITE_FRAGMENT_COLOUR( vec4( avgValue.rgb / coefficientSum, 1.0 ) );
    #endif
   
}
#endif

// =================================================================================================
//
// BLUR
//
// =================================================================================================

#ifdef D_POSTPROCESS_GUASS_FLAT_CUBEMAP

//-----------------------------------------------------------------------------
//      Global Data

//blur params x = bool x-axis, y = bool y-axis, z = Sigma
// The sigma value for the gaussian function: higher value means more blur
// A good value for 9x9 is around 3 to 5
// A good value for 7x7 is around 2.5 to 4
// A good value for 5x5 is around 2 to 3.5
// ... play around with this based on what you need :)



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 


STATIC_CONST ivec2 kaFaceRotIdx[ 6 ][ 4 ] =
{
    // Up, Right, Down, Left
    { ivec2( 4, 0 ), ivec2( 1, 0 ), ivec2( 5, 0 ), ivec2( 3, 0 ) },
    { ivec2( 4, 1 ), ivec2( 2, 0 ), ivec2( 5, 3 ), ivec2( 0, 0 ) },
    { ivec2( 4, 2 ), ivec2( 3, 0 ), ivec2( 5, 2 ), ivec2( 1, 0 ) },
    { ivec2( 4, 3 ), ivec2( 0, 0 ), ivec2( 5, 1 ), ivec2( 2, 0 ) },
    { ivec2( 2, 2 ), ivec2( 1, 3 ), ivec2( 0, 0 ), ivec2( 3, 1 ) },
    { ivec2( 0, 0 ), ivec2( 1, 1 ), ivec2( 2, 2 ), ivec2( 3, 3 ) }
};

STATIC_CONST vec2 kaRots[ 4 ][ 2 ] =
{
    { vec2(  1.0,  1.0 ),   vec2(  0.0,  0.0 ) },
    { vec2(  0.0,  0.0 ),   vec2(  1.0, -1.0 ) },
    { vec2( -1.0, -1.0 ),   vec2(  0.0,  0.0 ) },
    { vec2(  0.0,  0.0 ),   vec2( -1.0,  1.0 ) }
};

vec2 Stretch  ( vec2 lUVs )             { return lUVs.xy * vec2( 6.0, 1.0 ); }
vec2 Unstretch( vec2 lUVs )             { return lUVs.xy / vec2( 6.0, 1.0 ); }
vec2 Center   ( vec2 lUVs )             { return lUVs.xy - float2vec2( 0.5 ); }
vec2 Decenter ( vec2 lUVs )             { return lUVs.xy + float2vec2( 0.5 ); }
vec2 Fract    ( vec2 lUVs, int liDir )  { return vec2( fract( lUVs.x ), bool( liDir & 0x1 ) ? lUVs.y : 1.0 - fract( lUVs.y ) ); }
vec2 ShiftL   ( vec2 lUVs, int liFace ) { return lUVs.xy - vec2( float( liFace ), 0.0 ); }
vec2 ShiftR   ( vec2 lUVs, int liFace ) { return lUVs.xy + vec2( float( liFace ), 0.0 ); }
vec2 Rotate   ( vec2 lUVs, int liRot  ) { return lUVs.xy * kaRots[ liRot ][ 0 ] + lUVs.yx * kaRots[ liRot ][ 1 ]; }

ivec2
DirectionIdx(
    vec2 lUVsPos,
    vec2 lUVsNeg )
{
    ivec2 lDirIdxVec2 = ivec2( -1, -1 );

    // Up, Right, Down, Left
    lDirIdxVec2.x = lUVsPos.y >= 1.0 ? 0 : -1;
    lDirIdxVec2.x = lUVsPos.x >= 1.0 ? 1 : -1;
    lDirIdxVec2.y = lUVsNeg.y <  0.0 ? 2 : -1;
    lDirIdxVec2.y = lUVsNeg.x <  0.0 ? 3 : -1;

    return lDirIdxVec2;
}

vec2
GetFlatCubemapCoords(
    vec2 lUVs,
    int  liFaceIdx,
    int  liDirIdx )
{
    if ( liDirIdx == -1 ) return ShiftR( lUVs, liFaceIdx );
    
    int     liFaceAdjIdx    = kaFaceRotIdx[ liFaceIdx ][ liDirIdx ].x;
    int     liRotIdx        = kaFaceRotIdx[ liFaceIdx ][ liDirIdx ].y;
    vec2    lUVsAdjVec2     = ShiftR( Decenter( Rotate( Center( lUVs ), liRotIdx ) ), liFaceAdjIdx );
    
    return lUVsAdjVec2;
}

void
ComputeCoords(
    in  vec2  lUVs,
    in  vec2  lStepSizeVec2,
    in  float lfOffset,
    out vec2  lUVsPos,
    out vec2  lUVsNeg )
{
    int liFace;
    
    liFace  = int( Stretch( lUVs ).x );

    lUVsPos = lUVs + lfOffset * lStepSizeVec2;
    lUVsNeg = lUVs - lfOffset * lStepSizeVec2;

    vec2    lUVsFacePos = ShiftL( Stretch( lUVsPos ), liFace );
    vec2    lUVsFaceNeg = ShiftL( Stretch( lUVsNeg ), liFace );
    ivec2   lDirVec2    = DirectionIdx( lUVsFacePos, lUVsFaceNeg );

    lUVsFacePos = lDirVec2.x == -1 ? lUVsFacePos : Fract( lUVsFacePos, lDirVec2.x );
    lUVsFaceNeg = lDirVec2.y == -1 ? lUVsFaceNeg : Fract( lUVsFaceNeg, lDirVec2.y );

    lUVsPos = GetFlatCubemapCoords( lUVsFacePos, liFace, lDirVec2.x );
    lUVsNeg = GetFlatCubemapCoords( lUVsFaceNeg, liFace, lDirVec2.y );

    lUVsPos = Unstretch( lUVsPos );
    lUVsNeg = Unstretch( lUVsNeg );
}

FRAGMENT_MAIN_COLOUR_SRT
{    
    const float pi = 3.14159265;
    vec3   incrementalGaussian; // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
    vec4   avgValue = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4   lCenter;
    float coefficientSum = 0.0f;
    float sigma;

    // Assumes bound texture and output texture have the same size and mip settings
    vec2 lStepSizeVec2 = 1.0 / GetTexResolutionLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), int( lUniforms.mpCustomPerMesh.gBlurParamsVec4.w ) );
    lStepSizeVec2     *= lUniforms.mpCustomPerMesh.gBlurParamsVec4.xy;
    vec2    lTexCoords = TEX_COORDS.xy;

    lCenter = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );

    // The sigma value for the gaussian function: higher value means more blur
    // A good value for 9x9 is around 3 to 5
    // A good value for 7x7 is around 2.5 to 4
    // A good value for 5x5 is around 2 to 3.5
    // ... play around with this based on what you need :)
    sigma = lUniforms.mpCustomPerMesh.gBlurParamsVec4.z;

    incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * sigma);
    incrementalGaussian.y = exp(-0.5f / (sigma * sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

    // Take the central sample first...
    avgValue                = lCenter * incrementalGaussian.x;
    coefficientSum         += incrementalGaussian.x;
    
    // Go through the remaining X directional samples (D_GUASS_BLUR_TAPS on each side of the center)
    for (float i = 1.0f; i <= D_GUASS_BLUR_RADIUS; i++) 
    {
        incrementalGaussian.xy *= incrementalGaussian.yz;

        vec2 lUVsPos;
        vec2 lUVsNeg;

        vec4 lTexPos;
        vec4 lTexNeg;

        ComputeCoords( lTexCoords, lStepSizeVec2, i, lUVsPos, lUVsNeg );

        lTexPos     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lUVsPos, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );
        lTexNeg     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lUVsNeg, lUniforms.mpCustomPerMesh.gBlurParamsVec4.w );

        avgValue   += lTexNeg * incrementalGaussian.x;
        avgValue   += lTexPos * incrementalGaussian.x;

        coefficientSum  += 2 * incrementalGaussian.x;
    }

    #ifdef D_GUASS_BLUR_ADD
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR + vec4( avgValue.rgb / coefficientSum, 1.0 ) );
    #else
    WRITE_FRAGMENT_COLOUR( vec4( avgValue.rgb / coefficientSum, 1.0 ) );
    #endif
   
}
#endif

#ifdef D_POSTPROCESS_GUASS_SQUARE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 



FRAGMENT_MAIN_COLOUR_SRT
{    
    const float pi = 3.14159265;
    vec4    lCenter;
    vec3    incrementalGaussian; // Incremental Gaussian Coefficent Calculation (See GPU Gems 3 pp. 877 - 889)
    vec4    avgValue        = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    vec4    avgLineValue    = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    float   sigma           = lUniforms.mpCustomPerMesh.gBlurParamsVec4.z;
    vec2    lStepSizeVec2   = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    vec2    lTexCoords      = TEX_COORDS.xy;

    float   coefficientSum;
    float   coefficients[ D_GUASS_BLUR_RADIUS + 1 ];

    incrementalGaussian.x = 1.0f / (sqrt(2.0f * pi) * sigma);
    incrementalGaussian.y = exp(-0.5f / (sigma * sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;    
    coefficients[ 0 ]     = incrementalGaussian.x;
    coefficientSum        = incrementalGaussian.x;

    for (int i = 1; i <= D_GUASS_BLUR_RADIUS; i++)
    {
        incrementalGaussian.xy *= incrementalGaussian.yz;
        coefficients[ i ]       = incrementalGaussian.x;
        coefficientSum         += 2 * incrementalGaussian.x;
    }

    for (float j = -D_GUASS_BLUR_RADIUS; j <= D_GUASS_BLUR_RADIUS; j++)
    {
        lCenter         = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords + j * lStepSizeVec2 * vec2( 0.0, 1.0 ), 0.0 );
        avgLineValue    = lCenter * coefficients[ 0 ];        
            
        for (float i = 1.0f; i <= D_GUASS_BLUR_RADIUS; i++) 
        {
            vec4 lTexPos;
            vec4 lTexNeg;

            lTexPos = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords + vec2( i, j ) * lStepSizeVec2, 0.0);
            lTexNeg = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords - vec2( i, j ) * lStepSizeVec2, 0.0);

            avgLineValue += lTexNeg * coefficients[ int( i ) ];
            avgLineValue += lTexPos * coefficients[ int( i ) ];
        }

        avgValue += avgLineValue / coefficientSum * coefficients[ int( abs( j ) ) ];
    }
    
    avgValue /= coefficientSum * lUniforms.mpCustomPerMesh.gBlurParamsVec4.w;

    
    #if defined(D_SKIP_B) // added to avoid blurring DOF mask
    avgValue.b = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords, 0.0f ).b;
    #endif

    #ifdef D_GUASS_BLUR_ADD
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR + vec4( avgValue.rgb, 1.0 ) );
    #else
    WRITE_FRAGMENT_COLOUR( vec4( avgValue.rgb, 1.0 ) );
    #endif   
}
#endif


// =================================================================================================
//
// BLUR_KAWASE
//
// =================================================================================================

#if defined(D_POSTPROCESS_BLUR_KAWASE) || defined(D_POSTPROCESS_BLUR_KAWASE_PRESERVE_ALPHA)

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//    Functions

FRAGMENT_MAIN_COLOUR_SRT
{
#if defined ( D_PLATFORM_SWITCH )
    vec2 lRecipTexSize = vec2( 1.0 / 320.0, 1.0 / 180.0 );
#else
    vec2 lRecipTexSize = vec2( 1.0 / 480.0, 1.0 / 270.0 ); // Fixed buffer sized assumption!
#endif

#ifdef D_POSTPROCESS_BLUR_KAWASE
    #ifdef D_BLUR_KAWASE_ADD
        WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR + vec4( BlurKawase( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), TEX_COORDS, lRecipTexSize, lUniforms.mpCustomPerMesh.gBlurParamsVec4.x ).xyz, 1.0 ) );
    #else
        WRITE_FRAGMENT_COLOUR( vec4( BlurKawase( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), TEX_COORDS, lRecipTexSize, lUniforms.mpCustomPerMesh.gBlurParamsVec4.x ).xyz, 1.0 ) );
    #endif
#else
    WRITE_FRAGMENT_COLOUR(       BlurKawase( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), TEX_COORDS, lRecipTexSize, lUniforms.mpCustomPerMesh.gBlurParamsVec4.x ) );
#endif 
    
}

#endif


// =================================================================================================
//
// D_FX_COMBINE
//
// =================================================================================================

#ifdef D_POSTPROCESS_FX_COMBINE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 
 
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions
float
RGBToLuminance(
    vec3 lfLinearRgb )
{
    return dot( lfLinearRgb, vec3( 0.2126f, 0.7152f, 0.0722f ) );
}

float
PerceivedLuminance(
    float lfLuminance )
{
    float lfPerceivedLuminance = 0;

    if ( lfLuminance <= 216.0f / 24389.0f )
    {
        lfPerceivedLuminance = lfLuminance * 24389.0f / 27.0f;
    }
    else
    {
        lfPerceivedLuminance = pow( lfLuminance, 1.0f / 3.0f ) * 116.0f - 16.0f;
    }

    return lfPerceivedLuminance * 0.01f;
}

float
PerceivedLuminanceEncoded(
    float lfLuminance )
{
    return pow( PerceivedLuminance( lfLuminance ), 1.0f / 4.0f );
}

float
EncodeLogLuminance(
    float lfLogLuminance )
{
    return -lfLogLuminance / 3.0;
}

float
EncodePerceivedLuminance(
    float lfPerceivedLuminance )
{
    return pow( lfPerceivedLuminance, 1.0f / 4.0f );
}

vec3
Tonemap(
    vec3 fRgb )
{
    vec3 lvMaxChannel = float2vec3( max( max( 0.f, fRgb.r ), max( fRgb.g, fRgb.b ) ) + 1.f );
    return fRgb / lvMaxChannel;
}

vec3
RGBToYRB(
    vec3 fRgb )
{
    return vec3( fRgb.g + ( fRgb.r + fRgb.b ) / 2.0f, fRgb.r, ( fRgb.r + fRgb.b ) / 2.0f );
}

vec3
RGBToYCoCg(
    vec3 fRgb )
{
    vec3 fYCoCg;
    fYCoCg.y = fRgb.r - fRgb.b;
    float tmp = fRgb.b + fYCoCg.y / 2.0;
    fYCoCg.z = fRgb.g - tmp;
    fYCoCg.x = tmp + fYCoCg.z / 2.0;
    return fYCoCg;
}

vec3
PrepareInputFSR2(
    in  vec3 lvFragCol,
    out vec2 lvLuminance )
{
    float lfLuminance;
    float lfLogLuminance;
    float lfPerceivedLum;
    float lfPerceivedLumEcd;

    lfLuminance           = RGBToLuminance( lvFragCol );
    lfLogLuminance        = EncodeLogLuminance( log( max( 1e-03, lfLuminance ) ) );
    lvFragCol             = Tonemap( lvFragCol );
    lfLuminance           = RGBToLuminance( lvFragCol );
    lfPerceivedLum        = PerceivedLuminance( lfLuminance );
    lfPerceivedLumEcd     = EncodePerceivedLuminance( lfPerceivedLum );
    lvFragCol             = RGBToYCoCg( lvFragCol );
    lvLuminance           = vec2( lfLogLuminance, lfPerceivedLumEcd );

    return lvFragCol;
}

float
Exposure(
    SAMPLER2DARG( lExpBuff ) )
{
    float lfExp;
    lfExp  = texture2D(lExpBuff, vec2(0.25, 0.25)).r;
    lfExp += texture2D(lExpBuff, vec2(0.75, 0.25)).r;
    lfExp += texture2D(lExpBuff, vec2(0.25, 0.75)).r;
    lfExp += texture2D(lExpBuff, vec2(0.75, 0.75)).r;

    return lfExp;
}

float
ColorGradeSmoothClamp(
    float x )
{
    #if defined ( D_PLATFORM_METAL )
    const float u = 0.125;
    float q = ( 2.0 - u - 1.0 / u + x * ( 2.0 + 2.0 / u - x / u ) ) / 4.0;
    #else
    STATIC_CONST float u = 0.125;
    STATIC_CONST float q = ( 2.0 - u - 1.0 / u + x * ( 2.0 + 2.0 / u - x / u ) ) / 4.0;
    #endif

    return ( abs( 1.0 - x ) < u ) ? q : saturate( x );
}

vec3
LinearToGamma01(
    vec3 lvLinearColour )
{
    vec3 lvGammaColour;

    lvGammaColour.r = pow( lvLinearColour.r, 0.45454545454 );
    lvGammaColour.g = pow( lvLinearColour.g, 0.45454545454 );
    lvGammaColour.b = pow( lvLinearColour.b, 0.45454545454 );

    return lvGammaColour;
}

vec3
GammaToLinear01(
    vec3 lvGammaColour )
{
    vec3 lvLinearColour;

    lvLinearColour = lvGammaColour * ( lvGammaColour * ( lvGammaColour * ( 0.305306011 ) + ( 0.682171111 ) ) + ( 0.012522878 ) );

    return lvLinearColour;
}

// Precomputed white point TonemapKodak( 1.0 ) and its rcp
STATIC_CONST float kfWpScale    = 0.401149425287;
STATIC_CONST float kfWpScaleRcp = 2.49283667622;

// NOTE(sal): Here we're trying to do colour grading in HDR.
// Normally, we'd use LUT textures especially made for a log hdr linear space.
// However, we need to reuse the existing LUT textures, which were made for gamma LDR.
// As a workaround, we use the approach described in https://www.glowybits.com/blog/2016/12/21/ifl_iss_hdr_1/
vec3
ColourGradeHdr(
    in PerFrameUniforms      lPerFrameUniforms,
    in CustomPerMeshUniforms lPerMeshUniforms,
    in vec3                  lFragCol,
    in float                 lfDepth,
    in float                 lfLutFxAmount )
{
    float lfMax;
    float lfScaleRcp;
    vec3  lvOffset;

    lFragCol    = MUL( lFragCol, P3D65_TO_sRGB );
    lFragCol    = max( float2vec3( 0.0 ), lFragCol );
    lfMax       = max( 1.0e-6, max( lFragCol.r, max( lFragCol.g, lFragCol.b ) ) );

    // Soft Clamp to 1.0; store the scale to go back to HDR a few lines later
    lfScaleRcp  = ColorGradeSmoothClamp( lfMax ) / lfMax;
    lFragCol    = lFragCol * lfScaleRcp;

    // Tonemap + Gamma + Lut + revert everything to go back to the original space
    // Note that the tonmap doesn't actually compress the range (that's done by the smooth clamp).
    // The tonemap is there mostly to give our colours a bit more punch.
    lFragCol    = TonemapKodak( lFragCol.rgb ) * kfWpScaleRcp;
    lFragCol    = LinearToGamma01( lFragCol.rgb );
    lFragCol    = ApplyColourLUT( lPerFrameUniforms, lPerMeshUniforms, lFragCol, lfDepth, lfLutFxAmount );
    lFragCol    = GammaToLinear01( lFragCol.rgb );
    lFragCol    = TonemapKodakInverse( lFragCol.rgb * kfWpScale );
    lFragCol    = lFragCol / lfScaleRcp;

    return lFragCol;
}

//----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lTexCoords    = TEX_COORDS.xy;
    vec3    lFragCol      = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lTexCoords, 0.0 ).xyz;
    vec3    lBloomCol     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lTexCoords, 0.0 ).xyz;
    #if defined( D_LENS_FLARE )
    vec3    lLensFlareCol = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lTexCoords, 0.0 ).xyz;
    #endif
    float   lfDepth       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lTexCoords, 0.0 ).x;

    #if defined( D_LUT )
    float   lfLutFxAmount = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lTexCoords, 0.0 ).x;
    #endif

    #if defined( D_BLOOM_EXP )
    float   lfExposure    = Exposure( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer5Map ) );
    #endif

    #if defined( D_DEBANDFILTER )
    // Deband... but don't deband anything other than the background sky/space
    lFragCol              = DebandFilter( DEREF_PTR( lUniforms.mpPerFrame ), lFragCol, lTexCoords, lfDepth,
                                          SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ),
                                          SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap  ) );
    #endif

    #if defined( D_LENS_FLARE )
    lFragCol             += lLensFlareCol;
    #endif

    #if defined( D_BLOOM_EXP )
    lFragCol             += lBloomCol * lfExposure;
    #else
    lFragCol             += lBloomCol;
    #endif

    #if defined( D_LUT )
    lFragCol              = ColourGradeHdr( DEREF_PTR( lUniforms.mpPerFrame ),
                                            DEREF_PTR( lUniforms.mpCustomPerMesh ),
                                            lFragCol, lfDepth, lfLutFxAmount );
    #endif

    #if defined( D_GAMMA )
    lFragCol              = GammaCorrectOutput( lFragCol );
    #endif

    WRITE_FRAGMENT_COLOUR(  vec4( lFragCol, 1.0 ) );
}

#endif // D_LENSFLARE_COMBINE


// =================================================================================================
//
// COMBINE
//
// =================================================================================================

#ifdef D_POSTPROCESS_COMBINE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

//TF_BEGIN
float
Exposure(SAMPLER2DARG(lExpBuff))
{
    float lfExp;
    lfExp  = texture2D(lExpBuff, vec2(0.25, 0.25)).r;
    lfExp += texture2D(lExpBuff, vec2(0.75, 0.25)).r;
    lfExp += texture2D(lExpBuff, vec2(0.25, 0.75)).r;
    lfExp += texture2D(lExpBuff, vec2(0.75, 0.75)).r;

    return lfExp;
}

float
ColorGradeSmoothClamp(
    float x )
{
#if defined ( D_PLATFORM_METAL )
    const float u = 0.125;
    float q = ( 2.0 - u - 1.0 / u + x * ( 2.0 + 2.0 / u - x / u ) ) / 4.0;
#else
    STATIC_CONST float u = 0.125;
    STATIC_CONST float q = ( 2.0 - u - 1.0 / u + x * ( 2.0 + 2.0 / u - x / u ) ) / 4.0;
#endif

    return ( abs( 1.0 - x ) < u ) ? q : saturate( x );
}

vec3
LinearToGamma01(
    vec3 lvLinearColour )
{
    vec3 lvGammaColour;

    lvGammaColour.r = pow( lvLinearColour.r, 0.45454545454 );
    lvGammaColour.g = pow( lvLinearColour.g, 0.45454545454 );
    lvGammaColour.b = pow( lvLinearColour.b, 0.45454545454 );

    return lvGammaColour;
}

vec3
GammaToLinear01(
    vec3 lvGammaColour )
{
    vec3 lvLinearColour;

    lvLinearColour = lvGammaColour * ( lvGammaColour * ( lvGammaColour * ( 0.305306011 ) + ( 0.682171111 ) ) + ( 0.012522878 ) );

    return lvLinearColour;
}

// Precomputed white point TonemapKodak( 1.0 ) and its rcp
STATIC_CONST float kfWpScale    = 0.401149425287;
STATIC_CONST float kfWpScaleRcp = 2.49283667622;

// NOTE(sal): Here we're trying to do colour grading in HDR.
// Normally, we'd use LUT textures especially made for a log hdr linear space.
// However, we need to reuse the existing LUT textures, which were made for gamma LDR.
// As a workaround, we use the approach described in https://www.glowybits.com/blog/2016/12/21/ifl_iss_hdr_1/
vec3
ColourGradeHdr(
    in PerFrameUniforms      lPerFrameUniforms,
    in CustomPerMeshUniforms lPerMeshUniforms,
    in vec3                  lFragCol,
    in float                 lfDepth,
    in float                 lfLutFxAmount )
{
    float lfMax;
    float lfScaleRcp;
    vec3  lvOffset;

    lFragCol    = MUL( lFragCol, P3D65_TO_sRGB );
    lFragCol    = max( float2vec3( 0.0 ), lFragCol );
    lfMax       = max( 1.0e-6, max( lFragCol.r, max( lFragCol.g, lFragCol.b ) ) );

    // Soft Clamp to 1.0; store the scale to go back to HDR a few lines later
    lfScaleRcp  = ColorGradeSmoothClamp( lfMax ) / lfMax;
    lFragCol    = lFragCol * lfScaleRcp;

    // Tonemap + Gamma + Lut + revert everything to go back to the original space
    // Note that the tonmap doesn't actually compress the range (that's done by the smooth clamp).
    // The tonemap is there mostly to give our colours a bit more punch.
    lFragCol    = TonemapKodak( lFragCol.rgb ) * kfWpScaleRcp;
    lFragCol    = LinearToGamma01( lFragCol.rgb );
    lFragCol    = ApplyColourLUT( lPerFrameUniforms, lPerMeshUniforms, lFragCol, lfDepth, lfLutFxAmount );
    lFragCol    = GammaToLinear01( lFragCol.rgb );
    lFragCol    = TonemapKodakInverse( lFragCol.rgb * kfWpScale );
    lFragCol    = lFragCol / lfScaleRcp;

    return lFragCol;
}
//TF_END

#ifdef D_OUTPUT_LUMINANCE
FRAGMENT_MAIN_COLOUR01_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{
    float lfDepth;
    vec3  lFragCol;

    lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), TEX_COORDS ).rgb;

#if defined ( D_FORWARD ) && defined ( D_PLATFORM_METAL )
//TF_BEGIN
    vec2  newCoords;
    newCoords = IN(mTexCoordsVec2).xy;

    // Apply tonemap (in deferred done during depthOfField quad)
    {
        lFragCol.rgb = TonemapKodak(lFragCol.rgb) / TonemapKodak( vec3(1.0,1.0,1.0) );
    }

    // (in deferred done during lensflare resolve stage)
    // Apply Deband... but don't deband anything other than the background sky/space 
    lfDepth     = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ).x;
    lFragCol.rgb = DebandFilter(DEREF_PTR(lUniforms.mpPerFrame), lFragCol.rgb, newCoords, lfDepth, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBuffer1Map), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap));

    // Apply lens flare
    vec3 lLensFlareColorVec3 = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer5Map), newCoords).rgb;
    lFragCol.rgb += lLensFlareColorVec3;

#ifdef D_BLOOM
    float lfExposure = Exposure(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBuffer4Map));
    vec3 lBloomColor = vec3(texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer3Map), newCoords).r * lfExposure);

    lFragCol.rgb += lBloomColor;
#endif

    //TF_END
#endif // #ifdef D_FORWARD
    
#ifdef D_FORWARD
    // FX_COMBINE
    float lfLUTEffectAmount = 0.0;
    lfDepth     = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ).x;
#if defined(D_PLATFORM_METAL)
    uint lStencil = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gStencilBuffer), newCoords).r;
    if (lStencil & 0x24 && lfDepth < 1.0f)
        lfLUTEffectAmount = 1.0f;
#endif
    lFragCol              = ColourGradeHdr( DEREF_PTR( lUniforms.mpPerFrame ),
                                            DEREF_PTR( lUniforms.mpCustomPerMesh ),
                                            lFragCol, lfDepth, lfLUTEffectAmount );

#if defined( D_PLATFORM_METAL )
    // Screeneffect
    #if defined(D_SCREENEFFECT)
    lFragCol.rgb = ScreenEffect(
        DEREF_PTR(lUniforms.mpPerFrame),
        DEREF_PTR(lUniforms.mpCommonPerMesh),
        DEREF_PTR(lUniforms.mpCustomPerMesh),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gUIFullscreenEffect),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gUIFullscreenNormal),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gUIFullScreenRefraction),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gUICamoEffect),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gUICamoNormal),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap),
        lFragCol.rgb,
        newCoords
    );
    #endif
#endif

    // Color already in input gamma
    //lFragCol.rgb = clamp(MUL((lFragCol.rgb), P3D65_TO_sRGB), float2vec3(0.0), float2vec3(1.0));
    lFragCol.rgb = clamp(lFragCol.rgb, float2vec3(0.0), float2vec3(1.0));
    lFragCol    = TonemapKodak( lFragCol, 1.0f);
#if defined( D_PLATFORM_METAL )
    lFragCol    = GammaCorrectOutput( lFragCol );
#else
    lFragCol    = LinearToGamma01( lFragCol );
#endif
    // POSTPROCESS_COMBINE
#else
    lFragCol    = saturate( lFragCol );
    lFragCol    = TonemapKodak( lFragCol, 1.0 );
    lFragCol    = GammaCorrectOutput( lFragCol );
#endif

    #ifdef D_OUTPUT_LUMINANCE
    FRAGMENT_COLOUR0 = vec4( lFragCol, 1.0 );
    FRAGMENT_COLOUR1 = vec4( dot( lFragCol, kvLumaCoeff ), 0.0, 0.0, 1.0 );
    #else
    FRAGMENT_COLOUR  = vec4( lFragCol, 1.0 );
    #endif

}

#endif

// =================================================================================================
//
// COMBINE_AND_COPY_DEPTH
//
// =================================================================================================

#ifdef D_POSTPROCESS_COMBINE_AND_COPY_DEPTH

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_DEPTH_SRT
{
    float lfDepth;
    vec3  lFragCol;

    lfDepth     = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ).x;
    lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), TEX_COORDS ).rgb;
    lFragCol    = saturate( lFragCol );
    lFragCol    = TonemapKodak( lFragCol, 1.0 );
    lFragCol    = GammaCorrectOutput( lFragCol );

    FRAGMENT_COLOUR = vec4( lFragCol, 1.0 );
    FRAGMENT_DEPTH  = LinearNormToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth );
}

#endif

// =================================================================================================
//
// COMBINEHDR
//
// =================================================================================================

#ifdef D_POSTPROCESS_COMBINEHDR

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

vec3 SimpleReinhard2( in vec3 col )
{
    vec3 lHalfCol = col / ( vec3(1.0, 1.0, 1.0) + max( vec3(0.0, 0.0, 0.0), col ) );
    return lHalfCol;
}

vec3 SimpleUnReinhard2( in vec3 col )
{
    vec3 lHalfCol = col;
    return lHalfCol / ( vec3(1.0, 1.0, 1.0) - min( vec3(0.9999, 0.9999, 0.9999), lHalfCol ) );
}

float HybridGamma( float x, float gam )
{
    float y;

    if( x < 1.0 )
    {
        y = pow(x, gam);
    }
    else
    {
        y = x * gam - (gam - 1.0);
    }

    return y;
}

vec3 HybridGamma3( vec3 x, float gam )
{
    vec3 y;

    y.x = HybridGamma(x.x, gam);
    y.y = HybridGamma(x.y, gam);
    y.z = HybridGamma(x.z, gam);

    return y;

}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 newCoords = IN(mTexCoordsVec2).xy;

    vec4  lFragCol          = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), newCoords);
    float lfLUTEffectAmount = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), newCoords).r;
   
    {
            lFragCol.xyz = lFragCol.rgb;
            // switch to ACES AP1 color space (results in a more consistent curve)
            lFragCol.rgb = MUL( lFragCol.rgb, P3D65_TO_AP1 );
            lFragCol.xyz *= 10.0;

            // NOTE(sal): this is the old workaround charlie wrote to make use of our 3d LUT textures
            // in linear HDR space even if they were authored for gamma sRGB;
            // I've now replaced this with ColourGradeHdr, in the PostProcess resolve pass.
            // From my tests that gives us a closer mapping to the way the game looks in SDR.
            #if 0
            float lfDepth = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer1Map), newCoords).x;
            vec3 lFragColLut = SimpleReinhard2( lFragCol.rgb ); 
            lFragColLut *= lUniforms.mpCustomPerMesh.gHDRParamsVec4.x;
            lFragColLut = ApplyColourLUT( DEREF_PTR( lUniforms.mpPerFrame ), DEREF_PTR( lUniforms.mpCustomPerMesh ), lFragColLut, lfDepth, lfLUTEffectAmount );
            lFragColLut /= lUniforms.mpCustomPerMesh.gHDRParamsVec4.x;
            lFragColLut = SimpleUnReinhard2( lFragColLut );

            #ifdef D_PLATFORM_PC
            lFragCol.rgb = mix( lFragCol.rgb, lFragColLut, lUniforms.mpCustomPerMesh.gHDRParams2Vec4.y );
            #else
            lFragCol.rgb = mix( lFragCol.rgb, lFragColLut, 0.8 );
            #endif

            #endif

            lFragCol.rgb *= 3.0 / 4.0;

            if( false ) // if( newCoords.y > 0.5 )
            {
                float lfLum = max( 0.0, MUL( lFragCol.rgb, P3D65_TO_XYZ ).y );

                lfLum /= 0.18; // middle gray == 1
                float lfZone = floor( log2(lfLum) + 0.5 ); // middle gray = 0.0

                if( newCoords.y > 0.9 )
                {
                    lfZone = floor( (newCoords.x * 11.0) - 5.0 );
                }

                lfLum = pow( 2.0, lfZone ) * 0.18;

                vec3 lFragXYZ = D65_White_xyY;
                lFragXYZ.z = lfLum;
                lFragCol.rgb = max( float2vec3(0.0), MUL( xyY_TO_XYZ(lFragXYZ), XYZ_TO_P3D65 ) );
            }
        

            // ACES output curve
            lFragCol.rgb = ssts_f3( lFragCol.rgb, DEREF_PTR(lUniforms.mpCustomPerMesh) );
            lFragCol.rgb = MUL( lFragCol.rgb, AP1_TO_sRGB );
            
            lFragCol.rgb *= 0.01; // 1.0 == 100 nit 

            lFragCol.xyz *= 0.25;   // invert the HDR exposure

            #ifdef D_PLATFORM_PC
            lFragCol.rgb *= 4.0 / 3.0;    //lFragCol.xyz = max( float2vec3(0.0), MUL(lFragCol.xyz, BT709_TO_BT2020) );
            #else
            lFragCol.rgb *= 4.0 /  lUniforms.mpCustomPerMesh.gHDRParams2Vec4.y;
            #endif

            lFragCol.xyz = GammaCorrectOutput(lFragCol.xyz); // and apply the nonlinear scRGB encoding to match the sRGB material
  }

	FRAGMENT_COLOUR = lFragCol;
}

#endif

// =================================================================================================
//
// COMBINEHDR_AND_COPY_DEPTH
//
// =================================================================================================

#ifdef D_POSTPROCESS_COMBINEHDR_AND_COPY_DEPTH

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

vec3 SimpleReinhard2( in vec3 col )
{
    vec3 lHalfCol = col / ( vec3(1.0, 1.0, 1.0) + max( vec3(0.0, 0.0, 0.0), col ) );
    return lHalfCol;
}

vec3 SimpleUnReinhard2( in vec3 col )
{
    vec3 lHalfCol = col;
    return lHalfCol / ( vec3(1.0, 1.0, 1.0) - min( vec3(0.9999, 0.9999, 0.9999), lHalfCol ) );
}

float HybridGamma( float x, float gam )
{
    float y;

    if( x < 1.0 )
    {
        y = pow(x, gam);
    }
    else
    {
        y = x * gam - (gam - 1.0);
    }

    return y;
}

vec3 HybridGamma3( vec3 x, float gam )
{
    vec3 y;

    y.x = HybridGamma(x.x, gam);
    y.y = HybridGamma(x.y, gam);
    y.z = HybridGamma(x.z, gam);

    return y;

}

FRAGMENT_MAIN_COLOUR_DEPTH_SRT
{
    vec2  newCoords         = IN(mTexCoordsVec2).xy;
    vec3  lFragCol          = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), newCoords).rgb;
    float lfDepth           = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), newCoords).x;
    float lfLUTEffectAmount = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), newCoords).r;

    //lFragCol = DebandFilter(DEREF_PTR(lUniforms.mpPerFrame), lFragCol, newCoords, lfDepth, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer1Map), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBufferMap));    
    
    // We calculate the original constrast scale that colourcube would have produced and apply to our HDR colour
    if( false ) 
    {
        // SDR-in-HDR (for checking purposes)
        lFragCol.rgb = saturate( lFragCol.rgb );
        lFragCol.xyz = TonemapKodak(lFragCol) / TonemapKodak(vec3(1.0, 1.0, 1.0));
        lFragCol = GammaCorrectOutput(lFragCol);        
        lFragCol = ApplyColourLUT( DEREF_PTR( lUniforms.mpPerFrame ), DEREF_PTR( lUniforms.mpCustomPerMesh ), lFragCol, lfDepth, lfLUTEffectAmount );
        lFragCol.rgb = saturate(lFragCol.rgb);
        lFragCol.rgb *= 3.2 / 3.0;
    }
    else if( false ) 
    {
        // pre-2019 HDR filter
        vec3 lToneMapCol  = TonemapKodak( saturate( lFragCol.rgb ) ) / TonemapKodak( vec3( 1.0, 1.0, 1.0 ) );
        vec3 lToneMapColG = GammaCorrectOutput( lToneMapCol );
        vec3 lLutScale    = ApplyColourLUT( DEREF_PTR( lUniforms.mpPerFrame ), DEREF_PTR( lUniforms.mpCustomPerMesh ), lToneMapColG.rgb, lfDepth, lfLUTEffectAmount );
        lFragCol.rgb *= (lLutScale + 0.005) / (lToneMapColG + 0.005); // *lToneMapScale;
    }
    else
    {
            // switch to ACES AP1 color space (results in a more consistent curve)
            lFragCol.rgb = MUL( lFragCol.rgb, P3D65_TO_AP1 );
            lFragCol.xyz *= 10.0;

            #if 0
            vec3 lFragColLut = SimpleReinhard2( lFragCol.rgb ); 
            lFragColLut = ApplyColourLUT( DEREF_PTR( lUniforms.mpPerFrame ), DEREF_PTR( lUniforms.mpCustomPerMesh ), lFragColLut, lfDepth, lfLUTEffectAmount );
            lFragColLut = SimpleUnReinhard2( lFragColLut );

            #ifdef D_PLATFORM_PC
            lFragCol.rgb = mix( lFragCol.rgb, lFragColLut, lUniforms.mpCustomPerMesh.gHDRParams2Vec4.y );
            #else
            lFragCol.rgb = mix( lFragCol.rgb, lFragColLut, 0.8 );
            #endif
            #endif

            lFragCol.rgb *= 3.0 / 4.0;

            if( false ) // if( newCoords.y > 0.5 )
            {
                float lfLum = max( 0.0, MUL( lFragCol.rgb, P3D65_TO_XYZ ).y );

                lfLum /= 0.18; // middle gray == 1
                float lfZone = floor( log2(lfLum) + 0.5 ); // middle gray = 0.0

                if( newCoords.y > 0.9 )
                {
                    lfZone = floor( (newCoords.x * 11.0) - 5.0 );
                }

                lfLum = pow( 2.0, lfZone ) * 0.18;

                vec3 lFragXYZ = D65_White_xyY;
                lFragXYZ.z = lfLum;
                lFragCol.rgb = max( float2vec3(0.0), MUL( xyY_TO_XYZ(lFragXYZ), XYZ_TO_P3D65 ) );
            }
        

            // ACES output curve
            lFragCol.rgb = ssts_f3( lFragCol.rgb, DEREF_PTR(lUniforms.mpCustomPerMesh) );
            lFragCol.rgb = MUL( lFragCol.rgb, AP1_TO_sRGB );
            
            lFragCol.rgb *= 0.01; // 1.0 == 100 nit 

            lFragCol.xyz *= 0.25;   // invert the HDR exposure
            
            #ifdef D_PLATFORM_PC
            lFragCol.rgb *= 4.0 / 3.0;    //lFragCol.xyz = max( float2vec3(0.0), MUL(lFragCol.xyz, BT709_TO_BT2020) );
            #else
            lFragCol.rgb *= 4.0 /  lUniforms.mpCustomPerMesh.gHDRParams2Vec4.y;
            #endif

            lFragCol.xyz = GammaCorrectOutput(lFragCol.xyz); // and apply the nonlinear scRGB encoding to match the sRGB material

    }

    FRAGMENT_COLOUR = vec4(lFragCol, 1.0);
    FRAGMENT_DEPTH  = LinearNormToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth );
}

#endif

// =================================================================================================
//
// FSR2 CUSTOM INPUTS
//
// =================================================================================================

#ifdef D_POSTPROCESS_FSR2_CUSTOM_INPUTS

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions
float
RGBToLuminance(
    vec3 lfLinearRgb )
{
    return dot( lfLinearRgb, vec3( 0.2126f, 0.7152f, 0.0722f ) );
}

float
PerceivedLuminance(
    float lfLuminance )
{
    float lfPerceivedLuminance = 0;

    if ( lfLuminance <= 216.0f / 24389.0f )
    {
        lfPerceivedLuminance = lfLuminance * 24389.0f / 27.0f;
    }
    else
    {
        lfPerceivedLuminance = pow( lfLuminance, 1.0f / 3.0f ) * 116.0f - 16.0f;
    }

    return lfPerceivedLuminance * 0.01f;
}

float
PerceivedLuminanceEncoded(
    float lfLuminance )
{
    return pow( PerceivedLuminance( lfLuminance ), 1.0f / 4.0f );
}

float
EncodeLogLuminance(
    float lfLogLuminance )
{
    return -lfLogLuminance / 3.0;
}

float
EncodePerceivedLuminance(
    float lfPerceivedLuminance )
{
    return pow( lfPerceivedLuminance, 1.0f / 4.0f );
}

vec3
Tonemap(
    vec3 fRgb )
{
    vec3 lvMaxChannel = float2vec3( max( max( 0.f, fRgb.r ), max( fRgb.g, fRgb.b ) ) + 1.f );
    return fRgb / lvMaxChannel;
}

FRAGMENT_MAIN_COLOUR01_SRT
{
    vec3  lvFragCol;
    vec2  lvTexCoords;
    float lfLuminance;
    float lfLogLuminance;
    float lfPerceivedLum;
    float lfPerceivedLumEcd;

    lvTexCoords         = TEX_COORDS;
    lvFragCol           = Tonemap( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvTexCoords ).rgb );
    lfLuminance         = RGBToLuminance( lvFragCol );
    lfLogLuminance      = EncodeLogLuminance( log( max( 1e-03, lfLuminance ) ) );
    lfPerceivedLum      = PerceivedLuminance( lfLuminance );
    lfPerceivedLumEcd   = EncodePerceivedLuminance( lfLuminance );

    WRITE_FRAGMENT_COLOUR0( vec4( lfPerceivedLumEcd, 0.0, 0.0, 1.0 ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lfLogLuminance, 0.0, 0.0, 1.0 ) );
}

#endif


// =================================================================================================
//
// ADDITION
//
// =================================================================================================


#ifdef D_POSTPROCESS_ADDITION_4

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 


FRAGMENT_MAIN_COLOUR_SRT
{   
    vec3 lFragCol;
    vec2 newCoords = IN(mTexCoordsVec2).xy;

    // Combine
    lFragCol  =                  ( texture2D(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer1Map ), newCoords).xyz );
    lFragCol += GammaCorrectInput( texture2D(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer2Map ), newCoords).xyz );
    lFragCol += GammaCorrectInput( texture2D(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer3Map ), newCoords).xyz );
    lFragCol +=                  ( texture2D(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer4Map ), newCoords).xyz );
    lFragCol  = GammaCorrectOutput( lFragCol );

    FRAGMENT_COLOUR = vec4( lFragCol, 1.0 );    
}

#endif

// =================================================================================================
//
// ADDITION_2
//
// =================================================================================================

#ifdef D_POSTPROCESS_ADDITION_2

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{

    //Debug

    vec3 lFragCol;
    vec2 newCoords;

    newCoords = IN( mTexCoordsVec2 ).xy;

    lFragCol  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), newCoords ).xyz;
    lFragCol += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), newCoords ).xyz;

    FRAGMENT_COLOUR = vec4( lFragCol, 1.0 );


}

#endif

// =================================================================================================
//
// MOTIONBLUR
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTIONBLUR

//-----------------------------------------------------------------------------
//      Global Data

#if defined( D_MOTIONBLUR_SAMPLE_COUNT_ULTRA )

#define D_MOTIONBLUR_SAMPLES 24

#elif defined( D_MOTIONBLUR_SAMPLE_COUNT_HIGH ) 

#define D_MOTIONBLUR_SAMPLES 12

#elif defined( D_MOTIONBLUR_SAMPLE_COUNT_MED ) 

#define D_MOTIONBLUR_SAMPLES 8

#else

#define D_MOTIONBLUR_SAMPLES 4

#endif

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT      
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


#ifdef D_PLATFORM_ORBIS
// doesn't seem to result in any spills if we push to max occupancy
#pragma argument(targetoccupancy_atallcosts=100)
#endif


//-----------------------------------------------------------------------------
//      Functions 


#define SOFT_Z_EXTENT 0.1

float cone( in float lSampleDist, in float lfRcpSpeed )
{
    return saturate( 1.0 - lSampleDist * lfRcpSpeed );
}

float smoothstep_approx( float inval )
{
    // replaces the cylinder approximation
    // want a function that looks a bit like

    // 1       /----\    
    //        |      |
    //        |      |
    //        |      |      
    // 0 ____/        \______

    // centered around 0

    // first apply an inverse parabola - this parabola is 1 at +/-0.95
    // and 0 at +/-1.05
    float x = saturate( 5.5125 - 5 * inval * inval );

    // then apply smoothstep
    float x2 = x*x;
    return ( 3.0 * x2 - 2.0 * (x * x2) );
}

float cylinder( in float lSampleDist, in float lfRcpSpeed )
{
    // return 1.0 - smoothstep( 0.95 * lfSpeed, 1.05 * lfSpeed, lSampleDist );

    return smoothstep_approx( lSampleDist * lfRcpSpeed ); 
}

float softDepthCompare( in float zA, in float zB )
{
    return saturate( 1.0 - ( zA - zB ) * (1.0 / SOFT_Z_EXTENT) );
}

float hardDepthCompare( in float zA, in float zB )
{
    return zA < zB? 1.0 : 0.0;
}


vec3 SimpleReinhard( in vec3 col )
{
    vec3 lHalfCol = col / ( vec3(1.0, 1.0, 1.0) + max( vec3(0.0, 0.0, 0.0), col ) );
    return lHalfCol * 2.0;
}

vec3 SimpleUnReinhard( in vec3 col )
{
    vec3 lHalfCol = col * 0.5;
    return lHalfCol / ( vec3(1.0, 1.0, 1.0) - min( vec3(0.9999, 0.9999, 0.9999), lHalfCol ) );
}

vec3 SimpleHalfReinhard( in vec3 col )
{
    #ifdef D_MBLUR_REINHARD_APPROX
    vec3 lHalfCol = col / ( vec3(1.0, 1.0, 1.0) + col );
    return lHalfCol / 0.75;
    #else
    return col;
    #endif
}

vec3 SimpleUnHalfReinhard( in vec3 col )
{
    #ifdef D_MBLUR_REINHARD_APPROX
    vec3 lHalfCol = col * 0.75;
    return lHalfCol / ( vec3(1.0) - lHalfCol );
    #else
    return col;
    #endif
}

float BayerCustom(
    uvec2 lPos )
{    
    float rndoffset = BayerFract( lPos );

    #ifdef D_PASS_2
    rndoffset = 1.0 - rndoffset;
    #endif

    return rndoffset;
}

FRAGMENT_MAIN_COLOUR_SRT
{           
    vec2 tc = TEX_COORDS;

    float lfBaseSampleWeight = 1.0;

    float wacc = 0.0;
    vec4  acc  = vec4(0.0, 0.0, 0.0, 0.0);

    vec4 lTileMinMax = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), tc, 0.0);
    lTileMinMax.xy = DecodeMotion(lTileMinMax.xy) * lUniforms.mpPerFrame.gMBlurSettingsVec4.z;  
    lTileMinMax.zw = DecodeMotion(lTileMinMax.zw) * lUniforms.mpPerFrame.gMBlurSettingsVec4.z;  
    
    vec2  lDelta = lTileMinMax.xy;

    vec2 lDeltaInPixels = lDelta * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    float lDistInPixels = length( lDeltaInPixels );
    float lWantedDistInPixels = max( 0.0, lDistInPixels - abs( lUniforms.mpPerFrame.gTaaSettingsVec4.x ) );


    float lNumSamples = min( D_MOTIONBLUR_SAMPLES, ( lWantedDistInPixels * 1.5 ) );

    #if defined( D_MBLUR_DISCARD_IF_DOING_NOTHING ) && !defined D_COMPUTE
    if( lNumSamples < 0.1 )
    {
        discard;
    }
    #endif

    #ifdef D_MBLUR_DOWNRES_MODE
    vec4 lBaseSample = vec4( 0.0, 0.0, 0.0, 0.0 );    
    #else
    vec4 lBaseSample = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), tc, 0.0 );
    #endif
    vec4 lOutColor = lBaseSample;

    if( lNumSamples >= 2.0 )
    {
        lBaseSample.xyz = SimpleUnHalfReinhard( lBaseSample.xyz );

        #ifndef D_MOTIONBLUR_SAMPLE_COUNT_ULTRA
        lNumSamples = D_MOTIONBLUR_SAMPLES;
        #endif

        vec2 lMaxMinDifference = ( lTileMinMax.xy - lTileMinMax.zw ) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;


        #ifdef D_MOTIONBLUR_JITTER

        #ifndef D_PLATFORM_GLSL

            float rndoffset = BayerCustom( uvec2( tc * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) ) - 0.5;

        #else

            //float rndoffset = 0.0;
            #ifdef D_PASS_2
            float rndoffset = fract( sin( dot( tc.xy, vec2( 12.9898, 78.233 ) ) )* 43758.5453 ) - 0.5;
            #else
            float rndoffset = fract( sin( dot( tc.yx, vec2( 12.9898, 78.233 ) ) )* 43758.5453 ) - 0.5;
            #endif

        #endif

        #else

        float rndoffset = 0.0;

        #endif

        rndoffset *= 2.0;
        



        #ifndef D_MOTIONBLUR_ALWAYS_MULTIDIR

        bool lNeedsComplexTest = dot( lMaxMinDifference, lMaxMinDifference ) > 25.0f;

        #if defined( D_PLATFORM_PROSPERO ) && !defined D_COMPUTE_DISABLEWAVE32
        lNeedsComplexTest = (wave32::ballot(lNeedsComplexTest) != 0);
        #elif defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_XBOXONE )
        lNeedsComplexTest = ( ballot(lNeedsComplexTest) != 0 ); 
        #endif

        if( !lNeedsComplexTest )
        {
            // tile velocity is all identical, fall back on simple blur
            float fRecipSamples = 1.0 / ( lNumSamples - 1 );
            vec2  cVelocity = lDelta * (lWantedDistInPixels / lDistInPixels);
            float cSpeed = length( cVelocity );
            float lfRcpSpeed = 1.0 / ( length( cVelocity ) );

            vec2  lSampleStep   = cVelocity * fRecipSamples;
            vec2  lSampleCenter = tc - rndoffset * lSampleStep;
            vec2  lSampleStart  = lSampleCenter - (lNumSamples - 1) * 0.5 * lSampleStep;

            float lSampleStepLength = fRecipSamples * cSpeed;
            float lSampleStartDist = - ( lNumSamples * 0.5 ) * lSampleStepLength;

            for( float i = 0; i < lNumSamples; i++ )
            {
                vec2 lSamplePos = lSampleStart;
                lSampleStart += lSampleStep;

                float lSampleDist = abs( lSampleStartDist );
                lSampleStartDist += lSampleStepLength;

                //float weight = cylinder( lSampleDist, lfRcpSpeed );
                float weight = cylinder( i - ( lNumSamples * 0.5 ), 1.0 / ( lNumSamples ) );

                #ifdef D_PASS_2
                lSamplePos = tc + (lSamplePos - tc) * fRecipSamples * 2.0;
                #endif

                vec4 lNewSample = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lSamplePos, 0.0 );
                lNewSample.xyz = SimpleUnHalfReinhard( lNewSample.xyz );
                acc  += lNewSample * weight;  
                wacc += weight;
            }
        }
        else
        #endif

        {
            float lfBaseDepth;
            float lfBaseDepthNormalised;
            float lfBaseRcpSpeed;
            vec2 lBaseDelta;
            vec2 lEncodedBaseDelta;
            bool lWantsColorClipAA;
            vec2 reproj_tc = GetPrevPosition(tc, 
                                             lUniforms.mpPerFrame.gClipPlanesVec4,
                                             lUniforms.mpPerFrame.gInverseProjectionMat4,
                                             lUniforms.mpPerFrame.gInverseViewMat4,
                                             lUniforms.mpPerFrame.gInverseViewProjectionMat4,
                                             lUniforms.mpPerFrame.gViewProjectionMat4,
                                             lUniforms.mpPerFrame.gPrevViewProjectionMat4,
                                             lUniforms.mpPerFrame.gViewPositionVec3,
                                             lUniforms.mpPerFrame.gMBlurSettingsVec4,
                                             lUniforms.mpPerFrame.gFoVValuesVec4,
                                             lUniforms.mpPerFrame.giAntiAliasingIndex,
                                             SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer2Map ), 
                                             SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer3Map ),
                                             lfBaseDepth,
                                             lfBaseDepthNormalised,
                                             lfBaseRcpSpeed,
                                             lBaseDelta,
                                             lEncodedBaseDelta,
                                             lWantsColorClipAA );

            lBaseDelta  *= lUniforms.mpPerFrame.gMBlurSettingsVec4.z;
            vec2 lBasePerp = lBaseDelta - lDelta * dot( lBaseDelta,lDelta ) / dot( lDelta,lDelta );
            vec2 lBaseNorm = lBaseDelta * lfBaseRcpSpeed;

            vec2 lDeltaNorm;
            vec2 lBasePerpInPixels = lBasePerp * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
            if( BayerCustom( uvec2( tc * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy )  ) < length(lBasePerpInPixels) / 30.0 )
            {
                lDelta = lBaseDelta;
                lDeltaNorm = lBaseNorm;
            }  
            else
            {
                lDeltaNorm = normalize( lDelta );
            }

            vec2  cVelocity = lDelta * (lWantedDistInPixels / lDistInPixels);
            float cSpeed = length( cVelocity );

            float fRecipSamples = 1.0 / ( lNumSamples - 1 );
            vec2  lSampleStep   = cVelocity * fRecipSamples;
            vec2  lSampleCenter = tc - rndoffset * lSampleStep;
            vec2  lSampleStart  = lSampleCenter - (lNumSamples - 1) * 0.5 * lSampleStep;

            float lSampleStepLength = fRecipSamples * cSpeed;
            float lSampleStartDist = - ( lNumSamples * 0.5 ) * lSampleStepLength;


            for( float i = 0; i < lNumSamples; i++ )
            {
                vec2 lSamplePos =  lSampleStart + lSampleStep * i;
                float lSampleDist = abs( lSampleStartDist + lSampleStepLength * i );
                vec2 lSampleOffset = lSamplePos - lSampleCenter;

                float lfSampleDepth;
                float lfSampleDepthNormalised;
                float lfSampleRcpSpeed;
                vec2 lSampleDelta;
                vec2 lEncodedSampleDelta;
                vec2 reproj_sample = GetPrevPosition(lSamplePos, 
                                             lUniforms.mpPerFrame.gClipPlanesVec4,
                                             lUniforms.mpPerFrame.gInverseProjectionMat4,
                                             lUniforms.mpPerFrame.gInverseViewMat4,
                                             lUniforms.mpPerFrame.gInverseViewProjectionMat4,
                                             lUniforms.mpPerFrame.gViewProjectionMat4,
                                             lUniforms.mpPerFrame.gPrevViewProjectionMat4,
                                             lUniforms.mpPerFrame.gViewPositionVec3,
                                             lUniforms.mpPerFrame.gMBlurSettingsVec4,
                                             lUniforms.mpPerFrame.gFoVValuesVec4,
                                             lUniforms.mpPerFrame.giAntiAliasingIndex,
                                             SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer2Map ), 
                                             SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer3Map ),
                                             lfSampleDepth,
                                             lfSampleDepthNormalised,
                                             lfSampleRcpSpeed,
                                             lSampleDelta,
                                             lEncodedSampleDelta,
                                             lWantsColorClipAA );


                lSampleDelta  *= lUniforms.mpPerFrame.gMBlurSettingsVec4.z;
                vec2 lSampleDeltaNorm = lSampleDelta * lfSampleRcpSpeed;

                float lfBaseCylWeight   = cylinder( lSampleDist, lfBaseRcpSpeed );
                float lfSampleCylWeight = cylinder( lSampleDist, lfSampleRcpSpeed );

                /*

                // possible x/y slope version of the cone check, might well be usable

                float ySampleAmt = abs( dot( lDeltaNorm, lEncodedSampleDelta ) );
                float xSampleAmt = abs( dot( vec2( -lDeltaNorm.y, lDeltaNorm.x ), lEncodedSampleDelta ) );

                float yBaseAmt = abs( dot( lDeltaNorm, lBaseDelta ) );
                float xBaseAmt = abs( dot( vec2( -lDeltaNorm.y, lDeltaNorm.x ), lBaseDelta ) );

                float lFrontVelWeight = (xSampleAmt < ySampleAmt)? 1.0 : 0.0;
                float lBackVelWeight  = (xBaseAmt < yBaseAmt)? 1.0 : 0.0;

                */


                float lFrontVelWeight = abs( dot( lDeltaNorm, lSampleDeltaNorm ) );
                float lBackVelWeight  = abs( dot( lDeltaNorm, lBaseNorm ) );

                lFrontVelWeight *= lFrontVelWeight;
                lBackVelWeight  *= lBackVelWeight;
                lFrontVelWeight *= lFrontVelWeight;
                lBackVelWeight  *= lBackVelWeight;

                float lfFrontSampleWeight = lfSampleCylWeight * lFrontVelWeight;
                float lfBackSampleWeight  = lfBaseCylWeight   * lBackVelWeight;

                float weight = ( lfBaseDepth < lfSampleDepth? lfBackSampleWeight : lfFrontSampleWeight );
                weight = saturate( weight );

                // need to work out if the base sample *would* be blended into the far sample
                // (when the far sample does its gather)
                // which we can compute easily by swapping around some of these numbers

                float weightMirror = ( lfBaseDepth < lfSampleDepth? lfFrontSampleWeight : lfBackSampleWeight );
                weightMirror = 1.0 - saturate( weightMirror );

                // if the base sample is going to be blended into a far sample,
                // duck it down by an appropriate amount so we don't gain energy
                lfBaseSampleWeight += weightMirror;

                float fRecipSamplesMultiplier = (weight > 0.0) ? fRecipSamples : 0.0;
                #ifdef D_PASS_2
                lSamplePos = tc + (lSamplePos - tc) * fRecipSamplesMultiplier * 2.0;
                #endif

                vec4 lNewSample = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lSamplePos, 0.0 );

                lNewSample.xyz = SimpleUnHalfReinhard( lNewSample.xyz );
                acc  += lNewSample * weight;  
                wacc += weight;

            }
        }

        //FRAGMENT_COLOUR = vec4( reproj_tc, 0.0, 1.0 );    
        //FRAGMENT_COLOUR = vec4( texture2D( lUniforms.mpCustomPerMesh.gBufferMap, IN(mTexCoordsVec2) ).xyz, 1.0 );    
        //FRAGMENT_COLOUR = vec4( GammaCorrectOutput( acc.xyz / wacc ), 1.0 );
        //FRAGMENT_COLOUR = vec4(  (lBaseDelta*100)+0.5, 0.0, 1.0 );    

        lOutColor  = lBaseSample * max( 0.0, lfBaseSampleWeight - wacc ) + acc;
        lOutColor /= max( lfBaseSampleWeight, wacc );

        lOutColor.xyz = SimpleHalfReinhard( lOutColor.xyz );
    }


    #ifdef D_PASS_2  
    //FRAGMENT_COLOUR = vec4( lOutColor, 1.0 );
    WRITE_FRAGMENT_COLOUR( vec4( SimpleUnReinhard( lOutColor.xyz ), lOutColor.a ) );

    #else
    // tonemap in pass 1
    //FRAGMENT_COLOUR = vec4( GammaCorrectOutput( TonemapKodak( lOutColor ) / TonemapKodak( vec3( 1.0,1.0,1.0 ) ) ), 1.0 );
    WRITE_FRAGMENT_COLOUR( vec4( SimpleReinhard( lOutColor.xyz ), lOutColor.a ) );

    #endif

}
#endif

// =================================================================================================
//
// MOTION VECTOR DILATE
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTIONDILATE_PASS1

//-----------------------------------------------------------------------------
//      Global Data

#define D_NEIGHBORHOOD_SIZE 16.0
#define D_NEIGHBORHOOD_OVER 0.5
#define D_BUFFER_SCALING 0.125

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#ifdef D_PLATFORM_ORBIS
#pragma argument(targetoccupancy_atallcosts=70)
#pragma argument(nounrollallloops)
#endif

//-----------------------------------------------------------------------------
//      Functions 

vec2
GetPixelEncodedMotion(
    SAMPLER2DARG( lMotionMap ),
    in vec2 lTexCoords )
{
    vec2 lDelta = Texture2DNoFiltering( SAMPLER2DPARAM( lMotionMap ), lTexCoords ).xy;
#ifdef D_PLATFORM_METAL
    lDelta = clamp(lDelta, -1.0f, 1.0f);
#endif
    return lDelta * 0.5;
}

void
MinMaxNewSpeed(
    SAMPLER2DARG( lMotionMap ),
    in vec2 lTexCoords, 
    inout float lMinSpeed,
    inout float lMaxSpeed,
    inout vec2 lMinMotion,
    inout vec2 lMaxMotion )
{
    vec2 lEncodedMotion = GetPixelEncodedMotion( SAMPLER2DPARAM( lMotionMap ), lTexCoords ) * 2.0;
  
    vec2  lCenteredMotion = lEncodedMotion   - vec2( 0.5, 0.5 );
    float lSpeedSquared   = dot( lCenteredMotion, lCenteredMotion );

    if( lSpeedSquared > lMaxSpeed )
    {
        lMaxSpeed  = lSpeedSquared;
        lMaxMotion = lEncodedMotion;
    } 

    if( lSpeedSquared < lMinSpeed )
    {
        lMinSpeed  = lSpeedSquared;
        lMinMotion = lEncodedMotion;
    } 
}

FRAGMENT_MAIN_COLOUR_SRT
{           

    #if defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_XBOXONE )
    //#if 0

    #if 1

    vec2 lSample = TEX_COORDS;

    // read the texture on the current pixel, but also the quad above and below
    vec2 lMinMotion   = GetPixelEncodedMotion( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ), lSample ) * 2.0;
    vec2  lCenteredMotion   = lMinMotion   - vec2( 0.5,0.5 );
    float lMinSpeed   = dot( lCenteredMotion,   lCenteredMotion );
    float lMaxSpeed = lMinSpeed;
    vec2 lMaxMotion = lMinMotion;

    vec2 lSampleLt = lSample - vec2( lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 8.0, 0.0 );
    vec2 lSampleRt = lSample + vec2( lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 8.0, 0.0 );

    MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSampleLt, lMinSpeed, lMaxSpeed, lMinMotion, lMaxMotion );
    MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSampleRt, lMinSpeed, lMaxSpeed, lMinMotion, lMaxMotion );

        for( float xx = 0.0; xx < 1.0; xx += D_NEIGHBORHOOD_OVER )
        {
            vec2 lOffset = vec2( xx, 0.0 ) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
            
            MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSample   + lOffset, lMinSpeed, lMaxSpeed, lMinMotion, lMaxMotion );
            MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSampleLt + lOffset, lMinSpeed, lMaxSpeed, lMinMotion, lMaxMotion );
            MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSampleRt + lOffset, lMinSpeed, lMaxSpeed, lMinMotion, lMaxMotion );
        }

    // swap low bit of X
    float lAlt0MaxSpeed = LaneSwizzle( lMaxSpeed,   0x1f, 0, 1 );

    float lAlt0MaxMotionX  = LaneSwizzle( lMaxMotion.x, 0x1f, 0, 1 );
    float lAlt0MaxMotionY  = LaneSwizzle( lMaxMotion.y, 0x1f, 0, 1 );

    float lAlt0MinSpeed = LaneSwizzle( lMinSpeed,   0x1f, 0, 1 );

    float lAlt0MinMotionX  = LaneSwizzle( lMinMotion.x, 0x1f, 0, 1 );
    float lAlt0MinMotionY  = LaneSwizzle( lMinMotion.y, 0x1f, 0, 1 );

    if( lAlt0MaxSpeed > lMaxSpeed )
    {
        lMaxSpeed  = lAlt0MaxSpeed;
        lMaxMotion.x = lAlt0MaxMotionX;
        lMaxMotion.y = lAlt0MaxMotionY;
    }

    if( lAlt0MinSpeed < lMinSpeed )
    {
        lMinSpeed  = lAlt0MinSpeed;
        lMinMotion.x = lAlt0MinMotionX;
        lMinMotion.y = lAlt0MinMotionY;
    } 

    // swap bit 1 of X
    float lAlt1MaxSpeed = LaneSwizzle( lMaxSpeed,   0x1f, 0, 4 );

    float lAlt1MaxMotionX  = LaneSwizzle( lMaxMotion.x, 0x1f, 0, 4 );
    float lAlt1MaxMotionY  = LaneSwizzle( lMaxMotion.y, 0x1f, 0, 4 );

    float lAlt1MinSpeed = LaneSwizzle( lMinSpeed,   0x1f, 0, 4 );

    float lAlt1MinMotionX  = LaneSwizzle( lMinMotion.x, 0x1f, 0, 4 );
    float lAlt1MinMotionY  = LaneSwizzle( lMinMotion.y, 0x1f, 0, 4 );

    if( lAlt1MaxSpeed > lMaxSpeed )
    {
        lMaxSpeed  = lAlt1MaxSpeed;
        lMaxMotion.x = lAlt1MaxMotionX;
        lMaxMotion.y = lAlt1MaxMotionY;
    }

    if( lAlt1MinSpeed < lMinSpeed )
    {
        lMinSpeed  = lAlt1MinSpeed;
        lMinMotion.x = lAlt1MinMotionX;
        lMinMotion.y = lAlt1MinMotionY;
    } 

    // swap bit 2 of X
    float lAlt2MaxSpeed = LaneSwizzle( lMaxSpeed,   0x1f, 0, 16 );

    float lAlt2MaxMotionX  = LaneSwizzle( lMaxMotion.x, 0x1f, 0, 16 );
    float lAlt2MaxMotionY  = LaneSwizzle( lMaxMotion.y, 0x1f, 0, 16 );

    float lAlt2MinSpeed = LaneSwizzle( lMinSpeed,   0x1f, 0, 16 );

    float lAlt2MinMotionX  = LaneSwizzle( lMinMotion.x, 0x1f, 0, 16 );
    float lAlt2MinMotionY  = LaneSwizzle( lMinMotion.y, 0x1f, 0, 16 );

    if( lAlt2MaxSpeed > lMaxSpeed )
    {
        lMaxSpeed  = lAlt2MaxSpeed;
        lMaxMotion.x = lAlt2MaxMotionX;
        lMaxMotion.y = lAlt2MaxMotionY;
    }

    if( lAlt2MinSpeed < lMinSpeed )
    {
        lMinSpeed  = lAlt2MinSpeed;
        lMinMotion.x = lAlt2MinMotionX;
        lMinMotion.y = lAlt2MinMotionY;
    } 

    #else

    vec2 lSample = IN(mTexCoordsVec2);

    uint liLaneIndex = __v_mbcnt_lo_u32_b32(uint(-1), __v_mbcnt_hi_u32_b32(uint(-1), 0u));

    uint liLaneX = liLaneIndex & 0x55;          
         liLaneX = (liLaneX ^ (liLaneX >>  1)) & 0x33;
         liLaneX = (liLaneX ^ (liLaneX >>  2)) & 0x0f;
    uint liLaneY = (liLaneIndex >> 1) & 0x55;    
         liLaneY = (liLaneY ^ (liLaneY >>  1)) & 0x33;
         liLaneY = (liLaneY ^ (liLaneY >>  2)) & 0x0f;

    // read the texture on the current pixel, but also the quad above and below
    vec2 lEncodedMotion   = GetPixelEncodedMotion( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSample ) * 2.0;
    vec2 lEncodedMotionLt = GetPixelEncodedMotion( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSample - vec2( lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 8.0, 0.0 ) ) * 2.0;
    vec2 lEncodedMotionRt = GetPixelEncodedMotion( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSample + vec2( lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 8.0, 0.0 ) ) * 2.0;

    vec2  lCenteredMotion   = lEncodedMotion   - vec2( 0.5 );
    vec2  lCenteredMotionLt = lEncodedMotionLt - vec2( 0.5 );
    vec2  lCenteredMotionRt = lEncodedMotionRt - vec2( 0.5 );

    float lSpeedSquared   = dot( lCenteredMotion,   lCenteredMotion );
    float lSpeedSquaredLt = dot( lCenteredMotionLt, lCenteredMotionLt );
    float lSpeedSquaredRt = dot( lCenteredMotionRt, lCenteredMotionRt );

    for( uint xx=0; xx<8; ++xx )
    {
        // re-encode the loop X to morton tiling order
        uint xCode = (xx    ^ (xx    << 2)) & 0x33;
             xCode = (xCode ^ (xCode << 1)) & 0x55;

        // here is how we would get the lane with current Y but desired X
        // uint altLane = ( liLaneIndex & 0xAA ) | xCode;

        // the shuffle instruction allows us to do this though
        float lCurSpeedSquared   = LaneSwizzle( lSpeedSquared,   0xa, xCode, 0 );
        float lCurSpeedSquaredLt = LaneSwizzle( lSpeedSquaredLt, 0xa, xCode, 0 );
        float lCurSpeedSquaredRt = LaneSwizzle( lSpeedSquaredRt, 0xa, xCode, 0 );

        // important: do these swizzles while the execution mask is full
        // otherwise some threads may unexpectedly get back 0s
        float lCurMotionX = LaneSwizzle( lEncodedMotion.x, 0xa, xCode, 0 );
        float lCurMotionY = LaneSwizzle( lEncodedMotion.y, 0xa, xCode, 0 );

        float lCurMotionLtX = LaneSwizzle( lEncodedMotionLt.x, 0xa, xCode, 0 );
        float lCurMotionLtY = LaneSwizzle( lEncodedMotionLt.y, 0xa, xCode, 0 );

        float lCurMotionRtX = LaneSwizzle( lEncodedMotionRt.x, 0xa, xCode, 0 );
        float lCurMotionRtY = LaneSwizzle( lEncodedMotionRt.y, 0xa, xCode, 0 );

        if( lCurSpeedSquared > lMaxSpeed )
        {
            lMaxSpeed  = lCurSpeedSquared;
            lMaxMotion.x = lCurMotionX;
            lMaxMotion.y = lCurMotionY;
        } 

        if( lCurSpeedSquared < lMinSpeed )
        {
            lMinSpeed  = lCurSpeedSquared;
            lMinMotion.x = lCurMotionX;
            lMinMotion.y = lCurMotionY;
        } 

        if( xx >= liLaneX )
        {
            if( lCurSpeedSquaredLt > lMaxSpeed )
            {
                lMaxSpeed  = lCurSpeedSquaredLt;
                lMaxMotion.x = lCurMotionLtX;
                lMaxMotion.y = lCurMotionLtY;
            } 

            if( lCurSpeedSquaredLt < lMinSpeed )
            {
                lMinSpeed  = lCurSpeedSquaredLt;
                lMinMotion.x = lCurMotionLtX;
                lMinMotion.y = lCurMotionLtY;
            } 
        }

        if( xx <= liLaneX )
        {
            if( lCurSpeedSquaredRt > lMaxSpeed )
            {
                lMaxSpeed  = lCurSpeedSquaredRt;
                lMaxMotion.x = lCurMotionRtX;
                lMaxMotion.y = lCurMotionRtY;
            } 

            if( lCurSpeedSquaredRt < lMinSpeed )
            {
                lMinSpeed  = lCurSpeedSquaredRt;
                lMinMotion.x = lCurMotionRtX;
                lMinMotion.y = lCurMotionRtY;
            } 
        }
    }

    #endif

    #else

    #if defined(D_PLATFORM_METAL)
    const vec2 centerOffset = vec2( 0, 0 );
    #else
    const vec2 centerOffset = vec2( 0.5, 0.5 );
    #endif

    float lMaxSpeed = 0.0;
    vec2 lMaxMotion = centerOffset;

    float lMinSpeed = 10000.0;
    vec2 lMinMotion = centerOffset;
    #ifdef D_PLATFORM_METAL
    lMaxSpeed = -lMinSpeed;
    #endif

    vec2 lPixelSize = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * D_NEIGHBORHOOD_OVER;

    vec2 lStartUpperCorner = vec2( D_NEIGHBORHOOD_SIZE, D_NEIGHBORHOOD_SIZE * D_BUFFER_SCALING );

    // first pixel loc, the upper left 
    vec2 lStartSample = (TEX_COORDS) - lStartUpperCorner * lPixelSize * 0.5 + lPixelSize * 0.5;
    vec2 lSample = lStartSample;

    #ifndef D_DILATE_HORZ
    for( float y = 0; y < D_NEIGHBORHOOD_SIZE; y++ )
    #endif
    {

        #ifndef D_DILATE_VERT
        for( float x = 0; x < D_NEIGHBORHOOD_SIZE; x++ )
        #endif
        {
            // compute the previous position of this pixel
            vec2 lEncodedMotion = GetPixelEncodedMotion( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSample ) * 2.0;

            vec2  lCenteredMotion = lEncodedMotion - centerOffset;
            float lSpeedSquared = dot( lCenteredMotion, lCenteredMotion );

            if( lSpeedSquared > lMaxSpeed )
            {
                lMaxSpeed  = lSpeedSquared;
                lMaxMotion = lEncodedMotion;
            } 

            if( lSpeedSquared < lMinSpeed )
            {
                lMinSpeed  = lSpeedSquared;
                lMinMotion = lEncodedMotion;
            } 

            lSample.x += lPixelSize.x;
        }

        lSample.x = lStartSample.x;
        lSample.y += lPixelSize.y;
    }


    #endif


    WRITE_FRAGMENT_COLOUR( vec4( lMaxMotion, lMinMotion ) );    
}

#endif

// =================================================================================================
//
// MOTION VECTOR DILATE
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTIONDILATE_PASS2

//-----------------------------------------------------------------------------
//      Global Data

#define D_NEIGHBORHOOD_SIZE 16.0
#define D_NEIGHBORHOOD_OVER 0.5
#define D_BUFFER_SCALING 0.125

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#ifdef D_PLATFORM_ORBIS
#pragma argument(targetoccupancy_atallcosts=70)
#pragma argument(nounrollallloops)
#endif

vec2 ClipVector(
    in vec2 lMotion,
    in vec2 lRcpMaxVectorLength )
{
    vec2 lMotionNorm = abs( max( vec2( 1.0, 1.0 ), lMotion * lRcpMaxVectorLength ) );
    return lMotion / lMotionNorm;
}

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{         
    vec2 lPixelSize = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * D_NEIGHBORHOOD_OVER;

    vec2 lStartUpperCorner = vec2( D_NEIGHBORHOOD_SIZE * D_BUFFER_SCALING, D_NEIGHBORHOOD_SIZE );

    // first pixel loc, the upper left 
    vec2 lStartSample = TEX_COORDS - lStartUpperCorner * lPixelSize * 0.5 + lPixelSize * 0.5;
    vec2 lSample = lStartSample;

    float lMaxSpeed = 0.0;
    vec2 lMaxMotion = vec2( 0.5, 0.5 );

    float lMinSpeed = 10000.0;
    vec2 lMinMotion = vec2( 0.5, 0.5 );

    
    #if defined(D_PLATFORM_METAL)
    lMaxSpeed = -lMinSpeed;
    #endif

    #ifndef D_DILATE_HORZ
    for( float y = 0; y < D_NEIGHBORHOOD_SIZE; y++ )
    #endif
    {

        #ifndef D_DILATE_VERT
        for( float x = 0; x < D_NEIGHBORHOOD_SIZE; x++ )
        #endif
        {
            vec4 lMotionBuffer = Texture2DNoFiltering( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), lSample );

            vec2 lSmpMaxMotion = lMotionBuffer.xy;
            vec2 lSmpMinMotion = lMotionBuffer.zw;

            #if defined(D_PLATFORM_METAL)
            const vec2 centerOffset = vec2( 0, 0 );
            #else
            const vec2 centerOffset = vec2( 0.5, 0.5 );
            #endif
            float lMaxSpeedSquared = dot( lSmpMaxMotion - centerOffset, lSmpMaxMotion - centerOffset );
            float lMinSpeedSquared = dot( lSmpMinMotion - centerOffset, lSmpMinMotion - centerOffset );

            if( lMaxSpeedSquared > lMaxSpeed )
            {
                lMaxSpeed  = lMaxSpeedSquared;
                lMaxMotion = lSmpMaxMotion;
            } 

            if( lMinSpeedSquared < lMinSpeed )
            {
                lMinSpeed  = lMinSpeedSquared;
                lMinMotion = lSmpMinMotion;
            } 

            lSample.x += lPixelSize.x;
        }

        lSample.x = lStartSample.x;
        lSample.y += lPixelSize.y;
    }

    // clip the vectors

    vec2 lMaxVectorLength = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy * (D_NEIGHBORHOOD_SIZE / D_BUFFER_SCALING);
    vec2 lRcpMaxVectorLength = vec2( 1.0, 1.0 ) / lMaxVectorLength;

    vec2 lMaxMotionClip = lMaxMotion;// EncodeMotion( ClipVector( DecodeMotion( lMaxMotion ), lRcpMaxVectorLength ) );
    vec2 lMinMotionClip = lMinMotion;// EncodeMotion( ClipVector( DecodeMotion( lMinMotion ), lRcpMaxVectorLength ) );

    WRITE_FRAGMENT_COLOUR( vec4( lMaxMotionClip, lMinMotionClip ) );    
}

#endif

// =================================================================================================
//
// MOTION CROSS GATHER
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTION_X_GATHER

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

vec2
GetPixelEncodedMotion(
    SAMPLER2DARG( lMotionMap ),
    in vec2 lTexCoords )
{
    vec2 lDelta = Texture2DNoFiltering( SAMPLER2DPARAM( lMotionMap ), lTexCoords ).xy;
    return lDelta;
}

void
MinMaxNewSpeed(
    SAMPLER2DARG(   lMotionMap ),
    in    vec2      lTexCoords, 
    inout float     lMinSpeed,
    inout float     lMaxSpeed,
    inout vec2      lMinMotion,
    inout vec2      lMaxMotion )
{
    vec2 lEncodedMotion = GetPixelEncodedMotion( SAMPLER2DPARAM( lMotionMap ), lTexCoords );
  
    vec2  lCenteredMotion = lEncodedMotion - float2vec2( 0.5 );
    float lSpeedSquared   = dot( lCenteredMotion, lCenteredMotion );

    if( lSpeedSquared > lMaxSpeed )
    {
        lMaxSpeed  = lSpeedSquared;
        lMaxMotion = lEncodedMotion;
    } 

    if( lSpeedSquared < lMinSpeed )
    {
        lMinSpeed  = lSpeedSquared;
        lMinMotion = lEncodedMotion;
    } 
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lTexCoords                = TEX_COORDS;
    float lfMinSpeedDilated         = 1.0e16;
    float lfMaxSpeedDilated         = 0.0;
    vec2  lEncodedDeltaMinDilated   = float2vec2( 0.5 );
    vec2  lEncodedDeltaMaxDilated   = float2vec2( 0.5 );

    MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords,
                    lfMinSpeedDilated,          lfMaxSpeedDilated, 
                    lEncodedDeltaMinDilated,    lEncodedDeltaMaxDilated );

    for( float dy = -1.0; dy <= 1.0; dy += 1.0 )
    {
        for( float dx = -1.0; dx <= 1.0; dx += 1.0 )
        {
            if( dx != 0.0 || dy != 0.0 )
            {
                vec2 lCoordsOffset  = vec2( dx, dy ) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
                vec2 lSubTexCoords  = lTexCoords + lCoordsOffset;

                MinMaxNewSpeed( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ), lSubTexCoords,
                                lfMinSpeedDilated,          lfMaxSpeedDilated, 
                                lEncodedDeltaMinDilated,    lEncodedDeltaMaxDilated );
            }
        }
    }

    WRITE_FRAGMENT_COLOUR( vec4( lEncodedDeltaMaxDilated, lEncodedDeltaMinDilated ) );
}
#endif

// =================================================================================================
//
// MOTION FRONT MOST
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTION_FRONT

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lTexCoords    = TEX_COORDS;    
    vec2  lTexelSize    = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    float lfMaxDepth    = 0.0;
    vec2  lMotionFront  = float2vec2( 0.0 );

    for( float dy = -1.0; dy <= 1.0; dy += 1.0 )
    {
        for( float dx = -1.0; dx <= 1.0; dx += 1.0 )
        {            
            vec2  lCurrTexCoords = lTexCoords + vec2( dx, dy ) * lTexelSize;
            vec2  lMotion        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ),  lCurrTexCoords, 0.0 ).xy;
            float lfDepth        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lCurrTexCoords, 0.0 ).x;

            if ( lfDepth > lfMaxDepth )
            {
                lMotionFront    = lMotion;
                lfMaxDepth      = lfDepth;
            }
        }
    }

    WRITE_FRAGMENT_COLOUR( vec4( lMotionFront, 0.0, 0.0 ) );
}
#endif

// =================================================================================================
//
// MOTION UV TO SCREEN SPACE
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTION_UV2SCREEN

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lTexCoords        = TEX_COORDS;    
    vec2  lTexSize          = GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) );        
    vec2  lEncodedMotion    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords ).xy;
    vec2  lDecodedMotion    = DecodeMotion( lEncodedMotion );
    vec2  lScreenMotion     = lDecodedMotion * lTexSize;

    WRITE_FRAGMENT_COLOUR( vec4( lScreenMotion, 0.0, 0.0 ) );
}
#endif

// =================================================================================================
//
// MOTION VECTOR RESOLVE SIMPLE
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTIONRESOLVE_SIMPLE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_T1_SRT( vec2 )
{
    vec2 lvMotion;
    vec2 lvFragCoords   = TEX_COORDS;

    #if defined( D_MOTION_FROM_CAMERA )
    bool lbResolve  = true;
    #else
    bool lbResolve  = false;
    #endif

    if( !lbResolve )
    {
        lvMotion  = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), lvFragCoords, 0.0).xy;
        lbResolve = lvMotion.x >= 1.0;
    }

    if( lbResolve )
    {
        // no motion from uber; use gbuffer to reconstruct position in ndc and resolve motion

        // compute world position from depth and uv coords
        vec3  lvWorldPos;
        {
            float lfDepthRevZ;
            vec3  lvCamPos;
            mat4  lmInvViewProjSC;

            lvCamPos        = lUniforms.mpPerFrame.gViewPositionVec3;
            lmInvViewProjSC = lUniforms.mpPerFrame.gInverseViewProjectionSCMat4;
            lfDepthRevZ     = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lvFragCoords, 0.0 ) );
            lvWorldPos      = RecreatePositionFromRevZDepthSC( lfDepthRevZ, lvFragCoords, lvCamPos, lmInvViewProjSC );
        }

        // Compute the reprojected screen (NDC) position
        vec2 lvRpjScreenPos;
        {
            vec4  lvTempPos;
            mat4  lmViewProjPrev;

            lmViewProjPrev  = lUniforms.mpPerFrame.gPrevViewProjectionMat4;
            lvTempPos       = MUL( lmViewProjPrev, vec4( lvWorldPos, 1.0 ) );
            lvTempPos.xyz  /= lvTempPos.w;
            lvRpjScreenPos  = lvTempPos.xy;
        }

        // Convert the Frag pos from UV to screen (NDC) space
        vec2  lvScreenPos;
        {
            lvScreenPos = SCREENSPACE_AS_RENDERTARGET_UVS( lvFragCoords ) * 2.0 - 1.0;
        }

        // Pass to EncodeMotion the pos diff in NDC space (as done in Uber)
        lvMotion = EncodeMotion( lvRpjScreenPos - lvScreenPos );
    }

    FRAGMENT_OUTPUT_T0 = lvMotion;
}

#endif

// =================================================================================================
//
// MOTION VECTOR RESOLVE DILATED
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTIONRESOLVE_DILATED

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

STATIC_CONST float kfReconstructedDepthBilinearWeightThreshold = 0.05f;


//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_T2_SRT( vec2, float )
{
    // Dilate Depth
    float lfDepthRevZ;
    vec2  lvFragCoords;
    {
        // Coordinates of the dilated depth texel
        vec2 lvDltCoords;
        vec2 lvTexelSize;

        lfDepthRevZ  = 0.0;
        lvFragCoords = TEX_COORDS;
        lvDltCoords  = TEX_COORDS;
        lvTexelSize  = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;

        for ( int ii = -1; ii <= 1; ++ii )
        {
            for ( int jj = -1; jj <= 1; ++jj )
            {
                float lfDepthTmp;
                vec2  lvCoords;

                lvCoords    = lvFragCoords + vec2( ii, jj ) * lvTexelSize;
                lfDepthTmp  = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lvCoords, 0.0 ) );

                if ( lfDepthTmp > lfDepthRevZ )
                {
                    lfDepthRevZ = lfDepthTmp;
                    lvDltCoords = lvCoords;
                }
            }
        }

        lvFragCoords = lvDltCoords;
    }

    // Fetch Motion using coords of the dilated depth texel
    vec2 lvMotion;
    bool lbResolve;
    {
        lvMotion  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoords, 0.0 ).xy;
        lbResolve = lvMotion.x >= 1.0;
    }

    // Motion vectors not computed in uber; compute them here using the cam transforms, depth and uv coords
    if( lbResolve )
    {
        // Convert the Frag pos from UV to screen (NDC) space
        vec2  lvScreenPos;
        {
            lvScreenPos = SCREENSPACE_AS_RENDERTARGET_UVS( lvFragCoords ) * 2.0 - 1.0;
        }

        // Compute the reprojected screen (NDC) position
        vec2 lvRpjScreenPos;
        {
            vec4  lvTempPos;
            mat4  lmThisToPrev;
            mat4  lmViewTrans;
            mat4  lmViewProjPrev;

            lmThisToPrev    = lUniforms.mpPerFrame.gThisToPrevViewProjectionMat4;
            lvTempPos.xy    = lvScreenPos;
            lvTempPos.z     = lfDepthRevZ;
            lvTempPos.w     = 1.0;
            lvTempPos       = MUL( lmThisToPrev, vec4( lvTempPos ) );
            lvTempPos.xyz  /= lvTempPos.w;
            lvRpjScreenPos  = lvTempPos.xy;
        }

        // Pass to EncodeMotion the pos diff in NDC space (as done in Uber)
        lvMotion = EncodeMotion( lvRpjScreenPos - lvScreenPos );
    }

    #if defined(D_DEPTH_REPRJ_BKWD_RW)
    {
        vec2    lvTexSize;
        vec2    lvfRpjCoords;
        vec2    lvfPxPrevPos;
        ivec2   lviPxPrevPos;
        vec2    lvfPxFrac;

        lvTexSize       = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
        lvfRpjCoords    = TEX_COORDS + DecodeMotion( lvMotion );
        lvfPxPrevPos    = lvfRpjCoords * vec2( lvTexSize ) - float2vec2( 0.5 );
        lviPxPrevPos    = ivec2( lvfPxPrevPos );
        lvfPxFrac       = fract( lvfPxPrevPos );

        // Project current depth into previous frame locations.
        // Push to all pixels having some contribution if reprojection is using bilinear logic.
        if ( lviPxPrevPos.x >= 0 && lviPxPrevPos.x <= int(lvTexSize.x) &&
             lviPxPrevPos.y >= 0 && lviPxPrevPos.y <= int(lvTexSize.y) )
        {
            // Initialise the reprojected depth buffer with a biased reprojection of fragments from the previous frame's depth buffer;
            // note that the fragments are reprojected in place (xy don't change, but the depth is updated);
            // avoids wholes in the depth clip buffer due to pigeon-hole problem in the backward reprojection
            float lfDepthRpj;
            {
                vec3  lvWorldPos;
                vec3  lvCamPos;
                mat4  lmInvViewProjSC;
                vec4  lvTempPos;
                mat4  lmViewProjPrev;
                float lfDepthPrev;
                float lfDepthBias;


                lfDepthPrev     = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), TEX_COORDS, 0.0 ) );
                lvCamPos        = lUniforms.mpPerFrame.gViewPositionVec3;
                lmInvViewProjSC = lUniforms.mpPerFrame.gPrevInverseViewProjectionSCMat4;
                lvWorldPos      = RecreatePositionFromRevZDepthSC( lfDepthPrev, lvFragCoords, lvCamPos, lmInvViewProjSC );
                lmViewProjPrev  = lUniforms.mpPerFrame.gViewProjectionMat4;
                lvTempPos       = MUL( lmViewProjPrev, vec4( lvWorldPos, 1.0 ) );
                lfDepthBias     = 0.95;
                lfDepthRpj      = lvTempPos.z / lvTempPos.w * lfDepthBias;

                #if defined ( D_PLATFORM_METAL )
                imageAtomicMax(
                    IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjBkwd ),
                    ivec2( TEX_COORDS * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ),
                    asuint( lfDepthRpj ) );

                #else
                imageAtomicMax( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjBkwd ),
                                ivec2( TEX_COORDS * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ), asuint( lfDepthRpj ) );
                #endif
            }

            // Store current depth backward reprojected
            const float laBilinearWeights[ 2 ][ 2 ] =
            { { ( 1 - lvfPxFrac.x ) * ( 1 - lvfPxFrac.y ), ( lvfPxFrac.x ) * ( 1 - lvfPxFrac.y ) },
              { ( 1 - lvfPxFrac.x ) * (     lvfPxFrac.y ), ( lvfPxFrac.x ) * (     lvfPxFrac.y ) } };

            for ( int y = 0; y <= 1; ++y )
            {
                for ( int x = 0; x <= 1; ++x )
                {
                    if (  laBilinearWeights[ y ][ x ] > kfReconstructedDepthBilinearWeightThreshold )
                    {
                        #if defined ( D_PLATFORM_METAL )
                        imageAtomicMax( 
                            IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjBkwd ),
                            lviPxPrevPos + ivec2( x, y ),
                            asuint( lfDepthRevZ ) );
                        #else
                        imageAtomicMax( 
                            SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gDepthReprjBkwd ),
                            lviPxPrevPos + ivec2( x, y ),
                            asuint( lfDepthRevZ ) );
                        #endif
                    }
                }
            }
        }
    }
    #endif

    FRAGMENT_OUTPUT_T0 = lvMotion;
    FRAGMENT_OUTPUT_T1 = lfDepthRevZ;
}

#endif

// =================================================================================================
//
// MOTION VECTOR RESOLVE
//
// =================================================================================================

#ifdef D_POSTPROCESS_MOTIONRESOLVE

#if (defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS ))
#pragma PSSL_target_output_format(target 0 FMT_UNORM16_ABGR)
#endif

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{           
    vec2 lSample = IN(mTexCoordsVec2);

    // compute the previous position of this pixel
    float lfDepth;
    float lfDepthNormalised;
    float lfSpeed;
    vec2 lMotion;
    vec2 lEncodedMotion;
    bool lWantsColorClipAA;
    vec2 lSampleReproject = GetPrevPosition(lSample, 
                                     lUniforms.mpPerFrame.gClipPlanesVec4,
                                     lUniforms.mpPerFrame.gInverseProjectionMat4,
                                     lUniforms.mpPerFrame.gInverseViewMat4,
                                     lUniforms.mpPerFrame.gInverseViewProjectionMat4,
                                     lUniforms.mpPerFrame.gViewProjectionMat4,
                                     lUniforms.mpPerFrame.gPrevViewProjectionMat4,
                                     lUniforms.mpPerFrame.gViewPositionVec3,
                                     lUniforms.mpPerFrame.gMBlurSettingsVec4,
                                     lUniforms.mpPerFrame.gFoVValuesVec4,
                                     lUniforms.mpPerFrame.giAntiAliasingIndex,
                                     SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), 
                                     SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer1Map ),
                                     lfDepth,
                                     lfDepthNormalised,
                                     lfSpeed,
                                     lMotion,
                                     lEncodedMotion,
                                     lWantsColorClipAA );

    #ifdef D_WRITE_WANTS_CLIP
    lWantsColorClipAA = true;
    #endif

    if( !lWantsColorClipAA )
    {
        int liMaterialID = int( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lSample ).r * 255.0 );
        lWantsColorClipAA = ( liMaterialID & D_CLAMP_AA ) != 0;
    }

    float lfEncodedSettings = 0.0;
    if( lWantsColorClipAA ) lfEncodedSettings += 0.5;

    #ifdef D_WRITE_IS_TERRAIN
    lfEncodedSettings += 0.25;
    #endif

    FRAGMENT_COLOUR = vec4( lEncodedMotion, lfEncodedSettings, 1.0 );    
}

#endif


// =================================================================================================
//
// TEMPORAL AA
//
// =================================================================================================


#ifdef D_POSTPROCESS_STAA_APPLY


//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#define TAA_BACK_IS_TONEMAP 0
#define TAA_BACK_IS_TONEMAP_YCGCO 0
#define TAA_BACK_IS_YCGCO 0

#define D_SPEED_MULTIPLIER 1000.0

#if TAA_BACK_IS_TONEMAP

#define TAA_ACC_BUFFER_INPUT( pos ) texture2DLod(lUniforms.mpCustomPerMesh.gBuffer1Map, pos, 0.0).rgb
#define TAA_MAIN_BUFFER_INPUT( pos ) saturate( TonemapKodak( texture2DLod(lUniforms.mpCustomPerMesh.gBufferMap, pos, 0.0).rgb ) / TonemapKodak( vec3( 1.0,1.0,1.0 ) ) )

#define TAA_ACC_BUFFER_OUTPUT( col ) ( col )
#define TAA_MAIN_BUFFER_OUTPUT( col ) GammaCorrectOutput( col )

#define TAA_GET_LUMINANCE( col ) ( 0.25 * ( col.r + col.b ) + 0.5 * col.g )

#elif TAA_BACK_IS_TONEMAP_YCGCO

#define TAA_ACC_BUFFER_INPUT( pos ) ( RGBToYCgCo( texture2DLod(lUniforms.mpCustomPerMesh.gBuffer1Map, pos, 0.0).rgb ) )
#define TAA_MAIN_BUFFER_INPUT( pos ) ( TonemapKodakYCgCo( RGBToYCgCo( texture2DLod(lUniforms.mpCustomPerMesh.gBufferMap, pos, 0.0).rgb ) ) )

#define TAA_ACC_BUFFER_OUTPUT( col ) ( YCgCoToRGB(  col ) )
#define TAA_MAIN_BUFFER_OUTPUT( col ) GammaCorrectOutput( YCgCoToRGB(  col ) )

#define TAA_GET_LUMINANCE( col ) ( col.x )

#elif TAA_BACK_IS_YCGCO

#define TAA_ACC_BUFFER_INPUT( pos ) texture2DLod(lUniforms.mpCustomPerMesh.gBuffer1Map, pos, 0.0).rgb
#define TAA_MAIN_BUFFER_INPUT( pos ) SimpleReinhardLum( RGBToYCgCo( texture2DLod(lUniforms.mpCustomPerMesh.gBufferMap, pos, 0.0).rgb ) )

#define TAA_ACC_BUFFER_OUTPUT( col ) ( col )
#define TAA_MAIN_BUFFER_OUTPUT( col ) GammaCorrectOutput( TonemapKodak( YCgCoToRGB( SimpleUnReinhardLum( col ) ) ) / TonemapKodak( vec3( 1.0,1.0,1.0 ) ) )

#define TAA_GET_LUMINANCE( col ) ( col.x )

#else

#if defined( D_SAMPLERS_ARE_GLOBAL )

#define TAA_ACC_BUFFER_INPUT( pos ) texture2DLod( gBuffer1Map, pos, 0.0).rgb
#define TAA_MAIN_BUFFER_INPUT( pos ) ( SimpleReinhard( clamp( texture2DLod( gBufferMap, pos, 0.0).rgb, float2vec3(0.0), float2vec3(1024.0) )  ) )

#else

#define TAA_ACC_BUFFER_INPUT( pos ) texture2DLod(lUniforms.mpCustomPerMesh.gBuffer1Map, pos, 0.0).rgb
#define TAA_MAIN_BUFFER_INPUT( pos ) ( SimpleReinhard( clamp( texture2DLod(lUniforms.mpCustomPerMesh.gBufferMap, pos, 0.0).rgb, float2vec3(0.0), float2vec3(1024.0) )  ) )

#endif

#define TAA_ACC_BUFFER_OUTPUT( col ) ( col )
//#define TAA_MAIN_BUFFER_OUTPUT( col ) GammaCorrectOutput( TonemapKodak( SimpleUnReinhard( col ) ) / TonemapKodak( vec3( 1.0,1.0,1.0 ) ) )
#define TAA_MAIN_BUFFER_OUTPUT( col ) ( SimpleUnReinhard( col ) )


#define TAA_GET_LUMINANCE( col ) ( 0.25 * ( col.r + col.b ) + 0.5 * col.g )

#endif



vec3 SimpleReinhard( in vec3 col )
{
    return col / ( 1.0 + TAA_GET_LUMINANCE( col ) );
}

vec3 SimpleUnReinhard( in vec3 col )
{
    return col / ( 1.0 - TAA_GET_LUMINANCE( col ) );
}


vec3 SimpleReinhardLum( in vec3 col )
{
    return col * 1.0 / ( 1.0 + col.x );
}

vec3 SimpleUnReinhardLum( in vec3 col )
{
    return ( col ) * 1.0 / ( 1.0 - col.x );
}


#ifdef D_PLATFORM_ORBIS    
//#pragma argument (O4; fastmath; scheduler=minpressure)
//#pragma argument(reservelds=28)
//#pragma warning (disable:7203)
//#pragma argument(targetoccupancy_atallcosts=90)
#if !defined( D_SPLITSCREEN_TEST ) && !defined( D_TERRAIN_MODE )
#pragma argument(maxvgprcount=36)
#endif

#else

float rcp( float inX )
{
    return 1.0 / inX;
}

#endif

// returns from 1.0 for no clip, down to 0.0 for "the aabb is size 0"
float get_clip_factor(
    vec3 q,
    vec3 aabb_min,
    vec3 aabb_max )
{
    vec3 p_clip = 0.5 * (aabb_max + aabb_min);
    vec3 e_clip = 0.5 * (aabb_max - aabb_min);

    vec3 v_clip = q - p_clip;
    vec3 v_unit = v_clip / e_clip;
    vec3 a_unit = abs(v_unit);

    float ma_unit = max( a_unit.x, max( a_unit.y, a_unit.z ) );

    if( ma_unit > 1.0 )
        return 1.0 / ma_unit;
    else
        return 1.0;
}

vec3 clip_aabb(
    vec3 q,
    vec3 aabb_min,
    vec3 aabb_max )
{
        // note: only clips towards aabb center (but fast!)
        vec3 p_clip = 0.5 * (aabb_max + aabb_min);
        vec3 e_clip = 0.5 * (aabb_max - aabb_min);

        vec3 v_clip = q - p_clip;
        vec3 v_unit = e_clip / v_clip;
        vec3 a_unit = abs(v_unit);
        float ma_unit = saturate( min(a_unit.x, min(a_unit.y, a_unit.z)) );

        return p_clip + v_clip * ma_unit;
}

vec3 clip_lum(
    vec3 q,
    vec3 aabb_min,
    vec3 aabb_max )
{
    vec3  p_clip = 0.5 * (aabb_max + aabb_min);
    float e_clip = 0.5 * (aabb_max.x - aabb_min.x);

    vec3  v_clip = q - p_clip;

    float v_abs = abs( v_clip.x );

    if( v_abs > e_clip )
    {
        float a_unit = v_abs / e_clip;
        return p_clip + v_clip / a_unit;
    }
    else
    {
        return q;
    }
}

vec3 clip_lum_flat(
    vec3 q,
    float aabb_min,
    float aabb_max )
{
    float p_clip = 0.5 * (aabb_max + aabb_min);
    float e_clip = 0.5 * (aabb_max - aabb_min);

    float v_clip = q.x - p_clip;
    //TF_BEGIN replace v_clip.x by v_clip
    float v_abs = abs( v_clip );

    if( v_abs > e_clip )
    {
        float a_unit = v_abs / e_clip;
        return q * ( p_clip + v_clip / a_unit ) / q.x;
    }
    else
    {
        return q;
    }
}

FRAGMENT_MAIN_COLOUR_SRT
{           
    #ifdef D_ALWAYS_COLOUR_CLIP
        bool lbForceColorClip = true;
    #else
        bool lbForceColorClip = false;
    #endif

    #ifdef D_TERRAIN_MODE
        bool lbIsTerrain = true;
    #else
        bool lbIsTerrain = false;
    #endif

    vec2 lTexCoords = TEX_COORDS;
    vec2 lEncodedDeltaDilated = vec2( 0, 0 );
    vec4 lLowResMotion;
    vec2 lEncodedDeltaMinDilated = vec2( 0, 0 );
    vec3 cM;
    vec3 cM_jittered;
    vec3 cAcc;
    vec3 cAcc_as_read;
    float lfSpeedBlend = 1.0;

    vec2 ldejit = GetDejitteredTexCoord( lTexCoords.xy, lUniforms.mpPerFrame.gDeJitterVec3 );

#ifdef D_CENTER_ONLY_TAA
    vec2 lCenter = lTexCoords;
    lCenter.x = fract( lCenter.x * 2.0 );
    lCenter -= 0.5;
    
    bool lbInCenter = dot( lCenter, lCenter ) < ( 0.5 - lUniforms.mpPerFrame.gDeJitterVec3.z );
    /*if( !lbInCenter )
    {
        WRITE_FRAGMENT_COLOUR0( vec4( 0.0, 0.0, 1.0, 1.0 ) );
        return;
    }*/

    if( !lbInCenter )
    {
        cM = TAA_MAIN_BUFFER_INPUT( ldejit.xy );
        cAcc = cM;
    }
    else
#endif
    {
        // compute the previous position of this pixel
        vec3 lCenterSmpMotion = Texture2DNoFiltering( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ), lTexCoords ).xyz;

        lEncodedDeltaDilated    = lCenterSmpMotion.xy;
        lEncodedDeltaMinDilated = lCenterSmpMotion.xy;

        bool lbUsePrevAccum = true;
#if defined( D_NO_MOTION_DILATE_BUFF )
        lbUsePrevAccum = abs( lCenterSmpMotion.x - 0.5 ) < 0.45;
        lbUsePrevAccum = lbUsePrevAccum && abs( lCenterSmpMotion.y - 0.5 ) < 0.45;
#else
        lLowResMotion = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lTexCoords, 0.0 );
        
        lbUsePrevAccum = abs( lLowResMotion.x - 0.5 ) < 0.45;
        lbUsePrevAccum = lbUsePrevAccum && abs( lLowResMotion.y - 0.5 ) < 0.45;
#endif

        if( lUniforms.mpPerFrame.gTaaSettingsVec4.x < 0.0 )
        {
            lbForceColorClip = true;
        }

        if( lCenterSmpMotion.z >= 0.5 )
        {
            lbForceColorClip = true;
            lCenterSmpMotion.z -= 0.5;
        }

        if( lCenterSmpMotion.z >= 0.25 )
        {
            lbIsTerrain = true;
            lCenterSmpMotion.z -= 0.25;
        }

#ifndef D_NO_MOTION_DILATE
        {
            float lfSpeedDilated = dot( lCenterSmpMotion.xy - 0.5, lCenterSmpMotion.xy - 0.5 );
            if( lfSpeedDilated > 0.235 )
            {
                lbForceColorClip = true;
            }

            // dilate the motion vectors in a 3x3 max/min pattern so we don't miss nearby moving edges

#ifndef D_ALWAYS_COLOUR_CLIP
            {
                float lfMinSpeedDilated = lfSpeedDilated;

                for( float dy = -1.0; dy <= 1.0; dy += 1.0 )
                {
                    for( float dx = -1.0; dx <= 1.0; dx += 1.0 )
                    {
                        if( dx != 0.0 || dy != 0.0 )
                        {
                            vec2 lCoordsOffset = vec2( dx, dy ) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
                            vec2 lSubTexCoords = lTexCoords + lCoordsOffset;

                            vec3 lSmpMotion = Texture2DNoFiltering( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ), lSubTexCoords ).xyz;
                            float lSpeedSquared = dot( lSmpMotion.xy - 0.5, lSmpMotion.xy - 0.5 );

                            if( lSmpMotion.z >= 0.5 )
                            {
                                lbForceColorClip = true;
                            }

                            if( lSpeedSquared >= lfSpeedDilated )
                            {
                                lfSpeedDilated = lSpeedSquared;
                                lEncodedDeltaDilated = lSmpMotion.xy;
                            }

                            if( lSpeedSquared < lfMinSpeedDilated )
                            {
                                lfMinSpeedDilated = lSpeedSquared;
                                lEncodedDeltaMinDilated = lSmpMotion.xy;
                            }
                        }
                    }
                }
            }
#endif
        }
#endif
        
        // four points, plus center, from the current screen
        cM = TAA_MAIN_BUFFER_INPUT( ldejit.xy );

        // also the current screen but without dejitter, to blend to backbuffer
        cM_jittered = TAA_MAIN_BUFFER_INPUT( lTexCoords.xy );

        vec2 lDeltaDilated = DecodeMotion( lEncodedDeltaDilated );

        vec2 lTexCoordsReproject = lTexCoords + lDeltaDilated;

        float lfSpeed = length( lDeltaDilated ) * D_SPEED_MULTIPLIER;
        bool lReadAccBuffer = ((saturate( lTexCoordsReproject.x ) == lTexCoordsReproject.x) &&
            (saturate( lTexCoordsReproject.y ) == lTexCoordsReproject.y) &&
            (lfSpeed < 128.0));

        cAcc = cM_jittered;
        cAcc_as_read = cAcc;  // (for debug output)

        float largeVelBlend = 0.0;
        // if reproj is inside the back texture
        if( lReadAccBuffer )
        {
            // accum buffer result for current pixel
            cAcc = TAA_ACC_BUFFER_INPUT( lTexCoordsReproject );
            cAcc_as_read = cAcc;

            // blend out if velocity is a lot of the screen
            largeVelBlend = saturate( (1.0 / 64.0) * (lfSpeed - 64.0) );
            cAcc = mix( cAcc, cM_jittered, largeVelBlend );
        }

        // if there is no disagreement over the pixel color,
        // we can skip a LOT of texture reads, logic, etc
        vec3 cDiff = cM_jittered - cAcc;
        vec3 cDiff2 = cM - cAcc;

        lbUsePrevAccum = lbUsePrevAccum && dot( cDiff, cDiff ) + dot( cDiff2, cDiff2 ) > 1.0 / 1024.0;
        
        if( lbUsePrevAccum )
        {
            vec2 lPrevDeltaEncoded = lEncodedDeltaDilated;
            vec2 lPrevMinDeltaEncoded = lEncodedDeltaMinDilated;

#ifndef D_ALWAYS_COLOUR_CLIP
            //if( !lbForceColorClip )
            {
                vec4 lPrevSpeeds = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), lTexCoordsReproject, 0.0 );

                lPrevDeltaEncoded = lPrevSpeeds.xy;
                vec2 lPrevDelta = DecodeMotion( lPrevSpeeds.xy );

                largeVelBlend = saturate( (1.0 / 64.0) * (length( lPrevDelta ) * D_SPEED_MULTIPLIER - 64.0) );
                cAcc = mix( cAcc, cM_jittered, largeVelBlend );

                if( lPrevSpeeds.z != 1.0 )
                {
                    lPrevMinDeltaEncoded = lPrevSpeeds.zw;

                    vec2 lPrevMinDelta = DecodeMotion( lPrevSpeeds.zw );
                    vec2 lDeltaMinDilated = DecodeMotion( lEncodedDeltaMinDilated );

#ifndef D_TERRAIN_MODE
                    if( !lbIsTerrain && (lUniforms.mpPerFrame.gFoVValuesVec4.z == 1.0))
                    {
                        // blending constant to kill the backbuffer when movement changes
                        // prevZ == 1.0 means we clipped last frame, no motion info, need to clip again this frame
                        lfSpeedBlend = 
                            saturate( 
                                max( 
                                    max( 
                                        length( lPrevMinDelta - lDeltaDilated ),
                                        length( lPrevDelta    - lDeltaMinDilated ) ),
                                    max( 
                                        length( lPrevMinDelta - lDeltaMinDilated ),
                                        length( lPrevDelta    - lDeltaDilated ) ) 
                                ) * D_SPEED_MULTIPLIER 
                            );

                    }
#endif
                }
            }
#endif

            float rcpFrameOptZ = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * lUniforms.mpPerFrame.gTaaSettingsVec4.z;
            float rcpFrameOptW = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w * lUniforms.mpPerFrame.gTaaSettingsVec4.z;

            // X-shaped sample pattern

            float wk = -TAA_GET_LUMINANCE( cM_jittered );

            // four points, plus center, from the current screen
            vec3 cL_M = TAA_MAIN_BUFFER_INPUT( lTexCoords + vec2( -rcpFrameOptZ, -rcpFrameOptW ) );
            wk += TAA_GET_LUMINANCE( cL_M ) * 0.25;
            vec3 cMax = cL_M;
            vec3 cMin = cL_M;

            vec3 cR_M = TAA_MAIN_BUFFER_INPUT( lTexCoords + vec2( rcpFrameOptZ, -rcpFrameOptW ) );
            wk += TAA_GET_LUMINANCE( cR_M ) * 0.25;
            cMax = max( cMax, cR_M );
            cMin = min( cMin, cR_M );

            vec3 cU_M = TAA_MAIN_BUFFER_INPUT( lTexCoords + vec2( -rcpFrameOptZ, rcpFrameOptW ) );
            wk += TAA_GET_LUMINANCE( cU_M ) * 0.25;
            cMax = max( cMax, cU_M );
            cMin = min( cMin, cU_M );

            vec3 cD_M = TAA_MAIN_BUFFER_INPUT( lTexCoords + vec2( rcpFrameOptZ, rcpFrameOptW ) );
            wk += TAA_GET_LUMINANCE( cD_M ) * 0.25;
            cMax = max( cMax, cD_M );
            cMin = min( cMin, cD_M );

            // edge detection filter, produces blend values for the front and back buffers
            // bac buffer is more aggressive than the front buffer
            float kl = (lUniforms.mpPerFrame.gMBlurSettingsVec4.x);
            float kh = (lUniforms.mpPerFrame.gMBlurSettingsVec4.y);

            // this math is from the Crytek SMAA filter
            float blendAmountAcc  = 1.0 - saturate( rcp( mix( kl, kh, abs( wk ) * 10.0 ) ) );
            float blendAmountMain = 1.0 - saturate( rcp( mix( kl, kh, abs( wk ) * 1.0  ) ) );

#ifdef D_TERRAIN_MODE

            blendAmountAcc = mix( lUniforms.mpPerFrame.gTaaSettingsVec4.w, lUniforms.mpPerFrame.gTaaSettingsVec4.y, blendAmountAcc );

#else
            if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 1.0)
            {               
                blendAmountAcc = mix(lUniforms.mpPerFrame.gTaaSettingsVec4.w, lUniforms.mpPerFrame.gMBlurSettingsVec4.w, blendAmountAcc);
            }
            else
            {
                //VR Mode
                blendAmountAcc = mix(lUniforms.mpPerFrame.gTaaSettingsVec4.w, lUniforms.mpPerFrame.gTaaSettingsVec4.y, blendAmountAcc);
            }

#endif

            //blendAmountMain = min( blendAmountMain, get_clip_factor(cM_jittered, cMin, cMax) );

#ifdef D_NO_BACKBUFFER

            cM = mix( cM, cAcc, blendAmountMain );
            cM = clip_aabb( cM, cMin, cMax );

#else


            // clip the new value to the AABB _without_ the pixel center
            // this tends to kill alias-y razor lines in the front buffer
            vec3 cM_clip = clip_aabb( cM, cMin, cMax );

            // but for the back buffer value, clip _with_ the pixel center
            // thus allowing bright razor lines that have been smoothed well
            // to come forward, where they can edge things nicely
            vec3 cMin_WithPix = min( cM, cMin );
            vec3 cMax_WithPix = max( cM, cMax );
            vec3 cAcc_clip = clip_aabb( cAcc, cMin_WithPix, cMax_WithPix );

            cM = mix( cM_clip, cAcc_clip, blendAmountMain );

            // idea from staring at Uncharted 4 (before the actual presentation came out)
            // blend _aggressively_ to the backbuffer when change in motion vectors is small 
            //if( !lbForceColorClip )
            {
                cM = mix( cAcc, cM, lbForceColorClip ? 1.0 : lfSpeedBlend );
            }
            cAcc = mix( cAcc, cAcc_clip, saturate( lfSpeedBlend * 2.0 ) );
            cAcc = mix( cM_jittered, cAcc, blendAmountAcc );

#endif
        }
        else
        {
            cAcc = cM;
        }

        cAcc = max( vec3( 0.0, 0.0, 0.0 ), min( vec3( 10000.0, 10000.0, 10000.0 ), cAcc ) );

#ifdef D_NO_BACKBUFFER
        cM = max( vec3( 0.0, 0.0, 0.0 ), min( vec3( 10000.0, 10000.0, 10000.0 ), cM ) );
        cAcc = cM;
#endif
    }

    // debugging
    #ifdef D_SPLITSCREEN_TEST

    //FRAGMENT_COLOUR0 = vec4( vec3( lfSpeedBlend ), 1.0 );

    float lTestBoxesY = lTexCoords.y;
    #ifndef D_PLATFORM_OPENGL
    lTestBoxesY = 1.0 - lTestBoxesY;
    #endif   

    if( lTexCoords.x > 0.66 )
    {

        if( lTestBoxesY > 0.5 )
        {
            FRAGMENT_COLOUR0 = vec4( lLowResMotion.xy, 0.0, 1.0 );
        }
        else
        {
            FRAGMENT_COLOUR0 = vec4( lEncodedDeltaDilated, 0.0, 1.0 );
        }

    }
    else if( lTexCoords.x > 0.33 )
    {
        if( lTestBoxesY > 0.5 )
        {
            FRAGMENT_COLOUR0 = vec4( TAA_MAIN_BUFFER_OUTPUT( cAcc_as_read ), 1.0 );
        }
        else
        {
            FRAGMENT_COLOUR0 = vec4( vec3( lbForceColorClip? 1.0 : 0.0, 0.0, lfSpeedBlend ), 1.0 );
        }
    }
    else
    {
        if( lTestBoxesY > 0.5 )
        {
            FRAGMENT_COLOUR0 = vec4( TAA_MAIN_BUFFER_OUTPUT( cM ), 1.0 );
        }
        else
        {
            FRAGMENT_COLOUR0 = vec4( TAA_MAIN_BUFFER_OUTPUT( cM_jittered ), 1.0 );
        }
    }

    /*

    if( lTexCoords.x > 0.5 )
    {
        if( lTexCoords.y > 0.5 )
        {
        FRAGMENT_COLOUR0 = vec4( TAA_MAIN_BUFFER_OUTPUT( cAcc_NoReproject ), 1.0 );
        }
        else
        {


        //FRAGMENT_COLOUR0 = lReadAccBuffer? vec4( vec3( 1.0 ), 1.0 ) : vec4( vec3( 0.0 ), 1.0 );
         //   FRAGMENT_COLOUR0 =  vec4( vec3( largeVelBlend ), 1.0 );
          // FRAGMENT_COLOUR0 =  vec4( lEncodedDeltaDilated, 0.0, 1.0 );
          FRAGMENT_COLOUR0 =  vec4( lDeltaDilated, 0.0, 1.0 );

                
            if( lTexCoords.y > 0.25 )
            {
                if( lTexCoords.x < 0.75 )
                {
                    FRAGMENT_COLOUR0 = vec4( abs( DecodeMotion( lEncodedDeltaMinDilated ) ) * 20.0, 0.0, 1.0 );
                }
                else
                {
                    FRAGMENT_COLOUR0 = vec4( abs( lDeltaDilated ) * 20.0, 0.0, 1.0 );
                }
            }
            else
            {
                if( lTexCoords.x < 0.75 )
                {
                    FRAGMENT_COLOUR0 = vec4( abs( DecodeMotion( lPrevMinDeltaEncoded ) ) * 20.0, 0.0, 1.0 );
                }
                else
                {
                    FRAGMENT_COLOUR0 = vec4( abs( DecodeMotion( lPrevDeltaEncoded ) ) * 20.0, 0.0, 1.0 );
                }

            } 
        }
    }
    else if( lTexCoords.y > 0.5 )
    {
        FRAGMENT_COLOUR0 = vec4( TAA_MAIN_BUFFER_OUTPUT( cAcc ), 1.0 );
    }
    else
    {
        FRAGMENT_COLOUR0 = vec4( TAA_MAIN_BUFFER_OUTPUT( cM ), 1.0 );
    }
    */

    #else

    WRITE_FRAGMENT_COLOUR0( vec4( TAA_MAIN_BUFFER_OUTPUT( cM ), 1.0 ) );

    #endif


    // output a new value to the accum byffer
    WRITE_FRAGMENT_COLOUR1( vec4( TAA_ACC_BUFFER_OUTPUT( cAcc ), 1.0 ) );

    if( lbForceColorClip )
    {
        WRITE_FRAGMENT_COLOUR2( vec4( lEncodedDeltaDilated, 1.0, 1.0 ) );
    }
    else
    {
        WRITE_FRAGMENT_COLOUR2( vec4( lEncodedDeltaDilated, lEncodedDeltaMinDilated ) );
    }


}

#endif


// =================================================================================================
//
// GAMMACORRECT
//
// =================================================================================================

#ifdef D_POSTPROCESS_GAMMACORRECT

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lColourVec4;
    lColourVec4     = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS.xy );
    lColourVec4.rgb = GammaCorrectOutput( lColourVec4.rgb );

    WRITE_FRAGMENT_COLOUR( lColourVec4 );
}

#endif



// =================================================================================================
//
// GAMMACORRECT
//
// =================================================================================================

#ifdef D_POSTPROCESS_DEGAMMA

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    //#ifdef D_COMPUTE
    //vec4 lColourVec4 = FRAGMENT_COLOUR;
    //#else
    vec4 lColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS.xy);
    //#endif
    WRITE_FRAGMENT_COLOUR( vec4(GammaCorrectInput(lColourVec4.rgb), lColourVec4.a) );
}

#endif

// =================================================================================================
//
// CLEAR GBUFFER
//
// =================================================================================================

#ifdef D_POSTPROCESS_CLEAR_GBUFFER

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

FRAGMENT_MAIN_COLOUR_SRT
{
    FRAGMENT_COLOUR0 = vec4(0.0, 0.0, 0.0, 0.0);
    FRAGMENT_COLOUR1 = vec4(1.0, 1.0, 1.0, 1.0);
    FRAGMENT_COLOUR2 = vec4(0.0, 0.0, 0.0, 0.0);
    FRAGMENT_COLOUR3 = vec4(0.0, 0.0, 0.0, 0.0);
}

#endif





// =================================================================================================
//
// BILATERAL UPSAMPLE
//
// =================================================================================================

#ifdef D_POSTPROCESS_BILATERAL_UPSAMPLE
//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//    Functions

#if !defined( D_PLATFORM_GLSL ) 
    vec2
    BlurUVOffsets( in const uint luIdx )
    {
        // this form allows the nearest depth index search to compute the offsets without branching.
        return vec2( (luIdx & 1) ? 0.5 : -0.5, (luIdx & 2) ? 0.5 : -0.5 );
    }
#elif defined( D_PLATFORM_SWITCH )
    vec2
    BlurUVOffsets(in const uint luIdx)
    {
        // this form allows the nearest depth index search to compute the offsets without branching.
        return vec2( ((luIdx & 1)==1) ? 0.5 : -0.5, ((luIdx & 2)==2) ? 0.5 : -0.5);
    }
#else       
    #define BlurUVOffsets( idx )    ( kaBlurUVOffsets[ ( idx ) ] )
    const vec2 kaBlurUVOffsets[ 4 ] = vec2[ 4 ]( vec2( -0.5, -0.5 ), vec2( 0.5, -0.5 ), vec2( -0.5, 0.5 ), vec2( 0.5, 0.5 ) );
#endif    



#if defined( D_POSTPROCESS_BILATERAL_REDONLY )


    vec4
        BilateralBlur(
            SAMPLER2DARG(lColourTexture),
            SAMPLER2DARG(lLowResDepthTexture),
            SAMPLER2DARG(lHighResDepthTexture),
            in vec2 lTexCoordsVec2,
            in vec2 lTexSizeVec2,
            in vec2 lRecipTexSizeVec2,
            in vec4 lClipPlanes,
            inout uint luRejectResult)
{
    vec2  lAdjustedTexCoordsVec2;
    float lafDownsampledDepth[4];
    float lfFullResDepth;
    vec4  lColourReadVec4;
    float lafBilinearWeights[4];

#if !defined( D_POSTPROCESS_BILATERAL_SIMPLE )
    lfFullResDepth           = DecodeDepthFromColour( texture2D( lHighResDepthTexture, lTexCoordsVec2 ) );

    vec4 lvRawDepths = textureGatherRed( lLowResDepthTexture, lTexCoordsVec2 );

    lafDownsampledDepth[ 0 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.x ) );
    lafDownsampledDepth[ 1 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.y ) );
    lafDownsampledDepth[ 2 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.z ) );
    lafDownsampledDepth[ 3 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.w ) );
#endif

    vec2 lPixelLoc  = lTexCoordsVec2 * lTexSizeVec2 - vec2(0.5, 0.5);
    vec2 lSubPixel = fract( lPixelLoc ); 

    lafBilinearWeights[ 0 ] = ( 1.0 - lSubPixel.x )*(       lSubPixel.y );
    lafBilinearWeights[ 1 ] = (       lSubPixel.x )*(       lSubPixel.y );
    lafBilinearWeights[ 2 ] = (       lSubPixel.x )*( 1.0 - lSubPixel.y );
    lafBilinearWeights[ 3 ] = ( 1.0 - lSubPixel.x )*( 1.0 - lSubPixel.y );

    const float lfUpsampleDepthThreshold = 10.0 * lClipPlanes.w;        //  / farplane

#if !defined( D_POSTPROCESS_BILATERAL_SIMPLE )
    for ( uint i = 0u; i < 4u; i++ )
    {
        float currentDepthDiff    = abs( lafDownsampledDepth[i] - lfFullResDepth );
        bool  lbRejectSample      = currentDepthDiff > lfUpsampleDepthThreshold;

        if( lbRejectSample )
        {
            lafBilinearWeights[ i ] = 0.0;
        }
    }

    float lfTotalWeight = lafBilinearWeights[ 0 ] + lafBilinearWeights[ 1 ] + lafBilinearWeights[ 2 ] + lafBilinearWeights[ 3 ];

    if( lfTotalWeight > 0.0 )
    {
        vec4 lvWeights = vec4( lafBilinearWeights[ 0 ], lafBilinearWeights[ 1 ], lafBilinearWeights[ 2 ], lafBilinearWeights[ 3 ] );
        lvWeights /= lfTotalWeight;

        lColourReadVec4.r = dot( lvWeights, textureGatherRed  ( lColourTexture, lTexCoordsVec2 ) );
    }
    else
    {
        lColourReadVec4 = texture2D( lColourTexture, lTexCoordsVec2 );
    }
#else

    float lfTotalWeight = lafBilinearWeights[0] + lafBilinearWeights[1] + lafBilinearWeights[2] + lafBilinearWeights[3];
    {
        vec4 lvWeights = vec4(lafBilinearWeights[0], lafBilinearWeights[1], lafBilinearWeights[2], lafBilinearWeights[3]);
        lvWeights /= lfTotalWeight;

        lColourReadVec4.r = dot(lvWeights, textureGatherRed(lColourTexture, lTexCoordsVec2));
    }
#endif
    return  vec4(lColourReadVec4.r, 0.0, 0.0, 1.0);
}

#else

vec4
BilateralBlur(
    SAMPLER2DARG( lColourTexture ),
    SAMPLER2DARG( lLowResDepthTexture ),
    SAMPLER2DARG( lHighResDepthTexture ),
    in vec2 lTexCoordsVec2,
    in vec2 lTexSizeVec2,
    in vec2 lRecipTexSizeVec2,
    in vec4 lClipPlanes,
    inout uint luRejectResult)
{
    vec2  lAdjustedTexCoordsVec2;
    vec4  lColourReadVec4;

#if defined( D_POSTPROCESS_BILATERAL_WITH_RMAP )
    bool lbRejectSample = bool(luRejectResult & 4);
    uint luNearestDepthIndex = luRejectResult & 3;
    lAdjustedTexCoordsVec2 = lTexCoordsVec2;
    if (!lbRejectSample)
    {
        lTexCoordsVec2 += (BlurUVOffsets(luNearestDepthIndex) * lRecipTexSizeVec2);
    }
    lColourReadVec4 = vec4(texture2DLod(lColourTexture, lTexCoordsVec2, 0.0).r, 0.0, 0.0, 0.0);
#else

    float lafDownsampledDepth[4];
    float lfFullResDepth;

    lfFullResDepth           = DecodeDepthFromColour( texture2DLod( lHighResDepthTexture, lTexCoordsVec2, 0.0) );

    vec4 lvRawDepths = textureGatherRed( lLowResDepthTexture, lTexCoordsVec2 );
    lafDownsampledDepth[ 0 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.w ) ); // equivalent to texture2DLod( lLowResDepthTexture, lTexCoordsVec2 + (BlurUVOffsets( 0 ) * lRecipTexSizeVec2), 0.0 ).r
    lafDownsampledDepth[ 1 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.z ) ); // equivalent to texture2DLod( lLowResDepthTexture, lTexCoordsVec2 + (BlurUVOffsets( 1 ) * lRecipTexSizeVec2), 0.0 ).r
    lafDownsampledDepth[ 2 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.x ) ); // equivalent to texture2DLod( lLowResDepthTexture, lTexCoordsVec2 + (BlurUVOffsets( 2 ) * lRecipTexSizeVec2), 0.0 ).r
    lafDownsampledDepth[ 3 ] = DecodeDepthFromColour( float2vec4( lvRawDepths.y ) ); // equivalent to texture2DLod( lLowResDepthTexture, lTexCoordsVec2 + (BlurUVOffsets( 3 ) * lRecipTexSizeVec2), 0.0 ).r

#if defined (D_PARTICLE_BLEND)
    // Particle downsampled depth buffer is linear (and not normalised), so this si the best option there...
    // BUT: in D_PLATFORM_METAL Forward Pipeline, both buffers are already linear
    const float lfUpsampleDepthThreshold = 10.0;
#if !defined( D_PLATFORM_METAL )
    lfFullResDepth = FastDenormaliseDepth( lClipPlanes, lfFullResDepth );
#endif
#else
    const float lfUpsampleDepthThreshold = 10.0 * lClipPlanes.w;        //  / 1.0 / farplane
#endif

    uint  luNearestDepthIndex = 0u;
    float currentDepthDiff    = abs( lafDownsampledDepth[0] - lfFullResDepth );
    bool  lbRejectSample      = currentDepthDiff < lfUpsampleDepthThreshold;
    float lfMinDepthDiff      = currentDepthDiff;
    
    for ( uint i = 1u; i < 4u; i++ )
    {
        currentDepthDiff = abs( lafDownsampledDepth[i] - lfFullResDepth );
        lbRejectSample   = lbRejectSample && currentDepthDiff < lfUpsampleDepthThreshold;

        if ( currentDepthDiff < lfMinDepthDiff )
        {
            lfMinDepthDiff      = currentDepthDiff;
            luNearestDepthIndex = i;
        }
    }

    // Avoid blocky artifacts using edge detection
    lAdjustedTexCoordsVec2 = lTexCoordsVec2;
    if (!lbRejectSample)
    {
        lAdjustedTexCoordsVec2 = (round( lTexCoordsVec2 * lTexSizeVec2 ) + BlurUVOffsets( luNearestDepthIndex )) * lRecipTexSizeVec2;
    }
#if defined( D_POSTPROCESS_BILATERAL_OUTPUT_RMAP )
    luRejectResult = luNearestDepthIndex | (lbRejectSample ? 4 : 0);
#endif

    lColourReadVec4 = texture2DLod( lColourTexture, lAdjustedTexCoordsVec2, 0.0);

    lColourReadVec4.a = clamp( lColourReadVec4.a, 0.0, 1.0 );
#endif

    return lColourReadVec4;
}
#endif
 
FRAGMENT_MAIN_COLOUR_SRT
{
    uint luRejectResult;
#if defined( D_POSTPROCESS_BILATERAL_WITH_RMAP )
    luRejectResult = uint( texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh,gBuffer3Map), TEX_COORDS, 0.0).r );
#else
    luRejectResult = 0;
#endif

    vec4 lResult = BilateralBlur(    SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ),
                                     SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer1Map ),
                                     SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBuffer2Map ),
                                     TEX_COORDS,
                                     lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy * 0.5,
                                     lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * 2.0,
                                     lUniforms.mpPerFrame.gClipPlanesVec4,
        luRejectResult);

    #if defined( D_POSTPROCESS_BILATERAL_OUTPUT_RMAP )
        ivec2 liFragCoords = ivec2( SCREENSPACE_AS_RENDERTARGET_UVS( TEX_COORDS ) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy);
        #if defined ( D_PLATFORM_METAL )
        imageStore( IMAGE_GETMAP(lUniforms.mpCustomPerMesh, gBilatRejectMap), liFragCoords, ivec4(luRejectResult, 0.0, 0.0, 0.0) );
        #else
        imageStore(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBilatRejectMap), liFragCoords, ivec4(luRejectResult, 0.0, 0.0, 0.0));
        #endif
    #endif

    #ifdef D_BLEND_SCREEN
    {
        // Screen Overlay Mode for Light Shafts

        float lfLightAccumulation = lResult.r;
        float lfLightAccumulationSc = lfLightAccumulation * lUniforms.mpCustomPerMesh.gSkyUpperParamsVec4.y;
#if defined( D_POSTPROCESS_BILATERAL_WITH_RMAP )
        if (lfLightAccumulationSc < (2.0 / 255.0))
        {
            WRITE_FRAGMENT_COLOUR(vec4(0.0, 0.0, 0.0, 1.0));
            return;
        }
#endif

        lResult.rgb = mix(lUniforms.mpCustomPerMesh.gLightShaftColourBottomVec4.rgb, lUniforms.mpCustomPerMesh.gLightShaftColourTopVec4.rgb, lfLightAccumulation);
        lResult.rgb *= lfLightAccumulationSc;
#if defined( D_PLATFORM_SWITCH )
        float lfDepth = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer1Map), TEX_COORDS, 0.0).x;
#else
        float lfDepth = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer2Map), TEX_COORDS, 0.0).x;
#endif
        float lfDepthScale = (1.0 - sqrt(lfDepth));
        lResult.xyz = clamp(lfDepthScale * (lResult.xyz),0.0,128.0);
        lResult.a   = 1.0;
    }
    #endif

#if defined( D_POSTPROCESS_BILATERAL_REDONLY )
    WRITE_FRAGMENT_COLOUR( vec4(lResult.r, 0.0, 0.0, 1.0) );
#else

    #if !defined (D_PARTICLE_BLEND)
    // Remove errant colour-space conversion that wasn't there for previous particle blend...
    lResult.rgb = MUL(lResult.rgb, sRGB_TO_P3D65);
    #endif

    #if defined (D_PARTICLE_BLEND)
    lResult = vec4( lResult.rgb, 1.0 - lResult.a );

    #ifdef D_BLEND_POST_TAA
    vec4 lPostTaaTexVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), TEX_COORDS.xy, 0.0 );
    lResult.rgb = lPostTaaTexVec4.rgb + lResult.rgb;
    #endif

    #ifdef D_COMPUTE
    // manual blending in compute-land
    WRITE_FRAGMENT_COLOUR( lResult + FRAGMENT_COLOUR * ( 1.0 - lResult.a ) );
    #else
    WRITE_FRAGMENT_COLOUR( lResult );
    #endif

    #elif defined( D_BLEND_ADD ) && defined D_COMPUTE
    vec4 lStoredColour = FRAGMENT_COLOUR;
    lStoredColour.rgb += lResult.rgb;
    WRITE_FRAGMENT_COLOUR( lStoredColour );
    #elif defined( D_BLEND_BLEND ) && defined D_COMPUTE
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR * ( 1.0 - lResult.a ) + lResult * lResult.a );
    #else
    WRITE_FRAGMENT_COLOUR( lResult );
    #endif
#endif

}

#endif


// =================================================================================================
// 
// D_POSTPROCESS_DEPTH_AWARE_H_GUASS / D_POSTPROCESS_DEPTH_AWARE_V_GUASS
//
// =================================================================================================

#if defined( D_POSTPROCESS_DEPTH_AWARE_H_GUASS ) || defined( D_POSTPROCESS_DEPTH_AWARE_V_GUASS )

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

#define GaussFilterWeights( idx )       gauss_filter_weights[ ( idx ) ]

// http://dev.theomader.com/gaussian-kernel-calculator/

vec4
GatherGauss(
    SAMPLER2DARG( lColourTexture ),
    SAMPLER2DARG( lDepthTexture  ),
#if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
    SAMPLER2DARG( lRedTexture ),
    out float lRedResult,
#endif
    in float lfMip,
    in vec2  lFrameBufferSizeRecipVec2,
    in vec2  blurDirection,
    in vec2  uv,
    in vec4  lClipPlanes )
{

    #if defined( D_POSTPROCESS_DEPTH_AWARE_LITE )
    #define NUM_SAMPLES_HALF 2
    // sigma 1.7 Kernel Size = 5
    const float gauss_filter_weights[3] = { 0.26943,    0.227745,   0.13754 };
    #else

    #ifndef D_PLATFORM_PC
    #define NUM_SAMPLES_HALF 4
    // sigma 2.4 Kernel Size = 9
    const float gauss_filter_weights[5] = { 0.175713,   0.161305,   0.124789,   0.081355,   0.044695 };
    #else
    // sigma 2.765 Kernel Size = 15
    #define NUM_SAMPLES_HALF 7
    const float gauss_filter_weights[8] = { 0.14446445, 0.13543542, 0.11153505, 0.08055309, 0.05087564, 0.02798160, 0.01332457, 0.00545096 };
    #endif
    #endif

    #if defined( D_PLATFORM_SWITCH )
    const float kfBlurScale = 1.0 /  1.0; // 1.0 unit  threshold
    #else
    const float kfBlurScale = 1.0 /  0.5; // 0.5 units threshold
    #endif

    float centerDepth     = FastDenormaliseDepth( lClipPlanes * kfBlurScale, DecodeDepthFromColour( texture2DLod( lDepthTexture, uv, 0.0) ) );
    float accumWeights    = GaussFilterWeights( 0 );
    vec4  accumResult     = texture2DLod( lColourTexture, uv, 0.0 )   * accumWeights;
    #if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
    float accumRedResult  = texture2DLod( lRedTexture,    uv, 0.0 ).r * accumWeights;
    #endif

    for ( int r = 1; r <= NUM_SAMPLES_HALF; ++r )
    {
        vec2  uvOffset          = lFrameBufferSizeRecipVec2 * blurDirection * float( r );

        vec4  kernelSample1     = texture2DLod( lColourTexture, uv - uvOffset, lfMip );
        vec4  kernelSample2     = texture2DLod( lColourTexture, uv + uvOffset, lfMip );
        #if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
        float kernelRedSample1  = texture2DLod( lRedTexture,    uv - uvOffset, lfMip ).r;
        float kernelRedSample2  = texture2DLod( lRedTexture,    uv + uvOffset, lfMip ).r;
        #endif

        float kernelDepth1 = FastDenormaliseDepth( lClipPlanes * kfBlurScale, DecodeDepthFromColour( texture2DLod( lDepthTexture, uv - uvOffset, 0.0 ) ) );
        float kernelDepth2 = FastDenormaliseDepth( lClipPlanes * kfBlurScale, DecodeDepthFromColour( texture2DLod( lDepthTexture, uv + uvOffset, 0.0 ) ) );

        // Simple depth-aware filtering
        float depthDiff1 = abs( kernelDepth1 - centerDepth );
        float depthDiff2 = abs( kernelDepth2 - centerDepth );

        float g1 = saturate( 1.0 - ( depthDiff1 * depthDiff1 ) );
        float g2 = saturate( 1.0 - ( depthDiff2 * depthDiff2 ) );

        float gauss_weight = GaussFilterWeights( r );
        float weight1 = g1 * gauss_weight;
        float weight2 = g2 * gauss_weight;

        accumResult += weight1 * kernelSample1;
        accumResult += weight2 * kernelSample2;

        #if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
        accumRedResult += weight1 * kernelRedSample1;
        accumRedResult += weight2 * kernelRedSample2;
        #endif

        accumWeights += weight1;
        accumWeights += weight2;
    }

    #if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
    lRedResult = accumRedResult / accumWeights;
    #endif

    return  accumResult / accumWeights;
}

#if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
FRAGMENT_MAIN_COLOUR01_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{
    float lRedResult = 0.0;
    vec4 lResult = GatherGauss(
                            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ),
                            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ),
#if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
                            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ),
                            lRedResult,
#endif
#if defined( D_MIP )
                            lUniforms.mpCustomPerMesh.gBlurParamsVec4.w,
#else
                            0.0,
#endif
                            lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw,
#if defined( D_POSTPROCESS_DEPTH_AWARE_H_GUASS )
                            vec2(1.0, 0.0),
#else
                            vec2(0.0, 1.0),
#endif
                            TEX_COORDS,
                            lUniforms.mpPerFrame.gClipPlanesVec4 );

#if defined( D_POSTPROCESS_DEPTH_AWARE_RGBAR )
    WRITE_FRAGMENT_COLOUR0( lResult );
    WRITE_FRAGMENT_COLOUR1( vec4( lRedResult, 0.0, 0.0, 1.0 ) );
#else

#if defined( D_POSTPROCESS_DEPTH_AWARE_REDONLY )
    WRITE_FRAGMENT_COLOUR( vec4( lResult.r, 0.0, 0.0, 1.0 ) );
#else
    WRITE_FRAGMENT_COLOUR( lResult );
#endif

#endif
}

#endif


#ifdef D_POSTPROCESS_GUASS_SIMPLE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 



FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lTextureResolutionVec2 = GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) );
    vec2 lRecipTexSize = vec2( 1.0 / lTextureResolutionVec2.x, 1.0 / lTextureResolutionVec2.y );

    const float gauss_filter_weights[5] = { 0.382928,	0.241732,	0.060598,	0.005977,	0.000229 };

    vec2 centreUV = TEX_COORDS;

    vec4 lFragColorVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), centreUV ) * gauss_filter_weights[0];
    for ( int i = 1; i <= 4; i++ )
    {
        vec4 col;
        vec2 texCoordOffset = lUniforms.mpCustomPerMesh.gBlurParamsVec4.xy * float( i ) * lRecipTexSize;

        col = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), centreUV + texCoordOffset );
        col += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), centreUV - texCoordOffset );

        lFragColorVec4 += col * gauss_filter_weights[i];
    }

    WRITE_FRAGMENT_COLOUR( vec4(lFragColorVec4) );
}
#endif



#ifdef D_POSTPROCESS_SHARPEN

// =================================================================================================
//
// SHARPEN
//
// =================================================================================================

//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float gaWeights[ 25 ] =
{
    -0.00391, -0.01563, -0.02344, -0.01563, -0.00391,
    -0.01563, -0.06250, -0.09375, -0.06250, -0.01563,
    -0.02344, -0.09375, +0.85980, -0.09375, -0.02344,
    -0.01563, -0.06250, -0.09375, -0.06250, -0.01563,
    -0.00391, -0.01563, -0.02344, -0.01563, -0.00391
};



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lCentreUV      = TEX_COORDS;    
    vec4    lFragColor     = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lCentreUV );    
    vec4    lSharpenColor  = vec4( 0, 0, 0, 0 );
    float   lfCoeff        = lUniforms.mpCustomPerMesh.gSharpenStrength.x;
    float   lfDepth        = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lCentreUV ).r;
    vec2    lDepthBounds   = lUniforms.mpCustomPerMesh.gSharpenStrength.yz / 1000000;

    float   lfDepthFactor  = 1.0f - smoothstep( lDepthBounds.x, lDepthBounds.y, lfDepth );

    if ( lfDepthFactor > 0.0 )
    {
        for ( int i = -2; i <= 2; i++ )
        {
            for ( int j = -2; j <= 2; j++ )
            {
                int     liWeightIdx = ( i + 2 ) * 5 + ( j + 2 );
            
                vec2    lOffset     = vec2( j, i );
                float   lfWeight    = gaWeights[ liWeightIdx ];                       
            
                vec2    lSampleUV   = lCentreUV + lOffset * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;            
                vec4    lSampleCol  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lSampleUV );            
            
                lSharpenColor += lSampleCol * lfWeight;
            }        
        }

        //lFragColor = vec4(lfDepthFactor, lFragColor.yzw);
        lFragColor += lSharpenColor * lfCoeff * lfDepthFactor;
    }
    
    WRITE_FRAGMENT_COLOUR( lFragColor );
}

#endif

#ifdef D_BLOOM_RESOLVE

// =================================================================================================
//
// BLOOM_RESOLVE
//
// =================================================================================================

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    float lfDither;
    vec4  lFragColor;        
    lFragColor   = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ),  TEX_COORDS ) / lUniforms.mpCustomPerMesh.gBlurParamsVec4.x;
    lFragColor  += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ) / lUniforms.mpCustomPerMesh.gBlurParamsVec4.y;
    lFragColor  += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), TEX_COORDS ) / lUniforms.mpCustomPerMesh.gBlurParamsVec4.z;    
    //lFragColor  *= mix( 1.0, sqrt( length( lFragColor ), 0.5 );


    #if defined(D_SKIP_B) // added to avoid blurring DOF mask
    lFragColor.b = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ),  TEX_COORDS ).b;
    #endif

    WRITE_FRAGMENT_COLOUR( lFragColor );
}

#endif

#ifdef D_POSTPROCESS_BLOOM_EXPOSURE

// =================================================================================================
//
// BLOOM_EXPOSURE
//
// =================================================================================================
DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#define NB_SAMPLES  8
#define HISTERISIS  0.1
#define SQR_SCALE   3.0
#define LIN_SCALE   8.0

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lCoords;
    vec2    lTexCoords      = TEX_COORDS.xy;
    vec2    lStepSizeVec2   = 1.0 / vec2( GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) ) );
    vec4    lExpVec4        = float2vec4( 0.0 );
    
    float   lfCurrExp       = 0.0;
    float   lfPrevExp       = 0.0;
    float   lfLum           = 0.0;

    for (int ii = -NB_SAMPLES / 2; ii < NB_SAMPLES / 2; ++ii)
    {
        for (int jj = -NB_SAMPLES / 2; jj < NB_SAMPLES / 2; ++jj)
        {
            lCoords     = lTexCoords + vec2( ii, jj ) * lStepSizeVec2;
            vec4 lTemp  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lCoords );            
            lExpVec4   += SQR_SCALE * sqrt( lTemp );
        }
    }
    
    lExpVec4  /= NB_SAMPLES * NB_SAMPLES;
//TF_BEGIN
#if defined(D_FORWARD)
    lfLum = lExpVec4.r;
#else
    lfLum      = max( max( lExpVec4.r, lExpVec4.g ), lExpVec4.b );
#endif
//TF_END
    lfLum     *= LIN_SCALE;
    lfCurrExp  = 1.0 / lfLum;
    lfCurrExp  = mix( 0.015, 0.485, saturate( lfCurrExp ) );
    lfPrevExp  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lTexCoords ).r;
    lfCurrExp  = mix( lfPrevExp, lfCurrExp, HISTERISIS );

    WRITE_FRAGMENT_COLOUR( vec4( lfCurrExp, 0.0, 0.0, 0.0 ) );
}

#endif

#ifdef D_POSTPROCESS_BLOOM_APPLY

// =================================================================================================
//
// BLOOM_APPLY
//
// =================================================================================================
DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

float
Exposure( SAMPLER2DARG( lExpBuff ) ) 
{
    float lfExp;
    lfExp  = texture2D(lExpBuff, vec2(0.25, 0.25)).r;
    lfExp += texture2D(lExpBuff, vec2(0.75, 0.25)).r;
    lfExp += texture2D(lExpBuff, vec2(0.25, 0.75)).r;
    lfExp += texture2D(lExpBuff, vec2(0.75, 0.75)).r;

    return lfExp;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec3  lvFragCol  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), TEX_COORDS ).rgb;
    vec3  lvBloom    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ).rgb;

    #if defined( D_EXPOSURE )
    float lfExposure = Exposure( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ) );
    #endif

    #if defined( D_TONEMAP )
    // this is not the actual final tonemapper; it's just to match the look of other platforms
    // it seems like unfortunately on those we have a stray tonemap operation coming from DoF :(
    lvFragCol.rgb  = TonemapKodak( lvFragCol ) / TonemapKodak( float2vec3( 1.0 ) );
    #endif

    lvFragCol.rgb += lvBloom;

    #if defined( D_CONVERT_GAMUT )
    lvFragCol.rgb  = max( float2vec3( 0.0 ), MUL( lvFragCol.rgb, P3D65_TO_sRGB ) );
    #endif

    lvFragCol      = GammaCorrectOutput( lvFragCol );

    WRITE_FRAGMENT_COLOUR( vec4( lvFragCol, 1.0 ) );

}

#endif

#ifdef D_POSTPROCESS_BRIGHTPASS_NEW

// =================================================================================================
//
// BRIGHTPASS_NEW
//
// =================================================================================================

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 

#define sqrt_fast   sqrt_fast_0

vec3 
Threshold(
    in vec3 lColour,
    in float lfThreshold,      
    in float lfGain )
{   
    float lum;
    lum      = max( lColour.r, max( lColour.g, lColour.b ) );
    lum      = max( ( lum - lfThreshold ), 0.0 );
    lum      = 1.85 * sqrt_fast( lum );

    return lColour * lum * lfGain;
}

FRAGMENT_MAIN_COLOUR_SRT
{
#if !defined( D_REFLECTION_PROBE )
    vec3  lBrightColourVec3 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), TEX_COORDS ).xyz;
    lBrightColourVec3       = clamp( lBrightColourVec3, float2vec3(0.0), float2vec3(1024.0) );

#if !defined( D_ADD_IN_COMPUTE )
    float lfGlowFactor      = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS ).a;
#else
    float lfGlowFactor      = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), TEX_COORDS ).r;
#endif

#else
    vec4  lReflProbeVec4    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gProbeCubemapFlat ), TEX_COORDS );
    vec3  lBrightColourVec3 = clamp( lReflProbeVec4.xyz, float2vec3(0.0), float2vec3(1024.0) );
    float lfGlowFactor      = lReflProbeVec4.a;
#endif

    float lfThreshold       = min( lUniforms.mpCustomPerMesh.gHDRParamsVec4.y, 1.0 - lfGlowFactor );
    float lfGain            = dot( lBrightColourVec3, lBrightColourVec3 );
    lfGain                 /= lUniforms.mpCustomPerMesh.gHDRParamsVec4.x * lUniforms.mpCustomPerMesh.gHDRParamsVec4.x;
    
    if ( lfGain <= lfThreshold * lfThreshold )
         lBrightColourVec3  = float2vec3( 0.0 );
    else lBrightColourVec3  = Threshold( lBrightColourVec3, lfThreshold, invsqrt( lfGain ) );
    
#if !defined( D_ADD_IN_COMPUTE )
    WRITE_FRAGMENT_COLOUR( vec4( lBrightColourVec3, 1.0 ) );
#else
    WRITE_FRAGMENT_COLOUR( FRAGMENT_COLOUR + vec4( lBrightColourVec3, 1.0 ) );
#endif
}


#endif

#ifdef D_POSTPROCESS_BRIGHTPASS_TEMPORAL

// =================================================================================================
//
// BRIGHTPASS_TEMPORAL
//
// =================================================================================================

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions 
vec3  sqr( vec3  v ) { return v * v; }

vec3
ClipHistory(
    vec3 q,
    vec3 aabb_min,
    vec3 aabb_max )
{
    // note: only clips towards aabb center (but fast!)
    vec3 p_clip = 0.5 * ( aabb_max + aabb_min );
    vec3 e_clip = 0.5 * ( aabb_max - aabb_min );

    vec3 v_clip = q - p_clip;
    vec3 v_unit = e_clip / v_clip;
    vec3 a_unit = abs( v_unit );
    float ma_unit = saturate( min( a_unit.x, min( a_unit.y, a_unit.z ) ) );

    return p_clip + v_clip * ma_unit;
}

//----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lvTexCoords = TEX_COORDS.xy;

    // Get Motion
    vec3    lvEcdMotion = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lvTexCoords, 0.0 ).xyz;
    vec2    lvDcdMotion = DecodeMotion( lvEcdMotion.xy );
    vec2    lvRpjCoords = lvTexCoords + lvDcdMotion;
    vec2    lvStepSize  = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;

    // Sample current colour
    vec3    lvColCurr   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvTexCoords, 0.0 ).rgb;
    
    // Sample current 4 neighbours in a cross pattern
    vec3    lvColCurrT  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvTexCoords - vec2( 0.0, lvStepSize.y ), 0.0 ).rgb;
    vec3    lvColCurrB  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvTexCoords + vec2( 0.0, lvStepSize.y ), 0.0 ).rgb;
    vec3    lvColCurrL  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvTexCoords - vec2( lvStepSize.x, 0.0 ), 0.0 ).rgb;
    vec3    lvColCurrR  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvTexCoords + vec2( lvStepSize.x, 0.0 ), 0.0 ).rgb;

    // Sample history colour
    vec3    lvColHist   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lvRpjCoords, 0.0 ).rgb;

    // Compute clipping box using Salvi's variance based method
    vec3    lvMoment0   = lvColCurrT + lvColCurrB + lvColCurrL + lvColCurrR + lvColCurr;
    vec3    lvMoment1   = sqr( lvColCurrT ) + sqr( lvColCurrB ) + sqr( lvColCurrL ) + sqr( lvColCurrR ) + sqr( lvColCurr );
    vec3    lvAvg       = lvMoment0 / 5.0;
    vec3    lvStd       = sqrt( max( float2vec3( 0.0 ), lvMoment1 / 5.0 - lvAvg * lvAvg ) );

    // Clip History
    vec3    lvColHClip  = max( float2vec3( 0.0 ), ClipHistory( lvColHist, lvAvg - 4.0 * lvStd, lvAvg + 4.0 * lvStd ) );

    // Accumulate
    vec3    lvColFinal;

    // Use the average as it's more stable and we don't care about detail/sharpness
    // Let a bit of unclipped history in; trades minor amount of ghosting for more stability
    lvColFinal  = mix( lvColHist,  lvAvg,      0.125 );

    // Blend with the clipped history
    lvColFinal  = mix( lvColHClip, lvColFinal, 0.250 );

    WRITE_FRAGMENT_COLOUR( vec4( lvColFinal, 1.0 ) );
}

#endif


// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_HIGH_Z || D_POSTPROCESS_LOW_Z || D_POSTPROCESS_HIGH_LOW_Z_MULTI
// ------------------------------------------------------------------------------------------------
#if defined( D_POSTPROCESS_HIGH_Z ) || defined( D_POSTPROCESS_LOW_Z ) || defined( D_POSTPROCESS_HIGH_LOW_Z_MULTI )


// TODO(sal)  use max3 and min3 on supported platforms
//#if defined( D_POSTPROCESS_HIGH_Z )
//#define cmp_func_2( a, b )       max( a, b )
//#define cmp_func_3( a, b, c )    max( a, max( b, c ) )
//#else
//#define cmp_func_2( a, b )       min( a, b )
//#define cmp_func_3( a, b, c )    min( a, min( b, c ) )
//#endif

#if defined( D_PLATFORM_XBOXONE )
#define max3( a, b, c )    __XB_Max3_F32( a, b, c )
#define min3( a, b, c )    __XB_Min3_F32( a, b, c )
#elif !defined( D_PLATFORM_ORBIS )
#define max3( a, b, c )    max( a, max( b, c ) )
#define min3( a, b, c )    min( a, min( b, c ) )
#endif

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

#define Z_COMPUTE( cmp_func )                                                                                                       \
                                                                                                                                    \
    float lfDepth = 0.0;                                                                                                            \
                                                                                                                                    \
    vec4 depthTexelValues;                                                                                                          \
	depthTexelValues.x = textureLoadF(lgDepthMap,                                                                                   \
                                      previousLevelBaseTexelCoord,                                                                  \
                                      liLodIdx).r;                                                                                  \
	depthTexelValues.y = textureLoadF(lgDepthMap,                                                                                   \
                                      min( previousLevelBaseTexelCoord + ivec2(1, 0), previousLevelDimensions ),                    \
                                      liLodIdx).r;                                                                                  \
	depthTexelValues.z = textureLoadF(lgDepthMap,                                                                                   \
                                      min( previousLevelBaseTexelCoord + ivec2(1, 1), previousLevelDimensions ),                    \
                                      liLodIdx).r;                                                                                  \
	depthTexelValues.w = textureLoadF(lgDepthMap,                                                                                   \
                                      min( previousLevelBaseTexelCoord + ivec2(0, 1), previousLevelDimensions ),                    \
                                      liLodIdx).r;                                                                                  \
                                                                                                                                    \
	lfDepth            = cmp_func( depthTexelValues.x,                                                                              \
                                     cmp_func##3( depthTexelValues.y, depthTexelValues.z, depthTexelValues.w ) );                   \
                                                                                                                                    \
    /* Incorporate additional texels if the previous level's width or height (or both) are odd. */                                  \
	bool shouldIncludeExtraColumnFromPreviousLevel  = previousLevelDimensions.x < 2 || ((previousLevelDimensions.x & 1) != 0);      \
	bool shouldIncludeExtraRowFromPreviousLevel     = previousLevelDimensions.y < 2 || ((previousLevelDimensions.y & 1) != 0);      \
                                                                                                                                    \
	if (shouldIncludeExtraColumnFromPreviousLevel)                                                                                  \
    {                                                                                                                               \
		vec2 extraColumnTexelValues;                                                                                                \
		extraColumnTexelValues.x = textureLoadF(lgDepthMap,                                                                         \
                                                min( previousLevelBaseTexelCoord + ivec2(2, 0), previousLevelDimensions ),          \
                                                liLodIdx).r;                                                                        \
		extraColumnTexelValues.y = textureLoadF(lgDepthMap,                                                                         \
                                                min( previousLevelBaseTexelCoord + ivec2(2, 1), previousLevelDimensions ),          \
                                                liLodIdx).r;                                                                        \
                                                                                                                                    \
		/* In the case where the width and height are both odd, need to include the */                                              \
        /*   'corner' value as well. */                                                                                             \
		if (shouldIncludeExtraRowFromPreviousLevel)                                                                                 \
        {                                                                                                                           \
			float cornerTexelValue = textureLoadF(lgDepthMap,                                                                       \
                                                  min( previousLevelBaseTexelCoord + ivec2(2, 2), previousLevelDimensions ),        \
                                                  liLodIdx).r;                                                                      \
            lfDepth = cmp_func(lfDepth, cornerTexelValue);                                                                          \
		}                                                                                                                           \
        lfDepth = cmp_func##3(lfDepth, extraColumnTexelValues.x, extraColumnTexelValues.y);                                         \
	}                                                                                                                               \
	if (shouldIncludeExtraRowFromPreviousLevel)                                                                                     \
    {                                                                                                                               \
		vec2 extraRowTexelValues;                                                                                                   \
		extraRowTexelValues.x = textureLoadF(lgDepthMap,                                                                            \
                                             min( previousLevelBaseTexelCoord + ivec2(0, 2), previousLevelDimensions ),             \
                                             liLodIdx).r;                                                                           \
		extraRowTexelValues.y = textureLoadF(lgDepthMap,                                                                            \
                                             min( previousLevelBaseTexelCoord + ivec2(1, 2), previousLevelDimensions ),             \
                                             liLodIdx).r;                                                                           \
        lfDepth = cmp_func##3(lfDepth, extraRowTexelValues.x, extraRowTexelValues.y);                                               \
	}                                                                                                                               \
                                                                                                                                    \
    return lfDepth;

float 
GetHighZ(
    SAMPLER2DARG( lgDepthMap ),
    int   liLodIdx,
    ivec2 thisLevelTexelCoord,
    ivec2 previousLevelBaseTexelCoord,
    ivec2 previousLevelDimensions )
{
    Z_COMPUTE( max )
}

float 
GetLowZ(
    SAMPLER2DARG( lgDepthMap ),
    int   liLodIdx,
    ivec2 thisLevelTexelCoord,
    ivec2 previousLevelBaseTexelCoord,
    ivec2 previousLevelDimensions )
{
    Z_COMPUTE( min )
}

STATIC_CONST float SLICE_SPLIT  = 0.0075;
STATIC_CONST float SLICE_EPS    = 0.0005;

// Based on:
// https://miketuritzin.com/post/hierarchical-depth-buffers/
#if !defined( D_POSTPROCESS_HIGH_LOW_Z_MULTI )
FRAGMENT_MAIN_COLOUR_SRT
#else
FRAGMENT_MAIN_COLOUR0123_SRT
#endif
{    
    int   liLodIdx                      = int(lUniforms.mpCustomPerMesh.gBlurParamsVec4.w);
    ivec2 thisLevelTexelCoord           = ivec2( IN_SCREEN_POSITION.x, IN_SCREEN_POSITION.y );
    ivec2 previousLevelBaseTexelCoord   = 2 * thisLevelTexelCoord;
    ivec2 previousLevelDimensions       = ivec2( GetTexResolutionLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), int( lUniforms.mpCustomPerMesh.gBlurParamsVec4.w ) ) );


#if defined( D_PLATFORM_VULKAN ) || defined( D_PLATFORM_ORBIS )
    // In Vulkan/PS4/PS5 we use the image view to determine which lod we are reading from
    previousLevelDimensions = ivec2( GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) ) );
    liLodIdx                = 0;
#endif

#if !defined( D_POSTPROCESS_HIGH_LOW_Z_MULTI )

    float lfHiLowZ   = 
    #if defined( D_POSTPROCESS_HIGH_Z )
                      GetHighZ( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ),
    #else
                      GetLowZ(  SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ),
    #endif
                                liLodIdx,
                                thisLevelTexelCoord,
                                previousLevelBaseTexelCoord,
                                previousLevelDimensions );

    WRITE_FRAGMENT_COLOUR( float2vec4( lfHiLowZ ) );
#else

    vec4  lHiLowZ;

    #if !defined( D_POSTPROCESS_HIGH_LOW_Z_MULTI_START )
    lHiLowZ.x   = GetHighZ( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ),
                            liLodIdx,
                            thisLevelTexelCoord,
                            previousLevelBaseTexelCoord,
                            previousLevelDimensions );

    lHiLowZ.y   = GetHighZ( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ),
                            liLodIdx,
                            thisLevelTexelCoord,
                            previousLevelBaseTexelCoord,
                            previousLevelDimensions );

    lHiLowZ.z   = GetLowZ(  SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ),
                            liLodIdx,
                            thisLevelTexelCoord,
                            previousLevelBaseTexelCoord,
                            previousLevelDimensions );

    lHiLowZ.w   = GetLowZ(  SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ),
                            liLodIdx,
                            thisLevelTexelCoord,
                            previousLevelBaseTexelCoord,
                            previousLevelDimensions );
    #else

    float lfDepth;

    lfDepth     = GetHighZ( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ),
                            liLodIdx,
                            thisLevelTexelCoord,
                            previousLevelBaseTexelCoord,
                            previousLevelDimensions );

    lHiLowZ.x   = lfDepth > SLICE_SPLIT - SLICE_EPS ? lfDepth : 0.0;
    lHiLowZ.y   = lfDepth < SLICE_SPLIT + SLICE_EPS ? lfDepth : 0.0;
    lHiLowZ.z   = lfDepth > SLICE_SPLIT - SLICE_EPS ? lfDepth : 1.0;
    lHiLowZ.w   = lfDepth < SLICE_SPLIT + SLICE_EPS ? lfDepth : 1.0;

#endif

    WRITE_FRAGMENT_COLOUR0( float2vec4( lHiLowZ.x ) );
    WRITE_FRAGMENT_COLOUR1( float2vec4( lHiLowZ.y ) );
    WRITE_FRAGMENT_COLOUR2( float2vec4( lHiLowZ.z ) );
    WRITE_FRAGMENT_COLOUR3( float2vec4( lHiLowZ.w ) );
#endif
}
#endif

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_MIN
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_MIN

#if defined ( D_PLATFORM_ORBIS )
#pragma argument(unrollallloops)
#endif

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{    
    float   lfMin = 1.0 / 1.0e-16;

    vec2    lCentreUV       = TEX_COORDS.xy;
    vec2    lStepSizeVec2   = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;

    for ( int i = -3; i <= 3; i++ )
    {
        for ( int j = -3; j <= 3; j++ )
        {
            //int     liIdx       = ( i + 2 ) * 5 + ( j + 2 );
            vec2    lOffset     = vec2( j, i );
            vec2    lSampleUV   = lCentreUV + lOffset * lStepSizeVec2;
            vec4    lSampleCol  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lSampleUV );

            lfMin = min( lSampleCol.x, lfMin );
        }        
    }

    WRITE_FRAGMENT_COLOUR( float2vec4( lfMin ) );
}
#endif


// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_DISOCCLUSION_TEMPORAL
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_DISOCCLUSION_TEMPORAL

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

STATIC_CONST float DISOCCL_EPS  = 0.010;
STATIC_CONST float DISOCCL_HYST = 0.025;

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lFragCoordsVec2         = TEX_COORDS.xy;
    
    vec4    lEncodedMotionVec4      = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer2Map), lFragCoordsVec2, 0.0);
    vec2    lMotionCurrVec2         = DecodeMotion(lEncodedMotionVec4.xy);
    vec2    lReprojCoordsVec2       = lFragCoordsVec2 + lMotionCurrVec2;

    float   lfDepthCurrRevZ         = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lFragCoordsVec2,   0.0 ) );
    float   lfDepthPrevLinNorm      = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lReprojCoordsVec2, 0.0 ) );
    float   lfDepthPrevRevZ         = LinearNormToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepthPrevLinNorm );
    vec3    lWorldPosVec3           = RecreatePositionFromRevZDepth( lfDepthCurrRevZ, lFragCoordsVec2,
                                                                  lUniforms.mpPerFrame.gViewPositionVec3,
                                                                  lUniforms.mpPerFrame.gInverseViewProjectionMat4 );
    vec4    lScreenPosPrevVec4      = MUL( lUniforms.mpPerFrame.gThisToPrevViewProjectionMat4, vec4( lWorldPosVec3, 1.0 ) );
    float   lfDepthPrevRevZCpt      = lScreenPosPrevVec4.z / lScreenPosPrevVec4.w;
    float   lfDepthPrevLinNormCpt   = ReverseZToLinearDepthNorm( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepthPrevRevZCpt );

    float   lfThreshold             = mix( 0.0275, 0.00025, saturate( length( lMotionCurrVec2 ) * 512.0 ) );
    float   lfDisocclLin            = step( lfThreshold,          abs( lfDepthPrevLinNorm - lfDepthPrevLinNormCpt ) );
    float   lfDisocclRevz           = step( lfThreshold / 10.0,   abs( lfDepthPrevRevZ - lfDepthPrevRevZCpt ) );
    float   lfDisoccl               = max( lfDisocclLin, lfDisocclRevz );
    WRITE_FRAGMENT_COLOUR( float2vec4( lfDisoccl ) );
    //WRITE_FRAGMENT_COLOUR( vec4( lWorldPosVec3 / 128.0, 1.0 ) );
}
#endif

// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_SPEED_TEMPORAL
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_SPEED_TEMPORAL

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

float
GetSpeedWeight(
    vec2    lMotionVec2,
    float   lfSpeedWeightPrev )
{
    // Approximate speed based on current motion. Flatten the speed curve using a square root approximation.
    // Blend speed with its previous value from the accumulation buffer.

#ifdef D_PLATFORM_METAL
    const float  kfSpeedCoeff        = 128.0;
    const float  kfSpeedWeightAcc    = 0.5;

    // Avoid tiny movements likely caused by jitter
    // TODO(sal): make threshold a function of the jitter vector
    const float  kfSpeedLowThreshold = 0.000185;
#else
    STATIC_CONST float  kfSpeedCoeff        = 128.0;
    STATIC_CONST float  kfSpeedWeightAcc    = 0.5;

    // Avoid tiny movements likely caused by jitter
    // TODO(sal): make threshold a function of the jitter vector
    STATIC_CONST float  kfSpeedLowThreshold = 0.000185;
#endif

    const float         kfMotion            = length( lMotionVec2 );
    const float         kfSpeed             = kfMotion * step( kfSpeedLowThreshold, kfMotion ) * kfSpeedCoeff;
    float               lfSpeedWeight       = min( 1.0, kfSpeed );
    lfSpeedWeight                           = mix( lfSpeedWeightPrev, lfSpeedWeight, kfSpeedWeightAcc );

    return lfSpeedWeight;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lvUvCoords          = TEX_COORDS.xy;

    // Get Motion
    vec3    lvEcdMotion         = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lvUvCoords, 0.0 ).xyz;
    vec2    lvDcdMotion         = DecodeMotion( lvEcdMotion.xy );
    vec2    lvRpjCoords         = lvUvCoords + lvDcdMotion;

    if ( lvRpjCoords.x <  0 || lvRpjCoords.y <  0 ||
         lvRpjCoords.x >= 1 || lvRpjCoords.y >= 1 )
    {
        WRITE_FRAGMENT_COLOUR( float2vec4( 1.0 ) );
        return;
    }

    // Get Previous speed weight
    float   lfSpeedWeightPrev   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvRpjCoords, 0.0 ).x;
    float   lfSpeedWeight       = GetSpeedWeight( lvDcdMotion, lfSpeedWeightPrev );

    WRITE_FRAGMENT_COLOUR( float2vec4( lfSpeedWeight ) );
}
#endif


// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_FLAT_TO_RT_COPY
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_FLAT_TO_RT_COPY

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lUVsVec2    = TEX_COORDS;
    float   lfMip       = lUniforms.mpCustomPerMesh.gCustomParamsVec4.y;
    if (lUniforms.mpCustomPerMesh.gCustomParamsVec4.z > 0.0)
    {
        float   lfFace = lUniforms.mpCustomPerMesh.gCustomParamsVec4.x;
        lUVsVec2 = (lUVsVec2 + vec2(lfFace, 0.0)) * vec2(1.0 / 6.0, 1.0);
    }
#if defined( D_REFLECTION_PROBE )
    vec3    lColourVe3  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gProbeCubemapFlat ), lUVsVec2, lfMip ).rgb;
#else
    vec3    lColourVe3  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap        ), lUVsVec2, lfMip ).rgb;
#endif

    WRITE_FRAGMENT_COLOUR( vec4( lColourVe3, 1.0 ) );
}
#endif


// ------------------------------------------------------------------------------------------------
// D_POSTPROCESS_BLUR_RED_4X4
// ------------------------------------------------------------------------------------------------
#ifdef D_POSTPROCESS_BLUR_RED_4X4

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

float
BlurRed4x4(
    in vec2       lTexCoords,
    SAMPLER2DARG( lTexMap))
{
    vec2 lTexSize   = vec2( GetTexResolution( lTexMap ) );
    vec2 lTexelSize = 1.0 / lTexSize;
    vec4 lCloudShadow0 = textureGatherRed( lTexMap, lTexCoords);
    vec4 lCloudShadow1 = textureGatherRed( lTexMap, lTexCoords + vec2(1.0, 0.0) * lTexelSize);
    vec4 lCloudShadow2 = textureGatherRed( lTexMap, lTexCoords + vec2(0.0, 1.0) * lTexelSize);
    vec4 lCloudShadow3 = textureGatherRed( lTexMap, lTexCoords + vec2(1.0, 1.0) * lTexelSize);

    float lfD1 = dot(lCloudShadow0, float2vec4(0.0625));
    float lfD2 = dot(lCloudShadow1, float2vec4(0.0625));
    float lfD3 = dot(lCloudShadow2, float2vec4(0.0625));
    float lfD4 = dot(lCloudShadow3, float2vec4(0.0625));

    return lfD1 + lfD2 + lfD3 + lfD4;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lUVsVec2    = TEX_COORDS;
    float   lfRed       = BlurRed4x4( lUVsVec2, SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ) );

    WRITE_FRAGMENT_COLOUR( vec4( lfRed, 0.0, 0.0, 1.0 ) );
}
#endif

// =================================================================================================
//
// CHECKERBOARD_RESOLVE_SCATTERING
//
// =================================================================================================

#if defined( D_POSTPROCESS_CHECKERBOARD_RESOLVE_SCATTERING )

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT

INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

bool
SameSurface(
    vec4 lvA,
    vec4 lvB )
{
    return lvA.a == lvB.a;
}

vec2
DifferentialBlending(
    vec4 lvA,
    vec4 lvB )
{
    //float lfA_Lum = dot( lvA.rgb, vec3( 0.2126, 0.7152, 0.0722 ) );
    //float lfB_Lum = dot( lvA.rgb, vec3( 0.2126, 0.7152, 0.0722 ) );
    //float lfDiff  = max( max( abs( lvA.a - lvB.a ), abs( lfA_Lum - lfB_Lum ) ), 0.01 );
    //
    //return 1.0 / lfDiff;

    return float2vec2( 1.0 / ( distance( lvA, lvB ) + 0.01 ) );
}

vec4
GetValidityWeights(
    in  uint luReject,
    in  int  liSampleIdx,
    out bool lbAnyValid )
{
    vec4 lvWeights;

    luReject    = luReject >> ( 4 * liSampleIdx );
    luReject    = luReject &  0xf;

    lbAnyValid  = luReject > 0;

    lvWeights.x = float( luReject &  0x1 );

    luReject    = luReject >> 1;
    lvWeights.y = float( luReject &  0x1 );

    luReject    = luReject >> 1;
    lvWeights.z = float( luReject &  0x1 );

    luReject    = luReject >> 1;
    lvWeights.w = float( luReject &  0x1 );

    lvWeights   = liSampleIdx == 0 ? lvWeights.yxwz : lvWeights;

    return lvWeights;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    ivec2 lFragCoordsFsVec2 = ivec2( IN_SCREEN_POSITION.xy );
    ivec2 lFragCoordsCbVec2 = ivec2( lFragCoordsFsVec2.xy ) / 2;

    bvec2 lvEven        = bvec2( 1 - ivec2( lFragCoordsFsVec2.xy ) & 0x1 );

    int   liSmpIdx      = lvEven.y ? 1 : 0;

    vec4  lvScatter     = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2, liSmpIdx );
    uint  luReject      = textureLoadU(   SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0uMap  ), lFragCoordsCbVec2, 0 ).r;

    #if defined( D_EVEN )
    bool  lbPassThrough = lvEven.x == lvEven.y;
    #else
    bool  lbPassThrough = lvEven.x != lvEven.y;
    #endif

    if ( !lbPassThrough )
    {
        #if defined( D_EVEN )
        ivec2 lvOffset = lvEven.y ? ivec2(  1, -1 ) : ivec2( -1, 1 );
        #else
        ivec2 lvOffset = lvEven.y ? ivec2( -1, -1 ) : ivec2(  1, 1 );
        #endif

        // Scatter cross blend
        {
            vec4  lvWeights;
            vec4  laCbScatter[ 4 ];
            bool  lbAnyValid;

            // 0 : even row (smp 1) - left  | odd row (smp 0) - right
            // 1 : even row (smp 1) - right | odd row (smp 0) - left
            // 2 : even row (smp 1) - up    | odd row (smp 0) - down
            // 3 : even row (smp 1) - down  | odd row (smp 0) - up

            #if defined( D_EVEN )
            laCbScatter[ 0 ] = lvScatter;
            laCbScatter[ 1 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2 + ivec2( lvOffset.x, 0 ),      liSmpIdx );
            #else
            laCbScatter[ 0 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2 + ivec2( lvOffset.x, 0 ),      liSmpIdx );
            laCbScatter[ 1 ] = lvScatter;
            #endif
            laCbScatter[ 2 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2 + ivec2( 0, lvOffset.y ),  1 - liSmpIdx );
            laCbScatter[ 3 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2,                           1 - liSmpIdx );

            lvWeights        = GetValidityWeights( luReject, liSmpIdx, lbAnyValid );

            if ( !lbAnyValid )
            {
                lvWeights        = float2vec4( 1.0 );
            }
            else
            {
                lvWeights.xy    *= DifferentialBlending( laCbScatter[ 0 ], laCbScatter[ 1 ] );
                lvWeights.zw    *= DifferentialBlending( laCbScatter[ 2 ], laCbScatter[ 3 ] );
            }

            lvScatter        = lvWeights.x * laCbScatter[ 0 ];
            lvScatter       += lvWeights.y * laCbScatter[ 1 ];
            lvScatter       += lvWeights.z * laCbScatter[ 2 ];
            lvScatter       += lvWeights.w * laCbScatter[ 3 ];
            lvScatter       /= lvWeights.x + lvWeights.y + lvWeights.z + lvWeights.w;
        }
    }

    WRITE_FRAGMENT_COLOUR( lvScatter );
}

#endif

// =================================================================================================
//
// CHECKERBOARD_RESOLVE_LIGHTSHAFTS
//
// =================================================================================================

#if defined( D_POSTPROCESS_CHECKERBOARD_RESOLVE_LIGHTSHAFTS )

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT

INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

vec4
GetValidityWeights(
    in  uint luReject,
    in  int  liSampleIdx,
    out bool lbAnyValid )
{
    vec4 lvWeights;

    luReject    = luReject >> ( 4 * liSampleIdx );
    luReject    = luReject &  0xf;

    lbAnyValid  = luReject > 0;

    lvWeights.x = float( luReject &  0x1 );

    luReject    = luReject >> 1;
    lvWeights.y = float( luReject &  0x1 );

    luReject    = luReject >> 1;
    lvWeights.z = float( luReject &  0x1 );

    luReject    = luReject >> 1;
    lvWeights.w = float( luReject &  0x1 );

    lvWeights   = liSampleIdx == 0 ? lvWeights.yxwz : lvWeights;

    return lvWeights;
}

float
DifferentialBlending(
    float lfA,
    float lfB )
{
    float lfDiff  = max( abs( lfA - lfB ), 0.01 );

    return 1.0 / lfDiff;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    ivec2 lFragCoordsFsVec2 = ivec2( IN_SCREEN_POSITION.xy );
    ivec2 lFragCoordsCbVec2 = ivec2( lFragCoordsFsVec2.xy ) / 2;

    bvec2 lvEven        = bvec2( 1 - ivec2( lFragCoordsFsVec2.xy ) & 0x1 );

    int   liSmpIdx      = lvEven.y ? 1 : 0;
    float lfLightshafts = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2, liSmpIdx ).r;
    uint  luReject      = textureLoadU(   SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0uMap  ), lFragCoordsCbVec2, 0 ).r;

    #if defined( D_EVEN )
    bool  lbPassThrough = lvEven.x == lvEven.y;
    #else
    bool  lbPassThrough = lvEven.x != lvEven.y;
    #endif

    if ( !lbPassThrough )
    {
        #if defined( D_EVEN )
        ivec2 lvOffset = lvEven.y ? ivec2(  1, -1 ) : ivec2( -1, 1 );
        #else
        ivec2 lvOffset = lvEven.y ? ivec2( -1, -1 ) : ivec2(  1, 1 );
        #endif

        // Lightshafts cross blend
        {
            vec4  lvWeights;
            float laCbLightshafts[ 4 ];
            bool  lbAnyValid;

            // 0 : even row (smp 1) - left  | odd row (smp 0) - right
            // 1 : even row (smp 1) - right | odd row (smp 0) - left
            // 2 : even row (smp 1) - up    | odd row (smp 0) - down
            // 3 : even row (smp 1) - down  | odd row (smp 0) - up

            #if defined( D_EVEN )
            laCbLightshafts[ 0 ] = lfLightshafts;
            laCbLightshafts[ 1 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2 + ivec2( lvOffset.x, 0 ),     liSmpIdx ).r;
            #else
            laCbLightshafts[ 0 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2 + ivec2( lvOffset.x, 0 ),     liSmpIdx ).r;
            laCbLightshafts[ 1 ] = lfLightshafts;
            #endif
            laCbLightshafts[ 2 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2 + ivec2( 0, lvOffset.y ), 1 - liSmpIdx ).r;
            laCbLightshafts[ 3 ] = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lFragCoordsCbVec2,                          1 - liSmpIdx ).r;

            lvWeights            = GetValidityWeights( luReject, liSmpIdx, lbAnyValid );

            if ( !lbAnyValid )
            {
                lvWeights        = float2vec4( 1.0 );
            }

            lfLightshafts        = lvWeights.x * laCbLightshafts[ 0 ];
            lfLightshafts       += lvWeights.y * laCbLightshafts[ 1 ];
            lfLightshafts       += lvWeights.z * laCbLightshafts[ 2 ];
            lfLightshafts       += lvWeights.w * laCbLightshafts[ 3 ];
            lfLightshafts       /= lvWeights.x + lvWeights.y + lvWeights.z + lvWeights.w;
        }
    }

    #if defined( D_REPROJECT )
    {
        // quick loose reprojection to stabilise the lightshafts a bit

        vec2  lvUVsCoords   = ( vec2( lFragCoordsFsVec2.xy ) + float2vec2( 0.5 ) ) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
        vec2  lvEcdMotion   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lvUVsCoords, 0.0 ).rg;
        vec2  lvDcdMotion   = DecodeMotion( lvEcdMotion );
        vec2  lvRpjCoords   = lvUVsCoords + lvDcdMotion;
        bool  lbValid       = saturate( lvRpjCoords.x ) == lvRpjCoords.x && saturate( lvRpjCoords.y ) == lvRpjCoords.y;

        if (  lbValid )
        {
            float lfLightShaftsAcc;
            lfLightShaftsAcc    = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvRpjCoords, 0.0 ).r;
            lfLightShaftsAcc    = clamp( lfLightShaftsAcc, lfLightshafts - 0.1, lfLightshafts + 0.1 );
            lfLightshafts       = mix( lfLightShaftsAcc, lfLightshafts, 0.375 );
        }
    }
    #endif

    WRITE_FRAGMENT_COLOUR( vec4( lfLightshafts, 0.0, 0.0, 1.0 ) );
}

#endif

#if defined( D_POSTPROCESS_CHECKERBOARD_SAMPLE_POSITION_WRITE )

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes

DECLARE_INPUT

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_COLOUR_SRT
{
    WRITE_FRAGMENT_COLOUR( vec4( gl_SamplePosition, 0.0, 0.0 ) );
}

#endif

#if defined( D_POSTPROCESS_CHECKERBOARD_SAMPLE_POSITION_READ )

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes

DECLARE_INPUT

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_COLOUR01_SRT
{
    ivec2 lvFragCoords  = ivec2( IN_SCREEN_POSITION.xy );

    vec2  lvPosSmp0     = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lvFragCoords, 0 ).rg;
    vec2  lvPosSmp1     = textureLoadMsF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0MsMap ), lvFragCoords, 1 ).rg;

    WRITE_FRAGMENT_COLOUR0( vec4( lvPosSmp0, 0.0, 0.0 ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lvPosSmp1, 0.0, 0.0 ) );
}

#endif


#if defined( D_POSTPROCESS_CHECKERBOARD_REJECT )

#if defined( D_PLATFORM_PC )
#define D_REJECT_DEBUG
#endif

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes

DECLARE_INPUT

INPUT_SCREEN_POSITION
INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions

bool
ValidDepth(
    float lfDepthTxl,
    float lfDepthAdj,
    float lfThreshold )
{
    float  lfDiff = abs( lfDepthTxl - lfDepthAdj );
    return lfDiff / lfDepthTxl < 0.05 || lfDiff < lfThreshold;
}

#if !defined( D_REJECT_DEBUG )
FRAGMENT_MAIN_T1_SRT( uint )
#else
FRAGMENT_MAIN_T2_SRT( uint, float )
#endif
{
    ivec2 lvFragCoordsCb = ivec2( IN_SCREEN_POSITION.xy );
    ivec2 lvFragCoordsFs = lvFragCoordsCb * 2;

    uint  luMask;
    float lfThreshold;
    vec4  lvDepthsTxl;
    vec4  lvDepthsAdj;

    lfThreshold = 16.0 * lUniforms.mpPerFrame.gClipPlanesVec4.w;
    lvDepthsTxl = textureGatherRed( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS );

    #if defined( D_EVEN )
    lvDepthsAdj[ 0 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2( -1,  1 ) ).r; // left
    lvDepthsAdj[ 1 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2(  0,  2 ) ).r; // down
    lvDepthsAdj[ 2 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2(  2,  0 ) ).r; // right
    lvDepthsAdj[ 3 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2(  1, -1 ) ).r; // up

    luMask  = uint( ValidDepth( lvDepthsTxl[ 0 ], lvDepthsAdj[ 0 ], lfThreshold ) );       // left
    luMask |= uint( ValidDepth( lvDepthsTxl[ 0 ], lvDepthsTxl[ 1 ], lfThreshold ) ) << 1;  // right
    luMask |= uint( ValidDepth( lvDepthsTxl[ 0 ], lvDepthsTxl[ 3 ], lfThreshold ) ) << 2;  // up
    luMask |= uint( ValidDepth( lvDepthsTxl[ 0 ], lvDepthsAdj[ 1 ], lfThreshold ) ) << 3;  // down
    luMask |= uint( ValidDepth( lvDepthsTxl[ 2 ], lvDepthsTxl[ 3 ], lfThreshold ) ) << 4;  // left
    luMask |= uint( ValidDepth( lvDepthsTxl[ 2 ], lvDepthsAdj[ 2 ], lfThreshold ) ) << 5;  // right
    luMask |= uint( ValidDepth( lvDepthsTxl[ 2 ], lvDepthsAdj[ 3 ], lfThreshold ) ) << 6;  // up
    luMask |= uint( ValidDepth( lvDepthsTxl[ 2 ], lvDepthsTxl[ 1 ], lfThreshold ) ) << 7;  // down

    #else
    lvDepthsAdj[ 0 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2( -1,  0 ) ).r; // left
    lvDepthsAdj[ 1 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2(  1,  2 ) ).r; // down
    lvDepthsAdj[ 2 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2(  2,  1 ) ).r; // right
    lvDepthsAdj[ 3 ] = textureLoadOffsetF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvFragCoordsFs, 0, ivec2(  0, -1 ) ).r; // up

    luMask  = uint( ValidDepth( lvDepthsTxl[ 1 ], lvDepthsTxl[ 0 ], lfThreshold ) );       // left
    luMask |= uint( ValidDepth( lvDepthsTxl[ 1 ], lvDepthsAdj[ 2 ], lfThreshold ) ) << 1;  // right
    luMask |= uint( ValidDepth( lvDepthsTxl[ 1 ], lvDepthsTxl[ 2 ], lfThreshold ) ) << 2;  // up
    luMask |= uint( ValidDepth( lvDepthsTxl[ 1 ], lvDepthsAdj[ 1 ], lfThreshold ) ) << 3;  // down
    luMask |= uint( ValidDepth( lvDepthsTxl[ 3 ], lvDepthsAdj[ 0 ], lfThreshold ) ) << 4;  // left
    luMask |= uint( ValidDepth( lvDepthsTxl[ 3 ], lvDepthsTxl[ 2 ], lfThreshold ) ) << 5;  // right
    luMask |= uint( ValidDepth( lvDepthsTxl[ 3 ], lvDepthsAdj[ 3 ], lfThreshold ) ) << 6;  // up
    luMask |= uint( ValidDepth( lvDepthsTxl[ 3 ], lvDepthsTxl[ 0 ], lfThreshold ) ) << 7;  // down
    #endif

    FRAGMENT_OUTPUT_T0 = luMask;

    #if defined( D_REJECT_DEBUG )
    FRAGMENT_OUTPUT_T1 = float( luMask ) / 255.0;
    #endif
}

#endif

//TF_BEGIN
// ------------------------------------------------------------------------------------------------
// D_MSAA_RESOLVE_DEPTH
// ------------------------------------------------------------------------------------------------
#ifdef D_MSAA_RESOLVE
DECLARE_INPUT
#if defined(D_PLATFORM_GLSL)
#define T_STENCIL int
#else
#define T_STENCIL uint
#endif
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT
{
#if defined ( D_PLATFORM_METAL )
	float lfReverseDepth = 1.0f;
	T_STENCIL luStencil = 0;
	ivec2 screen_coords = ivec2(TEX_COORDS * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy);
	for (int i = 0; i < lUniforms.mpPerFrame.gMSAASamples; ++i)
	{
		vec4 lDepthVec4 = texture2DMS(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), screen_coords, i);
		uvec4 lStencilVec4 = texture2DMS(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gStencilBuffer), screen_coords, i);
		lfReverseDepth = min(DecodeDepthFromColour(lDepthVec4), lfReverseDepth);
		luStencil = max(T_STENCIL(lStencilVec4.r), luStencil);
	}

	float lfDepthLinearNorm = ReverseZToLinearDepthNorm(lUniforms.mpPerFrame.gClipPlanesVec4, lfReverseDepth);
	WRITE_FRAGMENT_COLOUR(EncodeDepthToColour(lfDepthLinearNorm));
	WRITE_FRAGMENT_DEPTH(lfReverseDepth);
	WRITE_FRAGMENT_STENCIL(luStencil);
#else
    WRITE_FRAGMENT_COLOUR( EncodeDepthToColour( 0.0f ) );
    WRITE_FRAGMENT_DEPTH( 1.0 );

    #if defined ( D_PLATFORM_XBOXONE ) && !defined( D_COMPUTE )
    WRITE_FRAGMENT_STENCIL( 0 );
    #endif
#endif
}
#endif

// ------------------------------------------------------------------------------------------------
// D_RESOLVE_DEPTH
// ------------------------------------------------------------------------------------------------
#ifdef D_RESOLVE_DEPTH
DECLARE_INPUT
#if defined(D_PLATFORM_GLSL)
#define T_STENCIL int
#else
#define T_STENCIL uint
#endif
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_DEPTH_STENCIL_SRT
{
#if defined ( D_PLATFORM_METAL )
	vec2    lFragCoordsVec2 = TEX_COORDS.xy;
	vec4 lDepthVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lFragCoordsVec2);
	uvec4 lStencilVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gStencilBuffer), lFragCoordsVec2);
	float lfReverseDepth = DecodeDepthFromColour(lDepthVec4);
	T_STENCIL luStencil = T_STENCIL(lStencilVec4.r);

	float lfDepthLinearNorm = ReverseZToLinearDepthNorm(lUniforms.mpPerFrame.gClipPlanesVec4, lfReverseDepth);

	WRITE_FRAGMENT_COLOUR(EncodeDepthToColour(lfDepthLinearNorm));
	WRITE_FRAGMENT_DEPTH(lfReverseDepth);
	WRITE_FRAGMENT_STENCIL(luStencil);
#else
    WRITE_FRAGMENT_COLOUR(EncodeDepthToColour(0.0f));
    WRITE_FRAGMENT_DEPTH(1.0);

    #if defined ( D_PLATFORM_XBOXONE ) && !defined( D_COMPUTE )
    WRITE_FRAGMENT_STENCIL(0);
    #endif
#endif
}
#endif


// =================================================================================================
//
// D_POSTPROCESS_DOWNSAMPLE_DUALFILTER
//
// =================================================================================================

#ifdef D_POSTPROCESS_DOWNSAMPLE_DUALFILTER

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
	// Get half pixel size, expect input texture 2x size of framebuffer
	vec2 lHalfpixel = vec2(lUniforms.mpCustomPerMesh.gBlurParamsVec4.xy) * (lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * 2.0);

	// Center value 1/2
	vec4 lFragCol = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy) * 4.0;

	// Pixel corners 1/8 each
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy - lHalfpixel.xy);
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + lHalfpixel.xy);
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(lHalfpixel.x, -lHalfpixel.y));
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy - vec2(lHalfpixel.x, -lHalfpixel.y));

	lFragCol *= 0.125;

	WRITE_FRAGMENT_COLOUR(lFragCol);
}
#endif

// =================================================================================================
//
// D_POSTPROCESS_UPSAMPLE_DUALFILTER
//
// =================================================================================================

#ifdef D_POSTPROCESS_UPSAMPLE_DUALFILTER

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
	// Get half pixel size, expect input texture 0.5x size of framebuffer
	vec2 lHalfpixel = vec2(lUniforms.mpCustomPerMesh.gBlurParamsVec4.xy) * (lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * 0.5);

	vec4 lFragCol = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(-lHalfpixel.x * 2.0, 0.0)); // left
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(0.0, lHalfpixel.y * 2.0)); //up
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(lHalfpixel.x * 2.0, 0.0)); // right
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(0.0, -lHalfpixel.y * 2.0)); // bottom

	// diagonal cornors
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(-lHalfpixel.x, lHalfpixel.y)) * 2.0;
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(lHalfpixel.x, lHalfpixel.y)) * 2.0;
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(lHalfpixel.x, -lHalfpixel.y)) * 2.0;
	lFragCol += texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), TEX_COORDS.xy + vec2(-lHalfpixel.x, -lHalfpixel.y)) * 2.0;

	lFragCol /= 12.0;

	WRITE_FRAGMENT_COLOUR(lFragCol);
}
#endif
//TF_END