////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonUtils.shader.h
///     @author     strgiu
///     @date       
///
///     @brief      CommonUtils
///
///     Copyright (c) 2021 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef D_COMMON_UTILS_H
#define D_COMMON_UTILS_H

//-----------------------------------------------------------------------------
//      Includes
#include "Common/Defines.shader.h"
#include "Common/ShaderFastLibs/ShaderFastMathLib.h"
//#include "Common/ShaderFastLibs/fastApproximateAtan.h"

//-----------------------------------------------------------------------------
//      Defines
#define M_E             2.71828182845904523536   // e
#define M_LOG2E         1.44269504088896340736   // log2(e)
#define M_LOG10E        0.434294481903251827651  // log10(e)
#define M_LN2           0.693147180559945309417  // ln(2)
#define M_LN10          2.30258509299404568402   // ln(10)
#define M_PI            3.14159265358979323846   // pi
#define M_PI_2          1.57079632679489661923   // pi/2
#define M_PI_4          0.785398163397448309616  // pi/4
#define M_1_PI          0.318309886183790671538  // 1/pi
#define M_2_PI          0.636619772367581343076  // 2/pi
#define M_2_SQRTPI      1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2         1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2       0.707106781186547524401  // 1/sqrt(2)
#define M_GOLDEN_RATIO  1.61803398874989484820   // golden ratio

#define sqrt_fast_0         fastSqrtNR0
#define invsqrt_fast_0      fastRcpSqrtNR0
#define rcp_fast_0          fastRcpNR0

#define sqrt_fast_1         fastSqrtNR1
#define invsqrt_fast_1      fastRcpSqrtNR1
#define rcp_fast_1          fastRcpNR1

#define sqrt_fast_2         fastSqrtNR2
#define invsqrt_fast_2      fastRcpSqrtNR2
#define rcp_fast_2          fastRcpNR2

//-----------------------------------------------------------------------------
//      Constants
STATIC_CONST vec3 LUM_GAMMA     = vec3(0.299,  0.587,  0.114);
STATIC_CONST vec3 LUM_LINEAR    = vec3(0.2126, 0.7152, 0.0722);

//-----------------------------------------------------------------------------
//      Functions

//-----------------------------------------------------------------------------
///     Bayer
//-----------------------------------------------------------------------------
float
Bayer(
    uvec2 lPos )
{
#if defined(D_PLATFORM_METAL)
    const mat4 bayer = mat4(
#else
    STATIC_CONST mat4 bayer = mat4(
#endif
        vec4(1,  9, 3, 11), vec4(13, 5, 15, 7),
        vec4(4, 12, 2, 10), vec4(16, 8, 14, 6) );

    uvec2  positionMod = uvec2(lPos & 3);
    float  rndoffset   = bayer[positionMod.x][positionMod.y];
    return rndoffset;
}

//-----------------------------------------------------------------------------
///     BayerFract
//-----------------------------------------------------------------------------
float
BayerFract(
    uvec2 lPos )
{
    return Bayer( lPos ) / 17.0;
}

//TF_BEGIN
#if defined(D_BLOOM)
/// Bloom threshold
#define sqrt_fast   sqrt_fast_0

vec3
BrightThreshold(
	in vec3 lColour,
	in float lfThreshold,
	in float lfGain)
{
	float lum;
	lum = max(lColour.r, max(lColour.g, lColour.b));
	lum = max((lum - lfThreshold), 0.0);
	lum = 1.85 * sqrt_fast(lum);

	return lColour * lum * lfGain;
}
#endif
//TF_END
#endif // D_COMMON_UTILS_H