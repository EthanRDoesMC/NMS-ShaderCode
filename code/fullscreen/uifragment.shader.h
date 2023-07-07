////////////////////////////////////////////////////////////////////////////////
///
///     @file       UIFragment.h
///     @author     User
///     @date       
///
///     @brief      DebugFragment
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/CommonUtils.shader.h"
#include "Fullscreen/PostCommon.h"

#include "Common/CommonPostProcess.shader.h"
#include "Common/Noise3D.glsl"
#include "Common/CommonFragment.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/ACES.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

        INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )

DECLARE_INPUT_END

#if defined ( D_UI )

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lCoordsVec2         = TEX_COORDS.xy;
    vec2 lDeformedCoordsVec2 = GetBulgeDeform( lCoordsVec2 );

    vec3 lFragmentColourVec3 = vec3( 0.0,0.0,0.0 );

    float lfOverallMagnitude = lUniforms.mpCustomPerMesh.gUIDeformVec4.x;
    if( lfOverallMagnitude > 0.0 )
    {
        float lfFlickerAmount = lUniforms.mpCustomPerMesh.gUIDeformVec4.y;
        lDeformedCoordsVec2.x +=
            ( ( lfFlickerAmount * 0.1 ) + 2.0 * ( max( sin( lUniforms.mpPerFrame.gfTime + sin( lUniforms.mpPerFrame.gfTime * 113.0 ) ), 0.98 ) - 0.98 ) ) *
            0.05 *
            sin( 113.0 * lUniforms.mpPerFrame.gfTime * sin( lDeformedCoordsVec2.y * 827.0 ) ) *
            lfOverallMagnitude;
    }

    if( lDeformedCoordsVec2.y > 1.0 || lDeformedCoordsVec2.y < 0.0 )
    {
        discard;
    }

    vec4 lUIColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIMap ), lDeformedCoordsVec2 );
    if ( lUIColourVec4.a >= 1.0 )
    {
        discard;
    }

    WRITE_FRAGMENT_COLOUR( lUIColourVec4 );
}

#elif defined( D_BINOCS )

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lCoordsVec2            = TEX_COORDS.xy;    
    vec4 lFragmentColourVec4    = float2vec4(0.0);
    float lfBinocularsOn        = lUniforms.mpCustomPerMesh.gVignetteVec4.w;

    // Binoculars
    lFragmentColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIZoomEffect ), lCoordsVec2, 0.0 );

    if( lfBinocularsOn == 0.0 )
    {
        lFragmentColourVec4.a = 0.0;
    }
    else
    {
        if( lfBinocularsOn < 2.0 )
        {
            lFragmentColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIZoomEffect ), lCoordsVec2, 0.0 );
        }
        else if( lfBinocularsOn < 3.0 )
        {
            lFragmentColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIMissionSurveyingEffect ), lCoordsVec2, 0.0 );
        }
        else 
        {
            lFragmentColourVec4 = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUISurveyingEffect ), lCoordsVec2, 0.0 );
        }

        lFragmentColourVec4.a *= 1.0 - fract( lfBinocularsOn );
    }

    WRITE_FRAGMENT_COLOUR( lFragmentColourVec4 );
}

#elif defined( D_SCREEN_FADE )

FRAGMENT_MAIN_COLOUR_SRT
{
    WRITE_FRAGMENT_COLOUR( lUniforms.mpCustomPerMesh.gFrontendFlashColourVec4 );
}

#elif defined( D_SCREENEFFECT_SWITCH_FSR2 )

vec2
GetColourSeparationRB(
    SAMPLER2DARG( lBufferMap ),
    float lfDistance,
    vec2  lScreenCoordsVec2 )
{
    vec2 lSeparateColourVec2;

    lSeparateColourVec2.x = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x + lfDistance, lScreenCoordsVec2.y ), 0.0 ).x;
    lSeparateColourVec2.y = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x - lfDistance, lScreenCoordsVec2.y ), 0.0 ).z;

    return lSeparateColourVec2;
}

#if !defined ( D_PLATFORM_METAL )

float
GetVignette(
    vec2  lScreenCoordsVec2 )
{
    float lfStart     = 0.85;
    float lfVignette  = 16.0 * lScreenCoordsVec2.x * lScreenCoordsVec2.y * ( 1.0 - lScreenCoordsVec2.x ) * ( 1.0 - lScreenCoordsVec2.y );

    lfVignette         = lfVignette + lfStart;
    lfVignette        *= lfVignette;
    lfVignette         = saturate( lfVignette );

    return lfVignette;
}

vec3
GetScanlines(
    float lfTime,
    vec2  lScreenCoordsVec2 )
{
    vec3 lEffectsColourVec3 = vec3( 0.95, 1.05, 0.95 );

    lEffectsColourVec3 *= 0.95  + 0.05  * sin(  10.0 * lfTime + lScreenCoordsVec2.y * 1280.0 ); // Scan lines
    lEffectsColourVec3 *= 0.995 + 0.005 * sin( 110.0 * lfTime );                                // Pulse

    return lEffectsColourVec3;
}

vec3
Scanline(
    vec2  uv,
    float angle,
    vec3  color,
    float size,
    float strength,
    float decay)
{
    uv[1] -= (0.5 + 0.5 * cos(mod(angle, 3.14*2.0) / 2.0));
    uv[1] *= 1000.0 * size;

    float col = 1.0 / uv[1];
    float damp = saturate(pow(abs(uv[0]), decay) + pow(abs(1.0 - uv[0]), decay));
    col -= damp * 0.2;
    col = clamp(col, 0.0, strength);
    return color * col;
}

STATIC_CONST vec3 kLumcoeff = vec3(0.2126, 0.7152, 0.0722);

#endif

vec3
LinearToGamma01(
    vec3 lvLinearColour )
{
    vec3 lvGammaColour;

    lvGammaColour.r = pow( lvLinearColour.r, 0.45454545454 );
    lvGammaColour.g = pow( lvLinearColour.g, 0.45454545454 );
    lvGammaColour.b = pow( lvLinearColour.b, 0.45454545454 );

    return lvGammaColour;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lCoordsVec2              = TEX_COORDS.xy;
    vec2 lScreenspacePositionVec2 = SCREENSPACE_AS_RENDERTARGET_UVS( lCoordsVec2 );
    vec3 lFragmentColourVec3      = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lCoordsVec2, 0 ).rgb;

    // disable in VR or if EEngineSetting_VignetteAndScanlines sets this off
    bool  lbIsUIEnabled         = lUniforms.mpCustomPerMesh.gUIDeformVec4.w > -0.5 && lUniforms.mpPerFrame.gFoVValuesVec4.z == 1.0;

    float lfVignetteCutoffHigh  = lUniforms.mpCustomPerMesh.gVignetteVec4.y;
    float lfRedFlash            = lUniforms.mpCustomPerMesh.gCriticalHitPointsVec4.x;
    float lfFullScreenDesat     = lUniforms.mpCustomPerMesh.gCriticalHitPointsVec4.y;
    float lfRedVignetteStrength = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.x;
    float lfLowHealthPulseRate  = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.y;
    float lfEffectProgression   = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.w;
    float lfShieldDownScanline  = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.z;

    if( lbIsUIEnabled )
    {
        float lfEffectStrength = 1.0 - saturate( lCoordsVec2.y * 4.5 );
        
        if ( lfEffectStrength > 0.003 )
        {
            // Colour Separation
            vec2 lSeparateColourVec2;
            lFragmentColourVec3.rgb     = lFragmentColourVec3.rbg;
            lSeparateColourVec2         = GetColourSeparationRB( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ), 3.0 / 1280.0, lCoordsVec2 );
            lFragmentColourVec3.rg      = mix( lFragmentColourVec3.rg, lSeparateColourVec2, lfEffectStrength );
            lFragmentColourVec3.rgb     = lFragmentColourVec3.rbg;

            // Vignette and scanlines
            vec3  lEffectColourVec3     = lFragmentColourVec3;
			#if defined ( D_PLATFORM_METAL )
			float lfVignette            = GetVignette( lScreenspacePositionVec2 ).x;	
			#else
            float lfVignette            = GetVignette( lScreenspacePositionVec2 );
			#endif
            vec3 lScanlineVec3          = GetScanlines( lUniforms.mpPerFrame.gfTime, lCoordsVec2 );
            lEffectColourVec3          *= lScanlineVec3;
            lEffectColourVec3          *= lfVignette;
            lFragmentColourVec3         = mix( lFragmentColourVec3, lEffectColourVec3, lfEffectStrength );
        }
    }

    // Blended effects
    {
        float lfAlpha = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lCoordsVec2, 0 ).r;

        if ( lfAlpha < 0.99 )
        {
            vec3  lvBlendedEffects  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lCoordsVec2, 0 ).rgb;
            lFragmentColourVec3     = lFragmentColourVec3 * lfAlpha + lvBlendedEffects;
        }
    }

    // Shield down scanline
    float lfScanLineValue       = abs( lfShieldDownScanline );
    float lfScanLineIntensity   = 1.0 - abs( lfScanLineValue - 0.5 ) * 2.0;

    if ( lfScanLineIntensity > 0.0 )
    {
        vec2  lScanlineCoords     = lCoordsVec2;
        float lfScanLineDirection = sign( lfShieldDownScanline );

        lFragmentColourVec3 += Scanline( lScanlineCoords, lfScanLineValue * 3.1416 * 2.0, vec3( 0.0, 0.8, 1.0 ) * 1.0, 0.025 * lfScanLineDirection, 0.2, 3.0 ) * lfScanLineIntensity;
        lFragmentColourVec3 += Scanline( lScanlineCoords, lfScanLineValue * 3.1416 * 2.0, vec3( 1.0, 1.0, 1.0 ) * 3.0, 0.25  * lfScanLineDirection, 0.2, 3.0 ) * lfScanLineIntensity;
    }

    float lfLuminance = dot( lFragmentColourVec3, kLumcoeff );

    // Shield down desaturation and overlay effect
    if ( lfEffectProgression > 0.0 )
    {
        float lfDistVignette      = distance( lCoordsVec2, vec2( 0.5, 0.5 ) );
        float lfLowHealthVignette = 1.0 - smoothstep( 0.3, 1.0, lfDistVignette );
        lFragmentColourVec3       = mix( lFragmentColourVec3, float2vec3( lfLuminance ), lfEffectProgression * ( 1.0 - lfLowHealthVignette ) );
    }

    // Desaturation
    if ( lfFullScreenDesat > 0.0 )
    {
        lFragmentColourVec3 = mix( lFragmentColourVec3, float2vec3( lfLuminance ) * 0.8, lfFullScreenDesat );
    }

    // Red flash
    if ( lfRedFlash > 0.0 )
    {
        lFragmentColourVec3.r = mix( lFragmentColourVec3.r, 0.8, lfRedFlash * 0.4 );
    }

    // Red vignette
    if ( lfRedVignetteStrength > 0.0 )
    {
        float lfDistVignette           = distance( lCoordsVec2, vec2( 0.5, 0.5 ) );
        float lfCriticHealthVignette   = 1.0 - smoothstep( 0.05, 1.0, lfDistVignette );
        float lfRedVignettePulseAmount = max( 0.0, sin( lUniforms.mpPerFrame.gfTime * lfLowHealthPulseRate ) );
        lFragmentColourVec3.r         += ( lfVignetteCutoffHigh * 0.7 + 0.3 * lfRedVignettePulseAmount ) * lfRedVignetteStrength * ( 1.0 - lfCriticHealthVignette );
    }

    // Flash from front end. This is independent from HUD/hazard flashes.
    lFragmentColourVec3 = mix( lFragmentColourVec3, lUniforms.mpCustomPerMesh.gFrontendFlashColourVec4.xyz, lUniforms.mpCustomPerMesh.gFrontendFlashColourVec4.w );

    lFragmentColourVec3 = saturate( lFragmentColourVec3 );
    lFragmentColourVec3 = TonemapKodak( lFragmentColourVec3, 1.0 );
    lFragmentColourVec3 = LinearToGamma01( lFragmentColourVec3 );

    ivec2  lPixVec2         = ivec2( lCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );
    float  lfSign           = ( lPixVec2.x & 1 ) == ( lPixVec2.y & 1 ) ? 1.0 : -1.0;
    vec4   lFinalColVec4    = ( lfSign * vec4( lFragmentColourVec3, 1.0 ) + 1.0 ) * ( 511.0 / 1023.0 );

    WRITE_FRAGMENT_COLOUR( lFinalColVec4 );

}

#elif defined( D_BLENDED_EFFECTS )

FRAGMENT_MAIN_T2_SRT( vec3, float )
{
    vec2  lCoordsVec2           = TEX_COORDS.xy;
    half  lfVignetteCutoffLow   = lUniforms.mpCustomPerMesh.gVignetteVec4.x;
    half  lfVignetteCutoffHigh  = lUniforms.mpCustomPerMesh.gVignetteVec4.y;
    half  lfVignetteAdjust      = lUniforms.mpCustomPerMesh.gVignetteVec4.z;
    half4 lvColour              = half4( 0.0, 0.0, 0.0, 1.0 );

    bool  lbIsVignetteEnabled   = false;
    {
        lbIsVignetteEnabled = lUniforms.mpCustomPerMesh.gUIDeformVec4.w > -0.5 && lUniforms.mpPerFrame.gFoVValuesVec4.z == 1.0;
        lbIsVignetteEnabled = lbIsVignetteEnabled || lUniforms.mpCustomPerMesh.gUIDeformVec4.w < -1.0;
        lbIsVignetteEnabled = lbIsVignetteEnabled && lUniforms.mpPerFrame.gFoVValuesVec4.z != 2.0;
    }

    // Full screen vignette
    if( lbIsVignetteEnabled )
    {
        half2   lVignetteXY     =  half2( SCREENSPACE_AS_RENDERTARGET_UVS( lCoordsVec2.xy ) );
        half    lfDistVignette  = distance( lVignetteXY, half2( 0.5, 0.5 ) );

        // Circular vignette. Only have it on the top of the screen
        half    lfDistance      = lVignetteXY.y >= 0.5 ? lfDistVignette : abs( lVignetteXY.x - 0.5 );
        half    lfVignette      = smoothstep( lfVignetteCutoffLow, lfVignetteCutoffHigh, lfDistance );

        lvColour.a             *= 1.0 - lfVignette;
    }

    half  lfHazardProgression   = saturate( 1.0 - lfVignetteAdjust );
    half  lfExtraEffect         = saturate( lfVignetteAdjust - 1.0 );
    lfHazardProgression         = mix( lfHazardProgression, half( 0.0 ), lfExtraEffect );

    if ( lfHazardProgression < 1.0 )
    {
        half3 lFullScreenNormalVec3         = DecodeNormalMapHalf( 
			half4( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIFullscreenNormal ), lCoordsVec2 ) ) );
        half  lfDistortionSeperateAmount    = 64.0 / 1280.0;
        vec2  lDistortedTextureCoords       = lCoordsVec2.xy + vec2( lFullScreenNormalVec3.xy ) * lfDistortionSeperateAmount;
        half4 lUIFullscreenVec4             = half4( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIFullscreenEffect ), lDistortedTextureCoords, 0.0 ) );
        half  lfDistVignette                = distance( lCoordsVec2, vec2( 0.5, 0.5 ) );
        half  lfHazardVignette              = 1.0 - FastSmoothStepHalf( half( 0.3 ), half( 0.7 ), lfDistVignette * half( 2.0 ) );
        half  lfAlphaMask                   = mix( half( 1.0 ) - lfHazardVignette, half( 1.0 ), lfExtraEffect );
        half  lfHeightmapIntensity          = saturate( lUIFullscreenVec4.a - lfHazardProgression ) * lfAlphaMask;
        lfHeightmapIntensity               *= saturate( 1.0 / ( 1.0 - lfHazardProgression ) );
        lfHeightmapIntensity                = mix( lfHeightmapIntensity, half( 1.0 ), lfExtraEffect );

        half  lfHazardIntensity             = lfHeightmapIntensity * step( lfHazardProgression, lUIFullscreenVec4.a );
        lfHazardIntensity                   = saturate( lfHazardIntensity );
        lfHazardIntensity                   = mix( lfHazardIntensity, half( 1.0 ), lfExtraEffect );

        if ( lfHazardIntensity > 0.003 )
        {
            // NOTE(sal): the hazard calculations used to be done in gamma space,
            // but I moved screen effects to linear space, which greatly simplifies our pipeline.
            // Here we use a combination of squared interpolation params and sqrt_fast_0 colours
            // to mimic the old gamma space blending and (cheaply) match the old look

            // Lighting
            half4 lUIRefraction     = half4( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIFullScreenRefraction ), lCoordsVec2 ) );

            #if 0
            // We want always some kind of specular value, a fixed light direction and a 
            // view angle using the hemisphere pointing in the opposite direction will do the trick
            half3 lCameraAtVec3     = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraNoHeadTrackingMat4, 2 );
            half3 lLightDir         = reflect( half3( 0.0, 0.0, 1.0 ), lFullScreenNormalVec3.xyz );
            half3 lViewDir          = normalize( half3( lCameraAtVec3.x, lCameraAtVec3.y, -1.0 ) );
            half  lfSpecularRefl    = lUIRefraction.y * pow( saturate( dot( lLightDir, lViewDir ) ), 256.0 );
            lvColour.rgb           += float2vec3( lfSpecularRefl ) * lfHazardIntensity;
            #endif

            half3 lLightCol         = lUIFullscreenVec4.rgb * half3( sqrt( lUniforms.mpCommonPerMesh.gLightColourVec4.rgb ) * 0.5 + float2vec3( 0.5 ) );
            half3 lDistortedCol     = half3( sqrt( texture2DLod(  SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lDistortedTextureCoords, 0.0 ).rgb ) );
            half3 lHazardColour     = mix( lDistortedCol * ( 1.0 - ( 1.0 - lfHazardVignette ) * lfExtraEffect ), lLightCol, lUIRefraction.x );
            lvColour.rgb           += 2.0 * lDistortedCol * lHazardColour * ( 1.0 - lfHazardIntensity ) * lfHazardIntensity;
            lvColour.rgb           += lHazardColour * lHazardColour * lfHazardIntensity * lfHazardIntensity;
            lvColour.a             *= (1.0 - lfHazardIntensity) * (1.0 - lfHazardIntensity);
        }
    }

    half lfCamouflage = lUniforms.mpCustomPerMesh.gCriticalHitPointsVec4.w;
    // Camouflage
    if( lfCamouflage > 0.0 )
    {
        half4 lCamoCol       = half4( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUICamoEffect ), lCoordsVec2, 0.0 ) );
        half3 lCamoNormal    = half3( DecodeNormalMap( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUICamoNormal ), lCoordsVec2 ) ) );

        half  lfNormalBlend = 0.0;
        if ( lfCamouflage < 1.0 )
        {
            // We want the "tiles" of the camo full-screen effect to grow-outwards whilst finishing at there final alpha (determined by the red
            // channel of the colour texture) - we therefore have a sliding window that will lerp different alpha values from 0 to their target
            // value based on the value in the alpha texture & the fade in window.
            const half lfFadeInWindow = 0.07;
            const half lfInvFadeInWindow = 1.0 / lfFadeInWindow;
            lCamoCol.a      = saturate( ( ( lCamoCol.a - 1.0 ) + ( lfCamouflage * ( 1.0 + lfFadeInWindow ) ) ) * lfInvFadeInWindow );
            lfNormalBlend   = lCamoCol.a;
            lCamoCol.a      = lCamoCol.r * lCamoCol.a;
            lCamoCol.rgb    = half3( 0.2, 0.25, 1.0 );
        }

        half  lfDistortionSeperateAmount = 64.0 / 1280.0;
        vec2  lDistortedBufferCoords     = lCoordsVec2.xy + vec2( lCamoNormal.xy ) * lfDistortionSeperateAmount * lfNormalBlend;
        half3 lDistortedCol              = half3( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lDistortedBufferCoords, 0.0 ).rgb );

        lvColour     *= 1.0 - lfCamouflage;
        lvColour.rgb += lDistortedCol * lfCamouflage;
        lvColour     *= 1.0 - lCamoCol.a;
        lvColour.rgb += lCamoCol.rgb  * lCamoCol.a;
    }

    FRAGMENT_OUTPUT_T0 = vec3( lvColour.rgb );
    FRAGMENT_OUTPUT_T1 = lvColour.a;
}

#else

//float width = 1280.0;
//float height = 720.0;

vec4 GetDropletColour(
    SAMPLER2DARG( lBufferMap ),
    in PerFrameUniforms      lRenderTargetUniforms,
    in CustomPerMeshUniforms lMeshUniforms, 
    vec2 texcoord,
    vec2 lDeformedTexcoordVec2 )
{
    vec3 final = texture2DLod(lBufferMap, texcoord, 0.0).rgb;
    float timer         = lMeshUniforms.gWashParamsVec4.x;
    float resettimer    = lMeshUniforms.gWashParamsVec4.y;
    float dropfade = clamp(resettimer*10.0,0.0,1.0);

    float grainsize = 25.0;
    //texture edge bleed removal
    float fade = 12.0;
    vec2 distortFade = vec2(0.0, 0.0);
    distortFade.x = clamp(lDeformedTexcoordVec2.x*fade,0.0,1.0);
    distortFade.x -= clamp(1.0-(1.0-lDeformedTexcoordVec2.x)*fade,0.0,1.0);
    distortFade.y = clamp(lDeformedTexcoordVec2.y*fade,0.0,1.0);
    distortFade.y -= clamp(1.0-(1.0-lDeformedTexcoordVec2.y)*fade,0.0,1.0); 
    float dfade = 1.0-pow((1.0-distortFade.x*distortFade.y),2.0);

    if (resettimer < 1.0 
#ifdef D_SCREENEFFECT_VR
        && lRenderTargetUniforms.gFoVValuesVec4.z != 2.0
#endif
        )  // don't do droplet effect if not running or in VR
    {
        vec2 wave=vec2(0.0, 0.0);
        vec2 wavecoordR;
        vec2 wavecoordG;
        vec2 wavecoordB;    	
        wave.x = sin((texcoord.x-texcoord.y*4.0)-timer*1.5)*0.25;
        wave.x += cos((texcoord.y*8.0-texcoord.x*12.0)+timer*4.2)*0.5;
        wave.x += sin((texcoord.x*18.0+texcoord.y*16.0)+timer*3.5)*0.25;

        wave.y = sin((texcoord.x*4.0+texcoord.x*5.0)+timer*2.5)*0.25;
        wave.y += cos((texcoord.y*6.0+texcoord.x*12.0)-timer*2.5)*0.5;
        wave.y += sin((texcoord.x*22.0-texcoord.y*24.0)+timer*4.5)*0.25;

        wave = wave*dfade;
        wavecoordR = texcoord-wave*0.002;
        wavecoordG = texcoord-wave*0.003;	
        wavecoordB = texcoord-wave*0.004;
        vec3 wavecolor = vec3(0.0, 0.0, 0.0);
        wavecolor.r = texture2DLod(lBufferMap, wavecoordR, 0.0).r;
        wavecolor.g = texture2DLod(lBufferMap, wavecoordG, 0.0).g;
        wavecolor.b = texture2DLod(lBufferMap, wavecoordB, 0.0).b;
        final = mix(wavecolor,final,dropfade);


        if (dfade > 0.0 && dropfade > 0.0)
        {
            //_SCE_BREAK();
            float noiz = 0.0;
            noiz += snoise(vec3(lDeformedTexcoordVec2*vec2(1280.0/90.0,720.0/250.0)+vec2(0.0,timer*1.2),1.0+timer*0.2))*0.25;
            noiz += snoise(vec3(lDeformedTexcoordVec2*vec2(1280.0/1200.0,720.0/1800.0)+vec2(0.0,timer*0.5),3.0+timer*0.3))*0.75;
           
            float dropletmask = smoothstep(0.02+resettimer,0.03+resettimer,noiz*0.5+0.5);		
            dropletmask = clamp(dropletmask,0.0,1.0);
            if (dropletmask > 0.0)
            {
                float droplet;
                droplet   = clamp(smoothstep(0.0+resettimer,0.5+resettimer,noiz*0.5+0.5),0.0,1.0);
                droplet   = pow(clamp(droplet,0.0,1.0),0.1)*3.0;
                vec2 droplets   = vec2(dFdx(lDeformedTexcoordVec2+vec2(droplet,droplet)).r,dFdy(lDeformedTexcoordVec2+vec2(droplet,droplet)).g);
                vec2 dropcoordR;
                vec2 dropcoordG;	
                vec2 dropcoordB;
                droplets = droplets*dfade;
                dropcoordR = (texcoord-droplets * 1.2);
                dropcoordG = (texcoord-droplets * 1.3);	
                dropcoordB = (texcoord-droplets * 1.5);	
                vec3 dropletcolor = vec3(0.0,0.0,0.0);	
                dropletcolor.r = texture2DLod(lBufferMap, dropcoordR, 0.0).r;
                dropletcolor.g = texture2DLod(lBufferMap, dropcoordG, 0.0).g;
                dropletcolor.b = texture2DLod(lBufferMap, dropcoordB, 0.0).b;
                    
                final = mix(final,dropletcolor,dropletmask*dropfade);
            }
        }
    }

    return vec4(final,1.0);

}

//-----------------------------------------------------------------------------
//      Functions

vec2
GetColourSeparationRB(
    SAMPLER2DARG( lBufferMap ),
    float lfDistance,
    vec2  lScreenCoordsVec2 )
{
    vec2 lSeparateColourVec2;

    lSeparateColourVec2.x = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x + lfDistance, lScreenCoordsVec2.y ), 0.0 ).x;
    lSeparateColourVec2.y = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x - lfDistance, lScreenCoordsVec2.y ), 0.0 ).z;

    return lSeparateColourVec2;
}

#if !defined ( D_PLATFORM_METAL )

STATIC_CONST vec3 kLumcoeff = vec3(0.299, 0.587, 0.114);

vec3
Scanline(
    vec2  uv,
    float angle,
    vec3  color,
    float size,
    float strength,
    float decay)
{
    uv[1] -= (0.5 + 0.5 * cos(mod(angle, 3.14*2.0) / 2.0));
    uv[1] *= 1000.0 * size;

    float col = 1.0 / uv[1];
    float damp = clamp(pow(abs(uv[0]), decay) + pow(abs(1.0 - uv[0]), decay), 0.0, 1.0);
    col -= damp * 0.2;
    col = clamp(col, 0.0, strength);
    return color * col;
}

vec3
GetColourSeparation(
    SAMPLER2DARG( lBufferMap ),
    float lfDistance,
    vec2  lScreenCoordsVec2 )
{
    vec3 lSeparateColourVec3;

    lSeparateColourVec3.r = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x + lfDistance, lScreenCoordsVec2.y ), 0.0 ).x;
    lSeparateColourVec3.g = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x,              lScreenCoordsVec2.y ), 0.0 ).y;
    lSeparateColourVec3.b = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x - lfDistance, lScreenCoordsVec2.y ), 0.0 ).z;

    return lSeparateColourVec3;
}

float
GetVignette(
    vec2  lScreenCoordsVec2 )
{
    float lfStart     = 0.85;
    float lfVignette  = 16.0 * lScreenCoordsVec2.x * lScreenCoordsVec2.y * ( 1.0 - lScreenCoordsVec2.x ) * ( 1.0 - lScreenCoordsVec2.y );

    lfVignette         = lfVignette + lfStart;
    lfVignette        *= lfVignette;
    lfVignette         = saturate( lfVignette );

    return lfVignette;
}

vec3
GetScanlines(
    float lfTime,
    vec2  lScreenCoordsVec2 )
{
    vec3 lEffectsColourVec3 = vec3( 0.95, 1.05, 0.95 );

    lEffectsColourVec3 *= 0.95  + 0.05  * sin(  10.0 * lfTime + lScreenCoordsVec2.y * 1280.0 ); // Scan lines
    lEffectsColourVec3 *= 0.995 + 0.005 * sin( 110.0 * lfTime );                                // Pulse

    return lEffectsColourVec3;
}

#endif

#if defined ( D_OUTPUT_LUMINANCE )
FRAGMENT_MAIN_COLOUR01_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{
    vec2 lCoordsVec2 = TEX_COORDS.xy;
    vec2 lScreenspacePositionVec2 = SCREENSPACE_AS_RENDERTARGET_UVS( lCoordsVec2 );
    vec3 lFragmentColourVec3;
    vec2 lDeformedCoordsVec2;

    #if defined ( D_SCREENEFFECT_VR ) 
    bool  lbIsUIEnabled = false;
    bool  lbIsVignetteEnabled = true;

    lScreenspacePositionVec2.x = (lScreenspacePositionVec2.x - lUniforms.mpPerFrame.gVREyeInfoVec3.y) * lUniforms.mpPerFrame.gVREyeInfoVec3.z;

    lFragmentColourVec3  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ),  lCoordsVec2, 0.0 ).xyz;
    lDeformedCoordsVec2  = lCoordsVec2;
    lbIsUIEnabled = false;
    #else
    bool  lbIsUIEnabled         = lUniforms.mpCustomPerMesh.gUIDeformVec4.w > -0.5 ? true : false;
    lbIsUIEnabled               = lbIsUIEnabled && (lUniforms.mpPerFrame.gFoVValuesVec4.z == 1.0); // disable in VR or if EEngineSetting_VignetteAndScanlines sets this off

    bool  lbIsVignetteEnabled   = lbIsUIEnabled;
    lbIsVignetteEnabled         = lbIsVignetteEnabled || (lUniforms.mpCustomPerMesh.gUIDeformVec4.w < -1.0 ? true : false);
    lbIsVignetteEnabled         = lbIsVignetteEnabled && (lUniforms.mpPerFrame.gFoVValuesVec4.z != 2.0);

    #if defined( D_SCREENEFFECT_SWITCH )
    ivec2  lPixVec2     = ivec2( lCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );

    #if defined( D_FLIP_AND_PACK )
    float  lfSign       = ( lPixVec2.x & 1 ) == ( lPixVec2.y & 1 ) ? 1.0 : -1.0;
    #endif

    lFragmentColourVec3  = textureLoadF( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lPixVec2, 0 ).rgb;
    lDeformedCoordsVec2  = lCoordsVec2;
    #else
    // Get the scene colour
    lFragmentColourVec3 = GetDropletColour( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh,gBufferMap ), DEREF_PTR( lUniforms.mpPerFrame ), DEREF_PTR( lUniforms.mpCustomPerMesh ), lCoordsVec2, lScreenspacePositionVec2 ).xyz;

    lDeformedCoordsVec2 = GetBulgeDeform( lCoordsVec2 );
    #endif
    float lfOverallMagnitude = lUniforms.mpCustomPerMesh.gUIDeformVec4.x;

    if ( lfOverallMagnitude > 0.0 )
    {
        float lfFlickerAmount = lUniforms.mpCustomPerMesh.gUIDeformVec4.y;

        lDeformedCoordsVec2.x +=
            ((lfFlickerAmount * 0.1) + 2.0 * (max(sin(lUniforms.mpPerFrame.gfTime + sin(lUniforms.mpPerFrame.gfTime * 113.0)), 0.98) - 0.98)) *
            0.05 *
            sin(113.0 * lUniforms.mpPerFrame.gfTime * sin(lDeformedCoordsVec2.y * 827.0)) *
            lfOverallMagnitude;
    }
    #endif

    vec2 lTextureCoordinatesVec2 = lCoordsVec2;
    #if defined ( D_SCREENEFFECT_VR )
    lTextureCoordinatesVec2.x = (lTextureCoordinatesVec2.x - lUniforms.mpPerFrame.gVREyeInfoVec3.y) * lUniforms.mpPerFrame.gVREyeInfoVec3.z;
    #endif

    float lfVignetteCutoffLow = lUniforms.mpCustomPerMesh.gVignetteVec4.x;
    float lfVignetteCutoffHigh = lUniforms.mpCustomPerMesh.gVignetteVec4.y;
    float lfVignetteAdjust = lUniforms.mpCustomPerMesh.gVignetteVec4.z;    

    float lfRedFlash = lUniforms.mpCustomPerMesh.gCriticalHitPointsVec4.x;
    float lfFullScreenDesat = lUniforms.mpCustomPerMesh.gCriticalHitPointsVec4.y;
    float lfCamouflage = lUniforms.mpCustomPerMesh.gCriticalHitPointsVec4.w;

    float lfRedVignetteStrength = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.x;
    float lfLowHealthPulseRate = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.y;
    float lfShieldDownScanline = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.z;
    float lfEffectProgression = lUniforms.mpCustomPerMesh.gUILowHealthVignetteVec4.w;

    if( lbIsUIEnabled )
    {
        float lfEffectStrength = 1.0 - saturate( lDeformedCoordsVec2.y * 4.5 );
        
        if ( lfEffectStrength > 0.003 )
        {
            // Colour Separation
            vec2 lSeparateColourVec2;
            lFragmentColourVec3.rgb     = lFragmentColourVec3.rbg;
            lSeparateColourVec2         = GetColourSeparationRB( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap ), 3.0 / 1280.0, lCoordsVec2 );
            lFragmentColourVec3.rg      = mix( lFragmentColourVec3.rg, lSeparateColourVec2, lfEffectStrength );
            lFragmentColourVec3.rgb     = lFragmentColourVec3.rbg;

            // Vignette and scanlines
            vec3  lEffectColourVec3     = lFragmentColourVec3;
			#if defined ( D_PLATFORM_METAL )						
			float lfVignette            = GetVignette( lScreenspacePositionVec2 ).x;
			#else
            float lfVignette            = GetVignette( lScreenspacePositionVec2 );
			#endif

            #if !defined( D_SCREENEFFECT_VR )
            vec3 lScanlineVec3          = GetScanlines( lUniforms.mpPerFrame.gfTime, lDeformedCoordsVec2 );
            lEffectColourVec3          *= lScanlineVec3;
            #endif
            lEffectColourVec3          *= lfVignette;
            lFragmentColourVec3         = mix( lFragmentColourVec3, lEffectColourVec3, lfEffectStrength );
        }
    }


    // Full screen vignette
    vec2 lVignetteXY = lScreenspacePositionVec2.xy;
#if defined ( D_SCREENEFFECT_VR )
    float lVignetteOffset = 0.1;
    lVignetteXY.x += (lUniforms.mpPerFrame.gVREyeInfoVec3.x * 2.0 - 1.0) * lVignetteOffset;
    lVignetteXY.y += lVignetteOffset;
    lVignetteXY.x = clamp(lVignetteXY.x, lUniforms.mpPerFrame.gVREyeInfoVec3.x == 1.0 ? 0.5 : 0.0, lUniforms.mpPerFrame.gVREyeInfoVec3.x == 1.0 ? 1.0 : 0.5);
    float lTop = 0.5 - lVignetteOffset;
#else
    float lTop = 0.5;
#endif
    float lfDistVignette = distance( lVignetteXY, vec2( 0.5, 0.5 ) );

#if !defined ( D_SCREENEFFECT_VR )
    if( lbIsVignetteEnabled )
    {
        //
        // Circular vignette. Only have it on the top of the screen
        //
#if defined ( D_PLATFORM_METAL )
        float d = lScreenspacePositionVec2.y >= lTop ? lfDistVignette : abs( lVignetteXY.x - 0.5 );
#else
        float d = lScreenspacePositionVec2.y >= lTop ? lfDistVignette : distance( lVignetteXY.x, 0.5 );
#endif
        float lfCircleVignette = 1.0 - smoothstep( lfVignetteCutoffLow, lfVignetteCutoffHigh, d );

        lFragmentColourVec3 *= lfCircleVignette;
    }
#endif

    // Blend in the UI colour
    // HAZARD
    float lfHazardProgression = saturate( 1.0 - lfVignetteAdjust );
    float lfExtraEffect       = saturate( lfVignetteAdjust - 1.0 );
    lfHazardProgression       = mix( lfHazardProgression, 0.0, lfExtraEffect );

    if ( lfHazardProgression < 1.0 )
    {
        vec3 lFullScreenNormalVec3 = DecodeNormalMap( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIFullscreenNormal ), lTextureCoordinatesVec2 ) );

        #if defined ( D_SCREENEFFECT_VR )
        float lfDistortionSeperateAmount = 16.0 / 1280.0;
        float lfDistortionSeperateAmountCoords = (lUniforms.mpPerFrame.gVREyeInfoVec3.x * 2.0 - 1.0) * 0.1; 
        vec2 lDistortedBufferCoords = lTextureCoordinatesVec2.xy + lFullScreenNormalVec3.xy * lfDistortionSeperateAmountCoords;
        lDistortedBufferCoords.x = saturate( lDistortedBufferCoords.x );
        lDistortedBufferCoords.x = ( lDistortedBufferCoords.x / lUniforms.mpPerFrame.gVREyeInfoVec3.z ) + lUniforms.mpPerFrame.gVREyeInfoVec3.y;
        #else
        float lfDistortionSeperateAmount = 64.0 / 1280.0;
        vec2 lDistortedBufferCoords = lCoordsVec2.xy + lFullScreenNormalVec3.xy * lfDistortionSeperateAmount;
        #endif
        vec2 lDistortedTextureCoords = lTextureCoordinatesVec2.xy + vec2( lFullScreenNormalVec3.xy ) * lfDistortionSeperateAmount;
        vec4 lUIFullscreenVec4       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIFullscreenEffect ), lDistortedTextureCoords, 0.0 );

        float lfHazardVignette     = 1.0 - smoothstep( 0.3, 1.0, lfDistVignette * 2.0 );
        float lfAlphaMask          = mix( 1.0 - lfHazardVignette, 1.0, lfExtraEffect );
        float lfHeightmapIntensity = saturate( lUIFullscreenVec4.a - lfHazardProgression ) * lfAlphaMask;
        lfHeightmapIntensity      *= saturate( 1.0 / ( 1.0 - lfHazardProgression ) );
        lfHeightmapIntensity       = mix( lfHeightmapIntensity, 1.0, lfExtraEffect );

        float lfHazardIntensity    = lfHeightmapIntensity * step( lfHazardProgression, lUIFullscreenVec4.a );
        lfHazardIntensity          = saturate( lfHazardIntensity );
        lfHazardIntensity          = mix( lfHazardIntensity, 1.0, lfExtraEffect );
        #if defined ( D_SCREENEFFECT_VR )
        lfHazardIntensity         *= ( lfDistVignette < 0.25 ) ? 0.0 : saturate( ( lfDistVignette - 0.25 ) * 11.0 );
        #endif

        if ( lfHazardIntensity > 0.003 )
        {
            // NOTE(sal): the hazard calculations used to be done in gamma space,
            // but I moved screen effects to linear space, which greatly simplifies our pipeline.
            // Here we use a combination of squared interpolation params and sqrt_fast_0 colours
            // to mimic the old gamma space blending and (cheaply) match the old look
            lFragmentColourVec3     = sqrt_fast_1( lFragmentColourVec3 );
            lFragmentColourVec3    *= 1.0 - lfHazardIntensity;

            // Lighting
            vec4 lUIRefraction      = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUIFullScreenRefraction ), lTextureCoordinatesVec2 );

            // We want always some kind of specular value, a fixed light direction and a 
            // view angle using the hemisphere pointing in the opposite direction will do the trick
            vec3 lCameraAtVec3      = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraNoHeadTrackingMat4, 2 );
            vec3 lLightDir          = reflect( vec3( 0.0, 0.0, 1.0 ), lFullScreenNormalVec3.xyz );
            vec3 lViewDir           = normalize( vec3( lCameraAtVec3.x, lCameraAtVec3.y, -1.0 ) );
            float lfSpecularRefl    = lUIRefraction.y * pow( saturate( dot( lLightDir, lViewDir ) ), 256.0 );
            lFragmentColourVec3    += float2vec3( lfSpecularRefl ) * lfHazardIntensity;

            vec3 lLightCol          = lUIFullscreenVec4.rgb * ( sqrt_fast_0( lUniforms.mpCommonPerMesh.gLightColourVec4.rgb ) * 0.5 + float2vec3( 0.5 ) );
            vec3 lDistortedCol      = sqrt_fast_0( texture2DLod(  SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lDistortedBufferCoords, 0.0 ).rgb );
            vec3 lHazardColour      = mix( lDistortedCol * ( 1.0 - ( 1.0 - lfHazardVignette ) * lfExtraEffect ), lLightCol, lUIRefraction.x );
            lFragmentColourVec3    += lHazardColour * lfHazardIntensity;
            lFragmentColourVec3    *= lFragmentColourVec3;
        }
    }

    // Shield down desaturation and overlay effect
    if ( lfEffectProgression > 0.0 )
    {
        float lfLowHealthVignette = 1.0 - smoothstep( 0.3, 1.0, lfDistVignette );
        float lfLuma              = dot( lFragmentColourVec3.rgb, kLumcoeff );
        lFragmentColourVec3       = mix( lFragmentColourVec3.rgb, float2vec3( lfLuma ), lfEffectProgression * ( 1.0 - lfLowHealthVignette ) );
    }
    
    // Shield down scanline
    float lfScanLineValue     = abs( lfShieldDownScanline );
    float lfScanLineIntensity = 1.0 - abs( lfScanLineValue - 0.5 ) * 2.0;
    if ( lfScanLineIntensity > 0.0 )
    {
        vec2 lScanlineCoords = lDeformedCoordsVec2;
        float lfScanLineDirection = sign( lfShieldDownScanline );

        lFragmentColourVec3.rgb += Scanline(lScanlineCoords, lfScanLineValue * 3.1416 * 2.0, vec3(0.0, 0.8, 1.0) * 1.0, 0.025 * lfScanLineDirection, 0.2, 3.0) * lfScanLineIntensity;
        lFragmentColourVec3.rgb += Scanline(lScanlineCoords, lfScanLineValue * 3.1416 * 2.0, vec3(1.0, 1.0, 1.0) * 3.0, 0.25  * lfScanLineDirection, 0.2, 3.0) * lfScanLineIntensity;
    }

    // Desaturation
    {
        float lumFullScreen = dot( lFragmentColourVec3, kLumcoeff );
        lFragmentColourVec3 = mix( lFragmentColourVec3, float2vec3( lumFullScreen ) * 0.8, lfFullScreenDesat );
    }

    // Camouflage
    if( lfCamouflage > 0.0 )
    {
        vec4 lCamoCol       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUICamoEffect ), lTextureCoordinatesVec2, 0.0 );
        vec3 lCamoNormal    = DecodeNormalMap( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gUICamoNormal ), lTextureCoordinatesVec2 ) );

        float lfNormalBlend = 0.0;
        if ( lfCamouflage < 1.0 )
        {
            // We want the "tiles" of the camo full-screen effect to grow-outwards whilst finishing at there final alpha (determined by the red
            // channel of the colour texture) - we therefore have a sliding window that will lerp different alpha values from 0 to their target
            // value based on the value in the alpha texture & the fade in window.
            const float lfFadeInWindow = 0.07;
            const float lfInvFadeInWindow = 1.0 / lfFadeInWindow;
            lCamoCol.a      = saturate( ( ( lCamoCol.a - 1.0 ) + ( lfCamouflage * ( 1.0 + lfFadeInWindow ) ) ) * lfInvFadeInWindow );
            lfNormalBlend   = lCamoCol.a;
            lCamoCol.a      = lCamoCol.r * lCamoCol.a;
            lCamoCol.rgb    = vec3( 0.2, 0.25, 1.0 );
        }


        #ifdef D_SCREENEFFECT_VR
        float lfDistortionSeperateAmount = 16.0 / 1280.0;
        vec2  lDistortedBufferCoords     = lTextureCoordinatesVec2.xy + lCamoNormal.xy * lfDistortionSeperateAmount * lfNormalBlend;
        lDistortedBufferCoords.x         = saturate( lDistortedBufferCoords.x );
        lDistortedBufferCoords.x         = (lDistortedBufferCoords.x / lUniforms.mpPerFrame.gVREyeInfoVec3.z) + lUniforms.mpPerFrame.gVREyeInfoVec3.y;
        #else
        float lfDistortionSeperateAmount = 64.0 / 1280.0;
        vec2  lDistortedBufferCoords     = lCoordsVec2.xy + lCamoNormal.xy * lfDistortionSeperateAmount * lfNormalBlend;
        #endif

        vec3 lDistortedCol = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lDistortedBufferCoords, 0.0 ).rgb;

        lFragmentColourVec3.rgb = mix( lFragmentColourVec3.rgb, lDistortedCol, lfCamouflage );
        lFragmentColourVec3.rgb = mix( lFragmentColourVec3.rgb, lCamoCol.rgb, lCamoCol.a );
    }

    // Red flash
    if ( lfRedFlash > 0.0 )
    {
        lFragmentColourVec3.r = mix( lFragmentColourVec3.r, 0.8, lfRedFlash * 0.4 );
    }

    // Red vignette
    if ( lfRedVignetteStrength > 0.0 )
    {
        float lfCriticHealthVignette   = 1.0 - smoothstep( 0.05, 1.0, lfDistVignette );
        float lfRedVignettePulseAmount = max( 0.0, sin( lUniforms.mpPerFrame.gfTime * lfLowHealthPulseRate ) );
        lFragmentColourVec3.r         += ( lfVignetteCutoffHigh * 0.7 + 0.3 * lfRedVignettePulseAmount ) * lfRedVignetteStrength * ( 1.0 - lfCriticHealthVignette );
    }

    // Flash from front end. This is independent from HUD/hazard flashes.
    lFragmentColourVec3.rgb = mix( lFragmentColourVec3.rgb,  lUniforms.mpCustomPerMesh.gFrontendFlashColourVec4.xyz,  lUniforms.mpCustomPerMesh.gFrontendFlashColourVec4.w );

    #if defined ( D_SCREENEFFECT_VR )
    if (lbIsVignetteEnabled)
    {
        //
        // Circular vignette. Only have it on the top of the screen
        //
        float d = lScreenspacePositionVec2.y >= lTop ? lfDistVignette : abs( lVignetteXY.x - float( 0.5 ) );
        float lfCircleVignette = 1.0 - smoothstep(lfVignetteCutoffLow, lfVignetteCutoffHigh, d);

        lFragmentColourVec3 *= lfCircleVignette;
    }
    #endif

    #if defined ( D_SCREENEFFECT_VR ) || defined( D_SCREENEFFECT_SWITCH )
    // in VR we do the Combine stage here
    {
        lFragmentColourVec3     = saturate( lFragmentColourVec3.rgb );
        lFragmentColourVec3     = TonemapKodak( lFragmentColourVec3.rgb, 1.0 );
        lFragmentColourVec3     = GammaCorrectOutput( lFragmentColourVec3.rgb );
    }
    #endif

    #if !defined( D_FLIP_AND_PACK )
    vec4 lFinalColVec4  = vec4( lFragmentColourVec3, 1.0 );
    #else
    vec4 lFinalColVec4  = ( lfSign * vec4( lFragmentColourVec3, 1.0 ) + 1.0 ) * ( 511.0 / 1023.0 );
    #endif

    #if defined( D_OUTPUT_LUMINANCE ) && defined( D_FLIP_AND_PACK )
    #error Unexpected flag combination
    #endif

    #if !defined( D_OUTPUT_LUMINANCE )
    WRITE_FRAGMENT_COLOUR( lFinalColVec4 );
    #else
    WRITE_FRAGMENT_COLOUR0( lFinalColVec4 );
    #endif

    #if defined( D_OUTPUT_LUMINANCE )
    WRITE_FRAGMENT_COLOUR1( vec4( dot( lFragmentColourVec3, kLumcoeff ), 0.0, 0.0, 1.0 ) );
    #endif
}

#endif