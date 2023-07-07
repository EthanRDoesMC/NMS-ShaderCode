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

// FSR2 pass 3
// SRV  7 : FSR2_ReconstructedPrevNearestDepth  : r_ReconstructedPrevNearestDepth
// SRV  8 : FSR2_DilatedVelocity                : r_dilated_motion_vectors
// SRV  9 : FSR2_DilatedDepth                   : r_dilatedDepth
// UAV 12 : FSR2_DepthClip                      : rw_depth_clip
// CB   0 : cbFSR2

//#version 450

#ifndef D_PLATFORM_SWITCH
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require
#endif


#define FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH      0
#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                1
#define FSR2_BIND_SRV_DILATED_DEPTH                         2
#define FSR2_BIND_CB_FSR2                                   4

#include "Fullscreen/FidelityFX/ffx_fsr2_callbacks_glsl.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_common.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_sample.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_depth_clip.h"

layout(location = 0) out float outDepthClip;

void main()
{
    FfxFloat32 fOutDepthClip;
    DepthClip(FFX_MIN16_I2(gl_FragCoord), fOutDepthClip);
    outDepthClip = float(fOutDepthClip);
}
