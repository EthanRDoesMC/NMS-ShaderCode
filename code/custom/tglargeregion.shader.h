////////////////////////////////////////////////////////////////////////////////
///
///     @file       ComputeNoise.h
///     @author     ccummings
///     @date       
///
///     @brief      Basic shader to execute a series of Noise3D samples
///
///     Copyright (c) 2016 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "Common/Defines.shader.h"
#include "Custom/TerrainGenCommon.h"

#ifdef D_PLATFORM_ORBIS			
#pragma argument(nofastmath)
#endif


#ifndef D_PLATFORM_ORBIS			
    ROBUFF(float,           mafPositionX, 0);
    ROBUFF(float,           mafPositionY, 1);
    ROBUFF(float,           mafPositionZ, 2);
    RWBUFF(sVoronoiResults, maVoronoiResults, 3);
#endif

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkLargeNoiseComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;

    vec3 lTurbPos = TurbulencePositions(vec3(GETBUFF(mafPositionX)[idx], GETBUFF(mafPositionY)[idx], GETBUFF(mafPositionZ)[idx]), float2vec3(1.0f / 20.0f), float2vec3(5.0f), 2);

    float lfDivisionFactor = lUniforms.mfRegionVoronoiPointDivisions * 0.5f;

    sVoronoiResults lResults;
    VoronoiDistanceOnCube(lTurbPos, lUniforms.miRegionVoronoiSectorSeed, lUniforms.mfRegionPlanetRadius, lfDivisionFactor, lResults.miID, lResults.mfCenterDistance, lResults.mCenterPosition, lResults.mFloorPosition, lResults.mNormal);

    GETBUFF(maVoronoiResults)[idx] = lResults;
}
  