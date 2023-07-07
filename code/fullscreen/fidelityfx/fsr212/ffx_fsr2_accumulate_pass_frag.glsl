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

// FSR2 pass 5
// SRV  4 : FSR2_Exposure                       : r_exposure
// SRV  6 : m_UpscaleTransparencyAndComposition : r_transparency_and_composition_mask
// SRV  8 : FSR2_DilatedVelocity                : r_dilated_motion_vectors
// SRV 10 : FSR2_InternalUpscaled2              : r_internal_upscaled_color
// SRV 11 : FSR2_LockStatus2                    : r_lock_status
// SRV 12 : FSR2_DepthClip                      : r_depth_clip
// SRV 13 : FSR2_PreparedInputColor             : r_prepared_input_color
// SRV 14 : FSR2_LumaHistory                    : r_luma_history
// SRV 16 : FSR2_LanczosLutData                 : r_lanczos_lut
// SRV 26 : FSR2_MaximumUpsampleBias            : r_upsample_maximum_bias_lut
// SRV 27 : FSR2_ReactiveMaskMax                : r_reactive_max
// SRV 28 : FSR2_ExposureMips                   : r_imgMips
// UAV 10 : FSR2_InternalUpscaled1              : rw_internal_upscaled_color
// UAV 11 : FSR2_LockStatus1                    : rw_lock_status
// UAV 18 : DisplayOutput                       : rw_upscaled_output
// CB   0 : cbFSR2
// CB   1 : FSR2DispatchOffsets

//#version 450

#ifndef D_PLATFORM_SWITCH
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require
#endif

#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                 0
#define FSR2_BIND_SRV_INTERNAL_UPSCALED                      1
#define FSR2_BIND_SRV_INTERNAL_UPSCALED_WEIGHT               2
#define FSR2_BIND_SRV_LOCK_STATUS                            3
#define FSR2_BIND_SRV_DEPTH_CLIP                             4
#define FSR2_BIND_SRV_PREPARED_INPUT_COLOR                   5
#define FSR2_BIND_SRV_LUMA_HISTORY                           6
#define FSR2_BIND_SRV_REACTIVE_MAX                           7
#define FSR2_BIND_SRV_EXPOSURE_MIPS                          8
#define FSR2_BIND_SRV_LANCZOS_LUT                            9
#define FSR2_BIND_SRV_LANCZOS_SUM_LUT                       10
#define FSR2_BIND_SRV_EXPOSURE                              11
#define FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK     12
#define FSR2_BIND_UAV_LOCK_STATUS                           13

#define FSR2_BIND_CB_FSR2                                   14

#if FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT == 0
#undef  FSR2_BIND_SRV_INTERNAL_UPSCALED_WEIGHT
#endif

#if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION == 0
#undef  FSR2_BIND_SRV_LANCZOS_SUM_LUT
#endif

#if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_ACCUMULATION
#undef  FSR2_BIND_SRV_REACTIVE_MAX
#define FSR2_BIND_SRV_REACTIVE_MASK                          7
#endif

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE
#undef  FSR2_BIND_SRV_EXPOSURE
#endif

#if FFX_FSR2_CUSTOM_OPTION_NO_ACCUMULATION_MASK
#undef  FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK
#endif

#if FFX_FSR2_OPTION_USE_LANCZOS_LUT == 0
#undef  FSR2_BIND_SRV_LANCZOS_LUT
#endif

#if FFX_FSR2_CUSTOM_OPTION_COMPACT_LOCKS
#undef  FSR2_BIND_UAV_LOCK_STATUS
#define FSR2_BIND_UAV_LOCK_STATUS_UNORM                     13
#endif

#if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS
#undef  FSR2_BIND_UAV_LOCK_STATUS
#undef  FSR2_BIND_UAV_LOCK_STATUS_UNORM
#endif

#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_callbacks_glsl.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_common.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_sample.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_upsample.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_postprocess_lock_status.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_reproject.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_accumulate.h"

#if FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOR_OUTPUT == 0
    #if FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT == 0
    layout(location = 0) out vec4  outInternalColor;
    layout(location = 1) out vec3  outLockStatus;
    #else
    layout(location = 0) out vec3  outInternalColor;
    layout(location = 1) out float outInternalWeight;
    layout(location = 2) out vec3  outLockStatus;
    #endif
#else
    #if FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT == 0
    layout(location = 0) out vec3  outColor;
    layout(location = 1) out vec4  outInternalColor;
    layout(location = 2) out vec3  outLockStatus;
    #else
    layout(location = 0) out vec3  outColor;
    layout(location = 1) out vec3  outInternalColor;
    layout(location = 2) out float outInternalWeight;
    layout(location = 3) out vec3  outLockStatus;
    #endif
#endif

void main()
{
    FfxFloat32x3 fOutput;
    FfxFloat32x4 fOutputInternal;
    FfxFloat32x3 fOutLockStatus;

    Accumulate(FFX_MIN16_I2(gl_FragCoord), fOutput, fOutputInternal, fOutLockStatus);

    #if FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOR_OUTPUT
    outColor            = vec3(fOutput);
    #endif

    #if FFX_FSR2_CUSTOM_OPTION_SPLIT_INTERNAL_UPSCALED_WEIGHT
    outInternalColor    = vec3(fOutputInternal.rgb);
    outInternalWeight   = float(fOutputInternal.w);
    #else
    outInternalColor    = vec4(fOutputInternal);
    #endif
    outLockStatus       = vec3(fOutLockStatus);
}
