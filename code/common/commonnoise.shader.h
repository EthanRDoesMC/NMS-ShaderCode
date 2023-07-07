////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonDepth.h
///     @author     User
///     @date       
///
///     @brief      CommonDepth
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONNOISE_H
#define D_COMMONNOISE_H

#define D_USE_NOISETEXTURE

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Common.shader.h"
#include "Common/noise3d.glsl"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Functions


//-----------------------------------------------------------------------------
//      Global Data
    
STATIC_CONST mat3 kNoiseMat3      = mat3(  0.00,  0.80,  0.60,
                                          -0.80,  0.36, -0.48,
                                          -0.60, -0.48,  0.64 );
//-----------------------------------------------------------------------------
//      Functions 

//-----------------------------------------------------------------------------
///
///     Hash
///
///     @brief      Hash
///
///     @param      float lfNumber
///     @return     Hash
///
//-----------------------------------------------------------------------------
float
Hash(
    float lfNumber )
{
    return fract( sin( lfNumber ) * 43758.5453 );
}

//-----------------------------------------------------------------------------
///
///     Noise
///
///     @brief      Noise
///
///     @param      vec3 lfIndex 
///     @return     
///
//-----------------------------------------------------------------------------

#if defined ( D_USE_NOISETEXTURE )

//const mat3 m = mat3(  0.00,  0.80,  0.60, -0.80,  0.36, -0.48, -0.60, -0.48,  0.64 );

float Noise3D(
    in vec3 lPositionVec3, 
    SAMPLER2DARG( lNoiseMap ) )
{
    vec3 p = floor(lPositionVec3);
    vec3 f = fract(lPositionVec3);
    f = f*f*(3.0-2.0*f);
    vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    vec2 rg = texture2DLod( lNoiseMap, (uv+ 0.5)/256.0, 0.0 ).yx;
#ifdef D_NORMALISED_NOISE
    return mix( rg.x, rg.y, f.z );
#else
    return -1.0+2.0*mix( rg.x, rg.y, f.z );
#endif
}

float Noise3DFast(
    in vec3 lPositionVec3,
    SAMPLER2DARG(lNoiseMap))
{
    vec3 p = floor(lPositionVec3);
    vec3 f = fract(lPositionVec3);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture2DLod(lNoiseMap, (uv + 0.5) / 256.0, 0.0).yx;
    return mix(rg.x, rg.y, f.z);
}


float TurbulenceNoise3DFast(
    in vec3 lPositionVec3,
    SAMPLER2DARG(lNoiseMap))
{
    vec3 p = floor(lPositionVec3);
    vec3 f = fract(lPositionVec3);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture2DLod(lNoiseMap, (uv + 0.5) / 256.0, 0.0).yx;
    return abs(-1.0 + 2.0*mix(rg.x, rg.y, f.z));
}


#else

float
Noise3D(
    in vec3 lPositionVec3, 
    SAMPLER2DARG( lNoiseMap ) )
{

    vec3 lFloorVec3 = floor( lPositionVec3 );
    vec3 lFractVec3 = fract( lPositionVec3 );

    lFractVec3 = lFractVec3 * lFractVec3 * ( 3.0 - 2.0 * lFractVec3 );

    float lfIndex = lFloorVec3.x + lFloorVec3.y * 57.0 + 113.0 * lFloorVec3.z;

    float res = mix( mix( mix( Hash( lfIndex + 0.0 ), Hash( lfIndex + 1.0 ), lFractVec3.x ),
        mix( Hash( lfIndex + 57.0 ), Hash( lfIndex + 58.0 ), lFractVec3.x ), lFractVec3.y ),
        mix( mix( Hash( lfIndex + 113.0 ), Hash( lfIndex + 114.0 ), lFractVec3.x ),
        mix( Hash( lfIndex + 170.0 ), Hash( lfIndex + 171.0 ), lFractVec3.x ), lFractVec3.y ), lFractVec3.z );

    return res;
}

float
Noise3DFast(
    in vec3 lPositionVec3,
    SAMPLER2DARG( lNoiseMap ) )
{
    return Noise3D( lPositionVec3, lNoiseMap );
}

float TurbulenceNoise3DFast(
    in vec3 lPositionVec3,
    SAMPLER2DARG(lNoiseMap))
{
    return abs(-1.0 + 2.0 * Noise3D(lPositionVec3, lNoiseMap));
}

#endif

#ifdef D_NO_MATRIX_MULTIPLY
#define MULTIPLY_NOISE_POSITION(x, y) (x * 2.0)
#else
#define MULTIPLY_NOISE_POSITION(x, y) MUL(kNoiseMat3, x * y)
#endif

//-----------------------------------------------------------------------------
///
///     FractBrownianMotion
///
///     @brief      FractBrownianMotion
///
///     @param      vec3 lfIndex 
///     @return     
///
//-----------------------------------------------------------------------------
float
FractBrownianMotion4(
    vec3 lPositionVec3, 
    SAMPLER2DARG( lNoiseMap ) )
{
    vec3 lPosCopy = lPositionVec3; // copy because for some reason PSSL compiler balks if you operate on this directly then pass it on.
    float lfSum;
    lfSum = 0.5000 * Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) );
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.02);

    lfSum += 0.2500 * Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) );
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.03);

    lfSum += 0.1250 * Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) );
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.01);

    lfSum += 0.0625 * Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) );
    return lfSum / 0.9375;
}

// Faster variant (also normalized 0.0.1.0)

float
FractBrownianMotion4Fast(
vec3 lPositionVec3,
SAMPLER2DARG(lNoiseMap))
{
    vec3 lPosCopy = lPositionVec3; // copy because for some reason PSSL compiler balks if you operate on this directly then pass it on.
    float lfSum;
    lfSum = 0.5000 * Noise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.02);

    lfSum += 0.2500 * Noise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.03);

    lfSum += 0.1250 * Noise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.01);

    lfSum += 0.0625 * Noise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    return lfSum / 0.9375;
}

float
Turbulence6(
    vec3 lPositionVec3,
    SAMPLER2DARG(lNoiseMap))
{
    vec3 lPosCopy = lPositionVec3; // copy because for some reason PSSL compiler balks if you operate on this directly then pass it on.
    float lfSum = 0.0;
    lfSum += 0.5000 * TurbulenceNoise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.02);

    lfSum += 0.2500 * TurbulenceNoise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.03);

    lfSum += 0.1250 * TurbulenceNoise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.01);

    lfSum += 0.0625 * TurbulenceNoise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.04);

    lfSum += 0.03125 * TurbulenceNoise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.02);

    lfSum += 0.015625 * TurbulenceNoise3DFast(lPosCopy, SAMPLER2DPARAM(lNoiseMap));

    return lfSum / 0.9375;
}

//-----------------------------------------------------------------------------
///
///     FractBrownianMotion6
///
///     @brief      FractBrownianMotion6
///
///     @param      vec3 lPositionVec3
///     @param      SAMPLER2DARG
///     @param      lNoiseMap 
///     @return     float
///
//-----------------------------------------------------------------------------
float 
FractBrownianMotion6( 
    vec3 lPositionVec3,
    SAMPLER2DARG( lNoiseMap ) )
{
    vec3 lPosCopy = lPositionVec3; 
    float lfSum = 0.0;

    lfSum    += 0.500000*( 0.5+0.5*Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) ) ); 
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.02);

    lfSum    += 0.250000*( 0.5+0.5*Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) ) ); 
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.03);

    lfSum    += 0.125000*( 0.5+0.5*Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) ) ); 
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.01);

    lfSum    += 0.062500*( 0.5+0.5*Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) ) ); 
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.04);

    lfSum    += 0.031250*( 0.5+0.5*Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) ) ); 
    lPosCopy = MULTIPLY_NOISE_POSITION(lPosCopy, 2.01);

    lfSum    += 0.015625*( 0.5+0.5*Noise3D( lPosCopy, SAMPLER2DPARAM( lNoiseMap ) ) ); 

    return lfSum/0.96875;
}



//-----------------------------------------------------------------------------
///
///     DistortedNoise
///
///     @brief      DistortedNoise
///
///     @param      vec3 lPositionVec3
///     @param      float lfScale1
///     @param      float lfScale2
///     @param      SAMPLER2DARG
///     @param      lNoiseMap 
///     @return     float
///
//-----------------------------------------------------------------------------
float 
DistortedNoise( 
    vec3  lPositionVec3, 
    float lfScale1, 
    float lfScale2,
    SAMPLER2DARG( lNoiseMap ) )
{
    float lfNoise1   = FractBrownianMotion4Fast( lPositionVec3 * lfScale1, SAMPLER2DPARAM( lNoiseMap ) );
    float lfNoise2   = FractBrownianMotion4Fast( lPositionVec3 * lfScale1 + vec3( 1000.0, 1000.0, 1000.0 ), SAMPLER2DPARAM( lNoiseMap ) );

    vec3  lInputVec3 = vec3( lfNoise1, lfNoise2, 1.0 );
    float lfReturn   = FractBrownianMotion6( lPositionVec3 * lfScale1 + lInputVec3 * lfScale2, SAMPLER2DPARAM( lNoiseMap ) );

    return lfReturn * 0.5 + 0.5;
}

#endif
