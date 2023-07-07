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

// FSR2 pass 2
// SRV  2 : m_MotionVector                      : r_motion_vectors
// SRV  3 : m_depthbuffer                       : r_depth
// UAV  7 : FSR2_ReconstructedPrevNearestDepth  : rw_reconstructed_previous_nearest_depth
// UAV  8 : FSR2_DilatedVelocity                : rw_dilated_motion_vectors
// UAV  9 : FSR2_DilatedDepth                   : rw_dilatedDepth
// CB   0 : cbFSR2

//#version 450

#ifndef D_PLATFORM_SWITCH
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require
#endif

#define FSR2_BIND_SRV_MOTION_VECTORS                        0
#define FSR2_BIND_SRV_DEPTH                                 1
#define FSR2_BIND_SRV_REACTIVE_MASK                         2
#define FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK     3
#define FSR2_BIND_SRV_PREPARED_INPUT_COLOR                  4
#define FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH      5
#define FSR2_BIND_UAV_DILATED_MOTION_VECTORS                6
#define FSR2_BIND_UAV_DILATED_DEPTH                         7
#define FSR2_BIND_UAV_DILATED_REACTIVE_MASKS                8
#define FSR2_BIND_CB_FSR2                                   9

#if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
#undef  FSR2_BIND_UAV_DILATED_MOTION_VECTORS
#define FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM          6
#endif

#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_callbacks_glsl.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_common.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_sample.h"
#include "Fullscreen/FidelityFX/FSR212/ffx_fsr2_reconstruct_dilated_velocity_and_previous_depth.h"

#ifndef FFX_FSR2_THREAD_GROUP_WIDTH
#define FFX_FSR2_THREAD_GROUP_WIDTH 8
#endif // #ifndef FFX_FSR2_THREAD_GROUP_WIDTH
#ifndef FFX_FSR2_THREAD_GROUP_HEIGHT
#define FFX_FSR2_THREAD_GROUP_HEIGHT 8
#endif // #ifndef FFX_FSR2_THREAD_GROUP_HEIGHT
#ifndef FFX_FSR2_THREAD_GROUP_DEPTH
#define FFX_FSR2_THREAD_GROUP_DEPTH 1
#endif // #ifndef FFX_FSR2_THREAD_GROUP_DEPTH
#ifndef FFX_FSR2_NUM_THREADS
#define FFX_FSR2_NUM_THREADS layout (local_size_x = FFX_FSR2_THREAD_GROUP_WIDTH, local_size_y = FFX_FSR2_THREAD_GROUP_HEIGHT, local_size_z = FFX_FSR2_THREAD_GROUP_DEPTH) in;
#endif // #ifndef FFX_FSR2_NUM_THREADS

FFX_FSR2_NUM_THREADS
void main()
{
	ReconstructAndDilate(FFX_MIN16_I2(gl_GlobalInvocationID.xy));
}
