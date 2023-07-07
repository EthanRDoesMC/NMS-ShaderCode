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

FFX_GROUPSHARED FfxFloat32 fLogLumSumWaveShared[2];

FfxFloat32 SampleLogLuminanceClamped(FFX_MIN16_I2 iPxPos, FfxFloat32x2 fRcpRenderSize)
{
    return all(FFX_LESS_THAN_EQUAL(iPxPos, RenderSize())) ?
            SamplePreparedInputLogLuminance(FfxFloat32x2(iPxPos) * fRcpRenderSize) : 0.0f;
}

void ReduceLuminance(FFX_MIN16_I2 iPxQuartPos)
{
    FfxFloat32 fLogLumSum4 = 0.0f;
    FFX_MIN16_I2 iPxPos = iPxQuartPos * 4;

    if (all(FFX_LESS_THAN(iPxPos, RenderSize())))
    {
        FfxFloat32x2 fRcpRenderSize = 1.0f / RenderSize();
        fLogLumSum4  = SampleLogLuminanceClamped(iPxPos + FFX_MIN16_I2(1, 1), fRcpRenderSize);
        fLogLumSum4 += SampleLogLuminanceClamped(iPxPos + FFX_MIN16_I2(3, 1), fRcpRenderSize);
        fLogLumSum4 += SampleLogLuminanceClamped(iPxPos + FFX_MIN16_I2(1, 3), fRcpRenderSize);
        fLogLumSum4 += SampleLogLuminanceClamped(iPxPos + FFX_MIN16_I2(3, 3), fRcpRenderSize);
        fLogLumSum4 *= 4.0f;
    }
    FfxFloat32 fLogLumSumWave = subgroupAdd(fLogLumSum4);

    if (subgroupElect())
    {
        fLogLumSumWaveShared[gl_SubgroupID] = fLogLumSumWave;
    }
    FFX_GROUP_MEMORY_BARRIER();

    if (gl_SubgroupID == 0 && subgroupElect())
    {
        StoreReducedLuminance(iPxQuartPos >> 3, fLogLumSumWaveShared[0] + fLogLumSumWaveShared[1]);
    }
}
