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

// FSR2 pass 6
// SRV  4 : m_Exposure                          : r_exposure
// SRV 19 : FSR2_InternalUpscaled1              : r_rcas_input
// UAV 18 : DisplayOutput                       : rw_upscaled_output
// CB   0 : cbFSR2
// CB   1 : cbRCAS

//#version 450

#ifndef D_PLATFORM_SWITCH
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_samplerless_texture_functions : require
#endif



#define FSR2_BIND_SRV_EXPOSURE              0
#define FSR2_BIND_SRV_RCAS_INPUT            1
#define FSR2_BIND_UAV_UPSCALED_OUTPUT       2
#define FSR2_BIND_CB_FSR2                   3
#define FSR2_BIND_CB_RCAS                   4

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE
#undef  FSR2_BIND_SRV_EXPOSURE
#endif

#include "Fullscreen/FidelityFX/ffx_fsr2_callbacks_glsl.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_common.h"

//Move to prototype shader!
#if defined(FSR2_BIND_CB_RCAS)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    layout (set = 1, binding = FSR2_BIND_CB_RCAS, std140) uniform cbRCAS_t
    {
        uvec4 rcasConfig;
    } cbRCAS;
    #endif // !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)

    uvec4 RCASConfig()
    {
        return cbRCAS.rcasConfig;
    }
#else
    uvec4 RCASConfig()
    {
        return uvec4(0);
    }
#endif

#if FFX_HALF
vec4 LoadRCAS_Input(FfxInt16x2 iPxPos)
{
    return texelFetch(r_rcas_input, iPxPos, 0);
}
#else
vec4 LoadRCAS_Input(FfxInt32x2 iPxPos)
{
    return texelFetch(r_rcas_input, iPxPos, 0);
}
#endif

#include "Fullscreen/FidelityFX/ffx_fsr2_rcas.h"

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
    RCAS(gl_LocalInvocationID.xyz, gl_WorkGroupID.xyz, gl_GlobalInvocationID.xyz);
}
