// This file is part of the FidelityFX SDK.
//
// Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

FFX_STATIC const FfxFloat32 DepthClipBaseScale = 4.0f;

FfxFloat32 ComputeSampleDepthClip(FFX_MIN16_I2 iPxSamplePos, FfxFloat32 fPreviousDepth, FfxFloat32 fPreviousDepthBilinearWeight, FfxFloat32 fCurrentDepthViewSpace)
{
    FfxFloat32 fPrevNearestDepthViewSpace = abs(ConvertFromDeviceDepthToViewSpace(fPreviousDepth));

    // Depth separation logic ref: See "Minimum Triangle Separation for Correct Z-Buffer Occlusion"
    // Intention: worst case of formula in Figure4 combined with Ksep factor in Section 4
    // TODO: check intention and improve, some banding visible
    const FfxFloat32 fHalfViewportWidth = RenderSize().x * 0.5f;
    FfxFloat32 fDepthThreshold = ffxMin(fCurrentDepthViewSpace, fPrevNearestDepthViewSpace);

    // WARNING: Ksep only works with reversed-z with infinite projection.
    const FfxFloat32 Ksep = 1.37e-05f;
    FfxFloat32 fRequiredDepthSeparation = Ksep * fDepthThreshold * TanHalfFoV() * fHalfViewportWidth;
    FfxFloat32 fDepthDiff = fCurrentDepthViewSpace - fPrevNearestDepthViewSpace;

    FfxFloat32 fDepthClipFactor = (fDepthDiff > 0) ? ffxSaturate(fRequiredDepthSeparation / fDepthDiff) : 1.0f;

#ifdef _DEBUG
    rw_debug_out[iPxSamplePos] = FfxFloat32x4(fCurrentDepthViewSpace, fPrevNearestDepthViewSpace, fDepthDiff, fDepthClipFactor);
#endif

    return fPreviousDepthBilinearWeight * fDepthClipFactor * DepthClipBaseScale;
}

FfxFloat32 ComputeDepthClip(FfxFloat32x2 fUvSample, FfxFloat32 fCurrentDepthViewSpace)
{
    FfxFloat32x2 fPxSample = fUvSample * RenderSize() - 0.5f;
    FFX_MIN16_I2 iPxSample = FFX_MIN16_I2(floor(fPxSample));
    FfxFloat32x2 fPxFrac = ffxFract(fPxSample);

    const FfxFloat32 fBilinearWeights[2][2] = {
        {
            (1 - fPxFrac.x) * (1 - fPxFrac.y),
            (fPxFrac.x) * (1 - fPxFrac.y)
        },
        {
            (1 - fPxFrac.x) * (fPxFrac.y),
            (fPxFrac.x) * (fPxFrac.y)
        }
    };

    FfxFloat32 fDepth = 0.0f;
    FfxFloat32 fWeightSum = 0.0f;
    for (FfxInt32 y = 0; y <= 1; ++y) {
        for (FfxInt32 x = 0; x <= 1; ++x) {
            FFX_MIN16_I2 iSamplePos = iPxSample + FFX_MIN16_I2(x, y);
            if (IsOnScreen(iSamplePos, FFX_MIN16_I2(RenderSize()))) {
                FfxFloat32 fBilinearWeight = fBilinearWeights[y][x];
                if (fBilinearWeight > reconstructedDepthBilinearWeightThreshold) {
                    fDepth += ComputeSampleDepthClip(iSamplePos, LoadReconstructedPrevDepth(iSamplePos), fBilinearWeight, fCurrentDepthViewSpace);
                    fWeightSum += fBilinearWeight;
                }
            }
        }
    }

    return (fWeightSum > 0) ? fDepth / fWeightSum : DepthClipBaseScale;
}

FfxFloat32 ComputeSampleDepthClipLightweight(FfxFloat32 fPreviousDepth, FfxFloat32 fPreviousDepthBilinearWeight, FfxFloat32 fCurrentDepthViewSpace, FfxFloat32 fSeparationFactor)
{
    FfxFloat32 fPrevNearestDepthViewSpace = abs(ConvertFromDeviceDepthToViewSpace(fPreviousDepth));

    FfxFloat32 fDepthThreshold = ffxMin(fCurrentDepthViewSpace, fPrevNearestDepthViewSpace);
    FfxFloat32 fRequiredDepthSeparation = fDepthThreshold * fSeparationFactor;
    FfxFloat32 fDepthDiff = fCurrentDepthViewSpace - fPrevNearestDepthViewSpace;

    FfxFloat32 fDepthClipFactor = fDepthDiff > 0 ? ffxSaturate(fRequiredDepthSeparation / fDepthDiff) : 1.0f;

    return fPreviousDepthBilinearWeight * fDepthClipFactor * DepthClipBaseScale;
}

FfxFloat32 ComputeDepthClipLightweight(FfxFloat32x2 fUvSample, FfxFloat32 fCurrentDepth, FfxFloat32 fCurrentDepthViewSpace)
{

    FfxFloat32x2 fPxSample = fUvSample * RenderSize() - 0.5f;
    FFX_MIN16_I2 iPxSample = FFX_MIN16_I2(floor(fPxSample));
    FfxFloat32x2 fPxFrac = ffxFract(fPxSample);

    if (!IsOnScreen(iPxSample, FFX_MIN16_I2(RenderSize()) + FFX_MIN16_I2(1, 1))) {
        return 1.0f;
    }

    const FfxFloat32x4 fBilinearWeights = {
            (1 - fPxFrac.x) * (1 - fPxFrac.y),
            (    fPxFrac.x) * (1 - fPxFrac.y),
            (1 - fPxFrac.x) * (    fPxFrac.y),
            (    fPxFrac.x) * (    fPxFrac.y)
    };

    // WARNING: Ksep only works with reversed-z with infinite projection.
    const FfxFloat32 Ksep = 1.37e-05f;
    const FfxFloat32 fHalfViewportWidth = RenderSize().x * 0.5f;
    FfxFloat32 fSeparationFactor = Ksep * TanHalfFoV() * fHalfViewportWidth;

    FfxFloat32 fDepth = 0.0f;
    FfxFloat32 fWeightSum = 0.0f;
    FfxFloat32x4 fPreviousReconstructedDepths = GatherReconstructedPrevDepth(FfxFloat32x2(iPxSample + FFX_MIN16_I2(1, 1)) / RenderSize()).wzxy;

    FFX_UNROLL
    for (FfxInt32 ii = 0; ii < 4; ++ii) {
        if (fBilinearWeights[ii] > reconstructedDepthBilinearWeightThreshold) {
            fDepth += ComputeSampleDepthClipLightweight(fPreviousReconstructedDepths[ii], fBilinearWeights[ii], fCurrentDepthViewSpace, fSeparationFactor);
            fWeightSum += fBilinearWeights[ii];
        }
    }

    fDepth /= fWeightSum;

    #if FFX_FSR2_CUSTOM_OPTION_DEPTH_MIN_BIAS
    // NOTE(sal): when depth is 0 disocclusions can inaccurate;
    // for this reason we bias the depth clip factor by a tiny amount
    // which is needed to ensure locks are cleared correctly
    if (fCurrentDepth == 0.0f)
    {
        fDepth = min(fDepth, 252.0f / 255.0f);
    }
    #endif

    return fDepth;
}

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
void DepthClip(FFX_MIN16_I2 iPxPos, FFX_PARAMETER_INOUT FfxFloat32 fOutDepthClip)
#else
void DepthClip(FFX_MIN16_I2 iPxPos)
#endif
{
    FfxFloat32x2 fDepthUv = (FfxFloat32x2(iPxPos) + 0.5f) / RenderSize();
    FfxFloat32x2 fMotionVector = LoadDilatedMotionVector(iPxPos);
    FfxFloat32x2 fDilatedUv = fDepthUv + fMotionVector;
    FfxFloat32 fCurrentDepth = LoadDilatedDepth(iPxPos);
    FfxFloat32 fCurrentDepthViewSpace = abs(ConvertFromDeviceDepthToViewSpace(fCurrentDepth));

    #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_DEPTH_CLIP == 0
    FfxFloat32 fDepthClip = ComputeDepthClip(fDilatedUv, fCurrentDepthViewSpace);
    #else
    FfxFloat32 fDepthClip = ComputeDepthClipLightweight(fDilatedUv, fCurrentDepth, fCurrentDepthViewSpace);
    #endif

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
    fOutDepthClip = fDepthClip;
#else
    StoreDepthClip(iPxPos, fDepthClip);
#endif
}
