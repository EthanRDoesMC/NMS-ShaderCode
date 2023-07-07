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

FfxFloat32 GetLuma(FFX_MIN16_I2 pos)
{
    //add some bias to avoid locking dark areas
    return FfxFloat32(LoadPreparedInputColorLuma(pos));
}

FfxFloat32x4 GatherLuma(FfxFloat32x2 fUV)
{
    //add some bias to avoid locking dark areas
    return FfxFloat32x4(GatherPreparedInputColorLuma(fUV));
}

FfxFloat32x4 GatherOffsetLuma(FfxFloat32x2 fUV, FFX_MIN16_I2 iOffset)
{
    //add some bias to avoid locking dark areas
    return FfxFloat32x4(GatherOffsetPreparedInputColorLuma(fUV, iOffset));
}

FfxFloat32 ComputeThinFeatureConfidence(FFX_MIN16_I2 pos)
{
    const FfxInt32 RADIUS = 1;

    FfxFloat32 fNucleus = GetLuma(pos);

    FfxFloat32 similar_threshold = 1.05f;
    FfxFloat32 dissimilarLumaMin = FSR2_FLT_MAX;
    FfxFloat32 dissimilarLumaMax = 0;

    /*
     0 1 2
     3 4 5
     6 7 8
    */

    #define SETBIT(x) (1U << x)

    FfxUInt32 mask = SETBIT(4); //flag fNucleus as similar

    const FfxUInt32 rejectionMasks[4] = {
        SETBIT(0) | SETBIT(1) | SETBIT(3) | SETBIT(4), //Upper left
        SETBIT(1) | SETBIT(2) | SETBIT(4) | SETBIT(5), //Upper right
        SETBIT(3) | SETBIT(4) | SETBIT(6) | SETBIT(7), //Lower left
        SETBIT(4) | SETBIT(5) | SETBIT(7) | SETBIT(8), //Lower right
    };

    FfxInt32 idx = 0;
    FFX_UNROLL
    for (FfxInt32 y = -RADIUS; y <= RADIUS; y++) {
        FFX_UNROLL
        for (FfxInt32 x = -RADIUS; x <= RADIUS; x++, idx++) {
            if (x == 0 && y == 0) continue;

            FFX_MIN16_I2 samplePos = ClampLoad(pos, FFX_MIN16_I2(x, y), FFX_MIN16_I2(RenderSize()));

            FfxFloat32 sampleLuma = GetLuma(samplePos);
            FfxFloat32 difference = ffxMax(sampleLuma, fNucleus) / ffxMin(sampleLuma, fNucleus);

            if (difference > 0 && (difference < similar_threshold)) {
                mask |= SETBIT(idx);
            } else {
                dissimilarLumaMin = ffxMin(dissimilarLumaMin, sampleLuma);
                dissimilarLumaMax = ffxMax(dissimilarLumaMax, sampleLuma);
            }
        }
    }

    FfxBoolean isRidge = fNucleus > dissimilarLumaMax || fNucleus < dissimilarLumaMin;

    if (FFX_FALSE == isRidge) {

        return 0;
    }

    FFX_UNROLL
    for (FfxInt32 i = 0; i < 4; i++) {

        if ((mask & rejectionMasks[i]) == rejectionMasks[i]) {
            return 0;
        }
    }

    return 1;
}

FfxBoolean Ridge(FfxFloat32 fNucleus, FfxFloat32 dissimilarLumaMin, FfxFloat32 dissimilarLumaMax)
{
    return fNucleus > dissimilarLumaMax || fNucleus < dissimilarLumaMin;
}

FfxFloat32 ComputeThinFeatureConfidenceLightweight(FFX_MIN16_I2 iPos)
{
    const FfxFloat32 similar_threshold = 1.05f;

    FFX_MIN16_U uSimilarityMask;
    FfxFloat32 dissimilarLumaMin = FSR2_FLT_MAX;
    FfxFloat32 dissimilarLumaMax = 0;

    FfxFloat32 fNucleus;
    FfxFloat32x2 fUv = FfxFloat32x2(iPos) / RenderSize();
    {
        FfxFloat32x4 fNeightbours = GatherLuma(fUv).xwzy;
        fNucleus = fNeightbours[3];

        uSimilarityMask  = 0;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[0], fNucleus) < similar_threshold * ffxMin(fNeightbours[0], fNucleus)) << 0;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[1], fNucleus) < similar_threshold * ffxMin(fNeightbours[1], fNucleus)) << 1;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[2], fNucleus) < similar_threshold * ffxMin(fNeightbours[2], fNucleus)) << 2;

        if (uSimilarityMask == 0x7) {
            return 0.0f;
        }
        if((uSimilarityMask & 0x1) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[0]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[0]);
        }
        if((uSimilarityMask & 0x2) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[1]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[1]);
        }
        if((uSimilarityMask & 0x4) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[2]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[2]);
        }

        if (!Ridge(fNucleus, dissimilarLumaMin, dissimilarLumaMax)) {
            return 0.0f;
        }
    }

    {
        FfxFloat32x2 fNeightbours = GatherOffsetLuma(fUv, FFX_MIN16_I2(1, 0)).zy;

        uSimilarityMask  = uSimilarityMask >> 2;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[0], fNucleus) < similar_threshold * ffxMin(fNeightbours[0], fNucleus)) << 1;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[1], fNucleus) < similar_threshold * ffxMin(fNeightbours[1], fNucleus)) << 2;

        if (uSimilarityMask == 0x7) {
            return 0.0f;
        }
        if ((uSimilarityMask & 0x2) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[0]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[0]);
        }
        if ((uSimilarityMask & 0x4) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[1]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[1]);
        }

        if (!Ridge(fNucleus, dissimilarLumaMin, dissimilarLumaMax)) {
            return 0.0f;
        }
    }

    {
        FfxFloat32x2 fNeightbours = GatherOffsetLuma(fUv, FFX_MIN16_I2(1, 1)).yx;

        uSimilarityMask  = uSimilarityMask >> 2;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[0], fNucleus) < similar_threshold * ffxMin(fNeightbours[0], fNucleus)) << 1;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[1], fNucleus) < similar_threshold * ffxMin(fNeightbours[1], fNucleus)) << 2;

        if (uSimilarityMask == 0x7) {
            return 0.0f;
        }
        if ((uSimilarityMask & 0x2) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[0]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[0]);
        }
        if ((uSimilarityMask & 0x4) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[1]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[1]);
        }

        if (!Ridge(fNucleus, dissimilarLumaMin, dissimilarLumaMax)) {
            return 0.0f;
        }
    }

    {
        FfxFloat32x2 fNeightbours = GatherOffsetLuma(fUv, FFX_MIN16_I2(0, 1)).xw;

        uSimilarityMask  = uSimilarityMask >> 2;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[0], fNucleus) < similar_threshold * ffxMin(fNeightbours[0], fNucleus)) << 1;
        uSimilarityMask |= FFX_MIN16_U(ffxMax(fNeightbours[1], fNucleus) < similar_threshold * ffxMin(fNeightbours[1], fNucleus)) << 2;

        if (uSimilarityMask == 0x7) {
            return 0.0f;
        }
        if ((uSimilarityMask & 0x2) == 0) {
            dissimilarLumaMin = ffxMin(dissimilarLumaMin, fNeightbours[0]);
            dissimilarLumaMax = ffxMax(dissimilarLumaMax, fNeightbours[0]);
        }

        if (!Ridge(fNucleus, dissimilarLumaMin, dissimilarLumaMax)) {
            return 0.0f;
        }
    }

    return 1.0f;
}

void ComputeLockStatus(FFX_MIN16_I2 iPxLrPos)
{
    #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS == 0
    FfxFloat32 fConfidenceOfThinFeature = ComputeThinFeatureConfidence(iPxLrPos);
    #else
    FfxFloat32 fConfidenceOfThinFeature = ComputeThinFeatureConfidenceLightweight(iPxLrPos);
    #endif

    if (fConfidenceOfThinFeature > 0.0f)
    {
        FfxFloat32x2 fSrcJitteredPos = FfxFloat32x2(iPxLrPos) + 0.5f - Jitter();
        FfxFloat32x2 fLrPosInHr = fSrcJitteredPos / DownscaleFactor();
        FFX_MIN16_I2 iPxHrPos = FFX_MIN16_I2(fLrPosInHr);

        #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS == 0
        LOCK_STATUS_T fLockStatus = LoadLockStatus(iPxHrPos);
        fLockStatus[LOCK_LIFETIME_REMAINING] = (fLockStatus[LOCK_LIFETIME_REMAINING] == LOCK_STATUS_F1(0.0f)) ? LOCK_STATUS_F1(-LockInitialLifetime()) : LOCK_STATUS_F1(-(LockInitialLifetime() * 2));
        StoreLockStatus(iPxHrPos, fLockStatus);
        #else
        LOCK_STATUS_T fLockStatus = LoadRwLockStatus(iPxHrPos);
        FfxFloat32 fEdgeStatus = (fLockStatus[LOCK_LIFETIME_REMAINING] == LOCK_STATUS_F1(0.0f)) ? 0.5f : 1.0f;
        StoreLockStatus(iPxHrPos, fLockStatus, fEdgeStatus);
        #endif
    }
}

void ComputeLock(FFX_MIN16_I2 iPxLrPos)
{
    ComputeLockStatus(iPxLrPos);
}