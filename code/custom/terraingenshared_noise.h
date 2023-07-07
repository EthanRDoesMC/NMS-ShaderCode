//-----------------------------------------------------------------------------
//Terrain gen core code shared between CPU and GPU
//This can be included from headers in c++ code
//-----------------------------------------------------------------------------

#ifndef _TGENSHARED_NOISE_H_
#define _TGENSHARED_NOISE_H_
#include "Custom/TerrainGenShared_Core.h"
#include "Custom/SharedBegin.inl"

TKINLINE vec3
CalculateSeedOffset(
    int liSeed)
{
    vec3 lSeedOffset;
    lSeedOffset.x = 64.0f * (liSeed % 13);
    lSeedOffset.y = 64.0f * (liSeed % 13);
    lSeedOffset.z = 64.0f * (liSeed % 17);
    return lSeedOffset;
}

float
Noise3d(float x, float y, float z);

float
Noise3d(float x, float y, float z, OUTPARAM(float) dnoise_dx, OUTPARAM(float) dnoise_dy, OUTPARAM(float) dnoise_dz);

float
ScaleNoise(
    float lfNoise,
    float lfAmount );

float
ScaleNoise(
    float lfNoise,
    float lfOneMinusAmount,
    float lfOneOverAmount);

float
UberNoise(
    vec3 lPosition,
    int   liOctaves,
    float lfPerturbFeatures,      
    float lfSharpToRoundFeatures, 
    float lfAmplifyFeatures,
    float lfAltitudeErosion,
    float lfRidgeErosion,         
    float lfSlopeErosion,         
    float lfLacunarity,           
    float lfGain,
	float lfRemapFromMin,
	float lfRemapFromMax,
	float lfRemapToMin,
	float lfRemapToMax,
    float lfSlopeGain,
    float lfSlopeBias );

void
UberNoiseSOA4(
    float* lpStore,
    vec3* lPos,
    int   liOctaves,
    float lfPerturbFeatures,
    float lfSharpToRoundFeatures,
    float lfAmplifyFeatures,
    float lfAltitudeErosion,
    float lfRidgeErosion,
    float lfSlopeErosion,
    float lfLacunarity,
    float lfGain,
    float lfRemapFromMin,
    float lfRemapFromMax,
    float lfRemapToMin,
    float lfRemapToMax,
    float lfSlopeGain,
    float lfSlopeBias );

float 
OctaveNoise( 
    vec3 lPosition,
    int liNumOctaves );

float 
SmoothNoise(
    vec3 lPosition, 
    vec3 lSeedOffset, 
    float lfFeatureScale, 
    int liNumOctaves);

vec3 
TurbulencePositions( 
    vec3 lPos,
    vec3 lFrequency, 
    vec3 lPower, 
    int liNumOctaves );

float
GenerateRegionNoise(     
    vec3        lPosition,
    float       lfRegionSize,
    float       lfRegionScale,
    float       lfRegionRatio,
    float       lfRegionGain,
    vec3        lSeedOffset,
	int         lbIsLegacy );

TKINLINE float
GenerateRegionNoise(
    vec3        lPosition,
    float       lfRegionSize,
    float       lfRegionScale,
    float       lfRegionRatio,
    float       lfRegionGain,
    int         liSeed,
    int         lbIsLegacy)
{
    return GenerateRegionNoise(lPosition, lfRegionSize, lfRegionScale, lfRegionRatio, lfRegionGain, CalculateSeedOffset(liSeed), lbIsLegacy);
}

float
GenerateNoise2D(
    vec3        lPosition,
    float       lfRegionSize,
    float       lfRegionScale,
    float       lfRegionRatio,
    float       lfRegioGain,
    vec3        lSeedOffset,
	int         lbIsLegacy );

TKINLINE float
GenerateNoise2D(
    vec3        lPosition,
    float       lfRegionSize,
    float       lfRegionScale,
    float       lfRegionRatio,
    float       lfRegionGain,
    int         liSeed,
    int         lbIsLegacy)
{
    return GenerateNoise2D(lPosition, lfRegionSize, lfRegionScale, lfRegionRatio, lfRegionGain, CalculateSeedOffset(liSeed), lbIsLegacy);
}

void
VoronoiDistanceOnCube(
    vec3                    lSpherePosition,
    int                     liSeed,
    float                   lfRadius,
    float                   lfScaleFactor,
    OUTPARAM(uint64)        liID,
    OUTPARAM(float)         lfCenterDistance,
    OUTPARAM(vec3)          lCenterPosition,
    OUTPARAM(vec3)          lFacePosition,
    OUTPARAM(vec3)          lNormal
    #ifdef TK_TGEN_VORONOI_DEBUG
    ,OUTPARAM(sVoronoiDebugOutput)  lDebug
    #endif
    );

//-----------------------------------------------------------------------------
//Make a 3D random vector between -0.5 to 0.5 from an input vector
//Note: The dot products are really important, as they mean each component 
//affects the other components, so every coordinate gives a different vector
//-----------------------------------------------------------------------------

TKFORCEINLINE
vec3
VoronoiRandom3f(
    vec3 lPosition,
    uint liSeed)
{
    STATIC_CONST vec3 kVoronoiVecMultX = vec3(1.0f, 57.0f, 113.0f);
    STATIC_CONST vec3 kVoronoiVecMultY = vec3(57.0f, 113.0f, 1.0f);
    STATIC_CONST vec3 kVoronoiVecMultZ = vec3(113.0f, 1.0f, 57.0f);
    float lfX = dot(lPosition, kVoronoiVecMultX);
    float lfY = dot(lPosition, kVoronoiVecMultY);
    float lfZ = dot(lPosition, kVoronoiVecMultZ);
    return vec3(RandFloatFromFloat(lfX, liSeed), RandFloatFromFloat(lfY, liSeed), RandFloatFromFloat(lfZ, liSeed)) * 0.01f;
}

//-----------------------------------------------------------------------------
//Calculate an id based off a seed and a random offset
//-----------------------------------------------------------------------------
TKFORCEINLINE
uint64
VoronoiID(
    uint    luSeed,
    vec3    lRandomOffset)
{
    uint64 liX = HashMixFloatTo64(lRandomOffset.x);
    uint64 liY = HashMixFloatTo64(lRandomOffset.y);
    uint64 liZ = HashMixFloatTo64(lRandomOffset.z);
    uint64 lResult = HashMixPairU64(liX, liY);
    lResult = HashMixPairU64(lResult, liZ);
    lResult = HashMixPairU64(lResult, (uint64)luSeed);
    return lResult;
}


#include "Custom/SharedEnd.inl"
#endif
