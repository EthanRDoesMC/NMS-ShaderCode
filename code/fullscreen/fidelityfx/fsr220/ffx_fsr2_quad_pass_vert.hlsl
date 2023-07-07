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

#include "Common/Defines.shader.h"


struct UniformBuffer
{
};

DECLARE_INPUT

INPUT(vec4, mkLocalPositionVec4, POSITION0)
INPUT(vec2, mkTexCoordsVec4,     TEXCOORD0)

DECLARE_INPUT_END

DECLARE_OUTPUT

vec4 ProjPos   : S_POSITION;           // Projected space position 
vec2 TexCoord  : TEXCOORD0;

OUTPUT_SCREEN_SLICE

DECLARE_OUTPUT_END


STATIC_CONST  mat4 kFSQuadProj = mat4(2.0 / 1280.0, 0.0, 0.0, 0.0,
    0.0, -2.0 / 720.0, 0.0, 0.0,
    0.0, 0.0, -1.0, 0.0,
    -1.0, 1.0, 0.0, 1.0);

/*STATIC_CONST vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  3.0),
    vec2( 3.0, -1.0)
);*/


#ifdef D_PLATFORM_ORBIS
ROOT_SIG(VS) void main(cInput In, out cOutput Out, uint gl_VertexIndex : S_VERTEX_ID)
#else
VERTEX_MAIN_INSTANCED_SRT
#endif
{
    OUT(ProjPos) =  MUL(kFSQuadProj, IN(mkLocalPositionVec4));
    OUT(TexCoord) = IN(mkTexCoordsVec4);
    //OUT(ProjPos) = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    //vec2 lTex = (vec2(positions[gl_VertexIndex]) + 1.0f) * 0.5;
    //lTex.y = 1.0 - lTex.y;
    //OUT(TexCoord) = lTex;

}