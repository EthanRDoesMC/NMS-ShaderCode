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

FfxFloat32 GetPxHrVelocity(FfxFloat32x2 fMotionVector)
{
    return ffxApproximateSqrt(ffxDot2(fMotionVector * DisplaySize(), fMotionVector * DisplaySize()));
}

void Accumulate(FFX_MIN16_I2 iPxHrPos, FFX_PARAMETER_INOUT UPSAMPLE_F4 fHistory, FFX_PARAMETER_IN UPSAMPLE_F4 fUpsampled, FFX_PARAMETER_IN UPSAMPLE_F fMaxAverageWeight)
{
    fHistory.w = fHistory.w + fUpsampled.w;

    fUpsampled.rgb = YCoCgToRGB(fUpsampled.rgb);

    const UPSAMPLE_F fAlpha = fUpsampled.w / fHistory.w;
    fHistory.rgb = ffxLerp(fHistory.rgb, fUpsampled.rgb, fAlpha);
    fHistory.w = ffxMin(fHistory.w, fMaxAverageWeight);
}

void RectifyHistory(
    RectificationBoxData clippingBox,
    FFX_PARAMETER_INOUT UPSAMPLE_F4 fHistory,
    FFX_PARAMETER_IN UPSAMPLE_F fDepthClipFactor,
    FFX_PARAMETER_IN UPSAMPLE_F fLumaStabilityFactor,
    FFX_PARAMETER_IN UPSAMPLE_F fLockContributionThisFrame)
{
    UPSAMPLE_F fScaleFactorInfluence = UPSAMPLE_F(1.0f / DownscaleFactor().x - 1);
    UPSAMPLE_F fBoxScale = UPSAMPLE_F(1.0f) + (UPSAMPLE_F(0.5f) * fScaleFactorInfluence);

    UPSAMPLE_F3 fScaledBoxVec = clippingBox.boxVec * fBoxScale;
    UPSAMPLE_F3 boxMin = clippingBox.boxCenter - fScaledBoxVec;
    UPSAMPLE_F3 boxMax = clippingBox.boxCenter + fScaledBoxVec;
    UPSAMPLE_F3 boxCenter = clippingBox.boxCenter;
    UPSAMPLE_F boxVecSize = length(clippingBox.boxVec);

    boxMin = ffxMax(clippingBox.aabbMin, boxMin);
    boxMax = ffxMin(clippingBox.aabbMax, boxMax);

    UPSAMPLE_F3 distToClampOutside = UPSAMPLE_F3(ffxMax(ffxMax(UPSAMPLE_F3_BROADCAST(0.0f), boxMin - UPSAMPLE_F3(fHistory.xyz)), ffxMax(UPSAMPLE_F3_BROADCAST(0.0f), UPSAMPLE_F3(fHistory.xyz) - boxMax)));

    if (any(FFX_GREATER_THAN(distToClampOutside, UPSAMPLE_F3_BROADCAST(0.0f)))) {

        const UPSAMPLE_F3 clampedHistorySample = clamp(UPSAMPLE_F3(fHistory.xyz), boxMin, boxMax);

        UPSAMPLE_F3 clippedHistoryToBoxCenter = abs(clampedHistorySample - boxCenter);
        UPSAMPLE_F3 historyToBoxCenter = abs(UPSAMPLE_F3(fHistory.xyz) - boxCenter);
        UPSAMPLE_F3 HistoryColorWeight;
        HistoryColorWeight.x = historyToBoxCenter.x > UPSAMPLE_F(0) ? clippedHistoryToBoxCenter.x / historyToBoxCenter.x : UPSAMPLE_F(0.0f);
        HistoryColorWeight.y = historyToBoxCenter.y > UPSAMPLE_F(0) ? clippedHistoryToBoxCenter.y / historyToBoxCenter.y : UPSAMPLE_F(0.0f);
        HistoryColorWeight.z = historyToBoxCenter.z > UPSAMPLE_F(0) ? clippedHistoryToBoxCenter.z / historyToBoxCenter.z : UPSAMPLE_F(0.0f);

        UPSAMPLE_F3 fHistoryContribution = HistoryColorWeight;

        // only lock luma
        fHistoryContribution += UPSAMPLE_F3_BROADCAST(ffxMax(UPSAMPLE_F(fLockContributionThisFrame), fLumaStabilityFactor));
        fHistoryContribution *= (fDepthClipFactor * fDepthClipFactor);

        fHistory.xyz = UPSAMPLE_F3(ffxLerp(clampedHistorySample.xyz, fHistory.xyz, ffxSaturate(fHistoryContribution)));
    }
}

void RectifyHistoryLightweight(
    RectificationBoxDataLightweight clippingBox,
    FFX_PARAMETER_INOUT UPSAMPLE_F4 fHistory,
    FFX_PARAMETER_IN UPSAMPLE_F fDepthClipFactor,
    FFX_PARAMETER_IN UPSAMPLE_F fLumaStabilityFactor,
    FFX_PARAMETER_IN UPSAMPLE_F fLockContributionThisFrame)
{
    UPSAMPLE_F3 boxCenter  = clippingBox.aabbCenter;
    UPSAMPLE_F3 boxExtent  = clippingBox.aabbExtent;

    UPSAMPLE_F3 historyToBoxCenter = UPSAMPLE_F3(fHistory.xyz) - boxCenter;

    if (any(FFX_GREATER_THAN(abs(historyToBoxCenter), boxExtent))) {

        UPSAMPLE_F3 clampedHistoryToBoxCenter = clamp(historyToBoxCenter, - boxExtent, + boxExtent);
        UPSAMPLE_F3 clampedHistorySample      = clampedHistoryToBoxCenter + boxCenter;
        UPSAMPLE_F3 fHistoryContribution      = clampedHistoryToBoxCenter / historyToBoxCenter;

        // only lock luma
        fHistoryContribution += UPSAMPLE_F3_BROADCAST(ffxMax(UPSAMPLE_F(fLockContributionThisFrame), fLumaStabilityFactor));
        fHistoryContribution *= fDepthClipFactor * fDepthClipFactor;

        fHistory.xyz = UPSAMPLE_F3(ffxLerp(clampedHistorySample.xyz, fHistory.xyz, ffxSaturate(fHistoryContribution)));
    }
}

void WriteUpscaledOutput(FFX_MIN16_I2 iPxHrPos, FfxFloat32x3 fUpscaledColor)
{
    StoreUpscaledOutput(iPxHrPos, fUpscaledColor);
}

FFX_MIN16_F GetLumaStabilityFactor(FfxFloat32x2 fHrUv)
{
    FFX_MIN16_F fLumaStabilityFactor = FFX_MIN16_F(SampleLumaStabilityFactor(fHrUv));

    return fLumaStabilityFactor;
}

FFX_MIN16_F GetLockContributionThisFrame(FfxFloat32x2 fUvCoord, FFX_MIN16_F fAccumulationMask, FFX_MIN16_F fParticleMask, LOCK_STATUS_T fLockStatus)
{
    const UPSAMPLE_F fNormalizedLockLifetime = GetNormalizedRemainingLockLifetime(fLockStatus);

    // Rectify on lock frame
    FFX_MIN16_F fLockContributionThisFrame = ffxSaturate(fNormalizedLockLifetime * UPSAMPLE_F(4));

    fLockContributionThisFrame *= FFX_MIN16_F(1.0f) - fParticleMask;
    //Take down contribution in transparent areas
    fLockContributionThisFrame *= FFX_MIN16_F(fAccumulationMask.r > 0.1f);

    return fLockContributionThisFrame;
}

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
void FinalizeLockStatus(FFX_MIN16_I2 iPxHrPos, FFX_PARAMETER_INOUT LOCK_STATUS_T fLockStatus, FfxFloat32 fUpsampledWeight)
#else
void FinalizeLockStatus(FFX_MIN16_I2 iPxHrPos, LOCK_STATUS_T fLockStatus, FfxFloat32 fUpsampledWeight)
#endif
{
    // Increase trust
    const UPSAMPLE_F fTrustIncreaseLanczosMax = UPSAMPLE_F(accumulationMax);
    const UPSAMPLE_F fTrustIncrease = UPSAMPLE_F(fUpsampledWeight / fTrustIncreaseLanczosMax);
    fLockStatus[LOCK_TRUST] = ffxMin(LOCK_STATUS_F1(1), fLockStatus[LOCK_TRUST] + LOCK_STATUS_F1(fTrustIncrease));

    // Decrease lock lifetime
    const UPSAMPLE_F fLifetimeDecreaseLanczosMax = UPSAMPLE_F(JitterSequenceLength()) * UPSAMPLE_F(averageLanczosWeightPerFrame);
    const UPSAMPLE_F fLifetimeDecrease = UPSAMPLE_F(fUpsampledWeight / fLifetimeDecreaseLanczosMax);
    fLockStatus[LOCK_LIFETIME_REMAINING] = ffxMax(LOCK_STATUS_F1(0), fLockStatus[LOCK_LIFETIME_REMAINING] - LOCK_STATUS_F1(fLifetimeDecrease));


    #if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT == 0
    StoreLockStatus(iPxHrPos, fLockStatus);
    #else
    fLockStatus[LOCK_LIFETIME_REMAINING] = EncodeLockLifetime(fLockStatus[LOCK_LIFETIME_REMAINING]);
    #endif
}

UPSAMPLE_F ComputeMaxAccumulationWeight(UPSAMPLE_F fMaxAverageWeight, UPSAMPLE_F fReactiveMax, UPSAMPLE_F fDepthClipFactor, UPSAMPLE_F fLuminanceDiff, LockState lockState) {


    UPSAMPLE_F normalizedMinimum = UPSAMPLE_F(accumulationMaxOnMotion) / UPSAMPLE_F(accumulationMax);

    UPSAMPLE_F fReactiveMaxAccumulationWeight = UPSAMPLE_F(1) - fReactiveMax;
    UPSAMPLE_F fMotionMaxAccumulationWeight = fMaxAverageWeight / UPSAMPLE_F(accumulationMax);
    UPSAMPLE_F fDepthClipMaxAccumulationWeight = fDepthClipFactor;

    UPSAMPLE_F fLuminanceDiffMaxAccumulationWeight = ffxSaturate(ffxMax(normalizedMinimum, UPSAMPLE_F(1) - fLuminanceDiff));

    UPSAMPLE_F maxAccumulation = UPSAMPLE_F(accumulationMax) * ffxMin(
        ffxMin(fReactiveMaxAccumulationWeight, fMotionMaxAccumulationWeight),
        ffxMin(fDepthClipMaxAccumulationWeight, fLuminanceDiffMaxAccumulationWeight)
    );

    return (lockState.NewLock && !lockState.WasLockedPrevFrame) ? UPSAMPLE_F(accumulationMaxOnMotion) : maxAccumulation;
}

UPSAMPLE_F2 ComputeKernelWeight(UPSAMPLE_F fHistoryWeight, UPSAMPLE_F fDepthClipFactor, UPSAMPLE_F fReactivityFactor) {

    UPSAMPLE_F fOneMinusReactiveMax = UPSAMPLE_F(1) - fReactivityFactor;

    UPSAMPLE_F fKernelSizeBias = ffxSaturate((fHistoryWeight - UPSAMPLE_F(0.5)) / UPSAMPLE_F(3));
    UPSAMPLE_F2 fKernelWeight = UPSAMPLE_F(1) + (UPSAMPLE_F(1.0f) / UPSAMPLE_F2(DownscaleFactor()) - UPSAMPLE_F(1)) * UPSAMPLE_F(fKernelSizeBias) * fOneMinusReactiveMax;
    //average value on disocclusion, to help decrease high value sample importance wait for accumulation to kick in
    fKernelWeight *= FFX_BROADCAST_MIN_FLOAT16X2(UPSAMPLE_F(0.5) + fDepthClipFactor * UPSAMPLE_F(0.5));

    return ffxMin(FFX_BROADCAST_MIN_FLOAT16X2(1.99), fKernelWeight);
}

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
void Accumulate(FfxFloat32x2 fPxHrPos, FFX_PARAMETER_OUT FfxFloat32x3 fOutput, FFX_PARAMETER_OUT FfxFloat32x4 fOutputInternal, FFX_PARAMETER_OUT FfxFloat32x3 fOutLockStatus)
#else
void Accumulate(FFX_MIN16_I2 iPxHrPos)
#endif
{
#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
    const FFX_MIN16_I2 iPxHrPos = FFX_MIN16_I2(fPxHrPos);
#else
    const FfxFloat32x2 fPxHrPos = FfxFloat32x2(iPxHrPos) + 0.5f;
#endif
    const FfxFloat32x2 fPxLrPos = fPxHrPos * DownscaleFactor(); // Source resolution output pixel center position
    const FfxFloat32x2 fPxLrPosJittered = fPxLrPos + Jitter();
    const FFX_MIN16_I2 iPxLrPos = FFX_MIN16_I2(fPxLrPosJittered);           // TODO: what about weird upscale factors...

    const FfxFloat32x2 fMotionVector = GetMotionVector(iPxHrPos, iPxLrPos);

    UPSAMPLE_F4 fHistoryColorAndWeight = UPSAMPLE_F4(0.0f, 0.0f, 0.0f, 0.0f);
    LOCK_STATUS_T fLockStatus = CreateNewLockSample();
    FfxBoolean bIsExistingSample = FFX_TRUE;

    FfxFloat32x2 fReprojectedHrUv = FfxFloat32x2(0, 0);
    ComputeReprojectedUVs(iPxHrPos, fMotionVector, fReprojectedHrUv, bIsExistingSample);

    if (bIsExistingSample) {
        ReprojectHistoryColor(fReprojectedHrUv, fHistoryColorAndWeight);
        ReprojectHistoryLockStatus(iPxHrPos, fReprojectedHrUv, fLockStatus);
    }

    const FfxFloat32x2 fLrUvJittered = fPxLrPosJittered / RenderSize();
    const FFX_MIN16_F fDepthClipFactor = FFX_MIN16_F(SampleDepthClip(fLrUvJittered));

    FFX_MIN16_F fLuminanceDiff = FFX_MIN16_F(0.0f);
    LockState lockState = PostProcessLockStatus(fLrUvJittered, FFX_MIN16_F(fDepthClipFactor), fLockStatus, fLuminanceDiff);

    const FfxFloat32x2 fHrUv = fPxHrPos / DisplaySize();
    const FFX_MIN16_F fLumaStabilityFactor = FFX_MIN16_F(GetLumaStabilityFactor(fHrUv));
    #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION == 0
    const FFX_MIN16_F2 fDilatedReactiveMasks = FFX_MIN16_F2(SampleDilatedReactiveMasks(fLrUvJittered));
    #else
    const FFX_MIN16_F2 fDilatedReactiveMasks = FFX_MIN16_F2(SampleReactiveMask(fLrUvJittered), 0.0f);
    #endif
    const FFX_MIN16_F fReactiveMax = fDilatedReactiveMasks.x;
    #if FFX_FSR2_CUSTOM_OPTION_NO_ACCUMULATION_MASK == 0
    const FFX_MIN16_F fAccumulationMask = fDilatedReactiveMasks.y;
    #else
    const FFX_MIN16_F fAccumulationMask = FFX_MIN16_F(1.0f);
    #endif

    // No trust in reactive areas
    fLockStatus[LOCK_TRUST] = ffxMin(fLockStatus[LOCK_TRUST], LOCK_STATUS_F1(1.0f) - LOCK_STATUS_F1(pow(fReactiveMax, 1.0f / 3.0f)));
    fLockStatus[LOCK_TRUST] = ffxMin(fLockStatus[LOCK_TRUST], LOCK_STATUS_F1(fDepthClipFactor));

    const FFX_MIN16_F fMaxAverageWeight = ffxLerp(FFX_MIN16_F(accumulationMax), FFX_MIN16_F(accumulationMaxOnMotion), FFX_MIN16_F(ffxSaturate(GetPxHrVelocity(fMotionVector) * 10.0f)));

    // Kill accumulation based on shading change
    fHistoryColorAndWeight.w = ffxMin(fHistoryColorAndWeight.w, FFX_MIN16_F(accumulationMax * ffxPow(UPSAMPLE_F(1) - fLuminanceDiff, 2.0f)));
    fHistoryColorAndWeight.w = ffxMin(fHistoryColorAndWeight.w, ComputeMaxAccumulationWeight(fMaxAverageWeight, fReactiveMax, fDepthClipFactor, fLuminanceDiff, lockState));

    UPSAMPLE_F2 fKernelWeight = ComputeKernelWeight(UPSAMPLE_F(fHistoryColorAndWeight.w), UPSAMPLE_F(fDepthClipFactor), ffxMax((UPSAMPLE_F(1) - fLockStatus[LOCK_TRUST]), UPSAMPLE_F(fReactiveMax)));
#if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION == 0
    RectificationBoxData clippingBox;
    UPSAMPLE_F4 fUpsampledColorAndWeight = ComputeUpsampledColorAndWeight(iPxHrPos, fKernelWeight, clippingBox);
#else
    RectificationBoxDataLightweight clippingBox;
    UPSAMPLE_F4 fUpsampledColorAndWeight = ComputeUpsampledColorAndWeightLightweight(fPxLrPos, fKernelWeight, fMotionVector, clippingBox);
#endif

    FFX_MIN16_F fLockContributionThisFrame = GetLockContributionThisFrame(fHrUv, fAccumulationMask, fReactiveMax, fLockStatus);

    // Update accumulation and rectify history
    if (fHistoryColorAndWeight.w > 0.0f) {

    #if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION == 0
        RectifyHistory(clippingBox, fHistoryColorAndWeight, UPSAMPLE_F(fDepthClipFactor), UPSAMPLE_F(fLumaStabilityFactor), UPSAMPLE_F(fLockContributionThisFrame));
    #else
        RectifyHistoryLightweight(clippingBox, fHistoryColorAndWeight, UPSAMPLE_F(fDepthClipFactor), UPSAMPLE_F(fLumaStabilityFactor), UPSAMPLE_F(fLockContributionThisFrame));
    #endif

        fHistoryColorAndWeight.rgb = YCoCgToRGB(fHistoryColorAndWeight.rgb);
    }

    Accumulate(iPxHrPos, fHistoryColorAndWeight, fUpsampledColorAndWeight, fMaxAverageWeight);

    //Subtract accumulation weight in reactive areas
    fHistoryColorAndWeight.w -= fUpsampledColorAndWeight.w * UPSAMPLE_F(fReactiveMax);

#if FFX_FSR2_CUSTOM_OPTION_COMPACT_HISTORY_WEIGHT
    fHistoryColorAndWeight.w  = ffxSqrt(fHistoryColorAndWeight.w / UPSAMPLE_F(16.0f));
#endif

#if FFX_FSR2_CUSTOM_OPTION_SATURATE_OUTPUT
    // NOTE(sal): somehow we can end up with negative values here,
    // which when stored in an R11FG11FB10F target cause a bunch of nans and black corruption;
    // for now let's saturate here to guard for that;
    // we should look into this further to find the actual source of negative values
    fHistoryColorAndWeight.rgb = ffxSaturate(fHistoryColorAndWeight.rgb);
#endif
#if FFX_FSR2_OPTION_HDR_COLOR_INPUT && FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING == 0
    fHistoryColorAndWeight.rgb = InverseTonemap(fHistoryColorAndWeight.rgb);
#endif

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE == 0
    fHistoryColorAndWeight.rgb /= Exposure();
#endif

    FinalizeLockStatus(iPxHrPos, fLockStatus, fUpsampledColorAndWeight.w);

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT == 0
    StoreInternalColorAndWeight(iPxHrPos, fHistoryColorAndWeight);
#endif

    // Output final color when RCAS is disabled
#if FFX_FSR2_OPTION_APPLY_SHARPENING == 0 && FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT == 0
    WriteUpscaledOutput(iPxHrPos, fHistoryColorAndWeight.rgb);
#endif

#if FFX_FSR2_CUSTOM_OPTION_RUN_ON_FRAGMENT
    fOutputInternal = fHistoryColorAndWeight;
    fOutLockStatus  = fLockStatus;
    fOutput         = fHistoryColorAndWeight.rgb;
#endif
}
