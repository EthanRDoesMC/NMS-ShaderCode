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

#ifndef FFX_FSR2_UPSAMPLE_H
#define FFX_FSR2_UPSAMPLE_H

FfxFloat32 SmoothStep(FfxFloat32 x, FfxFloat32 a, FfxFloat32 b)
{
    x = clamp((x - a) / (b - a), 0.f, 1.f);
    return x * x * (3.f - 2.f * x);
}

FFX_STATIC const FfxUInt32 iLanczos2SampleCount = 16;

void DeringingWithMinMax(UPSAMPLE_F3 fDeringingMin, UPSAMPLE_F3 fDeringingMax, FFX_PARAMETER_INOUT UPSAMPLE_F3 fColor, FFX_PARAMETER_OUT FfxFloat32 fRangeSimilarity)
{
    fRangeSimilarity = fDeringingMin.x / fDeringingMax.x;
    fColor = clamp(fColor, fDeringingMin, fDeringingMax);
}

void Deringing(RectificationBoxData clippingBox, FFX_PARAMETER_INOUT UPSAMPLE_F3 fColor)
{
    fColor = clamp(fColor, clippingBox.aabbMin, clippingBox.aabbMax);
}

void Deringing(RectificationBoxDataLightweight clippingBox, FFX_PARAMETER_INOUT UPSAMPLE_F3 fColor)
{
    fColor = clamp(fColor, clippingBox.aabbCenter - clippingBox.aabbExtent, clippingBox.aabbCenter + clippingBox.aabbExtent);
}

UPSAMPLE_F GetUpsampleLanczosWeight(UPSAMPLE_F2 fSrcSampleOffset, UPSAMPLE_F2 fKernelWeight)
{
    UPSAMPLE_F2 fSrcSampleOffsetBiased = UPSAMPLE_F2(fSrcSampleOffset * fKernelWeight);
    UPSAMPLE_F fSampleWeight = Lanczos2ApproxSq(dot(fSrcSampleOffsetBiased, fSrcSampleOffsetBiased));		// TODO: check other distances (l0, l1, linf...)

    return fSampleWeight;
}

UPSAMPLE_F Pow3(UPSAMPLE_F x)
{
    return x * x * x;
}

UPSAMPLE_F4 ComputeUpsampledColorAndWeight(FFX_MIN16_I2 iPxHrPos, UPSAMPLE_F2 fKernelWeight, FFX_PARAMETER_INOUT RectificationBoxData clippingBox)
{
    // We compute a sliced lanczos filter with 2 lobes (other slices are accumulated temporaly)
    FfxFloat32x2 fDstOutputPos = FfxFloat32x2(iPxHrPos) + FFX_BROADCAST_FLOAT32X2(0.5f);      // Destination resolution output pixel center position
    FfxFloat32x2 fSrcOutputPos = fDstOutputPos * DownscaleFactor();                   // Source resolution output pixel center position
    FfxInt32x2 iSrcInputPos = FfxInt32x2(floor(fSrcOutputPos));                     // TODO: what about weird upscale factors...

    UPSAMPLE_F3 fSamples[iLanczos2SampleCount];

    FfxFloat32x2 fSrcUnjitteredPos = (FfxFloat32x2(iSrcInputPos) + FFX_BROADCAST_FLOAT32X2(0.5f)) - Jitter();                // This is the un-jittered position of the sample at offset 0,0
    
    UPSAMPLE_I2 offsetTL;
    offsetTL.x = (fSrcUnjitteredPos.x > fSrcOutputPos.x) ? UPSAMPLE_I(-2) : UPSAMPLE_I(-1);
    offsetTL.y = (fSrcUnjitteredPos.y > fSrcOutputPos.y) ? UPSAMPLE_I(-2) : UPSAMPLE_I(-1);

    RectificationBox fRectificationBox;

    //Load samples
    // If fSrcUnjitteredPos.y > fSrcOutputPos.y, indicates offsetTL.y = -2, sample offset Y will be [-2, 1], clipbox will be rows [1, 3].
    // Flip row# for sampling offset in this case, so first 0~2 rows in the sampled array can always be used for computing the clipbox.
    // This reduces branch or cmove on sampled colors, but moving this overhead to sample position / weight calculation time which apply to less values.
    const FfxBoolean bFlipRow = fSrcUnjitteredPos.y > fSrcOutputPos.y;
    const FfxBoolean bFlipCol = fSrcUnjitteredPos.x > fSrcOutputPos.x;

    UPSAMPLE_F2 fOffsetTL = UPSAMPLE_F2(offsetTL);

    FFX_UNROLL
    for (FfxInt32 row = 0; row < 4; row++) {

        FFX_UNROLL
        for (FfxInt32 col = 0; col < 4; col++) {
            FfxInt32 iSampleIndex = col + (row << 2);

            FfxInt32x2 sampleColRow = FfxInt32x2(bFlipCol ? (3 - col) : col, bFlipRow ? (3 - row) : row);
            FfxInt32x2 iSrcSamplePos = FfxInt32x2(iSrcInputPos) + offsetTL + sampleColRow;

            const FfxInt32x2 sampleCoord = ClampLoad(iSrcSamplePos, FfxInt32x2(0, 0), FfxInt32x2(RenderSize()));

            fSamples[iSampleIndex] = LoadPreparedInputColor(FFX_MIN16_I2(sampleCoord));
        }
    }

    RectificationBoxReset(fRectificationBox, fSamples[0]);

    UPSAMPLE_F3 fColor = UPSAMPLE_F3(0.f, 0.f, 0.f);
    UPSAMPLE_F fWeight = UPSAMPLE_F(0.f);
    UPSAMPLE_F2 fBaseSampleOffset = UPSAMPLE_F2(fSrcUnjitteredPos - fSrcOutputPos);

    FFX_UNROLL
    for (FfxUInt32 iSampleIndex = 0; iSampleIndex < iLanczos2SampleCount; ++iSampleIndex)
    {
        FfxInt32 row = FfxInt32(iSampleIndex >> 2);
        FfxInt32 col = FfxInt32(iSampleIndex & 3);

        const FfxInt32x2 sampleColRow = FfxInt32x2(bFlipCol ? (3 - col) : col, bFlipRow ? (3 - row) : row);
        const UPSAMPLE_F2 fOffset = fOffsetTL + UPSAMPLE_F2(sampleColRow);
        UPSAMPLE_F2 fSrcSampleOffset = fBaseSampleOffset + fOffset;

        FfxInt32x2 iSrcSamplePos = FfxInt32x2(iSrcInputPos) + FfxInt32x2(offsetTL) + sampleColRow;

        UPSAMPLE_F fSampleWeight = UPSAMPLE_F(IsOnScreen(FFX_MIN16_I2(iSrcSamplePos), FFX_MIN16_I2(RenderSize()))) * GetUpsampleLanczosWeight(fSrcSampleOffset, fKernelWeight);

        // Update rectification box
        if(all(FFX_LESS_THAN(FfxInt32x2(col, row), FFX_BROADCAST_INT32X2(3))))
        {
            //update clipping box in non-locked areas
            const UPSAMPLE_F fSrcSampleOffsetSq = dot(fSrcSampleOffset, fSrcSampleOffset);
            UPSAMPLE_F fBoxSampleWeight = UPSAMPLE_F(1) - ffxSaturate(fSrcSampleOffsetSq / UPSAMPLE_F(3));
            fBoxSampleWeight *= fBoxSampleWeight;
            RectificationBoxAddSample(fRectificationBox, fSamples[iSampleIndex], fBoxSampleWeight);
        }

        fWeight += fSampleWeight;
        fColor += fSampleWeight * fSamples[iSampleIndex];
    }

    // Normalize for deringing (we need to compare colors)
    fColor = fColor / (abs(fWeight) > FSR2_EPSILON ? fWeight : UPSAMPLE_F(1.f));

    RectificationBoxComputeVarianceBoxData(fRectificationBox);
    clippingBox = RectificationBoxGetData(fRectificationBox);

    Deringing(RectificationBoxGetData(fRectificationBox), fColor);

    if (any(FFX_LESS_THAN(fKernelWeight, UPSAMPLE_F2_BROADCAST(1.0f)))) {
        fWeight = UPSAMPLE_F(averageLanczosWeightPerFrame);
    }

    return UPSAMPLE_F4(fColor, ffxMax(UPSAMPLE_F(0), fWeight));
}

UPSAMPLE_F4 ComputeUpsampledColorAndWeightLightweight(FfxFloat32x2 fLrPos, UPSAMPLE_F2 fKernelWeight, FfxFloat32x2 fMotion, FFX_PARAMETER_INOUT RectificationBoxDataLightweight clippingBox)
{
    FfxFloat32x2 fGridPos       = floor(fLrPos) + FFX_BROADCAST_FLOAT32X2(0.5f);
    FfxFloat32x2 fGridPosUnjitt = fGridPos - Jitter();                                      // This is the un-jittered position of the sample at offset 0,0
    FfxFloat32x2 fSmpOffset     = fLrPos - fGridPosUnjitt;
    FfxFloat32x2 fSmpBasePos    = fGridPos - FfxFloat32x2(fSmpOffset.x < 0.0f, fSmpOffset.y < 0.0f);
    UPSAMPLE_F2  fSmpFract      = ffxFract(UPSAMPLE_F2(fSmpOffset));
    UPSAMPLE_F   fSharpness     = ffxSaturate(fKernelWeight.x - UPSAMPLE_F(0.5f));
    UPSAMPLE_F   fWeight        = UPSAMPLE_F(averageLanczosWeightPerFrame);

    if (fKernelWeight.x > 1.3666f)
    {
        UPSAMPLE_F fSmpDistSq   = dot(UPSAMPLE_F2(fSmpFract) - UPSAMPLE_F2(0.5f, 0.5f), UPSAMPLE_F2(fSmpFract) - UPSAMPLE_F2(0.5f, 0.5f));
        UPSAMPLE_F fWeightCurve = UPSAMPLE_F(1.0f) - UPSAMPLE_F(4.0f) * fSmpDistSq;

        fWeight += (fKernelWeight.x - UPSAMPLE_F(1.3666f)) * (UPSAMPLE_F(0.446f)  - UPSAMPLE_F(1.62f) * fWeightCurve * fWeightCurve);
    }

    UPSAMPLE_F   c              = UPSAMPLE_F(-1.0f) + UPSAMPLE_F(2.25f) * fSharpness;
    UPSAMPLE_F   b              = UPSAMPLE_F( 2.0f) * ffxSaturate(-c);
    UPSAMPLE_F   d              = abs(c) * (UPSAMPLE_F(0.6f) + UPSAMPLE_F(0.4f) * abs(c));
    UPSAMPLE_F2  x              = fSmpFract;
    UPSAMPLE_F2  s              = x * x * (UPSAMPLE_F(3.0f) - UPSAMPLE_F(2.0f) * x);

    // Optimised weight calculations using approximation curves
    UPSAMPLE_F   fSmpCenterPull = UPSAMPLE_F(0.5f);
    UPSAMPLE_F2  fOffset0       = UPSAMPLE_F2_BROADCAST(UPSAMPLE_F(-1.0f) + fSmpCenterPull);
    UPSAMPLE_F2  fOffset3       = UPSAMPLE_F2_BROADCAST(UPSAMPLE_F( 2.0f) - fSmpCenterPull);
    UPSAMPLE_F2  fOffset12      = c > UPSAMPLE_F(0.0f) ? s * (UPSAMPLE_F(1.0f) - c) + x * c : UPSAMPLE_F(0.5f) * d + s * (UPSAMPLE_F(1.0f) - d);
    UPSAMPLE_F2  fOffset03      = c > UPSAMPLE_F(0.0f) ? x : s;
    FfxFloat32x2 fRenderSizeRcp = FFX_BROADCAST_FLOAT32X2(1.0f) / RenderSize();

    UPSAMPLE_F2  x2_minus_x     = x * (x - UPSAMPLE_F(1.0f));
    UPSAMPLE_F2  fWeight03      = UPSAMPLE_F(1.0f / 6.0f) *  b *  (UPSAMPLE_F(3.0f) * x2_minus_x + UPSAMPLE_F(1.0f)) + c * x2_minus_x;
    UPSAMPLE_F2  fWeight12      = UPSAMPLE_F2(1.0f, 1.0f) - fWeight03;
    UPSAMPLE_F2  fWeight3       = fWeight03 * UPSAMPLE_F2(fOffset03);
    UPSAMPLE_F2  fWeight0       = fWeight03 - fWeight3;
    UPSAMPLE_F   fWeightSum;

    fWeight0    /= fWeight12;
    fWeight3    /= fWeight12;
    fWeightSum   = (UPSAMPLE_F(1.0f) + fWeight0.x + fWeight3.x + fWeight0.y + fWeight3.y) * (UPSAMPLE_F(1.0f) - fSmpCenterPull);
    fWeight03    = - fSmpCenterPull * (fWeight0 + fWeight3);

    UPSAMPLE_F3  fSample;
    UPSAMPLE_F3  fColor   = UPSAMPLE_F3(0.f, 0.f, 0.f);
    UPSAMPLE_F3  fMoment0 = UPSAMPLE_F3(0.f, 0.f, 0.f);
    UPSAMPLE_F3  fMoment1 = UPSAMPLE_F3(0.f, 0.f, 0.f);

    FfxFloat32x2 fOffset;
    fOffset.x       = fOffset12.x;

    fOffset.y       = fOffset0 .y;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * fWeight0 .y;

    fOffset.y       = fOffset3 .y;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * fWeight3 .y;

    fOffset.y       = fOffset03.y;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * fWeight03.y;

    fOffset.y       = fOffset12.y;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * (UPSAMPLE_F(1.0f) - fSmpCenterPull);

    fOffset.x       = fOffset03.x;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * fWeight03.x;

    fOffset.x       = fOffset3 .x;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * fWeight3 .x;

    fOffset.x       = fOffset0 .x;
    fSample         = SamplePreparedInputColor((fSmpBasePos + fOffset) * fRenderSizeRcp);
    fMoment0       += fSample;
    fMoment1       += fSample * fSample;
    fColor         += fSample * fWeight0 .x;

    fColor         /= fWeightSum;

    // Rectification
    {
        UPSAMPLE_F3 fAvg;
        UPSAMPLE_F3 fStd;
        UPSAMPLE_F  fGamma;

        fAvg            = fMoment0 / UPSAMPLE_F(7.0f);
        fStd            = ffxSqrt(abs(fMoment1 * UPSAMPLE_F3_BROADCAST(7.0f) - fMoment0 * fMoment0)) / UPSAMPLE_F3_BROADCAST(7.0f);
        fGamma          = clamp(fSharpness * UPSAMPLE_F(4.0f), UPSAMPLE_F(0.75f), UPSAMPLE_F(2.0f));

        RectificationBoxDataLightweightSetCenter(clippingBox, fAvg);
        RectificationBoxDataLightweightSetExtent(clippingBox, fGamma * fStd);
    }

    // Deringing
    Deringing(clippingBox, fColor);

    return UPSAMPLE_F4(fColor, ffxMax(UPSAMPLE_F(0), fWeight));
}


#endif //!defined( FFX_FSR2_UPSAMPLE_H )
