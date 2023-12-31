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
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkBuildObjectTypesPerPositionComputeUniforms )
{
    uint luNumObjectTypes = lUniforms.miNumObjectTypes;
    uint luNumElements = ReadGDS_u32(lUniforms.muNumPositionsGDSAddress)*luNumObjectTypes;
    uint luDispatchIdx = dispatchThreadID.x;

    //build the result for this entry
    sAddObjectPosIndex lResult;
    uint luAdd = 0;
    if (luDispatchIdx < luNumElements)
    {
        //get position this corresponds to and the random seed for this position
        uint luPos = luDispatchIdx/luNumObjectTypes;
        uint luRandSeed  = lUniforms.mauPositionSeeds[luPos];

        //get the offset from the beginning of dispatches corresponding to this position
        uint liOffsetFromPosStart = luDispatchIdx%luNumObjectTypes;

        //attempt to create the object, and increment position in buffer on success
        luAdd = BuildObjectTypeForPosition(lUniforms.mPriorityMap, luPos, luRandSeed, liOffsetFromPosStart, lResult);
    }


    //get a predicate that contains a 1 bit if want to add something, and a 0 if not
#if defined( D_PLATFORM_PROSPERO ) && !defined D_COMPUTE_DISABLEWAVE32
    uint64 pred = wave32::__v_cmp_ne_u32(0,luAdd);
    //get number of threads below this one that want to add something (i.e. the offset from the counter position we should add at)
    uint offset = __v_mbcnt_lo_u32_b32((uint)pred, 0);
#else	
    uint64 pred = __v_cmp_ne_u32(0,luAdd);
    //get number of threads below this one that want to add something (i.e. the offset from the counter position we should add at)
    uint offset = __v_mbcnt_hi_u32_b32((uint)(pred >> 32), __v_mbcnt_lo_u32_b32((uint)pred, 0));
#endif


    //get total bits that are set in 'pred'
    //note: this is a valid scalar action, as 'pred' is a scalar value returned from __v_cmp_ne_u32
    uint sum = (uint)__s_bcnt1_i32_b64(pred);

    //perform the ordered append, adding 'sum to the gds counter and get the base position
    uint base = OrderedCount(lUniforms.muNumOutputPositionsGDSAddress,sum,K_ORDERED_WAVE_RELEASE | K_ORDERED_WAVE_DONE);

    //if we want to output a result, output it to base+offset
    uint output_idx = base+offset;
    if (luAdd && output_idx < lUniforms.miObjectTypePerPositionBufferSize)
    {
        lUniforms.maObjectTypePerPositionBuffer[output_idx] = lResult;
    }

}
  
