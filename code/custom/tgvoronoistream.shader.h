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
	ROBUFF(float,         mafX, 0);
	ROBUFF(float,         mafY, 1);
	ROBUFF(float,         mafZ, 2);

    RWBUFF(uint64,        maiID, 3);
    RWBUFF(float,         mafCenterDistance, 4);
	RWBUFF(float,         maCenterX, 5);
	RWBUFF(float,         maCenterY, 6);
	RWBUFF(float,         maCenterZ, 7);
	RWBUFF(float,         maFloorPositionX, 8);
	RWBUFF(float,         maFloorPositionY, 9);
	RWBUFF(float,         maFloorPositionZ, 10);
	RWBUFF(float,         maNormalX, 11);
	RWBUFF(float,         maNormalY, 12);
	RWBUFF(float,         maNormalZ, 13);

    #ifdef TK_TGEN_VORONOI_DEBUG
	RWBUFF(sVoronoiDebugOutput, maDebugResults, 14);
    #endif
#endif	


// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkVoronoiStreamComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;

    vec3 lPos = vec3(GETBUFF(mafX)[idx], GETBUFF(mafY)[idx], GETBUFF(mafZ)[idx]);

    uint64 liId;
    float lfCenterDistance;
    vec3 lCenterPosition;
    vec3 lFloorPosition;
    vec3 lNormal;

    #ifdef TK_TGEN_VORONOI_DEBUG
    sVoronoiDebugOutput lDbg;
    VoronoiDistanceOnCube(lPos, lUniforms.miSeed, lUniforms.mfRadius, lUniforms.mfScaleFactor, liId, lfCenterDistance, lCenterPosition, lFloorPosition, lNormal, lDbg);
    lUniforms.maDebugResults[idx] = lDbg;
    #else
    VoronoiDistanceOnCube(lPos, lUniforms.miSeed, lUniforms.mfRadius, lUniforms.mfScaleFactor, liId, lfCenterDistance, lCenterPosition, lFloorPosition, lNormal);
    #endif

    GETBUFF(maiID[)idx] = liId;
    GETBUFF(mafCenterDistance)[idx] = lfCenterDistance;
    GETBUFF(maCenterX)[idx] = lCenterPosition.x;
    GETBUFF(maCenterY)[idx] = lCenterPosition.y;
    GETBUFF(maCenterZ)[idx] = lCenterPosition.z;
    GETBUFF(maFloorPositionX)[idx] = lFloorPosition.x;
    GETBUFF(maFloorPositionY)[idx] = lFloorPosition.y;
    GETBUFF(maFloorPositionZ)[idx] = lFloorPosition.z;
    GETBUFF(maNormalX)[idx] = lNormal.x;
    GETBUFF(maNormalY)[idx] = lNormal.y;
    GETBUFF(maNormalZ)[idx] = lNormal.z;  
}
  