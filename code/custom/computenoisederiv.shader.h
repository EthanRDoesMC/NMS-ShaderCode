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

	vec3 request = GLOBALS(mRequests)[idx];

	vec3 dres;
	GLOBALS(mNoiseResults)[idx] = Noise3d(request.x, request.y, request.z, dres.x, dres.y, dres.z);
	GLOBALS(mDerivResults)[idx] = dres;
}
  