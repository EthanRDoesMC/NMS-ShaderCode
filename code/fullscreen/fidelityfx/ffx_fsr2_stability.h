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

void CompareAndSwap(FFX_PARAMETER_INOUT FfxFloat32 fA, FFX_PARAMETER_INOUT FfxFloat32 fB)
{
    if (fA > fB) {
        FfxFloat32 fTemp = fA;
        fA = fB;
        fB = fTemp;
    }
}

FfxFloat32 ComputeLumaDifferenceFactor(FfxFloat32 fLumaA, FfxFloat32 fLumaB)
{
    FfxFloat32   fLumaDiffMin = 0.01f;
    FfxFloat32   fLumaDiffMax = 0.25f;

    return (abs(fLumaA - fLumaB) - fLumaDiffMin) / fLumaDiffMax;
}

void ComputeLumaStabilityFactor(FFX_MIN16_I2 iPxLrPos, FfxFloat32 fCurrentFrameLuma, FFX_PARAMETER_INOUT FfxFloat32x4 fLumaHistory)
{
    FfxFloat32 fStability = 0.0f;

    if (FrameIndex() > 3) {
        FfxFloat32 fDiffs0 = MinDividedByMax(fLumaHistory[2], fCurrentFrameLuma);
        FfxFloat32 fDiffs1 = ffxMax(MinDividedByMax(fLumaHistory[0], fCurrentFrameLuma), MinDividedByMax(fLumaHistory[1], fCurrentFrameLuma));

        fStability = ffxSaturate(fDiffs1 - fDiffs0);
    }

    //move history
    fLumaHistory[0] = fLumaHistory[1];
    fLumaHistory[1] = fLumaHistory[2];
    fLumaHistory[2] = fCurrentFrameLuma;
    fLumaHistory[3] = fStability;
}

void ComputeLumaStabilityFactorTest(FFX_MIN16_I2 iPxLrPos, FfxFloat32 fLumaVec[5], FFX_PARAMETER_INOUT FfxFloat32x4 fLumaHistory)
{
    FfxFloat32 fLumaCurr = fLumaVec[0];

    if (FrameIndex() > 3) {

        FfxFloat32  fMoment0    = 0.0f;
        FfxFloat32  fMoment1    = 0.0f;
        FfxFloat32  fLumaDiff   = 1.0f;

        FFX_UNROLL
        for (FfxInt32 iLuma = 0; iLuma < 5; ++iLuma)
        {
            FfxFloat32 fLumaCurrDiff = ComputeLumaDifferenceFactor(fLumaVec[iLuma], fLumaHistory[2]);
            if (fLumaCurrDiff < fLumaDiff)
            {
                fLumaDiff   = fLumaCurrDiff;
                fLumaCurr   = fLumaVec[iLuma];
            }
            fMoment0 += fLumaVec[iLuma];
            fMoment1 += fLumaVec[iLuma] * fLumaVec[iLuma];
        }

        fLumaDiff = ffxMax(fLumaDiff, ComputeLumaDifferenceFactor(fLumaCurr, fLumaHistory[1]));
        fLumaDiff = ffxMax(fLumaDiff, ComputeLumaDifferenceFactor(fLumaCurr, fLumaHistory[0]));

        fMoment0 /= 5.0f;
        fMoment1 /= 5.0f;

        FfxFloat32 fStd;
        fStd = ffxApproximateSqrt(ffxMax(0.0f, fMoment1 - fMoment0 * fMoment0));

        FfxFloat32 fStabilityCurr;
        FfxFloat32 fStabilityPrev;

        fStabilityCurr = ffxSaturate(1.0f - fLumaDiff / (fStd * 32.0f + 0.5f));
        fStabilityCurr = ffxPow(fStabilityCurr, 2.0f);
        fStabilityPrev = fLumaHistory.a;
        fStabilityPrev = ffxMin( fStabilityPrev, fStabilityCurr);
        fLumaHistory.a = ffxLerp(fStabilityPrev, fStabilityCurr, 0.25f) * 0.95f;
    }
    else
    {
        fLumaHistory.a = FfxFloat32(0);
    }

    //move history
    fLumaHistory[0] = fLumaHistory[1];
    fLumaHistory[1] = fLumaHistory[2];
    fLumaHistory[2] = fLumaCurr;
}

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
void Stability(FFX_MIN16_I2 iPxLrPos, FFX_PARAMETER_OUT FfxFloat32x4 fOutCurrentFrameLumaHistory)
#else
void Stability(FFX_MIN16_I2 iPxLrPos)
#endif
{
    FfxFloat32x2 fHistoryUv     = (FfxFloat32x2(iPxLrPos) + 0.5f) / RenderSize();
    FfxFloat32x2 fMotion        = LoadDilatedMotionVector(iPxLrPos);
    FfxFloat32x4 fLumaHistory   = SampleLumaHistory(fHistoryUv + fMotion);
    FFX_MIN16_I2 iQuadId        = FFX_MIN16_I2(iPxLrPos.x & 0x1, iPxLrPos.y & 0x1);
    FfxFloat32x3 fLumaQuad0     = GatherPreparedInputColorLuma(FfxFloat32x2(iPxLrPos) / RenderSize()).xyz;
    FfxFloat32x2 fLumaQuad1     = GatherOffsetPreparedInputColorLuma(FfxFloat32x2(iPxLrPos) / RenderSize(), FFX_MIN16_I2(1, 1)).xz;
    FfxFloat32   fLumaVec[5];

    FFX_UNROLL
    for (FfxInt32 iLuma = 0; iLuma < 3; ++iLuma)
    {
        fLumaVec[iLuma]     = ffxPow(fLumaQuad0[iLuma], 4.0f);
    }
    FFX_UNROLL
    for (FfxInt32 iLuma = 0; iLuma < 2; ++iLuma)
    {
        fLumaVec[iLuma + 3] = ffxPow(fLumaQuad1[iLuma], 4.0f);
    }

    ComputeLumaStabilityFactorTest(iPxLrPos, fLumaVec, fLumaHistory);

    #if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT == 0
    StoreLumaHistory(iPxLrPos, fLumaHistory);
    #else
    fOutCurrentFrameLumaHistory = fLumaHistory;
    #endif
}
