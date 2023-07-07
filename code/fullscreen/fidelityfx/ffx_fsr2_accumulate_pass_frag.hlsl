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


#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                 0
#define FSR2_BIND_SRV_INTERNAL_UPSCALED                      1
#define FSR2_BIND_SRV_LOCK_STATUS                            2
#define FSR2_BIND_SRV_DEPTH_CLIP                             3
#define FSR2_BIND_SRV_PREPARED_INPUT_COLOR                   4
#define FSR2_BIND_SRV_LUMA_HISTORY                           5
#define FSR2_BIND_SRV_DILATED_REACTIVE_MASKS                 6
#define FSR2_BIND_SRV_EXPOSURE_MIPS                          7
#define FSR2_BIND_SRV_LANCZOS_LUT                            8
#define FSR2_BIND_SRV_EXPOSURE                               9
#define FSR2_BIND_UAV_LOCK_STATUS                            0

#define FSR2_BIND_CB_FSR2                                    0

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
#define FSR2_BIND_UAV_LOCK_STATUS_UNORM                      0
#endif

#if FFX_FSR2_CUSTOM_OPTION_LIGHTWEIGHT_LOCKS
#undef  FSR2_BIND_UAV_LOCK_STATUS
#undef  FSR2_BIND_UAV_LOCK_STATUS_UNORM
#endif

#include "Fullscreen/FidelityFX/ffx_fsr2_callbacks_hlsl.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_common.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_sample.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_upsample.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_postprocess_lock_status.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_reproject.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_accumulate.h"

#if defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_PROSPERO )

struct cInput
{
    FfxFloat32x4       ProjPosition   : S_POSITION;
    FfxFloat32x2       TexCoord       : TEXCOORD0;
    uint               ScreenSliceIndex : S_RENDER_TARGET_INDEX;
};

struct cOutput 
{ 
    FfxFloat32x4  outColor : S_TARGET_OUTPUT0;
    FfxFloat32x4  outInternalColor : S_TARGET_OUTPUT1;
    FfxFloat32x4  outLockStatus : S_TARGET_OUTPUT2;
};	

//DEF_SRT(UniformBuffer, lUniforms);

void main( cInput In, out cOutput Out)
{
    FfxFloat32x3 fOutput;
    FfxFloat32x4 fOutputInternal;
    FfxFloat32x3 fOutLockStatus;

    FfxFloat32x2 gl_FragCoord = In.TexCoord.xy * FfxFloat32x2(DisplaySize());
    Accumulate(gl_FragCoord, fOutput, fOutputInternal, fOutLockStatus);
    Out.outColor = FfxFloat32x4(fOutput, 1.0);
    Out.outInternalColor = FfxFloat32x4(fOutputInternal);
    Out.outLockStatus = FfxFloat32x4(fOutLockStatus, 1.0);
}

#else

//UNUSED
FFX_FSR2_ROOTSIG void main(void)
{
}

#endif


