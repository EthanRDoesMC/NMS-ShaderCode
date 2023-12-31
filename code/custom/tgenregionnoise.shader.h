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
	ROBUFF(float, mafX, 0);
	ROBUFF(float, mafY, 1);
	ROBUFF(float, mafZ, 2);

	RWBUFF(float, mafNoise, 3);
	RWBUFF(vec4, mavDebugOutput, 4);
#endif

STATIC_CONST int kiDefaultRegionOctaves = 3; 

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkGenerateRegionNoiseComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;

    vec3 lSeedOffset                    = lUniforms.mSeedOffset;
    float lfFeatureScale                = lUniforms.mfFeatureScale;
    float lfRegionScale                 = lUniforms.mfRegionScale;
    float lfRegionRatio                 = lUniforms.mfRegionRatio;
    float lfOneMinusSqrtRegionRatio     = lUniforms.mfOneMinusSqrtRegionRatio;
    float lfOneOverSqrtRegionRatio      = lUniforms.mfOneOverSqrtRegionRatio;
    float lfRegionGain                  = lUniforms.mfRegionGain;

    float lfRegionNoise;

    lfRegionNoise = 1.0f;

    if ( lfRegionScale > 1.0f && lfRegionRatio > 0.0f && lfRegionRatio < 1.0f )
    {
        vec3 lPosition = vec3(GETBUFF(mafX)[idx], GETBUFF(mafY)[idx], GETBUFF(mafZ)[idx]);
        lfRegionNoise = SmoothNoise(
            lPosition,
            lSeedOffset,
            lfFeatureScale,
            kiDefaultRegionOctaves);

        lfRegionNoise = ScaleNoise(lfRegionNoise,lfOneMinusSqrtRegionRatio,lfOneOverSqrtRegionRatio);

        lfRegionNoise = saturate(lfRegionNoise);
        lfRegionNoise = RegionGain( lfRegionNoise, lfRegionGain);
    }

    GETBUFF(mafNoise)[idx] = lfRegionNoise;

}
  