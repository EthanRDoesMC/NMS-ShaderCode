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

void ComputeLumaStabilityFactorTest(FFX_MIN16_I2 iPxLrPos, FfxFloat32 fCurrentFrameLuma, FfxFloat32 fDeathClip, FFX_PARAMETER_INOUT FfxFloat32x4 fLumaHistory)
{
    if (FrameIndex() > 3) {
        FfxFloat32x4 fLumaSorted;
        fLumaSorted = FfxFloat32x4(fLumaHistory.xyz, fCurrentFrameLuma);

        CompareAndSwap(fLumaSorted[0], fLumaSorted[1]);
        CompareAndSwap(fLumaSorted[2], fLumaSorted[3]);
        CompareAndSwap(fLumaSorted[0], fLumaSorted[2]);
        CompareAndSwap(fLumaSorted[1], fLumaSorted[3]);
        CompareAndSwap(fLumaSorted[1], fLumaSorted[2]);

        FfxFloat32 fMedian = fLumaSorted[1] * 0.5f + fLumaSorted[2] * 0.5f;
        FfxFloat32 fDiff   = ffxMax(fMedian - fLumaSorted[0], fLumaSorted[3] - fMedian);

        FfxFloat32 fStabilityCurr;
        FfxFloat32 fStabilityPrev;
        fStabilityCurr = 1.0f - ffxSaturate(ffxPow(fDiff, 0.5f) * 4.0f);
        fStabilityCurr = fStabilityCurr * fDeathClip * fDeathClip;
        fStabilityPrev = fLumaHistory.a;
        fLumaHistory.a = ffxLerp(ffxMin(fStabilityCurr, fStabilityPrev), fStabilityCurr, 0.05f * fDeathClip);
    }
    else
    {
        fLumaHistory.a = FfxFloat32(0);
    }

    //move history
    fLumaHistory[0] = fLumaHistory[1];
    fLumaHistory[1] = fLumaHistory[2];
    fLumaHistory[2] = fCurrentFrameLuma;
}

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
void Stability(FFX_MIN16_I2 iPxLrPos, FFX_PARAMETER_OUT FfxFloat32x4 fOutCurrentFrameLumaHistory)
#else
void Stability(FFX_MIN16_I2 iPxLrPos)
#endif
{
#if 0
    FfxFloat32x2 fInputUv       = (FfxFloat32x2(iPxLrPos) + 0.5f + Jitter()) / RenderSize();
    FfxFloat32x2 fHistoryUv     = (FfxFloat32x2(iPxLrPos) + 0.5f) / RenderSize();
    FfxFloat32x2 fMotion        = LoadDilatedMotionVector(iPxLrPos);
    #if FFX_FSR2_CUSTOM_OPTION_LOCKS_LUMA_BIAS == 0
    FfxFloat32   fPerceivedLuma = ffxPow(SamplePreparedInputColorLuma(fInputUv), 6.0f);
    #else
    FfxFloat32   fPerceivedLuma = ffxPow(SamplePreparedInputColorLuma(fInputUv), 4.0f);
    #endif
    FfxFloat32x4 fLumaHistory   = SampleLumaHistory(fHistoryUv + fMotion);
    FfxFloat32   fDeathClip     = LoadDepthClip(iPxLrPos);

    ComputeLumaStabilityFactorTest(iPxLrPos, fPerceivedLuma, fDeathClip, fLumaHistory);
#else

    FfxFloat32   fPerceivedLuma = RGBToPerceivedLuma(LoadPreparedInputColor(iPxLrPos));
    FfxFloat32x4 fLumaHistory = LoadLumaHistory(iPxLrPos);

    ComputeLumaStabilityFactor(iPxLrPos, fPerceivedLuma, fLumaHistory);
#endif
    #if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT == 0
    StoreLumaHistory(iPxLrPos, fLumaHistory);
    #else
    fOutCurrentFrameLumaHistory = fLumaHistory;
    #endif
}
