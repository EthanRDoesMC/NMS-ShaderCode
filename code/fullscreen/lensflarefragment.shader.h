////////////////////////////////////////////////////////////////////////////////
///
///     @file       LensFlareFragment.h
///     @author     User
///     @date       
///
///     @brief      LensFlareFragment
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
#include "Common/Common.shader.h"
#include "Fullscreen/PostCommon.h"
#include "Common/Common.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPostProcess.shader.h"


// =================================================================================================
//
// D_LENSFLARE_BRIGHT
//
// =================================================================================================

#ifdef D_LENSFLARE_BRIGHT

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


STATIC_CONST vec3   kLumcoeff       = vec3(0.15,0.25,0.6);
STATIC_CONST float  kfThreshold_0   =  3.00;
STATIC_CONST float  kfThreshold_1   =  5.00;
STATIC_CONST float  kfGain_0        =  0.25;
STATIC_CONST float  kfGain_1        =  0.75;
STATIC_CONST float  kfSunCoeff      =  0.10;
STATIC_CONST float  kfSunStrength   =  1.20;

vec3 
Threshold(
    in vec3  lColour,
    in float lfThreshold,
    in float lfGain )
{
    float lum    = dot(lColour, kLumcoeff);
    float thresh = max((lum-lfThreshold) * lfGain, 0.0);
    return lColour * thresh / ( 1.0 + thresh );
}

#if !defined( D_LENSFLARE_SUN )
FRAGMENT_MAIN_COLOUR01_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{
    vec3 lBrightVec3, lOutBrightVec3;
    lBrightVec3     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), TEX_COORDS.xy, 0.0 ).rgb;
    lBrightVec3     *= lUniforms.mpCustomPerMesh.gThresholdParamsVec4.x;

    lOutBrightVec3  = Threshold( lBrightVec3, kfThreshold_0, kfGain_0 );

#if !defined( D_LENSFLARE_SUN )
    vec3 lSunVec3;
    lSunVec3        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS.xy, 0.0 ).rgb;
    
    lOutBrightVec3  = max( lOutBrightVec3, lSunVec3 );
    WRITE_FRAGMENT_COLOUR0( vec4(lOutBrightVec3, 1.0) );

    float lfSunStrength = length( lSunVec3 ) * kfSunStrength;
    float lfThreshold   = max( kfThreshold_1 - lfSunStrength, 0.0 );
    lOutBrightVec3  = Threshold( lBrightVec3, lfThreshold, kfGain_1 + lfSunStrength );
    WRITE_FRAGMENT_COLOUR1( vec4(lOutBrightVec3, 1.0) );
#else
    float lfOpacity;
    lfOpacity       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), TEX_COORDS.xy, 0.0 ).a;
    lOutBrightVec3 *= ( 1.0 - lfOpacity ) * kfSunCoeff;
    
    WRITE_FRAGMENT_COLOUR( vec4(lOutBrightVec3, 1.0) );
#endif
}

#endif // D_LENSFLARE_BRIGHT

// =================================================================================================
//
// D_LENSFLARE_FEATURE
//
// =================================================================================================

#ifdef D_LENSFLARE_FEATURE

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

#define PI          3.141592653589793
#define NB_GHOSTS   5
#define ANA_COUNT   64

STATIC_CONST vec3 kChromaDistortion = vec3(0.015, 0.035, 0.055);
STATIC_CONST vec3 kChromaAnamorph   = vec3(0.100, 0.075, 1.000);
STATIC_CONST vec3 kFeaturesWeights  = vec3(0.070, 0.850, 0.200);

//-----------------------------------------------------------------------------
//      Functions 

vec2 
flipTexcoords(
    in vec2 texcoords ) 
{
    return -texcoords + 1.0;
}

float 
vignette(
    in vec2 coords )
{
    float dist = distance(coords, vec2(0.5,0.5));
    dist = smoothstep(0.25, 0.45, dist);
    return clamp(dist,0.0,1.0);
}

float 
cart2line( 
    in vec2 texcoords )
{
    texcoords = normalize( texcoords - 0.5 );
    return atan2( texcoords.y, texcoords.x ) * 2.0 / PI;
}

vec2 
star_coords( 
    in vec2  texcoords, 
    in float time )
{
    return vec2( cart2line( texcoords ), mod( time / 100.0, 1.0 ) );
}

vec2 
distort_coords(
    in vec2 texcoords, 
    in vec4 dist_vec,
    in vec2 shear,
    in vec2 scale )
{
    scale      = vec2( 1.0, 1.0 );
    texcoords *= scale;
    dist_vec  *= vec4( 1.0, 0.5, 0.5, 1.0 );

    mat3 rotation;
    rotation[ 0 ]   = vec3( dist_vec.x, dist_vec.y, 0.0 );
    rotation[ 1 ]   = vec3( dist_vec.z, dist_vec.w, 0.0 );
    rotation[ 2 ]   = vec3( 0.0,        0.0,        1.0 ); 

    vec2  rot_coords = MUL( rotation, vec3( ( texcoords - 0.5 * scale ), 1.0 ) ).xy;
    rot_coords       = rot_coords * 0.5 * ( 1.0 / scale ) * 1.85 + 0.5;

    return rot_coords;
}

vec3 
textureDistorted(
    SAMPLER2DARG( tex ),
    in vec2       sample_center,
    in vec2       sample_vector,
    in vec3       distortion )
{
    vec3 col = vec3(0.0, 0.0, 0.0);
    col.r    = texture2DLod( tex, sample_center + sample_vector * distortion.r, 0.0).r;
    col.g    = texture2DLod( tex, sample_center + sample_vector * distortion.g, 0.0).g;
    col.b    = texture2DLod( tex, sample_center + sample_vector * distortion.b, 0.0).b;

    return col;
}

//TF_BEGIN
#if defined(D_FORWARD)
vec3
textureDistorted_frwd(
	SAMPLER2DARG(tex),
	in vec2       sample_center,
	in vec2       sample_vector,
	in vec3       distortion)
{
	vec3 col = vec3(0.0, 0.0, 0.0);
	col.r = texture2DLod(tex, sample_center + sample_vector * distortion.r, 0.0).g;
	col.g = texture2DLod(tex, sample_center + sample_vector * distortion.g, 0.0).g;
	col.b = texture2DLod(tex, sample_center + sample_vector * distortion.b, 0.0).g;

	return col;
}
#endif
//TF_END

#ifdef D_PLATFORM_ORBIS
#pragma argument(unrollallloops)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
#endif

FRAGMENT_MAIN_COLOUR_SRT
{    
    vec2 lTexCoord  = TEX_COORDS.xy;
    vec2 lHaloCoord = distort_coords( lTexCoord, lUniforms.mpCustomPerMesh.gLensHaloDistortionVec4, vec2( 1.2, 1.6 ), vec2( 0.9, 0.9 ) );
    vec2 lDirtCoord = TEX_COORDS.xy;//distort_coords( lTexCoord, lUniforms.mpCustomPerMesh.gLensDirtDistortionVec4, vec2( 1.1, 1.3 ), vec2( 1.00, 1.77 ) );

#if defined(D_PLATFORM_METAL)
	lTexCoord = clamp(lTexCoord, vec2(0.002), vec2(0.998));
#else
    lTexCoord = clamp(lTexCoord, 0.002, 0.998);
#endif

    vec2    image_center    = vec2(0.5,0.5);
    vec2    sample_vector   = (image_center - lTexCoord) * 0.3;
    vec2    halo_vector     = normalize(sample_vector)   * 0.45;
    vec2    star_vector     = star_coords( lTexCoord, lUniforms.mpPerFrame.gfTime / 4.0 );
    float   vignette_val    = vignette(lTexCoord);
    float   tex_dim         = GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLensNoise ) ).x;
    float   rt_res          = lUniforms.mpPerFrame.gFrameBufferSizeVec4.x;
    int     lod             = int( round( tex_dim / rt_res ) );

    vec3    lLensNoise      = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLensNoise     ), star_vector, lod).rgb;
    vec3    lLensDirtGhost  = texture2D(    SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLensDirtGhost ), lDirtCoord  ).rgb;
    //vec3    lLensDirtHalo   = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLensDirtHalo  ), lDirtCoord  ).rgb;
    //vec3    lLensDirtAna    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLensDirtAna   ), lDirtCoord  ).rgb;

    // lens halo
    vec3 halo = float2vec3( 0.0 );
    {
//TF_BEGIN
#if defined(D_FORWARD)
		halo += textureDistorted_frwd(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap),
			lHaloCoord + halo_vector, halo_vector, kChromaDistortion).rgb;
#else
        halo += textureDistorted(   SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap ), 
                                    lHaloCoord + halo_vector, halo_vector, kChromaDistortion ).rgb;
#endif
//TF_END
        halo *= mix( 1.0, lLensNoise.r * lLensNoise.r * 1.5, 0.95 );
        halo *= vignette_val;
    }

    // lens ghosts
    vec3 ghosts = float2vec3( 0.0 );
    {
        for (int i = 0; i < NB_GHOSTS; ++i) 
        {
            vec2 offset = sample_vector * float(i);
//TF_BEGIN
#if defined(D_FORWARD)
			vec3 ghost = textureDistorted_frwd(SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap),
				lTexCoord + offset, offset, kChromaDistortion).rgb;
#else
            vec3 ghost  = textureDistorted( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ),
                                            lTexCoord + offset, offset, kChromaDistortion ).rgb;
#endif
//TF_END
            ghosts     += ghost * 1.0 / ( 0.5 * i + 1.0 );
        }
        ghosts *= dot( sample_vector, sample_vector ) * 16.0;
    }

    // lens anamorph
    //vec3 anamorph = float2vec3( 0.0 );
    //{
    //    float s;
    //    for (int i = -ANA_COUNT / 2; i < ANA_COUNT / 2; ++i) 
    //    {
    //        s = clamp(1.0/abs(float(i)),0.0,1.0);
    //
    //        vec2 lCoords = vec2( lTexCoord.x + float(i)*(1.0 / ANA_COUNT), lTexCoord.y );
    //        if ( abs( lCoords.x - 0.5 ) > 0.5 ) continue;        
    //        anamorph += texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), 
    //                                  lCoords, 0.0).rgb*s;        
    //    }
    //    anamorph *= kChromaAnamorph;
    //}
        
    halo        *= /*( lLensDirtHalo  * 0.5 + 0.7 ) */ kFeaturesWeights.x;
    ghosts      *= ( lLensDirtGhost * 0.5 + 0.7 ) *    kFeaturesWeights.y;
    //anamorph    *= /*( lLensDirtAna   * 0.5 + 0.7 ) */ kFeaturesWeights.z;
    
    vec3 result  = halo + ghosts /*+ anamorph*/;

    WRITE_FRAGMENT_COLOUR( vec4( result, 1.0 ) );
}


#endif // D_LENSFLARE_FEATURE


// =================================================================================================
//
// D_LENSFLARE_COMBINE
//
// =================================================================================================

#ifdef D_LENSFLARE_COMBINE

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

//-----------------------------------------------------------------------------
//      Functions 

// https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting
// slide 142
vec3 FilmicTonemap(vec3 x) {
    float A = 1.00; //  shoulder strength       default: 0.22
    float B = 0.70; //  linear   strength       default: 0.30
    float C = 0.11; //  linear   angle          default: 0.15
    float D = 0.18; //  toe      strength       default: 0.20
    float E = 0.00; //  toe      numerator      default: 0.04
    float F = 0.51; //  toe      denominator    default: 0.30
    
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3
FilmicTonemap( vec3 color, float exp, float w ) 
{
    vec3 curr   = FilmicTonemap( exp * color   );
    vec3 white  = FilmicTonemap( float2vec3(w) );
    return curr / white;
}

vec3
SmootherStep( vec3 x )
{
    x = clamp( x, float2vec3(0.0), float2vec3(1.0) );
    return x * x * x * ( x * ( x * 6.0f - 15.0f ) + 10.0f );
}

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


//----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{	
	vec2    lTexCoords    = TEX_COORDS.xy;
    vec3    lFragCol      = texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lTexCoords, 0.0).xyz;
    vec3    lBloomCol     = texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lTexCoords, 0.0).xyz;
    vec3    lLensFlareCol = texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lTexCoords, 0.0).xyz;
    float   lfDepth       = texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lTexCoords, 0.0).x;

    // Deband... but don't deband anything other than the background sky/space
    lFragCol.rgb = DebandFilter(DEREF_PTR(lUniforms.mpPerFrame), lFragCol.rgb, lTexCoords, lfDepth, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer4Map), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh,gBuffer1Map));

    float   lfExposure    = Exposure(SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ));    

    lFragCol             += lBloomCol * lfExposure;
    lFragCol             += lLensFlareCol;

    //lFragCol              = mix( FilmicTonemap( lFragCol, 1.0, 4.0 ), SmootherStep( lFragCol ) * SmootherStep( lFragCol ), 0.25 );    
    #if !defined(D_NO_GAMMA_CORRECT)
    lFragCol              = GammaCorrectOutput(lFragCol);
    #endif

    WRITE_FRAGMENT_COLOUR( vec4(lFragCol, 1.0) );
}
#endif // D_LENSFLARE_COMBINE


// =================================================================================================
//
// D_LENSFLARE_COMBINE_OLD
//
// =================================================================================================

#ifdef D_LENSFLARE_COMBINE_OLD

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


//----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{	
    vec3 lFragCol;
    vec2 newCoords = TEX_COORDS.xy;

	vec3 lLensDirt = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLensDirtGhost), newCoords, 0.0).xyz;
    vec3 lLensFlareCol = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBuffer4Map), newCoords, 0.0).xyz;
    lLensFlareCol *= (lLensDirt* (lUniforms.mpCustomPerMesh.gThresholdParamsVec4.w + 0.2)) + (1.0 - lUniforms.mpCustomPerMesh.gThresholdParamsVec4.w);

	lFragCol = (texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), newCoords, 0.0).xyz);
    lFragCol +=                  (texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer2Map ), newCoords, 0.0).xyz);
    lFragCol += GammaCorrectInput(texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer3Map ), newCoords, 0.0).xyz);
    lFragCol += GammaCorrectInput(lLensFlareCol);
    lFragCol  = GammaCorrectOutput(lFragCol);

    if ( newCoords.x > 0.5 ) discard;    

    WRITE_FRAGMENT_COLOUR( vec4(lFragCol, 1.0) );
}

#endif // D_LENSFLARE_COMBINE


// =================================================================================================
//
// D_LENSFLARE_ANAMORPH
//
// =================================================================================================

#ifdef D_LENSFLARE_ANAMORPH

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

float rand(float n) { return fract(sin(n) * 43758.5453123); }

float noise(float p) {
    float fl = floor(p);
    float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

// This shader takes a shit-ton of samples, which is kind of horrendous
// the render target is at 1/16 res though, so perf is reasonable
// still... it's all a bit yuck :(
STATIC_CONST float gaWeights[ 325 ] = 
{
    0.018650, 0.018701, 0.018750, 0.018701, 0.018650, 0.019656, 0.019714,  0.019771, 0.019714, 0.019656,
    0.020756, 0.020823, 0.020889, 0.020823, 0.020756, 0.021961, 0.022040,  0.022117, 0.022040, 0.021961,
    0.023288, 0.023380, 0.023469, 0.023380, 0.023288, 0.024752, 0.024860,  0.024966, 0.024860, 0.024752,
    0.026375, 0.026502, 0.026627, 0.026502, 0.026375, 0.028179, 0.028331,  0.028480, 0.028331, 0.028179,
    0.030195, 0.030376, 0.030556, 0.030376, 0.030195, 0.032457, 0.032675,  0.032892, 0.032675, 0.032457,
    0.035007, 0.035273, 0.035537, 0.035273, 0.035007, 0.037897, 0.038223,  0.038549, 0.038223, 0.037897,
    0.041190, 0.041595, 0.042000, 0.041595, 0.041190, 0.044968, 0.045475,  0.045983, 0.045475, 0.044968,
    0.049328, 0.049970, 0.050617, 0.049970, 0.049328, 0.054397, 0.055220,  0.056055, 0.055220, 0.054397,
    0.060335, 0.061407, 0.062500, 0.061407, 0.060335, 0.067351, 0.068767,  0.070222, 0.068767, 0.067351,
    0.075714, 0.077616, 0.079592, 0.077616, 0.075714, 0.085777, 0.088384,  0.091124, 0.088384, 0.085777,
    0.098010, 0.101660, 0.105556, 0.101660, 0.098010, 0.113032, 0.118266,  0.123967, 0.118266, 0.113032,
    0.131663, 0.139373, 0.148000, 0.139373, 0.131663, 0.154965, 0.166665,  0.180247, 0.166665, 0.154965,
    0.184254, 0.202585, 0.225000, 0.202585, 0.184254, 0.220957, 0.250650,  0.289796, 0.250650, 0.220957,
    0.266078, 0.315683, 0.388889, 0.315683, 0.266078, 0.318636, 0.403250,  0.552000, 0.403250, 0.318636,
    0.372196, 0.515770, 0.850000, 0.515770, 0.372196, 0.409482, 0.638686,  1.488889, 0.638686, 0.409482,
    0.400277, 0.710711, 3.300000, 0.710711, 0.400277, 0.311543, 0.608410,  2.000000, 0.608410, 0.311543,
    0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.311543, 0.608410,  2.000000, 0.608410, 0.311543,
    0.400277, 0.710711, 3.300000, 0.710711, 0.400277, 0.409482, 0.638686,  1.488889, 0.638686, 0.409482,
    0.372196, 0.515770, 0.850000, 0.515770, 0.372196, 0.318636, 0.403250,  0.552000, 0.403250, 0.318636,
    0.266078, 0.315683, 0.388889, 0.315683, 0.266078, 0.220957, 0.250650,  0.289796, 0.250650, 0.220957,
    0.184254, 0.202585, 0.225000, 0.202585, 0.184254, 0.154965, 0.166665,  0.180247, 0.166665, 0.154965,
    0.131663, 0.139373, 0.148000, 0.139373, 0.131663, 0.113032, 0.118266,  0.123967, 0.118266, 0.113032,
    0.098010, 0.101660, 0.105556, 0.101660, 0.098010, 0.085777, 0.088384,  0.091124, 0.088384, 0.085777,
    0.075714, 0.077616, 0.079592, 0.077616, 0.075714, 0.067351, 0.068767,  0.070222, 0.068767, 0.067351,
    0.060335, 0.061407, 0.062500, 0.061407, 0.060335, 0.054397, 0.055220,  0.056055, 0.055220, 0.054397,
    0.049328, 0.049970, 0.050617, 0.049970, 0.049328, 0.044968, 0.045475,  0.045983, 0.045475, 0.044968,
    0.041190, 0.041595, 0.042000, 0.041595, 0.041190, 0.037897, 0.038223,  0.038549, 0.038223, 0.037897,
    0.035007, 0.035273, 0.035537, 0.035273, 0.035007, 0.032457, 0.032675,  0.032892, 0.032675, 0.032457,
    0.030195, 0.030376, 0.030556, 0.030376, 0.030195, 0.028179, 0.028331,  0.028480, 0.028331, 0.028179,
    0.026375, 0.026502, 0.026627, 0.026502, 0.026375, 0.024752, 0.024860,  0.024966, 0.024860, 0.024752,
    0.023288, 0.023380, 0.023469, 0.023380, 0.023288, 0.021961, 0.022040,  0.022117, 0.022040, 0.021961,
    0.020756, 0.020823, 0.020889, 0.020823, 0.020756, 0.019656, 0.019714,  0.019771, 0.019714, 0.019656,
    0.018650, 0.018701, 0.018750, 0.018701, 0.018650 
};


//----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{	    
    int     tt          = -1;
    float   lfWeight    = 0.0;
    float   lfWeightSum = 0.0;
    float   lfNoise     = 0.0;
    float   lfTime      = lUniforms.mpPerFrame.gfTime;
    vec2    lCoords     = TEX_COORDS.xy;
    float   lfStepScale = min( lUniforms.mpPerFrame.gFrameBufferSizeVec4.y / 60.0 * 16.0, 24.0 );
    
#if !defined ( D_PLATFORM_SWITCH )
    vec2    lStepSize = 1.0 / GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) );
#else
    vec2    lStepSize = 1.0 / vec2( GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) ) );
#endif

    vec3    lFragCol    = float2vec3( 0.0 );  
    vec3    lCurrCol    = float2vec3( 0.0 );

    lStepSize          *= vec2( lfStepScale, -lfStepScale / 8.0 - 1.0 );
    
    for ( int ii = -32; ii <= 32; ii += 1 )
    {                
        float invsqrt_ii_x2 = (ii !=0) ? invsqrt(abs(ii) * 2.0) : 0.0;
        for ( int jj = -8; jj <= 8; jj += 4 )
        {               
            float lfX    = float(ii);
            float lfY    = float(jj) * invsqrt_ii_x2;
            lCoords      = vec2( lfX - sign( ii ), lfY ) * lStepSize;
            lCoords      = TEX_COORDS.xy + lCoords;
            ++tt;            
            if ( abs( lCoords.x - 0.5 ) > 0.5 ) continue;        
            if ( abs( lCoords.y - 0.5 ) > 0.5 ) continue;
            lfNoise      = mix( 1.0, noise( lfTime * ( abs( lfX * lfY ) + 1.0 ) * 0.1 + lfX ), 0.5 );
            lfWeight     = gaWeights[ tt ]; 
            lfWeight     = min( 0.875, lfWeight );
            lfWeight    *= lfNoise;
            lCurrCol     = texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lCoords, 0.0).xyz;
            #if defined(D_BROADCAST_G) // broadcast single channel bloom
            lCurrCol = lCurrCol.ggg;
            #endif
            lFragCol    += lCurrCol * lfWeight;
            lfWeightSum += lfWeight;
        }
    }

    lfWeightSum = min( lfWeightSum * 0.33, 3.0 );
    lFragCol   /= lfWeightSum;

    WRITE_FRAGMENT_COLOUR( vec4( lFragCol, 1.0) );
}

#endif // D_LENSFLARE_ANAMORPH


// =================================================================================================
//
// D_LENSFLARE_ANAMORPH_RESOLVE
//
// =================================================================================================

#ifdef D_LENSFLARE_ANAMORPH_RESOLVE

//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float kfTonemapScale   = 16.0;
STATIC_CONST vec3  kChromaAnamorph  = vec3(0.22, 0.33, 0.99);

//-----------------------------------------------------------------------------
//      Typedefs and Classes 
 
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

//----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{	            
    vec2    lCoords     = TEX_COORDS.xy;    
    
#if !defined ( D_PLATFORM_SWITCH )
    vec2    lStepSize = 1.0 / GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) );
#else
    vec2    lStepSize = 1.0 / vec2( GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) ) );
#endif
    
    vec3    lFragCol    = texture2DLod(SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lCoords, 0.0).xyz;  
    
    lFragCol  *= kChromaAnamorph;
    lFragCol   = TonemapKodak( lFragCol );
    lFragCol  /= TonemapKodak( float2vec3( kfTonemapScale ) );    

    // This will cause hue shifting, but that's ok :)
    lFragCol   = lFragCol * lFragCol;


    WRITE_FRAGMENT_COLOUR( vec4(lFragCol, 1.0) );
}

#endif // D_LENSFLARE_ANAMORPH_RESOLVE