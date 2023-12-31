////////////////////////////////////////////////////////////////////////////////
///
///     @file       SkyFragment.h
///     @author     User
///     @date       
///
///     @brief      SkyFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#define D_USE_NOISETEXTURE
#define D_NO_MATRIX_MULTIPLY

#ifdef D_PLATFORM_ORBIS
#pragma argument(targetoccupancy_atallcosts=80)
#pragma argument(unrollallloops)
#endif

//-----------------------------------------------------------------------------
//      Include files

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif
#include "Common/Defines.shader.h"

#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
#endif

#include "Common/CommonUniforms.shader.h"
//TF_BEGIN
#include "Common/CommonUtils.shader.h"
//TF_END
#include "Custom/SkyCommon.h"
#include "Common/CommonGBuffer.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonScattering.shader.h"
#include "Common/CommonNoise.shader.h"
#include "Common/ACES.shader.h"

//-----------------------------------------------------------------------------
//      Global Data
#if defined(D_LENSFLARE)
//TF_BEGIN
STATIC_CONST float  kfThreshold_0 = 3.00;
STATIC_CONST float  kfGain_0 = 0.25;
STATIC_CONST float  kfSunCoeff = 0.10;
STATIC_CONST vec3   kLumcoeff       = vec3(0.15,0.25,0.6);

vec3 
Threshold(
    in vec3  lColour,
    in float lfThreshold,
    in float lfGain )
{
    float lum    = dot(lColour, kLumcoeff);
    float thresh = max((lum-lfThreshold) * lfGain, 0.0);
    return lColour * thresh / ( 1.0 + thresh );
}
//TF_END
#endif

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec4, mWorldPositionVec4, TEXCOORD0)
INPUT(vec4, mLocalPositionVec4, TEXCOORD1)
INPUT(vec4, mScreenSpacePositionVec4, TEXCOORD2)
#if defined( D_OUTPUT_MOTION_VECTORS )
INPUT(vec4, mPrevScreenSpacePositionVec4, TEXCOORD3)
#endif

#if defined ( D_OUTPUT_MOTION_VECTORS ) && defined( D_SKY_CUBE )
    INPUT_VARIANT( vec4, mPrevScreenPosition, TEXCOORD17, HAS_MOTION_VECTORS )
#endif
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//    Functions

// These booleans are useful for validating the efficacy of
// the short-circuit optimisation this shader has (the tradeoffs in number
// of branches & the warps they do/don't speedup).
STATIC_CONST bool  kfDisableShortCircuitOptimisation = false; 
STATIC_CONST bool  kfShowDebugThresholdColours       = false; 
#if defined( D_PLATFORM_SWITCH )
STATIC_CONST float kfShortCircuitThreshold           = (1.0 / 256.0);
#else
STATIC_CONST float kfShortCircuitThreshold           = (1.0 / 256.0);
#endif

float CalculateFBMNoise(vec3 lPosition, float lfNoiseStrength, SAMPLER2DARG(lNoiseMap))
{
    if ((lfNoiseStrength > kfShortCircuitThreshold) || kfDisableShortCircuitOptimisation)
    {
        return (FractBrownianMotion6(lPosition, SAMPLER2DPARAM(lNoiseMap)) - 0.5) * 2.0 * lfNoiseStrength;
    }
    return 0.0;
}

float CalculateTurbulenceNoise(vec3 lPosition, float lfNoiseStrength, SAMPLER2DARG(lNoiseMap))
{
    if ((lfNoiseStrength > kfShortCircuitThreshold) || kfDisableShortCircuitOptimisation)
    {
        float lfTurbulenceNoise = Turbulence6(lPosition, SAMPLER2DPARAM(lNoiseMap));
        return lfTurbulenceNoise * lfNoiseStrength;
    }
    return 0.0;
}

float CalculateRidgeNoise(vec3 lPosition, float lfNoiseStrength, SAMPLER2DARG(lNoiseMap))
{
    if ((lfNoiseStrength > kfShortCircuitThreshold) || kfDisableShortCircuitOptimisation)
    {
        float lfRidgeNoise = Turbulence6(lPosition, SAMPLER2DPARAM(lNoiseMap));
        lfRidgeNoise = 1.0 - lfRidgeNoise;
        lfRidgeNoise *= lfRidgeNoise;
        lfRidgeNoise *= lfRidgeNoise;
        lfRidgeNoise *= lfRidgeNoise;

        return lfRidgeNoise * lfNoiseStrength;
    }
    return 0.0;
}

float SmoothPeak(float lfPeakValue, float lfInvWidth, float lfCurrentValue)
{
    float lfRangeValue = 1.0 - clamp(abs(lfPeakValue - lfCurrentValue) * lfInvWidth, 0.0, 1.0);
    return smoothstep(0.0, 1.0, lfRangeValue);
}


#if defined( D_SKY_CLEAR )


#if (defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS )) 
#pragma PSSL_target_output_format(target 1 FMT_UNORM16_ABGR)
#endif

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main Fragment
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------


FRAGMENT_MAIN_COLOUR_SRT
{
//#if !defined( D_PLATFORM_SWITCH )
    vec3  lLocalPositionVec3 = normalize(IN(mLocalPositionVec4).xyz);
    float lfNebulaSeed = lUniforms.mpCustomPerMaterial.gSpaceCloudColourVec4.a;

    float lfDistortionNoise;
    lfDistortionNoise = FractBrownianMotion4(lLocalPositionVec3 * 2.0 + float2vec3(lfNebulaSeed + 100.0), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
    float lfSkyBoxMask = smoothstep(0.0, 0.4, lfDistortionNoise + 0.20);

    // Unpack parameters variables
    float lfNebulaTendrilStrength = mix(0.0, lUniforms.mpCustomPerMaterial.gSpaceNebulaColour1Vec4.a, 1.0 - lfSkyBoxMask);
    float lfNebulaWispyness0 = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour2Vec4.a;
    float lfNebulaWispyness1 = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour3Vec4.a;

    float lfNebulaWispyness = mix(lfNebulaWispyness0, lfNebulaWispyness1, lfSkyBoxMask);

    float lfNebulaSparseness = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.r;
    float lfNebulaNoiseFrequency = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.g;
    float lfCloudNoiseFrequency = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.b;
    float lfNebulaFrequency = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.a;

    float lfNebulaFogAmount = mix(0.0, lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.r, 1.0 - lfSkyBoxMask);
    float lfNebulaBrightness = lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.g;
    float lfNebulaCloudStrength0 = lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.b;
    float lfNebulaCloudStrength1 = lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.a;

    float lfNebulaCloudStrength = mix(lfNebulaCloudStrength0, lfNebulaCloudStrength1, lfSkyBoxMask);

    float lfNebulaFBMStrength0 = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.r;
    float lfNebulaFBMStrength1 = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.g;
    float lfNebulaTurbulenceStrength = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.b;
    float lfNebulaDistortionStrength = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.a;

    float lfNebulaFBMStrength = mix(lfNebulaFBMStrength0, lfNebulaFBMStrength1, 1.0 - lfSkyBoxMask);


    float lfSkyBoxValue;
    float lfNebulaFog;
    {
        vec3 lInputVec3 = lLocalPositionVec3;
        vec3 lCubeMapSampleDir = lInputVec3 * vec3(-1.0, 1.0, -1.0);

        {
            lCubeMapSampleDir = normalize(lCubeMapSampleDir + (vec3(lfDistortionNoise, (-lfDistortionNoise * 1.5) - 1.0, lfDistortionNoise * lfDistortionNoise * 2.0) * lfNebulaDistortionStrength));
        }

        lfSkyBoxValue = 1.0;

        lfSkyBoxValue *= FractBrownianMotion6(lCubeMapSampleDir * lfNebulaFrequency + float2vec3(lfNebulaSeed), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));

        float lfFogMultiplier = 0.1 * lfNebulaFogAmount;
        // Having a little bit of fog can produce a nice effect in some circumstances.
        lfNebulaFog = smoothstep(lfNebulaSparseness - (lfNebulaSparseness * lfNebulaFogAmount), lfNebulaSparseness, lfSkyBoxValue) * lfFogMultiplier;

        lfSkyBoxValue = max((lfSkyBoxValue - lfNebulaSparseness) / ((lfNebulaSparseness + ((1.0 - lfNebulaSparseness) * lfNebulaWispyness)) - lfNebulaSparseness), 0.0);
        lfSkyBoxValue *= lfSkyBoxValue;
    }
#if !defined( D_PLATFORM_SWITCH )
    FRAGMENT_COLOUR0 = vec4(0.0, 0.0, 0.0, 0.0);
#if defined( D_OUTPUT_MOTION_VECTORS )
    vec2 lScreenSpaceMotionVec2 = IN(mPrevScreenSpacePositionVec4).xy / IN(mPrevScreenSpacePositionVec4).w - IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w;
    FRAGMENT_COLOUR1 = vec4(EncodeMotion(lScreenSpaceMotionVec2), 0.0, 0.0);
#else
    FRAGMENT_COLOUR1 = vec4(1.0, 1.0, 1.0, 1.0);
#endif
#endif
    FRAGMENT_COLOUR2 = vec4(lfDistortionNoise, lfSkyBoxValue, lfNebulaFog, 0.0);
//#endif
    FRAGMENT_COLOUR3 = vec4(float(D_UNLIT) / 255.0, 0.0, 0.0, 0.0);
}

#elif defined( D_SCRATCHPAD )

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main Fragment
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------

#include "Common/CommonLighting.shader.h"

FRAGMENT_MAIN_COLOUR_SRT
{

    mat3 lUpMatrix = GetInverseWorldUpTransform(lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4);

    vec3 lTransformedNormalVec3 = MUL(lUpMatrix, -normalize(IN(mWorldPositionVec4).xyz));

    vec3 SpecularIBL = GetImageBasedReflectionLighting(SAMPLER2DPARAM_SRT(
        lUniforms.mpCustomPerMaterial,gDualPMapBack),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gDualPMapFront),
        lTransformedNormalVec3,
        0.0);

    FRAGMENT_COLOUR = vec4(SpecularIBL, 1.0);
}

#elif defined(D_SKY_CUBE)
//TF_BEGIN

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main Fragment
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if defined ( D_OUTPUT_MOTION_VECTORS )
FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(HAS_MOTION_VECTORS)
#elif defined(D_BLOOM) || defined(D_LENSFLARE)
FRAGMENT_MAIN_COLOUR01_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{
	vec4 lFragmentColourVec4 = float2vec4(1.0);
	vec3 lTexCoordVec3 = IN(mLocalPositionVec4).xyz;
	vec3 lNebulaAndStarsVec3 = textureCube(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gSkyCubeMap), lTexCoordVec3).rgb;

	vec3  lLightPositionVec3 = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz;
	vec3  lLightRightVec3 = normalize(cross(lLightPositionVec3, vec3(0.0, 1.0, 0.0)));
	vec3  lLightUpVec3 = cross(lLightPositionVec3, lLightRightVec3);
	vec3  lLightPosition2Vec3 = normalize(lLightPositionVec3 + float2vec3(0.225) * lLightUpVec3);
	vec3  lLightPosition3Vec3 = normalize(lLightPositionVec3 + float2vec3(0.225) * lLightRightVec3);

	vec3  lViewPositionVec3 = float2vec3(0.0);
	vec3  lWorldPositionVec3 = normalize(IN(mWorldPositionVec4).xyz - lUniforms.mpPerFrame.gViewPositionVec3);
	vec3  lLocalPositionVec3 = normalize(IN(mLocalPositionVec4).xyz);

	float lfAtmosphereSize = lUniforms.mpCustomPerMaterial.gScatteringParamsVec4.b;

	vec3  lSunWavelengthVec3 = (lUniforms.mpCommonPerMesh.gLightColourVec4.rgb);

	vec3  lSpaceSkyColour1Vec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour1Vec4.rgb;
	float lfStarsNum = lUniforms.mpCustomPerMaterial.gSpaceSkyColour1Vec4.a;
	vec3  lSpaceSkyColour2Vec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour2Vec4.rgb;
	vec3  lSpaceSkyColour3Vec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour3Vec4.rgb;

	float lfSpaceSunSize = lUniforms.mpCustomPerMaterial.gSpaceSkyColour3Vec4.a;
	float lfSpaceCenterPowerVec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour2Vec4.a;

	vec3  lSunColour = lUniforms.mpCustomPerMaterial.gSpaceSunColourVec4.rgb;

	float lfStarWeight = 0.9 / lfStarsNum;

	//Lighting
	lFragmentColourVec4.rgb = saturate(GetSpaceColour(
		lWorldPositionVec3,
		lViewPositionVec3,
		lLightPositionVec3,
		lSpaceSkyColour1Vec3,
		lSpaceSkyColour2Vec3,
		lSpaceSkyColour3Vec3,
		lfSpaceCenterPowerVec3)) * lfStarWeight;

	if (lfStarsNum > 1.0)
		lFragmentColourVec4.rgb += saturate(GetSpaceColour(
			lWorldPositionVec3,
			lViewPositionVec3,
			lLightPosition2Vec3,
			lSpaceSkyColour2Vec3.zxy,
			lSpaceSkyColour1Vec3.zyx,
			lSpaceSkyColour3Vec3.yxz,
			lfSpaceCenterPowerVec3)) * lfStarWeight;

	if (lfStarsNum > 2.0)
		lFragmentColourVec4.rgb += saturate(GetSpaceColour(
			lWorldPositionVec3,
			lViewPositionVec3,
			lLightPosition3Vec3,
			lSpaceSkyColour2Vec3.zyx,
			lSpaceSkyColour3Vec3.xzy,
			lSpaceSkyColour1Vec3.zyx,
			lfSpaceCenterPowerVec3)) * lfStarWeight;


	vec3 lSunScatteringVec3 = float2vec3(0.0);

	if (lfStarsNum <= 1.0)
	{
		lSunScatteringVec3 += InScatteringPhase(
			float2vec3(1.0),
			lSunColour,
			0.8,
			1.0,
			MiePhasePN(lLightPositionVec3, lWorldPositionVec3, lfSpaceSunSize));
	}
	else
	{
		vec3 lHSVColour = RGBToHSV(lSunColour * lSunColour * lSunColour);

		// Sun colouring
		lSunScatteringVec3 += InScatteringPhase(
			float2vec3(1.0),
			HSVToRGB(vec3(fract(lHSVColour.r), lHSVColour.gb)),
			0.9,
			1.0,
			MiePhasePN(lLightPositionVec3, lWorldPositionVec3, lfSpaceSunSize));

		lSunScatteringVec3 += InScatteringPhase(
			float2vec3(1.0),
			HSVToRGB(vec3(fract(lHSVColour.r + 1.0 / lfStarsNum), lHSVColour.gb)),
			0.7,
			1.0,
			MiePhasePN(lLightPosition2Vec3, lWorldPositionVec3, lfSpaceSunSize));

		if (lfStarsNum > 2.0)
			lSunScatteringVec3 += InScatteringPhase(
				float2vec3(1.0),
				HSVToRGB(vec3(fract(lHSVColour.r + 2.0 / lfStarsNum), lHSVColour.gb)),
				0.3,
				1.0,
				MiePhasePN(lLightPosition3Vec3, lWorldPositionVec3, lfSpaceSunSize));
	}

	lFragmentColourVec4.rgb += lNebulaAndStarsVec3;

	lFragmentColourVec4.rgb += clamp(lSunScatteringVec3, float2vec3(0.0), float2vec3(64.0));

#if defined(D_BLOOM)
    float lfGlowFactor = 0.0f;
    vec3  lBrightColourVec3 = lFragmentColourVec4.xyz;
    lBrightColourVec3 = clamp(lBrightColourVec3, float2vec3(0.0), float2vec3(1024.0));

    float lfThreshold       = min( lUniforms.mpCustomPerMaterial.gHDRParamsVec4.y, 1.0 - lfGlowFactor );
    float lfGain            = dot( lBrightColourVec3, lBrightColourVec3 );
    lfGain                 /= lUniforms.mpCustomPerMaterial.gHDRParamsVec4.x * lUniforms.mpCustomPerMaterial.gHDRParamsVec4.x;

    if (lfGain <= lfThreshold * lfThreshold)
            lBrightColourVec3 = float2vec3(0.0);
    else lBrightColourVec3 = BrightThreshold(lBrightColourVec3, lfThreshold, invsqrt(lfGain));

    float lBloomLum = P3D65_TO_Y(lBrightColourVec3);
#endif

#if defined(D_LENSFLARE)
    vec3 lBrightColourFlareVec3  = Threshold( lFragmentColourVec4.rgb, kfThreshold_0, kfGain_0 ) * kfSunCoeff;
	float lLensflareLum = P3D65_TO_Y(lBrightColourFlareVec3);
#endif

#if defined ( D_OUTPUT_MOTION_VECTORS ) || defined(D_BLOOM) || defined(D_LENSFLARE)
	FRAGMENT_COLOUR0 = lFragmentColourVec4;
#else
	FRAGMENT_COLOUR = lFragmentColourVec4;
#endif

#if defined(D_BLOOM) || defined(D_LENSFLARE)
	FRAGMENT_COLOUR1 = vec4(
#if defined(D_BLOOM)
		lBloomLum,
#else
		0.0,
#endif
#if defined(D_LENSFLARE)
		lLensflareLum,
#else
		0.0,
#endif
		0.0,
		1.0);
#endif

#if defined ( D_OUTPUT_MOTION_VECTORS )
#ifdef D_PLATFORM_METAL
    if(HAS_MOTION_VECTORS)
#endif
    {
        vec4 lScreenSpacePositionVec4 = IN(mScreenSpacePositionVec4);
        vec4 lPrevScreenPositionVec4 = IN(mPrevScreenPosition);
        vec2 lScreenSpaceMotionVec4 = (lPrevScreenPositionVec4.xy / lPrevScreenPositionVec4.w - lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w);
        FRAGMENT_COLOUR2 = vec4(lScreenSpaceMotionVec4, 0, 0);
    }
#endif

}

//TF_END
#else

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main Fragment
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    float lfStarsNum = lUniforms.mpCustomPerMaterial.gSpaceSkyColour1Vec4.a;
    vec3  lLightPositionVec3 = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz;
    vec3  lLightRightVec3 = normalize(cross(lLightPositionVec3, vec3(0.0, 1.0, 0.0)));
    vec3  lLightUpVec3 = cross(lLightPositionVec3, lLightRightVec3);
    vec3  lLightPosition2Vec3 = normalize(lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz + float2vec3(0.225) * lLightUpVec3);
    vec3  lLightPosition3Vec3 = normalize(lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz + float2vec3(0.225) * lLightRightVec3);

    vec3  lViewPositionVec3 = float2vec3(0.0);
    vec3  lWorldPositionVec3 = normalize(IN(mWorldPositionVec4).xyz - lUniforms.mpPerFrame.gViewPositionVec3);
    vec3  lLocalPositionVec3 = normalize(IN(mLocalPositionVec4).xyz);

    // Get params from uniforms
    float lfAtmosphereSize = lUniforms.mpCustomPerMaterial.gScatteringParamsVec4.b;


    vec3  lSunWavelengthVec3 = (lUniforms.mpCommonPerMesh.gLightColourVec4.rgb);


    vec3  lSpaceSkyColour1Vec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour1Vec4.rgb;
    vec3  lSpaceSkyColour2Vec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour2Vec4.rgb;
    vec3  lSpaceSkyColour3Vec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour3Vec4.rgb;

    float lfSpaceSunSize = lUniforms.mpCustomPerMaterial.gSpaceSkyColour3Vec4.a;
    float lfSpaceCenterPowerVec3 = lUniforms.mpCustomPerMaterial.gSpaceSkyColour2Vec4.a;

    vec3  lSunColour = lUniforms.mpCustomPerMaterial.gSpaceSunColourVec4.rgb;

    float lfStarWeight = 0.9 / lfStarsNum;

    vec4 lFragmentColourVec4 = vec4(0.0, 0.0, 0.0, 1.0);

#ifndef D_SKY_BAKE
    lFragmentColourVec4.rgb = saturate(GetSpaceColour(
        lWorldPositionVec3,
        lViewPositionVec3,
        lLightPositionVec3,
        lSpaceSkyColour1Vec3,
        lSpaceSkyColour2Vec3,
        lSpaceSkyColour3Vec3,
        lfSpaceCenterPowerVec3)) * lfStarWeight;

    if (lfStarsNum > 1.0)
    lFragmentColourVec4.rgb += saturate(GetSpaceColour(
        lWorldPositionVec3,
        lViewPositionVec3,
        lLightPosition2Vec3,
        lSpaceSkyColour2Vec3.zxy,
        lSpaceSkyColour1Vec3.zyx,
        lSpaceSkyColour3Vec3.yxz,
        lfSpaceCenterPowerVec3)) * lfStarWeight;

    if (lfStarsNum > 2.0)
    lFragmentColourVec4.rgb += saturate(GetSpaceColour(
        lWorldPositionVec3,
        lViewPositionVec3,
        lLightPosition3Vec3,
        lSpaceSkyColour2Vec3.zyx,
        lSpaceSkyColour3Vec3.xzy,
        lSpaceSkyColour1Vec3.zyx,
        lfSpaceCenterPowerVec3)) * lfStarWeight;

    vec3 lSunScatteringVec3 = float2vec3(0.0);

    if (lfStarsNum <= 1.0)
    {
        lSunScatteringVec3 += InScatteringPhase(
            float2vec3(1.0),
            lSunColour,
            0.8,
            1.0,
            MiePhasePN(lLightPositionVec3, lWorldPositionVec3, lfSpaceSunSize));
    }
    else
    {
        vec3 lHSVColour = RGBToHSV(lSunColour * lSunColour * lSunColour);

        // Sun colouring
        lSunScatteringVec3 += InScatteringPhase(
            float2vec3(1.0),
            HSVToRGB(vec3(fract(lHSVColour.r), lHSVColour.gb)),
            0.9,
            1.0,
            MiePhasePN(lLightPositionVec3, lWorldPositionVec3, lfSpaceSunSize));

        lSunScatteringVec3 += InScatteringPhase(
            float2vec3(1.0),
            HSVToRGB(vec3(fract(lHSVColour.r + 1.0 / lfStarsNum), lHSVColour.gb)),
            0.7,
            1.0,
            MiePhasePN(lLightPosition2Vec3, lWorldPositionVec3, lfSpaceSunSize));

        if (lfStarsNum > 2.0)
        lSunScatteringVec3 += InScatteringPhase(
            float2vec3(1.0),
            HSVToRGB(vec3(fract(lHSVColour.r + 2.0 / lfStarsNum), lHSVColour.gb)),
            0.3,
            1.0,
            MiePhasePN(lLightPosition3Vec3, lWorldPositionVec3, lfSpaceSunSize));
    }
#endif

    vec3 lfDebugThresholdColours = vec3(0.0, 0.0, 0.0);

    {
        vec3  lSpaceCloudColourVec3 = lUniforms.mpCustomPerMaterial.gSpaceCloudColourVec4.rgb;
        vec3  lSpaceNebulaColour1Vec3 = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour1Vec4.rgb;
        vec3  lSpaceNebulaColour2Vec3 = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour2Vec4.rgb;
        vec3  lSpaceNebulaColour3Vec3 = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour3Vec4.rgb;
        vec3  lSpaceNebulaColour4Vec3 = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.rgb;

        float lfNebulaSeed = lUniforms.mpCustomPerMaterial.gSpaceCloudColourVec4.a;

        vec2 lFragCoordsVec2 = SCREENSPACE_AS_RENDERTARGET_UVS((IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w) * 0.5 + 0.5);

#ifdef D_SKY_FORWARD
		//TF_BEGIN
		float lfDistortionNoise = FractBrownianMotion4(lLocalPositionVec3 * 2.0 + float2vec3(lfNebulaSeed + 100.0), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
		//TF_END
#else
#ifdef D_PLATFORM_ORBIS
        vec3 lDistortionNoise_SkyBoxValue_NebulaFog = texture2DArray(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gBufferMap), vec3(lFragCoordsVec2.xy, lUniforms.mpPerFrame.gVREyeInfoVec3.x)).xyz;
#else
        vec3 lDistortionNoise_SkyBoxValue_NebulaFog = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gBufferMap), lFragCoordsVec2.xy).xyz;
#endif

        float lfDistortionNoise = lDistortionNoise_SkyBoxValue_NebulaFog.x;
#endif
        float lfSkyBoxMask = smoothstep(0.0, 0.4, lfDistortionNoise + 0.20);

        // Unpack parameters variables
        float lfNebulaTendrilStrength    = mix(0.0, lUniforms.mpCustomPerMaterial.gSpaceNebulaColour1Vec4.a, 1.0 - lfSkyBoxMask);
        float lfNebulaWispyness0         = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour2Vec4.a;
        float lfNebulaWispyness1         = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour3Vec4.a;

        float lfNebulaWispyness          = mix(lfNebulaWispyness0, lfNebulaWispyness1, lfSkyBoxMask);

        float lfNebulaSparseness         = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.r;
        float lfNebulaNoiseFrequency     = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.g;
        float lfCloudNoiseFrequency      = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.b;
        float lfNebulaFrequency          = lUniforms.mpCustomPerMaterial.gSpaceNebulaColour4Vec4.a;

        float lfNebulaFogAmount          = mix(0.0, lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.r, 1.0 - lfSkyBoxMask);
        float lfNebulaBrightness         = lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.g;
        float lfNebulaCloudStrength0     = lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.b;
        float lfNebulaCloudStrength1     = lUniforms.mpCustomPerMaterial.gSpaceNebulaParamsVec4.a;

        float lfNebulaCloudStrength      = mix(lfNebulaCloudStrength0, lfNebulaCloudStrength1, lfSkyBoxMask);

        float lfNebulaFBMStrength0       = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.r;
        float lfNebulaFBMStrength1       = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.g;
        float lfNebulaTurbulenceStrength = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.b;
        float lfNebulaDistortionStrength = lUniforms.mpCustomPerMaterial.gSpaceNormalParamsVec4.a;

        float lfNebulaFBMStrength        = mix(lfNebulaFBMStrength0, lfNebulaFBMStrength1, 1.0 - lfSkyBoxMask);

        float lfNebula = 0.0;
        vec3  lNebulaColourVec3 = float2vec3(0.0);

#ifdef D_SKY_FORWARD
		//TF_BEGIN
		float lfSkyBoxValue;
		float lfNebulaFog;
        {
			vec3 lCubeMapSampleDir = lLocalPositionVec3 * vec3(-1.0, 1.0, -1.0);
			lCubeMapSampleDir = normalize(lCubeMapSampleDir + (vec3(lfDistortionNoise, (-lfDistortionNoise * 1.5) - 1.0, lfDistortionNoise * lfDistortionNoise * 2.0) * lfNebulaDistortionStrength));

			lfSkyBoxValue = 1.0;
			lfSkyBoxValue *= FractBrownianMotion6(lCubeMapSampleDir * lfNebulaFrequency + float2vec3(lfNebulaSeed), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));

			float lfFogMultiplier = 0.1 * lfNebulaFogAmount;
			// Having a little bit of fog can produce a nice effect in some circumstances.
			lfNebulaFog = smoothstep(lfNebulaSparseness - (lfNebulaSparseness * lfNebulaFogAmount), lfNebulaSparseness, lfSkyBoxValue) * lfFogMultiplier;

			lfSkyBoxValue = max((lfSkyBoxValue - lfNebulaSparseness) / ((lfNebulaSparseness + ((1.0 - lfNebulaSparseness) * lfNebulaWispyness)) - lfNebulaSparseness), 0.0);
			lfSkyBoxValue *= lfSkyBoxValue;
		}
		//TF_END
#else
		float lfSkyBoxValue = lDistortionNoise_SkyBoxValue_NebulaFog.y;
		float lfNebulaFog = lDistortionNoise_SkyBoxValue_NebulaFog.z;
#endif

        {
            const float lfOriginalSkyBoxValue = lfSkyBoxValue;

            float lfCloudDensity = lfSkyBoxValue;
            float lfNebulaDensity = lfSkyBoxValue;
            if (((lfSkyBoxValue + lfNebulaFog )> kfShortCircuitThreshold) || kfDisableShortCircuitOptimisation)
            {
                lfDebugThresholdColours.r += 000.25;
                {

                    vec3 lNoiseSamplePosition = lLocalPositionVec3;
                    lNoiseSamplePosition *= (sin(lUniforms.mpPerFrame.gfTime * 0.020) * 0.1) + (cos(lUniforms.mpPerFrame.gfTime * 0.020) * 0.1) + 1;
                    lNoiseSamplePosition *= 0.35 * 0.5 * 0.5;

                    lfCloudDensity *= 4.0;
                    lfCloudDensity = (lfCloudDensity - 1.0) * 0.33333333;

                    {
                        float lfPositionScaleFactor = 550.0 * lfCloudNoiseFrequency;
                        float lfNoiseScaleFactor = 1.5;
                        float lfNoiseLayeringFactor = SmoothPeak(0.6, 1.0 / 0.55, lfOriginalSkyBoxValue);

                        float lfNoise = CalculateFBMNoise(lNoiseSamplePosition * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
                        lfCloudDensity = (lfCloudDensity + lfNoise);
                    }
                }

                vec3 lNoiseSamplePositionSquareWave = lLocalPositionVec3;
                lNoiseSamplePositionSquareWave *= asin(sin(lUniforms.mpPerFrame.gfTime * 0.010)) * 0.15 + 1.0;

                // Apply high frequency noise
                lNebulaColourVec3 = lSpaceNebulaColour1Vec3 * lfNebulaBrightness * 0.5;
                if((lfNebulaDensity > (1.0/256.0)) || kfDisableShortCircuitOptimisation)
                {
                    lfDebugThresholdColours.g += 000.25;
                    {
                        const float lfPositionScaleFactor = 35.0 * lfNebulaNoiseFrequency;
                        const float lfNoiseScaleFactor = 0.3 * lfNebulaTurbulenceStrength;
                        float lfNoiseLayeringFactor = SmoothPeak(0.6, 1.0 / 0.35, lfOriginalSkyBoxValue);

                        float lfNoise = CalculateTurbulenceNoise(lNoiseSamplePositionSquareWave * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
                        lfNebulaDensity = (lfNebulaDensity + lfNoise);
                    }

                    {
                        const float lfPositionScaleFactor = 50.0 * lfNebulaNoiseFrequency;
                        const float lfNoiseScaleFactor = 0.5 * lfNebulaTurbulenceStrength;
                        float lfNoiseLayeringFactor = SmoothPeak(0.25, 1.0 / 0.30, lfOriginalSkyBoxValue) * lfOriginalSkyBoxValue;

                        float lfNoise = CalculateTurbulenceNoise(lNoiseSamplePositionSquareWave * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
                        lfNebulaDensity = (lfNebulaDensity + lfNoise);
                    }

                    {
                        const float lfPositionScaleFactor = 50.0 * lfNebulaNoiseFrequency;
                        const float lfNoiseScaleFactor = 1.0 * lfNebulaTurbulenceStrength;
                        float lfNoiseLayeringFactor = SmoothPeak(0.1, 1.0 / 0.1, lfOriginalSkyBoxValue) * lfOriginalSkyBoxValue;

                        float lfNoise = CalculateTurbulenceNoise(lNoiseSamplePositionSquareWave * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
                        lfNebulaDensity = (lfNebulaDensity + lfNoise);
                    }

                    {
                        float lfPositionScaleFactor = 200.0 * lfNebulaNoiseFrequency;
                        float lfNoiseScaleFactor = 1.0 * lfNebulaFBMStrength * 0.35;
                        float lfNoiseLayeringFactor = 0.5 - saturate(0.5 - lfNebulaDensity);

                        float lfNoise = CalculateFBMNoise(lNoiseSamplePositionSquareWave * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
                        lfNebulaDensity = max(lfNebulaDensity + lfNoise, 0.0);
                    }

                    {
                        const float lfPositionScaleFactor = 35.0 * lfNebulaNoiseFrequency;
                        const float lfNoiseScaleFactor = lfNebulaTendrilStrength;
                        float lfNoiseLayeringFactor = SmoothPeak(0.6, 1.0 / 0.35, lfOriginalSkyBoxValue);

                        float lfNoise = CalculateRidgeNoise(lNoiseSamplePositionSquareWave * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));
                        lfNebulaDensity = (lfNebulaDensity + lfNoise);
                    }

                    lNebulaColourVec3 = mix(lNebulaColourVec3, lSpaceNebulaColour2Vec3 * lfNebulaBrightness * 0.75, saturate(lfNebulaDensity * 0.3));
                    lNebulaColourVec3 = mix(lNebulaColourVec3, lSpaceNebulaColour3Vec3 * lfNebulaBrightness, saturate((lfNebulaDensity - 0.3) * 0.3));
                }

                // Fade-out nebula emissive components by cloud density
                lfNebulaDensity = lfNebulaDensity / max(1.0, (1.0 + (lfCloudDensity * lfCloudDensity * 22.0 * lfNebulaCloudStrength)));

                lfNebulaDensity = max(lfNebulaDensity, 0.0);

                // Create non-linear response to nebula density for more interesting shapes.
                lfNebulaDensity -= SmoothPeak(0.2, 1.0 / 0.2, lfNebulaDensity) * 0.01;

                if ((lfNebulaFog > kfShortCircuitThreshold) || kfDisableShortCircuitOptimisation)
                {
                    lfDebugThresholdColours.b += 000.25;
                    float lfFogMultiplier = 1.0 - saturate(lfNebulaDensity);

                    {
                        float lfPositionScaleFactor = 10.0 * lfNebulaNoiseFrequency;
                        float lfNoiseScaleFactor = 1.0;
                        float lfNoiseLayeringFactor = 1.0;

                        float lfNoise = CalculateFBMNoise(lNoiseSamplePositionSquareWave * lfPositionScaleFactor, lfNoiseScaleFactor * lfNoiseLayeringFactor, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNoiseMap));

                        lfNebulaFog += lfNoise * lfNebulaFog * 6.0;
                    }

                    lfNebulaDensity += lfNebulaFog * (lfFogMultiplier) * (lfFogMultiplier);
                }

                // Mixin lNebulaColourVec3 to the skybox
                float lfMixStrength = saturate(lfNebulaDensity * 0.85);
                float lfMixProportion = mix(0.0, 1.0, lfMixStrength);

                // We very deliberately do not call "saturate" on lfNebulaDensity so that
                // we can preserve the highlight details in the form of increased brightness where
                // adding layers of noise > 1.0.
                lNebulaColourVec3 *= max(1.0, lfNebulaDensity * 0.85);

                lFragmentColourVec4.rgb += lNebulaColourVec3 * (1.0 - lfMixProportion) * lfMixStrength * 0.9;
                lFragmentColourVec4.rgb = mix(lFragmentColourVec4.rgb, lNebulaColourVec3, lfMixProportion * lfMixStrength * 0.9);


                if ((lfCloudDensity > kfShortCircuitThreshold) || kfDisableShortCircuitOptimisation)
                {

                    lfDebugThresholdColours.r += 000.25;
                    float lfCloudStrength = smoothstep(0.0, 2.0, lfCloudDensity * lfNebulaCloudStrength);
                    lfCloudStrength = 1.0 - lfCloudStrength;
                    lfCloudStrength *= lfCloudStrength;
                    lfCloudStrength *= lfCloudStrength;
                    lfCloudStrength = 1.0 - lfCloudStrength;

                    lFragmentColourVec4.rgb = mix(lFragmentColourVec4.rgb, lSpaceCloudColourVec3, lfCloudStrength);
                }
            }
        }
    }

#ifndef D_SKY_BAKE
    lFragmentColourVec4.rgb += clamp(lSunScatteringVec3, float2vec3(0.0), float2vec3(64.0));
#endif

    if (kfShowDebugThresholdColours)
    {
        lFragmentColourVec4.rgb = lfDebugThresholdColours;
    }

#ifndef D_SKY_FORWARD
    lFragmentColourVec4.a = 0.0; // cause sky to replace what's under it in HDRBUF
#endif

    // treating sky as sRGB helps prevent heavy air from blowing out
    lFragmentColourVec4.rgb = MUL(lFragmentColourVec4.rgb, sRGB_TO_P3D65);
    // mad hack :(
    lFragmentColourVec4.rgb = MUL(lFragmentColourVec4.rgb, sRGB_TO_P3D65);

    FRAGMENT_COLOUR = lFragmentColourVec4;
}

#endif