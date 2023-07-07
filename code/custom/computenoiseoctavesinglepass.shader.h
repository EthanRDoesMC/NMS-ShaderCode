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
#include "Custom/ComputeNoiseCommon.h"

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_SRT( 64, 1, 1 )
{
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.mpPerDispatchUniforms.miNumRequests)
		return;

	idx += lUniforms.mpPerDispatchUniforms.miFirstRequest;

	vec3 lPosition = GLOBALS(mRequests)[idx];

	float lfTotal = 0.0f;
	float lfFrequency = 1.0f;
	float lfAmplitude = 1.0f;

	// We have to keep track of the largest possible amplitude,
	// because each octave adds more, and we need a value in [-1, 1].
	float lfMaxAmplitude = 0;

	for (int liOctave = 0; liOctave < lUniforms.mpPerDispatchUniforms.miOctaves; liOctave++)
	{
		lfTotal += Noise3d(lPosition.x * lfFrequency, lPosition.y * lfFrequency, lPosition.z * lfFrequency) * lfAmplitude;

		lfFrequency *= 2.0f;
		lfMaxAmplitude += lfAmplitude;
		lfAmplitude *= 0.5f;
	}

	GLOBALS(mNoiseResults)[idx] = lfTotal / lfMaxAmplitude;
}
  