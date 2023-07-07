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


// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkGenerateUberNoiseLayer3DComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;

    vec3 lPosition = vec3(GETBUFF(mafX[idx]), GETBUFF(mafY)[idx], GETBUFF(mafZ)[idx]);

    lPosition += lUniforms.mSeedOffset;

    lPosition *= lUniforms.mfFeatureScale;

    GETBUFF(mafNoise)[idx] = UberNoise(
                                    lPosition, 
                                    lUniforms.miOctaves, 
                                    lUniforms.mfPerturbFeatures,
                                    lUniforms.mfSharpToRoundFeatures,
                                    lUniforms.mfAmplifyFeatures,
                                    lUniforms.mfAltitudeErosion,
                                    lUniforms.mfRidgeErosion,
                                    lUniforms.mfSlopeErosion,
                                    lUniforms.mfLacunarity,
                                    lUniforms.mfGain,
                                    lUniforms.mfRemapFromMin,
                                    lUniforms.mfRemapFromMax,
                                    lUniforms.mfRemapToMin,
                                    lUniforms.mfRemapToMax,
                                    lUniforms.mfSlopeGain,
                                    lUniforms.mfSlopeBias
                                    );
}