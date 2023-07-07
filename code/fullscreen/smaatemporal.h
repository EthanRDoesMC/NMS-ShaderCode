////////////////////////////////////////////////////////////////////////////////
///
///     @file       SMAATemporal.h
///     @author     strgiu
///     @date       
///
///     @brief      SMAATemporal
///
///     Copyright (c) 2020 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SMAA Defines
#include "Fullscreen/SMAADefines.h"

/** Techniques based on:
*       - Hybrid Reconstruction Antialiasing, Michal Drobot, 
*         GPU Pro 360 Guide to Rendering, pag. 433
*       - Filmic SMAA Sharp Morphological and Temporal Antialiasing,
*         Jorge Jimenez, SIGGRAPH 2016
*/

//-----------------------------------------------------------------------------
// Functions

vec2
DecodeMotion(
    in vec2 lBuffer )
{
    return ( lBuffer - 0.5 ) * 2.0;
}

void 
GatherBlock3x3RGB(
    SMAATexture2DArg( paramTex ),
    vec2              texcoords, 
    inout vec3        blockSamples[9] )
{    
    // Sample the "rounded" 3x3 neighbourhood of the current solution
    blockSamples[ 0 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1, -1) ).rgb;
    blockSamples[ 1 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0, -1) ).rgb;
    blockSamples[ 2 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1, -1) ).rgb;
    blockSamples[ 3 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1,  0) ).rgb;    
    blockSamples[ 4 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0,  0) ).rgb;
    blockSamples[ 5 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1,  0) ).rgb;
    blockSamples[ 6 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1,  1) ).rgb;
    blockSamples[ 7 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0,  1) ).rgb;
    blockSamples[ 8 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1,  1) ).rgb;
}

void 
GatherBlock3x3RG(
    SMAATexture2DArg( paramTex ),
    vec2              texcoords, 
    inout vec2        blockSamples[9] )
{    
    // Sample the "rounded" 3x3 neighbourhood of the current solution
    blockSamples[ 0 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1, -1) ).rg;
    blockSamples[ 1 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0, -1) ).rg;
    blockSamples[ 2 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1, -1) ).rg;
    blockSamples[ 3 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1,  0) ).rg;    
    blockSamples[ 4 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0,  0) ).rg;
    blockSamples[ 5 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1,  0) ).rg;
    blockSamples[ 6 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1,  1) ).rg;
    blockSamples[ 7 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0,  1) ).rg;
    blockSamples[ 8 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1,  1) ).rg;
}

void 
GatherBlock3x3R(
    SMAATexture2DArg( paramTex ),
    vec2              texcoords, 
    inout float       blockSamples[9] )
{    
    // Sample the "rounded" 3x3 neighbourhood of the current solution
    blockSamples[ 0 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1, -1) ).r;
    blockSamples[ 1 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0, -1) ).r;
    blockSamples[ 2 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1, -1) ).r;
    blockSamples[ 3 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1,  0) ).r;    
    blockSamples[ 4 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0,  0) ).r;
    blockSamples[ 5 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1,  0) ).r;
    blockSamples[ 6 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2(-1,  1) ).r;
    blockSamples[ 7 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 0,  1) ).r;
    blockSamples[ 8 ] = SMAASampleLevelZeroOffset( paramTex, texcoords, ivec2( 1,  1) ).r;
}


vec3 
SumAbsDiffVec3(
    in vec3 blockSamplesA[9],
    in vec3 blockSamplesB[9]
)
{
    vec3 sad = float2vec3( 0.0 );

    for ( int ii = 0; ii < 9; ++ii )    
        sad += abs( blockSamplesA[ ii ] - blockSamplesB[ ii ] );

    return sad;
}

vec2
SumAbsDiffVec2(
    in vec2 blockSamplesA[9],
    in vec2 blockSamplesB[9]
)
{
    vec2 sad = float2vec2( 0.0 );

    for ( int ii = 0; ii < 9; ++ii )    
        sad += abs( blockSamplesA[ ii ] - blockSamplesB[ ii ] );    

    return sad;
}

float
SumAbsDiffFloat(
    in float blockSamplesA[9],
    in float blockSamplesB[9]
)
{
    float sad = 0.0;

    for ( int ii = 0; ii < 9; ++ii )    
        sad += abs( blockSamplesA[ ii ] - blockSamplesB[ ii ] );    

    return sad;
}

void 
MinMaxAvgBlock3x3(
    in  vec3 blockSamples[9],
    out vec3 minVal,
    out vec3 maxVal,
    out vec3 avgVal
)
{
    minVal = float2vec3( 1.0e16 );
    maxVal = float2vec3( 0.0 );
    avgVal = float2vec3( 0.0 );

    // avgVal
    for ( int ii = 0; ii < 9; ++ii )
    {
        minVal  = min( blockSamples[ ii ], minVal );
        maxVal  = max( blockSamples[ ii ], maxVal );
        avgVal += blockSamples[ ii ];
    }
    avgVal /= 9.0;
}

vec3 
HistoryClip(
    vec3 history, 
    vec3 target, 
    vec3 neighborMin, 
    vec3 neighborMax)
{
	vec3  boxMin = neighborMin;
	vec3  boxMax = neighborMax;
          
	vec3  rayOrigin = history;
	vec3  rayDir = target - history;
	rayDir = dot( rayDir, rayDir ) < 1.0/65536.0 ? float2vec3(1.0/65536.0) : rayDir;
	vec3  invRayDir = 1.0 / rayDir;

	vec3  minIntersect   = (boxMin - rayOrigin) * invRayDir;
	vec3  maxIntersect   = (boxMax - rayOrigin) * invRayDir;
	vec3  enterIntersect = min( minIntersect, maxIntersect );
	float factor         = max( enterIntersect.x, max( enterIntersect.y, enterIntersect.z ) );

    return mix( history, target, factor );
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

float
ColourCoherence(
    vec3 colourSAD )
{
    float colourFactorSAD = dot(colourSAD, vec3(0.2126, 0.7152, 0.0722));
    return 1.0 - saturate( colourFactorSAD / SMAA_COLOUR_COHERENCE_FACTOR );
}

float
MotionCoherence(
    vec2 motionSAD )
{
    float motionFactorSAD = motionSAD.x + motionSAD.y;
    return 1.0 - saturate( motionFactorSAD / SMAA_MOTION_COHERENCE_FACTOR );
}

float
DepthLCoherence(
    float depthLSAD )
{
    return 1.0 - saturate( step( 0.25, depthLSAD / SMAA_DEPTHL_COHERENCE_FACTOR ) );
}

//-----------------------------------------------------------------------------
// Temporal Supersampling

vec4 
SMAATemporalSupersamplingPS(
    vec2 texcoord,
    SMAATexture2DArg( colourTexFrame0 ),
    SMAATexture2DArg( colourTexFrame1 ),
    SMAATexture2DArg( colourTexFrame2 ),
    SMAATexture2DArg( colourTexFrame3 ),
    SMAATexture2DArg( colourTexFrame4 ),
    SMAATexture2DArg( motionTexFrame0 ),
    SMAATexture2DArg( motionTexFrame1 ),
    SMAATexture2DArg( motionTexFrame2 ),
    SMAATexture2DArg( motionTexFrame3 ),
    SMAATexture2DArg( depthLTexFrame0 ),
    SMAATexture2DArg( depthLTexFrame4 )
) 
{
    // assumes jitter cycles every 4 frames

    vec3  colourSamplesFrame0[9];
    vec3  colourSamplesFrame4[9];
          
    vec2  motionSamplesFrame0[9];
    vec2  motionSamplesFrame3[9];

    float depthFSamplesFrame0[9];
    float depthFSamplesFrame4[9];

    float colourCoherence,  motionCoherence,    depthLCoherence;

    float depthLSAD;

    vec2  motionSAD;
    vec2  motionSamples[4];

    vec3  colourSAD;
    vec3  colourMin,        colourMax,              colourAvg;
    vec3  colourSamples[5], colourSamplesClip[4],   colourFinal, colourFinalClip;

    vec2  texcoordFrame[5];

    texcoordFrame[0]    = texcoord;

    GatherBlock3x3R(   SMAATexture2DParam( depthLTexFrame0 ), texcoordFrame[0], depthFSamplesFrame0 );
    GatherBlock3x3RG(  SMAATexture2DParam( motionTexFrame0 ), texcoordFrame[0], motionSamplesFrame0 );
    GatherBlock3x3RGB( SMAATexture2DParam( colourTexFrame0 ), texcoordFrame[0], colourSamplesFrame0 );

    motionSamples[0]    = round( DecodeMotion( motionSamplesFrame0[4] ) * GetTexResolution( gBufferMap ) ) / GetTexResolution( gBufferMap );
    texcoordFrame[1]    = texcoordFrame[0] + motionSamples[0];

    motionSamples[1]    = round( DecodeMotion( SMAASample( motionTexFrame1, texcoordFrame[1] ).rg ) * GetTexResolution( gBufferMap ) ) / GetTexResolution( gBufferMap );
    texcoordFrame[2]    = texcoordFrame[1] + motionSamples[1];

    motionSamples[2]    = round( DecodeMotion( SMAASample( motionTexFrame2, texcoordFrame[2] ).rg ) * GetTexResolution( gBufferMap ) ) / GetTexResolution( gBufferMap );
    texcoordFrame[3]    = texcoordFrame[2] + motionSamples[2];

    motionSamples[3]    = round( DecodeMotion( SMAASample( motionTexFrame3, texcoordFrame[3] ).rg ) * GetTexResolution( gBufferMap ) ) / GetTexResolution( gBufferMap );
    texcoordFrame[4]    = texcoordFrame[3] + motionSamples[3];

    GatherBlock3x3R(   SMAATexture2DParam( depthLTexFrame4 ), texcoordFrame[4],  depthFSamplesFrame4 );
    GatherBlock3x3RG(  SMAATexture2DParam( motionTexFrame3 ), texcoordFrame[3],  motionSamplesFrame3 );
    GatherBlock3x3RGB( SMAATexture2DParam( colourTexFrame4 ), texcoordFrame[4],  colourSamplesFrame4 );

    colourSamples[0]    = colourSamplesFrame0[4];
    colourSamples[1]    = SMAASample( colourTexFrame1, texcoordFrame[1] ).rgb;
    colourSamples[2]    = SMAASample( colourTexFrame2, texcoordFrame[2] ).rgb;
    colourSamples[3]    = SMAASample( colourTexFrame3, texcoordFrame[3] ).rgb;
    colourSamples[4]    = colourSamplesFrame4[4];

    colourSAD  = SumAbsDiffVec3(  colourSamplesFrame0, colourSamplesFrame4 );
    motionSAD  = SumAbsDiffVec2(  motionSamplesFrame0, motionSamplesFrame3 );
    depthLSAD  = SumAbsDiffFloat( depthFSamplesFrame0, depthFSamplesFrame4 );

    motionSAD += abs( motionSamples[0] - motionSamples[1] );
    motionSAD += abs( motionSamples[0] - motionSamples[2] );
    motionSAD += abs( motionSamples[1] - motionSamples[2] );
    motionSAD += abs( motionSamples[1] - motionSamples[3] );
    motionSAD += abs( motionSamples[2] - motionSamples[3] );

    MinMaxAvgBlock3x3( colourSamplesFrame0, colourMin, colourMax, colourAvg );
        
    colourCoherence   = ColourCoherence( colourSAD );
    motionCoherence   = MotionCoherence( motionSAD );
    depthLCoherence   = DepthLCoherence( depthLSAD );

    colourSamplesClip[0] = colourSamples[0];
    colourSamplesClip[1] = clip_aabb( colourSamples[1], colourMin, colourMax );
    colourSamplesClip[2] = clip_aabb( colourSamples[2], colourMin, colourMax );
    colourSamplesClip[3] = clip_aabb( colourSamples[3], colourMin, colourMax );
    
    colourSamples[1]     = mix( colourSamplesClip[1], colourSamples[1], colourCoherence * depthLCoherence );
    colourSamples[2]     = mix( colourSamplesClip[2], colourSamples[2], colourCoherence * depthLCoherence );
    colourSamples[3]     = mix( colourSamplesClip[3], colourSamples[3], colourCoherence * depthLCoherence );

    //colourFinal       = mix( colourCurr,        colourPrev,    0.5 * motionCoherence )


    colourFinal       = ( colourSamples[0] + colourSamples[1] + colourSamples[2] + colourSamples[3]) / 4.0;
    colourFinalClip   = clip_aabb( colourFinal, colourMin, colourMax );
    colourFinal       = mix( colourSamples[0], colourFinal, 0.0 );

    return vec4( colourFinal, 1.0 );

    //return vec4( float2vec3( depthLCoherence * colourCoherence * motionCoherence ), 1.0 );
    //return vec4( float2vec3( depthLCoherence * colourCoherence ), 1.0 );
    //return vec4( float2vec3( 1.0 - depthLCoherence ), 1.0 );
    //return vec4( float2vec3( dot( texcoordFrame[4] * GetTexResolution(gBufferMap)  - texcoordFrame[0] * GetTexResolution(gBufferMap), texcoordFrame[4] * GetTexResolution(gBufferMap) - texcoordFrame[0] * GetTexResolution(gBufferMap)) * 8.0 ), 1.0 );
    //return vec4( float2vec3( dot(  float2vec2( 0.0 ) - motionSamples[0] * GetTexResolution(gBufferMap), float2vec2( 0.0 ) - motionSamples[0] * GetTexResolution(gBufferMap)) * 64.0 ), 1.0 );
}

//-----------------------------------------------------------------------------
// Temporal Filtering

vec4 
SMAATemporalFilteringPS(
    vec2 texcoord,
    vec2 dejitter,
    SMAATexture2DArg( supersampledTex ),
    SMAATexture2DArg( historyTex      ),
    SMAATexture2DArg( motionTexCurr   ),
    SMAATexture2DArg( motionTexPrev   ),
    SMAATexture2DArg( disocclusionTex )   
) 
{
    return float2vec4(0.0);
}