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
	ROBUFF(sVoronoiRequest,maRequests,0);
	RWBUFF(sVoronoiResults,maResults,1);	
#endif


// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkVoronoiComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;

    sVoronoiResults lResults;
    sVoronoiRequest lRequest = GETBUFF(maRequests)[idx];

    #ifdef TK_TGEN_VORONOI_DEBUG
    sVoronoiDebugOutput lDbg;
    VoronoiDistanceOnCube(lRequest.mSpherePosition, lRequest.miSeed, lRequest.mfRadius, lRequest.mfScaleFactor, lResults.miID, lResults.mfCenterDistance, lResults.mCenterPosition, lResults.mFloorPosition, lResults.mNormal, lDbg);
    GETBUFF(maDebugResults)[idx] = lDbg;
    #else
    VoronoiDistanceOnCube(lRequest.mSpherePosition, lRequest.miSeed, lRequest.mfRadius, lRequest.mfScaleFactor, lResults.miID, lResults.mfCenterDistance, lResults.mCenterPosition, lResults.mFloorPosition, lResults.mNormal);
    #endif

    GETBUFF(maResults)[idx] = lResults;

}
  