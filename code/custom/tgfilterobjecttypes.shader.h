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

#pragma argument(nofastmath)

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkFilterObjectTypesComputeUniforms )
{
    //get index
    uint idx = dispatchThreadID.x;

    //load number of elements
    uint luNumElements = ReadGDS_u32(lUniforms.muPosEntriesCountGDSAddress);

    //get filter flags (set of flags to say what we're gonna filter)
    uint luFilterFlags = lUniforms.muFilterFlags;
    
    //start with not valid
    uint luValid = 0;

    //check if within range
    if (idx < luNumElements)
    {
        //valid element, so start assuming valid
        luValid = 1;

        //K_REGIONFILTER_CURRENT_BITS == read existing filter bits (used for doing multiple filter passes)
        if (luFilterFlags & K_REGIONFILTER_CURRENT)
        {
            //load the existing bits from the array using the thread group index (64 bits per array entry, 64 threads per group!)
            uint64 luCurrentBits    = lUniforms.mauFilterBits[GetThreadGroupIdX()];

            //use cndmask, which will return 1 if the bit in luCurrentBits corresponding to the current thead is 1
#if defined( D_PLATFORM_PROSPERO ) && !defined D_COMPUTE_DISABLEWAVE32
            luValid &= wave32::__v_cndmask_b32(0,1,luCurrentBits);
#else			
            luValid &= __v_cndmask_b32(0,1,luCurrentBits);
#endif
        }

        //get this entry
        sAddObjectPosIndex lPosEntry            = lUniforms.maPosEntriesBuffer[idx];
        uint luObjectType                       = lPosEntry.muPosIndex_ObjectType >> 16;
        uint luPosIndex                         = lPosEntry.muPosIndex_ObjectType & 0xffff;

        //generate instance info
        sSpawnObjectPerInstanceParams lPerInstanceParams;
        sRegionHeightResult lResult             = lUniforms.maPositions[luPosIndex];
        uint luResultRandSeed                   = lUniforms.maPositionSeeds[luPosIndex];
        lPerInstanceParams.mPosition            = lResult.mPosition;
        lPerInstanceParams.mfSlopeValue         = lResult.mfSlopeValue;
        lPerInstanceParams.mfRatio              = lResult.mfRatio;
        lPerInstanceParams.meMaterial           = lResult.mMaterial;
        lPerInstanceParams.meSecondaryMaterial  = lResult.mSecondaryMaterial;
        lPerInstanceParams.muRandSeed           = luResultRandSeed;
        lPerInstanceParams.mNormal              = lResult.mNormal;
        lPerInstanceParams.mfDensity            = lPosEntry.mfDensity;

        //build test results
        sSpawnObjectResult lOutResult;
        ESpawnObjectStage leResultStage = eSpawnStage_Complete;

        if (luValid && (luFilterFlags & K_REGIONFILTER_CHECKTILETYPE))
        {
            if (!TrySpawnObject_CheckTileType(lOutResult, lUniforms.mGlobalSpawnParams, lUniforms.maPerTypeSpawnParams[luObjectType], lPerInstanceParams))
            {
                leResultStage = eSpawnStage_CheckTileType;
                luValid = 0;
            }
        }

        if (luValid && (luFilterFlags & K_REGIONFILTER_CHECKHEIGHT))
        {
            if (!TrySpawnObject_CheckHeight(lUniforms.mGlobalSpawnParams,lUniforms.maPerTypeSpawnParams[luObjectType],lPerInstanceParams))
            {
                leResultStage = eSpawnStage_CheckHeight;
                luValid = 0;
            }
        }

        if (luValid && (luFilterFlags & K_REGIONFILTER_CHECKPOSITION))
        {
            if (!TrySpawnObject_CheckPosition(lUniforms.mGlobalSpawnParams,lUniforms.maPerTypeSpawnParams[luObjectType],lPerInstanceParams))
            {
                leResultStage = eSpawnStage_CheckPosition;
                luValid = 0;
            }
        }

        if (luValid && (luFilterFlags & K_REGIONFILTER_CHECKDENSITY))
        {
            if (!TrySpawnObject_CheckDensity(lUniforms.mGlobalSpawnParams,lUniforms.maPerTypeSpawnParams[luObjectType],lPerInstanceParams))
            {
                leResultStage = eSpawnStage_CheckDensity;
                luValid = 0;
            }
        }

        if (luValid && (luFilterFlags & K_REGIONFILTER_CHECKNOISE))
        {
            if (!TrySpawnObject_CheckNoise(lOutResult,lUniforms.mGlobalSpawnParams,lUniforms.maPerTypeSpawnParams[luObjectType],lPerInstanceParams))
            {
                leResultStage = eSpawnStage_CheckNoise;
                luValid = 0;
            }
        }


        //store result
        lUniforms.maiSpawnResultStages[idx] = leResultStage;
        lUniforms.maSpawnResults[idx] = lOutResult;
    }

    //get a predicate that contains a 1 bit if valid, and a 0 if not for each thread
#if defined( D_PLATFORM_PROSPERO ) && !defined D_COMPUTE_DISABLEWAVE32
    uint64 pred = wave32::__v_cmp_ne_u32(0, luValid);
#else
    uint64 pred = __v_cmp_ne_u32(0, luValid);
#endif

    //thread 0 outputs the results from all threads
    if (GetThreadIdX() == 0)
        lUniforms.mauFilterBits[GetThreadGroupIdX()] = pred;
}
  
