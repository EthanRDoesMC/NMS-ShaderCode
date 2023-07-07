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
ROBUFF( float , mafX, 0 );
ROBUFF( float , mafY, 1 );
ROBUFF( float , mafZ, 2 );
ROBUFF( float , mafUberNoise, 3);
ROBUFF( float , mafRegionNoise, 4);
             
RWBUFF( float , mafNoise, 5);
#endif

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkUber3DCombineComputeUniforms )
{
    //get index and bail if gone to far
    uint idx = dispatchThreadID.x;
    if (idx >= lUniforms.miNumElements)
        return;

    float lfOneMinusSqrtRatio    = lUniforms.mfOneMinusSqrtRegionRatio;
    float lfOneOverSqrtRatio     = lUniforms.mfOneOverSqrtRegionRatio;
    float lfHeight               = lUniforms.mfHeight;

    float lfNoise           = GETBUFF(mafUberNoise)[idx];
    lfNoise                 = saturate( lfNoise );
    lfNoise                 = ScaleNoise(lfNoise,lfOneMinusSqrtRatio,lfOneOverSqrtRatio);
    lfNoise                *= GETBUFF(mafRegionNoise)[idx];

    // plateau noise
    if( lUniforms.mfPlateauStratas > 0.0f )
    {
        vec3 lPosition = vec3( GETBUFF(mafX)[ idx ], GETBUFF(mafY)[ idx ], GETBUFF(mafZ)[ idx ] );

        float lfPlateauNoise = 0.0f;

        if( lUniforms.mfPlateauRegionSize != 0.0f )
        {
            lfPlateauNoise = SmoothNoise(
                lPosition, 
                lUniforms.mSeedOffset,
                lUniforms.mfPlateauRegionSize,
                1 );
        }

        float lfFloor = floor( lfNoise * lUniforms.mfPlateauStratas );
        float lfFract = fract( lfNoise * lUniforms.mfPlateauStratas );

        lfFract = Gain( lfFract, lUniforms.miPlateauSharpness );

        lfNoise = lerp( ( lfFloor + lfFract ) * ( 1.0f / lUniforms.mfPlateauStratas ), lfNoise, lfPlateauNoise );
    }

    lfNoise                *= lUniforms.mfHeight;

    lfNoise                 = ldexp(round(ldexp(lfNoise,10)),-10);

    GETBUFF(mafNoise)[idx] = lfNoise;
}
  