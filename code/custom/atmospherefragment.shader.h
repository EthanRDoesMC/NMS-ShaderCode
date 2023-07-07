////////////////////////////////////////////////////////////////////////////////
///
///     @file       AtmosphereFragment.h
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

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "Custom/AtmosphereCommon.h"

#include "Common/CommonPostProcess.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonScattering.shader.h"

#if defined( D_SCATTERING_FORWARD )
#include "Common/ACES.shader.h"
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
INPUT( vec4, mWorldPositionVec4, TEXCOORD0 )
INPUT( vec4, mTexCoordsVec4, TEXCOORD1 )
INPUT( vec4, mWorldNormalVec3_mfDistanceFromPlanet, TEXCOORD2 )
#ifndef D_REFLECT
INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD3 )
#endif
DECLARE_INPUT_END

#ifdef D_SCATTERING

#ifdef D_PLATFORM_ORBIS
#pragma argument(unrollallloops)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
#pragma optionNV(fastmath on)
#endif

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float kfAtmosphereSize = 1.08;

FRAGMENT_MAIN_COLOUR_SRT
{
#if defined(D_PLATFORM_SWITCH) && defined(D_REGULAR_SCATTERING)
    // ImpreciseSwitchDepthBuffer:
    // The regular (24-bit) depth buffer on switch doesn't have enough precision at very far (inter-planetary) distances.
    // The 32-bit normalised depth buffer does though, for stuff in the distance we can add an extra explicit check to prevent
    // sorting issues.
    if (IN(mScreenSpacePositionVec4).w > (lUniforms.mpPerFrame.gClipPlanesVec4.y * 0.25))
    {
        vec2 lCoords = (IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w) * 0.5 + 0.5;
        lCoords = SCREENSPACE_AS_RENDERTARGET_UVS(lCoords);
        float lfDepth = DecodeDepthFromColour(texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), lCoords));

        float lfSceneDepth = FastDenormaliseDepth(lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth);
        if (lfSceneDepth < IN(mScreenSpacePositionVec4).w)
        {
            discard;
        }
    }
#endif

    vec4  lFragmentColourVec4 = vec4( 0.0, 0.0, 0.0, 1.0 );
    float lfInScattering;
    vec3  lSkyScatteringVec3;

    float lfDistanceFromPlanet  = IN( mWorldNormalVec3_mfDistanceFromPlanet ).w;

#ifdef D_SCATTER_MASK

    #if !defined( D_CHECKERBOARD )
    vec2 lCoords = ( IN( mScreenSpacePositionVec4 ).xy /  IN( mScreenSpacePositionVec4 ).w ) * 0.5 + 0.5;
    lCoords      = SCREENSPACE_AS_RENDERTARGET_UVS( lCoords );
    #else
    vec2  lCoords = ( floor( gl_FragCoord.xy ) + gl_SamplePosition ) / lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    #endif
#ifdef D_PLATFORM_ORBIS
    float lfDepth = DecodeDepthFromColour(texture2DArray(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), vec3( lCoords, lUniforms.mpPerFrame.gVREyeInfoVec3.x ) ));
#else
    float lfDepth = DecodeDepthFromColour(texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), lCoords));
#endif

    if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
#if defined( D_PLATFORM_ORBIS )
        if ( HmdFovMask_isInvisible(lCoords,
                                    lUniforms.mpPerFrame.gFrustumTanFovVec4,
                                    lUniforms.mpPerFrame.gVREyeInfoVec3)
                                    
            )
#elif defined( D_PLATFORM_VULKAN )
        if (lfDepth == 0.0)
#else

#if defined ( D_PLATFORM_SWITCH )
        if ( false )
#else
        if ( 0 )
#endif

#endif
        {
            discard;
        }
    }

    float lfSceneDepth = FastDenormaliseDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth );
    if( lfSceneDepth * lfSceneDepth < lengthSquared( IN( mWorldPositionVec4 ).xyz - lUniforms.mpPerFrame.gViewPositionVec3 ) )
    {
        discard;
    }
    if( lfSceneDepth < lUniforms.mpPerFrame.gClipPlanesVec4.y && lfDistanceFromPlanet < 0.08 * lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w )
    {
        discard;
    }

    // Create water plane, as water is not rendered to the g-buffer
    float lfWaterHeight = lUniforms.mpCustomPerMaterial.gWaterFogVec4.r;
    if(   lfWaterHeight > 0.0 )
    {

        vec3  lViewPositionVec3  = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3  lWorldPositionVec3 = ( IN( mWorldPositionVec4 ).xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );

        if( length( lViewPositionVec3 ) < lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w + lfWaterHeight )
        {
            float lfSkyColourFactor = saturate( dot( normalize( lViewPositionVec3 ), normalize( lWorldPositionVec3 - lViewPositionVec3 ) ) );
            
            vec3  lUnderwaterSkyColour = mix( lUniforms.mpCustomPerMaterial.gWaterFogColourFarVec4.rgb * lUniforms.mpCommonPerMesh.gLightColourVec4.xyz, lUniforms.mpCustomPerMaterial.gSkyColourVec4.rgb, lfSkyColourFactor );
            FRAGMENT_COLOUR = vec4( lUnderwaterSkyColour, 1.0 );
            return;
        }
    }


#elif defined( D_REFLECT_WATER )

    // Create water plane, as water is not rendered to the g-buffer
    float lfWaterHeight      = lUniforms.mpCustomPerMaterial.gWaterFogVec4.r;
    if( lfWaterHeight > 0.0 )
    {
        vec3  lViewPositionVec3  = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3  lWorldPositionVec3 = ( IN( mWorldPositionVec4 ).xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );

        // CAMERA IS REFLECTED IN WATER PLANE, TEST IS OPPOSITE OF WHAT YOU'D EXPECT
        if( length( lViewPositionVec3 ) > lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w + lfWaterHeight )
        {
            vec3  lUnderwaterSkyColour = lUniforms.mpCustomPerMaterial.gWaterFogColourFarVec4.rgb * lUniforms.mpCommonPerMesh.gLightColourVec4.xyz;
            FRAGMENT_COLOUR = vec4( lUnderwaterSkyColour, 1.0 );
            return;
        }
    }

#endif

    vec3  lSkyColour                 = lUniforms.mpCustomPerMaterial.gSkyColourVec4.rgb;
    vec3  lSkyUpperColour            = lUniforms.mpCustomPerMaterial.gSkyUpperColourVec4.rgb;
    vec3  lSkySolarColour            = lUniforms.mpCustomPerMaterial.gSkySolarColourVec4.rgb;
    vec3  lHorizonColour             = lUniforms.mpCustomPerMaterial.gHorizonColourVec4.rgb;
    vec3  lSunColour                 = lUniforms.mpCustomPerMaterial.gSunColourVec4.rgb;
    vec3  lSpaceSunColour            = lUniforms.mpCustomPerMaterial.gSpaceSunColourVec4.rgb;
    vec3  lGradientSpeed             = lUniforms.mpCustomPerMaterial.gSkyGradientSpeedVec4.rgb;
    float lfAtmosphereThickness      = lUniforms.mpCustomPerMaterial.gScatteringParamsVec4.g;  
    float lfHorizonMultiplier        = lUniforms.mpCustomPerMaterial.gScatteringParamsVec4.b;
    float lfHorizonFadeSpeed         = lUniforms.mpCustomPerMaterial.gScatteringParamsVec4.a;
    float lfSkyAlpha                 = lUniforms.mpCustomPerMaterial.gSkyColourVec4.a;
    float lfHorizonAlpha             = lUniforms.mpCustomPerMaterial.gSunPositionVec4.a;
    float lfSunSize                  = lUniforms.mpCustomPerMaterial.gSunColourVec4.a;
    float lfSunStrength              = lUniforms.mpCustomPerMaterial.gHorizonColourVec4.a;
    float lfSolarFadeSize            = lUniforms.mpCustomPerMaterial.gSkyUpperColourVec4.a;
    float lfSolarFadeStrength        = lUniforms.mpCustomPerMaterial.gSkySolarColourVec4.a;
    float lfSpaceAtmosphereThickness = mix( lUniforms.mpCustomPerMaterial.gSpaceScatteringParamsVec4.g, lUniforms.mpCustomPerMaterial.gSpaceHorizonColourVec4.a, saturate( (lfDistanceFromPlanet - 300000.0f) / 600000.0f ) );
    float lfSpaceHorizonMultiplier   = lUniforms.mpCustomPerMaterial.gSpaceScatteringParamsVec4.b;
    float lfSpaceHorizonFadeSpeed    = lUniforms.mpCustomPerMaterial.gSpaceScatteringParamsVec4.a;
    float lfSpaceSunStrength         = lUniforms.mpCustomPerMaterial.gSpaceSkyColourVec4.a;
    float lfSpaceSunSize             = lUniforms.mpCustomPerMaterial.gSpaceSkyColour3Vec4.a;
    float lfUpperSkyFadeSpeed        = lUniforms.mpCustomPerMaterial.gSkyGradientSpeedVec4.a;
    float lfUpperSkyFadeOffset       = lUniforms.mpCustomPerMaterial.gSkyUpperParamsVec4.r;
    float lfBlendTightness           = min( lUniforms.mpCustomPerMaterial.gScatteringParamsVec4.r, ( 0.8 - 0.1 ) * 0.5 );

    // Colour and alpha blending as we go to space
    float lfBlendStartHeight    = lUniforms.mpCustomPerMaterial.gFogFadeHeightsVec4.r;
    float lfBlendDistance       = lUniforms.mpCustomPerMaterial.gFogFadeHeightsVec4.g;
    float lfIsInSpace           = saturate( ( lfDistanceFromPlanet - lfBlendStartHeight ) / lfBlendDistance );
    float lfStarsNum            = lUniforms.mpCustomPerMaterial.gSpaceSkyColour1Vec4.a;

    lfAtmosphereThickness = mix( lfAtmosphereThickness,   lfSpaceAtmosphereThickness, lfIsInSpace );
    lfHorizonMultiplier   = mix( lfHorizonMultiplier,     lfSpaceHorizonMultiplier,   lfIsInSpace );
    lfHorizonFadeSpeed    = mix( lfHorizonFadeSpeed,      lfSpaceHorizonFadeSpeed,    lfIsInSpace );
    lfSunStrength         = mix( lfSunStrength,           lfSpaceSunStrength,         lfIsInSpace );
    lfSunSize             = mix( lfSunSize,               lfSpaceSunSize,             lfIsInSpace );
    lSunColour            = mix( lSunColour,              lSpaceSunColour,            lfIsInSpace );

    // Positions, with planet radius normalised
    vec3  lSunPositionVec3         = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz;
    vec3  lSunRightVec3            = normalize( cross( lSunPositionVec3, vec3( 0.0, 1.0, 0.0 ) ) );
    vec3  lSunUpVec3               = cross( lSunPositionVec3, lSunRightVec3 );
    vec3  lSunPosition2Vec3        = normalize( lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz + float2vec3( 0.225 ) * lSunUpVec3    );
    vec3  lSunPosition3Vec3        = normalize( lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz + float2vec3( 0.225 ) * lSunRightVec3 );

    vec3  lLightPositionVec3       = /*normalize*/ ( -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz );
    vec3  lWorldPositionVec3       = ( IN( mWorldPositionVec4 ).xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) / lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;
    vec3  lViewPositionVec3        = ( lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) / lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;

    vec3 lNearPosition;
    vec3 lFarPosition;
    GetClampedRayIntersectionPoint( lViewPositionVec3, lWorldPositionVec3, kfAtmosphereSize, lViewPositionVec3, lFarPosition );

    // Light scattering
    lfInScattering = InScattering(
        lWorldPositionVec3,
        lLightPositionVec3,
        lViewPositionVec3,
        kfAtmosphereSize,
        lfHorizonFadeSpeed,
        lfAtmosphereThickness,
#if defined( D_REFLECT ) || defined( D_PLATFORM_SWITCH )
        true,
#else
        false,
#endif
        false );

    // Sky colouring
    lSkyScatteringVec3 = InScatteringPhase(
        lfInScattering,
        float2vec3( 1.0 ),
        lfSunStrength,
        lfHorizonFadeSpeed,
        RayleighPhase( lLightPositionVec3, lWorldPositionVec3 - lViewPositionVec3 ) );

    // Sky colouring
    vec3 lSolarScatteringVec3 = InScatteringPhase(
        lfInScattering,
        float2vec3( 1.0 ),
        lfSolarFadeStrength,
        1.0,
        MiePhase( lSunPositionVec3, lWorldPositionVec3 - lViewPositionVec3, lfSolarFadeSize ));

    // Shift scattering to cover full range
    vec3 lAdjustedScatteringVec3;
    lAdjustedScatteringVec3.x    = smoothstep( min( 0.1 + lfBlendTightness * lGradientSpeed.x, 0.45 ), max( 0.8 - lfBlendTightness * lGradientSpeed.x, 0.451 ), lSkyScatteringVec3.x * lfHorizonMultiplier );
    lAdjustedScatteringVec3.y    = smoothstep( min( 0.1 + lfBlendTightness * lGradientSpeed.y, 0.45 ), max( 0.8 - lfBlendTightness * lGradientSpeed.y, 0.451 ), lSkyScatteringVec3.x * lfHorizonMultiplier );
    lAdjustedScatteringVec3.z    = smoothstep( min( 0.1 + lfBlendTightness * lGradientSpeed.z, 0.45 ), max( 0.8 - lfBlendTightness * lGradientSpeed.z, 0.451 ), lSkyScatteringVec3.x * lfHorizonMultiplier );
    vec3 lScatteringColourVec3   = mix( lSkyColour, lSkyColour * max( lSkyScatteringVec3.x, 1.0 ), lfIsInSpace );

    lScatteringColourVec3        = mix( lSkyUpperColour,       lSkyColour,      saturate( max( lSkyScatteringVec3.x - lfUpperSkyFadeOffset, 0.0 ) * lfUpperSkyFadeSpeed * lGradientSpeed ) );
    lScatteringColourVec3        = mix( lScatteringColourVec3, lSkySolarColour, saturate( lSolarScatteringVec3    * lGradientSpeed ) );
    lFragmentColourVec4.rgb      = mix( lScatteringColourVec3, lHorizonColour,  saturate( lAdjustedScatteringVec3 * lGradientSpeed ) );

    lFragmentColourVec4.a        = lfSkyAlpha;
    lFragmentColourVec4.a        = mix( lFragmentColourVec4.a, lfHorizonAlpha, smoothstep( 0.1, 0.55, lSkyScatteringVec3.x * lfHorizonMultiplier ) );
    lFragmentColourVec4.a        = mix( lFragmentColourVec4.a, smoothstep( 0.0, 0.18, lSkyScatteringVec3.x ), lfIsInSpace );
    //lFragmentColourVec4.a        = saturate( ( lAdjustedScatteringVec3.x * lGradientSpeed.x + lAdjustedScatteringVec3.y * lGradientSpeed.y + lAdjustedScatteringVec3.z * lGradientSpeed.z ) / 3.0 );
    
    vec3 lSunScatteringVec3 = float2vec3( 0.0 );

    if( lfStarsNum <= 1.0 )
    {
        lSunScatteringVec3 += InScatteringPhase(
            lfInScattering,
            lSunColour,
            1.0,
            1.0,
            MiePhase( lSunPositionVec3, lWorldPositionVec3 - lViewPositionVec3, lfSunSize ) );
    }
    else
    {
        vec3 lHSVColour = RGBToHSV( lSunColour * lSunColour * lSunColour );

        // Sun colouring
        lSunScatteringVec3 += InScatteringPhase(
            lfInScattering,
            HSVToRGB( vec3( fract( lHSVColour.r ), lHSVColour.gb ) ),
            0.9,
            1.0,
            MiePhase( lSunPositionVec3, lWorldPositionVec3 - lViewPositionVec3, lfSunSize ) );

        lSunScatteringVec3 += InScatteringPhase(
            lfInScattering,
            HSVToRGB( vec3( fract( lHSVColour.r + 1.0 / lfStarsNum ), lHSVColour.gb ) ),
            0.7,
            1.0,
            MiePhase( lSunPosition2Vec3, lWorldPositionVec3 - lViewPositionVec3, lfSunSize ) );

        // Sun colouring
        if ( lfStarsNum > 2.0 )
        lSunScatteringVec3 += InScatteringPhase(
            lfInScattering,
            HSVToRGB( vec3( fract( lHSVColour.r + 2.0 / lfStarsNum ), lHSVColour.gb ) ),
            0.3,
            1.0,
            MiePhase( lSunPosition3Vec3, lWorldPositionVec3 - lViewPositionVec3, lfSunSize ) );
    }

    lFragmentColourVec4.rgb += clamp(lSunScatteringVec3, float2vec3(0.0), float2vec3(64.0));

  // in deferred this transform is applied after bilateral upscaling at start of VolumetricsUPS
#if defined (D_SCATTERING_FORWARD)
    lFragmentColourVec4.rgb =  MUL( sRGB_TO_P3D65, lFragmentColourVec4.rgb );
#endif

    FRAGMENT_COLOUR = lFragmentColourVec4;

//#if !defined( D_REFLECT ) && !defined( D_SCATTERING )
//    FRAGMENT_DEPTH  = LinearToLogDepth_Pixel( IN( mfLogZ ), lUniforms.mpPerFrame.gClipPlanesVec4 );
//#endif

}

#endif

#ifdef D_CLOUD

STATIC_CONST float HALFPI = 1.570796;
STATIC_CONST float QUARTPI = 0.785398;

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
#if defined(D_PLATFORM_SWITCH)
    // ImpreciseSwitchDepthBuffer:
    // The regular (24-bit) depth buffer on switch doesn't have enough precision at very far (inter-planetary) distances.
    // The 32-bit normalised depth buffer does though, for stuff in the distance we can add an extra explicit check to prevent
    // sorting issues.
    if (IN(mScreenSpacePositionVec4).w > (lUniforms.mpPerFrame.gClipPlanesVec4.y * 0.25))
    {
        vec2 lCoords = (IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w) * 0.5 + 0.5;
        lCoords = SCREENSPACE_AS_RENDERTARGET_UVS(lCoords);
        float lfDepth = DecodeDepthFromColour(texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), lCoords));

        float lfSceneDepth = FastDenormaliseDepth(lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth);
        if (lfSceneDepth < IN(mScreenSpacePositionVec4).w)
        {
            discard;
        }
    }
#endif

    vec4 lFragmentColourVec4 = vec4( 1.0, 0.0, 0.0, 1.0 );

    float lfStormStrengthColouring = lUniforms.mpCustomPerMaterial.gPlanetCloudParams2Vec4.a;

    float lfAdditionalCoverage = ( lUniforms.mpCustomPerMaterial.gPlanetCloudParamsVec4.a * 0.75 ) + ( lUniforms.mpCustomPerMaterial.gPlanetCloudParamsVec4.a * 0.3125 * lfStormStrengthColouring );
    float lfCoverMultiplier = clamp( lUniforms.mpCustomPerMaterial.gPlanetCloudParamsVec4.a, -0.9, 0.0 ) + 1.0;

    float lfDarkSide = dot( normalize( IN( mWorldNormalVec3_mfDistanceFromPlanet ).xyz ), lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz ) * 0.5 + 0.5;
    lfDarkSide       = smoothstep( 0.3, 0.6, lfDarkSide );

    float lfDistanceFade =  IN( mWorldNormalVec3_mfDistanceFromPlanet ).w;
    lfDistanceFade = clamp( ( lfDistanceFade - 10000.0 ) / 3000.0, 0.0, 1.0 );

    vec2 lTexCoords          = IN( mTexCoordsVec4 ).xy + vec2( lUniforms.mpCustomPerMaterial.gPlanetCloudParamsVec4.r, 0.0f );
    float lfDefaultCoverage1 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gCloudMap ), lTexCoords ).a;

    lTexCoords               = IN( mTexCoordsVec4 ).xy + vec2( lUniforms.mpCustomPerMaterial.gPlanetCloudParamsVec4.g, 0.0 );
    float lfDefaultCoverage2 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gCloudMap ), lTexCoords ).a;

    lTexCoords               = IN( mTexCoordsVec4 ).xy + vec2( lUniforms.mpCustomPerMaterial.gPlanetCloudParamsVec4.b, 0.0 );
    float lfDefaultCoverage3 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gCloudMap ), lTexCoords ).a;

    float lfPower1           = lUniforms.mpCustomPerMaterial.gPlanetCloudParams2Vec4.r;
    float lfCoverage1        = sin( pow( lfDefaultCoverage1, lfPower1 ) * HALFPI / 2.0 );

    float lfPower2           = lUniforms.mpCustomPerMaterial.gPlanetCloudParams2Vec4.g;
    float lfCoverage2        = sin( pow( lfDefaultCoverage2, lfPower2 ) * HALFPI / 2.0 );

    float lfPower3           = lUniforms.mpCustomPerMaterial.gPlanetCloudParams2Vec4.b;
    float lfCoverage3        = sin( pow( lfDefaultCoverage3, lfPower3 ) * HALFPI / 2.0 );

    float lfCoverage =  lfCoverage1 + lfCoverage2 + lfCoverage3;

    lFragmentColourVec4.rgb = vec3( lfCoverage, lfCoverage, lfCoverage ) * ( 1.0 - lfStormStrengthColouring ) + 0.5 - ( 0.44 * lfStormStrengthColouring );

    lfCoverage = saturate( ( lfCoverage + lfAdditionalCoverage ) ) * lfCoverMultiplier;

    lFragmentColourVec4.a = smoothstep( 0.05, 0.8, lfCoverage );

#ifdef D_ATMOSPHERE_SHADOW

    lFragmentColourVec4.a    = smoothstep( 0.0, 0.4, lFragmentColourVec4.a ) * 0.7;
    lFragmentColourVec4.rgb  = vec3( 0.0, 0.0, 0.0 );

#endif

    vec3  lToCamera = -normalize( IN( mWorldPositionVec4 ).xyz - lUniforms.mpPerFrame.gViewPositionVec3 );
    vec3  lNormal   = normalize( IN( mWorldNormalVec3_mfDistanceFromPlanet ).xyz );
    float lfFade    = dot( lToCamera, lNormal );

    // Blend out edge fade proportial to additional cloud cover
    lfFade = 1.0f - ( ( 1.0 - lfFade ) * ( 1.0 - lfAdditionalCoverage ) );

    lFragmentColourVec4.a *= clamp( lfFade, 0.0, 1.0 );
    lFragmentColourVec4.a *= lfDistanceFade;

    lFragmentColourVec4.rgb *= lUniforms.mpCommonPerMesh.gLightColourVec4.xyz;
    lFragmentColourVec4.rgb *= lfDarkSide;// * 0.75;

    FRAGMENT_COLOUR = lFragmentColourVec4;
}

#endif