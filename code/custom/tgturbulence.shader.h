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

#ifndef D_PLATFORM_ORBIS
RWBUFF(float, mafX, 0);
RWBUFF(float, mafY, 1);
RWBUFF(float, mafZ, 2);
#endif

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkTurbulenceComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;

    float x = GETBUFF(mafX)[idx];
    float y = GETBUFF(mafY)[idx];
    float z = GETBUFF(mafZ)[idx];

    vec3 res = TurbulencePositions(vec3(x,y,z),lUniforms.mFrequency,lUniforms.mPower,lUniforms.miNumOctaves);

    GETBUFF(mafX)[idx] = res.x;
    GETBUFF(mafY)[idx] = res.y;
    GETBUFF(mafZ)[idx] = res.z;
}
  