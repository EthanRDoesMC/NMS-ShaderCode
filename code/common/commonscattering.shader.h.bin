////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonDepth.h
///     @author     User
///     @date       
///
///     @brief      CommonDepth
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONSCATTERING_H
#define D_COMMONSCATTERING_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Common.shader.h"
#include "Common/noise3d.glsl"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Functions


//-----------------------------------------------------------------------------
//      Global Data
    
STATIC_CONST float kf4Pi        = 3.14159265358979323846 * 4.0;
STATIC_CONST float kf4Pi_Ln2    = 18.1294403076171875;
STATIC_CONST float kfSqrtPi_2   = 1.253314137315500251208;
STATIC_CONST float kfSqrt1_2    = 0.707106781186547524401;
STATIC_CONST float kfSqrtLn4_2  = 0.588705003261566162109375;
STATIC_CONST float kfSqrtPi_Ln2 = 2.1289341449737548828125;
STATIC_CONST float kfLn2        = 0.693147180559945309417;
STATIC_CONST float kf1_Ln2      = 1.44269502162933349609375;

//-----------------------------------------------------------------------------
//      Functions 

vec3
GetFarRayIntersectionPoint(
    vec3     lStartPointVec3,
    vec3     lDirection,
    float    lfRadius )
{
    float lfB                = dot( lStartPointVec3, lDirection );
    float lfC                = dot( lStartPointVec3, lStartPointVec3 ) - lfRadius * lfRadius;
    float lfDet              = lfB * lfB - lfC;

    if( lfDet >= 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfFar  = -lfB + lfSqrt;

        return lStartPointVec3 + lDirection * lfFar;
    }
    else
    {
        return float2vec3( 0.0 );
    }
}

vec3
GetNearRayIntersectionPoint(
    vec3     lStartPointVec3,
    vec3     lDirection,
    float    lfRadius )
{
    float lfB     = 2.0 * dot( lStartPointVec3, lDirection );
    float lfC     = dot( lStartPointVec3, lStartPointVec3 ) - ( lfRadius * lfRadius );
    float lfDet   = lfB*lfB - 4.0 * lfC;

    if( lfDet >= 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfNear = 0.5 * ( -lfB - lfSqrt );

        return normalize( lStartPointVec3 + lDirection * lfNear ) * lfRadius;
    }
    else
    {
        return float2vec3( 0.0 );
    }
}

float
GetNearRayIntersectionDist(
    in    vec3     lStartPointVec3,
    in    vec3     lDirectionVec3,
    in    float    lfRadius )
{
    float lfB     = dot( lStartPointVec3, lDirectionVec3 );
    float lfC     = dot( lStartPointVec3, lStartPointVec3 ) - lfRadius * lfRadius;
    float lfDet   = lfB * lfB - lfC;

    if( lfDet >= 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfNear = -lfB - lfSqrt;

        return lfNear;
    }
    return -1.0;
}

float Sqr( vec3  v ) { return dot( v, v ); }
float Sqr( float v ) { return v * v; }

float
GetClampedRayIntersectionPoint(
    in  vec3  lStartPointVec3,
    in  vec3  lEndPointVec3,
    in  float lfRadius,
    out vec3  lOutNearPointVec3,
    out vec3  lOutFarPointVec3 )
{
    lOutNearPointVec3 = lStartPointVec3;
    lOutFarPointVec3 = lEndPointVec3;

    float lfLengthSqr        = Sqr( lStartPointVec3 - lEndPointVec3 );
    vec3  lNormalisedRayVec3 = normalize( lStartPointVec3 - lEndPointVec3  );
    float lfB                = 2.0 * dot( lStartPointVec3, lNormalisedRayVec3 );
    float lfC                = dot( lStartPointVec3, lStartPointVec3 ) - ( lfRadius * lfRadius );
    float lfDet              = lfB * lfB - 4.0 * lfC;
     
    if( lfDet >= 0.0 && lfLengthSqr != 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfNear = 0.5 * ( -lfB + lfSqrt );
        float lfFar  = 0.5 * ( -lfB - lfSqrt );

        lOutFarPointVec3  = lStartPointVec3 + lNormalisedRayVec3 * lfFar;
        lOutNearPointVec3 = lStartPointVec3 + lNormalisedRayVec3 * lfNear;

        lfDet = 1.0;

        if( max( Sqr( lOutNearPointVec3 - lEndPointVec3 ), Sqr( lOutNearPointVec3 - lStartPointVec3 ) ) > lfLengthSqr )
        {
            lOutNearPointVec3 = lStartPointVec3;
            lfDet += 1.0;
        }

        if( max( Sqr( lOutFarPointVec3 - lEndPointVec3 ), Sqr( lOutFarPointVec3 - lStartPointVec3 ) ) > lfLengthSqr )
        {
            lOutFarPointVec3 = lEndPointVec3;
            lfDet += 2.0;
        }

        return lfDet;
    }

    return 0.0;
}

float
GetClampedRayIntersectionDist(
    in  vec3  lStartPointVec3,
    in  vec3  lDirectionVec3,
    in  float lfLength,
    in  float lfRadius )
{
    float lfB                = dot( lStartPointVec3, lDirectionVec3 );
    float lfC                = dot( lStartPointVec3, lStartPointVec3 ) - lfRadius * lfRadius;
    float lfDet              = lfB * lfB - lfC;

    if( lfDet >= 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfNear = max( 0.0,        -lfB - lfSqrt );
        float lfFar  = min( lfLength,   -lfB + lfSqrt );

        return lfFar - lfNear;
    }

    return 0.0;
}

float
GetRayIntersectionPoint(
    in  vec3  lStartPointVec3,
    in  vec3  lEndPointVec3,
    in  float lfRadius,
    out vec3  lOutNearPointVec3,
    out vec3  lOutFarPointVec3 )
{
    lOutNearPointVec3 = lStartPointVec3;
    lOutFarPointVec3 = lEndPointVec3;

    float lfLength           = length( lStartPointVec3 - lEndPointVec3 );
    vec3  lNormalisedRayVec3 = normalize( lStartPointVec3 - lEndPointVec3 );
    float lfB                = 2.0 * dot( lStartPointVec3, lNormalisedRayVec3 );
    float lfC                = dot( lStartPointVec3, lStartPointVec3 ) - ( lfRadius * lfRadius );
    float lfDet              = lfB * lfB - 4.0 * lfC;

    if( lfDet >= 0.0 && lfLength != 0.0 )
    {
        float lfSqrt = sqrt( lfDet );
        float lfNear = 0.5 * ( -lfB + lfSqrt );
        float lfFar  = 0.5 * ( -lfB - lfSqrt );

        lOutFarPointVec3  = lStartPointVec3 + lNormalisedRayVec3 * lfFar;
        lOutNearPointVec3 = lStartPointVec3 + lNormalisedRayVec3 * lfNear;

        lfDet = 1.0;

        if( max( length( lOutNearPointVec3 - lEndPointVec3 ), length( lOutNearPointVec3 - lStartPointVec3 ) ) > lfLength )
        {
            lfDet += 1.0;
        }

        if( max( length( lOutFarPointVec3 - lEndPointVec3 ), length( lOutFarPointVec3 - lStartPointVec3 ) ) > lfLength )
        {
            lfDet += 2.0;
        }

        return lfDet;
    }

    return 0.0;
}

float
GetAtmosphereDensity(
    vec3      lSamplePointVec3,
    float     lfInvAtmosphereSize,
    float     lfScaleDepth )
{ 
    float lfDensity;

    // Distances from planet
    float lfHeight            = length( lSamplePointVec3 );
    float lfHeightAbovePlanet = lfHeight - 1.0; // Inner radius size

    // Scale based on average depth of scattering particles
    lfDensity = ( lfHeightAbovePlanet * lfInvAtmosphereSize );
    lfDensity = lfDensity / lfScaleDepth;

    // Exponential curve out from planet
    return exp( -lfDensity );

}


#if defined ( D_PLATFORM_METAL )
STATIC_CONST float p = 0.47047;
STATIC_CONST float a1 = 0.3480242;
STATIC_CONST float a2 = -0.0958798;
STATIC_CONST float a3 = 0.7478556;
#endif

float
ErfDiff(
    float x, float y )
{
#if !defined ( D_PLATFORM_METAL )
    STATIC_CONST float p  =  0.47047;
    STATIC_CONST float a1 =  0.3480242;
    STATIC_CONST float a2 = -0.0958798;
    STATIC_CONST float a3 =  0.7478556;
#endif

    float sx = x > 0 ? 1.0 : -1.0;
    float sy = y > 0 ? 1.0 : -1.0;
    float xa = abs( x );
    float ya = abs( y );
    float ex = exp2( - xa * xa * 0.5 );
    float ey = exp2( - ya * ya * 0.5 );
    float tx = 1.0 / ( 1.0 + p * xa * kfSqrtLn4_2 );
    float ty = 1.0 / ( 1.0 + p * ya * kfSqrtLn4_2 );
    float xx = tx * ( a1 + tx * ( a2 + tx * a3 ) ) * ex;
    float yy = ty * ( a1 + ty * ( a2 + ty * a3 ) ) * ey;

    return sx - sy + sy * yy - sx * xx;
}

#if defined ( D_PLATFORM_METAL )
STATIC_CONST float eps = 1.0e-16;
#endif

float
ApproximateIntegral(
    float a, float b, float k, float s )
{
#if !defined ( D_PLATFORM_METAL )
    STATIC_CONST float eps = 1.0e-16;
#endif

    float itgr;

    if ( k < eps )
    {
#if defined( D_COMPUTE_VERTICAL_INTEGRAL )
        float a_abs     = abs( a );
        float b_abs     = abs( b );
        float denom     = a * b * s * ( a - b );
        float eps_a     = exp( s - s * b_abs );
        float eps_b     = exp( s - s * a_abs );
        float term_a    = a * b_abs;
        float term_b    = b * a_abs;
        float nom       = eps_a * term_a - eps_b * term_b;
        itgr            = nom / denom;
#else
        itgr            = 0;
#endif
    }
    else
    {
        float sk        = s * k;
        float sk_isqrt  = invsqrt( sk );
        float p_sqrt    = kfSqrtPi_Ln2;
        float ex_sk     = exp2( s - sk - 0.5 );
        float ex_sk_pi  = ex_sk * p_sqrt;
        float er_fact   = s * sk_isqrt;
        float a_er      = a * er_fact;
        float b_er      = b * er_fact;
        float denom     = a_er - b_er;
        float term      = ex_sk_pi / denom;
        float er        = ErfDiff( a_er, b_er );

        itgr            = max( term * er, 0.0 );
    }

    return itgr;
}

float
OutScatteringGroundTruth(
    vec3     lStartPointVec3,
    vec3     lEndPointVec3,
    float    lfDist,
    float    lfDensityFactor )
{
    // Computes the integral using a densely sampled Riemann midpoint sum
    // very expensive due to the high number of samples, but useful for debug comparisons

    int   liSegments                = 128;
    float lfIntegrationFactor       = length( lEndPointVec3 - lStartPointVec3 ) / float( liSegments );
    float lfS                       = lfDensityFactor * kfLn2;
    float lfScattering              = 0;

    for ( int ii = 0; ii < liSegments; ++ii )
    {
        float t = ( float( ii ) + 0.5f ) / float( liSegments );
        lfScattering += exp( - lfS * ( length( lStartPointVec3 * ( 1.0f - t ) + lEndPointVec3 * t ) - 1.0f ) );
    }

    return lfScattering * lfIntegrationFactor;
}

float
OutScatteringTrapezoidExp(
    vec3     lStartPointVec3,
    vec3     lEndPointVec3,
    float    lfDist,
    float    lfDensityFactor )
{
    // Trapezoidal Riemann sum using a limited number of samples and exp ops

    #if !defined ( D_PLATFORM_SWITCH )
    float lfSegments = float( 20 );
    #else
    float lfSegments = float( 7 );
    #endif

    vec3  lSamplePointVec3      = lStartPointVec3;
    vec3  lRayStepVec3          = ( lEndPointVec3 - lStartPointVec3 ) / lfSegments;
    float lfIntegrationFactor   = length( lRayStepVec3 ) * 0.5;
    float lfInvAtmosphereSizeSN = - lfDensityFactor * kfLn2;

    float lScattering;
    lScattering=  exp( (length(lSamplePointVec3 + 0.0 *  lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 1.0 *  lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 2.0 *  lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 3.0 *  lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 5.0 *  lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 7.0 *  lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 11.0 * lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 15.0 * lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );
    lScattering+= exp( (length(lSamplePointVec3 + 20.0 * lRayStepVec3)-1.0) * lfInvAtmosphereSizeSN );

    return lScattering * lfIntegrationFactor;
}

float
OutScatteringTrapezoidExp2(
    vec3     lStartPointVec3,
    vec3     lEndPointVec3,
    float    lfDist,
    float    lfDensityFactor )
{
    // Optimised version of OutScatteringTrapezoidExp:
    // 1. exp2 is a native GCN instruction - so convert the original natural exp with a log identity
    // 2. re-arrange inner exp2 maths to allow for efficient MAC code gen.

    #if !defined ( D_PLATFORM_SWITCH )
    float lfSegments = float( 20 );
    #else
    float lfSegments = float( 7 );
    #endif

    vec3  lSamplePointVec3          = lStartPointVec3;
    vec3  lRayStepVec3              = ( lEndPointVec3 - lStartPointVec3 ) / lfSegments;
    float lfIntegrationFactor       = length( lRayStepVec3 ) * 0.5;
    float lfInvAtmosphereSizeSN_L2  = lfDensityFactor;

    
    float lScattering2;
    lScattering2  = exp2( ( 1.0 - length(lSamplePointVec3 +  0.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * 1.0;           //  1.0 -  0.0
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 +  1.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 1.0 + 1.0 ); //  1.0 -  0.0 +  2.0 -  1.0
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 +  2.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 1.0 + 1.0 ); //  2.0 -  1.0 +  3.0 -  2.0 
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 +  3.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 1.0 + 2.0 ); //  3.0 -  2.0 +  5.0 -  3.0  
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 +  5.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 2.0 + 2.0 ); //  5.0 -  3.0 +  7.0 -  5.0
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 +  7.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 2.0 + 4.0 ); //  7.0 -  5.0 + 11.0 -  7.0
    
    #if !defined ( D_PLATFORM_SWITCH )
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 + 11.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 4.0 + 4.0 ); // 11.0 -  7.0 + 15.0 - 11.0
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 + 15.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * ( 4.0 + 5.0 ); // 15.0 - 11.0 + 20.0 - 15.0 
    lScattering2 += exp2( ( 1.0 - length(lSamplePointVec3 + 20.0 * lRayStepVec3) ) * lfInvAtmosphereSizeSN_L2) * 5.0;           // 20.0 - 15.0
    #endif

    return lScattering2 * lfIntegrationFactor;
}

float
OutScatteringApproxFitted(
    vec3     lStartPointVec3,
    vec3     lEndPointVec3,
    float    lfDist,
    float    lfDensityFactor )
{
    // Closed form approximation using a fitted curve

    // Trying out a better integral, so:
    //	integral e ^ ( nx - n ) dx gives us ( 1 / ( n e * n ) ) * ( e ^ na - e ^ nb ). 
    //  unfortunately the last exp() scale gives us a massive denominator, which reduces the output to close to 0.0.

    float lfInvAtmosphereSizeSN = - lfDensityFactor * kfLn2;

    float lExpX0 = lfDist                   * lfInvAtmosphereSizeSN;
    float lExpX1 = length( lEndPointVec3 )  * lfInvAtmosphereSizeSN;

    lExpX0 = exp( lExpX0 );
    lExpX1 = exp( lExpX1 );

    float lScattering2;
    lScattering2 = ( lExpX1 - lExpX0 ) * ( 1.0 / ( lfInvAtmosphereSizeSN * exp( lfInvAtmosphereSizeSN ) ) );

    return lScattering2;//*lfIntegrationFactor;
}

float
OutScatteringApproxLaplace(
    vec3     lStartPointVec3,
    vec3     lEndPointVec3,
    float    lfDist,
    float    lfDensityFactor )
{
    // Closed form approximation using Laplace's method

    vec3    lvDiff          = lEndPointVec3 - lStartPointVec3;
    float   lfArcLength     = length( lvDiff );

    vec3    lvDir           = lvDiff / lfArcLength;
    float   lfA             = dot( lStartPointVec3, lvDir );
    float   lfB             = lfA + lfArcLength;
    float   lfS             = lfDensityFactor;

    float   lfK             = sqrt( lfDist * lfDist - lfA * lfA );
    float   lfScattering    = ApproximateIntegral( lfA, lfB, lfK, lfS );

    return  lfScattering * lfArcLength;
}


float
OutScattering(
    vec3    lStartPointVec3,
    vec3    lEndPointVec3,
    float   lfDist,
    float   lfDensityFactor )
{

#define D_APPROX_LAPLACE

#if   defined( D_GROUND_TRUTH )
    return OutScatteringGroundTruth  ( lStartPointVec3, lEndPointVec3, lfDist, lfDensityFactor );
#elif defined( D_TRAPEZOID_EXP )
    return OutScatteringTrapezoidExp ( lStartPointVec3, lEndPointVec3, lfDist, lfDensityFactor );
#elif defined( D_TRAPEZOID_EXP2 )
    return OutScatteringTrapezoidExp2( lStartPointVec3, lEndPointVec3, lfDist, lfDensityFactor );
#elif defined( D_APPROX_FITTED )
    return OutScatteringApproxFitted ( lStartPointVec3, lEndPointVec3, lfDist, lfDensityFactor );
#elif defined( D_APPROX_LAPLACE )
    return OutScatteringApproxLaplace( lStartPointVec3, lEndPointVec3, lfDist, lfDensityFactor );
#else
    #error no valid scattering method selected;
#endif
}

vec3
OutScatteringReference(
    vec3     lStartPointVec3,
    vec3     lStartPoint,
    float    lfInvAtmosphereSize,
    float    lScatteringRatio,
    float    lfScaleDepth )
{
    
    // Atmosphere Scattering
    vec3 lScatteringVec3 = float2vec3( 0.0 );

    int   liSegments = 20;
    float lfSegments = float( liSegments );

    vec3  lSamplePointVec3    = lStartPointVec3;
    vec3  lRayStepVec3        = ( lStartPoint - lStartPointVec3 ) / lfSegments;
    float lfIntegrationFactor = length( lStartPoint - lStartPointVec3 ) / ( 2.0 * lfSegments );

    // Sample density at points along the ray
    for( int i = 0; i < liSegments + 1; i++ )
    {
        float lfDensity = 0.0;

        lfDensity = GetAtmosphereDensity(
            lSamplePointVec3,
            lfInvAtmosphereSize,
            lfScaleDepth );

        // Accumulate and move along ray
        lScatteringVec3  += float2vec3( lfDensity );
        lSamplePointVec3 += lRayStepVec3;
    }

    lScatteringVec3         *= lfIntegrationFactor;

    // Combine out scattering factors
    lScatteringVec3 *= float2vec3( kf4Pi * lScatteringRatio );

    return lScatteringVec3;
}

float
InScattering(
    vec3    lStartPointVec3,
    vec3    lSunPositionVec3,
    vec3    lCameraPositionVec3,
    float   lfOuterRadius,
    float   lfScatteringRatio,
    float   lfScaleDepth,
    bool    lbLowQuality,
    bool    lbNoTest )
{
    // NOTE(sal): Turns out our inscattering maths are a bit wrong; in particular,
    // the camera optical depth integral has some problems. I tried fixing it up,
    // but that has a noticeable impact on the look of our skies; I'm going to
    // leave the old, hacky integral in for now, and hopefully we can switch to a
    // midpoint Riemann sum ( D_MIDPOINT_SUM == 1 ) with correct numerical integration
    // ( D_HACKY_SUM == 0 ) in the future.

    #define D_MIDPOINT_SUM  0
    #define D_HACKY_SUM     1

    float lfInvAtmosphereSize   = 1.0 / ( lfOuterRadius - 1.0 );
    int   liSegments            = lbLowQuality ? 4 : 8;
    float lfSegments            = float( liSegments );
    float lfTotalScattering     = 0.0;

    // Space
    vec3 lAtmosphereHitPointNear = lStartPointVec3;
    vec3 lAtmosphereHitPointFar  = lCameraPositionVec3;

    if ( lfScaleDepth != 0.0 )
    {
        vec3  lRayStepVec3          = ( lAtmosphereHitPointNear - lAtmosphereHitPointFar ) / lfSegments;
        float lfSegmentLength       = length( lRayStepVec3 );
        #if D_HACKY_SUM
        float lfIntegrationScale    = 0.5;
        #else
        float lfIntegrationScale    = 1.0;
        #endif
        float lfIntegrationFactor   = lfSegmentLength * lfIntegrationScale;
        float lfDensityFactor       = lfInvAtmosphereSize / ( lfScaleDepth * kfLn2 );

        #if D_MIDPOINT_SUM
        // Mid-point Riemann sum
        vec3  lSamplePointVec3      = lAtmosphereHitPointFar + lRayStepVec3 * 0.5f;
        #else
        vec3  lSamplePointVec3      = lAtmosphereHitPointFar;
        #endif

        float lfCamOpticalDepth     = 0.0;
        float lfDensitySum          = 0.0;

        // Sum samples along ray from camera to world position
        for( int ii = 0; ii < liSegments; ++ii )
        {
            float lfDist        = length( lSamplePointVec3 );
            float lfHeight      = lfDist - 1.0;
            float lfHeightPwr   = -lfHeight * lfDensityFactor;

            if ( lfHeightPwr > -11.7027 )
            {
                float lfDensity     = exp2( -lfHeight * lfDensityFactor );

                #if D_HACKY_SUM
                lfDensitySum       += lfDensity;
                lfCamOpticalDepth   = lfDensitySum;

                if( ii > 0 )
                {
                    lfCamOpticalDepth *= lfIntegrationFactor;
                }
                #else
                lfCamOpticalDepth   += lfDensity * lfIntegrationFactor;
                #endif
                if ( lfHeightPwr > lfCamOpticalDepth * kf1_Ln2 - 9.96578 )
                {
                    // Out scattering between the light-source and the sample point
                    float lfSunOpticalDepth = OutScattering(
                                                lSamplePointVec3,
                                                lSamplePointVec3 + lSunPositionVec3,
                                                lfDist,
                                                lfDensityFactor );

                    float lfTransmittance   = exp2( - lfScatteringRatio * kf4Pi_Ln2 * ( lfSunOpticalDepth + lfCamOpticalDepth ) );

                    // Accumulate 
                    lfTotalScattering += lfDensity * lfTransmittance;
                }
        	}
            lSamplePointVec3 += lRayStepVec3;
        }
        lfTotalScattering *= lfIntegrationFactor;
    }
    return isnan(lfTotalScattering) ? 0.0 : lfTotalScattering;
}


float
MiePhase(
vec3  lSunPositionVec3,
vec3  lRayDirectionVec3,
float lfSunScale )
{
    float lfCos        = dot( normalize( lSunPositionVec3 ), lRayDirectionVec3 ) / length( lRayDirectionVec3 );
    float lfSunScale2  = lfSunScale * lfSunScale;

    return 1.5 * ( ( 1.0 - lfSunScale2 ) / ( 2.0 + lfSunScale2 ) ) * ( 1.0 + lfCos*lfCos ) / pow( 1.0 + lfSunScale2 - 2.0*lfSunScale*lfCos, 1.5 );
}

float
MiePhaseNoNorm(
    vec3  lSunPositionVec3,
    vec3  lRayDirectionVec3,
    float lfSunScale )
{
    float lfCos        = dot( lSunPositionVec3, lRayDirectionVec3 );
    float lfSunScale2  = lfSunScale * lfSunScale;

    return 1.5 * ( ( 1.0 - lfSunScale2 ) / ( 2.0 + lfSunScale2 ) ) * ( 1.0 + lfCos*lfCos ) / pow( 1.0 + lfSunScale2 - 2.0*lfSunScale*lfCos, 1.5 );
}

// MiePhase with pre-normalised inputs assumed.

float
MiePhasePN(
vec3  lSunPositionVec3,
vec3  lRayDirectionVec3,
float lfSunScale)
{
    float lfCos = dot(lSunPositionVec3, lRayDirectionVec3);
    float lfSunScale2 = lfSunScale * lfSunScale;

    return 1.5 * ((1.0 - lfSunScale2) / (2.0 + lfSunScale2)) * (1.0 + lfCos*lfCos) / pow(max(1.0 + lfSunScale2 - 2.0*lfSunScale*lfCos, 0.0000001), 1.5);
}


float
RayleighPhase(
vec3  lSunPositionVec3,
vec3  lRayDirectionVec3 )
{
    float lfCos = dot( normalize( lSunPositionVec3 ), lRayDirectionVec3 ) / length( lRayDirectionVec3 );
    return 0.75 * ( 1.0 + lfCos*lfCos );
}

float
RayleighPhaseNoNorm(
    vec3  lSunPositionVec3,
    vec3  lRayDirectionVec3 )
{
    float lfCos = dot( lSunPositionVec3, lRayDirectionVec3 );
    return 0.75 * ( 1.0 + lfCos*lfCos );
}

float
InScatteringPhase(
    float lfInScattering,
    float lfSunIntensity,
    float lfSunFactor,
    float lScatteringRatio,
    float lfPhase )
{
    float  lfScatterStrength = lScatteringRatio * lfPhase;
    return lfSunIntensity * lfSunFactor * lfScatterStrength * lfInScattering;
}

vec3
InScatteringPhase(
    float lfInScattering,
    vec3  lSunColour,
    float lfSunFactor,
    float lScatteringRatio,
    float lfPhase )
{
    float  lfScatterStrength = lScatteringRatio * lfPhase;
    return lSunColour * lfSunFactor * lfScatterStrength * lfInScattering;
}

vec3
InScatteringPhase(
    vec3  lInScatteringVec3,
    vec3  lSunColour,
    float lfSunFactor,
    float lScatteringRatio,
    float lfPhase )
{
    vec3  lSunStrengthVec3     = lSunColour * lfSunFactor;
    float lScatterStrengthVec3 = lScatteringRatio * lfPhase;

    return lSunStrengthVec3 * lScatterStrengthVec3 * lInScatteringVec3;
}

vec3
GetSpaceColour(
    vec3  lWorldPositionVec3,
    vec3  lCameraPositionVec3,
    vec3  lSunPositionVec3,
    vec3  lColour1Vec3,
    vec3  lColour2Vec3,
    vec3  lColour3Vec3,
    float lfPower )
{
    vec3 lCircumSolar0Vec3;
    vec3 lCircumSolar1Vec3;
    vec3 lCircumSolar2Vec3;

    lCircumSolar0Vec3 = /*GammaCorrectInput*/ ( lColour1Vec3 );
    lCircumSolar1Vec3 = /*GammaCorrectInput*/ (lColour2Vec3)-lCircumSolar0Vec3;
    lCircumSolar2Vec3 = /*GammaCorrectInput*/ (lColour3Vec3)-lCircumSolar0Vec3;

    float lfVS = dot( normalize( lWorldPositionVec3 - lCameraPositionVec3 ), normalize( lSunPositionVec3 - lCameraPositionVec3 ) );

    if( lfVS > 0.0 )
    {
        lfVS = pow( lfVS, lfPower );
        return (lCircumSolar0Vec3)+( lCircumSolar1Vec3 * lfVS );
    }
    else
    {
        lfVS = pow( abs( lfVS ), lfPower );
        return (lCircumSolar0Vec3)+( lCircumSolar2Vec3 * lfVS );
    }
}

vec3
GetSpaceColour(
    vec3  lWorldDirVec3,
    vec3  lSunDirVec3,
    vec3  lColour1Vec3,
    vec3  lColour2Vec3,
    vec3  lColour3Vec3,
    float lfPower )
{
    vec3 lCircumSolar0Vec3;
    vec3 lCircumSolar1Vec3;
    vec3 lCircumSolar2Vec3;

    lCircumSolar0Vec3 = lColour1Vec3;
    lCircumSolar1Vec3 = lColour2Vec3 - lCircumSolar0Vec3;
    lCircumSolar2Vec3 = lColour3Vec3 - lCircumSolar0Vec3;

    float lfVS = dot( lWorldDirVec3, lSunDirVec3 );

    if( lfVS > 0.0 )
    {
        lfVS = pow( lfVS, lfPower );
        return lCircumSolar0Vec3 + lCircumSolar1Vec3 * lfVS;
    }
    else
    {
        lfVS = pow( -lfVS, lfPower );
        return lCircumSolar0Vec3 + lCircumSolar2Vec3 * lfVS;
    }
}

#endif
