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

#ifdef D_PLATFORM_SWITCH
#define GLOBALS(NAME) NAME
#else
#define GLOBALS(NAME)  lUniforms.mpGlobalUniforms. ## NAME
struct GlobalUniforms
{
#endif
	REGULARBUFFER(float,mInputs,0);
	RW_REGULARBUFFER(float,mOutputs,1);
#ifdef D_PLATFORM_SWITCH
#else
};
#endif

struct PerDispatchUniforms
{
	int	    miOperation MTL_ID(0);  //0=add, 1=sub, 2=mul, 3=div
    int     miNumOperations;
    int     miFirstAInput;
    int     miFirstBInput;
    int     miFirstOutput;
};

DECLARE_UNIFORMS
#ifndef D_PLATFORM_SWITCH
	DECLARE_PTR( GlobalUniforms, mpGlobalUniforms          )
#endif
	DECLARE_PTR( PerDispatchUniforms, mpPerDispatchUniforms )	
DECLARE_UNIFORMS_END

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_SRT(64, 1, 1)
{
    //get index and bail if gone to far
	int idx = (int) dispatchThreadID.x;
	if (idx >= lUniforms.mpPerDispatchUniforms.miNumOperations)
		return;

    //calculate buffer positions
	int lAIdx = idx + lUniforms.mpPerDispatchUniforms.miFirstAInput;    //add offset into buffer
	int lBIdx = idx + lUniforms.mpPerDispatchUniforms.miFirstBInput;    //add offset into buffer
	int lOutIdx = idx + lUniforms.mpPerDispatchUniforms.miFirstOutput;   //add offset into buffer

    //do the operation
    if (lUniforms.mpPerDispatchUniforms.miOperation == 0)
        GLOBALS(mOutputs)[lOutIdx] = GLOBALS(mInputs)[lAIdx]+GLOBALS(mInputs)[lBIdx];
    else if (lUniforms.mpPerDispatchUniforms.miOperation == 1)
        GLOBALS(mOutputs)[lOutIdx] = GLOBALS(mInputs)[lAIdx]-GLOBALS(mInputs)[lBIdx];
    else if (lUniforms.mpPerDispatchUniforms.miOperation == 2)
        GLOBALS(mOutputs)[lOutIdx] = GLOBALS(mInputs)[lAIdx]*GLOBALS(mInputs)[lBIdx];
    else if (lUniforms.mpPerDispatchUniforms.miOperation == 3)
        GLOBALS(mOutputs)[lOutIdx] = GLOBALS(mInputs[)lAIdx]/GLOBALS(mInputs)[lBIdx];
}
  