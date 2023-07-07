////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonGTAOSpatialFilter.h
///     @author     User
///     @date       
///
///     @brief      CommonGTAOSpatialFilter
///
///     Copyright (c) 2019 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONGTAOSPATIALFILTER_H
#define D_COMMONGTAOSPATIALFILTER_H

//-----------------------------------------------------------------------------
//      Include files

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Functions

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Functions 

float SampleDenormalisedDepth(SAMPLER2DARG(lDepthMap), vec4 lClipPlanesVec4, vec2 lFragCoordsVec2)
{
    float lfDepth = Texture2DNoFiltering(SAMPLER2DPARAM(lDepthMap), lFragCoordsVec2).r;
    return FastDenormaliseDepth(lClipPlanesVec4, lfDepth);
}

vec4 SingleAxisBilateralBlur(
    SAMPLER2DARG(lAOMap),
    SAMPLER2DARG(lDepthMap),
    vec4 lClipPlanesVec4,
    vec2 lTexelSizeVec2,
    vec2 lFragCoordsVec2,
    vec2 lAxisVec2,
    float lfDepthRef
)
{
    // Feeble attempt to reduce v_exp_f32 latency (not that it matters, given the texture reads)
#if 0
    {
        vec2 lSamplePos0Vec2 = lFragCoordsVec2 + -1 * lAxisVec2 * lTexelSizeVec2;
        vec2 lSamplePos1Vec2 = lFragCoordsVec2 +  0 * lAxisVec2 * lTexelSizeVec2;
        vec2 lSamplePos2Vec2 = lFragCoordsVec2 +  1 * lAxisVec2 * lTexelSizeVec2;
        vec2 lSamplePos3Vec2 = lFragCoordsVec2 +  2 * lAxisVec2 * lTexelSizeVec2;

        float lfDepth0 = SampleDenormalisedDepth(SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lSamplePos0Vec2);
        float lfDepth1 = SampleDenormalisedDepth(SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lSamplePos1Vec2);
        float lfDepth2 = SampleDenormalisedDepth(SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lSamplePos2Vec2);
        float lfDepth3 = SampleDenormalisedDepth(SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lSamplePos3Vec2);

        vec4 lDepthRefVec4 = lfDepthRef;
        vec4 lDepthVec4 = vec4(lfDepth0, lfDepth1, lfDepth2, lfDepth3);
        vec4 lDeltaDepthVec4 = lDepthRefVec4 - lDepthVec4;
        vec4 lWeightVec4 = exp2(-lDeltaDepthVec4 * lDeltaDepthVec4);

        float lfAOSample0 = Texture2DNoFiltering(SAMPLER2DPARAM(lAOMap), lSamplePos0Vec2).x;
        float lfAOSample1 = Texture2DNoFiltering(SAMPLER2DPARAM(lAOMap), lSamplePos1Vec2).x;
        float lfAOSample2 = Texture2DNoFiltering(SAMPLER2DPARAM(lAOMap), lSamplePos2Vec2).x;
        float lfAOSample3 = Texture2DNoFiltering(SAMPLER2DPARAM(lAOMap), lSamplePos3Vec2).x;
        vec4 lAOSamplesVec4 = vec4(lfAOSample0, lfAOSample1, lfAOSample2, lfAOSample3);

        float lfTotalAO = dot(lAOSamplesVec4, lWeightVec4);
        float lfTotalWeight = dot(lWeightVec4, 1);
        return lfTotalAO / lfTotalWeight;
    }
#endif

    // AO is sampled in 4x4 blocks of 16 random directions. This blur equally weights all samples, assuming nearby
    // pixels share similar occlusion results. The sampling is also not centred, which can lead to pixel-shifting,
    // but in practice this has not been observed.
    float lfTotalAO = 0;
    float lfTotalWeight = 0;
    for (int x = -1; x < 3; x++)
    {
        vec2 lSamplePosVec2 = lFragCoordsVec2 + x * lAxisVec2 * lTexelSizeVec2;
        float lfDepth1 = SampleDenormalisedDepth(SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lSamplePosVec2);

        // Calculate depth weight in projective space to preserve hard edges
        // GCN, exp2 maps to HW intrinsic v_exp_f32
        float lfNorm = lfDepthRef * 0.25;
        float lfDeltaDepth = (lfDepthRef - lfDepth1) / lfNorm;
        float lfWeight = exp2(-lfDeltaDepth * lfDeltaDepth);

        float lfAOSample = Texture2DNoFiltering(SAMPLER2DPARAM(lAOMap), lSamplePosVec2).x;
        lfTotalAO += lfAOSample * lfWeight;
        lfTotalWeight += lfWeight;
    }

    return float2vec4(lfTotalAO / lfTotalWeight);
}

vec4 GTAOSpatialFilter(
    SAMPLER2DARG(lAOMap),
    SAMPLER2DARG(lDepthMap),
    in vec2 lFragCoordsVec2,
    in vec4 lFrameBufferSizeVec4,
    in vec4 lClipPlanesVec4
)
{
    // Get reference depth
    float lfDepth0 = SampleDenormalisedDepth(SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lFragCoordsVec2);
    vec2 lTexelSizeVec2 = 1.0 / lFrameBufferSizeVec4.xy;

#ifdef D_DENOISE_H
    return SingleAxisBilateralBlur(SAMPLER2DPARAM(lAOMap), SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lTexelSizeVec2, lFragCoordsVec2, vec2(1, 0), lfDepth0);
#endif
#ifdef D_DENOISE_V
    return SingleAxisBilateralBlur(SAMPLER2DPARAM(lAOMap), SAMPLER2DPARAM(lDepthMap), lClipPlanesVec4, lTexelSizeVec2, lFragCoordsVec2, vec2(0, 1), lfDepth0);
#endif
}

#endif