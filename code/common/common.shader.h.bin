////////////////////////////////////////////////////////////////////////////////
///
///     @file       Common.h
///     @author     User
///     @date       
///
///     @brief      Common
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef D_COMMON_H
#define D_COMMON_H

#define D_TERRAINCOLOURARRAY_SIZE       23
#define D_TERRAINCOLOURARRAY_SIZE_FLOAT 6


STATIC_CONST vec3 kGammaOutVec3 = vec3( 1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2 );
STATIC_CONST vec3 kGammaInVec3  = vec3( 2.2, 2.2, 2.2 );
STATIC_CONST vec4 RGBToHSV_K    = vec4( 0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0 );
STATIC_CONST vec4 HSVToRGB_K    = vec4( 1.0,  2.0 / 3.0, 1.0 / 3.0,  3.0 );
STATIC_CONST mat3 BT709_TO_BT2020 = mat3(   //ref: ARIB STD-B62 and BT.2087
	0.6274, 0.3293, 0.0433,
	0.0691, 0.9195, 0.0114,
	0.0164, 0.0880, 0.8956
    );
STATIC_CONST mat3 BT2020_TO_BT709 = mat3(
    1.6605, -0.5877, -0.0728,
    -0.1246, 1.1330, -0.0084,
    -0.0182, -0.1006, 1.1187
    );

STATIC_CONST half4 HSVToRGBHalf_K = half4( 1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0 );

//-----------------------------------------------------------------------------
///
///     Mix half precison
///
//-----------------------------------------------------------------------------

half mix_h(half x, half y, half a)
{
    return (x * half(1.0 - a)) + y * a;
}

half2 mix_h(half2 x, half2 y, half a)
{
    return (x * half(1.0 - a)) + y * a;
}

half3 mix_h(half3 x, half3 y, half a)
{
    return (x * half(1.0 - a)) + y * a;
}

half4 mix_h(half4 x, half4 y, half a)
{
    return (x * half(1.0 - a)) + y * a;
}

//-----------------------------------------------------------------------------
///
///     GammaCorrect
///
//-----------------------------------------------------------------------------
#if !( defined( D_PLATFORM_ORBIS ) && defined( _F07_UNLIT ) && defined( D_TERRAIN ) ) && !defined( D_IBL )

float 
GammaCorrectInputF(
    in float lfValue )
{
    float lfSign = (lfValue < 0.0)? -1.0 : 1.0;
    float lfCorrectValue = lfValue * lfSign;

    if( lfCorrectValue > 1.0 )
    {
        lfCorrectValue = pow( lfCorrectValue, 2.4 );
    }
    else if( lfCorrectValue > 0.0 )
    {        
        #if defined( D_PLATFORM_PC ) || defined( D_PLATFORM_METAL )
        lfCorrectValue = pow( lfCorrectValue, 2.2 ); // lfCorrectValue * ( lfCorrectValue * ( lfCorrectValue * ( 0.305306011 ) + ( 0.682171111 ) ) + ( 0.012522878 ) );
        #else
        lfCorrectValue = lfCorrectValue * ( lfCorrectValue * ( lfCorrectValue * ( 0.305306011 ) + ( 0.682171111 ) ) + ( 0.012522878 ) );
        #endif
    }

    lfCorrectValue *= lfSign;

    return lfCorrectValue;
}


vec3 
GammaCorrectInput(
    in vec3 lColourVec3 )
{
    vec3 lCorrectColourVec3;

    lCorrectColourVec3.x = GammaCorrectInputF(lColourVec3.x);
    lCorrectColourVec3.y = GammaCorrectInputF(lColourVec3.y);
    lCorrectColourVec3.z = GammaCorrectInputF(lColourVec3.z);

    return lCorrectColourVec3;
}

#endif

// Only works for the range [0,1] -- not reversible
vec3 
GammaCorrectInput01(
    in vec3 lColourVec3 )
{
    vec3 lCorrectColourVec3;
    lCorrectColourVec3 = lColourVec3 * ( lColourVec3 * ( lColourVec3 * float2vec3( 0.305306011 ) + float2vec3( 0.682171111 ) ) + float2vec3( 0.012522878 ) );

    return lCorrectColourVec3;
}

// Only works for the range [0,1] -- not reversible
half3
GammaCorrectInput01Half(
    in half3 lColourVec3)
{
    half3 lCorrectColourVec3;
    lCorrectColourVec3 = lColourVec3 * (lColourVec3 * (lColourVec3 * float2half3(0.305306011) + float2half3(0.682171111) ) + float2half3(0.012522878) );

    return lCorrectColourVec3;
}

//-----------------------------------------------------------------------------
///
///     GammaCorrect
///
//-----------------------------------------------------------------------------
#if !( defined( D_PLATFORM_ORBIS ) && defined( _F07_UNLIT ) && defined( D_TERRAIN ) ) && !defined( D_IBL )

float 
GammaCorrectOutputF(
    in float lfValue )
{
    float lfSign = (lfValue < 0.0)? -1.0 : 1.0;
    float lfCorrectValue = lfValue * lfSign;

    if( lfCorrectValue < 1.0 )
    {
        lfCorrectValue = pow(lfCorrectValue, 0.45454545454);
    }
    else if( lfCorrectValue > 0.0 )
    {
        lfCorrectValue = pow(lfCorrectValue, 0.416666667);
    }

    lfCorrectValue *= lfSign;

    return lfCorrectValue;
}



vec3 
GammaCorrectOutput(
    in vec3 lColourVec3 )
{
    vec3 lCorrectColourVec3;

    lCorrectColourVec3.x = GammaCorrectOutputF(lColourVec3.x);
    lCorrectColourVec3.y = GammaCorrectOutputF(lColourVec3.y);
    lCorrectColourVec3.z = GammaCorrectOutputF(lColourVec3.z);

    return lCorrectColourVec3;
}

#endif

//-----------------------------------------------------------------------------
///
///     RGBToHSV
///
//-----------------------------------------------------------------------------
vec3 
RGBToHSV(
    vec3 lRGB )
{
    //vec4 p = mix( vec4(lRGB.bg, RGBToHSV_K.wz), vec4(lRGB.gb, RGBToHSV_K.xy), step(lRGB.b, lRGB.g) );
    //vec4 q = mix( vec4(p.xyw, lRGB.r), vec4(lRGB.r, p.yzx), step(p.x, lRGB.r) );
    // This variant is faster, since it generates conditional moves
    vec4 p = lRGB.g < lRGB.b ? vec4(lRGB.bg, RGBToHSV_K.wz) : vec4(lRGB.gb, RGBToHSV_K.xy);
    vec4 q = lRGB.r < p.x ? vec4(p.xyw, lRGB.r) : vec4(lRGB.r, p.yzx);    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

half3
RGBToHSVHalf(
    half3 lRGB)
{
    // This variant is faster, since it generates conditional moves
#if defined ( D_PLATFORM_METAL )
    half4 p = lRGB.g < lRGB.b ? half4(lRGB.bg, half2( RGBToHSV_K.wz )) : half4(lRGB.gb, half2( RGBToHSV_K.xy ));
#else
    half4 p = lRGB.g < lRGB.b ? half4(lRGB.bg, RGBToHSV_K.wz) : half4(lRGB.gb, RGBToHSV_K.xy);
#endif
    half4 q = lRGB.r < p.x ? half4(p.xyw, lRGB.r) : half4(lRGB.r, p.yzx);
    half d = q.x - min(q.w, q.y);
    half e = 1.0e-10;
    return half3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}


//-----------------------------------------------------------------------------
///
///     HSVToRGB
///
///     @brief      http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
///
//-----------------------------------------------------------------------------
vec3 
HSVToRGB(
    vec3 lHSV )
{
    vec3 p = abs(fract(lHSV.xxx + HSVToRGB_K.xyz) * 6.0 - HSVToRGB_K.www);
    return lHSV.z * mix(HSVToRGB_K.xxx, saturate(p - HSVToRGB_K.xxx), lHSV.y);
}

half3
HSVToRGBHalf(
    half3 lHSV )
{
    half3 p = abs( fract( lHSV.xxx + HSVToRGBHalf_K.xyz ) * 6.0 - HSVToRGBHalf_K.www );
    return lHSV.z * mix_h( HSVToRGBHalf_K.xxx, saturate( p - HSVToRGBHalf_K.xxx ), lHSV.y );
}

vec3
AdditiveBlendInHsvSpace(in vec3 a, in vec3 b, in float scale)
{
	// Additive op in HSV space which chooses shortest distance via Hue.
	vec3 colourOutput;

	colourOutput.yz = a.yz + b.zy * scale;
	float cw = a.x > b.x ? 1.0 + b.x - a.x : b.x - a.x;
	float ccw = a.x > b.x ? b.x - a.x : b.x - (1.0 + a.x);
	colourOutput.x = fract(abs(cw) < abs(ccw) ? a.x + cw * scale : a.x + 1.0 + ccw * scale);

	return colourOutput;
}

vec3
InterpolationInHsvSpace(in vec3 a, in vec3 b, in float scale)
{
	// Interpolating op which chooses shortest distance via Hue
	vec3 colourOutput;

	colourOutput.yz = mix(a.yz, b.zy, scale);
	float cw = a.x > b.x ? 1.0 + b.x - a.x : b.x - a.x;
	float ccw = a.x > b.x ? b.x - a.x : b.x - (1.0 + a.x);
	colourOutput.x = fract(1.0 + a.x + (abs(cw) < abs(ccw) ? cw : ccw) * scale);

	return colourOutput;
}

vec3
AdditiveBlendInHsvSpaceCw(in vec3 a, in vec3 b, in float scale)
{
	// Additive op in HSV space, always takes the CW path
	vec3 colourOutput;

	colourOutput.yz = a.yz + b.zy * scale;
	float cw = a.x > b.x ? 1.0 + b.x - a.x : b.x - a.x;
	colourOutput.x = fract(a.x + cw * scale);

	return colourOutput;
}

vec3
InterpolationInHsvSpaceCw(in vec3 a, in vec3 b, in float scale)
{
	// Intgerpolating op, always CW
	vec3 colourOutput;

	colourOutput.yz = mix(a.yz, b.zy, scale);
	float cw = a.x > b.x ? 1.0 + b.x - a.x : b.x - a.x;
	colourOutput.x = fract(a.x + cw * scale);

	return colourOutput;
}

//-----------------------------------------------------------------------------
///
///     HDR ( Perceptual Quantizer(PQ), Rec. 2020 color space. ) helpers
///
//------------------------------------------------------------------------------

#if !defined( D_PLATFORM_ORBIS)
float CndMask(bool cnd, float src0, float src1)
{
    return cnd ? src0 : src1;
}
#endif

STATIC_CONST float kRefWhiteLevel = 100.0;

vec4 SRGB_OETF(vec4 L)
{
	vec3 dark = L.xyz * 12.92;
	vec3 light = 1.055 * pow(L.xyz, float2vec3(1.0 / 2.4)) - 0.055;

	vec4 r;
    r.x = CndMask(L.x <= 0.0031308, dark.x, light.x);
    r.y = CndMask(L.y <= 0.0031308, dark.y, light.y);
    r.z = CndMask(L.z <= 0.0031308, dark.z, light.z);
	r.w = L.w;
	return r;
}

vec4 SRGB_EOTF(vec4 E)
{
	vec3 dark = E.xyz / 12.92;
	vec3 light = pow((E.xyz + 0.055) / (1 + 0.055), float2vec3(2.4));
	vec4 r;
    r.x = CndMask(E.x <= 0.04045, dark.x, light.x);
    r.y = CndMask(E.y <= 0.04045, dark.y, light.y);
    r.z = CndMask(E.z <= 0.04045, dark.z, light.z);
	r.w = E.w;
	return r;
}

//apply gamma adjustment to (minL, maxL).
vec4 GammaAdjOOTF(vec4 L, float minLNits, float maxLNits, float gamma, bool inverse)
{
	vec3 nits = L.xyz * kRefWhiteLevel;
	
	vec4 i = vec4((nits - minLNits) / (maxLNits - minLNits), 1.0);
	vec3 j;
	if (inverse){
		j = SRGB_EOTF(pow(i, float2vec4(1.0 / gamma))).xyz;
	}
	else{
		j = pow(SRGB_OETF(i).xyz,float2vec3(gamma));
	}
	vec3 adj = (minLNits + (maxLNits - minLNits) * j) / kRefWhiteLevel;
	vec4 ret;
    ret.x = CndMask(nits.x >= minLNits && nits.x < maxLNits, adj.x, L.x);
    ret.y = CndMask(nits.y >= minLNits && nits.y < maxLNits, adj.y, L.y);
    ret.z = CndMask(nits.z >= minLNits && nits.z < maxLNits, adj.z, L.z);
	ret.w = L.w;
	return ret;
}

//input: normalized L in units of RefWhite (1.0=100nits), output: normalized E
vec4 PQ_OETF(vec4 L, uint gamma_adj, float gamma)
{
	if (gamma_adj != 0)
		L = GammaAdjOOTF(L, 0.0, 300.0, gamma, false);
	const float c1 = 0.8359375;//3424.f/4096.f;
	const float c2 = 18.8515625;//2413.f/4096.f*32.f;
	const float c3 = 18.6875; //2392.f/4096.f*32.f;
	const float m1 = 0.159301758125; //2610.f / 4096.f / 4;
	const float m2 = 78.84375;// 2523.f / 4096.f * 128.f;
	L = L * kRefWhiteLevel / 10000.0;
	vec3 Lm1 = pow(L.xyz, float2vec3(m1));
	vec3 X = (c1 + c2 * Lm1) / (1.0 + c3 * Lm1);
	vec4 res = vec4(pow(X, float2vec3(m2)), L.w);
	return res;
}

//input: normalized E (0.0, 1.0), output: normalized L in units of RefWhite
vec4 PQ_EOTF(vec4 E, uint gamma_adj, float gamma)
{
	const float c1 = 0.8359375;//3424.f/4096.f;
	const float c2 = 18.8515625;//2413.f/4096.f*32.f;
	const float c3 = 18.6875; //2392.f/4096.f*32.f;
	const float m1 = 0.159301758125; //2610.f / 4096.f / 4;
	const float m2 = 78.84375;// 2523.f / 4096.f * 128.f;
	vec3 M = c2 - c3 * pow(E.xyz, float2vec3(1.0 / m2));
	vec3 N = max(pow(E.xyz, float2vec3(1.0 / m2)) - c1, 0.0);
	vec3 L = pow(N / M, float2vec3(1.0 / m1)); //normalized nits (1.0 = 10000nits)
	L = L * 10000.0 / kRefWhiteLevel; //convert to normalized L in units of RefWhite
	return (gamma_adj !=0) ? GammaAdjOOTF(vec4(L, E.w), 0.0, 300.0, gamma, true) : vec4(L, E.w);

}

// PQ OETF fast approximation
// http://www.glowybits.com/blog/2017/01/04/ifl_iss_hdr_2/

vec3 PQ_OETF_Fast(vec3 x)
{
    x = (x * (x * (x * (x * (x * 533095.76 + 47438306.2) + 29063622.1) + 575216.76) + 383.09104) + 0.000487781) /
        (x * (x * (x * (x * 66391357.4 + 81884528.2) + 4182885.1) + 10668.404) + 1.0);
    return x;
}

//-----------------------------------------------------------------------------
///
///     BrightnessVibranceContrast
///
//-----------------------------------------------------------------------------
vec3 BrightnessVibranceContrast(
    vec3  lInputColour,
    float lfBrightness,
    float lfVibrance,
    float lfContrast)
{
    vec3 lBrtResult     = lInputColour * lfBrightness;

    // get lum
    vec3 lLuma          = float2vec3( dot(lBrtResult, vec3( 0.2125, 0.7154, 0.0721 )) );

    // get saturation
    float lfMaxCol      = max( lBrtResult.r, max(lBrtResult.g, lBrtResult.b) );
    float lfMinCol      = min( lBrtResult.r, min(lBrtResult.g, lBrtResult.b) );
    float lfCurSatV     = lfMaxCol - lfMinCol;

    // lerp by 1 + (1 - vibrance) - current saturation
    float lfVibranceMix = (1.0 + (lfVibrance * (1.0 - (sign(lfVibrance) * lfCurSatV))));
    vec3 lVibResult     = mix( lLuma, lBrtResult, lfVibranceMix );

    // lerp from mid gray for contrast
    vec3 lContrastBase  = vec3( 0.5, 0.5, 0.5 );
    vec3 lConResult     = mix( lContrastBase , lVibResult, lfContrast );

    return lConResult;
}


//-----------------------------------------------------------------------------
///
///     Desaturate
///
//-----------------------------------------------------------------------------
vec3 Desaturate( vec3 color, float lfAmount )
{
    vec3 gray = float2vec3( dot( vec3( 0.299, 0.587, 0.114 ), color) );
    return mix( color, gray, lfAmount );
}


//-----------------------------------------------------------------------------
///
///     Inverse
///
//-----------------------------------------------------------------------------
mat4
Inverse(
    const mat4 lInMat4 )
{
#ifdef D_PLATFORM_GLSL
    return inverse( lInMat4 );
#else
    float det = determinant( lInMat4 );
    det = 1.0f / det;

    mat4 _M = lInMat4;
    mat4 IM;
    IM[0][0] = det * ( _M[1][2]*_M[2][3]*_M[3][1] - _M[1][3]*_M[2][2]*_M[3][1] + _M[1][3]*_M[2][1]*_M[3][2] - _M[1][1]*_M[2][3]*_M[3][2] - _M[1][2]*_M[2][1]*_M[3][3] + _M[1][1]*_M[2][2]*_M[3][3] );
    IM[0][1] = det * ( _M[0][3]*_M[2][2]*_M[3][1] - _M[0][2]*_M[2][3]*_M[3][1] - _M[0][3]*_M[2][1]*_M[3][2] + _M[0][1]*_M[2][3]*_M[3][2] + _M[0][2]*_M[2][1]*_M[3][3] - _M[0][1]*_M[2][2]*_M[3][3] );
    IM[0][2] = det * ( _M[0][2]*_M[1][3]*_M[3][1] - _M[0][3]*_M[1][2]*_M[3][1] + _M[0][3]*_M[1][1]*_M[3][2] - _M[0][1]*_M[1][3]*_M[3][2] - _M[0][2]*_M[1][1]*_M[3][3] + _M[0][1]*_M[1][2]*_M[3][3] );
    IM[0][3] = det * ( _M[0][3]*_M[1][2]*_M[2][1] - _M[0][2]*_M[1][3]*_M[2][1] - _M[0][3]*_M[1][1]*_M[2][2] + _M[0][1]*_M[1][3]*_M[2][2] + _M[0][2]*_M[1][1]*_M[2][3] - _M[0][1]*_M[1][2]*_M[2][3] );
    IM[1][0] = det * ( _M[1][3]*_M[2][2]*_M[3][0] - _M[1][2]*_M[2][3]*_M[3][0] - _M[1][3]*_M[2][0]*_M[3][2] + _M[1][0]*_M[2][3]*_M[3][2] + _M[1][2]*_M[2][0]*_M[3][3] - _M[1][0]*_M[2][2]*_M[3][3] );
    IM[1][1] = det * ( _M[0][2]*_M[2][3]*_M[3][0] - _M[0][3]*_M[2][2]*_M[3][0] + _M[0][3]*_M[2][0]*_M[3][2] - _M[0][0]*_M[2][3]*_M[3][2] - _M[0][2]*_M[2][0]*_M[3][3] + _M[0][0]*_M[2][2]*_M[3][3] );
    IM[1][2] = det * ( _M[0][3]*_M[1][2]*_M[3][0] - _M[0][2]*_M[1][3]*_M[3][0] - _M[0][3]*_M[1][0]*_M[3][2] + _M[0][0]*_M[1][3]*_M[3][2] + _M[0][2]*_M[1][0]*_M[3][3] - _M[0][0]*_M[1][2]*_M[3][3] );
    IM[1][3] = det * ( _M[0][2]*_M[1][3]*_M[2][0] - _M[0][3]*_M[1][2]*_M[2][0] + _M[0][3]*_M[1][0]*_M[2][2] - _M[0][0]*_M[1][3]*_M[2][2] - _M[0][2]*_M[1][0]*_M[2][3] + _M[0][0]*_M[1][2]*_M[2][3] );
    IM[2][0] = det * ( _M[1][1]*_M[2][3]*_M[3][0] - _M[1][3]*_M[2][1]*_M[3][0] + _M[1][3]*_M[2][0]*_M[3][1] - _M[1][0]*_M[2][3]*_M[3][1] - _M[1][1]*_M[2][0]*_M[3][3] + _M[1][0]*_M[2][1]*_M[3][3] );
    IM[2][1] = det * ( _M[0][3]*_M[2][1]*_M[3][0] - _M[0][1]*_M[2][3]*_M[3][0] - _M[0][3]*_M[2][0]*_M[3][1] + _M[0][0]*_M[2][3]*_M[3][1] + _M[0][1]*_M[2][0]*_M[3][3] - _M[0][0]*_M[2][1]*_M[3][3] );
    IM[2][2] = det * ( _M[0][1]*_M[1][3]*_M[3][0] - _M[0][3]*_M[1][1]*_M[3][0] + _M[0][3]*_M[1][0]*_M[3][1] - _M[0][0]*_M[1][3]*_M[3][1] - _M[0][1]*_M[1][0]*_M[3][3] + _M[0][0]*_M[1][1]*_M[3][3] );
    IM[2][3] = det * ( _M[0][3]*_M[1][1]*_M[2][0] - _M[0][1]*_M[1][3]*_M[2][0] - _M[0][3]*_M[1][0]*_M[2][1] + _M[0][0]*_M[1][3]*_M[2][1] + _M[0][1]*_M[1][0]*_M[2][3] - _M[0][0]*_M[1][1]*_M[2][3] );
    IM[3][0] = det * ( _M[1][2]*_M[2][1]*_M[3][0] - _M[1][1]*_M[2][2]*_M[3][0] - _M[1][2]*_M[2][0]*_M[3][1] + _M[1][0]*_M[2][2]*_M[3][1] + _M[1][1]*_M[2][0]*_M[3][2] - _M[1][0]*_M[2][1]*_M[3][2] );
    IM[3][1] = det * ( _M[0][1]*_M[2][2]*_M[3][0] - _M[0][2]*_M[2][1]*_M[3][0] + _M[0][2]*_M[2][0]*_M[3][1] - _M[0][0]*_M[2][2]*_M[3][1] - _M[0][1]*_M[2][0]*_M[3][2] + _M[0][0]*_M[2][1]*_M[3][2] );
    IM[3][2] = det * ( _M[0][2]*_M[1][1]*_M[3][0] - _M[0][1]*_M[1][2]*_M[3][0] - _M[0][2]*_M[1][0]*_M[3][1] + _M[0][0]*_M[1][2]*_M[3][1] + _M[0][1]*_M[1][0]*_M[3][2] - _M[0][0]*_M[1][1]*_M[3][2] );
    IM[3][3] = det * ( _M[0][1]*_M[1][2]*_M[2][0] - _M[0][2]*_M[1][1]*_M[2][0] + _M[0][2]*_M[1][0]*_M[2][1] - _M[0][0]*_M[1][2]*_M[2][1] - _M[0][1]*_M[1][0]*_M[2][2] + _M[0][0]*_M[1][1]*_M[2][2] );

    return IM;
#endif
}

mat2
Inverse(
    const mat2 lInMat2 )
{
#ifdef D_PLATFORM_GLSL
    return inverse( lInMat2 );
#else
    float det = determinant( lInMat2 );
    det = 1.0f / det;

    mat2 _M = lInMat2;
    mat2 IM;
    IM[0][0] = det * _M[1][1];
    IM[1][1] = det * _M[0][0];
    IM[0][1] = det * -_M[1][0];
    IM[1][0] = det * -_M[0][1];

    return IM;
#endif
}

float
lengthSquared( vec2 lInVec2 )
{
    return dot( lInVec2, lInVec2 );
}

float
lengthSquared( vec3 lInVec3 )
{
    return dot( lInVec3, lInVec3 );
}

half
AsinPolynomialApproximationHalf(
    half lInput )
{
    const half a = 1.0 / 6.0;
    const half b = 3.0 / 40.0;
    const half c = 15.0 / ( 48.0 * 7.0 );

    half lInputSquared = lInput * lInput;
    half lInputQuad = lInputSquared * lInputSquared;

    half lOutput = lInput;
    lOutput += a * lInputSquared * lInput;
    lOutput += b * lInputQuad * lInput;
    lOutput += c * lInputQuad * lInputSquared * lInput;

    return lOutput;
}

float
lengthSquared( vec4 lInVec4 )
{
    return dot( lInVec4, lInVec4 );
}

float
AsinPolynomialApproximation( float lInput )
{
    const float a = 1.0 / 6.0;
    const float b = 3.0 / 40.0;
    const float c = 15.0 / ( 48.0 * 7.0 );

    #if defined ( D_PLATFORM_SWITCH )
    float lInputSquared = lInput * lInput;
    float lInputQuad = lInputSquared * lInputSquared;

    float lOutput = lInput;
    lOutput += a * lInputSquared * lInput;
    lOutput += b * lInputQuad * lInput;
    lOutput += c * lInputQuad * lInputSquared * lInput;

    return lOutput;
    #else
    return lInput + a * pow( lInput, 3 ) + b * pow( lInput, 5 ) + c * pow( lInput, 7 );
    #endif
}

float
SigmoidFunction(
    float lfInput,
    float lfCentroid,
    float lfSlopePower )
{
    return saturate( 1.0 / ( 1.0 + exp( -lfSlopePower * ( lfInput - lfCentroid ) ) ) );
}

#if defined ( D_PLATFORM_SWITCH )

float
RSqrtApproximationStep(
    float lfInput,
    const int lfConstant )
{
    int liValue = floatBitsToInt( lfInput );
    liValue = lfConstant - ( liValue >> 1 );
    float lfResult = intBitsToFloat( liValue );

    return lfResult;
}

float
RSqrtApproximation(
    float lfInput )
{
    const float kfHalf = 0.5;
    const float kfOneAndHalf = 1.5;

    lfInput = kfHalf * lfInput;
    float lfResult = RSqrtApproximationStep( lfInput, 0x5F375A86 );
    lfResult *= kfOneAndHalf - ( kfHalf * lfResult * lfResult );

    return lfResult;
}

#endif // defined ( D_PLATFORM_SWITCH )

vec3
NormaliseApproximation(
    vec3 lvInput )
{
#if defined ( D_PLATFORM_SWITCH )

    float lfSquaredMagnitude = dot( lvInput, lvInput );
    float lfRcpMagnitude = RSqrtApproximation( lfSquaredMagnitude );

    vec3 lvResult = lvInput * lfRcpMagnitude;
    return lvResult;

#else
    return normalize(lvInput);
#endif
}

half3
NormaliseApproximationHalf(
    half3 lvInput )
{
#if defined ( D_PLATFORM_SWITCH )

    half lfSquaredMagnitude = dot( lvInput, lvInput );
    half lfRcpMagnitude = RSqrtApproximation( lfSquaredMagnitude );

    half3 lvResult = lvInput * lfRcpMagnitude;
    return lvResult;

#else
    return normalize( lvInput );
#endif
}

//TF_BEGIN
vec4 Unpack_8_8_8_8(uint p)
{
	return vec4(float(p & 0x000000FF) / 255.0,
		float((p & 0x0000FF00) >> 8) / 255.0,
		float((p & 0x00FF0000) >> 16) / 255.0,
		float((p & 0xFF000000) >> 24) / 255.0
	);
}

vec4 Unpack_2_10_10_10(uint p)
{
	vec4 lOut;
	uint liW = p >> 30;
	switch (liW)
	{
	case 0:
		lOut.w = 0.0;
		break;
	case 1:
		lOut.w = 1.0;
		break;
	case 2:
	case 3:
		lOut.w = -1.0;
		break;
	};

	int liX = int(p & 0x3ff);
	int liY = int((p >> 10) & 0x3ff);
	int liZ = int((p >> 20) & 0x3ff);

	if ((liX & 0x200) != 0)
	{
		liX |= 0xfffffc00;
	}
	lOut.x = float(liX / 511.0);

	if ((liY & 0x200) != 0)
	{
		liY |= 0xfffffc00;
	}
	lOut.y = float(liY / 511.0);

	if ((liZ & 0x200) != 0)
	{
		liZ |= 0xfffffc00;
	}
	lOut.z = float(liZ / 511.0);

	return lOut;
}
//TF_END

#endif
