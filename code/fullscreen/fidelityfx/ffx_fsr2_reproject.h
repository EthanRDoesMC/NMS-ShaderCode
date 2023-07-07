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

#ifndef FFX_FSR2_REPROJECT_H
#define FFX_FSR2_REPROJECT_H

FFX_MIN16_F4 WrapHistory(FfxInt32x2 iPxSample)
{
    return LoadHistory(iPxSample);
}

DeclareCustomFetchBicubicSamplesMin16(FetchHistorySamples, WrapHistory)
DeclareCustomTextureSample(HistorySample, Lanczos2, FetchHistorySamples)

FfxFloat32x4 HistorySample5Taps(FfxFloat32x2 fUvSample, FfxInt32x2 iTextureSize)
{
    FfxFloat32x2 fPxSample      = fUvSample * FfxFloat32x2(iTextureSize) - FFX_BROADCAST_FLOAT32X2(0.5f);
    FfxInt32x2   iPxSample      = FfxInt32x2(floor(fPxSample));
    FfxFloat32x2 fPxFrac        = ffxFract(fPxSample);

    FfxFloat32x2 fWeight0       = fPxFrac * (FFX_BROADCAST_FLOAT32X2(-0.5f) + fPxFrac * (FFX_BROADCAST_FLOAT32X2( 1.0f) - FFX_BROADCAST_FLOAT32X2(0.5f) * fPxFrac));
    FfxFloat32x2 fWeight2       = fPxFrac * (FFX_BROADCAST_FLOAT32X2( 0.5f) + fPxFrac * (FFX_BROADCAST_FLOAT32X2( 2.0f) - FFX_BROADCAST_FLOAT32X2(1.5f) * fPxFrac));
    FfxFloat32x2 fWeight1       = FFX_BROADCAST_FLOAT32X2(1.0f)   + fPxFrac * fPxFrac * (FFX_BROADCAST_FLOAT32X2(-2.5f) + FFX_BROADCAST_FLOAT32X2(1.5f) * fPxFrac);
    FfxFloat32x2 fWeight3       =                                   fPxFrac * fPxFrac * (FFX_BROADCAST_FLOAT32X2(-0.5f) + FFX_BROADCAST_FLOAT32X2(0.5f) * fPxFrac);
    FfxFloat32x2 fWeight12      = fWeight1 + fWeight2;

    FfxFloat32x2 fTexCoords1    = iPxSample   + FFX_BROADCAST_FLOAT32X2(0.5f);
    FfxFloat32x2 fTexCoords0    = fTexCoords1 - FFX_BROADCAST_FLOAT32X2(1.0f);
    FfxFloat32x2 fTexCoords3    = fTexCoords1 + FFX_BROADCAST_FLOAT32X2(2.0f);
    FfxFloat32x2 fTexCoords12   = fTexCoords1 + fWeight2 / fWeight12;
    FfxFloat32x2 fTexSizeRcp    = 1.0f / FfxFloat32x2(iTextureSize);

    fTexCoords0  *= fTexSizeRcp;
    fTexCoords3  *= fTexSizeRcp;
    fTexCoords12 *= fTexSizeRcp;

    FfxFloat32x4 fHistorySample;
    fHistorySample  = SampleHistory(FfxFloat32x2(fTexCoords12.x, fTexCoords0.y )) * fWeight12.x * fWeight0.y;
    fHistorySample += SampleHistory(FfxFloat32x2(fTexCoords0.x,  fTexCoords12.y)) * fWeight0.x  * fWeight12.y;
    fHistorySample += SampleHistory(FfxFloat32x2(fTexCoords12.x, fTexCoords12.y)) * fWeight12.x * fWeight12.y;
    fHistorySample += SampleHistory(FfxFloat32x2(fTexCoords3.x,  fTexCoords12.y)) * fWeight3.x  * fWeight12.y;
    fHistorySample += SampleHistory(FfxFloat32x2(fTexCoords12.x, fTexCoords3.y )) * fWeight12.x * fWeight3.y;

    FfxFloat32 fWeightSum;
    fWeightSum  = fWeight12.x * fWeight0.y;
    fWeightSum += fWeight0.x  * fWeight12.y;
    fWeightSum += fWeight12.x * fWeight12.y;
    fWeightSum += fWeight3.x  * fWeight12.y;
    fWeightSum += fWeight12.x * fWeight3.y;

    return fHistorySample / fWeightSum;
}

UPSAMPLE_F4 HistorySample3Taps(FfxFloat32x2 fUvSample, FfxInt32x2 iTextureSize, FfxUInt32 uIndex)
{
    // Slice xy cross sharpening across two frames (sharpen horizontally on even frames, vertically on odd)
    FfxFloat32   fTexSize       = FfxFloat32(iTextureSize[uIndex]);
    FfxFloat32   fTexSizeRcp    = 1.0f / fTexSize;
    FfxFloat32   fSmpPos        = fUvSample[uIndex] * fTexSize;
    FfxFloat32   fPxCtrPos      = floor(fSmpPos  - 0.5f) + 0.5f;
    FfxFloat32   fSmpFrac       = fSmpPos - fPxCtrPos;
    FfxFloat32   fTexCoords0    = (fPxCtrPos  - 1.0f) * fTexSizeRcp;
    FfxFloat32   fTexCoords3    = fTexCoords0 + 3.0f  * fTexSizeRcp;

    FfxFloat32x2 fTexCoordsSmp0 = uIndex == 0 ? FfxFloat32x2(fTexCoords0,  fUvSample.y) : FfxFloat32x2(fUvSample.x, fTexCoords0);
    FfxFloat32x2 fTexCoordsSmp2 = uIndex == 0 ? FfxFloat32x2(fTexCoords3,  fUvSample.y) : FfxFloat32x2(fUvSample.x, fTexCoords3);

    UPSAMPLE_F   fFactor        =  UPSAMPLE_F(fSmpFrac * (fSmpFrac - 1.0f));
    UPSAMPLE_F   fWeight12      = -fFactor  + UPSAMPLE_F(1.0f);
    UPSAMPLE_F   fWeight3       =  fFactor  * UPSAMPLE_F(fSmpFrac);
    UPSAMPLE_F   fWeight0       =  UPSAMPLE_F(1.0f) - fWeight12 - fWeight3;

    UPSAMPLE_F4  fHistorySample;
#if FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT == 0
    fHistorySample  = SampleHistory(fTexCoordsSmp0) * fWeight0;
    fHistorySample += SampleHistory(fUvSample)      * fWeight12;
    fHistorySample += SampleHistory(fTexCoordsSmp2) * fWeight3;
    fHistorySample  = ffxMax(UPSAMPLE_F4_BROADCAST(0.f), fHistorySample);
#else
    fHistorySample.rgb  = SampleHistoryColor(fTexCoordsSmp0) * fWeight0;
    fHistorySample.rgb += SampleHistoryColor(fUvSample)      * fWeight12;
    fHistorySample.rgb += SampleHistoryColor(fTexCoordsSmp2) * fWeight3;
    fHistorySample.rgb  = ffxMax(UPSAMPLE_F3_BROADCAST(0.f), fHistorySample.rgb);
    fHistorySample.w    = SampleHistoryWeight(fUvSample);
#endif

    return fHistorySample;
}

FFX_MIN16_F4 WrapLockStatus(FfxInt32x2 iPxSample)
{
    return FFX_MIN16_F4(LoadLockStatus(FFX_MIN16_I2(iPxSample)), 0);
}

#if 1
DeclareCustomFetchBilinearSamples(FetchLockStatusSamples, WrapLockStatus)
DeclareCustomTextureSample(LockStatusSample, Bilinear, FetchLockStatusSamples)
#else
DeclareCustomFetchBicubicSamplesMin16(FetchLockStatusSamples, WrapLockStatus)
DeclareCustomTextureSample(LockStatusSample, Lanczos2, FetchLockStatusSamples)
#endif



FfxFloat32x2 GetMotionVector(FFX_MIN16_I2 iPxHrPos, FFX_MIN16_I2 iPxLrPos)
{
#if FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS
    FfxFloat32x2 fDilatedMotionVector = LoadDilatedMotionVector(iPxLrPos);
#else
    FfxFloat32x2 fDilatedMotionVector = LoadInputMotionVector(iPxHrPos);
#endif

    return fDilatedMotionVector;
}

void ComputeReprojectedUVs(FfxInt32x2 iPxHrPos, FfxFloat32x2 fMotionVector, FFX_PARAMETER_OUT FfxFloat32x2 fReprojectedHrUv, FFX_PARAMETER_OUT FfxBoolean bIsExistingSample)
{
    FfxFloat32x2 fHrUv = (iPxHrPos + 0.5f) / DisplaySize();
    fReprojectedHrUv = fHrUv + fMotionVector;

    bIsExistingSample = (fReprojectedHrUv.x >= 0.0f && fReprojectedHrUv.x <= 1.0f) &&
        (fReprojectedHrUv.y >= 0.0f && fReprojectedHrUv.y <= 1.0f);
}

void ReprojectHistoryColor(FfxFloat32x2 fReprojectedHrUv, FFX_PARAMETER_OUT UPSAMPLE_F4 fHistoryColorAndWeight)
{
#if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION
    fHistoryColorAndWeight = HistorySample3Taps(fReprojectedHrUv, DisplaySize(), FrameIndex() & 1);
#else
    fHistoryColorAndWeight = UPSAMPLE_F4(HistorySample(fReprojectedHrUv, DisplaySize()));
#endif

#if FFX_FSR2_CUSTOM_OPTION_COMPACT_HISTORY_WEIGHT
    fHistoryColorAndWeight.w *= fHistoryColorAndWeight.w * UPSAMPLE_F(16.0f);
#endif

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE == 0
    fHistoryColorAndWeight.rgb *= Exposure();
#endif

#if FFX_FSR2_OPTION_HDR_COLOR_INPUT && FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING == 0
    fHistoryColorAndWeight.rgb = Tonemap(fHistoryColorAndWeight.rgb);
#endif

    fHistoryColorAndWeight.rgb = RGBToYCoCg(fHistoryColorAndWeight.rgb);
}

void ReprojectHistoryLockStatus(FFX_MIN16_I2 iPxHrPos, FfxFloat32x2 fReprojectedHrUv, FFX_PARAMETER_OUT LOCK_STATUS_T fReprojectedLockStatus)
{
    fReprojectedLockStatus = SampleLockStatus(fReprojectedHrUv);

    #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS == 0
    // If function is called from Accumulate pass, we need to treat locks differently
    LOCK_STATUS_F1 fInPlaceLockLifetime = LoadRwLockStatus(iPxHrPos)[LOCK_LIFETIME_REMAINING];

    // Keep lifetime if new lock
    if (fInPlaceLockLifetime < 0.0f) {
        fReprojectedLockStatus[LOCK_LIFETIME_REMAINING] = fInPlaceLockLifetime;
    }
    #else
    FFX_MIN16_F fEdgeStatus = LoadEdgeStatus(iPxHrPos);

    // Keep lifetime if new lock
    if (fEdgeStatus > FFX_MIN16_F(0)) {
        fReprojectedLockStatus[LOCK_LIFETIME_REMAINING] = LOCK_STATUS_F1(-LockInitialLifetime()) * LOCK_STATUS_F1(fEdgeStatus) * LOCK_STATUS_F1(2.0f);
    }
    #endif
}
#endif //!defined( FFX_FSR2_REPROJECT_H )
