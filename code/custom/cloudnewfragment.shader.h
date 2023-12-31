////////////////////////////////////////////////////////////////////////////////
///
///     @file       CloudFragment.h
///     @author     User
///     @date       
///
///     @brief      CloudFragment
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 

#define D_USE_NOISETEXTURE
#define D_NORMALISED_NOISE

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPostProcess.shader.h"
#include "Custom/CloudCommon.h"

#include "Common/CommonPlanet.shader.h"

#ifdef D_PLATFORM_ORBIS
#pragma argument(unrollallloops)
#pragma argument (O4; fastmath; scheduler=minpressure)
//#pragma argument (targetoccupancy_atallcosts=6)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
#pragma optionNV(fastmath on)
#endif

#if defined( D_PLATFORM_SWITCH )
#define D_USE_WORLEY_CLOUD
#endif

#if defined ( D_PLATFORM_SWITCH ) || defined(D_PLATFORM_METAL)
#define CLOUDCOLOUR1    half
#define CLOUDCOLOUR3    half3
#define CLOUDCOLOUR4    half4
#else
#define CLOUDCOLOUR1    float
#define CLOUDCOLOUR3    vec3
#define CLOUDCOLOUR4    vec4
#endif

//-----------------------------------------------------------------------------
//      Global Data




//-----------------------------------------------------------------------------
//      Typedefs and Classes 

#define PI 3.141592653589793
#define CLOUD_MAX_STEP          20
#define CLOUD_MIN_STEP          ( CLOUD_MAX_STEP * 0.2  )
#define WEIGHT_FLOOR            0.003
#define VULKAN_DEPTH_MIN        2.00001e-8
#define TEMPORAL_GRID_WIDTH     4

#if defined ( D_PLATFORM_SWITCH )
#define D_ENABLE_ROUGH_RENDER
#endif

#if defined(D_PLATFORM_METAL)
#define D_USE_CLOUD_MACROS
#endif

//-----------------------------------------------------------------------------
//      Functions


//float kfHackDebugScreenSide = 0.0;


vec4
DebugColour(
    float lfValue,
    float lfMin,
    float lfMax )
{
    float lfUnitValue;
    //float lfAlpha;
    //vec4  lColour;

    lfUnitValue = 1.0 - saturate( ( lfValue - lfMin ) / ( lfMax - lfMin ) );
    //lfAlpha     = 1.0;


    vec3 r = lfUnitValue * 2.1 - vec3( 1.8, 1.14, 0.3 );
    return vec4( 1.0 - r * r, 1.0 );

    /*
    if ( lfUnitValue < 0.0 )
    {
        lColour = vec4( 0.0, 0.0, 0.0, 1.0 );
    }
    else
    if ( lfUnitValue < 0.125 )
    {
        lfAlpha = (lfUnitValue - 0.0) / 0.125;
        lColour = vec4( 1.0, 0.0, 0.0, 1.0 );
    }
    else
    if ( lfUnitValue < 0.25 )
    {
        lfAlpha = (lfUnitValue - 0.125) / 0.125;
        lColour = vec4( 1.0, 0.5, 0.0, 1.0 );
    }
    else
    if ( lfUnitValue < 0.375 )
    {
        lfAlpha = (lfUnitValue - 0.25) / 0.125;
        lColour = vec4( 1.0, 1.0, 0.0, 1.0 );
    }
    else
    if ( lfUnitValue < 0.5 )
    {
        lfAlpha = (lfUnitValue - 0.375) / 0.125;
        lColour = vec4( 0.0, 0.5, 0.0, 1.0);
    }
    else
    if ( lfUnitValue < 0.625 )
    {
        lfAlpha = (lfUnitValue - 0.5) / 0.125;
        lColour = vec4( 0.5, 1.0, 0.5, 1.0);
    }
    else
    if ( lfUnitValue < 0.75 )
    {
        lfAlpha = (lfUnitValue - 0.625) / 0.125;
        lColour = vec4( 0.0, 1.0, 1.0, 1.0);
    }
    else
    if ( lfUnitValue < 0.875 )
    {
        lfAlpha = (lfUnitValue - 0.75) / 0.125;
        lColour = vec4( 0.0, 0.0, 1.0, 1.0);
    }
    else
    if ( lfUnitValue <= 1.0 )
    {
        lfAlpha = (lfUnitValue - 0.875) / 0.125;
        lColour = vec4( 1.0, 0.0, 1.0, 1.0);
    }
    else
    {
        lColour = vec4( 1.0, 1.0, 1.0, 1.0);
    }

    lColour.xyz *= (lfAlpha * 0.5) + 0.5;
    return lColour;
    */
}


#if !defined(D_USE_CLOUD_MACROS)
vec4
GetTriPlanarMap(
    in vec3  lBlendWeights,
    in vec3  lWorldPositionVec3,
    SAMPLER2DARG( lTexture ) )
{
    vec4 lColourVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );

    if ( lBlendWeights.x > WEIGHT_FLOOR )
    {
        vec2  lCoord1Vec2 = lWorldPositionVec3.yz;
        lColourVec4 += texture2DLod( lTexture, lCoord1Vec2, 0 ) * lBlendWeights.x;
    }

    if ( lBlendWeights.y > WEIGHT_FLOOR )
    {
        vec2  lCoord2Vec2 = lWorldPositionVec3.zx;
        lColourVec4 += texture2DLod( lTexture, lCoord2Vec2, 0 ) * lBlendWeights.y;
    }

    if ( lBlendWeights.z > WEIGHT_FLOOR )
    {
        vec2  lCoord3Vec2 = lWorldPositionVec3.xy;
        lColourVec4 += texture2DLod( lTexture, lCoord3Vec2, 0 ) * lBlendWeights.z;
    }

    return lColourVec4;
}
#else
#define GetTriPlanarMap( lBlendWeights, lWorldPositionVec3, lTexture ) \
vec4( \
    texture2DLod(lTexture, (lWorldPositionVec3).yz, 0) * lBlendWeights.x + \
    texture2DLod(lTexture, (lWorldPositionVec3).zx, 0) * lBlendWeights.y + \
    texture2DLod(lTexture, (lWorldPositionVec3).xy, 0) * lBlendWeights.z \
)
#endif

#if !defined(D_USE_CLOUD_MACROS)
vec2
GetTriPlanarMapGB(
    in vec3  lBlendWeights,
    in vec3  lWorldPositionVec3,
    SAMPLER2DARG( lTexture ) )
{
    vec2 lColourVec2 = vec2( 0.0, 0.0 );

    if ( lBlendWeights.x > WEIGHT_FLOOR )
    {
        vec2  lCoord1Vec2 = lWorldPositionVec3.yz;
        lColourVec2 += texture2DLod( lTexture, lCoord1Vec2, 0 ).gb * lBlendWeights.x;
    }

    if ( lBlendWeights.y > WEIGHT_FLOOR )
    {
        vec2  lCoord2Vec2 = lWorldPositionVec3.zx;
        lColourVec2 += texture2DLod( lTexture, lCoord2Vec2, 0 ).gb * lBlendWeights.y;
    }

    if ( lBlendWeights.z > WEIGHT_FLOOR )
    {
        vec2 lCoord3Vec2 = lWorldPositionVec3.xy;
        lColourVec2 += texture2DLod( lTexture, lCoord3Vec2, 0 ).gb * lBlendWeights.z;
    }

    return lColourVec2;
}
#else
#define GetTriPlanarMapGB( lBlendWeights, lWorldPositionVec3, lTexture ) \
vec2( \
    texture2DLod( lTexture, (lWorldPositionVec3).yz, 0).gb * lBlendWeights.x + \
    texture2DLod( lTexture, (lWorldPositionVec3).zx, 0).gb * lBlendWeights.y + \
    texture2DLod( lTexture, (lWorldPositionVec3).xy, 0).gb * lBlendWeights.z \
)
#endif
/*
vec4
GetTriPlanarMapCheap(
    in vec3  lBlendWeights,
    in vec3  lWorldPositionVec3,
    SAMPLER2DARG( lTexture ) )
{
    vec2  lCoordVec2 = vec2( 0.0, 0.0 );


    if ( lBlendWeights.x > 0.1 )
    {
        lCoordVec2 = lWorldPositionVec3.yz;
    }
    else
    if ( lBlendWeights.y > 0.1 )
    {
        lCoordVec2 = lWorldPositionVec3.zx;
    }
    else
    if ( lBlendWeights.z > 0.1 )
    {
        lCoordVec2 = lWorldPositionVec3.xy;
    }
    else
    {
        vec2  lCoord1Vec2 = lWorldPositionVec3.yz;
        vec2  lCoord2Vec2 = lWorldPositionVec3.zx;
        vec2  lCoord3Vec2 = lWorldPositionVec3.xy;

        vec4 lColour1Vec4 = texture2DLod( lTexture, lCoord1Vec2, 0.0 );
        vec4 lColour2Vec4 = texture2DLod( lTexture, lCoord2Vec2, 0.0 );
        vec4 lColour3Vec4 = texture2DLod( lTexture, lCoord3Vec2, 0.0 );

        return (lColour1Vec4 * lBlendWeights.x + lColour2Vec4 * lBlendWeights.y + lColour3Vec4 * lBlendWeights.z);
    }

    return texture2DLod(lTexture, lCoordVec2, 0.0);
    //return vec4(lWorldPositionVec3,0.0);
    //return vec4( 0.0, 0.0, 0.0, 0.0);
}
*/

#if !defined(D_USE_CLOUD_MACROS)
vec3
GetWindDirection(
    in vec3  lBlendWeights,
    in vec2  lWindVec2 )
{
    vec2  lWindDirectionVec3;

    vec3  lWindDirectionXVec3 = vec3( 0.5, lWindVec2.y, lWindVec2.x );
    vec3  lWindDirectionYVec3 = vec3( lWindVec2.y, 0.5, lWindVec2.x );
    vec3  lWindDirectionZVec3 = vec3( lWindVec2.x, lWindVec2.y, 0.5 );

    return ( lWindDirectionXVec3 * lBlendWeights.x + lWindDirectionYVec3 * lBlendWeights.y + lWindDirectionZVec3 * lBlendWeights.z );
}
#else
#define GetWindDirection( lBlendWeights, lWindVec2 ) \
vec3( \
    vec3(0.5, lWindVec2.y, lWindVec2.x) * lBlendWeights.x + \
    vec3(lWindVec2.y, 0.5, lWindVec2.x) * lBlendWeights.y + \
    vec3(lWindVec2.x, lWindVec2.y, 0.5) * lBlendWeights.z \
)
#endif

vec3
InternalRaySphereIntersect(
    in float sphereRadius,
    in vec3  origin,
    in vec3  rayDirection )
{
    float a0 = sphereRadius * sphereRadius - dot( origin, origin );
    float a1 = dot( origin, rayDirection );
    float result = sqrt( a1 * a1 + a0 ) - a1;

    return origin + rayDirection * result;
}

STATIC_CONST uvec2 inverseBayerArray[16] = {
    uvec2( 0, 0 ),    // 1
    uvec2( 2, 2 ),    // 2
    uvec2( 0, 2 ),    // 3
    uvec2( 2, 0 ),    // 4
    uvec2( 1, 1 ),    // 5
    uvec2( 3, 3 ),    // 6
    uvec2( 1, 3 ),    // 7
    uvec2( 3, 1 ),    // 8
    uvec2( 0, 1 ),    // 9
    uvec2( 2, 3 ),    // 10
    uvec2( 0, 3 ),    // 11
    uvec2( 2, 1 ),    // 12
    uvec2( 1, 0 ),    // 13
    uvec2( 3, 2 ),    // 14
    uvec2( 1, 2 ),    // 15
    uvec2( 3, 0 ) };  // 16

#if !defined(D_USE_CLOUD_MACROS)
uvec2
InverseBayer(
    uint luFrameIndex )
{
    //vec2 lOffset = inverseBayerArray[ uint( lfFrameIndex ) ] * 0.25;
    uvec2 lOffset = inverseBayerArray[luFrameIndex];

    return lOffset;
}
#else
#define InverseBayer( luFrameIndex ) inverseBayerArray[luFrameIndex]
#endif


STATIC_CONST mat4 bayer = mat4(
    vec4( 1, 9, 3, 11 ),
    vec4( 13, 5, 15, 7 ),
    vec4( 4, 12, 2, 10 ),
    vec4( 16, 8, 14, 6 ) );

#if !defined(D_USE_CLOUD_MACROS)
float
Bayer(
    uvec2 lPos )
{
    uvec2 positionMod = uvec2( lPos & 3 );

    float rndoffset = bayer[positionMod.x][positionMod.y];

    return rndoffset;
}
#else
#define Bayer( lPos ) bayer[(lPos.x & 3)][(lPos.y & 3)]
#endif

#define M_CALCULATE_HORIZON_DISTANCE( innerRadius, outerRadius )        sqrt( ( outerRadius * outerRadius ) - ( innerRadius * innerRadius ) )
#define M_CALCULATE_PLANET_RADIUS( atmosphereHeight, horizonDistance )  ((atmosphereHeight * atmosphereHeight + horizonDistance * horizonDistance) / (2.0 * atmosphereHeight)) - atmosphereHeight;

#define vec4_COVERAGE( f)	f.r
#define vec4_RAIN( f)		f.g
#define vec4_TYPE( f)		f.b

float
GetRayIntersectionPoint(
    in vec3  lStartPointVec3,
    in vec3  lEndPointVec3,
    in float lfRadius,
    out vec3 lOutNearPointVec3,
    out vec3 lOutFarPointVec3 )
{
    lOutNearPointVec3 = lStartPointVec3;
    lOutFarPointVec3 = lEndPointVec3;

    float lfLength = length( lStartPointVec3 - lEndPointVec3 );
    vec3  lNormalisedRayVec3 = normalize( lStartPointVec3 - lEndPointVec3 );
    float lfB = 2.0 * dot( lStartPointVec3, lNormalisedRayVec3 );
    float lfC = dot( lStartPointVec3, lStartPointVec3 ) - ( lfRadius * lfRadius );
    float lfDet = lfB * lfB - 4.0 * lfC;

    if ( lfDet >= 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfNear = 0.5 * ( -lfB + lfSqrt );
        float lfFar = 0.5 * ( -lfB - lfSqrt );

        lOutFarPointVec3 = lStartPointVec3 + lNormalisedRayVec3 * lfFar;
        lOutNearPointVec3 = lStartPointVec3 + lNormalisedRayVec3 * lfNear;

        lfDet = 1.0;

        return lfDet;
    }

    return 0.0;
}

//-----------------------------------------------------------------------------
///
///     GetRayIntersectionPointFast
///
///     @brief      GetRayIntersectionPointFast
///
///     @return     float which indicates number of intersections, 0 for none, more than 0 for any.
///
//-----------------------------------------------------------------------------
float
GetRayIntersectionPointFast(
    in  vec3  lStartPointVec3,
    in  vec3  lNormalisedDirectionVec3, // Assuming that this is a normalised direction vector.
    in  float lfRadius,
    out vec3  lOutNearPointVec3,
    out vec3  lOutFarPointVec3 )
{
    lOutNearPointVec3 = lStartPointVec3;
    lOutFarPointVec3 = lStartPointVec3 + lNormalisedDirectionVec3;

    float lfB = dot( lStartPointVec3, -lNormalisedDirectionVec3 ); // Shouldn't need negative of norm'd direction, but we're assuming it elsewhere.
    float lfC = dot( lStartPointVec3, lStartPointVec3 ) - ( lfRadius * lfRadius );
    float lfDiscriminant = lfB * lfB - lfC;

    if ( lfDiscriminant >= 0.0 )
    {
        float lfSqrt = sqrt( lfDiscriminant );
        float lfNear = -lfB + lfSqrt;
        float lfFar = -lfB - lfSqrt;

        lOutNearPointVec3 = lStartPointVec3 - lNormalisedDirectionVec3 * lfNear;
        lOutFarPointVec3 = lStartPointVec3 - lNormalisedDirectionVec3 * lfFar;
        
        return 1.0;
    }

    return 0.0;
}

//#define D_CLOUD_RENDER

vec3
GetScreenSpaceViewDir(
    in vec2  lFragCoordsVec2,
    in mat4  lInverseProjectionMatrix,
    in mat4  lInverseViewMatrix )
{
    vec4 lPositionVec4;
    lPositionVec4.x = lFragCoordsVec2.x * 2.0 - 1.0;

#ifndef D_PLATFORM_OPENGL   
    lPositionVec4.y = ( 1.0f - lFragCoordsVec2.y ) * 2.0 - 1.0;
#else
    lPositionVec4.y = lFragCoordsVec2.y * 2.0 - 1.0;
#endif

    lPositionVec4.z = 0.0;
    lPositionVec4.w = 1.0;

    // Inverse projection
    lPositionVec4 = MUL( lInverseProjectionMatrix, lPositionVec4 );
    lPositionVec4 = lPositionVec4 / lPositionVec4.w;

    // Inverse view
    mat4 lViewMat = lInverseViewMatrix;
    MAT4_SET_POS( lViewMat, vec4( 0.0, 0.0, 0.0, 1.0 ) );
    lPositionVec4 = MUL( lViewMat, lPositionVec4 );

    vec3 lViewVectorVec3 = normalize( lPositionVec4.xyz );

    return lViewVectorVec3;
}

#if defined ( D_TEMPORAL_RENDER_HIGH )

vec3
GetWorldSpaceViewDir(
    in vec2  lScreenPositionVec2,
    in mat4  lInverseViewProjectionMatrix,
    in mat4  lInverseViewMatrix )
{
    vec4 lPositionVec4 = vec4( lScreenPositionVec2.x, lScreenPositionVec2.y, 0.0, 1.0 );
    
    // Inverse projection
    lPositionVec4 = MUL( lInverseViewProjectionMatrix, lPositionVec4 );
    lPositionVec4 = lPositionVec4 / lPositionVec4.w;
    
    vec3 lViewVectorVec3 = normalize( lPositionVec4.xyz );
    
    return lViewVectorVec3;
}

#else

vec3
GetWorldSpaceViewDir(
    in vec2  lScreenPositionVec2,
    in mat4  lInverseProjectionMatrix,
    in mat4  lInverseViewMatrix )
{
    vec4 lPositionVec4 = vec4( lScreenPositionVec2.x, lScreenPositionVec2.y, 0.0, 1.0 );

    // Inverse projection
    lPositionVec4 = MUL( lInverseProjectionMatrix, lPositionVec4 );
    lPositionVec4 = lPositionVec4 / lPositionVec4.w;

    // Inverse view
    mat4 lViewMat = lInverseViewMatrix;
    MAT4_SET_POS( lViewMat, vec4( 0.0, 0.0, 0.0, 1.0 ) );
    lPositionVec4 = MUL( lViewMat, lPositionVec4 );

    vec3 lViewVectorVec3 = normalize( lPositionVec4.xyz );

    return lViewVectorVec3;
}

#endif

vec3
GetWorldSpaceCoordinates(
    in vec3  lScreenPositionVec3,
    in mat4  lInverseProjectionMatrix,
    in mat4  lInverseViewMatrix )
{
    vec4 lPositionVec4 = vec4( lScreenPositionVec3.x, lScreenPositionVec3.y, lScreenPositionVec3.z, 1.0 );
    
    // Inverse projection
    lPositionVec4 = MUL( lInverseProjectionMatrix, lPositionVec4 );
    lPositionVec4 = lPositionVec4 / lPositionVec4.w;
    
    // Inverse view
    mat4 lViewMat = lInverseViewMatrix;
    MAT4_SET_POS( lViewMat, vec4( 0.0, 0.0, 0.0, 1.0 ) );
    lPositionVec4 = MUL( lViewMat, lPositionVec4 );
    
    return lPositionVec4.xyz;
}

vec3
GetWorldSpaceCoordinatesDenormalised(
    in vec3  lScreenPositionVec3,
    in mat4  lInverseProjectionMatrix,
    in mat4  lInverseViewMatrix )
{
    vec4 lPositionVec4 = vec4( lScreenPositionVec3.x, lScreenPositionVec3.y, lScreenPositionVec3.z, 1.0 );
    
    // Inverse projection
    lPositionVec4 = MUL( lInverseProjectionMatrix, lPositionVec4 );
    //lPositionVec4 = lPositionVec4 / lPositionVec4.w;

    lPositionVec4.w = 1.0;
    
    // Inverse view
    lPositionVec4 = MUL( lInverseViewMatrix, lPositionVec4 );
    
    return lPositionVec4.xyz;
}

vec3
GetWorldSpaceCoordinatesOfScreenPixel(
    in vec2  lScreenPositionVec2,
    in mat4  lInverseViewProjectionMatrix )
{
    vec4 lPositionVec4 = vec4( lScreenPositionVec2.x, lScreenPositionVec2.y, 0.0, 1.0 );
    
    // Inverse projection
    lPositionVec4 = MUL( lInverseViewProjectionMatrix, lPositionVec4 );
    lPositionVec4 = lPositionVec4 / lPositionVec4.w;
    
    return lPositionVec4.xyz;
}

#if defined ( D_ENABLE_ROUGH_RENDER )
STATIC_CONST int kiLightSampleIterations = 2;
#else
STATIC_CONST int kiLightSampleIterations = 6;
#endif

#if defined ( D_PLATFORM_SWITCH )
    // We can increase occupancy by using static const vars for anything which is static on this platform.
    STATIC_CONST float kfConeRadius = 5.0;
    STATIC_CONST float kfRandomWalk = kfConeRadius / kiLightSampleIterations;
    STATIC_CONST vec4 kCloudHeightGradient1 = { 0.1, 0.15, 0.15, 0.2 };
    STATIC_CONST vec4 kCloudHeightGradient2 = { 0.0, 0.1, 0.3, 0.6 };
    STATIC_CONST vec4 kCloudHeightGradient3 = { 0.0, 0.2, 0.3, 1.0 };
    STATIC_CONST float kfHorizonCoverageStart = 0.3f;
    STATIC_CONST float kfHorizonCoverageEnd = 0.4f;
    STATIC_CONST float kfAmbientDensity = 0.1f;
    STATIC_CONST float kfLightScalar = 5.0f;
    STATIC_CONST float kfAmbientScalar = 1.721854f;
    STATIC_CONST float kfDensity = 1.0f;
    STATIC_CONST float kfForwardScatteringG = 0.9f;
    STATIC_CONST float kfBackwardScatteringG = 0.3f;
    STATIC_CONST float kfDarkOutlineScalar = 1.0f;
    STATIC_CONST float kfBaseScale = 1.0f;
    STATIC_CONST float kfSampleScalar = 5.0f;
    STATIC_CONST float kfSampleThreshold = 0.25f;
    STATIC_CONST float kfCloudBottomFade = 1.0f;
    STATIC_CONST float kfDetailScale = 6.0f;
    STATIC_CONST float kfErosionEdgeSize = 0.5f;
    STATIC_CONST float kfCloudDistortion = 50.0f;
    STATIC_CONST float kfCloudDistortionScale = 1.0f;
    STATIC_CONST float kfRayMinimumY = 0.0f;
    STATIC_CONST float kfLODDistance = 0.0f;
    STATIC_CONST float kfHorizonFadeStartAlpha = 0.0f;
    STATIC_CONST float kfHorizonFadeScalar = 0.25f;
    STATIC_CONST float kfHorizonDistance = 11165.0f;
#endif

struct cCloudProperties
{
    // Data
    int   miMaxIterations;

    float mfHorizonCoverageStart;
    float mfHorizonCoverageEnd;
    
    vec3 mLightConeStepVec3;

    #if !defined ( D_PLATFORM_SWITCH )
    float mfLightConeRandomWalk;
    #endif
    
    float mfHenyeyBalancePoint;

    float mfDensity;
    float mfAmbientDensity;
    float mfForwardScatteringG;
    float mfBackwardScatteringG;
    float mfDarkOutlineScalar;

    float mfAnimationScale;
    float mfBaseScaleScalar;
    float mfSampleScalar;
    float mfSampleThreshold;
    float mfCloudBottomFade;
    float mfDetailScale;
    float mfErosionEdgeSize;
    float mfCloudDistortion;
    float mfCloudDistortionScale;
    float mfRayMinimumY;
    float mfLODDistance;
    float mfHorizonFadeStartAlpha;
    float mfHorizonFadeScalar;	// Fades clouds on horizon, 1.0 . 10.0 (1.0 = smooth fade, 10 = no fade)
    float mfHorizonDistance;
    
    float mfAtmosphereStartHeight;
    float mfAtmosphereEndHeight;
    float mfStratosphereHeight;

    // Calculated
    float mfRayAboveHorizon;

    float mfCloudRatio;
    float mfCloudStrengthModifier;
    float mfRatioAbovePlanet;
    float mfAtmosphereThickness;
    float mfAtmosphereStartRatio;
    float mfEarthRadius;
    float mfMaxDistance;
    float mfRayStepLengthMin;
    float mfRayStepLengthMax;
    float mfBaseScale;
    float mfCoverageScale;

    int muiTemporalCloudStep;

    CLOUDCOLOUR1 mfLightScalar;
    CLOUDCOLOUR1 mfAmbientScalar;

    vec3  mLightDirectionVec3;
    vec3  mCameraPositionVec3;
    vec3  mTriplanarBlendWeights;
    vec3  mWindDirection;

    CLOUDCOLOUR3  mLightColourVec3;
};


float
SmoothThreshold(
    in float value,
    in float threshold,
    in float edgeSize )
{
    return smoothstep( threshold, threshold + edgeSize, value );
}

vec3
SmoothThreshold(
    in vec3  value,
    in float threshold,
    in float edgeSize )
{
    value.r = smoothstep( threshold, threshold + edgeSize, value.r );
    value.g = smoothstep( threshold, threshold + edgeSize, value.g );
    value.b = smoothstep( threshold, threshold + edgeSize, value.b );

    return value;
}

float
MixNoise(
    in float value,
    in float noise,
    in float a,
    in float b,
    in float height )
{
    float s = smoothstep( a, b, height );
    value += noise * s;
    //value *= lerp( 1.0, 0.5, s);

    return value;
}

#if !defined(D_USE_CLOUD_MACROS)
float
Lerp3(
    in float v0,
    in float v1,
    in float v2,
    in float a )
{
    return a < 0.5 ? mix( v0, v1, a * 2.0 ) : mix( v1, v2, ( a - 0.5 ) * 2.0 );
}
#else
#define LerpF3( v0, v1, v2, a ) mix( mix( v0, v1, saturate( a * 2.0 ) ), v2, saturate( (a - 0.5) * 2.0 ) )
#endif

#if !defined(D_USE_CLOUD_MACROS)
vec4
Lerp3(
    in vec4 v0,
    in vec4 v1,
    in vec4 v2,
    in float a )
{
    return vec4( Lerp3( v0.x, v1.x, v2.x, a ),
        Lerp3( v0.y, v1.y, v2.y, a ),
        Lerp3( v0.z, v1.z, v2.z, a ),
        Lerp3( v0.w, v1.w, v2.w, a ) );
}
#else
#define Lerp3( v0, v1, v2, a) vec4(LerpF3(v0.x, v1.x, v2.x, a), LerpF3(v0.y, v1.y, v2.y, a), LerpF3(v0.z, v1.z, v2.z, a), LerpF3(v0.w, v1.w, v2.w, a))
#endif

#if !defined(D_USE_CLOUD_MACROS)
float
NormalizedAtmosphereY(
    in vec3  lRayVec3,
    in float lfStartCloudLayerRatio,
    in float lfAtmosphereThicknessReciprocal )
{
    // Originally, this was...
    // ( Magnitude( Position Relative To Planet Centre ) - ( Planet Radius + Start Of Cloud Layer Height ) ) / ( Cloud Layer Height )
    //  but this can be simplified.
    //float y = length( lRayVec3 ) - ( lfEarthRadius + lfStartHeight );
    // return saturate( y * lfAtmosphereThicknessReciprocal );

    float scale = lfAtmosphereThicknessReciprocal * length( lRayVec3 ) - lfStartCloudLayerRatio;
    return saturate( scale );
}
#else
#define NormalizedAtmosphereY( lRayVec3, lfStartCloudLayerRatio, lfAtmosphereThicknessReciprocal ) saturate( lfAtmosphereThicknessReciprocal * length(lRayVec3) - lfStartCloudLayerRatio )
#endif

#if !defined(D_USE_CLOUD_MACROS)
float
LinearStep(
    float edge0,
    float edge1,
    float x )
{
    // Scale, and clamp x to 0..1 range
    x = saturate( ( x - edge0 ) / ( edge1 - edge0 ) );
    return x;
}
#else
#define LinearStep( edge0, edge1, x ) saturate( (x - edge0) / (edge1 - edge0) )
#endif

STATIC_CONST float kfBlahHackPower = 0.5;

#if !defined(D_USE_CLOUD_MACROS)
float
PowStep(
    float edge0,
    float edge1,
    float x )
{
    // Scale, and clamp x to 0..1 range
    x = saturate( ( x - edge0 ) / ( edge1 - edge0 ) );
    return pow( x, kfBlahHackPower );
}
#else
#define PowStep( edge0, edge1, x ) pow( saturate((x - edge0) / (edge1 - edge0)), kfBlahHackPower)
#endif

#if !defined(D_USE_CLOUD_MACROS)
float
InversePowStep(
    float edge0,
    float edge1,
    float x )
{
    // Scale, and clamp x to 0..1 range
    x = saturate( ( x - edge0 ) / ( edge1 - edge0 ) );
    return 1.0 - pow( 1.0 - x, kfBlahHackPower );
}
#else
#define InversePowStep( edge0, edge1, x ) (1.0 - pow(1.0 - saturate((x - edge0) / (edge1 - edge0)), kfBlahHackPower))
#endif

#if !defined(D_USE_CLOUD_MACROS)
float
GradientStep(
    in float a,
    in vec4 gradient )
{
    return PowStep( gradient.x, gradient.y, a ) - InversePowStep( gradient.z, gradient.w, a );
}
#else
#define GradientStep( a, gradient) (PowStep(gradient.x, gradient.y, a) - InversePowStep(gradient.z, gradient.w, a))
#endif

#ifdef D_CLOUD_RENDER

#ifdef D_PLATFORM_ORBIS
#pragma argument(targetoccupancy_atallcosts=4)
#endif

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

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END


// Random on Unit Sphere
STATIC_CONST vec3   _Random0 = vec3( 0.4469, 0.5511, 0.7047 );
STATIC_CONST vec3   _Random1 = vec3( -0.0791, -0.0898, -0.9928 );
STATIC_CONST vec3   _Random2 = vec3( -0.0205, 0.8772, -0.4797 );
STATIC_CONST vec3   _Random3 = vec3( 0.4335, -0.0052, -0.9012 );
STATIC_CONST vec3   _Random4 = vec3( -0.8569, 0.0677, -0.5111 );
STATIC_CONST vec3   _Random5 = vec3( 0.8033, -0.5266, 0.2782 );

STATIC_CONST vec3 RandomUnitSphere[6] = { _Random0 * 0.0, _Random1 * 1.0, _Random2 * 2.0, _Random3 * 3.0, _Random4  * 4.0, _Random5 * 5.0 };

float
HenyeyGreensteinPhase(
    in float costh,
    in float g )
{
    return ( 1.0 - g * g ) / ( 4.0 * PI * pow( 1.0 + g * g - 2.0 * g * costh, 3.0 / 2.0 ) );
}


float
BeerTerm(
    in float lfPrecipitation,
    in float lfDensity )
{
    return exp( -( lfDensity * lfPrecipitation ) );
}

float
PowderTerm(
    in float lfDensityAtSample,
    in float lfCosTheta,
    in float lfDarkOutlineScalar,
    in float lfDensityScalar )
{
    float lfPowder = 1.0 - exp( -lfDensityScalar * lfDensityAtSample * 2.0 );
    //lfPowder = saturate( lfPowder * lfDarkOutlineScalar * 2.0 );
    //return mix( 1.0, lfPowder, smoothstep( 0.5, -0.5, lfCosTheta ) );
    return lfPowder;
}

CLOUDCOLOUR3
SampleAmbientLight(
    in CLOUDCOLOUR3  lBaseColour,
    in CLOUDCOLOUR3  lTopColour,
    in float atmosphereY )
{
    return mix( lBaseColour, lTopColour, atmosphereY );
}

vec2
SampleCoverage(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayVec3,
    in bool                  lbHasHorizon )
{
    vec3 lNewRayVec3 = lRayVec3 * lCloudProperties.mfCoverageScale;

#if !defined(D_USE_CLOUD_MACROS)
    vec2 lCoverageVec2 = GetTriPlanarMapGB(lCloudProperties.mTriplanarBlendWeights, lNewRayVec3, SAMPLER2DPARAM_LOCAL(lCustomPerMeshUniforms, gCoverage2D));
#else
    vec2 lCoverageVec2 = GetTriPlanarMapGB(lCloudProperties.mTriplanarBlendWeights, lNewRayVec3, SAMPLER_GETLOCAL(lCustomPerMeshUniforms, gCoverage2D));
#endif

    lCoverageVec2.x = saturate(lCloudProperties.mfCloudStrengthModifier + lCoverageVec2.x);

    return lCoverageVec2;

    /*
    coverageB.b = saturate( smoothstep( _HorizonCoverageEnd, _HorizonCoverageStart, depth) * 2.0);
    if ( lbHasHorizon )
    {
        vec4 lCoverageBVec4 = vec4( 1.0, 0.0, 0.0, 0.0 );

        float lfAlpha = smoothstep( lCloudProperties.mfHorizonCoverageStart, lCloudProperties.mfHorizonCoverageEnd, lfDepth );

        lCoverageBVec4 = vec4(  smoothstep( lCloudProperties.mfHorizonCoverageStart, lCloudProperties.mfHorizonCoverageEnd, lfDepth ),
                                0.0,
                                smoothstep( lCloudProperties.mfHorizonCoverageEnd, lCloudProperties.mfHorizonCoverageStart + ( lCloudProperties.mfHorizonCoverageEnd - lCloudProperties.mfHorizonCoverageStart ) * 0.5, lfDepth ),
                                0.0 );

        return mix( lCoverageVec4, lCoverageBVec4, lfAlpha );
    }
    else
    {
        return lCoverageVec4;
    }*/
}


// Utility function that maps a value from one range to another.
float
Remap(
    float original_value,
    float original_min,
    float original_max,
    float new_min,
    float new_max )
{
    return new_min + ( saturate( ( original_value - original_min ) / ( original_max - original_min ) ) * ( new_max - new_min ) );
}

float
GetDensityHeightGradientForPoint(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    float lfAtmosphereY,
    float  lfCloudType )
{
    vec4 lGradientVec4;

    #if defined ( D_PLATFORM_SWITCH )
    lGradientVec4 = Lerp3( 
        kCloudHeightGradient1,
        kCloudHeightGradient2,
        kCloudHeightGradient3,
        lfCloudType );
    #else
    lGradientVec4 = Lerp3( lCustomPerMeshUniforms.gCloudHeightGradient1,
        lCustomPerMeshUniforms.gCloudHeightGradient2,
        lCustomPerMeshUniforms.gCloudHeightGradient3,
        lfCloudType );
    #endif

    return GradientStep( lfAtmosphereY, lGradientVec4 );
}

float
SampleCloud(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayVec3,
    in float                 lfMipLevel,
    in int                   liQuality )
{
    // get height fraction  (be sure to create a cloud_min_max variable)
    float lfAtmosphereY = NormalizedAtmosphereY(
        lRayVec3,
        lCloudProperties.mfAtmosphereStartRatio,
        lCloudProperties.mfAtmosphereThickness );
        
    vec3 wind_direction = GetWindDirection( lCloudProperties.mTriplanarBlendWeights, lCustomPerMeshUniforms.gWindOffset.xy );

    float cloud_speed = lCloudProperties.mfAnimationScale;

    // cloud_top offset - push the tops of the clouds along this wind direction by this many units.
    float cloud_top_offset = 500.0;
    // skew in wind direction
    lRayVec3 += lfAtmosphereY * wind_direction * cloud_top_offset;

    //animate clouds in wind direction and add a small upward bias to the wind direction
    lRayVec3 += wind_direction * cloud_speed;

    vec2 lWeatherDataVec2 = SampleCoverage( lCustomPerMeshUniforms,
        lCloudProperties,
        lRayVec3,
        false );

    // cloud coverage is stored in the weather_data?s red channel.
    float cloud_coverage = lWeatherDataVec2.x * lCloudProperties.mfCloudRatio;

    // apply anvil deformations
    cloud_coverage = pow( cloud_coverage, Remap( lfAtmosphereY, 0.7, 0.8, 1.0, mix( 1.0, 0.5, lCloudProperties.mfCloudBottomFade ) ) );

    cloud_coverage = smoothstep( 0.25, 1.0, cloud_coverage );

    if ( cloud_coverage <= 0.0 || liQuality == 0 )
    {
        return max( 0.0, cloud_coverage );
    }


    //vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale + (lCustomPerMeshUniforms.gBaseOffset.xyz * lCloudProperties.mfAnimationScale )); // * gfTime
    vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale ); // * gfTime

                                                                                  // wind settings

                                                                                  // read the low frequency Perlin-Worley and Worley noises

    // We're storing the Worley noise in the green channel and the perlin noise in the alpha channel.
    vec2 low_frequency_noises = texture3DLod( SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gPerlin3D ), lLowFreqNoiseCoordsVec3, lfMipLevel ).ga;

    // We used to build an fBm out of  the low frequency Worley noises that can be used to add detail to the Low frequency Perlin-Worley noise
    //float low_freq_fBm = (low_frequency_noises.g * 0.625) + (low_frequency_noises.b * 0.25) + (low_frequency_noises.a * 0.125);
    float low_freq_fBm = low_frequency_noises.x;

    // define the base cloud shape by dilating it with the low frequency fBm made of Worley noise.
    float base_cloud = Remap( low_frequency_noises.y, -( 1.0 - low_freq_fBm ), 1.0, 0.0, 1.0 );

    // Get the density-height gradient using the density-height function (not included)
    //float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, kfHackDebugScreenSide > 0.5 ? lCloudProperties.mfLODDistance : lWeatherDataVec4.b );	
    float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, lWeatherDataVec2.y );

    // apply the height function to the base cloud shape
    base_cloud *= density_height_gradient;

    if ( base_cloud <= 0.0 )
    {
        return 0.0;
    }

    //Use remapper to apply cloud coverage attribute
    float base_cloud_with_coverage = Remap( base_cloud, 1.0 - cloud_coverage, 1.0, 0.0, 1.0 );

    //Multiply result by cloud coverage so that smaller clouds are lighter and more aesthetically pleasing.
    //base_cloud_with_coverage *= cloud_coverage;
    //base_cloud_with_coverage *= 1.0 - cloud_coverage;

    //define final cloud value
    //float final_cloud = mix( base_cloud_with_coverage, smoothstep( 0.4, 0.5, cloud_coverage ), lCloudProperties.mfSampleThreshold );
    float final_cloud = base_cloud_with_coverage;

    // only do detail work if we are taking expensive samples!
    if ( liQuality == 2 )
    {
        vec3 lDistortionCoordsVec3 = lRayVec3 * lCloudProperties.mfBaseScale * lCloudProperties.mfCloudDistortionScale;
        // add some turbulence to bottoms of clouds using curl noise.  Ramp the effect down over height and scale it by some value (200 in this example)
        //vec2 curl_noise = tex2Dlod(Cloud2DNoiseTexture,  Cloud2DNoiseSampler,  vec4 ( vec2(lRayVec3.x, lRayVec3.y), 0.0, 1.0 ).rg;
#if !defined(D_USE_CLOUD_MACROS)
        vec3 curl = GetTriPlanarMap(lCloudProperties.mTriplanarBlendWeights, lDistortionCoordsVec3, SAMPLER2DPARAM_LOCAL(lCustomPerMeshUniforms, gCurl2D)).xyz;
#else
        vec3 curl = GetTriPlanarMap(lCloudProperties.mTriplanarBlendWeights, lDistortionCoordsVec3, SAMPLER_GETLOCAL(lCustomPerMeshUniforms, gCurl2D)).xyz;
#endif
        //lRayVec3.xy += curl_noise.rg * (1.0 - lfAtmosphereY) * 200.0;
        lRayVec3 += curl * ( 1.0 - lfAtmosphereY ) * lCloudProperties.mfCloudDistortion;
        //lRayVec3 += curl * (1.0 - lfAtmosphereY) * 200.0;

        // sample high-frequency noises
        vec3 lHighFreqNoiseCoordsVec3 = ( lRayVec3 * lCloudProperties.mfBaseScale * lCloudProperties.mfDetailScale ); //+(lCustomPerMeshUniforms.gDetailOffset.xyz * lCloudProperties.mfAnimationScale); // * gfTime        
        float high_freq_fBm = texture3DLod( SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gDetail3D ), lHighFreqNoiseCoordsVec3, lfMipLevel ).g;

        //vec3 high_frequency_noises = texture3DLod(SAMPLER_GETLOCAL(lCustomPerMeshUniforms, gDetail3D), lHighFreqNoiseCoordsVec3, lfMipLevel).xyz;
        // build High frequency Worley noise fBm
        //float high_freq_fBm = (high_frequency_noises.r * 0.625) + (high_frequency_noises.g * 0.25) + (high_frequency_noises.b * 0.125);

        // get the lfAtmosphereY for use with blending noise types over height
        lfAtmosphereY = NormalizedAtmosphereY(
            lRayVec3,
            lCloudProperties.mfAtmosphereStartRatio,
            lCloudProperties.mfAtmosphereThickness );

        // transition from wispy shapes to billowy shapes over height
        float high_freq_noise_modifier = mix( high_freq_fBm, 1.0 - high_freq_fBm, saturate( lfAtmosphereY * 10.0 ) );
        //float high_freq_noise_modifier = 1.0 - high_freq_fBm;
        //float high_freq_noise_modifier = high_freq_fBm;

        // erode the base cloud shape with the distorted high frequency Worley noises.
        final_cloud = Remap( final_cloud, high_freq_noise_modifier * lCloudProperties.mfErosionEdgeSize, 1.0, 0.0, 1.0 );
    }

    //final_cloud = step(0.1, final_cloud);
    return final_cloud * lCloudProperties.mfSampleScalar;
    //return step( 0.5, cloud_coverage );
}


float
SampleCloudLighting(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayVec3,
    //in vec4                  lWeatherDataVec4,
    in float                 lfMipLevel,
    in int                   liQuality )
{
    // get height fraction  (be sure to create a cloud_min_max variable)
    float lfAtmosphereY = NormalizedAtmosphereY(
        lRayVec3,
        lCloudProperties.mfAtmosphereStartRatio,
        lCloudProperties.mfAtmosphereThickness );

    float cloud_speed = lCloudProperties.mfAnimationScale;

    // cloud_top offset - push the tops of the clouds along this wind direction by this many units.
    float cloud_top_offset = 500.0;

    // skew in wind direction, and animate clouds in wind direction and add a small upward bias to the wind direction
    lRayVec3 += lCloudProperties.mWindDirection * ( lfAtmosphereY * cloud_top_offset + cloud_speed );

    vec2 lWeatherDataVec2 = SampleCoverage( lCustomPerMeshUniforms,
        lCloudProperties,
        lRayVec3,
        false );

    // cloud coverage is stored in the weather_data?s red channel.
    float cloud_coverage = lWeatherDataVec2.x * lCloudProperties.mfCloudRatio;

    #if defined ( D_USE_WORLEY_CLOUD )
    const float kfOldDensityMinimum = 0.375;
    #else
    const float kfOldDensityMinimum = 0.25;
    #endif

    cloud_coverage = smoothstep( kfOldDensityMinimum, 1.0, cloud_coverage );

    if ( lWeatherDataVec2.x < WEIGHT_FLOOR )
    {
        return 0.0;
    }

    #if !defined ( D_PLATFORM_SWITCH )
    // apply anvil deformations
    cloud_coverage = pow( cloud_coverage, Remap( lfAtmosphereY, 0.7, 0.8, 1.0, mix( 1.0, 0.5, lCloudProperties.mfCloudBottomFade ) ) );
    #endif

    #if defined ( D_USE_WORLEY_CLOUD )

        // Get Worley noise from the red channel.
        vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale );
        
        float low_frequency_noise = texture3DLod(
            SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gWorleyNoiseMap ),
            lLowFreqNoiseCoordsVec3,
            lfMipLevel ).r;

        // Get the height gradient from weather data.
        float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, lWeatherDataVec2.y );

        // Creating a Fractal Brownian Motion model, scaled by height gradient, cloud coverage, and the smoothstep which removes some of the lowest
        //  density reads, and gives us a rough cloud shape, in discrete clouds.
        float rough_cloud_density = saturate( low_frequency_noise * density_height_gradient * cloud_coverage );

        // Erode cloud edges to give us something approaching a basic cloud shape.  The perlin noise isn't the right granularity to give
        //  us something nice if we apply that without a separate read to the 3D noise texture... but the basic shape is ok for a rough cloud.
        //float cloud_density = rough_cloud_density < 0.3 ? saturate( rough_cloud_density * low_frequency_noises.y ) : rough_cloud_density;
        float cloud_density = rough_cloud_density;
        cloud_density = cloud_density < WEIGHT_FLOOR ? 0.0 : cloud_density;

        return cloud_density;

    #else

        cloud_coverage = smoothstep( 0.25, 1.0, cloud_coverage );

        vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale ); // * gfTime

        // We're storing the Worley noise in the green channel and the perlin noise in the alpha channel.
        vec2 low_frequency_noises = texture3DLod( SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gPerlin3D ), lLowFreqNoiseCoordsVec3, lfMipLevel ).ga;

        // We used to build an fBm out of  the low frequency Worley noises that can be used to add detail to the Low frequency Perlin-Worley noise
        //float low_freq_fBm = (low_frequency_noises.g * 0.625) + (low_frequency_noises.b * 0.25) + ( low_frequency_noises.a * 0.125 );
        float low_freq_fBm = low_frequency_noises.x;

        // define the base cloud shape by dilating it with the low frequency fBm made of Worley noise.
        float base_cloud = Remap( low_frequency_noises.y, -( 1.0 - low_freq_fBm ), 1.0, 0.0, 1.0 );

        // Get the density-height gradient using the density-height function (not included)
        //float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, kfHackDebugScreenSide > 0.5 ? lCloudProperties.mfLODDistance : lWeatherDataVec4.b );
        float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, lWeatherDataVec2.y );

        // apply the height function to the base cloud shape
        base_cloud *= density_height_gradient;

        if ( base_cloud <= 0.0 )
        {
            return 0.0;
        }

        //Use remapper to apply cloud coverage attribute
        float base_cloud_with_coverage = Remap( base_cloud, 1.0 - cloud_coverage, 1.0, 0.0, 1.0 );

        //Multiply result by cloud coverage so that smaller clouds are lighter and more aesthetically pleasing.
        //define final cloud value
        return base_cloud_with_coverage * lCloudProperties.mfSampleScalar;

    #endif
}

float
SampleCloudRoughShape(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties lCloudProperties,
    in vec3 lRayVec3,
    in float lfMipLevel,
    inout float lfAtmosphereY )
{
    // get height fraction  (be sure to create a cloud_min_max variable)
    lfAtmosphereY = NormalizedAtmosphereY(
        lRayVec3,
        lCloudProperties.mfAtmosphereStartRatio,
        lCloudProperties.mfAtmosphereThickness );

    float cloud_speed = lCloudProperties.mfAnimationScale;

    // cloud_top offset - push the tops of the clouds along this wind direction by this many units.
    float cloud_top_offset = 500.0;

    // skew weather in wind direction, animate clouds in wind direction and add a small upward bias to the wind direction
    lRayVec3 += lCloudProperties.mWindDirection * ( lfAtmosphereY * cloud_top_offset + cloud_speed );

    vec2 lWeatherDataVec2 = SampleCoverage( lCustomPerMeshUniforms,
        lCloudProperties,
        lRayVec3,
        false );

    // cloud coverage is stored in the weather_data's red channel.
    float cloud_coverage = lWeatherDataVec2.x * lCloudProperties.mfCloudRatio;

    // create a sane floor for the coverage data.
    #if defined ( D_USE_WORLEY_CLOUD )
    const float kfOldDensityMinimum = 0.375;
    #else
    const float kfOldDensityMinimum = 0.25;
    #endif

    cloud_coverage = smoothstep( kfOldDensityMinimum, 1.0, cloud_coverage );

    if ( cloud_coverage < WEIGHT_FLOOR )
    {
        return 0.0;
    }

    #if !defined ( D_PLATFORM_SWITCH )
    // apply anvil deformations.  Very subtle, and not really that visible... so removed on Switch for the moment.
    cloud_coverage = pow( cloud_coverage, Remap( lfAtmosphereY, 0.7, 0.8, 1.0, mix( 1.0, 0.5, lCloudProperties.mfCloudBottomFade ) ) );
    #endif

    #if defined ( D_USE_WORLEY_CLOUD )

        // Get Worley noise from the red channel.
        vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale );
        
        float low_frequency_noise = texture3DLod( 
            SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gWorleyNoiseMap ),
            lLowFreqNoiseCoordsVec3,
            lfMipLevel ).r;

        // Get the height gradient from weather data.
        float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, lWeatherDataVec2.y );

        // Creating a Fractal Brownian Motion model, scaled by height gradient, cloud coverage, and the smoothstep which removes some of the lowest
        //  density reads, and gives us a rough cloud shape, in discrete clouds.
        float rough_cloud_density = saturate( low_frequency_noise * density_height_gradient * cloud_coverage );

        // Erode cloud edges to give us something approaching a basic cloud shape.  The perlin noise isn't the right granularity to give
        //  us something nice if we apply that without a separate read to the 3D noise texture... but the basic shape is ok for a rough cloud.
        //float cloud_density = rough_cloud_density < 0.3 ? saturate( rough_cloud_density * low_frequency_noises.y ) : rough_cloud_density;
        float cloud_density = rough_cloud_density;
        cloud_density = cloud_density < WEIGHT_FLOOR ? 0.0 : cloud_density;

        return cloud_density;

    #else

        //vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale + (lCustomPerMeshUniforms.gBaseOffset.xyz * lCloudProperties.mfAnimationScale )); // * gfTime
        vec3 lLowFreqNoiseCoordsVec3 = vec3( lRayVec3 * lCloudProperties.mfBaseScale ); // * gfTime

        // wind settings

        // We're storing the Worley noise in the green channel and the perlin noise in the alpha channel.
        vec2 low_frequency_noises = texture3DLod( SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gPerlin3D ), lLowFreqNoiseCoordsVec3, lfMipLevel ).ga;

        // We used to build an fBm out of  the low frequency Worley noises that can be used to add detail to the Low frequency Perlin-Worley noise
        //float low_freq_fBm = ( low_frequency_noises.g * 0.625 ) + ( low_frequency_noises.b * 0.25 ) + ( low_frequency_noises.a * 0.125 );
        float low_freq_fBm = low_frequency_noises.x;

        // define the base cloud shape by dilating it with the low frequency fBm made of Worley noise.
        float base_cloud = Remap( low_frequency_noises.y, -( 1.0 - low_freq_fBm ), 1.0, 0.0, 1.0 );

        // Get the density-height gradient using the density-height function (not included)
        //float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, kfHackDebugScreenSide > 0.5 ? lCloudProperties.mfLODDistance : lWeatherDataVec4.b );

        float density_height_gradient = GetDensityHeightGradientForPoint( lCustomPerMeshUniforms, lfAtmosphereY, lWeatherDataVec2.y );

        // apply the height function to the base cloud shape
        base_cloud *= density_height_gradient;

        //Use remapper to apply cloud coverage attribute
        float base_cloud_with_coverage = Remap( base_cloud, 1.0 - cloud_coverage, 1.0, 0.0, 1.0 );

        //Multiply result by cloud coverage so that smaller clouds are lighter and more aesthetically pleasing.
        //base_cloud_with_coverage *= cloud_coverage;
        //base_cloud_with_coverage *= 1.0 - cloud_coverage;

        //define final cloud value
        //float final_cloud = mix( base_cloud_with_coverage, smoothstep( 0.4, 0.5, cloud_coverage ), lCloudProperties.mfSampleThreshold );
        return base_cloud_with_coverage;

    #endif
}

float
SampleCloudFineShape(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties lCloudProperties,
    in vec3  lRayVec3,
    in float lfMipLevel,
    in float lfSampleDensity,
    in float lfAtmosphereY )
{
    vec3 lDistortionCoordsVec3 = lRayVec3 * lCloudProperties.mfBaseScale * lCloudProperties.mfCloudDistortionScale;
    // add some turbulence to bottoms of clouds using curl noise.  Ramp the effect down over height and scale it by some value (200 in this example)
    //vec2 curl_noise = tex2Dlod(Cloud2DNoiseTexture,  Cloud2DNoiseSampler,  vec4 ( vec2(lRayVec3.x, lRayVec3.y), 0.0, 1.0 ).rg;

#if !defined(D_USE_CLOUD_MACROS)
    vec3 curl = GetTriPlanarMap(lCloudProperties.mTriplanarBlendWeights, lDistortionCoordsVec3, SAMPLER2DPARAM_LOCAL(lCustomPerMeshUniforms, gCurl2D)).xyz;
#else
    vec3 curl = GetTriPlanarMap(lCloudProperties.mTriplanarBlendWeights, lDistortionCoordsVec3, SAMPLER_GETLOCAL(lCustomPerMeshUniforms, gCurl2D)).xyz;
#endif

    //lRayVec3.xy += curl_noise.rg * (1.0 - lfAtmosphereY) * 200.0;
    lRayVec3 += curl * ( 1.0 - lfAtmosphereY ) * lCloudProperties.mfCloudDistortion;
    //lRayVec3 += curl * (1.0 - lfAtmosphereY) * 200.0;

    // sample high-frequency noises
    vec3 lHighFreqNoiseCoordsVec3 = ( lRayVec3 * lCloudProperties.mfBaseScale * lCloudProperties.mfDetailScale ); //+(lCustomPerMeshUniforms.gDetailOffset.xyz * lCloudProperties.mfAnimationScale); // * gfTime
    float high_freq_fBm = texture3DLod( SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gDetail3D ), lHighFreqNoiseCoordsVec3, lfMipLevel ).g;

    //vec3 high_frequency_noises = texture3DLod( SAMPLER_GETLOCAL( lCustomPerMeshUniforms, gDetail3D ), lHighFreqNoiseCoordsVec3, lfMipLevel ).xyz;
    // build High frequency Worley noise fBm
    //float high_freq_fBm = ( high_frequency_noises.r * 0.625 ) + ( high_frequency_noises.g * 0.25 ) + ( high_frequency_noises.b * 0.125 );

    // get the lfAtmosphereY for use with blending noise types over height
    lfAtmosphereY = NormalizedAtmosphereY(
        lRayVec3,
        lCloudProperties.mfAtmosphereStartRatio,
        lCloudProperties.mfAtmosphereThickness );

    // transition from wispy shapes to billowy shapes over height
    float high_freq_noise_modifier = mix( high_freq_fBm, 1.0 - high_freq_fBm, saturate( lfAtmosphereY * 10.0 ) );
    //float high_freq_noise_modifier = 1.0 - high_freq_fBm;
    //float high_freq_noise_modifier = high_freq_fBm;

    // erode the base cloud shape with the distorted high frequency Worley noises.  Erode more where the density is lower.
    float final_cloud = Remap( lfSampleDensity, high_freq_noise_modifier * lCloudProperties.mfErosionEdgeSize, 1.0, 0.0, 1.0 );

    return final_cloud;
}

float
SampleCloudMedium(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayVec3,
    in float                 lfMipLevel,
    in int                   liQuality ) // FIXME! no quality indicator...
{
    float lfAtmosphereY = 0.0;
    float final_cloud = SampleCloudRoughShape( lCustomPerMeshUniforms, lCloudProperties, lRayVec3, lfMipLevel, lfAtmosphereY );

    return final_cloud * lCloudProperties.mfSampleScalar;
}

// a function to gather density in a cone for use with lighting clouds.
float
SampleCloudDensityAlongCone(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayVec3,
    in float                 lfMipLevel )
{
    float lfDensityAlongCone = 0.0;
    vec3  lLightRayVec3 = lRayVec3;

    // lighting ray march loop
#if defined (D_PLATFORM_ORBIS)
    for ( unsigned int i = 0; i < kiLightSampleIterations; ++i )
#else
    for ( int i = 0; i < kiLightSampleIterations; ++i )
#endif
    {
        float lfMipOffset = float( i );

        #if !defined ( D_PLATFORM_SWITCH )
        // add the current step offset to the sample position
        lLightRayVec3 += lCloudProperties.mLightConeStepVec3 + ( lCloudProperties.mfLightConeRandomWalk * RandomUnitSphere[i] );
        #else
        lLightRayVec3 += lCloudProperties.mLightConeStepVec3 + ( kfRandomWalk * RandomUnitSphere[ i ] );
        #endif

        // sample cloud density the expensive way
        lfDensityAlongCone += SampleCloudLighting( lCustomPerMeshUniforms,
            lCloudProperties,
            lLightRayVec3,
            lfMipLevel + lfMipOffset,
            1 );
    }

    return lfDensityAlongCone;
}

float
GetLightEnergy(
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayVec3,
    in float                 lfAtmosphereY,
    in float                 lfSampleDensity,  // dl is the density sampled along the light ray for the given sample position.
    in float                 lfSampleDensityLight,  // ds_lodded is the low lod sample of density at the given sample position.
    in float                 lfCosAngle,
    in float                 lfStepSize )
{
    //#if !defined (D_PLATFORM_ORBIS)
    //    // Henyey
    //    float lfForwardPhase   = HenyeyGreensteinPhase( lfCosAngle, lCloudProperties.mfForwardScatteringG );
    //    float lfBackwardsPhase = HenyeyGreensteinPhase( lfCosAngle, lCloudProperties.mfBackwardScatteringG );
    //    float lfPhaseProbability = max( lfForwardPhase, lfBackwardsPhase * lCloudProperties.mfDarkOutlineScalar );
    //    //Energy = max( HG( cos(?), eccentricity), silver_intensity * HG( cos(?), 0.99 ? silver_spread))
    //#endif

    // Henyey
    float lfG = lfCosAngle < lCloudProperties.mfHenyeyBalancePoint ? lCloudProperties.mfBackwardScatteringG : lCloudProperties.mfForwardScatteringG;
    float lfPhase = HenyeyGreensteinPhase( lfCosAngle, lfG );
    float lfPhaseProbability = lfCosAngle < lCloudProperties.mfHenyeyBalancePoint ? lCloudProperties.mfDarkOutlineScalar * lfPhase : lfPhase;
    
    // Beer.. with exponent reduction... 
    //float lfPrimaryAttenuation     = exp( -lfSampleDensityLight * lCloudProperties.mfDensity );
    //float lfSecondaryAttenuation   = exp( -lfSampleDensityLight * lCloudProperties.mfDensity * 0.25) * 0.7;
    // (x^a)^b = x^ab;
    float lfPrimaryAttenuation = exp( -lfSampleDensityLight * lCloudProperties.mfDensity );
    float lfSecondaryAttenuation = pow( lfPrimaryAttenuation, 0.25 ) * 0.7;

    // Reduce the secondary component when we look toward the sun.
    lfSecondaryAttenuation = Remap( lfCosAngle, 0.7, 1.0, lfSecondaryAttenuation, lfSecondaryAttenuation * 0.25 );
    float lfAttenuationProbability = max( lfSecondaryAttenuation, lfPrimaryAttenuation );

    // in-scattering - one difference from presentation slides - we also reduce this effect once light has attenuated to make it directional.
    //float depth_probability      = mix( 0.05 + pow( lfSampleDensityLoded, Remap( lfAtmosphereY, 0.3, 0.85, 0.5, 2.0 )), 1.0, saturate( lfSampleDensityLight / lfStepSize));
    //float lfDepthProbability = 0.05 + pow(lfSampleDensity, Remap(lfAtmosphereY, 0.3, 0.85, 0.5, 2.0));
    float lfDepthProbability = 0.05 + pow( lfSampleDensity, Remap( lfAtmosphereY, 0.3, 0.85, 0.5, 2.0 ) );
    float lfVerticalProbability = pow( Remap( lfAtmosphereY, 0.07, 0.14, 0.1, 1.0 ), 0.8 );
    float lfInScatterProbability = lfDepthProbability * lfVerticalProbability;

    return lfInScatterProbability * lfAttenuationProbability * lfPhaseProbability;
    //return lfPhaseProbability;
    //return 0.1;
    //return lfForwardPhase;
    //return lfDepthProbability;
    //return lfAttenuationProbability;
}

CLOUDCOLOUR4
SampleLightParticle(
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in CLOUDCOLOUR1          lfSampleDensity,
    in float                 lfSampleDensityLight,
    in float                 lfCosAngle,
    in vec3                  lRayVec3,
    //in float                 lfAtmosphereY,
    in bool                  lbHasHorizon )
{
    CLOUDCOLOUR4  lParticleVec4;
    
    float lfAtmosphereY = NormalizedAtmosphereY(
        lRayVec3,
        lCloudProperties.mfAtmosphereStartRatio,
        lCloudProperties.mfAtmosphereThickness );

#if defined(D_PLATFORM_METAL)
    CLOUDCOLOUR3 lAmbientLightVec3 = SampleAmbientLight(
        half3(lCustomPerMeshUniforms.gCloudBaseColour.xyz),
        half3(lCustomPerMeshUniforms.gCloudTopColour.xyz),
        pow(lfAtmosphereY, 0.5) * exp(-lfSampleDensityLight * lCloudProperties.mfAmbientDensity));
#else
    CLOUDCOLOUR3 lAmbientLightVec3 = SampleAmbientLight(
        lCustomPerMeshUniforms.gCloudBaseColour.xyz,
        lCustomPerMeshUniforms.gCloudTopColour.xyz,
        pow( lfAtmosphereY, 0.5 ) * exp( -lfSampleDensityLight * lCloudProperties.mfAmbientDensity ) );
#endif

    //float lfNormalizedDepth = distance( lCloudProperties.mCameraPositionVec3, lRayVec3 ) / lCloudProperties.mfMaxDistance;
    //vec3 lSunLightVec3     = SampleLight( lCustomPerMeshUniforms, lCloudProperties, lRayVec3, lfDensity, lfCosAngle, lfNormalizedDepth, lbHasHorizon );

    float lfStepSize = lCloudProperties.mfRayStepLengthMin;

    CLOUDCOLOUR1 lfLightEnergy = GetLightEnergy(
        lCloudProperties,
        lRayVec3,
        lfAtmosphereY,
        lfSampleDensity,
        lfSampleDensityLight,
        lfCosAngle,
        lfStepSize );

    CLOUDCOLOUR3 lSunLightVec3 = lCloudProperties.mLightColourVec3 * lfLightEnergy;

    lSunLightVec3 *= lCloudProperties.mfLightScalar;
    lAmbientLightVec3 *= lCloudProperties.mfAmbientScalar;

    lParticleVec4.rgb = lSunLightVec3 + lAmbientLightVec3;
    lParticleVec4.rgb *= CLOUDCOLOUR1( lfSampleDensity );
    lParticleVec4.a = lfSampleDensity;

    return lParticleVec4;
}

STATIC_CONST int bayerArray[16] = {
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 5
};

STATIC_CONST mat4 bayerFull = mat4(
    vec4( 1, 9, 3, 11 ),
    vec4( 13, 5, 15, 7 ),
    vec4( 4, 12, 2, 10 ),
    vec4( 16, 8, 14, 6 ) );

float
GetBayer(
    uvec2 lPos )
{
    //vec2 positionMod = vec2( uvec2( tc * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) & 3 );

    uvec2 positionMod = uvec2( lPos & 3 );

    float rndoffset = bayerFull[ positionMod.x ][ positionMod.y ];
    
    return rndoffset / 17.0;
}

int
GetBayerOffset(
    uvec2 lPos )
{
    uvec2 positionMod = uvec2( lPos & 3 );
    return int( round( bayerFull[ positionMod.x ][ positionMod.y ] - 1.0 ));
}

CLOUDCOLOUR4
Render(
    in PerFrameUniforms      lCommonPerFrameUniforms,
    in CustomPerMeshUniforms lCustomPerMeshUniforms,
    in CommonPerMeshUniforms lCommonPerMeshUniforms,
    in cCloudProperties      lCloudProperties,
    in vec3                  lRayDirectionVec3,
    in vec3                  lRayStartVec3,
    in vec3                  lRayEndVec3,
    in vec3                  lRayHighVec3,
    in bool                  lbAboveHighClouds,
    in bool                  lbHasHorizon,
    in bool                  lbHighCloudsOnly,
    in vec2                  lTexCoords )
{
#if defined ( D_TEMPORAL_RENDER_HIGH )
    // Initialise the depth we're using for the temporal render step.
    //float lfTemporalDepth = 0.0;
#endif

    CLOUDCOLOUR4 lColourVec4 = CLOUDCOLOUR4( 0.0, 0.0, 0.0, 0.0 );

    float lfMipLevel = 0.0;

#if !defined ( D_RENDER_HIGH )
    float lfAtmosphereY = 0.0;
#endif
    int   liIteration = 0;
    int   liDebugMissedRays = 0;
    int   liDebugRaysOutCloud = 0;
    int   liDebugRaysInCloud = 0;
    float lfDensity = 0.0;
    float lfCloudHighDensity = 0.0;

    #if defined ( D_TEMPORAL_RENDER_HIGH )
        float lfCloudHighDepth = 0.0;
    #endif

#if !defined ( D_RENDER_HIGH )
    vec3 lRayVec3 = lRayStartVec3;
#endif

    if ( lCloudProperties.mfRayAboveHorizon > lCloudProperties.mfRayMinimumY )
    {
        vec3 lBlendWeights = abs( normalize( lRayHighVec3 ) );
        lBlendWeights /= ( lBlendWeights.x + lBlendWeights.y + lBlendWeights.z );

        vec3 lWindDirection = GetWindDirection( lBlendWeights, lCustomPerMeshUniforms.gWindOffset.zw );

        // get light energy here
        // add to alpha here
        // attenuate light energy by alpha here to get light intensity    

        vec3 lRayHighCoverageVec3 = lRayHighVec3 + ( lWindDirection * lCloudProperties.mfAnimationScale * 0.5 );
        //vec3 lRayHighCoverageVec3 = lRayHighVec3;
        vec3 lRayHighCloudVec3 = lRayHighVec3 + ( lWindDirection * lCloudProperties.mfAnimationScale );
        //vec3 lRayHighCloudVec3 = lRayHighVec3;

#if !defined(D_USE_CLOUD_MACROS)
        vec4 lCloudsHighVec4 = GetTriPlanarMap(lBlendWeights, lRayHighCloudVec3 * lCloudProperties.mfCoverageScale * 4.0, SAMPLER2DPARAM_LOCAL(lCustomPerMeshUniforms, gCloudsHigh2D));
        float lfCoverage = GetTriPlanarMapGB(lBlendWeights, lRayHighCoverageVec3 * lCloudProperties.mfCoverageScale * 1.0, SAMPLER2DPARAM_LOCAL(lCustomPerMeshUniforms, gCoverage2D)).x;
#else
        vec4 lCloudsHighVec4 = GetTriPlanarMap(lBlendWeights, lRayHighCloudVec3 * lCloudProperties.mfCoverageScale * 4.0, SAMPLER_GETLOCAL(lCustomPerMeshUniforms, gCloudsHigh2D));
        float lfCoverage = GetTriPlanarMapGB(lBlendWeights, lRayHighCoverageVec3 * lCloudProperties.mfCoverageScale * 1.0, SAMPLER_GETLOCAL(lCustomPerMeshUniforms, gCoverage2D)).x;
#endif

        lfCloudHighDensity = Remap( lCloudsHighVec4.r, lfCoverage, 1.0, 0.0, 1.0 );
        lfCloudHighDensity *= lfCloudHighDensity;

        lfCloudHighDensity = max( lfCloudHighDensity, 0.01 );

        #if defined ( D_TEMPORAL_RENDER_HIGH )
            vec3 lCameraAt = MAT4_GET_COLUMN( lCommonPerFrameUniforms.gCameraMat4, 2 ).xyz;
            vec3 lPlanetRelativeCamera = lCommonPerFrameUniforms.gViewPositionVec3 - lCommonPerMeshUniforms.gPlanetPositionVec4.xyz;
            vec4 lCameraSpaceDepth = vec4( 0.0, 0.0, -dot( lPlanetRelativeCamera - lRayHighVec3, lCameraAt ), 1.0 );
            vec4 lCurrentScreenPositionVec4 = MUL( lCommonPerFrameUniforms.gProjectionNoJitterMat4, lCameraSpaceDepth );
            lCurrentScreenPositionVec4 /= lCurrentScreenPositionVec4.w;
            lfCloudHighDepth = lCurrentScreenPositionVec4.z;
        #endif
    }

    if ( !lbHasHorizon || lCloudProperties.mfRayAboveHorizon > lCloudProperties.mfRayMinimumY )
    {
        bool  lbIsInsideCloud = false;
        float lfSampleDensityPrevious = -1.0;
        int   liZeroDensitySampleCount = 0;
        int   liSampleCount = lCloudProperties.miMaxIterations;
        float lfLOD = 1.0;

        // Start the Ray March Loop
        if ( !lbHighCloudsOnly )
        {
            float lfCosAngle = dot( lRayDirectionVec3, -lCloudProperties.mLightDirectionVec3 );

#if defined ( D_RENDER_HIGH )
            float lfStep = 0.0;

            float lfFindStep = 0.0;
            float lfEntryStep = 0.0;
            float lfRayEndPoint = length( lRayEndVec3 - lRayStartVec3 );
            const float kfBinaryPoint = 0.03125;
#endif

            while ( liIteration < liSampleCount )
            {

#ifdef D_RENDER_LOW

                float lfSampleDensity = SampleCloudMedium( lCustomPerMeshUniforms,
                    lCloudProperties,
                    lRayVec3,
                    3.0,
                    0 ); // Low Quality

                //lColourVec4 += vec4(lfSampleDensity, lfSampleDensity, lfSampleDensity, lfSampleDensity);
                //lColourVec4 += vec4(lfSampleDensity * 0.01, 0.0, 0.0, 0.0);
                lColourVec4 += CLOUDCOLOUR4(lfSampleDensity, lfSampleDensity, lfSampleDensity, lfSampleDensity);

                lRayVec3 += lRayDirectionVec3 * ( lCloudProperties.mfRayStepLengthMax );

#elif defined (D_RENDER_MED)

                float lfSampleDensity = SampleCloudMedium( lCustomPerMeshUniforms,
                    lCloudProperties,
                    lRayVec3,
                    10.0,
                    1 ); // Med Quality

                lColourVec4 += CLOUDCOLOUR4(lfSampleDensity, lfSampleDensity, lfSampleDensity, lfSampleDensity);
                //lColourVec4 += vec4(lfSampleDensity, 0.0, 0.0, 0.0);

                lRayVec3 += lRayDirectionVec3 * ( lCloudProperties.mfRayStepLengthMax );

#else

                // Generate our distance along the ray, and use that as input to a LOD'ing function.  We can now use this to calculate the ray position.
                float lfLogStep = log( lfStep * 0.02 );
                float lfLodStep = max( lfLogStep, 0.0 ) + 1.0;
                float lfExpStep = 1.0; // Try with larger exp step... ( lfStep * lfLodStep >= 2000 ) ? exp( ( lfStep * lfLodStep - 2000 ) * 0.01 ) : 1.0;
                vec3 lRayVec3 = lRayStartVec3 + lRayDirectionVec3 * lfStep * lfLodStep * lfExpStep;

                if ( lColourVec4.a > 0.99 || lfRayEndPoint <= ( lfExpStep * lfLodStep * lfStep ) )
                {
                    break;
                }

                //Prevent branch divergence by sampling medium rate, and passing this into further calculations...
                float lfAtmosphereY;
                float lfSampleMipLevel = lbIsInsideCloud ? lfMipLevel : 10;

                float lfSampleDensity = SampleCloudRoughShape(
                    lCustomPerMeshUniforms,
                    lCloudProperties,
                    lRayVec3,
                    lfSampleMipLevel,
                    lfAtmosphereY );

                if ( !lbIsInsideCloud )
                {
                    // If we are outside of a cloud, then continue
                    if ( lfSampleDensity <= 0.0 )
                    {
                        lfStep += lCloudProperties.mfRayStepLengthMax;
                        liDebugRaysOutCloud++;
                    }
                    else
                    {
                        // Previously we captured the original entry point and stepped backwards, aggregating
                        //  lighting as we went to create our clouds.  Now we're binary chopping based on the
                        //  max step length to give us a rough entry point so we can reduce view dependent effects.
                        lfFindStep = lCloudProperties.mfRayStepLengthMax * 0.5;
                        lfStep -= lfFindStep;

                        liDebugMissedRays++;
                        lbIsInsideCloud = true;
                    }
                }
                else
                {
                    #if !defined ( D_ENABLE_ROUGH_RENDER )
                    lfSampleDensity = SampleCloudFineShape( lCustomPerMeshUniforms,
                        lCloudProperties,
                        lRayVec3,
                        lfMipLevel,
                        lfSampleDensity,
                        lfAtmosphereY );
                    #endif
                    
                    lfSampleDensity *= lCloudProperties.mfSampleScalar;

                    // If we just sampled a zero and the previous sample was zero, increment the counter
                    if ( lfSampleDensity <= lCloudProperties.mfSampleThreshold )
                    {
                        if ( lfFindStep > 0.0 )
                        {
                            if ( lfFindStep <= lCloudProperties.mfRayStepLengthMax * kfBinaryPoint )
                            {
                                // We have found the leading edge (or as close as we're going to get to the cloud leading edge...)
                                lfStep += lfFindStep;
                                lfFindStep = 0.0;
                            }
                            else
                            {
                                lfFindStep *= 0.5;
                                lfStep += lfFindStep;
                            }
                        }
                        else if ( lfSampleDensityPrevious <= lCloudProperties.mfSampleThreshold )
                        {
                            ++liZeroDensitySampleCount;

                            if ( liZeroDensitySampleCount > 10 )
                            {
                                // If not then set cloud_test it zero so that we go back to cheap sample case
                                lbIsInsideCloud = false;
                                liZeroDensitySampleCount = 0;
                            }

                        }

                    }
                    else
                    {
                        if ( lfFindStep > 0.0 )
                        {
                            if ( lfFindStep <= lCloudProperties.mfRayStepLengthMax * kfBinaryPoint )
                            {
                                // We have found the leading edge (or as close as we're going to get to the cloud leading edge...)
                                lfFindStep = 0.0;
                            }
                            else
                            {
                                // This might not be the cloud edge, burn an iteration back stepping by a chop.
                                lfFindStep *= 0.5;
                                lfStep -= lfFindStep;

                            }

                        }

                        if ( lfFindStep == 0.0 )
                        {

                            // We're only doing this if we aren't searching for an edge.
                            lfDensity += lfSampleDensity;
                            float lfSampleDensityLight = SampleCloudDensityAlongCone(
                                lCustomPerMeshUniforms,
                                lCloudProperties,
                                lRayVec3,
                                lfMipLevel );

                            // get light energy here
                            // add to alpha here
                            // attenuate light energy by alpha here to get light intensity        
                            {
                                CLOUDCOLOUR4 lParticleVec4 = SampleLightParticle( lCustomPerMeshUniforms,
                                    lCloudProperties,
                                    lfSampleDensity,
                                    lfSampleDensityLight,
                                    lfCosAngle,
                                    lRayVec3,
                                    lbHasHorizon );

                                {
                                    #if defined ( D_TEMPORAL_RENDER_HIGH )
                                        // Oh why can't we get things in the right space... we're constructing our own point in camera space and projecting.
                                        //  Doing this as we were geting incorrectly transposed projection matrices at one point!
                                        //vec3 lCameraAt = MAT4_GET_COLUMN( lCommonPerFrameUniforms.gCameraMat4, 2 ).xyz;
                                        //vec3 lPlanetRelativeCamera = lCommonPerFrameUniforms.gViewPositionVec3 - lCommonPerMeshUniforms.gPlanetPositionVec4.xyz;
                                        //vec4 lCameraSpaceDepth = vec4( 0.0, 0.0, -dot( lPlanetRelativeCamera - lRayVec3, lCameraAt ), 1.0 );
                                        //vec4 lCurrentScreenPositionVec4 = MUL( lCommonPerFrameUniforms.gProjectionNoJitterMat4, lCameraSpaceDepth );
                                        //lCurrentScreenPositionVec4 /= lCurrentScreenPositionVec4.w;
                                        //lDensityMotionVectorVec4.x = lCurrentScreenPositionVec4.z;
                                    
                                        // Weight our colour and motion vectors.
                                        //lMotionVectorVec4 = ( 1.0 - lColourVec4.a ) * lDensityMotionVectorVec4 + lMotionVectorVec4;
                                    #endif // D_TEMPORAL_RENDER_HIGH
                                    
                                    lColourVec4 = ( 1.0 - lColourVec4.a ) * lParticleVec4 + lColourVec4;
                                    lColourVec4 = saturate( lColourVec4 );
                                }
                            }
                        }

                        liZeroDensitySampleCount = 0;
                    }

                    if ( lfFindStep == 0.0 )
                    {
                        lfStep += lCloudProperties.mfRayStepLengthMin;
                    }

                    lfSampleDensityPrevious = lfSampleDensity;
                    liDebugRaysInCloud++;
                }

#endif // !D_RENDER_LOW

                ++liIteration;

                lColourVec4 = saturate( lColourVec4 );

#if !defined ( D_RENDER_HIGH )
                lfAtmosphereY = NormalizedAtmosphereY(
                    lRayVec3,
                    lCloudProperties.mfAtmosphereStartRatio,
                    lCloudProperties.mfAtmosphereThickness );

                lfLOD = floor( lfAtmosphereY * 3.0 ) + 1.0;

                if ( lColourVec4.a > 0.99 || lfAtmosphereY >= 1.0 )
                {
                    break;
                }
#endif
            }

            //if ( lfDensity <= 0.003 && !lbEarlyOut )
            //{
            //    lColourVec4 = vec4( 0.0, 1.0, 0.0, 1.0 );
            //}
            //else if ( lbEarlyOut )
            //{
            //    lColourVec4 = vec4( 0.0, 1.0, 1.0, 1.0 );
            //}
        }

        if ( lfCloudHighDensity >= 0.01 )
        {
            CLOUDCOLOUR4 lParticleVec4;
            lParticleVec4.rgb = mix(
                CLOUDCOLOUR3( lCustomPerMeshUniforms.gCloudTopColour.rgb ),
                CLOUDCOLOUR3( lCustomPerMeshUniforms.gCloudBaseColour.rgb ),
                lfCloudHighDensity );
            lParticleVec4.a = lfCloudHighDensity;
            lParticleVec4.rgb /= lParticleVec4.a;

            lParticleVec4.rgb = saturate( lParticleVec4.rgb );


            if ( lbAboveHighClouds )
            {
                // There is almost no point rendering this here... if we want to integrate this properly, we need to handle z properly for this layer from this side!
                lColourVec4 = lParticleVec4.a * lParticleVec4 + ( lColourVec4 * ( 1.0 - lParticleVec4.a ) );

                #if defined ( D_TEMPORAL_RENDER_HIGH )
                //    lfTemporalDepth = lParticleVec4.a * lfCloudHighDepth + ( lfTemporalDepth * ( 1.0 - lParticleVec4.a ) );
                #endif
            }
            else
            {
                // This will render strato clouds with 1.0 alpha where the other cloud density is zero... which makes the transition in the other branch of this
                //  predicate very stark :(
                lColourVec4 = lColourVec4.a * lColourVec4 + ( lParticleVec4 * ( 1.0 - lColourVec4.a ) );

                #if defined ( D_TEMPORAL_RENDER_HIGH )
                //    lfTemporalDepth = lColourVec4.a * lfTemporalDepth + ( lfCloudHighDepth * ( 1.0 - lColourVec4.a ) );
                #endif
            }
        }

        if ( lbHasHorizon )
        {
            float lfFadeMinimumY = lCloudProperties.mfRayMinimumY;
            //float lfFadeMinimumY = 0.0;

            float fade = smoothstep( lfFadeMinimumY,
                lfFadeMinimumY + ( 1.0 - lfFadeMinimumY ) * mix( lCloudProperties.mfHorizonFadeScalar, 0.0, lCloudProperties.mfRatioAbovePlanet ),
                lCloudProperties.mfRayAboveHorizon );

            lColourVec4 *= lCloudProperties.mfHorizonFadeStartAlpha + fade * ( 1.0 - lCloudProperties.mfHorizonFadeStartAlpha );
        }
    }

    lColourVec4 = saturate( lColourVec4 );

    /*
    if ( lTexCoords.x < 0.25 )
    {
        //return vec4( DebugColour( float(liDebugRaysOutCloud + liDebugRaysInCloud + liDebugMissedRays) / 128.0, 0.0, 1.0 ).xyz, 1.0 );
        return vec4(DebugColour(float(liDebugRaysInCloud) / 128.0, 0.0, 1.0).xyz, 1.0);
        //return vec4(DebugColour( lfDensity, 0.0, 1.0).xyz, 1.0);
        //return vec4( DebugColour( float(lCloudProperties.miMaxIterations) / 128.0, 0.0, 1.0 ).xyz, 1.0 );
    }
    else
    if ( lTexCoords.x < 0.5 )
    {
        return vec4( DebugColour( float( liDebugRaysInCloud ) / 128.0, 0.0, 1.0 ).xyz, 1.0 );
        //return vec4(DebugColour(lCloudProperties.mfRayStepLengthMin / 100.0, 0.0, 1.0).xyz, 1.0);
    }
    else
    if ( lTexCoords.x < 0.75 )
    {
        return vec4( DebugColour( float( liDebugRaysOutCloud ) / 128.0, 0.0, 1.0 ).xyz, 1.0 );
        //return vec4( DebugColour(  lCloudProperties.mfRayStepLengthMax / 100.0, 0.0, 1.0 ).xyz, 1.0 );
    }
    else
    {
        return lColourVec4;
    }*/

    return lColourVec4;
}


//-----------------------------------------------------------------------------
///
///     Fragment Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------



FRAGMENT_MAIN_COLOUR01_SRT
{
    cCloudProperties lCloudProperties;

    #if defined ( D_TEMPORAL_RENDER_HIGH ) || defined ( D_CLOUD_TEMPORAL_INFILL_EDGES )

    // We're throwing away more threads than other methods of early-out, so do this first.
    //int liFullBufferWidth = int( lUniforms.mpPerFrame.gFrameBufferSizeVec4.x * TEMPORAL_GRID_WIDTH );
    //int liFullBufferHeight = int( lUniforms.mpPerFrame.gFrameBufferSizeVec4.y * TEMPORAL_GRID_WIDTH );
    int liBufferWidth = int( lUniforms.mpPerFrame.gFrameBufferSizeVec4.x );
    int liBufferHeight = int( lUniforms.mpPerFrame.gFrameBufferSizeVec4.y );

    vec2 lTexelCoordsVec2 = floor( TEX_COORDS.xy * vec2( liBufferWidth, liBufferHeight ) );
    ivec2 lTextureCoordsIVec2 = ivec2( lTexelCoordsVec2.xy );
    vec2 lTexCoords = TEX_COORDS.xy;
    #else
    vec2 lTexCoords = TEX_COORDS.xy;
    #endif

    #ifdef D_PLATFORM_ORBIS
        // We're not within the frame at all, just throw the pixels away entirely.
        if ( lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0 )
        {
            #if defined ( D_TEMPORAL_RENDER_HIGH )
            // We're increasing the field of view slightly to ensure that we can render the extra strip of pixels which will be filtered back in.
            vec4 lIncreasedFovVec4 = lUniforms.mpPerFrame.gFrustumTanFovVec4 - 0.5;
            #else
            vec4 lIncreasedFovVec4 = lUniforms.mpPerFrame.gFrustumTanFovVec4;
            #endif
        
            if ( HmdFovMask_isInvisible( lTexCoords,
                lIncreasedFovVec4,
                lUniforms.mpPerFrame.gVREyeInfoVec3 )
            )
            {
                #if defined( D_COMPUTE )
                    return;
                #else
                    discard;
                #endif
            }
        }
    #endif
    
    #if defined ( D_TEMPORAL_RENDER_HIGH )
    // Assume that we want to discard pixels if we can.
    bool lbDiscard = false;
    #else
    bool lbDiscard = false;
    #endif

    #if defined ( D_TEMPORAL_RENDER_HIGH )
    {
        // Check whether we're not on the temporal cloudstep, if not, we might be able to skip this pixel.
        lCloudProperties.muiTemporalCloudStep = int( floor( lUniforms.mpCustomPerMesh.gAnimationParamsVec4.w + 0.25 ) );
        ivec2 lGridOffsetsIVec2 = lTextureCoordsIVec2 & 0x3;
        ivec2 lGridMaskIVec2 = lTextureCoordsIVec2 >> 2;

        // Swizzle the pattern, 4x4 grid per grid to give us a less obvious pattern flying through clouds etc.
        int liOffset = lGridOffsetsIVec2.y * TEMPORAL_GRID_WIDTH + lGridOffsetsIVec2.x;
        int liNumBlocks = int( lUniforms.mpPerFrame.gFrameBufferSizeVec4.x * 0.25 );

        int liMaskA = lGridMaskIVec2.x;
        int liMaskB = ( lGridMaskIVec2.y * 619 );
        liOffset = liOffset + ( ( ( liMaskA + liMaskB ) * 619 ) / 151 );
        liOffset = bayerArray[ liOffset & 0xf ];

        if ( liOffset != lCloudProperties.muiTemporalCloudStep )
        {
            // Check whether we need to carry out an infill step.  Originally this section was a larger part of the D_CLOUD_TEMPORAL_INFILL_EDGES context,
            // but we're rendering this with the temporal step now.  Check whether we want to infill the edges here.

            //vec3 lScreenCoordinatesVec3 = vec3( lTexCoords.x * 2.0 - 1.0, ( 1.0 - lTexCoords.y ) * 2.0 - 1.0, 0.0 );
            //vec4 lPrevScreenCoordinatesVec4 = MUL( lUniforms.mpPerFrame.gReprojectionNoJitterMat4, vec4( lScreenCoordinatesVec3, 1.0 ) );
            //lPrevScreenCoordinatesVec4 /= lPrevScreenCoordinatesVec4.w;

            //vec4 lPrevTextureCoordsVec4 = lPrevScreenCoordinatesVec4;

            //lPrevTextureCoordsVec4.x = 0.5 * lPrevTextureCoordsVec4.x + 0.5;
            //lPrevTextureCoordsVec4.y = 1.0 - ( 0.5 * lPrevTextureCoordsVec4.y + 0.5 );
            
            float lfReverseZDepth = LinearNormToReverseZDepth(
                lUniforms.mpPerFrame.gClipPlanesVec4,
                1.0 );

            vec4 lWorldPositionVec4;
            lWorldPositionVec4.xyz = RecreatePositionFromRevZDepth(
                lfReverseZDepth,
                lTexCoords,
                lUniforms.mpPerFrame.gViewPositionVec3,
                lUniforms.mpPerFrame.gInverseViewProjectionNoJitterMat4 );
            lWorldPositionVec4.w = 1.0;

            lWorldPositionVec4 = MUL(
                lUniforms.mpPerFrame.gPrevViewProjectionNoJitterMat4,
                lWorldPositionVec4 );

            lWorldPositionVec4 /= lWorldPositionVec4.w;

            vec4 lPrevScreenCoordinatesVec4 = lWorldPositionVec4;
            vec2 lPrevTextureCoordsVec4 = lWorldPositionVec4.xy;
            lPrevTextureCoordsVec4 = 0.5 * lPrevTextureCoordsVec4 + 0.5;
            lPrevTextureCoordsVec4.y = 1.0 - lPrevTextureCoordsVec4.y;

            float lPixelX = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z;
            float lPixelY = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w;

            float lfMinX = 0.0;
            float lfMinY = 0.0;
            float lfMaxX = 1.0;
            float lfMaxY = 1.0;

            bool lbInfillX = lPrevTextureCoordsVec4.x <= lfMinX || lPrevTextureCoordsVec4.x >= lfMaxX;
            bool lbInfillY = lPrevTextureCoordsVec4.y <= lfMinY || lPrevTextureCoordsVec4.y >= lfMaxY;

            if ( !lbInfillX && !lbInfillY )
            {
#if defined( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }

        // How much more of the buffer do we need to render.
        //lbDiscard = false;

        // Our original implementation of the temporal render full raycast step had us rendering into a smaller buffer.
        //  This didn't seem to get us a faster renderer, possibly since we gated on memory and branch divergence.
        //  The code below the commented out section discards threads, but seems to give us better times.
        //{
        //    // We need to generate the correct offset in the temporal grid and offset the texture coordinates for that.
        //    lCloudProperties.muiTemporalCloudStep = int( floor( lUniforms.mpCustomPerMesh.gAnimationParamsVec4.w + 0.5 ) );
        //    int liTexU = lCloudProperties.muiTemporalCloudStep & 3;
        //    int liTexV = lCloudProperties.muiTemporalCloudStep / 4;
        //    lTexCoords.xy = floor( lTexCoords.xy * vec2( liBufferWidth, liBufferHeight ) );
        //    lTexCoords.xy *= TEMPORAL_GRID_WIDTH;
        //    lTexCoords.xy += vec2( liTexU, liTexV );
        //    lTexCoords.xy /= vec2( liFullBufferWidth, liFullBufferHeight );
        //}
    }
    #endif

    CLOUDCOLOUR4     lCloudColourVec4;
    vec3             lPlanetRelativeCamera;
    vec3             lRayStartVec3;
    vec3             lRayEndVec3;

    vec3             lRayStartOuterVec3 = vec3( 0.0, 0.0, 0.0 );
    vec3             lRayEndOuterVec3 = vec3( 0.0, 0.0, 0.0 );

    vec3             lRayHighVec3;
    vec3             lRayDirectionVec3;
    float            lfCloudDepth = 0.0;

    float            lfPlanetRadius = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;
    bool             lbHasHorizon = false;
    bool             lbHighCloudsOnly = false;

    CLOUDCOLOUR3     lDebugColour = CLOUDCOLOUR3( 1.0, 0.0, 0.0 );
    bool             lbDebug = false;
    
    {
        lCloudProperties.miMaxIterations = int( lUniforms.mpCustomPerMesh.gCoverageParamsVec4.x );
        lCloudProperties.mfHenyeyBalancePoint = lUniforms.mpCustomPerMesh.gLightingParamsVec4.z;

        lCloudProperties.mLightConeStepVec3 = vec3(
            lUniforms.mpCustomPerMesh.gLightConeParamsVec4.x,
            lUniforms.mpCustomPerMesh.gLightConeParamsVec4.y,
            lUniforms.mpCustomPerMesh.gLightConeParamsVec4.z );

        lCloudProperties.mfAnimationScale = lUniforms.mpCustomPerMesh.gAnimationParamsVec4.x * lUniforms.mpPerFrame.gfTime;
        lCloudProperties.mfCloudRatio = lUniforms.mpCustomPerMesh.gAnimationParamsVec4.y;
        lCloudProperties.mfCloudStrengthModifier = lUniforms.mpCustomPerMesh.gAnimationParamsVec4.z;

        lCloudProperties.mfAtmosphereStartHeight = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.y;
        lCloudProperties.mfAtmosphereEndHeight = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.z;
        lCloudProperties.mfStratosphereHeight = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.w;

        #if !defined ( D_PLATFORM_SWITCH )

        lCloudProperties.mfHorizonCoverageStart = lUniforms.mpCustomPerMesh.gCoverageParamsVec4.y;
        lCloudProperties.mfHorizonCoverageEnd = lUniforms.mpCustomPerMesh.gCoverageParamsVec4.z;
        lCloudProperties.mfAmbientDensity = lUniforms.mpCustomPerMesh.gCoverageParamsVec4.w;

        lCloudProperties.mfLightScalar = lUniforms.mpCustomPerMesh.gLightingParamsVec4.x;
        lCloudProperties.mfAmbientScalar = lUniforms.mpCustomPerMesh.gLightingParamsVec4.y;

        lCloudProperties.mfDensity = lUniforms.mpCustomPerMesh.gLightScatteringParamsVec4.x;
        lCloudProperties.mfForwardScatteringG = lUniforms.mpCustomPerMesh.gLightScatteringParamsVec4.y;
        lCloudProperties.mfBackwardScatteringG = lUniforms.mpCustomPerMesh.gLightScatteringParamsVec4.z;
        lCloudProperties.mfDarkOutlineScalar = lUniforms.mpCustomPerMesh.gLightScatteringParamsVec4.w;

        lCloudProperties.mfBaseScaleScalar = lUniforms.mpCustomPerMesh.gModelingBaseParamsVec4.x;
        lCloudProperties.mfSampleScalar = lUniforms.mpCustomPerMesh.gModelingBaseParamsVec4.y;
        lCloudProperties.mfSampleThreshold = lUniforms.mpCustomPerMesh.gModelingBaseParamsVec4.z;
        lCloudProperties.mfCloudBottomFade = lUniforms.mpCustomPerMesh.gModelingBaseParamsVec4.w;

        lCloudProperties.mfDetailScale = lUniforms.mpCustomPerMesh.gModelingDetailParamsVec4.x;
        lCloudProperties.mfErosionEdgeSize = lUniforms.mpCustomPerMesh.gModelingDetailParamsVec4.y;
        lCloudProperties.mfCloudDistortion = lUniforms.mpCustomPerMesh.gModelingDetailParamsVec4.z;
        lCloudProperties.mfCloudDistortionScale = lUniforms.mpCustomPerMesh.gModelingDetailParamsVec4.w;

        lCloudProperties.mfRayMinimumY = lUniforms.mpCustomPerMesh.gOptimisationParamsVec4.x;
        lCloudProperties.mfLODDistance = lUniforms.mpCustomPerMesh.gOptimisationParamsVec4.y;
        lCloudProperties.mfHorizonFadeStartAlpha = lUniforms.mpCustomPerMesh.gOptimisationParamsVec4.z;
        lCloudProperties.mfHorizonFadeScalar = lUniforms.mpCustomPerMesh.gOptimisationParamsVec4.w;	// Fades clouds on horizon, 1.0 . 10.0 (1.0 = smooth fade, 10 = no fade)

        lCloudProperties.mfHorizonDistance = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.x;

        lCloudProperties.mfLightConeRandomWalk = lUniforms.mpCustomPerMesh.gLightConeParamsVec4.w;

        #else

        lCloudProperties.mfHorizonCoverageStart = kfHorizonCoverageStart;
        lCloudProperties.mfHorizonCoverageEnd = kfHorizonCoverageEnd;
        lCloudProperties.mfAmbientDensity = kfAmbientDensity;

        lCloudProperties.mfLightScalar = kfLightScalar;
        lCloudProperties.mfAmbientScalar = kfAmbientScalar;

        lCloudProperties.mfDensity = kfDensity;
        lCloudProperties.mfForwardScatteringG = kfForwardScatteringG;
        lCloudProperties.mfBackwardScatteringG = kfBackwardScatteringG;
        lCloudProperties.mfDarkOutlineScalar = kfDarkOutlineScalar;
        
        lCloudProperties.mfBaseScaleScalar = kfBaseScale;
        lCloudProperties.mfSampleScalar = kfSampleScalar;
        lCloudProperties.mfSampleThreshold = kfSampleThreshold;
        lCloudProperties.mfCloudBottomFade = kfCloudBottomFade;

        lCloudProperties.mfDetailScale = kfDetailScale;
        lCloudProperties.mfErosionEdgeSize = kfErosionEdgeSize;
        lCloudProperties.mfCloudDistortion = kfCloudDistortion;
        lCloudProperties.mfCloudDistortionScale = kfCloudDistortionScale;

        lCloudProperties.mfRayMinimumY = kfRayMinimumY;
        lCloudProperties.mfLODDistance = kfLODDistance;
        lCloudProperties.mfHorizonFadeStartAlpha = kfHorizonFadeStartAlpha;
        lCloudProperties.mfHorizonFadeScalar = kfHorizonFadeScalar;	// Fades clouds on horizon, 1.0 . 10.0 (1.0 = smooth fade, 10 = no fade)

        lCloudProperties.mfHorizonDistance = kfHorizonDistance;

        #endif
    }

    // Calculated Values
    {
        lCloudProperties.mfEarthRadius = lfPlanetRadius;
        // Do the opposite of this M_CALCULATE_PLANET_RADIUS
        lCloudProperties.mfHorizonDistance = sqrt( ( ( lCloudProperties.mfEarthRadius + lCloudProperties.mfAtmosphereStartHeight )*lCloudProperties.mfAtmosphereStartHeight*2.0 ) - ( lCloudProperties.mfAtmosphereStartHeight*lCloudProperties.mfAtmosphereStartHeight ) );

        lCloudProperties.mfAtmosphereThickness = lCloudProperties.mfAtmosphereEndHeight - lCloudProperties.mfAtmosphereStartHeight;
        lCloudProperties.mfAtmosphereThickness = 1.0 / lCloudProperties.mfAtmosphereThickness;
        lCloudProperties.mfAtmosphereStartRatio = ( lCloudProperties.mfEarthRadius + lCloudProperties.mfAtmosphereStartHeight ) * lCloudProperties.mfAtmosphereThickness;

        //lCloudProperties.mfEarthRadius                   = M_CALCULATE_PLANET_RADIUS( lCloudProperties.mfAtmosphereStartHeight, lCloudProperties.mfHorizonDistance );
        lCloudProperties.mfMaxDistance = M_CALCULATE_HORIZON_DISTANCE( lCloudProperties.mfEarthRadius, ( lCloudProperties.mfEarthRadius + lCloudProperties.mfAtmosphereEndHeight ) );
        lCloudProperties.mfBaseScale = 1.0 / lCloudProperties.mfAtmosphereEndHeight * lCloudProperties.mfBaseScaleScalar;
        lCloudProperties.mfCoverageScale = 1.0 / lCloudProperties.mfMaxDistance;


    }


    lPlanetRelativeCamera = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

    float lfFrameIndex = mod( lUniforms.mpCustomPerMesh.gCloudSubFrameParamsVec4.x, 16.0 );

    #if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_VULKAN )
    if ( lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0 )
    {
        // FIXME: NIC, Move this up above the block...
    #if defined( D_PLATFORM_ORBIS )
        if ( HmdFovMask_isInvisible( lTexCoords,
            lUniforms.mpPerFrame.gFrustumTanFovVec4,
            lUniforms.mpPerFrame.gVREyeInfoVec3 )
            )
    #elif defined( D_PLATFORM_VULKAN )
        // This needs to be less than the vulkan depth min used in the DEPTH_DOWN stage before this...
        float lfDepth = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoords, 0.0 ).r;
        if ( lfDepth < VULKAN_DEPTH_MIN )
    #endif
        {
            WRITE_FRAGMENT_COLOUR0( vec4( 0.0,0.0,0.0,0.0 ) );
            WRITE_FRAGMENT_COLOUR1( EncodeDepthToColour( 0.0 ) );
            return;
        }
    }
    #endif

    //kfHackDebugScreenSide = lTexCoords.x;


//     {
//         uvec2 luPixelCoordVec2 = uvec2( round( lTexCoords * vec2(1920.0,1080.0) ) );
//         luPixelCoordVec2 += InverseBayer( uint( floor( lfFrameIndex ) ) );
// 
//         lTexCoords = vec2(luPixelCoordVec2) * (1.0/vec2(1920.0,1080.0));
//     }
// 
//     if ( int(lfFrameIndex) != int(mod(lTexCoords.x * 960.0, 4.0)) )
//     {
//         discard;
//         //FRAGMENT_COLOUR = vec4( 1.0, 0.0, 0.0, 1.0 );
//         //return;
//     }

#ifndef D_PLATFORM_OPENGL   
    vec2 lScreenCoordinatesVec2 = vec2( lTexCoords.x * 2.0 - 1.0, ( 1.0f - lTexCoords.y ) * 2.0 - 1.0 );
#else
    vec2 lScreenCoordinatesVec2 = vec2( lTexCoords.x * 2.0 - 1.0, lTexCoords.y * 2.0 - 1.0 );
#endif

#if defined ( D_TEMPORAL_RENDER_HIGH )
    // We want to generate a proper z and motion buffer at some stage, so it would be good to get a sensible cloud rendered in relatively the same space as the world.
    // lRayDirectionVec3 = GetWorldSpaceViewDir( lScreenCoordinatesVec2, lUniforms.mpPerFrame.gInverseViewProjectionNoJitterMat4, lUniforms.mpPerFrame.gInverseViewMat4 );
    vec3 lScreenPositionVec3 = GetWorldSpaceCoordinatesOfScreenPixel( lScreenCoordinatesVec2, lUniforms.mpPerFrame.gInverseViewProjectionNoJitterMat4 );    
    lRayDirectionVec3 = normalize( lScreenPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3 );
#else
    lRayDirectionVec3 = GetWorldSpaceViewDir( lScreenCoordinatesVec2, lUniforms.mpPerFrame.gInverseProjectionMat4, lUniforms.mpPerFrame.gInverseViewMat4 );
#endif

    float lfCloudHeightMin = lCloudProperties.mfAtmosphereStartHeight;
    float lfCloudHeightMax = lCloudProperties.mfAtmosphereEndHeight;
    float lfStratoCloudsHeight = lCloudProperties.mfStratosphereHeight;

    float lfHeightAbovePlanet = length( lPlanetRelativeCamera ) - lfPlanetRadius;

    lRayStartVec3 = normalize( lPlanetRelativeCamera ) * ( lCloudProperties.mfEarthRadius + lfHeightAbovePlanet );

    {
        float lfDeterminantMin = 0.0;
        float lfDeterminantMax = 0.0;
        float lfDeterminantHigh = 0.0;

        vec3 lIntersectionMinNear;
        vec3 lIntersectionMinFar;
        vec3 lIntersectionMaxNear;
        vec3 lIntersectionMaxFar;
        vec3 lIntersectionHighNear;
        vec3 lIntersectionHighFar;
        vec3 lCameraAt = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 ).xyz;

        lfDeterminantMin = GetRayIntersectionPointFast( lRayStartVec3, lRayDirectionVec3, lCloudProperties.mfEarthRadius + lfCloudHeightMin, lIntersectionMinNear, lIntersectionMinFar );
        lfDeterminantMax = GetRayIntersectionPointFast( lRayStartVec3, lRayDirectionVec3, lCloudProperties.mfEarthRadius + lfCloudHeightMax, lIntersectionMaxNear, lIntersectionMaxFar );
        lfDeterminantHigh = GetRayIntersectionPointFast( lRayStartVec3, lRayDirectionVec3, lCloudProperties.mfEarthRadius + lfStratoCloudsHeight, lIntersectionHighNear, lIntersectionHighFar );

        if ( lfHeightAbovePlanet < lfCloudHeightMin )
        {
            // This section handles camera under the cloud minimum height layer.

            /*
            float lfDepth = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gDepthMap), IN(mTexCoordsVec2)).r;

            if (lfDepth != 0.0)
            {
                lbDiscard = true;
            }
            */

            // On planet, below cloud layer
            if ( lfDeterminantMin == 0.0 )
            {
                //discard;
                lbDiscard = true;
            }

            lRayStartVec3 = lIntersectionMinFar;
            lRayEndVec3 = lIntersectionMaxFar;
            lRayHighVec3 = lIntersectionHighFar;
            lbHasHorizon = true;

            lfCloudDepth = dot( lPlanetRelativeCamera - lRayStartVec3, lCameraAt );
        }
        else
        if ( lfHeightAbovePlanet > lfCloudHeightMax )
        {
            // This section handles camera above planet, outside of ray marched cloud layer

            if ( lfDeterminantMax == 0.0 ||
                dot( lIntersectionMaxNear - lRayStartVec3, lRayDirectionVec3 ) < 0.0 )
            {
                // We do not intersect with the outer sphere at all (no ray marched clouds)
                lbHighCloudsOnly = true;
            }

            if ( lfHeightAbovePlanet > lfStratoCloudsHeight )
            {
                if ( lfDeterminantHigh == 0.0 ||
                     dot( lIntersectionHighNear - lRayStartVec3, lRayDirectionVec3 ) < 0.0 )
                {
                    // We don't intersect with any clouds
                    //discard;
                    //lbDebug = true;
                    WRITE_FRAGMENT_COLOUR0( vec4( 0.0, 0.0, 0.0, 0.0 ) );
                    WRITE_FRAGMENT_COLOUR1( EncodeDepthToColour( 1.0 ) );
                    return;
                }
            }


            if ( lfDeterminantMin == 0.0 )
            {
                // We don't intersect with the inner sphere
                lRayEndVec3 = lIntersectionMaxFar;
                lfCloudDepth = lUniforms.mpPerFrame.gClipPlanesVec4.y - 100.0;

            }
            else
            {
                // We do intersect with the inner sphere
                lRayEndVec3 = lIntersectionMinNear;
                lfCloudDepth = dot( lPlanetRelativeCamera - lIntersectionMinNear, lCameraAt );
            }


            lRayStartVec3 = lIntersectionMaxNear;

            if ( lfHeightAbovePlanet > lfStratoCloudsHeight )
            {
                lRayHighVec3 = lIntersectionHighNear;
                //lbDebug = true; 
            }
            else
            {
                lRayHighVec3 = lIntersectionHighFar;
            }

        }
        else
        {
            // In the ray marched clouds
            if ( lfDeterminantMin == 0.0 ||
                 dot( lIntersectionMinNear - lRayStartVec3, lRayDirectionVec3 ) < 0.0 )
            {
                // If we don't intersect with the minimum cloud layer, or the near intersection point is behind us...
                lRayEndVec3 = lIntersectionMaxFar;
            }
            else
            {
                // If bow intersection points with the minimum cloud layer are in front of the start position of the ray.
                lRayEndVec3 = lIntersectionMinNear;
                lRayStartOuterVec3 = lIntersectionMinFar;   // Need to look at this, as the outer vector is going to start at the intersection of the min cloud layer, which feels very wrong...
                lRayEndOuterVec3 = lIntersectionMaxNear;

                // No two pass...
                //lbTwoPass = true;
            }

            lRayHighVec3 = lIntersectionHighFar;

            //lfCloudDepth = abs(lPlanetRelativeCamera - lRayStartVec3).z;
            //lfCloudDepth = length(lRayEndVec3);
            lfCloudDepth = 0.0;
        }

        float lfRayLength = length( lRayEndVec3 - lRayStartVec3 );

        /*
        float lfRayStepLengthMin = lCloudProperties.mfAtmosphereThickness / (float(lCloudProperties.miMaxIterations) * 0.5);
        int liNumIterations      = min( lCloudProperties.miMaxIterations, int( lfRayLength / lfRayStepLengthMin) );
        lfRayLength = min( lfRayLength, lCloudProperties.mfAtmosphereThickness * 8.0 );
        lCloudProperties.mfRayStepLengthMin = lfRayLength / float(liNumIterations * 4.0);
        lCloudProperties.mfRayStepLengthMax = lfRayLength / (float(liNumIterations) * 0.5);
        lCloudProperties.miMaxIterations    = liNumIterations;
        */

#if defined (D_PLATFORM_ORBIS) || defined ( D_PLATFORM_SWITCH )
        // We want to scale the steps by 1 / dot( ray, unitz ) so that our sample points align in screen space. Please note, we want to keep the ray length
        //  roughly the same otherwise we get shearing in the cloud layer as different rays march through at different rates.  We're keeping an effective
        //  spherical projection... as have tried bending the projection by shearing the step length in proportion to the distance from the centre of the 
        //  screen, but this creates other visual distortions.
        lCloudProperties.mfRayStepLengthMax = CLOUD_MAX_STEP;
        lCloudProperties.mfRayStepLengthMin = CLOUD_MIN_STEP;
#else
        // For other platforms we're giving the chance for a user to decrease the step size significantly.  128 iterations is really the limit before we
        //  see rippling and other visual distortions... so better GPUs can step this up as needed.

        const float lfNumIterations = float( lCloudProperties.miMaxIterations );
        const float lfDefaultNumIterations = 128.0;
        const float lfIterationScale = lfDefaultNumIterations / lfNumIterations;

        lCloudProperties.mfRayStepLengthMax = CLOUD_MAX_STEP * lfIterationScale;
        lCloudProperties.mfRayStepLengthMin = CLOUD_MIN_STEP * lfIterationScale;
#endif
    }

    if ( lbDiscard )
    {
        lCloudColourVec4 = CLOUDCOLOUR4( 0.0, 0.0, 0.0, 0.0 );
        lfCloudDepth = 0.0;
    }
    else
    {
        vec3 lNormalizedPlanetRelativeCamera;
        vec3 lBlendWeights;

        lNormalizedPlanetRelativeCamera = normalize( lPlanetRelativeCamera );

        //lBlendWeights = pow(abs(normalize(lRayStartVec3)), vec3(32.0, 32.0, 32.0));
        lBlendWeights = pow( abs( normalize( lRayStartVec3 ) ), vec3( 32.0, 32.0, 32.0 ) );

        lBlendWeights /= ( lBlendWeights.x + lBlendWeights.y + lBlendWeights.z );

        float lfDistanceFromPlanetCenter = length( lPlanetRelativeCamera );
        float lfOppositeSide = sqrt( lfDistanceFromPlanetCenter*lfDistanceFromPlanetCenter - lfPlanetRadius * lfPlanetRadius );
        float lfCosB = dot( lRayDirectionVec3, lNormalizedPlanetRelativeCamera );
        float lfSinC = lfOppositeSide / lfDistanceFromPlanetCenter;

        lCloudProperties.mfRayMinimumY = -lfSinC;
        lCloudProperties.mfRayAboveHorizon = lfCosB;
        lCloudProperties.mCameraPositionVec3 = lNormalizedPlanetRelativeCamera * ( lCloudProperties.mfEarthRadius + lfHeightAbovePlanet );
        lCloudProperties.mLightDirectionVec3 = -lUniforms.mpCustomPerMesh.gSunPositionVec4.xyz;//lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz;
        lCloudProperties.mTriplanarBlendWeights = lBlendWeights;
        lCloudProperties.mfRatioAbovePlanet = saturate( lfHeightAbovePlanet / lCloudProperties.mfAtmosphereEndHeight );
        lCloudProperties.mLightColourVec3 = CLOUDCOLOUR3( lUniforms.mpCommonPerMesh.gLightColourVec4.rgb );

        lCloudProperties.mWindDirection = GetWindDirection( lCloudProperties.mTriplanarBlendWeights, lUniforms.mpCustomPerMesh.gWindOffset.xy );

        /*
        vec2 lBlueNoiseCoords = lTexCoords * vec2((lUniforms.mpPerFrame.gFrameBufferSizeVec4.x) / 512.0, (lUniforms.mpPerFrame.gFrameBufferSizeVec4.y) / 512.0);
        float ditherValue = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBlueNoiseMap), lBlueNoiseCoords, 0.0).x;
        ditherValue = (ditherValue * 2.0) -1.0;
        ditherValue = sign(ditherValue)*(1.0 - sqrt(1.0 - abs(ditherValue)));
        */
        float ditherValue = GetBayer( uvec2( lTexCoords * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) );

        //vec2 lTextureResolutionVec2 = GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ) );
        //float ditherValue = GetBayer( uvec2( lTexCoords * lTextureResolutionVec2.xy ) );

        lRayStartVec3 += lRayDirectionVec3 * lCloudProperties.mfRayStepLengthMax * ditherValue;

        //lCloudColourVec4 = vec4( ditherValue, ditherValue, ditherValue, 1.0 );

        bool lbAboveHighClouds = lfHeightAbovePlanet > lfStratoCloudsHeight;

        lCloudColourVec4 = Render(
            DEREF_PTR( lUniforms.mpPerFrame ),
            DEREF_PTR( lUniforms.mpCustomPerMesh ),
            DEREF_PTR( lUniforms.mpCommonPerMesh ),
            lCloudProperties,
            lRayDirectionVec3,
            lRayStartVec3,
            lRayEndVec3,
            lRayHighVec3,
            lbAboveHighClouds,
            //false,
            lbHasHorizon,
            //true,
            lbHighCloudsOnly,
            lTexCoords );

        /*
        if ( lbTwoPass )
        {
            float lfRayLength                   = length( lRayEndOuterVec3 - lRayStartOuterVec3 );
            int liNumIterations                 = min( lCloudProperties.miMaxIterations, int( lfRayLength / lCloudProperties.mfRayStepLengthMin ) );
            lCloudProperties.mfRayStepLengthMax = lfRayLength / liNumIterations;
            lCloudProperties.miMaxIterations    = liNumIterations;

            vec4 lCloudOuterColourVec4 = Render(    DEREF_PTR(lUniforms.mpCustomPerMesh),
                                                    lCloudProperties,
                                                    lRayDirectionVec3,
                                                    lRayStartOuterVec3,
                                                    lRayHighVec3,
                                                    lbAboveHighClouds,
                                                    true,
                                                    //false,
                                                    lbHighCloudsOnly,
                                                    lTexCoords );

            //lCloudColourVec4 = lCloudOuterColourVec4;

            //lCloudColourVec4 = lCloudOuterColourVec4.a * lCloudOuterColourVec4 + (lCloudColourVec4 * (1.0 - lCloudOuterColourVec4.a));
            lCloudColourVec4 = lCloudColourVec4.a * lCloudColourVec4 + (lCloudOuterColourVec4 * (1.0 - lCloudColourVec4.a));
        }*/


        lCloudColourVec4.a *= 1.0 - smoothstep( lfStratoCloudsHeight * 4.0, lfStratoCloudsHeight * 5.0, lfHeightAbovePlanet );
        lCloudColourVec4.rgb *= lCloudColourVec4.a;

        float lfDarkSide = dot( normalize( lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ), lCloudProperties.mLightDirectionVec3 ) * 0.35 + 0.65;
        lCloudColourVec4.rgb *= lfDarkSide;

        //FRAGMENT_COLOUR = vec4(0.0);

        //float lfFrameIndex = 0.0;
        //lTexCoords += InverseBayer( lfFrameIndex ) * (lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw * 0.25);  
        //lTexCoords += InverseBayer( lfFrameIndex ) * (1.0/(vec2( 1920.0, 1080.0 ) * 0.5));

        //flost lfPixelCoordX = lTexCoords.x * 960.0;

//         {
//             uint luPixelCoordX = uint(round(mod(lTexCoords.x * 960.0, 16.0)));
//             luPixelCoordX += InverseBayer(uint(floor(lfFrameIndex))).x;
// 
//             vec2 lModTex = mod((lTexCoords * vec2(1920.0, 1080.0) * 0.5), 16.0) * (1.0 / 16.0);
//             vec2 lModCheck = mod((lTexCoords * vec2(1920.0, 1080.0) * 0.5), 32.0) * (1.0 / 32.0);
//             float lfCheck = (lModCheck.x < 0.5 && lModCheck.y < 0.5) || (lModCheck.x > 0.5 && lModCheck.y > 0.5) ? 0.0 : 1.0;
// 
//             //lModTex *= lfCheck;
// 
//             //FRAGMENT_COLOUR = vec4( lModTex.x, lModTex.y, 1.0 - lfCheck, 1.0 );
// 
//             //vec4 lFragCol = RandomColour( int( lModTex.x * 16.0 ) );
//             vec4 lFragCol = RandomColour(luPixelCoordX);
//             //vec4 lFragCol = RandomColour( int( (lTexCoords.x * 1920.0 ) + lfFrameIndex ) );
// 
//             lFragCol = mix(lFragCol, vec4(0.5), lfCheck);
// 
//             FRAGMENT_COLOUR = vec4(lFragCol.xyz, 1.0);
//         }
    }

    // Write our fragments out...
    WRITE_FRAGMENT_COLOUR0( vec4( lCloudColourVec4 ) );
    
    float lfDepth = FastNormaliseDepth( lUniforms.mpPerFrame.gClipPlanesVec4, max( lfCloudDepth, 40.0 ) );
    WRITE_FRAGMENT_COLOUR1( EncodeDepthToColour( lfDepth ) );
    
    //WRITE_FRAGMENT_COLOUR0( vec4( 1.0, 0.0, 0.0, 1.0 ) );
    //WRITE_FRAGMENT_COLOUR1( float2vec4( 40.0 ) );
}

#endif // D_CLOUD_RENDER


#include "Common/CommonPostProcess.shader.h"

#ifdef D_CLOUD_TEMPORAL

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

/*
#define TAA_GET_LUMINANCE( col ) ( 0.25 * ( col.r + col.b ) + 0.5 * col.g )

vec3 SimpleUnReinhard( in vec3 col )
{
    return col / ( 1.0 - TAA_GET_LUMINANCE( col ) );
}
*/

//-----------------------------------------------------------------------------
///
///     GetPrevPosition
///
///     @brief      GetPrevPosition
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec2 GetPrevPositionSimple(
    in vec2 lFragCoordsVec2,
    in vec4 lClipPlanesVec4,
    in mat4 lInverseProjectionMat4,
    in mat4 lInverseViewMat4,
    in mat4 lProjectionMat4,
    in mat4 lViewProjectionMat4,
    in mat4 lPrevViewProjectionMat4,
    in vec3 lViewPositionVec3,
    in vec4 lMBlurSettingsVec4,
    //SAMPLER2DARG( lMotionMap ),
    //SAMPLER2DARG( lDepthMap ),
    out float lfOutDepth,
    out float lfOutDepthNormalised,
    out float lfOutRcpSpeed,
    out vec2 lOutDelta,
    out vec2 lOutEncodedDelta,
    out bool lWantsColorClipAA )
{
    vec2 lReprojectFragCoordsVec2;


    vec4 lPositionVec4;
    lPositionVec4.x = lFragCoordsVec2.x * 2.0 - 1.0;

#ifndef D_PLATFORM_OPENGL    
    lPositionVec4.y = ( 1.0f - lFragCoordsVec2.y ) * 2.0 - 1.0;
#else
    lPositionVec4.y = lFragCoordsVec2.y * 2.0 - 1.0;
#endif

    lPositionVec4.z = 0.0;
    lPositionVec4.w = 1.0;

    // Inverse projection
    lPositionVec4 = MUL( lInverseProjectionMat4, lPositionVec4 );
    lPositionVec4 = lPositionVec4 / lPositionVec4.w;

    // Inverse view
    mat4 lViewMat = lInverseViewMat4;
    MAT4_SET_POS( lViewMat, vec4( 0.0, 0.0, 0.0, 1.0 ) );
    lPositionVec4 = MUL( lViewMat, lPositionVec4 );

    vec3 lViewVectorVec3 = normalize( lPositionVec4.xyz );
    //lPositionVec4.xyz = lViewVectorVec3 * lfDepth + lViewPositionVec3;
    lPositionVec4.xyz = lViewVectorVec3 * lClipPlanesVec4.y + lViewPositionVec3;
    lPositionVec4.w = 1.0;

    lPositionVec4 = MUL( lPrevViewProjectionMat4, lPositionVec4 );

#ifndef D_PLATFORM_OPENGL    
    lPositionVec4.y = -lPositionVec4.y;
#endif

    lPositionVec4.xyz /= lPositionVec4.w;
    lReprojectFragCoordsVec2 = lPositionVec4.xy;
    lReprojectFragCoordsVec2 = lReprojectFragCoordsVec2 * 0.5 + 0.5;

    return lReprojectFragCoordsVec2;

}

#define GET_LUMINANCE( col ) ( 0.25 * ( col.r + col.b ) + 0.5 * col.g )

vec4
clip_aabb(
    vec4 q,
    vec4 aabb_min,
    vec4 aabb_max )
{
    // note: only clips towards aabb center (but fast!)
    vec4 p_clip = 0.5 * ( aabb_max + aabb_min );
    vec4 e_clip = 0.5 * ( aabb_max - aabb_min );

    vec4 v_clip = q - p_clip;
    vec4 v_unit = e_clip / v_clip;
    vec4 a_unit = abs( v_unit );
    float ma_unit = saturate( min( a_unit.x, min( a_unit.y, a_unit.z ) ) );

    return p_clip + v_clip * ma_unit;
}

#if !defined( D_PLATFORM_ORBIS )

float rcp( float inX )
{
    return 1.0 / inX;
}

#endif

FRAGMENT_MAIN_COLOUR_SRT
{
    /*
     vec4 cloud;
     //cloud.xyz = SimpleUnReinhard( texture2D( lUniforms.mpCustomPerMesh.gBuffer3Map, IN( mTexCoordsVec2 ) ).xyz );
     cloud.xyz = texture2D( lUniforms.mpCustomPerMesh.gBuffer3Map, IN( mTexCoordsVec2 ) ).xyz;
     cloud.a = 1.0;
     FRAGMENT_COLOUR = cloud;

     return;
     */
     /*
     vec4  lFragmentColourVec4 = texture2D( lUniforms.mpCustomPerMesh.gBuffer2Map, IN( mTexCoordsVec2 ) );

     if ( lFragmentColourVec4.a <= 0.0 )
     {
         discard;

     }
     */

     /*
     float lfDepth      = DecodeDepthFromColour( texture2D( lUniforms.mpCustomPerMesh.gDepthMap, IN( mTexCoordsVec2 ) ) );
     float lfCloudDepth = DecodeDepthFromColour( texture2D( lUniforms.mpCustomPerMesh.gCloudDepthMap, IN( mTexCoordsVec2 ) ) );

     if ( lfDepth < lfCloudDepth )
     {
         //discard;
         FRAGMENT_COLOUR = vec4(0.0,0.0,0.0,0.0);
         return;
     }*/




     vec2  lFrameSizeVec2 = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
     float lfFrameIndex = mod( lUniforms.mpCustomPerMesh.gCloudSubFrameParamsVec4.x, 16.0 );
     //float lfFrameIndex   = 0.0;
     float lfDitherValue = Bayer( uvec2( IN( mTexCoordsVec2 ) * lFrameSizeVec2 ) ) - 1.0;


     vec4 lFragColVec4;

     //cloud = RandomColour( int(lfFrameIndex) );
     vec2  lDeditheredCoordVec2 = IN( mTexCoordsVec2 );

     {
         uvec2 luPixelCoordVec2 = uvec2( round( lDeditheredCoordVec2 * vec2( 1920.0, 1080.0 ) ) );
         luPixelCoordVec2 -= InverseBayer( uint( floor( lfFrameIndex ) ) );

         lDeditheredCoordVec2 = vec2( luPixelCoordVec2 ) * ( 1.0 / vec2( 1920.0, 1080.0 ) );
     }


     //lFragColVec4 = texture2D( lUniforms.mpCustomPerMesh.gBuffer2Map, lDeditheredCoordVec2 );

     //lFragColVec4 = texture2D( lUniforms.mpCustomPerMesh.gBuffer2Map, IN( mTexCoordsVec2 ) );


     if ( int( lfFrameIndex ) == int( lfDitherValue ) )
     {
         lFragColVec4 = TEXELFETCH( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), ivec2( IN( mTexCoordsVec2 ) * lFrameSizeVec2 * 0.25 ), 0 );
     }
     else
         //     {
         //         lFragColVec4 = textureLoadF( lUniforms.mpCustomPerMesh.gBuffer5Map, ivec2( IN( mTexCoordsVec2 ) * lFrameSizeVec2 ), 0 );
         // 
         //         //discard;
         //     }
             {
                 vec2 lSample = IN( mTexCoordsVec2 );

                 // compute the previous position of this pixel
                 float lfDepth;
                 float lfDepthNormalised;
                 float lfSpeed;
                 vec2  lMotion;
                 vec2  lEncodedMotion;
                 bool  lWantsColorClipAA;
                 vec2  lSampleReproject = GetPrevPositionSimple( lSample,
                                                             lUniforms.mpPerFrame.gClipPlanesVec4,
                                                             lUniforms.mpPerFrame.gInverseProjectionMat4,
                                                             lUniforms.mpPerFrame.gInverseViewMat4,
                                                             lUniforms.mpPerFrame.gViewProjectionMat4,
                                                             lUniforms.mpPerFrame.gPrevViewProjectionMat4,
                                                             lUniforms.mpPerFrame.gViewPositionVec3,
                                                             lUniforms.mpPerFrame.gMBlurSettingsVec4,
                     //SAMPLER2DPARAM( lUniforms.mpCustomPerMesh.gBufferMap ),
                     //SAMPLER2DPARAM( lUniforms.mpCustomPerMesh.gBuffer1Map ),
                     lfDepth,
                     lfDepthNormalised,
                     lfSpeed,
                     lMotion,
                     lEncodedMotion,
                     lWantsColorClipAA );

{
                     //vec4 lPrevSpeeds = texture2D( lUniforms.mpCustomPerMesh.gBuffer4Map, lSampleReproject );
                     //vec2 lPrevDelta = DecodeMotion( lPrevSpeeds.xy );

                     if ( //lPrevSpeeds.z      != 1.0 ||
                          lSampleReproject.y <  0.0 ||
                          lSampleReproject.y >  1.0 ||
                          lSampleReproject.x <  0.0 ||
                          lSampleReproject.x >  1.0 )
                     {
                         lFragColVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), IN( mTexCoordsVec2 ) );
                         //lFragColVec4 = texture2D( lUniforms.mpCustomPerMesh.gBuffer2Map, lDeditheredCoordVec2 );
                         //lFragColVec4 = textureLoadF( lUniforms.mpCustomPerMesh.gBuffer2Map, ivec2(IN( mTexCoordsVec2 ) * lFrameSizeVec2 * 0.25), 0 );
                         //lFragColVec4 = vec4(1.0, 0.0, 1.0, 1.0 );
                     }
                     else
                     {
                         //lFragColVec4 = texture2D( lUniforms.mpCustomPerMesh.gBuffer5Map, lSampleReproject );
                         //lFragColVec4 = textureLoadF( lUniforms.mpCustomPerMesh.gBuffer2Map, ivec2(IN( mTexCoordsVec2 ) * lFrameSizeVec2 * 0.25), 0 );
                         //lFragColVec4 = texture2D( lUniforms.mpCustomPerMesh.gBuffer2Map, IN( mTexCoordsVec2 ) );
                         lFragColVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lDeditheredCoordVec2 );

                         vec4 lReprojectColVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), lSampleReproject );

                         //lFragColVec4 = lReprojectColVec4;

                         //lFragColVec4 = mix( lReprojectColVec4, lFragColVec4, 0.025 );

                         {
                             float rcpFrameOptZ = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 0.25;
                             float rcpFrameOptW = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w * 0.25;
                             //float rcpFrameOptZ = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 1.0;
                             //float rcpFrameOptW = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w * 1.0;
                             /*
                             if ( IN( mTexCoordsVec2 ).x < 0.5 )
                             {
                                 rcpFrameOptZ = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 0.125;
                                 rcpFrameOptW = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w * 0.125;
                             }
                             else
                             {
                                 rcpFrameOptZ = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z * 0.25;
                                 rcpFrameOptW = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w * 0.25;
                             }
                             */

                             vec4 cM = lFragColVec4;

                             vec3 cM3 = cM.rgb * cM.a;
                             //float wk = -TAA_GET_LUMINANCE( cM_jittered );
                             float wk = -GET_LUMINANCE( cM3 );

                             // four points, plus center, from the current screen
                             vec4 cL_M = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lDeditheredCoordVec2 + vec2( -rcpFrameOptZ, -rcpFrameOptW ) );
                             vec3 cL_M3 = cL_M.rgb * cL_M.a;
                             wk += GET_LUMINANCE( cL_M3 ) * 0.25;
                             vec4 cMax = cL_M;
                             vec4 cMin = cL_M;

                             vec4 cR_M = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lDeditheredCoordVec2 + vec2( rcpFrameOptZ, -rcpFrameOptW ) );
                             vec3 cR_M3 = cR_M.rgb * cR_M.a;
                             wk += GET_LUMINANCE( cR_M3 ) * 0.25;
                             cMax = max( cMax, cR_M );
                             cMin = min( cMin, cR_M );

                             vec4 cU_M = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lDeditheredCoordVec2 + vec2( -rcpFrameOptZ, rcpFrameOptW ) );
                             vec3 cU_M3 = cU_M.rgb * cU_M.a;
                             wk += GET_LUMINANCE( cU_M3 ) * 0.25;
                             cMax = max( cMax, cU_M );
                             cMin = min( cMin, cU_M );

                             vec4 cD_M = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lDeditheredCoordVec2 + vec2( rcpFrameOptZ, rcpFrameOptW ) );
                             vec3 cD_M3 = cD_M.rgb * cD_M.a;
                             wk += GET_LUMINANCE( cD_M3 ) * 0.25;
                             cMax = max( cMax, cD_M );
                             cMin = min( cMin, cD_M );

                             // edge detection filter, produces blend values for the front and back buffers
                             // bac buffer is more aggressive than the front buffer
                             //float gfTaaLowFreqConstant = 2.0f;
                             //float gfTaaHighFreqConstant = 4.0f;

                             float kl = ( lUniforms.mpPerFrame.gMBlurSettingsVec4.x );
                             float kh = ( lUniforms.mpPerFrame.gMBlurSettingsVec4.y );

                             // this math is from the Crytek SMAA filter
                             float blendAmount = 1.0 - saturate( rcp( mix( kl, kh, abs( wk ) * 5.0 ) ) );

                             cM = clip_aabb( cM, cMin, cMax );
                             cM = mix( cM, lReprojectColVec4, blendAmount );

                             //lFragColVec4 = vec4( cM, blendAmount );
                             lFragColVec4 = cM;
                             //lFragColVec4 = vec4( blendAmount, 0.0, 0.0, 1.0 );
                         }


                         //if ( abs(lReprojectColVec4.a - lFragColVec4.a) < 0.25 )
         //                 if ( abs(lReprojectColVec4.a - lFragColVec4.a)          < 0.75 ||
         //                      length( lReprojectColVec4.xyz - lFragColVec4.xyz ) < 0.75 )
         //                 {
         //                     lFragColVec4 = lReprojectColVec4;
         //                 }

                         //lFragColVec4 = vec4(1.0);
                         //lFragColVec4 = TEXELFETCH( lUniforms.mpCustomPerMesh.gBuffer5Map, ivec2( lSampleReproject * lFrameSizeVec2 ), 0 );
                     }
                 }
             }

             FRAGMENT_COLOUR = lFragColVec4;
             //FRAGMENT_COLOUR = vec4(1.0,0.0,0.0,1.0);
}

#endif // D_CLOUD_TEMPORAL


#ifdef D_CLOUD_COPY

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
    vec4  lFragmentColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), IN( mTexCoordsVec2 ) );
    lFragmentColourVec4.a = max( lFragmentColourVec4.a - ( 1.0 / 255.0 ), 0.0 );
    
    if ( lFragmentColourVec4.a == 0.0 )
    {
        discard;
    }
    
    float lfDepth = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gDepthMap ), IN( mTexCoordsVec2 ) ) );
    float lfCloudDepth = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gCloudDepthMap ), IN( mTexCoordsVec2 ) ) );

    if ( lfDepth < lfCloudDepth )
    {
        discard;
    }

    //vec4  lFragmentColourDebugVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), IN( mTexCoordsVec2 ) );
    //vec4 lOutput = IN( mTexCoordsVec2 ).x > 0.0 ? lFragmentColourVec4 : lFragmentColourDebugVec4;
    
    vec4 lOutput = lFragmentColourVec4;
    FRAGMENT_COLOUR = lOutput;
}

#endif

#ifdef D_CLOUD_TEMPORAL_COPY

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
    //if ( IN( mTexCoordsVec2 ).x < 0.5 )
    //{
    //    discard;
    //}

    vec4  lFragmentColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), IN( mTexCoordsVec2 ) );
    lFragmentColourVec4.a = max( lFragmentColourVec4.a - ( 1.0 / 255.0 ), 0.0 );

    if ( lFragmentColourVec4.a == 0.0 )
    {
        discard;
    }

    float lfDepth = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gDepthMap ), IN( mTexCoordsVec2 ) ) );
    
    float lfCloudDepth = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gCloudDepthMap ), IN( mTexCoordsVec2 ) ).x;
    
    // We need a minimum z-depth to prevent clouds rendering over cockpit etc.
    lfCloudDepth = max( 0.00001, lfCloudDepth );

    if ( lfDepth < lfCloudDepth )
    {
        discard;
    }

    //if (lFragmentColourVec4.a > 0.0)
    {
        lFragmentColourVec4.rgb /= lFragmentColourVec4.a;
    }

    FRAGMENT_COLOUR = lFragmentColourVec4;
}

#endif // D_CLOUD_TEMPORAL_COPY


#ifdef D_CLOUD_SUN

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_END


float
ComputeScattering(
    in float costh,
    in float g )
{
    return ( 1.0 - g * g ) / ( 4.0 * PI * pow( 1.0 + g * g - 2.0 * g * costh, 3.0 / 2.0 ) );
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec3 lLightPositionVec3 = lUniforms.mpCustomPerMesh.gSunPositionVec4.xyz/* - lUniforms.mpPerFrame.gViewPositionVec3*/;
//vec3 lLightPositionVec3 = -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz;

vec3 lViewDirection = GetScreenSpaceViewDir( IN( mTexCoordsVec2 ), lUniforms.mpPerFrame.gInverseProjectionMat4, lUniforms.mpPerFrame.gInverseViewMat4 );

float lfLDN = dot( lViewDirection, normalize( lLightPositionVec3 ) );

float lfAlpha = 1.0 - texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), IN( mTexCoordsVec2 ) ).a;

float lfSunScatter = ComputeScattering( lfLDN, 0.6 );
lfSunScatter *= lfAlpha;

FRAGMENT_COLOUR = vec4( lfSunScatter, lfSunScatter, lfSunScatter, lfSunScatter );
//FRAGMENT_COLOUR = lUniforms.mpCustomPerMesh.gSunPositionVec4;
}

#endif


#ifdef D_CLOUD_RAYS

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END


#define NUM_GODRAY_SAMPLES 128



FRAGMENT_MAIN_COLOUR_SRT
{
    vec3 lLightPositionVec3 = lUniforms.mpPerFrame.gViewPositionVec3 + ( lUniforms.mpCustomPerMesh.gSunPositionVec4.xyz * 10000.0 );
    //vec3 lLightPositionVec3 =  lUniforms.mpPerFrame.gViewPositionVec3 + lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz
    vec2 lTexCoord = IN( mTexCoordsVec2 ).xy;
    vec4 lScreenLightPosVec4 = MUL( lUniforms.mpPerFrame.gViewProjectionMat4, vec4( lLightPositionVec3, 1.0 ) );

    lScreenLightPosVec4.xyz /= lScreenLightPosVec4.w;

    vec2 lSunTexCoords = ( lScreenLightPosVec4.xy * 0.5 ) + 0.5;
    //lSunTexCoords.y = 1.0 - lSunTexCoords.y;


    float lfDensity = lUniforms.mpCustomPerMesh.gSunRayParams.x;
    float lfDecay = lUniforms.mpCustomPerMesh.gSunRayParams.y;
    float lfExposure = lUniforms.mpCustomPerMesh.gSunRayParams.z;
    float lfWeight = lUniforms.mpCustomPerMesh.gSunRayParams.w;


    if ( lSunTexCoords.x < 0.0 ||
        lSunTexCoords.x >= 1.0 ||
        lSunTexCoords.y < 0.0 ||
        lSunTexCoords.y >= 1.0 )
    {
        discard;
    }

    // Calculate vector from pixel to light source in screen space.
    vec2 deltaTexCoord = ( lTexCoord - lSunTexCoords.xy );
    // Divide by number of samples and scale by control factor.
    deltaTexCoord *= 1.0 / NUM_GODRAY_SAMPLES * lfDensity;
    // Store initial sample.
    vec3 color = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoord ).xyz;
    // Set up illumination decay factor.
    float lfIlluminationDecay = 1.0;
    // Evaluate summation from Equation 3 NUM_SAMPLES iterations.

    for ( int i = 0; i < NUM_GODRAY_SAMPLES; i++ )
    {
        // Step sample location along ray.
        lTexCoord -= deltaTexCoord;
        // Retrieve sample at new location.
        vec3 lSampleVec3 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lTexCoord ).xyz;
        // Apply sample attenuation scale/decay factors.
        lSampleVec3 *= lfIlluminationDecay * lfWeight;
        // Accumulate combined color.
        color += lSampleVec3;
        // Update exponential decay factor.
        lfIlluminationDecay *= lfDecay;
    }

    // Output final color with a further scale control factor.
    FRAGMENT_COLOUR = vec4( color * lfExposure, 1.0 );

    //FRAGMENT_COLOUR = vec4( vec3(length(lTexCoord - lSunTexCoords)), 1.0 );
    //FRAGMENT_COLOUR = vec4( 1.0 );

}

#endif

#ifdef D_CLOUD_RENDER2D
//-----------------------------------------------------------------------------
///
///     Fragment Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------


DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

#if defined( D_CLOUD_RENDER2D ) && !defined( D_COMPUTE )
flat INPUT( vec4, mUpVec3_mfCameraHeight, TEXCOORD1 )
flat INPUT( vec3, mCross1Vec3, TEXCOORD2 )
flat INPUT( vec3, mCross2Vec3, TEXCOORD3 )
#endif

DECLARE_INPUT_END

#ifdef D_PLATFORM_ORBIS
#pragma argument(maxvgprcount=28)
#endif

#include "Common/CommonNoise.shader.h"

vec3
DecodeGBufferPosition_Texture(
    in  vec2  lScreenPosVec2,
    in  vec4  lClipPlanes,
    in  mat4  lInverseProjectionMat4,
    in  mat4  lInverseViewMat4,
    in  vec3  lEyePositionVec3,
    SAMPLER2DARG( lBuffer1Map ) )
{
    vec4 lBuffer1_Vec4 = texture2D( lBuffer1Map, lScreenPosVec2 );

    float lfDepth = FastDenormaliseDepth( lClipPlanes, DecodeDepthFromColour( lBuffer1_Vec4 ) );
    vec3 lPositionVec3 = RecreatePositionFromDepth( lfDepth, lScreenPosVec2, lEyePositionVec3, lInverseProjectionMat4, lInverseViewMat4 );
    //vec3 lPositionVec3 = RecreateViewPositionFromDepth( lfDepth, lScreenPosVec2, lInverseProjectionMat4 );

    //return lPositionVec3 - lEyePositionVec3;
    return lPositionVec3;
}



FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lOutputVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec2 lTexCoords = TEX_COORDS;


    vec3 lPositionVec3 = DecodeGBufferPosition_Texture( lTexCoords,
                                                lUniforms.mpPerFrame.gClipPlanesVec4,
                                                lUniforms.mpPerFrame.gInverseProjectionMat4,
                                                lUniforms.mpPerFrame.gInverseViewMat4,
                                                lUniforms.mpPerFrame.gViewPositionVec3,
                                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ) );

    // Needs to be planet relative for render offset stuff and for shadows anyway
    lPositionVec3 -= lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

    {
        vec3             lPlanetRelativeCamera;
        vec3             lRayVec3;
        float            lfPlanetRadius = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;

        float lfAtmosphereStartHeight = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.y;
        float lfAtmosphereEndHeight = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.z;

        // Do the opposite of this M_CALCULATE_PLANET_RADIUS
        //float lfMaxDistance = sqrt( ( ( lfPlanetRadius + lfAtmosphereStartHeight) * lfAtmosphereStartHeight * 2.0 ) - ( lfAtmosphereStartHeight * lfAtmosphereStartHeight ) );
        float lfMaxDistance = M_CALCULATE_HORIZON_DISTANCE( lfPlanetRadius, ( lfPlanetRadius + lfAtmosphereEndHeight ) );
        float lfCoverageScale = 1.0 / lfMaxDistance;
        lPlanetRelativeCamera = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

        vec3 lNormalizedPlanetRelativeCamera = normalize( lPlanetRelativeCamera );
        vec3 lLightDirectionVec3 = ( lNormalizedPlanetRelativeCamera );//lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz;
        //vec3 lLightDirectionVec3 = lUniforms.mpCustomPerMesh.gSunPositionVec4.xyz * 10000.0;
        vec3 lPlanetUpNormalVec3 = lNormalizedPlanetRelativeCamera;


        {
            vec3 lIntersectionNear;
            vec3 lIntersectionFar;
            float lfSphereDistance;
            float lfDistanceThroughClouds;
            float lfHeightAbovePlanet = length(lPlanetRelativeCamera) - lfPlanetRadius;

            //lfDistanceThroughClouds = saturate( (lfHeightAbovePlanet - lfAtmosphereStartHeight) / (lfAtmosphereEndHeight - lfAtmosphereStartHeight) );
            lfSphereDistance = lfPlanetRadius;
            //lfSphereDistance       +=  mix(lfAtmosphereStartHeight, lfAtmosphereEndHeight, lfDistanceThroughClouds );
            lfSphereDistance += lfAtmosphereStartHeight;

            vec3 lPositionVec3 = DecodeGBufferPosition_Texture(lTexCoords,
                lUniforms.mpPerFrame.gClipPlanesVec4,
                lUniforms.mpPerFrame.gInverseProjectionMat4,
                lUniforms.mpPerFrame.gInverseViewMat4,
                lUniforms.mpPerFrame.gViewPositionVec3,
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBuffer1Map));

            // Needs to be planet relative for render offset stuff and for shadows anyway
            lPositionVec3 -= lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

            float lfDeterminant = GetRayIntersectionPoint(lPositionVec3, lPositionVec3 + lLightDirectionVec3, lfSphereDistance, lIntersectionNear, lIntersectionFar);
            //lfDeterminant = GetRayIntersectionPoint(lPositionVec3, lPositionVec3 + lPlanetUpNormalVec3, lfSphereDistance, lIntersectionNear, lIntersectionFar);
            //lfDeterminant = GetRayIntersectionPoint( lRayStartVec3, lRayStartVec3 + lRayDirectionVec3, lfPlanetRadius + lfCloudHeightMin, lIntersectionNear, lIntersectionFar );


            if ( lfDeterminant == 0.0 )
            {
                lOutputVec4 = vec4( 1.0, 1.0, 1.0, 1.0 );
            }
            else
            {
                vec3  lRayVec3 = lIntersectionFar;
                //lRayVec3 = MUL( lCoverageRotMat3, lRayVec3 );
                vec3 lBlendWeights;

                //lBlendWeights = pow(abs(lPlanetUpNormalVec3), vec3(32.0, 32.0, 32.0));
                lBlendWeights = pow( abs( normalize( lRayVec3 ) ), vec3( 32.0, 32.0, 32.0 ) );
                lBlendWeights /= ( lBlendWeights.x + lBlendWeights.y + lBlendWeights.z );

                vec3 wind_direction = GetWindDirection( lBlendWeights, lUniforms.mpCustomPerMesh.gWindOffset.xy );

                //vec3 wind_direction = lCustomPerMeshUniforms.gBaseOffset.xyz;
                float cloud_speed = lUniforms.mpCustomPerMesh.gAnimationParamsVec4.x * lUniforms.mpPerFrame.gfTime;

                //animate clouds in wind direction and add a small upward bias to the wind direction
                //lRayVec3+= (wind_direction + vec3(0.0, 0.1, 0.0)  ) * time * cloud_speed;
                lRayVec3 += wind_direction * cloud_speed;

                lRayVec3 *= lfCoverageScale;

#if !defined(D_USE_CLOUD_MACROS)
                float lfCoverageTexValue = GetTriPlanarMapGB(lBlendWeights, lRayVec3, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gCoverage2D)).x;
#else
                float lfCoverageTexValue = GetTriPlanarMapGB(lBlendWeights, lRayVec3, SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gCoverage2D)).x;
#endif



                //lfCoverageTexValue *= lUniforms.mpCustomPerMesh.gAnimationParamsVec4.y;
                float lfCoverage = (1.0 - smoothstep(0.5, 1.0, lfCoverageTexValue)) * 0.5;
                float lfStratoCloudsHeight = lUniforms.mpCustomPerMesh.gAtmosphereParamsVec4.w;

                lfCoverage = mix( lfCoverage, 1.0, smoothstep( lfStratoCloudsHeight * 4.0, lfStratoCloudsHeight * 5.0, lfHeightAbovePlanet ) );
                lOutputVec4 = vec4( lfCoverage, lfCoverage, lfCoverage, lfCoverage );
            }
        }

    }

    WRITE_FRAGMENT_COLOUR( saturate( lOutputVec4 ) );
}
#endif // D_CLOUD_RENDER2D

#ifdef D_CLOUD_TEMPORAL_REPROJECT

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
 INPUT_SCREEN_POSITION

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
///
///     Fragment Main
///
///     @brief      Reprojection of the previous frame's cloud buffer in the
///                 current frame.
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR01_SRT
{
    cCloudProperties lCloudProperties;
    vec2  lTexCoords = TEX_COORDS.xy;

    // We can't run this with VR, because the temporal grid causes problems with stereo fusion.
    // Basic temporal algorithm:
    //  a. Correct the previous frame pixel positions in the current viewprojection space <- you are here
    //  b. Blit to the feedback buffer.
    //  c. Render the full cloud ray-cast for a single pixel in sixteen.
    //  d. Render any pixels which were missing as a result of a reproject as infills.
    //  e. $$$
	//  f... also motion vectors should be applied, which we're not doing at the moment.
    
    // 1. Calculate the current world space position of the screen coordinates, and its position in the previous frame buffer.

    vec2 lTexelCoordsVec2 = lTexCoords.xy;
    float lfReverseZDepth = 0.0;

    vec4 lScreenPosition;
    lScreenPosition.xy = SCREENSPACE_AS_RENDERTARGET_UVS( lTexelCoordsVec2 ) * 2.0 - 1.0;
    lScreenPosition.z = lfReverseZDepth;
    lScreenPosition.w = 1.0;

    vec4 lWorldPositionVec4 = MUL( lUniforms.mpPerFrame.gInverseViewProjectionNoJitterMat4, lScreenPosition );
    
    vec4 lPrevScreenCoordinatesVec4 = MUL(
        lUniforms.mpPerFrame.gPrevViewProjectionNoJitterMat4,
        lWorldPositionVec4 );
    lPrevScreenCoordinatesVec4 /= lPrevScreenCoordinatesVec4.w;

    vec2 lPrevTextureCoordsVec4 = lPrevScreenCoordinatesVec4.xy;
    lPrevTextureCoordsVec4 = 0.5 * lPrevTextureCoordsVec4 + 0.5;
    lPrevTextureCoordsVec4.y = 1.0 - lPrevTextureCoordsVec4.y;

    // 2. If it was previously offscreen, we're going to leave it to another pass to fix.

    float lfMinX = 0.0;
    float lfMinY = 0.0;
    float lfMaxX = 1.0f;
    float lfMaxY = 1.0f;

    if ( lPrevTextureCoordsVec4.x < lfMinX || lPrevTextureCoordsVec4.x >= lfMaxX ||
        lPrevTextureCoordsVec4.y < lfMinY || lPrevTextureCoordsVec4.y >= lfMaxY )
    {
#if defined( D_COMPUTE )
        return;
#else
        discard;
#endif
    }

    // 3. Sample our nearest-neighbour to transform previous pixels to our current view.

    vec4 lColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lPrevTextureCoordsVec4.xy, 0 );
    //float lfDepth = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lPrevTextureCoordsVec4.xy, 0 ).x;

    vec2 lBilinearTexCentroidVec2 = lPrevTextureCoordsVec4.xy;
    float lfTexelStepWidth = lUniforms.mpPerFrame.gFrameBufferSizeVec4.z;
    float lfTexelStepHeight = lUniforms.mpPerFrame.gFrameBufferSizeVec4.w;
    
    vec2 laPixelCoordinates[ 4 ];
    laPixelCoordinates[ 0 ] = vec2( lBilinearTexCentroidVec2.x, lBilinearTexCentroidVec2.y );
    laPixelCoordinates[ 1 ] = vec2( lBilinearTexCentroidVec2.x + lfTexelStepWidth, lBilinearTexCentroidVec2.y );
    laPixelCoordinates[ 2 ] = vec2( lBilinearTexCentroidVec2.x, lBilinearTexCentroidVec2.y + lfTexelStepHeight );
    laPixelCoordinates[ 3 ] = vec2( lBilinearTexCentroidVec2.x + lfTexelStepWidth, lBilinearTexCentroidVec2.y + lfTexelStepHeight );

    float laDepths[ 4 ];
    laDepths[ 0 ] = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), laPixelCoordinates[ 0 ].xy, 0 ).x;
    laDepths[ 1 ] = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), laPixelCoordinates[ 1 ].xy, 0 ).x;
    laDepths[ 2 ] = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), laPixelCoordinates[ 2 ].xy, 0 ).x;
    laDepths[ 3 ] = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), laPixelCoordinates[ 3 ].xy, 0 ).x;
    
    // Bilinear filter the depth values.
    float lfWeight = 0.5;
    float lfDepth0 = ( 1.0 - lfWeight ) * laDepths[ 0 ] + ( lfWeight ) * laDepths[ 1 ];
    float lfDepth1 = ( 1.0 - lfWeight ) * laDepths[ 2 ] + ( lfWeight ) * laDepths[ 3 ];
    float lfBilinearFilteredDepth = ( 1.0 - lfWeight ) * lfDepth0 + lfWeight * lfDepth1;
    
    // A large floating point number, and our depths should be less than this.  Calculate the closest depth.
    //float lfClosestTexel = 100000.0;
    //lfClosestTexel = laDepths[ 0 ] < lfClosestTexel ? laDepths[ 0 ] : lfClosestTexel;
    //lfClosestTexel = laDepths[ 1 ] < lfClosestTexel ? laDepths[ 1 ] : lfClosestTexel;
    //lfClosestTexel = laDepths[ 2 ] < lfClosestTexel ? laDepths[ 2 ] : lfClosestTexel;
    //lfClosestTexel = laDepths[ 3 ] < lfClosestTexel ? laDepths[ 3 ] : lfClosestTexel;
    
    // Calculate the distance between depth values.
    float lfMaxDepth = max( max( laDepths[ 0 ], laDepths[ 1 ] ), max( laDepths[ 2 ], laDepths[ 3 ] ) );
    float lfMinDepth = min( min( laDepths[ 0 ], laDepths[ 1 ] ), min( laDepths[ 2 ], laDepths[ 3 ] ) );
    float lfDepthDiff = lfMaxDepth - lfMinDepth;
    
    float lfSelector = step( 0.0005, lfDepthDiff );
    float lfDepth = saturate( mix( lfBilinearFilteredDepth, laDepths[ 0 ], lfSelector ) );

    // 4. We have the nearest neighbours, but their depth will be in the previous frame, we need to transform the depth to the current frame and carry out the filter.
    //  We're not doing a filter now, as the reprojection will be constrained to a smaller depth range.

    {
        float lfActualReverseDepth = LinearNormToReverseZDepth(
            lUniforms.mpPerFrame.gClipPlanesVec4,
            lfDepth );

        vec4 lPrevScreenPosition;
        lPrevScreenPosition.xy = SCREENSPACE_AS_RENDERTARGET_UVS( lPrevTextureCoordsVec4.xy ) * 2.0 - 1.0;
        lPrevScreenPosition.z = lfActualReverseDepth;
        lPrevScreenPosition.w = 1.0;

        vec4 lActualPositionVec4 = MUL( lUniforms.mpPerFrame.gPrevInverseViewProjectionNoJitterMat4, lPrevScreenPosition );
        
        lActualPositionVec4 = MUL(
            lUniforms.mpPerFrame.gViewProjectionNoJitterMat4,
            lActualPositionVec4 );
        lActualPositionVec4 /= lActualPositionVec4.w;

        lfDepth = ReverseZToLinearDepthNorm(
            lUniforms.mpPerFrame.gClipPlanesVec4,
            lActualPositionVec4.z );
        lfDepth = saturate( lfDepth );
    }

    WRITE_FRAGMENT_COLOUR0( saturate( lColourVec4 ) );
    WRITE_FRAGMENT_COLOUR1( EncodeDepthToColour( lfDepth ) );
}

#endif // D_CLOUD_TEMPORAL_REPROJECT

#ifdef D_CLOUD_TEMPORAL_BLIT

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
 INPUT_SCREEN_POSITION

INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
///
///     Fragment Main
///
///     @brief      Temporal blit copies from the reprojection buffer, which 
///                 also contains the edge reconstruction, and the small ray-cast
///                 buffer to generate the final cloud frame.
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------

FRAGMENT_MAIN_COLOUR01_SRT
{
    cCloudProperties lCloudProperties;
    
    // We're using the blit step to copy the output of the reprojection into a history buffer that we can use in the next frame.  Previously
    //  we would use the blit step to copy the grid cloud calculation as part of the blit, we don't need to do this anymore, as the infill
    //  due to missing reprojection and the grid pass happen in the same step.
    // We can't run the temporal cloud when we're running VR as it messes with stereo fusion.

    vec2 lTexCoords = TEX_COORDS.xy;

    // Our pixel offset in the larger buffer.  We're assuming that a texel offset of 0.5, 0.5 has been added during texel interpolation.
    //  We needed this with compute, we don't need this with fragment shaders.
    //vec2 lTexCoords = floor( TEX_COORDS.xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );
    //ivec2 lTextureCoordsIVec2 = ivec2( lTexCoords );
    //lTexCoords /= lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    
    {
        vec4 lColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), TEX_COORDS.xy, 0 );
        WRITE_FRAGMENT_COLOUR0( saturate( lColourVec4 ) );

        float lfDepth = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), TEX_COORDS.xy, 0 ).x;
        WRITE_FRAGMENT_COLOUR1( float2vec4( lfDepth ) );
    }
}

#endif // D_CLOUD_TEMPORAL_BLIT
