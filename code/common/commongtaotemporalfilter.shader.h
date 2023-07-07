////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonGTAOTemporalFilter.h
///     @author     User
///     @date       
///
///     @brief      CommonGTAOTemporalFilter
///
///     Copyright (c) 2019 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONGTAOTEMPORALFILTER_H
#define D_COMMONGTAOTEMPORALFILTER_H

//-----------------------------------------------------------------------------
//      Include files
#include "Common/CommonUtils.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Functions
#define sqrt_fast   sqrt_fast_0

//-----------------------------------------------------------------------------
//      Global Data

vec2 BilinearSample( SAMPLER2DARG(lTextureMap), vec2 lFragCoordsVec2 )
{
    vec2 lTextureSizeVec2 = vec2(GetTexResolution(lTextureMap));
    vec2 lTexelSizeVec2   = 1.0 / lTextureSizeVec2;

    // 4-tap neighbour sample, assuming the incoming texture co-ordinate has
    // already been pre-stepped by half a pixel, so that point-sampling doesn't
    // oscillate between texels due to numerical imprecision.
    // TODO(don): Is it worth expressing this as textureGather? Note multiplying UVs up by texture resolution and then
    // down again to get texel-rounded co-ordinates suffers from precision errors.
    vec2 lafSamplesVec2[ 4 ];
    for ( int ii = 0; ii < 2; ++ii )
    {
        for ( int jj = 0; jj < 2; ++jj )
        {                        
            vec2 lCoordVec2                 = lFragCoordsVec2 + vec2( lTexelSizeVec2.x * ii, lTexelSizeVec2.y * jj );            
            lafSamplesVec2[ ii * 2 + jj ]   = Texture2DNoFiltering( SAMPLER2DPARAM( lTextureMap ), lCoordVec2 ).xy;
        }
    }

    // Calculate bilinear weights for a half-pixel-pre-stepped texture
    // co-ordinate.
    vec2 lWeightsVec2 = fract( lFragCoordsVec2 * lTextureSizeVec2 );
    lWeightsVec2      = max( lWeightsVec2 - 0.5, vec2(0, 0) );

    vec2  lSampleVec2;

    // Bilinear blend
    vec2  lfTempA   = mix( lafSamplesVec2[ 0 ], lafSamplesVec2[ 2 ], lWeightsVec2.x );
    vec2  lfTempB   = mix( lafSamplesVec2[ 1 ], lafSamplesVec2[ 3 ], lWeightsVec2.x );

    lSampleVec2     = mix( lfTempA, lfTempB, lWeightsVec2.y );

    return lSampleVec2;
}

void GatherNeighbours( SAMPLER2DARG(lThisFrameAOMap), vec2 lFragCoordsVec2, inout float lafNeighbours[8] )
{
    // Get the size of a texel
    vec2 lTextureSizeVec2   = vec2( GetTexResolution( lThisFrameAOMap ) );
    vec2 lTexelSizeVec2     = 1.0 / lTextureSizeVec2;
    float lfU               = lTexelSizeVec2.x;
    float lfV               = lTexelSizeVec2.y;

    // Sample the "rounded" 3x3 neighbourhood of the current solution
    // TODO(don): textureGatherRed?
    lafNeighbours[ 0 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2(-lfU, -lfV)).x;
    lafNeighbours[ 1 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2(   0, -lfV)).x;
    lafNeighbours[ 2 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2( lfU, -lfV)).x;
    lafNeighbours[ 3 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2(-lfU,    0)).x;    
    lafNeighbours[ 4 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2( lfU,    0)).x;
    lafNeighbours[ 5 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2(-lfU,  lfV)).x;
    lafNeighbours[ 6 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2(   0,  lfV)).x;
    lafNeighbours[ 7 ] = Texture2DNoFiltering(SAMPLER2DPARAM(lThisFrameAOMap), lFragCoordsVec2 + vec2( lfU,  lfV)).x;
}

float GetSpeedWeight( vec2 lMotionVec2, float lfSpeedWeightPrev )
{
    // Approximate speed based on current motion. Flatten the speed curve using a square root approximation.
    // Blend speed with its previous value from the accumulation buffer.
    
#ifdef D_PLATFORM_METAL
    const float  kfSpeedCoeff        = 128.0;
    const float  kfSpeedWeightAcc    = 0.5;

    // Avoid tiny movements likely caused by jitter
    // TODO(sal): make threshold a function of the jitter vector
    const float  kfSpeedLowThreshold = 0.000185; 
#else
    STATIC_CONST float  kfSpeedCoeff        = 128.0;
    STATIC_CONST float  kfSpeedWeightAcc    = 0.5;

    // Avoid tiny movements likely caused by jitter
    // TODO(sal): make threshold a function of the jitter vector
    STATIC_CONST float  kfSpeedLowThreshold = 0.000185; 
#endif
    const float         kfMotion            = length( lMotionVec2 );
    const float         kfSpeed             = kfMotion * step( kfSpeedLowThreshold, kfMotion ) * kfSpeedCoeff;
    float               lfSpeedWeight       = min( 1.0, kfSpeed );
    lfSpeedWeight                           = mix( lfSpeedWeightPrev, lfSpeedWeight, kfSpeedWeightAcc );

    return lfSpeedWeight;
}

float GetBlendWeight( float lfSpeedWeight, float lfSpeedWeightPrev )
{
#ifdef D_PLATFORM_METAL
    // Blend current speed with diff between current speed and accumulation
    const float  kfDiffSpeedAcc   = 0.09;
#else
    // Blend current speed with diff between current speed and accumulation
    STATIC_CONST float  kfDiffSpeedAcc   = 0.09;
#endif
    float               lfBlendWeight    = mix( abs( lfSpeedWeight - lfSpeedWeightPrev ), lfSpeedWeight, kfDiffSpeedAcc );
    return lfBlendWeight;
}


float ClampAOHistory( float lfOldAO, float lfThisFrameAO, float lafNeighbours[8], vec2 lFragCoordsVec2, float lfBlendWeight )
{
    float lfNeighboursMin = 1.0;
    float lfNeighboursMax = 0.0;

    for ( int ii = 0; ii < 8; ++ii )
    {
        lfNeighboursMin = min( lfNeighboursMin, lafNeighbours[ ii ] );
        lfNeighboursMax = max( lfNeighboursMax, lafNeighbours[ ii ] );
    }

    lfNeighboursMin = min( lfNeighboursMin, lfThisFrameAO );
    lfNeighboursMax = max( lfNeighboursMax, lfThisFrameAO );

    // Clip history to sample neighbourhood
    // Each frame of the GTAO solution will render slightly different results as they introduce new ray
    // directions based on the frame number. For flat walls this clamping becomes a problem as the min and
    // max will be the same and subsequent pixels will always be clamped, causing flickering. Adding
    // a bit of tolerance to the clamp allows this flickering to be cancelled out while introducing
    // ghosting which is very hard to see.


    // GTAO Siggraph 2016 presentation, page 100
    // Keep tolerance window tight when speed is high    
#ifdef D_PLATFORM_METAL
    const float  kfToleranceMin       = 0.00;
    const float  kfToleranceMax       = 0.0675;
    const float  kfClampScale         = 36.0;
    const float  kfNeighboursMinRange = 0.35;
#else
    STATIC_CONST float  kfToleranceMin       = 0.00;
    STATIC_CONST float  kfToleranceMax       = 0.0675;
    STATIC_CONST float  kfClampScale         = 36.0;
    STATIC_CONST float  kfNeighboursMinRange = 0.35;
#endif
    const float         kfNeighboursRange    = max( lfNeighboursMax - lfNeighboursMin, kfNeighboursMinRange );
    const float         kfClampWindow        = lfBlendWeight * kfNeighboursRange * kfClampScale;        
    const float         kfTolerance          = max( ( 1.0 - min( 1.0, kfClampWindow ) ) * kfToleranceMax, kfToleranceMin );    

    lfNeighboursMin -= kfTolerance;
    lfNeighboursMax += kfTolerance;    

    lfOldAO = clamp( lfOldAO, lfNeighboursMin, lfNeighboursMax );
    return lfOldAO;
}

vec4 GTAOTemporalFilter(
    SAMPLER2DARG( lAOHistoryMap ),
    SAMPLER2DARG( lDepthMap ),
    SAMPLER2DARG( lThisFrameAOMap ),
    SAMPLER2DARG( lMotionMap ),
    in vec2 lFragCoordsVec2,
    in vec4 lClipPlanesVec4,
    in vec4 lFrameBufferSizeVec4
)
{
    vec2  lTextureSizeVec2  = vec2( GetTexResolution( lDepthMap ) );
    vec2  lTexelSizeVec2    = vec2( 1.0, 1.0 ) / lTextureSizeVec2;
    vec2  lTexCoordsVec2    = lFragCoordsVec2 + lTexelSizeVec2 * fract( lFrameBufferSizeVec4.xy / lTextureSizeVec2 );

    float lfDepthRevZ       = Texture2DNoFiltering( SAMPLER2DPARAM( lDepthMap ), lTexCoordsVec2 ).x;

    // Ignore background
    if ( lfDepthRevZ == 0.0 )
        return vec4( 1.0, 0.0, 0.0, 0.0 );

    // Reprojected UVs in previous frame
    vec3 lEncodedMotionVec2             = Texture2DNoFiltering( SAMPLER2DPARAM( lMotionMap ), lTexCoordsVec2 ).xyz;
    vec2 lMotionVec2                    = DecodeMotion( lEncodedMotionVec2.xy );    
    vec2 lReprojectFragCoordsVec2       = lFragCoordsVec2 + lMotionVec2;

    // Reset for texels that were previously outside the view
    float lfThisFrameAO = Texture2DNoFiltering( SAMPLER2DPARAM( lThisFrameAOMap ), lFragCoordsVec2 ).x;
    if ( lReprojectFragCoordsVec2.x <  0 || lReprojectFragCoordsVec2.y <  0 ||
         lReprojectFragCoordsVec2.x >= 1 || lReprojectFragCoordsVec2.y >= 1)
        return vec4( lfThisFrameAO, 0, 0, 0 );

    float lafNeighbours[ 8 ];
    GatherNeighbours( SAMPLER2DPARAM( lThisFrameAOMap ), lFragCoordsVec2, lafNeighbours );
    
    vec2  lHistorySampleVec2            = BilinearSample( SAMPLER2DPARAM( lAOHistoryMap ), lReprojectFragCoordsVec2 );
    float lfOldAO                       = lHistorySampleVec2.x;
    float lfSpeedWeightPrev             = lHistorySampleVec2.y;
    float lfSpeedWeight                 = GetSpeedWeight( lMotionVec2,   lfSpeedWeightPrev );
    float lfBlendWeight                 = GetBlendWeight( lfSpeedWeight, lfSpeedWeightPrev );
    lfOldAO                             = ClampAOHistory( lfOldAO, lfThisFrameAO, lafNeighbours, lFragCoordsVec2, lfBlendWeight );
#ifdef D_PLATFORM_METAL
    // Widen or tighten blend factor based on blend weight        
    const float  kfBlendLambdaMin    = 0.0975;
    const float  kfBlendLambdaMax    = 0.1875;        
#else
    // Widen or tighten blend factor based on blend weight        
    STATIC_CONST float  kfBlendLambdaMin    = 0.0975;
    STATIC_CONST float  kfBlendLambdaMax    = 0.1875;        
#endif
    const float         kfBlendLambda       = mix( kfBlendLambdaMin, kfBlendLambdaMax, lfBlendWeight );

    // EMA
    vec4 lNewAOTexel;
    lNewAOTexel.x = mix( lfOldAO, lfThisFrameAO, kfBlendLambda );
    lNewAOTexel.y = lfSpeedWeight;
    lNewAOTexel.z = 0;
    lNewAOTexel.w = 0;
    return lNewAOTexel;

}

#endif