////////////////////////////////////////////////////////////////////////////////
///
///     @file       PostProcessFragment.h
///     @author     User
///     @date       
///
///     @brief      DepthOfFieldFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

//-----------------------------------------------------------------------------
//      Include files

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/CommonScattering.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonGBuffer.shader.h"
#include "LightCommon.shader.h"
#include "Common/CommonLighting.shader.h"
#include "Common/CommonPostProcess.shader.h"

#ifdef D_PLATFORM_ORBIS
#pragma argument(unrollallloops)
#elif defined( D_PLATFORM_OPENGL )
#pragma optionNV(unroll all)
#endif
#ifdef D_PLATFORM_PROSPERO
#pragma argument (O4; fastmath; scheduler=minpressure; targetoccupancy_atallcosts=10)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
//precision mediump float;
#pragma optionNV(fastmath on)
#define D_USE_PACKING
#endif

// =================================================================================================
//
// RAYMARCH
//
// =================================================================================================

#ifdef D_PLATFORM_SWITCH
#define NB_STEPS_LOW
#endif

#ifdef NB_STEPS_LOW
#define NB_STEPS         8
#else
#define NB_STEPS        16
#endif
#define NB_STEPS_RCP    1.0 / float( NB_STEPS )

#define PI      3.1415926535897932384626433832795 

#define TAU     0.0001
#define PHI     10000000.0
// 1.0 / PI
#define PI_RCP  0.31830988618379067153776752674503

#if defined(D_SAMPLERS_ARE_GLOBAL)
float
PCF(
in CustomPerMeshUniforms lPerMaterialUniforms,
in CommonPerMeshUniforms   lMeshUniforms,
in vec3                    lTexCoordVec3,
in vec4                 lShadowMapSize,
in float                lfFilterRadius,
in bool                 lbHQ)
{
#ifdef D_ENABLE_REVERSEZ_PROJECTION
    lTexCoordVec3.z = 1.0 - lTexCoordVec3.z;
#endif
    
    float shadow = shadow2D(gShadowMap, lTexCoordVec3).x;
    if (!lbHQ)
    {
        return shadow;
    }

    vec3 lTexOffsetCoordVec3 = lTexCoordVec3;

    const float offsetx = lfFilterRadius * (lShadowMapSize.z / D_NUMBER_OF_CASCADES);
    const float offsety = lfFilterRadius * (lShadowMapSize.w);

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(-1.0 * offsetx, 0.0);
    shadow += shadow2D(gShadowMap, lTexOffsetCoordVec3).x;

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(1.0 * offsetx, 0.0);
    shadow += shadow2D(gShadowMap, lTexOffsetCoordVec3).x;

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(0.0, -1.0 * offsety);
    shadow += shadow2D(gShadowMap, lTexOffsetCoordVec3).x;

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(0.0, 1.0 * offsety);
    shadow += shadow2D(gShadowMap, lTexOffsetCoordVec3).x;

    return shadow * (1.0 / 5.0);
}

#else

float
PCF(
in CustomPerMeshUniforms lPerMaterialUniforms,
in CommonPerMeshUniforms   lMeshUniforms,
in vec3                    lTexCoordVec3,
in vec4                 lShadowMapSize,
in float                lfFilterRadius,
in bool                 lbHQ)
{
    #ifdef D_ENABLE_REVERSEZ_PROJECTION
        lTexCoordVec3.z = 1.0 - lTexCoordVec3.z;
    #endif
    
    float shadow = shadow2D(lPerMaterialUniforms.gShadowMap, lTexCoordVec3).x;
    if (!lbHQ)
    {
        return shadow;
    }
    vec3 lTexOffsetCoordVec3 = lTexCoordVec3;

    const float offsetx = lfFilterRadius * (lShadowMapSize.z / D_NUMBER_OF_CASCADES);
    const float offsety = lfFilterRadius * (lShadowMapSize.w);

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(-1.0 * offsetx, 0.0);
    shadow += shadow2D(lPerMaterialUniforms.gShadowMap, lTexOffsetCoordVec3).x;

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(1.0 * offsetx, 0.0);
    shadow += shadow2D(lPerMaterialUniforms.gShadowMap, lTexOffsetCoordVec3).x;

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(0.0, -1.0 * offsety);
    shadow += shadow2D(lPerMaterialUniforms.gShadowMap, lTexOffsetCoordVec3).x;

    lTexOffsetCoordVec3.xy = lTexCoordVec3.xy + vec2(0.0, 1.0 * offsety);
    shadow += shadow2D(lPerMaterialUniforms.gShadowMap, lTexOffsetCoordVec3).x;

    return shadow * (1.0 / 5.0);
}

#endif


float
ComputeShadowIntensityPCF(
    in CustomPerMeshUniforms   lPerMaterialUniforms,
    in PerFrameUniforms        lRenderTargetUniforms,
    in CommonPerMeshUniforms   lMeshUniforms,
    in vec3                    lPositionVec3 )
{
    vec3   lProjectedPos;
    float lfTexelSize = lRenderTargetUniforms.gShadowSizeVec4.w; 
    lProjectedPos = MUL(lMeshUniforms.gaShadowMat4[0], vec4(lPositionVec3, 1.0)).xyz;

    // Check if you are outside the high detail shadow cascade
    if (!D_INSIDE_SM_BOUNDS1((lProjectedPos.xyz), lfTexelSize))
    {
        lProjectedPos = MUL(lMeshUniforms.gaShadowMat4[1], vec4(lPositionVec3, 1.0)).xyz;

        if (!D_INSIDE_SM_BOUNDS2((lProjectedPos.xyz), lfTexelSize))
        {
            lProjectedPos = MUL(lMeshUniforms.gaShadowMat4[2], vec4(lPositionVec3, 1.0)).xyz;

            if (!D_INSIDE_SM_BOUNDS3((lProjectedPos.xyz), lfTexelSize))
            {
                // outside all cascades
                return 1.0;
            }
            else
            {   // inside cascade 2
                lProjectedPos.x += 2.0;
                lProjectedPos.x = (lProjectedPos.x * D_CASCADE_SIZE);
                return PCF(lPerMaterialUniforms, lMeshUniforms, lProjectedPos, lRenderTargetUniforms.gShadowSizeVec4, 0.5, false);
            }
        }
        else
        {
            // inside cascade 1
            lProjectedPos.x += 1.0;
        }
    }

    lProjectedPos.x = (lProjectedPos.x * D_CASCADE_SIZE);

    return PCF(lPerMaterialUniforms, lMeshUniforms, lProjectedPos, lRenderTargetUniforms.gShadowSizeVec4, 0.5, true);

}


// Mie scaterring approximated with Henyey-Greenstein phase function.
float
ComputeScattering(
float lfLightDotView,
float lfScattering)
{
    float result = 1.0f - lfScattering;
    result *= result;
    result /= (4.0f * PI * pow(1.0f + lfScattering * lfScattering - (2.0f * lfScattering) * lfLightDotView, 1.5f));
    return result;
}

float
InterleavedGradientNoise(
    uvec2 lvPos )
{
    return fract( 52.9829189 * fract( 0.06711056 * float( lvPos.x ) + 0.00583715 * float( lvPos.y ) ) ) * 16.0 / 17.0;
}

float
InterleavedGradientNoise(
    uvec2   lvPos,
    int     liFrame )
{
    liFrame = liFrame & 0xff;
    float x = float( lvPos.x ) + 5.588238 * float( liFrame );
    float y = float( lvPos.y ) + 5.588238 * float( liFrame );
    return fract( 52.9829189 * fract( 0.06711056 * x + 0.00583715 * y ) ) * 16.0 / 17.0;
}


float
Bayer(
uvec2 lPos)
{
    const mat4 bayer = mat4(
        vec4(1, 9, 3, 11),
        vec4(13, 5, 15, 7),
        vec4(4, 12, 2, 10),
        vec4(16, 8, 14, 6)
        ) / 17.0;

    //vec2 positionMod = vec2( uvec2( tc * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) & 3 );

    uvec2 positionMod = uvec2(lPos & 3);

    float rndoffset = bayer[positionMod.x][positionMod.y];
    //float rndoffset = float(positionMod.x) * 0.25;

    return rndoffset;
}

#if defined( NB_STEPS_LOW )
// Note(sal): if the number of steps is low, to increase quality,
// we sample more towards the end of the march;
// that's cause usually that's where the most discontinuities are
float
InterpolationCurve(
    float lfX )
{
    return 1.0 - ( 1.0 - lfX ) * ( 1.0 - lfX );
}
#else
float
InterpolationCurve(
    float lfX )
{
    return lfX;
}
#endif


vec4 RayMarch(  in vec2 lFragCoordsVec2,
                in vec3 lWorldPosition,
                in CustomPerMeshUniforms    lPerMaterialUniforms,
                in PerFrameUniforms         lRenderTargetUniforms,
                in CommonPerMeshUniforms    lPerMeshUniforms
    )

{
    vec4 lFragCol;

    #if defined(D_PLATFORM_OPENGL) && D_PLATFORM_PC_LOWEND

    // Intel Driver crashes compiling this shader.
    lFragCol = vec4( 0.0 );

    #else

    float lfLightAccumulation = 0.0;

    if ( lPerMaterialUniforms.gLightShaftParamsVec4.y != 0.0 )
    {
        float lfRayLength           = length( lWorldPosition );
        vec3  lRayDirVec3           = lWorldPosition / lfRayLength;
        float lfSunScatter          = ComputeScattering( dot( lRayDirVec3, lPerMaterialUniforms.gSunPositionVec4.xyz ),
                                                         lPerMaterialUniforms.gLightShaftParamsVec4.x );

        if ( lfSunScatter > 0.003 )
        {
            float lfDitherValue;
            {
                #if !defined( D_CHECKERBOARD )
                uvec2 lvPixCoords   = uvec2( lFragCoordsVec2 * lRenderTargetUniforms.gFrameBufferSizeVec4.xy );
                lfDitherValue       = Bayer( lvPixCoords );
                #else
                uvec2 lvOffset      = uvec2( gl_SampleID == ( lRenderTargetUniforms.giFrameIndex & 0x1 ), 0 );
                uvec2 lvPixCoords   = uvec2( lFragCoordsVec2 * lRenderTargetUniforms.gFrameBufferSizeVec4.xy * 2.0 ) / uvec2( 2, 1 );
                lfDitherValue       = InterleavedGradientNoise( lvPixCoords + lvOffset );
                #endif

                lfDitherValue      *= NB_STEPS_RCP;

                #if defined( D_PLATFORM_SWITCH )
                // On Switch dithering can cause quite noticeable pixelation
                // scaling down the dither extent helps hiding the issue
                lfDitherValue      *= 0.75;
                #endif
            }

            int     liCount                 = 0;
            float   lfMarchDist             = lfDitherValue * lfRayLength;
            float   lfShadowLimit           = sqrt( lRenderTargetUniforms.gShadowFadeParamVec4.w );
            vec3    lStartPositionVec3      = lRenderTargetUniforms.gViewPositionVec3;

            for (; liCount < NB_STEPS && lfMarchDist < lfShadowLimit; ++liCount )
            {
                float   lfShadowMapValue;
                vec3    lCurrentPositionVec3;

                lCurrentPositionVec3    = lStartPositionVec3 + lRayDirVec3 * lfMarchDist;
                lfShadowMapValue        = ComputeShadowIntensityPCF( lPerMaterialUniforms, lRenderTargetUniforms, lPerMeshUniforms, lCurrentPositionVec3 );
                lfLightAccumulation    += lfShadowMapValue;
                lfMarchDist             = InterpolationCurve( lfDitherValue + ( liCount + 1 ) * NB_STEPS_RCP ) * lfRayLength;
            }

            // Account for shorter loops when we surpass the shadow limit before reaching the end of the march
            lfLightAccumulation += float( NB_STEPS - liCount );
            lfLightAccumulation  = saturate( lfSunScatter * lfLightAccumulation * NB_STEPS_RCP );
            lfLightAccumulation *= lPerMaterialUniforms.gLightShaftParamsVec4.y;
            lfLightAccumulation  = smoothstep( lPerMaterialUniforms.gLightShaftParamsVec4.z, lPerMaterialUniforms.gLightShaftParamsVec4.w, lfLightAccumulation );
        }
    }

    lFragCol = float2vec4( lfLightAccumulation );
    #endif

    return lFragCol;
}




#ifdef D_SCATTERING

#ifdef D_PLATFORM_ORBIS
//#pragma argument(targetoccupancy_atallcosts=40)
#endif

//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float kfAtmosphereSize     = 1.08;
STATIC_CONST float kfAtmosphereHeight   = kfAtmosphereSize - 1.0;


void
HeightFog(
    in    PerFrameUniforms      lPerFrame,
    in    CommonPerMeshUniforms lCommonPerMesh,
    in    CustomPerMeshUniforms lCustomPerMesh,
    in    float lfDepth,
    in    float lfHeight,
    in    float lfFogStrengthFade,
    inout vec4  lFragmentColourVec4 )
{
    float lfHeightFogStrength       = lCustomPerMesh.gHeightFogParamsVec4.r;
    float lfHeightFogOffset         = lCustomPerMesh.gHeightFogParamsVec4.g;
    float lfHeightFogHeight         = lCustomPerMesh.gHeightFogParamsVec4.b;
    float lfHeightFogMax            = lCustomPerMesh.gHeightFogParamsVec4.a;
    float lfHeightFogOutStrength    = lCustomPerMesh.gHeightFogColourVec4.a;
    float lfWaterHeight             = lCustomPerMesh.gWaterFogVec4.r;

    {
        float lfFogDistance     = lfDepth;

        float lfFogValue        = lfFogDistance * lfHeightFogStrength;
        lfFogValue              = exp( - lfHeightFogOffset - lfFogValue * lfFogValue );
        lfFogValue              = 1.0 - saturate( lfFogValue );

        float lfHeightFogValue  = ( lfHeight - lfWaterHeight ) / ( lfHeightFogHeight + 0.0001 );

        lfHeightFogValue        = 1.0 - saturate( lfHeightFogValue );
        lfHeightFogValue       *= lfHeightFogMax;

        float lfHeightFogOut    = lfFogDistance * lfHeightFogOutStrength;
        lfHeightFogOut          = exp( - lfHeightFogOffset - lfHeightFogOut * lfHeightFogOut );
        lfHeightFogOut          = 1.0 - saturate( lfHeightFogOut );

        lfFogValue              = clamp( lfHeightFogValue * lfFogValue, 0.0, 1.0 - lfFogStrengthFade );

        vec3  lLightColour      = lCommonPerMesh.gLightColourVec4.rgb;
        vec3  lHeightFogColour  = lCustomPerMesh.gHeightFogColourVec4.rgb * lLightColour;

        lFragmentColourVec4.rgb = mix( lFragmentColourVec4.rgb, lHeightFogColour, saturate( lfFogValue - lfHeightFogOut ) );
        lFragmentColourVec4.a   = max( lFragmentColourVec4.a, lfFogValue );
    }
}

void
AtmosphereDistanceFog(
    in    CustomPerMeshUniforms lCustomPerMesh,
    in    float lfFogDistance,
    in    float lfColourFade,
    in    float lfSpaceFogStrength,
    in    float lfFogStrengthFade,
    in    vec4  lFragmentColourVec4,
    inout vec4  lAtmosphereFogResultVec4 )
{
    // Fog values
    float lfFogMax               = mix( lCustomPerMesh.gFogParamsVec4.g, 1.0, lfColourFade );
    float lfFogColourStrength    = lCustomPerMesh.gFogParamsVec4.b;
    float lfFogColourMax         = lCustomPerMesh.gFogParamsVec4.a;

    vec3  lFogColour             = lCustomPerMesh.gFogColourVec4.rgb;

    float lfFogStrength          = lCustomPerMesh.gFogParamsVec4.r;
    float lfFogValue             = lfFogDistance * mix( lfFogStrength, lfSpaceFogStrength, lfFogStrengthFade );
    lfFogValue                   = exp( - lfFogValue * lfFogValue );
    lfFogValue                   = clamp( 1.0 - lfFogValue, 0.0, lfFogMax );

    float lfFogColour            = lfFogDistance * lfFogColourStrength;
    lfFogColour                  = exp( - lfFogColour * lfFogColour );
    lfFogColour                  = clamp( 1.0 - lfFogColour, 0.0, lfFogColourMax );

    lAtmosphereFogResultVec4.rgb = mix( lFogColour, lFragmentColourVec4.rgb, lfFogColour );
    lAtmosphereFogResultVec4.a   = max( lFragmentColourVec4.a, lfFogValue );
}

void
SpaceDistanceFog(
    in    float lfFogDistance,
    in    float lfNearestPlanetRadius,
    in    float lfHeight,
    in    float lfSpaceFogStrength,
    in    vec4  lFragmentColourVec4,
    inout vec4  lSpaceFogResultVec4 )
{
    if( lfHeight <= lfNearestPlanetRadius * kfAtmosphereHeight )
    {
        float lfFogValue        = lfFogDistance * lfSpaceFogStrength;
        lfFogValue              = exp( - lfFogValue * lfFogValue );
        lfFogValue              = 1.0 - saturate( lfFogValue );

        lSpaceFogResultVec4.rgb = lFragmentColourVec4.rgb;
        lSpaceFogResultVec4.a   = max( lFragmentColourVec4.a, lfFogValue );
    }
}

void
SpaceFog(
    in    CustomPerMeshUniforms lCustomPerMesh,
    in    float lfViewLightDot,
    in    float lfNearestPlanetRadius,
    in    float lfDepth,
    in    float lfHeight,
    in    float lfFogStrengthFade,
    in    float lfParamFade,
    in    float lfFarBlend,
    inout vec4  lFragmentColourVec4 )
{
    float lfSpaceFogStrength2       = lCustomPerMesh.gSpaceFogParamsVec4.r;
    float lfSpaceFogClamp           = lCustomPerMesh.gSpaceFogParamsVec4.g;

    if( lfHeight < lfNearestPlanetRadius * kfAtmosphereHeight )
    {
        lfSpaceFogClamp   = lCustomPerMesh.gSpaceFogParamsVec4.b;
        lfFogStrengthFade = saturate( 1.0 - length( vec3( lFragmentColourVec4.rgb ) ) );
    }

    float lfDistance    = lfDepth;
    float lfFogStrength = lCustomPerMesh.gFogParamsVec4.r;
    float lfFogValue    = lfDistance *  mix( lfFogStrength, lfSpaceFogStrength2, lfFogStrengthFade );
    lfFogValue          = exp( - lfFogValue * lfFogValue );
    lfFogValue          = saturate( 1.0 - lfFogValue );
    lfFogValue         *= lfParamFade * lfFarBlend;
    lfFogValue          = clamp( lfFogValue, 0.0, lfSpaceFogClamp );

    if (lfFogValue > 0.0)
    {
        float lfSpaceCenterPowerVec3    = lCustomPerMesh.gSpaceSkyColour2Vec4.a;

        vec3  lSpaceFogWaveColourVec3   = lCustomPerMesh.gSpaceSkyColour1Vec4.rgb;

        float lfVS = lfViewLightDot;
        if(   lfVS > 0.0 )
        {
            lfVS = pow(  lfVS, lfSpaceCenterPowerVec3 );
            lSpaceFogWaveColourVec3 += lfVS * ( lCustomPerMesh.gSpaceSkyColour2Vec4.rgb - lSpaceFogWaveColourVec3 );
        }
        else
        {
            lfVS = pow( -lfVS, lfSpaceCenterPowerVec3 );
            lSpaceFogWaveColourVec3 += lfVS * ( lCustomPerMesh.gSpaceSkyColour3Vec4.rgb - lSpaceFogWaveColourVec3 );
        }

        vec3  lSpaceFogColourVec3       = lCustomPerMesh.gSpaceFogColourVec4.rgb;
        float lfSpaceFogColourDistance  = lCustomPerMesh.gSpaceFogColourVec4.a;

        float lfFogColour               = lfDistance * lfSpaceFogColourDistance;
        lfFogColour                     = exp( - lfFogColour * lfFogColour );
        lfFogColour                     = 1.0 - saturate( lfFogColour );
        lSpaceFogColourVec3             = mix( lSpaceFogColourVec3, lSpaceFogWaveColourVec3, lfFogColour );

        vec3  lSpaceFogColour2Vec3      = lCustomPerMesh.gSpaceFogColour2Vec4.rgb;
        float lfSpaceFogColour2Distance = lCustomPerMesh.gSpaceFogColour2Vec4.a;

        lfFogColour                     = lfDistance * lfSpaceFogColour2Distance;
        lfFogColour                     = exp( - lfFogColour * lfFogColour );
        lfFogColour                     = 1.0 - saturate( lfFogColour );
        lSpaceFogColourVec3             = mix( lSpaceFogColour2Vec3, lSpaceFogColourVec3, lfFogColour );

        lFragmentColourVec4.rgb         = mix( lFragmentColourVec4.rgb, lSpaceFogColourVec3, saturate( lfFogValue ) );
    }

    lFragmentColourVec4.a        = max( lFragmentColourVec4.a, lfFogValue );
}

void
Fog(
    in    PerFrameUniforms      lPerFrame,
    in    CommonPerMeshUniforms lCommonPerMesh,
    in    CustomPerMeshUniforms lCustomPerMesh,
    in    vec3  lPrimaryRelativeViewPositionVec3,
    in    vec3  lPrimaryRelToNearestRelVec3,
    in    vec3  lWorldDirectionVec3,
    in    vec3  lLightDirectionVec3,
    in    float lfPrimaryPlanetRadius,
    in    float lfNearestPlanetRadius,
    in    float lfDepth,
    in    float lfHeight,
    in    float lfFarBlend,
    in    float lfColourFade,
    in    float lfFogStrengthFade,
    in    float lfParamFade,
    in    bool  lbThisIsPrimaryPlanet,
    in    bool  lbIsInSpace,
    inout vec4  lFragmentColourVec4 )
{
    float lfNearestDistSqr;
    float lfViewLightDot    = dot( lWorldDirectionVec3, lLightDirectionVec3 );
    {
        // Fog values
        float lfSpaceFogStrength    = mix( lCustomPerMesh.gSpaceScatteringParamsVec4.r, 4.0, lfFarBlend ) / lfNearestPlanetRadius;

        float lfAtmosFogDistance    = 0.0;

        if ( lfParamFade < 1.0 )
        {
            lfAtmosFogDistance  = GetClampedRayIntersectionDist(
                                    lPrimaryRelativeViewPositionVec3,
                                    lWorldDirectionVec3,
                                    lfDepth,
                                    kfAtmosphereSize * lfPrimaryPlanetRadius );
        }
        
        lPrimaryRelativeViewPositionVec3 += lPrimaryRelToNearestRelVec3;
        lfNearestDistSqr                  = Sqr( lPrimaryRelativeViewPositionVec3 );

        float lfSpaceFogDistance        = 0.0;

        if ( lfParamFade > 0.0 )
        {
            lfSpaceFogDistance  = GetClampedRayIntersectionDist(
                                    lPrimaryRelativeViewPositionVec3,
                                    lWorldDirectionVec3,
                                    lfDepth,
                                    kfAtmosphereSize * lfNearestPlanetRadius );
        }

        vec4 lAtmosphereFogResultVec4   = vec4( 0.0, 0.0, 0.0, 0.0 );
        vec4 lSpaceFogResultVec4        = lFragmentColourVec4;

        if ( lfAtmosFogDistance > 0.0 )
        {
            AtmosphereDistanceFog(
                lCustomPerMesh,
                lfAtmosFogDistance,
                lfColourFade,
                lfSpaceFogStrength,
                lfFogStrengthFade,
                lFragmentColourVec4,
                lAtmosphereFogResultVec4 );
        }

        if ( lfSpaceFogDistance > 0.0 )
        {
            SpaceDistanceFog(
                lfSpaceFogDistance,
                lfNearestPlanetRadius,
                lfHeight,
                lfSpaceFogStrength,
                lFragmentColourVec4,
                lSpaceFogResultVec4 );
        }

        lFragmentColourVec4 = mix( lAtmosphereFogResultVec4, lSpaceFogResultVec4, lfParamFade );
    }

    // Height Fog
    if ( lbThisIsPrimaryPlanet )
    {
        HeightFog(
            lPerFrame,
            lCommonPerMesh,
            lCustomPerMesh,
            lfDepth,
            lfHeight,
            lfFogStrengthFade,
            lFragmentColourVec4 );
    }

    //////
    //Water Fog
    {
        float lfWaterHeight = lCustomPerMesh.gWaterFogVec4.r;
        if( lfNearestDistSqr < Sqr( lfNearestPlanetRadius + lfWaterHeight ) )
        {
            float lfWaterFade = length(lPerFrame.gViewPositionVec3 - lCommonPerMesh.gPlanetPositionVec4.xyz) - lCommonPerMesh.gPlanetPositionVec4.w;
            lfWaterFade = 1.0 - saturate((lfWaterFade - 2000.0) / 1000.0);
            lFragmentColourVec4.a = 1.0 - ((lbThisIsPrimaryPlanet ? 1.0 : 0.0) * lfWaterFade);
        }
    }

    // Space fog
    if( lbIsInSpace )
    {
        SpaceFog(
            lCustomPerMesh,
            lfViewLightDot,
            lfNearestPlanetRadius,
            lfDepth,
            lfHeight,
            lfFogStrengthFade,
            lfParamFade,
            lfFarBlend,
            lFragmentColourVec4 );
    }
}

void
GetPositionsAndDirection(
    in   PerFrameUniforms   lPerFrame,
    in    vec2              lFragCoordsVec2,
    inout float             lfDepth,
    inout vec3              lCameraRelativePositionVec3,
    inout vec3              lWorldPositionVec3,
    inout vec3              lWorldDirectionVec3 )
{   // Recreate position
    lCameraRelativePositionVec3 = RecreatePositionFromDepthWithIVP_SC(
                                    lfDepth, lFragCoordsVec2,
                                    float2vec3( 0.0 ),
                                    lPerFrame.gInverseViewProjectionSCMat4,
                                    lPerFrame.gClipPlanesVec4 );

    lWorldPositionVec3  = lCameraRelativePositionVec3 + lPerFrame.gViewPositionVec3;
    lfDepth             = length( lCameraRelativePositionVec3 );
    lWorldDirectionVec3 = lCameraRelativePositionVec3 / lfDepth;
}

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT_NOPERSP( vec2, mTexCoordsVec2,  TEXCOORD0 )

DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR01_SRT
{
    vec4  lFragmentColourVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );

    #if !defined( D_CHECKERBOARD )
    vec2  lFragCoordsVec2     = TEX_COORDS;
    #else
    vec2  lFragCoordsVec2     = ( floor( gl_FragCoord.xy ) + gl_SamplePosition ) / lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    #endif

    #if defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_VULKAN )
    if ( lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0 )
    {
        #if defined( D_PLATFORM_ORBIS )
        if ( HmdFovMask_isInvisible( lFragCoordsVec2,
                                     lUniforms.mpPerFrame.gFrustumTanFovVec4,
                                     lUniforms.mpPerFrame.gVREyeInfoVec3 ) )
        #else // D_PLATFORM_VULKAN
        vec3 lNormalVec3 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer1Map, lFragCoordsVec2 ).xyz;
        if (lNormalVec3.x == 0.0 && lNormalVec3.y == 0.0 && lNormalVec3.z == 0.0)
        #endif
        {
            #ifdef D_COMPUTE
            return;
            #else
            discard;
            #endif
        }
    }
    #endif

    // Get fragment position and depth
    float lfDepth = FastDenormaliseDepth(
                        lUniforms.mpPerFrame.gClipPlanesVec4,
                        DecodeDepthFromColour( READ_GBUFFER( lUniforms.mpCustomPerMesh, gBufferMap, lFragCoordsVec2 ) ) );

    vec3  lWorldPositionVec3;
    vec3  lWorldDirectionVec3;
    {
        vec3  lCameraRelativePositionVec3;

        GetPositionsAndDirection( DEREF_PTR( lUniforms.mpPerFrame ),
                                  lFragCoordsVec2,    lfDepth, lCameraRelativePositionVec3,
                                  lWorldPositionVec3, lWorldDirectionVec3 );


        #if !defined( D_PLATFORM_OPENGL ) && !defined( D_PLATFORM_VULKAN ) && !defined( D_NO_GODRAYS ) && !defined( D_PLATFORM_METAL )
        // God-rays output to MRT1
        vec4 lRayStrength   = RayMarch(
                                lFragCoordsVec2,
                                lCameraRelativePositionVec3,
                                DEREF_PTR( lUniforms.mpCustomPerMesh ),
                                DEREF_PTR( lUniforms.mpPerFrame ),
                                DEREF_PTR( lUniforms.mpCommonPerMesh ) );
        lRayStrength.a      = 1.0;
        WRITE_FRAGMENT_COLOUR1( lRayStrength );
        #else
        WRITE_FRAGMENT_COLOUR1( vec4( 0.0, 0.0, 0.0, 0.0 ) );
        #endif
    }

    if( lfDepth >= lUniforms.mpPerFrame.gClipPlanesVec4.y - 100.0 )
    {
        #ifndef D_COMPUTE
        WRITE_FRAGMENT_COLOUR0( vec4( 0.0, 0.0, 0.0, 0.0 ) );
        #endif
        return;
    }

    vec3  lPrimaryPlanetPositionVec3       = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
    float lfPrimaryPlanetRadius            = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;
    vec3  lPrimaryRelativeViewPositionVec3 = lUniforms.mpPerFrame.gViewPositionVec3 - lPrimaryPlanetPositionVec3;
    {
        float lfWaterHeight = lUniforms.mpCustomPerMesh.gWaterFogVec4.r;
        if(   lfWaterHeight > 0.0 )
        {
            // Calculate positions relative to planet
            float lfWaterRadius     = lfPrimaryPlanetRadius + lfWaterHeight;
            float lfDist            = GetNearRayIntersectionDist(
                                        lPrimaryRelativeViewPositionVec3,
                                        lWorldDirectionVec3, lfWaterRadius );

            // If we hit the sphere and the water plane is closer than the original point
            if( lfDist > 0.0 && lfDepth > lfDist )
            {
                lWorldPositionVec3  = lPrimaryRelativeViewPositionVec3 +
                                      lWorldDirectionVec3 * lfDist     +
                                      lPrimaryPlanetPositionVec3;
                lfDepth             = lfDist;
            }
        }
    }

    // Find nearest planet
    int   liNearestIdx      = 0;
    float lfNearestHeight   = 3.402823466e+38f; // FLOAT_MAX
    {
        for( int ii = 0; ii < 6; ++ii )
        {
            if( lUniforms.mpCustomPerMesh.gaPlanetPositionsVec4[ ii ].w > 0.0 )
            {
                float lfNewHeight   = length( ( lUniforms.mpCustomPerMesh.gaPlanetPositionsVec4[ ii ].xyz - lWorldPositionVec3 ) );
                lfNewHeight        -= lUniforms.mpCustomPerMesh.gaPlanetPositionsVec4[ ii ].w;

                if( lfNewHeight <= lfNearestHeight )
                {
                    liNearestIdx    = ii;
                    lfNearestHeight = lfNewHeight;
                }
            }
        }
    }

    vec3  lLightDirectionVec3;
    float lfNearestPlanetRadius;
    vec3  lPrimaryRelToNearestRelVec3;
    {
        vec3  lNearestPlanetPosition;

        lNearestPlanetPosition      = lUniforms.mpCustomPerMesh.gaPlanetPositionsVec4[ liNearestIdx ].xyz;
        lfNearestPlanetRadius       = lUniforms.mpCustomPerMesh.gaPlanetPositionsVec4[ liNearestIdx ].w;
        lPrimaryRelToNearestRelVec3 = lPrimaryPlanetPositionVec3 - lNearestPlanetPosition;
        lLightDirectionVec3         = lUniforms.mpCustomPerMesh.gSunPositionVec4.xyz;
    }

    // Calculate various height fade values to interpolate between fog settings
    bool  lbIsInSpace            = false;
    bool  lbThisIsPrimaryPlanet  = false;
    bool  lbAtmosScattering      = false;
    bool  lbSpaceScattering      = false;
    float lfFarBlend             = 0.0;
#ifdef D_USE_PACKING
    uint  luPackedParams         = 0;
    {
#endif
        float lfColourFade               = 0.0;
        float lfParamFade                = 0.0;
        float lfFogStrengthFade          = 0.0;
        float lfParamFadeMix             = 0.0;
#ifndef D_USE_PACKING
    {
#endif
        bool  lbIndoorsFog               = false;

        float lfIsInSpace                = 0.0;
        float lfBlendStartHeight         = lUniforms.mpCustomPerMesh.gFogFadeHeightsVec4.r;
        float lfBlendDistance            = lUniforms.mpCustomPerMesh.gFogFadeHeightsVec4.g;
        float lfFogStrengthStartHeight   = lUniforms.mpCustomPerMesh.gFogFadeHeightsVec4.b;
        float lfFogStrengthDistance      = lUniforms.mpCustomPerMesh.gFogFadeHeightsVec4.a;
        float lfPositionBlendStartHeight = lUniforms.mpCustomPerMesh.gFogFadeHeights2Vec4.r;
        float lfPositionBlendDistance    = lUniforms.mpCustomPerMesh.gFogFadeHeights2Vec4.g;
        float lfColourBlendStartHeight   = lUniforms.mpCustomPerMesh.gFogFadeHeights2Vec4.b;
        float lfColourBlendDistance      = lUniforms.mpCustomPerMesh.gFogFadeHeights2Vec4.a;
        float lfFarBlendStartHeight      = lUniforms.mpCustomPerMesh.gFogFadeHeights3Vec4.b;
        float lfFarBlendDistance         = lUniforms.mpCustomPerMesh.gFogFadeHeights3Vec4.a;
        float lfFogStrengthPower         = lUniforms.mpCustomPerMesh.gSpaceFogParamsVec4.a;

        float lfDistanceFromPlanet  = length( lPrimaryRelativeViewPositionVec3 + lPrimaryRelToNearestRelVec3 )  - lfNearestPlanetRadius;
        float lfDistanceFromPrimary = length( lPrimaryRelativeViewPositionVec3 )                                - lfPrimaryPlanetRadius;

        float lfIsOffPrimaryPlanet  = saturate( ( lfDistanceFromPrimary - lfColourBlendStartHeight ) / lfColourBlendDistance );
        lfIsInSpace                 = saturate( ( lfDistanceFromPlanet  - lfColourBlendStartHeight ) / lfColourBlendDistance );
        lfColourFade                = mix( lfIsOffPrimaryPlanet, 0.0, ( 1.0 - lfIsOffPrimaryPlanet ) * lfIsInSpace );

        lfIsOffPrimaryPlanet        = saturate( ( lfDistanceFromPrimary - lfBlendStartHeight ) / lfBlendDistance );
        lfIsInSpace                 = saturate( ( lfDistanceFromPlanet  - lfBlendStartHeight ) / lfBlendDistance );
        lfParamFade                 = mix( lfIsOffPrimaryPlanet, 0.0, ( 1.0 - lfIsOffPrimaryPlanet ) * lfIsInSpace );

        lfIsOffPrimaryPlanet        = saturate( ( lfDistanceFromPrimary - lfFogStrengthStartHeight ) / lfFogStrengthDistance );
        lfIsInSpace                 = saturate( ( lfDistanceFromPlanet -  lfFogStrengthStartHeight ) / lfFogStrengthDistance );
        lfFogStrengthFade           = mix( lfIsOffPrimaryPlanet, 0.0, ( 1.0 - lfIsOffPrimaryPlanet ) * lfIsInSpace );
        lfFogStrengthFade           = 1.0 - saturate( pow( 1.0 - lfFogStrengthFade, lfFogStrengthPower ) );
        lbIndoorsFog                = lUniforms.mpCustomPerMesh.gFogColourVec4.w > 0.0;
        lfFogStrengthFade           = lbIndoorsFog ? 0.0 : lfFogStrengthFade;

        lfIsInSpace                 = saturate( ( lfDistanceFromPrimary - lfPositionBlendStartHeight ) / lfPositionBlendDistance );
        lfFarBlend                  = saturate( ( lfDistanceFromPlanet - lfFarBlendStartHeight )       / lfFarBlendDistance );

        lbIsInSpace                 = lfIsInSpace > 0.0 && !lbIndoorsFog;
        lbThisIsPrimaryPlanet       = abs( lfDistanceFromPlanet - lfDistanceFromPrimary ) < 100.0 && !lbIndoorsFog;
        lfParamFade                 = lbIndoorsFog                           ? 0.0 : lfParamFade;
        lfParamFadeMix              = lbThisIsPrimaryPlanet && !lbIndoorsFog ? 1.0 : lfParamFade;

#ifdef D_USE_PACKING
        luPackedParams              = packUnorm4x8( vec4( lfColourFade, lfParamFade, lfParamFadeMix, lfFogStrengthFade ) );
#endif

        lbAtmosScattering           = ( lfColourFade < 1.0 || lfParamFadeMix < 1.0 ) && !lbIndoorsFog;
        lbSpaceScattering           = ( lfColourFade > 0.0 || lfParamFadeMix < 1.0 ) && !lbIndoorsFog;
    }


    float lfAtmosInScattering = 0.0;

    if ( lbAtmosScattering )
    {
        vec3  lPrimaryRelViewPosScaledVec3  = lPrimaryRelativeViewPositionVec3 / lfPrimaryPlanetRadius;
        vec3  lDirectionToAtmosphereVec3    = lWorldDirectionVec3;

        vec3  lAtmospherePositionVec3   = GetFarRayIntersectionPoint( lPrimaryRelViewPosScaledVec3, lDirectionToAtmosphereVec3, kfAtmosphereSize );

        float lfAtmosphereThickness     = lUniforms.mpCustomPerMesh.gScatteringParamsVec4.g;
        float lfHorizonFadeSpeed        = lUniforms.mpCustomPerMesh.gScatteringParamsVec4.a;
        
        // Calculate scattering
        lfAtmosInScattering             = InScattering(
                                            lAtmospherePositionVec3,
                                            lLightDirectionVec3,
                                            lPrimaryRelViewPosScaledVec3,
                                            kfAtmosphereSize,
                                            lfHorizonFadeSpeed,
                                            lfAtmosphereThickness,
                                            #ifdef D_PLATFORM_SWITCH
                                            true,
                                            #else
                                            D_PLATFORM_PC_LOWEND,
                                            #endif
                                            false );
    }


    float lfSpaceInScattering = 0.0;

    if ( lbSpaceScattering )
    {
        vec3  lTerrainPositionVec3      = ( lPrimaryRelativeViewPositionVec3 + lfDepth * lWorldDirectionVec3 + lPrimaryRelToNearestRelVec3 ) / ( lfNearestHeight + lfNearestPlanetRadius );
        vec3  lViewPositionVec3         = lTerrainPositionVec3 - lWorldDirectionVec3 * kfAtmosphereHeight;

        float lfSpaceAtmosThickness     = lUniforms.mpCustomPerMesh.gSpaceScatteringParamsVec4.g;
        float lfSpaceHorizonFadeSpeed   = lUniforms.mpCustomPerMesh.gSpaceScatteringParamsVec4.a;

        // Calculate scattering
        lfSpaceInScattering             = InScattering(
                                           lTerrainPositionVec3,
                                           lLightDirectionVec3,
                                           lViewPositionVec3,
                                           kfAtmosphereSize,
                                           lfSpaceHorizonFadeSpeed,
                                           lfSpaceAtmosThickness,
                                           #ifdef D_PLATFORM_SWITCH
                                           true,
                                           #else
                                           D_PLATFORM_PC_LOWEND,
                                           #endif
                                           true );
    }

    float lfHorizonPlanetFade   = 0.0;

    // Atmosphere scattering
    vec3  lAtmosphereScattering = vec3( 1.0, 0.0, 0.0 );

    if ( lbAtmosScattering )
    {
        vec3  lHorizonColour                = lUniforms.mpCustomPerMesh.gHorizonColourVec4.rgb * lUniforms.mpCustomPerMesh.gSunPositionVec4.a;

        vec3  lDirectionToAtmosphereVec3    = lWorldDirectionVec3;
        vec3  lUpDirectionVec3              = normalize( lPrimaryRelativeViewPositionVec3 );

        vec3  lSkyColour                = lUniforms.mpCustomPerMesh.gSkyColourVec4.rgb * lUniforms.mpCustomPerMesh.gSkyColourVec4.a;
        vec3  lSunColour                = lUniforms.mpCustomPerMesh.gSunColourVec4.rgb;

        float lfHorizonMultiplier       = lUniforms.mpCustomPerMesh.gScatteringParamsVec4.b;
        float lfHorizonFadeSpeed        = lUniforms.mpCustomPerMesh.gScatteringParamsVec4.a;
        float lfBlendTightness          = min( lUniforms.mpCustomPerMesh.gScatteringParamsVec4.r, ( 0.8 - 0.1 ) * 0.5 );

        vec3  lGradientSpeed            = lUniforms.mpCustomPerMesh.gSkyGradientSpeedVec4.rgb;
        vec3  lSkyUpperColour           = lUniforms.mpCustomPerMesh.gSkyUpperColourVec4.rgb * lUniforms.mpCustomPerMesh.gSkyColourVec4.a;
        vec3  lSkySolarColour           = lUniforms.mpCustomPerMesh.gSkySolarColourVec4.rgb * lUniforms.mpCustomPerMesh.gSkyColourVec4.a;
        float lfSolarFadeSize           = lUniforms.mpCustomPerMesh.gSkyUpperColourVec4.a;
        float lfSolarFadeStrength       = lUniforms.mpCustomPerMesh.gSkySolarColourVec4.a;
        float lfUpperSkyFadeSpeed       = lUniforms.mpCustomPerMesh.gSkyGradientSpeedVec4.a;
        float lfUpperSkyFadeOffset      = lUniforms.mpCustomPerMesh.gSkyUpperParamsVec4.r;
        float lfSunStrength             = lUniforms.mpCustomPerMesh.gHorizonColourVec4.a;

        float lfRayleighPhase           = RayleighPhaseNoNorm( lLightDirectionVec3, lDirectionToAtmosphereVec3 );
        float lfMiePhase                = MiePhaseNoNorm( lLightDirectionVec3, lDirectionToAtmosphereVec3, lfSolarFadeSize );
        float lfAtmosphereInScattering  = InScatteringPhase( lfAtmosInScattering, 1.0, lfSunStrength, lfHorizonFadeSpeed, lfRayleighPhase );
        vec3  lSolarScatteringVec3      = InScatteringPhase( lfAtmosInScattering, lSunColour, lfSolarFadeStrength, 1.0, lfMiePhase );

        // Shift scattering to cover full range
        vec3 lAdjustedScatteringVec3 = smoothstep( min( 0.1 + lfBlendTightness * lGradientSpeed, float2vec3( 0.45  ) ),
                                                   max( 0.8 - lfBlendTightness * lGradientSpeed, float2vec3( 0.451 ) ), float2vec3( lfAtmosphereInScattering * lfHorizonMultiplier ) );
        vec3 lScatteringColourVec3   = lSkyColour;

        lScatteringColourVec3        = mix( lSkyUpperColour,       lSkyColour,      saturate( max( lfAtmosphereInScattering - lfUpperSkyFadeOffset, 0.0 ) * lfUpperSkyFadeSpeed * lGradientSpeed ) );
        lScatteringColourVec3        = mix( lScatteringColourVec3, lSkySolarColour, saturate( lSolarScatteringVec3    * lGradientSpeed ) );
        lAtmosphereScattering        = mix( lScatteringColourVec3, lHorizonColour,  saturate( lAdjustedScatteringVec3 * lGradientSpeed ) );

        // Fade on distant planets at the horizon
        lfHorizonPlanetFade         = 1.0 - smoothstep( 0.1, 0.8, lfAtmosphereInScattering * 2.2 );

        float lfAngleTest           = dot( lDirectionToAtmosphereVec3, lUpDirectionVec3 );
        lfAngleTest                 = smoothstep( 0.0, 0.2, -lfAngleTest );
        lAtmosphereScattering       = mix( lAtmosphereScattering, lHorizonColour, lfAngleTest );
    }

    // Space scattering
    float lfSpaceScattering = 0.0;

    if ( lbSpaceScattering )
    {
        float lfSpaceHorizonMultiplier  = lUniforms.mpCustomPerMesh.gSpaceScatteringParamsVec4.b;
        float lfSpaceHorizonFadeSpeed   = lUniforms.mpCustomPerMesh.gSpaceScatteringParamsVec4.a;
        float lfSpaceSunStrength        = lUniforms.mpCustomPerMesh.gSpaceSkyColourVec4.a;

        lfSpaceScattering               = InScatteringPhase(
                                            lfSpaceInScattering,
                                            1.0,
                                            lfSpaceSunStrength,
                                            lfSpaceHorizonFadeSpeed,
                                            RayleighPhaseNoNorm( lLightDirectionVec3, lWorldDirectionVec3 ) );

        vec3 lAdjustedScatteringVec3    = float2vec3( smoothstep( 0.1, 0.8, lfSpaceScattering * lfSpaceHorizonMultiplier ) );
        lfSpaceScattering               = mix( 1.0, lAdjustedScatteringVec3.x, lfFarBlend );

        // Fade on the dark side of distant planets
        lfHorizonPlanetFade             = min( lfHorizonPlanetFade, lfSpaceScattering * 0.2 + 0.01 );
    }

    // Blending as you leave atmosphere
#ifdef D_USE_PACKING
    float lfColourFade;
    float lfParamFade;
    float lfFogStrengthFade;
    float lfParamFadeMix;
    {
        vec4  lUnpackedParams   = unpackUnorm4x8( luPackedParams );

        lfColourFade            = lUnpackedParams.x;
        lfParamFade             = lUnpackedParams.y;
        lfParamFadeMix          = lUnpackedParams.z;
        lfFogStrengthFade       = lUnpackedParams.w;
#else
    {
#endif

        vec3  lSpaceHorizonColour;
        lSpaceHorizonColour     = lUniforms.mpCustomPerMesh.gaPlanetColoursVec4[ liNearestIdx ].xyz;
        lfParamFade             = mix( lfHorizonPlanetFade, lfParamFade,  lfParamFadeMix );
        lfColourFade            = mix( lfHorizonPlanetFade, lfColourFade, lfParamFadeMix );
        lFragmentColourVec4.rgb = mix(saturate(lAtmosphereScattering), saturate(lfSpaceScattering * lSpaceHorizonColour), lfColourFade);
    }

    Fog(
        DEREF_PTR( lUniforms.mpPerFrame ),
        DEREF_PTR( lUniforms.mpCommonPerMesh ),
        DEREF_PTR( lUniforms.mpCustomPerMesh ),
        lPrimaryRelativeViewPositionVec3,
        lPrimaryRelToNearestRelVec3,
        lWorldDirectionVec3,
        lLightDirectionVec3,
        lfPrimaryPlanetRadius,
        lfNearestPlanetRadius,
        lfDepth,
        lfNearestHeight,
        lfFarBlend,
        lfColourFade,
        lfFogStrengthFade,
        lfParamFade,
        lbThisIsPrimaryPlanet,
        lbIsInSpace,
        lFragmentColourVec4 );

    #ifdef D_COMPUTE
    if (lFragmentColourVec4.a == 0.0)
    {
        return;
    }
    WRITE_FRAGMENT_COLOUR0( FRAGMENT_COLOUR0 + lFragmentColourVec4 );
    #else
    if( lFragmentColourVec4.a == 0.0 )
    {
#ifdef D_PLATFORM_XBOXONE
        FRAGMENT_COLOUR0 = vec4(0.0, 0.0, 0.0, 0.0);
        return;
#else
        discard;
#endif
    }

// we use regular "replace" blend mode for metal forward
#if !defined( D_PLATFORM_METAL )
    lFragmentColourVec4.rgb /= lFragmentColourVec4.a;
#endif

    FRAGMENT_COLOUR0 = lFragmentColourVec4;
    #endif

}


#endif

// =================================================================================================
//
// RAYMARCH
//
// =================================================================================================

#ifdef D_RAYMARCH

//-----------------------------------------------------------------------------
//      Global Data

//SAMPLER2DSHADOW( gShadowMap, 15 );


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT

    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END


FRAGMENT_MAIN_COLOUR_SRT
{
#ifdef D_COMPUTE
    return;
#elif !defined D_PLATFORM_OPENGL && !defined( D_PLATFORM_VULKAN ) && !defined( D_PLATFORM_SWITCH ) && !defined( D_PLATFORM_METAL )
    discard;
#else

    vec2  lFragCoords   = TEX_COORDS;
    float lfDepth       = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lFragCoords, 0.0 ) );
    vec3  lWorldPos     = RecreatePositionFromNormDepthWithIVP_SC( lfDepth, lFragCoords, float2vec3( 0.0 ), lUniforms.mpPerFrame.gInverseViewProjectionSCMat4, lUniforms.mpPerFrame.gClipPlanesVec4 );

    FRAGMENT_COLOUR     = RayMarch(
                            lFragCoords,
                            lWorldPos,
                            DEREF_PTR( lUniforms.mpCustomPerMesh ),
                            DEREF_PTR( lUniforms.mpPerFrame ),
                            DEREF_PTR( lUniforms.mpCommonPerMesh ) );
#endif
}

#endif
