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


// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkIndirectDispatchComputeUniforms )
{
    //read and if necessary clamp and store the counter
    uint luCounter = ReadGDS_u32(lUniforms.muGDSCounterAddress);
    if (lUniforms.muPreClamp > 0)
    {
        luCounter = min(luCounter, lUniforms.muPreClamp);
        WriteGDS_u32(lUniforms.muGDSCounterAddress,luCounter);
    }

    //scale by the multiplier
    luCounter *= lUniforms.muMultiply;

    //calculate number of thread groups based on counter and thread group size
    uint luThreadGroupSize = lUniforms.muThreadGroupSize;
    uint luGroups = max((luCounter+luThreadGroupSize-1) / luThreadGroupSize,1);

    //fill out the args (note: must clamp to min of 1 group, as a 0 dispatch is invalid)
    lUniforms.mauIndirectDispatchArgs[0] = max(luGroups,1);
    lUniforms.mauIndirectDispatchArgs[1] = 1;
    lUniforms.mauIndirectDispatchArgs[2] = 1;
}
  
