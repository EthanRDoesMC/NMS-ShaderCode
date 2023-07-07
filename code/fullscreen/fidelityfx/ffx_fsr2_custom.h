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

#if !defined(FFX_FSR2_CUSTOM_H)
#define FFX_FSR2_CUSTOM_H

#if defined(FFX_GPU)
#include "Fullscreen/FidelityFX/ffx_core.h"

#ifndef FFX_FSR2_PREFER_WAVE64
#define FFX_FSR2_PREFER_WAVE64
//#define FFX_FSR2_PREFER_WAVE64 [WaveSize(64)]
#endif

#if defined( D_PLATFORM_SWITCH ) || defined( D_PLATFORM_PC )
#define FFX_FSR2_CUSTOM_IMPLEMENTATION
#endif
#if defined( D_PLATFORM_XBOXGDK ) || defined( D_PLATFORM_SCARLETT )
#define FFX_FSR2_CUSTOM_IMPLEMENTATION_SINGLECB
#endif

#if defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)

struct cbFSR2_t
{
    // Main
    FfxFloat32x2 fRenderOffsetUV;
    FfxInt32x2 iRenderOffset;
    FfxInt32x2 iRenderSize;
    FfxFloat32x2 fDisplayOffsetUV;
    FfxInt32x2 iDisplayOffset;
    FfxInt32x2 iDisplaySize;
    FfxUInt32x2 uLumaMipDimensions;
    FfxUInt32  uLumaMipLevelToUse;
    FfxUInt32  uFrameIndex;
    FfxFloat32x2 fDisplaySizeRcp;
    FfxFloat32x2 fJitter;
    FfxFloat32x4 fDeviceToViewDepth;
    FfxFloat32x2 depthclip_uv_scale;
    FfxFloat32x2 postprocessed_lockstatus_uv_scale;
    FfxFloat32x2 reactive_mask_dim_rcp;
    FfxFloat32x2 MotionVectorScale;
    FfxFloat32x2 fDownscaleFactor;
    FfxFloat32  fPreExposure;
    FfxFloat32  fTanHalfFOV;
    FfxFloat32x2 fMotionVectorJitterCancellation;
    FfxFloat32  fJitterSequenceLength;
    FfxFloat32  fLockInitialLifetime;
    FfxFloat32  fLockTickDelta;
    FfxFloat32  fDeltaTime;
    FfxFloat32  fDynamicResChangeFactor;
    FfxFloat32  fLumaMipRcp;
};

//#if defined(FSR2_BIND_CB_SPD)
struct cbSPD_t
{
    FfxUInt32        mips;
    FfxUInt32        numWorkGroups;
    FfxUInt32x2       workGroupOffset;
    FfxUInt32x2       renderSize;
};
//#endif // #if defined(FSR2_BIND_CB_SPD)

//#if defined(FSR2_BIND_CB_RCAS)
struct cbRCAS_t
{
    FfxUInt32x4       rcasConfig;
};
//#endif // #if defined(FSR2_BIND_CB_RCAS)

#ifdef FFX_GLSL
layout (std140)
#endif 
uniform cbFSR2Custom_t
{
    cbFSR2_t cbFSR2;

    #if defined(FSR2_BIND_CB_SPD)
    cbSPD_t  cbSPD;
    #endif

    #if defined(FSR2_BIND_CB_RCAS)
    cbRCAS_t cbRCAS;
    #endif
};

#if defined(D_PLATFORM_SWITCH)
#if defined(FSR2_BIND_SRV_INPUT_COLOR)
uniform sampler2D  r_input_color_jittered;
#endif
#if defined(FSR2_BIND_SRV_MOTION_VECTORS)
uniform sampler2D  r_motion_vectors;
#endif
#if defined(FSR2_BIND_SRV_DEPTH)
uniform sampler2D  r_depth;
#endif
#if defined(FSR2_BIND_SRV_EXPOSURE)
uniform sampler2D  r_exposure;
#endif
#if defined(FSR2_BIND_SRV_REACTIVE_MASK)
uniform sampler2D  r_reactive_mask;
#endif
#if defined(FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK)
uniform sampler2D  r_transparency_and_composition_mask;
#endif
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
uniform usampler2D r_reconstructed_previous_nearest_depth;
#endif
#if defined(FSR2_BIND_SRV_DILATED_MOTION_VECTORS)
uniform sampler2D  r_dilated_motion_vectors;
#endif
#if defined(FSR2_BIND_SRV_DILATED_DEPTH)
uniform sampler2D  r_dilatedDepth;
#endif
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
uniform sampler2D  r_internal_upscaled_color;
#endif
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED_WEIGHT)
uniform sampler2D  r_internal_upscaled_color_weight;
#endif
#if defined(FSR2_BIND_SRV_LOCK_STATUS)
uniform sampler2D  r_lock_status;
#endif
#if defined(FSR2_BIND_SRV_DEPTH_CLIP)
uniform sampler2D  r_depth_clip;
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
uniform sampler2D  r_prepared_input_color;
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
uniform sampler2D  r_prepared_input_color_luma;
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE)
uniform sampler2D  r_prepared_input_log_luminance;
#endif
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
uniform sampler2D  r_luma_history;
#endif
#if defined(FSR2_BIND_SRV_RCAS_INPUT)
uniform sampler2D  r_rcas_input;
#endif
#if defined(FSR2_BIND_SRV_LANCZOS_LUT)
uniform sampler2D  r_lanczos_lut;
#endif
#if defined(FSR2_BIND_SRV_LANCZOS_SUM_LUT)
uniform sampler3D  r_lanczos_sum_lut;
#endif
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
uniform sampler2D  r_imgMips;
#endif
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS)
uniform sampler2D  r_dilated_reactive_masks;
#endif

// UAV
#if defined FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH
layout (r32ui)          uniform uimage2D            rw_reconstructed_previous_nearest_depth;
#endif
#if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS
layout (rg32f)          uniform image2D             rw_dilated_motion_vectors;
#endif
#if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM
layout (r16)            uniform image2D             rw_dilated_motion_vectors;
#endif
#if defined FSR2_BIND_UAV_DILATED_DEPTH
layout (r32f)           uniform image2D             rw_dilatedDepth;
#endif
#if defined FSR2_BIND_UAV_INTERNAL_UPSCALED
layout (rgba32f)        uniform image2D             rw_internal_upscaled_color;
#endif
#if defined FSR2_BIND_UAV_LOCK_STATUS
layout (rgba32f)        uniform image2D             rw_lock_status;
#endif
#if defined FSR2_BIND_UAV_LOCK_STATUS_UNORM
layout (rgb10_a2)       uniform image2D             rw_lock_status;
#endif
#if defined FSR2_BIND_UAV_EDGE_STATUS
layout (r8)             uniform image2D             rw_edge_status;
#endif
#if defined FSR2_BIND_UAV_DEPTH_CLIP
layout (r32f)           uniform image2D             rw_depth_clip;
#endif
#if defined FSR2_BIND_UAV_PREPARED_INPUT_COLOR
layout (rgba32f)        uniform image2D             rw_prepared_input_color;
#endif
#if defined FSR2_BIND_UAV_PREPARED_INPUT_COLOR_LUMA
layout (r16f)           uniform image2D             rw_prepared_input_color_luma;
#endif
#if defined FSR2_BIND_UAV_LUMA_HISTORY
layout (rgba32f)        uniform image2D             rw_luma_history;
#endif
#if defined FSR2_BIND_UAV_LUMA_STABILITY
layout (r8)             uniform image2D             rw_luma_stability;
#endif
#if defined FSR2_BIND_UAV_UPSCALED_OUTPUT
layout (rgba32f)        uniform image2D             rw_upscaled_output;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE
layout (r32f)           uniform coherent image2D    rw_img_mip_shading_change;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE_MIP_5
layout (r32f)           uniform coherent image2D    rw_img_mip_5;
#endif
#if defined(FSR2_BIND_UAV_DILATED_REACTIVE_MASKS)
layout (rg32f)          uniform image2D             rw_dilated_reactive_masks;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE 
layout (rg32f)          uniform image2D             rw_exposure;
#endif
#if defined FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC 
layout (r32ui)          uniform coherent uimage2D   rw_spd_global_atomic;
#endif
#endif // #if defined(D_PLATFORM_SWITCH)

#if !defined(D_PLATFORM_SWITCH)
#if defined(FSR2_BIND_SRV_INPUT_COLOR)
layout(set = 1)   uniform sampler2D  r_input_color_jittered;
#endif
#if defined(FSR2_BIND_SRV_MOTION_VECTORS)
layout(set = 1)   uniform sampler2D  r_motion_vectors;
#endif
#if defined(FSR2_BIND_SRV_DEPTH)
layout(set = 1)   uniform sampler2D  r_depth;
#endif
#if defined(FSR2_BIND_SRV_EXPOSURE)
layout(set = 1)   uniform sampler2D  r_exposure;
#endif
#if defined(FSR2_BIND_SRV_REACTIVE_MASK)
layout(set = 1)   uniform sampler2D  r_reactive_mask;
#endif
#if defined(FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK)
layout(set = 1)   uniform sampler2D  r_transparency_and_composition_mask;
#endif
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
layout(set = 1)   uniform usampler2D r_reconstructed_previous_nearest_depth;
#endif
#if defined(FSR2_BIND_SRV_DILATED_MOTION_VECTORS)
layout(set = 1)   uniform sampler2D  r_dilated_motion_vectors;
#endif
#if defined(FSR2_BIND_SRV_DILATED_DEPTH)
layout(set = 1)   uniform sampler2D  r_dilatedDepth;
#endif
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
layout(set = 1)   uniform sampler2D  r_internal_upscaled_color;
#endif
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED_WEIGHT)
layout(set = 1)   uniform sampler2D  r_internal_upscaled_color_weight;
#endif
#if defined(FSR2_BIND_SRV_LOCK_STATUS)
layout(set = 1)   uniform sampler2D  r_lock_status;
#endif
#if defined(FSR2_BIND_SRV_DEPTH_CLIP)
layout(set = 1)   uniform sampler2D  r_depth_clip;
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
layout(set = 1)   uniform sampler2D  r_prepared_input_color;
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
layout(set = 1)   uniform sampler2D  r_prepared_input_color_luma;
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE)
layout(set = 1)   uniform sampler2D  r_prepared_input_log_luminance;
#endif
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
layout(set = 1)   uniform sampler2D  r_luma_history;
#endif
#if defined(FSR2_BIND_SRV_RCAS_INPUT)
layout(set = 1)   uniform sampler2D  r_rcas_input;
#endif
#if defined(FSR2_BIND_SRV_LANCZOS_LUT)
layout(set = 1)   uniform sampler2D  r_lanczos_lut;
#endif
#if defined(FSR2_BIND_SRV_LANCZOS_SUM_LUT)
layout(set = 1)   uniform sampler3D  r_lanczos_sum_lut;
#endif
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
layout(set = 1)   uniform sampler2D  r_imgMips;
#endif
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS)
layout(set = 1)   uniform sampler2D  r_dilated_reactive_masks;
#endif

// UAV
#if defined FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH
layout (set = 2, r32ui)          uniform uimage2D           rw_reconstructed_previous_nearest_depth;
#endif
#if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS
layout (set = 2, rg32f)          uniform image2D            rw_dilated_motion_vectors;
#endif
#if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM
layout (set = 2, rg16)           uniform image2D            rw_dilated_motion_vectors;
#endif
#if defined FSR2_BIND_UAV_DILATED_DEPTH
layout (set = 2, r32f)           uniform image2D            rw_dilatedDepth;
#endif
#if defined FSR2_BIND_UAV_INTERNAL_UPSCALED
layout (set = 2, rgba32f)        uniform image2D            rw_internal_upscaled_color;
#endif
#if defined FSR2_BIND_UAV_LOCK_STATUS
layout (set = 2, rgba32f)        uniform image2D            rw_lock_status;
#endif
#if defined FSR2_BIND_UAV_LOCK_STATUS_UNORM
layout (set = 2, rgb10_a2)       uniform image2D            rw_lock_status;
#endif
#if defined FSR2_BIND_UAV_EDGE_STATUS
layout (set = 2, r8)             uniform image2D            rw_edge_status;
#endif
#if defined FSR2_BIND_UAV_DEPTH_CLIP
layout (set = 2, r32f)           uniform image2D            rw_depth_clip;
#endif
#if defined FSR2_BIND_UAV_PREPARED_INPUT_COLOR
layout (set = 2, rgba32f)        uniform image2D            rw_prepared_input_color;
#endif
#if defined FSR2_BIND_UAV_PREPARED_INPUT_COLOR_LUMA
layout (set = 2, r16f)           uniform image2D            rw_prepared_input_color_luma;
#endif
#if defined FSR2_BIND_UAV_LUMA_HISTORY
layout (set = 2, rgba32f)        uniform image2D            rw_luma_history;
#endif
#if defined FSR2_BIND_UAV_LUMA_STABILITY
layout (set = 2,  r8)            uniform image2D            rw_luma_stability;
#endif
#if defined FSR2_BIND_UAV_UPSCALED_OUTPUT
layout (set = 2, rgba32f)        uniform image2D            rw_upscaled_output;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE
layout (set = 2, r32f)           uniform coherent image2D   rw_img_mip_shading_change;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE_MIP_5
layout (set = 2, r32f)           uniform coherent image2D   rw_img_mip_5;
#endif
#if defined(FSR2_BIND_UAV_DILATED_REACTIVE_MASKS)
layout (set = 2, rg32f)          uniform image2D            rw_dilated_reactive_masks;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE
layout (set = 2, rg32f)          uniform image2D            rw_exposure;
#endif
#if defined FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC 
layout (set = 2, r32ui)          uniform coherent uimage2D  rw_spd_global_atomic;
#endif
#endif // #if !defined(D_PLATFORM_SWITCH)

#endif // #if defined( FFX_FSR2_CUSTOM_IMPLEMENTATION)

#endif // #if defined(FFX_GPU)

#endif //!defined(FFX_FSR2_CUSTOM_H)
