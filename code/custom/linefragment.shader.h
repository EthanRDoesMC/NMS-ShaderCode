////////////////////////////////////////////////////////////////////////////////
///
///     @file       LineFragment.h
///     @author     User
///     @date       
///
///     @brief      LineFragment
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

#define D_LINE_UNIFORMS
#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Custom/LineCommon.h"

/*float noise(
    in vec3 x,
    in CustomPerMaterialUniforms lCustomUniforms)
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0 - 2.0*f);
    vec2 uv = (p.xy + vec2(37.0, 17.0)*p.z) + f.xy;
    vec2 rg = texture2D(lCustomUniforms.gNormalMap, (uv + 0.5) / 256.0, -100.0).yx;
    return mix(rg.x, rg.y, f.z);
}*/

float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec3 x) {
    const vec3 lNoiseStep = vec3(110, 241, 171);

    vec3 i = floor(x);
    vec3 f = fract(x);

    // For performance, compute the base input to a 1D hash from the integer part of the argument and the 
    // incremental change to the 1D based on the 3D . 1D wrapping
    float n = dot(i, lNoiseStep);

    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(hash(n + dot(lNoiseStep, vec3(0, 0, 0))), hash(n + dot(lNoiseStep, vec3(1, 0, 0))), u.x),
        mix(hash(n + dot(lNoiseStep, vec3(0, 1, 0))), hash(n + dot(lNoiseStep, vec3(1, 1, 0))), u.x), u.y),
        mix(mix(hash(n + dot(lNoiseStep, vec3(0, 0, 1))), hash(n + dot(lNoiseStep, vec3(1, 0, 1))), u.x),
        mix(hash(n + dot(lNoiseStep, vec3(0, 1, 1))), hash(n + dot(lNoiseStep, vec3(1, 1, 1))), u.x), u.y), u.z);
}

float fbm(vec3 x) {
    int NUM_OCTAVES = 1;
    float v = 0.0;
    float a = 0.5;
    vec3 shift = float2vec3(100);
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(x);
        x = x * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

float CalculateAlphaMultiplier(
    float lfStartFadeNear, float lfEndFadeNear,
    float lfStartIncreaseFar, float lfEndIncreaseFar,
    float lfFarBrightnessMutiplier, float lfEndFadeSpeed,
    float lfTimeToEnd,
    vec2 lDistanceAlongLaser)
{
    // This method calculates alpha fades for different purposes for player laser weapons
    // It both fades the alpha across the laser according to how the material uniforms are setup, and
    // can allow the laser to fade towards the tip once it has finished firing.
    lfStartFadeNear += (1.0 - lfTimeToEnd) * lfEndFadeSpeed;
    lfEndFadeNear   += (1.0 - lfTimeToEnd) * lfEndFadeSpeed;

    float lfAlphaNear = saturate( (lDistanceAlongLaser.x - lfStartFadeNear) / (lfEndFadeNear - lfStartFadeNear) );
    lfAlphaNear *= lfAlphaNear;
    lfAlphaNear = lfEndFadeNear == 0.0 ? 1.0 : lfAlphaNear;
    
    float lfAlphaFar;
    {
        float lfFarT = saturate( (lfEndIncreaseFar - lDistanceAlongLaser.y) / (lfStartIncreaseFar - lfEndIncreaseFar) );
        lfFarT = lfFarT * lfFarT;
        lfAlphaFar = lfFarT * lfFarBrightnessMutiplier;
    }
 
    return lfAlphaNear + (lfAlphaFar * lfAlphaNear);
}

float GetDepthFromTexture(
   in PerFrameUniforms          lPerFrameUniforms,
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAYARG(          lTexture ),
#else
    SAMPLER2DARG(               lTexture ),
#endif
	in vec2                        screenPos 
)
{
    #ifdef D_PLATFORM_ORBIS
    return DecodeDepthFromColour( texture2DArray( lTexture, vec3(screenPos,lPerFrameUniforms.gVREyeInfoVec3.x ) ) );
    #else
    return DecodeDepthFromColour( texture2D( lTexture, screenPos));
    #endif
}

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec2,   mTexCoordsVec2,          TEXCOORD0 )
    INPUT( vec4,   mWorldPositionVec4,      TEXCOORD1 )
    INPUT( vec2,   mDistanceAlongLaser,    TEXCOORD3 )
    INPUT( vec4,   mColourVec4,             TEXCOORD4 )

DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lFragmentColourVec4;
    vec4 lTintColourVec4 = IN( mColourVec4 );

    lFragmentColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), IN( mTexCoordsVec2 ).xy );

    vec3 lOriginalHSVVec3 = RGBToHSV(lFragmentColourVec4.xyz);
    vec3 lTintHSVVec3     = RGBToHSV(lTintColourVec4.xyz);

    vec3 lHSVColourVec3 = lOriginalHSVVec3;
    lHSVColourVec3.x = lTintHSVVec3.x;
    if (lTintHSVVec3.y > 0)
    {
        lFragmentColourVec4.rgb = HSVToRGB(lHSVColourVec3);
    }

#ifdef D_SUBSTANCES

    lFragmentColourVec4.a *= saturate( IN( mColourVec4 ).a );

    float lfProgress = pow(lUniforms.mpCommonPerMesh.gScanParamsCfg1Vec4.y, 3.0);
    vec4 lScanConfigPos = lUniforms.mpCommonPerMesh.gScanParamsPosVec4;

    vec3  lWorldToScanPt = IN(mWorldPositionVec4).xyz - lScanConfigPos.xyz;
    float lfDistToScanPt = length(lWorldToScanPt);

    // adjust distance so that values inside the pulse radius fall 0..1
    float lfRadius = 1.0 / lScanConfigPos.w;
    float lfPosOverRadius01 = lfDistToScanPt * lScanConfigPos.w;
    float lfValue = clamp( (lfProgress - lfPosOverRadius01) * 10.0 , 0.0, 1.0);
    lFragmentColourVec4.w *= lfValue;

    // Waves
    float lfHeight = GetHeight(IN(mWorldPositionVec4).xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4);
    float lfWaves = pow(max(0.0, sin(lUniforms.mpCommonPerMesh.gScanParamsCfg1Vec4.w * 3.0 + lfHeight * 0.1)), 5.0);

    // Clusters of cubes
    // Clusters pulse effect
    float lfPulse = clamp(sin(lUniforms.mpCommonPerMesh.gScanParamsCfg1Vec4.w), 0.0, 1.0);
    float lfPulseCore = pow(lfPulse, 50.0);

    // Noise Displacement
    float PI = 3.14159;
    float lfWave1 = lfPulse;
    float lfWave2 = clamp(sin(lUniforms.mpCommonPerMesh.gScanParamsCfg1Vec4.w * 0.5   + PI * 0.25   ), 0.0, 1.0);
    float lfWave3 = clamp(sin(lUniforms.mpCommonPerMesh.gScanParamsCfg1Vec4.w * 0.25  + PI * 0.125  ), 0.0, 1.0);
    //float lfWave4 = clamp(sin(lUniforms.mpPerFrame.gfTime.x * 0.125 + PI * 0.0625 ), 0.0, 1.0);
    vec3 lDisplacement = vec3(normalize(lfWave1), normalize(lfWave2), normalize(lfWave3)) * 20.0;

    // Noise value
    float lfNoise = fbm(IN(mWorldPositionVec4).xyz * 0.2 + lDisplacement);

    float lAlpha = lUniforms.mpCommonPerMesh.gScanParamsCfg2Vec4.x + 
                   saturate(step(lUniforms.mpCommonPerMesh.gScanParamsCfg2Vec4.y, lfNoise) * lfPulse + lfWaves) * lUniforms.mpCommonPerMesh.gScanParamsCfg2Vec4.w;
    
    lFragmentColourVec4.w *= lAlpha;

    //Apply underground fade based on depth test to the alpha
    vec2 screenPos = IN_SCREEN_POSITION.xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    float depthBuf = GetDepthFromTexture(   DEREF_PTR( lUniforms.mpPerFrame ),
                                            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial,gBufferMap ),
										    screenPos
										);
    //Convert ReverseZ to linear depth
    depthBuf = LinearNormToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, depthBuf );

    float depthPix = IN_SCREEN_POSITION.z;
    //Make it a curve
    float depthDiff = 1.0 - saturate(lUniforms.mpCommonPerMesh.gScanParamsCfg2Vec4.z * (depthBuf - depthPix));
    lFragmentColourVec4.w *= lUniforms.mpCommonPerMesh.gScanParamsCfg1Vec4.z * (depthDiff * depthDiff * depthDiff);
    
    lFragmentColourVec4.xyz += step(lUniforms.mpCommonPerMesh.gScanParamsCfg2Vec4.y, lfNoise) * lfPulseCore * lUniforms.mpCommonPerMesh.gScanParamsCfg2Vec4.w * 0.5;

#else
    lFragmentColourVec4.a *= lTintColourVec4.a;

    float lfStartFadeNear           = lUniforms.mpCustomPerMaterial.gFadeDistancesVec4.x;
    float lfEndFadeNear             = lUniforms.mpCustomPerMaterial.gFadeDistancesVec4.y;
    float lfStartIncreaseFar        = lUniforms.mpCustomPerMaterial.gFadeDistancesVec4.z;
    float lfEndIncreaseFar          = lUniforms.mpCustomPerMaterial.gFadeDistancesVec4.w;
    float lfFarBrightnessMutiplier  = lUniforms.mpCustomPerMaterial.gLaserParamsVec4.x;
    float lfEndFadeSpeed            = lUniforms.mpCustomPerMaterial.gLaserParamsVec4.y;
    float lfTextureStrength         = lUniforms.mpCustomPerMaterial.gLaserParamsVec4.z;
    float lfDistortionStrength      = lUniforms.mpCustomPerMaterial.gLaserParamsVec4.w;
    float lfTimeToEnd               = lUniforms.mpCommonPerMesh.gRuntimeLaserParamsVec4.x;
    float lfMinimumDamageLevel      = lUniforms.mpCustomPerMaterial.gTextureParamsVec4.w;
    float lfDamageLevel             = min(lfMinimumDamageLevel, lUniforms.mpCommonPerMesh.gRuntimeLaserParamsVec4.y - 18);

/*     lFragmentColourVec4.xyz *= CalculateAlphaMultiplier(
        lfStartFadeNear,
        lfEndFadeNear,
        lfStartIncreaseFar,
        lfEndIncreaseFar,
        lfFarBrightnessMutiplier,
        lfEndFadeSpeed,
        lfTimeToEnd,
        IN( mDistanceAlongLaser )
    ); */

    if (lfTextureStrength > 0.0)
    {
        float lfFadeDistance = lUniforms.mpCustomPerMaterial.gTextureParamsVec4.x;
        float lfScrollSpeed = lUniforms.mpCustomPerMaterial.gTextureParamsVec4.y;
        float lfNoiseScrollSpeed = lUniforms.mpCustomPerMaterial.gTextureParamsVec4.z;

        float lfDistanceStrength = max( lfFadeDistance - IN( mDistanceAlongLaser ).x, 0.0 ) / lfFadeDistance;
        lfDistanceStrength *= lfDistanceStrength;

        float lfVMultiplier = 1100.0 / (lfDamageLevel * lfDamageLevel + 1.0);
        {
            vec2 lTexLookupUv = vec2( IN( mDistanceAlongLaser ).x * 0.5, IN( mTexCoordsVec2 ).y );
            vec2 lNoiseTexLookupUv = lTexLookupUv * 0.7 + vec2( 0.0, lUniforms.mpPerFrame.gfTime * lfNoiseScrollSpeed );
            vec2 lNoise = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gNoiseMap ), lNoiseTexLookupUv ).xy - vec2(0.5, 0.5);
            lTexLookupUv += lNoise * (lfDistortionStrength * lfDamageLevel);
            lTexLookupUv.x -= lUniforms.mpPerFrame.gfTime * lfScrollSpeed;
            lTexLookupUv.y = clamp((lTexLookupUv.y - 0.5) * lfVMultiplier + 0.5, 0.0, 1.0);
            if (IN( mDistanceAlongLaser ).x > 0.0)
            {
                // We need to clamp the electric effect UVs as it's really noticable otherwise that they don't come out of the gun muzzle in VR
                float lfDistanceSaturated = saturate( IN( mDistanceAlongLaser ).x * 5.0 );
                lTexLookupUv.y -= 0.5;
                lTexLookupUv.y /= lfDistanceSaturated;
                if (abs( lTexLookupUv.y ) <= 0.5)
                {
                    lTexLookupUv.y += 0.5;
                    lFragmentColourVec4.xyz += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gEffectMap ), lTexLookupUv ).xyz * lfTextureStrength * lfDistanceStrength;
                }
            }
        }
    }

    if (IN( mDistanceAlongLaser ).x >= 0.0 && lfFarBrightnessMutiplier > 0.0)
    {
        lFragmentColourVec4.xyz *= CalculateAlphaMultiplier(
            lfStartFadeNear,
            lfEndFadeNear,
            lfStartIncreaseFar,
            lfEndIncreaseFar,
            lfFarBrightnessMutiplier,
            lfEndFadeSpeed,
            lfTimeToEnd,
            IN( mDistanceAlongLaser )
        );
    }

    // As this doesn't use the uber shader at all -  but we need some kind of flag to only enable soft depth fades for laser materials,
    // we use the otherwise unused transparent flag for this shader to enable depth fades (as it sort-of can be used in the same way to
    // enable depth fades for uber shader materials).
    // This is a bit of a hack, but as of writing probably the cleanest way to implement this. All laser materials have this flag set.
    // Tom RC - 2022-03-01
#if defined( _F09_TRANSPARENT )
    float lfLaserSoftFade = 0.5;
    if (lfLaserSoftFade > 0.0)
    {
        vec2 lScreenPos = IN_SCREEN_POSITION.xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;

        float lfDepthBuf  = GetDepthFromTexture(   DEREF_PTR( lUniforms.mpPerFrame ),
                                                   SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial,gBufferMap ),
						   lScreenPos 
						);

        lfDepthBuf        = FastDenormaliseDepth(  lUniforms.mpPerFrame.gClipPlanesVec4, lfDepthBuf );
        float lfDepthThis = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, IN_SCREEN_POSITION.z );

       
        float lfTransParam = lfLaserSoftFade;
        float lfDepthFade = saturate( (lfDepthBuf - lfDepthThis) * lfTransParam );
        lFragmentColourVec4.a *= lfDepthFade;
        lFragmentColourVec4.a = min( lfDepthFade, lFragmentColourVec4.a );
    }
#endif


#endif // D_SUBSTANCES
    FRAGMENT_COLOUR = lFragmentColourVec4;
}