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

#define GROUP_SIZE  8

#define FSR_RCAS_DENOISE 1

void WriteUpscaledOutput(FFX_MIN16_U2 iPxHrPos, FfxFloat32x3 fUpscaledColor)
{
    StoreUpscaledOutput(FFX_MIN16_I2(iPxHrPos), fUpscaledColor);
}

#if FFX_HALF
    #define FSR_RCAS_H
    FfxFloat16x4 FsrRcasLoadH(FfxInt16x2 p)
    {
        FfxFloat32x4 inputSample = LoadRCAS_Input(p); //TODO: fix type

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE == 0
        inputSample.rgb *= Exposure();
#endif

#if FFX_FSR2_OPTION_HDR_COLOR_INPUT && FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING == 0
        inputSample.rgb = Tonemap(inputSample.rgb);
#endif // #if FFX_FSR2_OPTION_HDR_COLOR_INPUT

        return FfxFloat16x4(inputSample);
    }
    void FsrRcasInputH(inout FfxFloat16 r, inout FfxFloat16 g, inout FfxFloat16 b) {}
#else
    #define FSR_RCAS_F
    FfxFloat32x4 FsrRcasLoadF(FfxInt32x2 p)
    {
        FfxFloat32x4 inputSample = LoadRCAS_Input(p);

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE == 0
        inputSample.rgb *= Exposure();
#endif

#if FFX_FSR2_OPTION_HDR_COLOR_INPUT && FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING == 0
        inputSample.rgb = Tonemap(inputSample.rgb);
#endif

        return inputSample;
    }

    void FsrRcasInputF(inout FfxFloat32 r, inout FfxFloat32 g, inout FfxFloat32 b) {}
#endif // #if FFX_HALF

#include "Fullscreen/FidelityFX/ffx_fsr1.h"


void CurrFilter(FFX_MIN16_U2 pos)
{
#if FFX_HALF
    FfxFloat16x3 c;
    FsrRcasH(c.r, c.g, c.b, pos, RCASConfig());

#if FFX_FSR2_OPTION_HDR_COLOR_INPUT && FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING == 0
    c = InverseTonemap(c);
#endif

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE == 0
    c /= FfxFloat16(Exposure());
#endif

    WriteUpscaledOutput(pos, c); //TODO: fix type
#else
    FfxFloat32x3 c;
    FsrRcasF(c.r, c.g, c.b, pos, RCASConfig());

#if FFX_FSR2_OPTION_HDR_COLOR_INPUT && FFX_FSR2_CUSTOM_OPTION_NO_TONEMAPPING == 0
    c = InverseTonemap(c);
#endif

#if FFX_FSR2_CUSTOM_OPTION_NO_EXPOSURE == 0
    c /= Exposure();
#endif

    WriteUpscaledOutput(pos, c);
#endif
}

void RCAS(FfxUInt32x3 LocalThreadId, FfxUInt32x3 WorkGroupId, FfxUInt32x3 Dtid)
{
    // Do remapping of local xy in workgroup for a more PS-like swizzle pattern.
    FfxUInt32x2 gxy = ffxRemapForQuad(LocalThreadId.x + LocalThreadId.y * 8) + FfxUInt32x2(WorkGroupId.x << 4u, WorkGroupId.y << 4u);
    CurrFilter(FFX_MIN16_U2(gxy));
    gxy.x += 8u;
    CurrFilter(FFX_MIN16_U2(gxy));
    gxy.y += 8u;
    CurrFilter(FFX_MIN16_U2(gxy));
    gxy.x -= 8u;
    CurrFilter(FFX_MIN16_U2(gxy));
}
