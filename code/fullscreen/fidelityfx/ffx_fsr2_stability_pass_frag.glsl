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

// FSR2 pass ??
// SRV ?? : FSR2_PreparedInputColorLuma         : r_prepared_input_color_luma
// SRV ?? : FSR2_LumaHistory                    : r_luma_history
// CB   0 : cbFSR2

//#version 450

#ifndef D_PLATFORM_SWITCH
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require
#endif

#define FSR2_BIND_SRV_PREPARED_INPUT_COLOR                  0
#define FSR2_BIND_SRV_DILATED_MOTION_VECTORS                1
#define FSR2_BIND_SRV_DEPTH_CLIP                            2
#define FSR2_BIND_SRV_LUMA_HISTORY                          3
#define FSR2_BIND_CB_FSR2                                   4

#if FFX_FSR2_CUSTOM_OPTION_SPLIT_COLOUR_LUMA_BUFFERS
#undef  FSR2_BIND_SRV_PREPARED_INPUT_COLOR
#define FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA             0
#endif

#include "Fullscreen/FidelityFX/ffx_fsr2_callbacks_glsl.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_common.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_stability.h"

layout(location = 0) out vec4 outCurrentFrameLumaHistory;

void main()
{
    FfxFloat32x4 fOutCurrentFrameLumaHistory;
    Stability(FFX_MIN16_I2(gl_FragCoord), fOutCurrentFrameLumaHistory);
    outCurrentFrameLumaHistory = vec4(fOutCurrentFrameLumaHistory);
}