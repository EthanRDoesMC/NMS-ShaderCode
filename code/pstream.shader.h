////////////////////////////////////////////////////////////////////////////////
///
///     @file       
///     @author     hdenholm
///     @date       
///
///     @brief      
///
///     Copyright (c) 2014 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/CommonRandom.shader.h"
#include "Common/CommonNoise.shader.h"
#include "Common/CommonFragment.shader.h"

#if defined ( D_PLATFORM_METAL )
#define D_ENABLE_PSTREAM_USES_VERTEXID
#endif

struct CustomPerMaterialUniforms
{
    vec4 gSpaceCloudColourVec4 MTL_ID(0);
    vec4 gSpaceNebulaColour3Vec4;
    vec4 gSpaceNebulaParamsVec4;
    vec4 gSpaceSkyColour1Vec4;
    vec4 gRenderingInformationVec4;

BEGIN_SAMPLERBLOCK
    SAMPLER2D( gDistortMap );
    SAMPLER2D( gFieldMap );

    SAMPLER2D(  gNoiseMap );
    SAMPLER2D( gPlasmaMap );

END_SAMPLERBLOCK    




struct CustomPerMeshUniforms
{
	vec4 gDoFParamsVec4 MTL_ID(0);
    vec4 gHDRParamsVec4;
};

//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal. Declare it AFTER commonuniforms is included.
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )       /*: PER_MESH*/
     DECLARE_PTR( CustomPerMeshUniforms,        mpCustomPerMesh )       /*: PER_MESH*/
     DECLARE_PTR( CustomPerMaterialUniforms,    mpCustomPerMaterial )       /*: PER_MATERIAL*/
DECLARE_UNIFORMS_END


//-----------------------------------------------------------------------------
//    Functions

float DistanceFalloff(
    in vec4  lv_MinDist_RecipMaxDist,
    in float depth)
{
  return saturate( (depth - lv_MinDist_RecipMaxDist.x) * lv_MinDist_RecipMaxDist.y );
}

#if defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
STATIC_CONST vec2 kCorners[6] = 
{
	vec2(0.f, 0.f),
	vec2(1.f, 0.f),
	vec2(0.f, 1.f),
	vec2(0.f, 1.f),
	vec2(1.f, 1.f),
	vec2(1.f, 0.f)
};
#endif

// =============================================================================================

#ifdef D_CARDS

#ifdef D_VERTEX

#include "Common/CommonDepth.shader.h"

//-----------------------------------------------------------------------------
DECLARE_INPUT
	//TF_BEGIN
	// Input not required when using vertexID
#if !defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
	INPUT(  vec2, mkfParticleCornerId,  POSITION0 )
#endif
	//TF_END
    INPUT(  vec3, mkCustom1Vec4,        TEXCOORD0 )
    INPUT(  vec4, mkCustom2Vec4,        TEXCOORD1 )
    INPUT(  vec4, mkCustom3Vec4,        TEXCOORD2 )
DECLARE_INPUT_END

DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE

    OUTPUT( vec2, mUV,                  TEXCOORD0 )
    OUTPUT( vec4, mDepth_Field_Seed2,   TEXCOORD1 )
    OUTPUT( vec4, mColour,              TEXCOORD2 )
    OUTPUT( vec3, mPlasmaChoice,        TEXCOORD3 )
DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
//TF_BEGIN
#if defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
VERTEX_MAIN_ID_SRT		 // Previously: VERTEX_MAIN_SRT
#else
VERTEX_MAIN_SRT
#endif
//TF_END
{
    vec3 lViewAt        = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 );

    float lParticleSize = IN(mkCustom1Vec4).x;
	//TF_BEGIN
#if defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
	vec2 lParticleUV = kCorners[vertexID];
	vec2 lCornerOffset = lParticleUV - 0.5;
#else
    vec2 lCornerOffset  = IN(mkfParticleCornerId) - 0.5;
#endif
	//TF_END

    vec3 lCardCenter    = IN(mkCustom2Vec4).xyz;
    vec3 lNormal        = IN(mkCustom3Vec4).xyz;

    vec3 lTangent1      = cross( lNormal, vec3( 0.0, 1.0, 0.0) );
    vec3 lTangent2      = cross( lNormal, lTangent1 );

    vec3 lCardEdge      = lCardCenter;
    lCardEdge          += lTangent1 * (lCornerOffset.x * lParticleSize);
    lCardEdge          += lTangent2 * (lCornerOffset.y * lParticleSize);

    vec4 lvPositionToTransformVec4 = vec4( lCardEdge, 1.0 );

    vec4 lvWorldPositionVec4 = MUL( lUniforms.mpCommonPerMesh.gWorldMat4, lvPositionToTransformVec4 );
    vec4 lScreenSpacePositionVec4 = MUL( lUniforms.mpPerFrame.gViewProjectionMat4, lvWorldPositionVec4);


    vec3 lCamPos         = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 3 );
    float lfDistToCamera = distance(lCamPos, lvWorldPositionVec4.xyz);
    float lfDistFromCameraNormalised = DistanceFalloff( lUniforms.mpCommonPerMesh.gLightColourVec4, lfDistToCamera );


    // we choose a fade value based on
    //      - world-point-to-camera angle (to fade as that diminishes)
    //      - alignment to normal (fade as we approach perp)
    //      - fade in and out on camera distance

    vec3 lWorldToCam  = normalize( lCamPos - lvWorldPositionVec4.xyz );
    float lAlpha      = saturate( dot( lWorldToCam, lViewAt ) );

    float lAngle      = abs( dot( lNormal, lViewAt ) );
    lAngle            = smoothstep( 0.5, 1.0, lAngle );

    lAlpha            = min( lAlpha, lAngle );

    lAlpha           *= 1.0 - smoothstep( 0.1, 0.75, lfDistFromCameraNormalised );
    lAlpha           *= smoothstep( 0.01, 0.05, lfDistFromCameraNormalised );


    // turn a 0..1 value into a 3-way choice, producing [1,0,0] . [0,1,0] . [0,0,1] for each third
    // this is then used to choose one channel from the RGB plasma texture
    vec3 lPlasmaShift = float2vec3( IN(mkCustom1Vec4).z );
    vec3 lPlasmaChoice = step( vec3(0.0, 0.33, 0.66), lPlasmaShift ) - step( vec3(0.33, 0.66, 1.0), lPlasmaShift );


    SCREEN_POSITION = lScreenSpacePositionVec4;
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);

	//TF_BEGIN
#if defined ( D_ENABLE_PSTREAM_USES_VERTEXID )	
    OUT(mUV)                  = lParticleUV;
#else
    OUT(mUV)                  = IN(mkfParticleCornerId);
#endif
	//TF_END
    OUT(mDepth_Field_Seed2)   = vec4( IN(mkCustom2Vec4).w, 0.0, IN(mkCustom1Vec4).y, IN(mkCustom3Vec4).a );
    OUT(mColour)              = vec4( 0, 0, 0, lAlpha );
    OUT(mPlasmaChoice)        = lPlasmaChoice;
}

#endif // D_VERTEX



#ifdef D_FRAGMENT

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec2, mUV,                 TEXCOORD0 )
    INPUT( vec4, mDepth_Field_Seed2,  TEXCOORD1 )
    INPUT( vec4, mColour,             TEXCOORD2 )
    INPUT( vec3, mPlasmaChoice,       TEXCOORD3 )
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    float lFade                 = IN(mColour).a;

    float lfDistToCentreOfUV    = distance( vec2(0.5, 0.5), IN(mUV) ) * 2.0;

    // fade out to edges
    lFade -= lfDistToCentreOfUV;

    if ( lFade <= 0.0 )
    {
        discard;
    }
    else
    {
        float lSubtraction          = IN(mDepth_Field_Seed2.x);
        float lRandomUVOffset       = IN(mDepth_Field_Seed2.w);

        // shuffle UV, textures are tiling
        vec2 lSampleUV              = IN(mUV) + vec2( lRandomUVOffset, lRandomUVOffset );

        // sample 3 different plasma images encoded in one rgb tex, mask out just one channel based on random input
        float lPlasma               = dot( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gPlasmaMap ), lSampleUV ).rgb, IN(mPlasmaChoice) );

        FRAGMENT_COLOUR             = vec4( lSubtraction, lSubtraction, lSubtraction, lPlasma * lFade * IN(mDepth_Field_Seed2.z) );
    }
}

#endif // D_FRAGMENT

#endif // D_CARDS


// =============================================================================================


#ifdef D_PSTREAM

#ifdef D_VERTEX

#include "Common/CommonDepth.shader.h"

#if defined( D_PLATFORM_ORBIS )
#pragma argument(indirect-draw)
#endif

//-----------------------------------------------------------------------------
DECLARE_INPUT
#if !defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
    INPUT(  vec2, mkfParticleCornerId,  POSITION0 )
#endif

    INPUT(  vec3, mkCustom1Vec4,        TEXCOORD0 )
    INPUT(  vec4, mkCustom2Vec4,        TEXCOORD1 )
    INPUT(  vec4, mkCustom3Vec4,        TEXCOORD2 )
DECLARE_INPUT_END

DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE

    OUTPUT( vec2, mUV,                  TEXCOORD0 )
    OUTPUT( vec4, mDepth_Field_Seed2,   TEXCOORD1 )
    OUTPUT( vec4, mColour,              TEXCOORD2 )
#ifndef D_PSTREAM_FIELD
    OUTPUT( vec4, mBlends,              TEXCOORD3 )
    OUTPUT( vec4, mWorldPositionVec4,   TEXCOORD4 )
#endif

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
///
///     Vertex Main
///
//-----------------------------------------------------------------------------
//TF_BEGIN
#if defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
VERTEX_MAIN_ID_SRT
#else
VERTEX_MAIN_SRT
#endif
//TF_END
{
    vec4  lScreenSpacePositionVec4;
    float lfLogDepth = 0.0;

    mat4 lTranspViewMat4 = (lUniforms.mpPerFrame.gCameraMat4);
    vec3 lViewUp    = MAT4_GET_COLUMN( lTranspViewMat4, 1 );
    vec3 lViewRight = MAT4_GET_COLUMN( lTranspViewMat4, 0 );

    float lParticleSize = IN(mkCustom1Vec4).x;
    vec3 lParticlePos = IN(mkCustom2Vec4).xyz;
    vec4 lInputColour = IN(mkCustom3Vec4);

#if defined( D_PSTREAM_SOLARSYSTEM )
    // Pre-calculate RNG values for fragment shader here!
    float lfRandomSeedForWhiteCorePrep = lInputColour.a;
    lfRandomSeedForWhiteCorePrep = lfRandomSeedForWhiteCorePrep * lfRandomSeedForWhiteCorePrep * lfRandomSeedForWhiteCorePrep;
    lfRandomSeedForWhiteCorePrep *= lfRandomSeedForWhiteCorePrep;
    float lfRandomSeedForHeroStars = lfRandomSeedForWhiteCorePrep;
    lfRandomSeedForHeroStars *= lfRandomSeedForHeroStars;
    lfRandomSeedForHeroStars *= lfRandomSeedForHeroStars;
    lfRandomSeedForHeroStars *= lfRandomSeedForHeroStars;
    lfRandomSeedForHeroStars = mix( 0.1, 0.5, lfRandomSeedForHeroStars );

	// This is an optimisation to make stars with a smaller corana produce smaller billboards, we are a bit more aggressive with this on weaker platforms due to the savings incurred
#if defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_VULKAN ) || defined( D_PLATFORM_SCARLETT )
    const float lfParticleBillboardResizer = saturate(0.10 + 4.0 * lfRandomSeedForHeroStars);
#else
    const float lfParticleBillboardResizer = saturate(0.05 + 4.0 * lfRandomSeedForHeroStars);
#endif

#endif

#if defined( D_PSTREAM_SOLARSYSTEM )
    {
        float lfSizeMultiplierReferenceResolution = 1080.0;
        float lfSizeMultiplierLowerBound = 0.5;
        float lfSizeMultiplierUpperBound = 2.0;
#if !defined( D_PLATFORM_SWITCH )
        // VR is enabled
        if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
        {
            // In VR due to the wider FOV we more aggressively increase the size of the stars to help reduce flicker.
            lfSizeMultiplierReferenceResolution = 1800.0; // we size stars normally at 1800p
            lfSizeMultiplierLowerBound = 0.5;
            lfSizeMultiplierUpperBound = 2.5;

            // Push the distance of the stars in VR out further by a factor of 10 so that they feel less
            // claustrophobic (this issue isn't noticeable in non-VR).
            const float lfVrDistanceMultiplier = 10.0;
            lParticleSize *= lfVrDistanceMultiplier;
            lParticlePos *= lfVrDistanceMultiplier;
        }

        // When temporal upsampling (e.g DLSS, XeSS, FSR2... ) is enabled - the very effective up-sampling allows us to shrink the stars smaller than
        // would normally work for a given resolution and still not have flicker.
        lfSizeMultiplierReferenceResolution *= lUniforms.mpCustomPerMaterial.gRenderingInformationVec4.x;
#endif

        // Up to 4k smaller stars look nicer.
        float lParticleSizeMultiplier = clamp(lfSizeMultiplierReferenceResolution * lUniforms.mpPerFrame.gFrameBufferSizeVec4.w, lfSizeMultiplierLowerBound, lfSizeMultiplierUpperBound);

        // Push stars out a bit (but keep them appearing at the same size) as it makes them look nicer
        {
            const float lfHardCodedMultiplier = 10.0;
            lParticleSize *= lfHardCodedMultiplier;
            lParticlePos *= lfHardCodedMultiplier;
        }

        lParticleSize *= lParticleSizeMultiplier;
        lInputColour.rgb /= lParticleSizeMultiplier;

        lParticleSize *= lfParticleBillboardResizer;
    }

#endif

#if defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
	//TF_BEGIN
    vec2 lParticleUV = kCorners[vertexID];		// Previously: IN(mkfParticleCornerId);
	//TF_END

    vec2  lCornerOffset = lParticleUV - 0.5;
#else
    vec2  lCornerOffset = IN(mkfParticleCornerId) - 0.5;
#endif

    vec2  lHashValue    = Hash2to3( lParticlePos.xz ).xy;
    
#ifdef D_PSTREAM_FIELD
    float lParticleRandom = IN(mkCustom1Vec4).y;
    lInputColour.a = lParticleRandom;
#endif
    
    
    float lfFieldValue = 0.0;
#ifdef D_PSTREAM_FIELD
	vec3 lvStableNoiseSamplePos = lUniforms.mpCustomPerMesh.gDoFParamsVec4.xyz + (lParticlePos * 0.1);
    lfFieldValue = saturate( 0.5 + (iqNoise3(lvStableNoiseSamplePos) * 0.5) );
#endif


    vec4 lvPositionToTransformVec4 = vec4(lParticlePos, 1.0);

    vec4 lvWorldPositionVec4 = MUL( lUniforms.mpCommonPerMesh.gWorldMat4, lvPositionToTransformVec4 );

    // form view-facing billboard
    lvWorldPositionVec4.xyz += lViewRight * ( lCornerOffset.x * lParticleSize );
    lvWorldPositionVec4.xyz += lViewUp    * ( lCornerOffset.y * lParticleSize );

    lScreenSpacePositionVec4 = MUL( lUniforms.mpPerFrame.gViewProjectionMat4, lvWorldPositionVec4 );

    vec3 lCamPos   = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 3 );
    vec3 lToCamera = lvWorldPositionVec4.xyz - lCamPos;
    float lfDistToCamera = length( lToCamera );
    lToCamera = normalize( lToCamera );

    // gLightColourVec4 xy has the near fade and falloff
    float lfDistFromCameraNormalised = DistanceFalloff( lUniforms.mpCommonPerMesh.gLightColourVec4, lfDistToCamera );
    
    SCREEN_POSITION = lScreenSpacePositionVec4;
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);

#if !defined ( D_ENABLE_PSTREAM_USES_VERTEXID )
    vec2 lParticleUV = IN(mkfParticleCornerId);
#endif

#if defined( D_PSTREAM_SOLARSYSTEM )
    lParticleUV = ((lParticleUV - vec2( 0.5, 0.5 )) * lfParticleBillboardResizer) + vec2( 0.5, 0.5 );
#endif

    vec4 lBlendValues;

#ifdef D_PSTREAM_STARS

    float lfInverseDistanceFromCamera = 1.0 - lfDistFromCameraNormalised;
    lBlendValues.x   = smoothstep( 0.88, 1.0, lfInverseDistanceFromCamera ); // lfColourTransit_DistFromCam
    lBlendValues.y   = smoothstep( 0.96, 1.0, lfInverseDistanceFromCamera ); // lfCoronaBlend_DistFromCam
    lBlendValues.z   = smoothstep( 0.10, 1.0, lfInverseDistanceFromCamera ); // lfGlowFade_DistFromCam

    float lfCoreBlur    = saturate((lfDistFromCameraNormalised - 0.2f) * 1.6f);
    lfCoreBlur          = sin(lfCoreBlur * 1.5707963);
    lBlendValues.w      = lfCoreBlur;

#ifndef D_PSTREAM_SOLARSYSTEM
    // blend a pale neutral blue to the star-colour based on being close to the camera
    //Do this based on the input alpha colour. When set to 1, we don't want this
    float lfBlend = (1.0 - lBlendValues.x) * (1.0 - lInputColour.a);
    vec3 lvTintColor = mix( 
        lInputColour.rgb, 
        vec3(0.873, 0.919, 1.0),
        lfBlend);

    lInputColour.rgb = lvTintColor;
#endif

#endif 

#ifdef D_PSTREAM_FIELD

    lBlendValues        = vec4( saturate( lfDistFromCameraNormalised * lfDistFromCameraNormalised * lfDistFromCameraNormalised ), 0, 0, 0 );

#endif 

    OUT(mUV)                = lParticleUV;
#if defined( D_PSTREAM_SOLARSYSTEM )
    OUT(mDepth_Field_Seed2) = vec4( lfDistFromCameraNormalised, lfFieldValue, lfRandomSeedForWhiteCorePrep, lfRandomSeedForHeroStars);
#else
    OUT(mDepth_Field_Seed2) = vec4( lfDistFromCameraNormalised, lfFieldValue, lHashValue.x, lHashValue.y );
#endif
    OUT(mColour)            = lInputColour;
#ifndef D_PSTREAM_FIELD
    OUT(mBlends) = lBlendValues;
    OUT(mWorldPositionVec4) = vec4( normalize( lParticlePos ), 1.0 );
#else
    OUT(mDepth_Field_Seed2).x = lBlendValues.x;
#endif
	
}


#endif



// =============================================================================================




#ifdef D_FRAGMENT

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec2, mUV,                 TEXCOORD0 )
    INPUT( vec4, mDepth_Field_Seed2,  TEXCOORD1 )
    INPUT( vec4, mColour,             TEXCOORD2 )
#ifndef D_PSTREAM_FIELD
    INPUT( vec4, mBlends,             TEXCOORD3 )
    INPUT( vec4, mWorldPositionVec4,  TEXCOORD4 )
#endif

DECLARE_INPUT_END


#ifdef D_PSTREAM_STARS

#if defined(D_PLATFORM_SWITCH)
precision mediump float;
#pragma optionNV(fastmath on)
#endif

//-----------------------------------------------------------------------------
///
///     Main
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lvUV = IN(mUV);
    vec2 lvSeed = IN(mDepth_Field_Seed2).zw;

    vec4 lBlendValues = IN(mBlends);

#ifdef D_PSTREAM_SOLARSYSTEM
    float lfColourTransit_DistFromCam   = lBlendValues.x;
    float lfCoronaBlend_DistFromCam     = lBlendValues.y;
    float lfGlowFade_DistFromCam        = lBlendValues.z;
#else
    float lfColourTransit_DistFromCam   = 1.0;
    float lfCoronaBlend_DistFromCam     = 1.0;
    float lfGlowFade_DistFromCam        = 1.0;
#endif

    float lfDistToCentreOfUV            = saturate(distance( vec2(0.5, 0.5), lvUV ));
    float lfUvDistFromCircumference     = 1.0 - lfDistToCentreOfUV;
    float lfWhiteCore                   = FastSmoothStep( 0.95,  0.01,  lfUvDistFromCircumference );
    float lfStarCoreAlpha               = lfWhiteCore;

    float lfWhiteCorePrep;
#ifdef D_PSTREAM_SOLARSYSTEM
    {
        // Note: the alpha channel for the input colour is effectively ignored within the context of solar system (non-galaxy map) stars.
        // therefore we hijack it as a random value that is initialised between 0-1.
        // We use this to ever so slightly change the boundary condition for the central "bright dot" that you get in the stars
        // - whilst it might seem subtle - the impact it has is quite important.
        float lfRandomSeed = lvSeed.x;
        float lfBoundaryValue = mix(0.9715, 0.969, lfRandomSeed);
        lfWhiteCorePrep  = FastSmoothStep(lfBoundaryValue, 0.01, lfUvDistFromCircumference);
    }
#else
    float lfCoronaGlow = FastSmoothStep(0.92, 0.04, lfUvDistFromCircumference);
    float lfNoiseGlow = 1.0 - lfCoronaGlow;
    {
        lfWhiteCorePrep = FastSmoothStep(0.94, 0.02, lfUvDistFromCircumference);
    }
#endif
    
    vec3 lvTintColor = IN(mColour.rgb);

    // glow halo
    {
      float lfNearShine = 1.0 / (1.0 + (lfDistToCentreOfUV * 6.0));
            lfNearShine = max( 0.0, lfNearShine - 0.25 ) * 1.4285;

      lfStarCoreAlpha = mix(
          lfStarCoreAlpha,
          1.0,
          saturate((lfNearShine * lfNearShine) * lfGlowFade_DistFromCam ));

#ifdef D_PSTREAM_SOLARSYSTEM
      {
          // Create a rare number of "hero" stars by allowing a very small subset of them to really
          // ramp up their brightness and cornal blur factor - this is a pretty arbitrary way of achieving
          // that - but it seems to produce the desired end-result.
          float lfRandomSeed = lvSeed.y;
          lvTintColor *= max(1.0, lfRandomSeed * 4.0);

          float lMultiplier = lfRandomSeed * 0.034;
          lfStarCoreAlpha *= lMultiplier;
      }
#endif
    }

    vec4 lvOutputCol;
#ifdef D_PSTREAM_SOLARSYSTEM
    {
        lvOutputCol.a = mix(lfStarCoreAlpha, 1.0, lfWhiteCorePrep);
        // This early-out lowers the quality of stars, but is a significant saving on Switch
#if defined( D_PLATFORM_SWITCH )
        if (saturate(lvOutputCol.a - lBlendValues.w) < (1.0 / 511.0))
        {
            discard;
        }
#endif
        lvOutputCol.rgb = lvTintColor;
    }
#else
    {
        vec3 lWhiteCoreColour = vec3( 1.0, 1.0, 1.0 );
        //Make the white core colour more like the tint colour based on the alpha value
        lWhiteCoreColour = mix( lWhiteCoreColour, lvTintColor, IN(mColour).a );

        // the expensive-ish solar flare effect. hence the branch.
        if (lfCoronaBlend_DistFromCam > 0.0)
        {
            float lfTimeScale = 0.01 + (lvSeed.y * 0.01);
            float lfTimeShift = lvSeed.x - (lUniforms.mpCustomPerMesh.gDoFParamsVec4.w * lfTimeScale);  // we borrow gDoFParamsVec4 for some scale/time params

            vec2 lfP2 = (lvUV - 0.5);
            float lfPolar = atan2(lfP2.y, lfP2.x) * 0.1591549430918; // 1 / (2 * Pi)

            float lfDistortV = (lfDistToCentreOfUV * 1.5) + (lfTimeShift * 0.4);

            vec3 lWaveTex = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gDistortMap), vec2((lfTimeScale + lfPolar - lfDistortV), 1.0 * lfDistortV), 0.0).rgb;

            float lfCoronalDistortion = lWaveTex.r;
            lfCoronalDistortion += lWaveTex.g;
            lfCoronalDistortion *= 0.75;


            float lfCoronal = lfCoronalDistortion * lfCoronaBlend_DistFromCam * lfCoronaGlow;

            lfStarCoreAlpha = mix(
                lfStarCoreAlpha,
                0.5 + (lWaveTex.b * 0.5),
                lfCoronal * lfCoronal);


            float lCoreTex = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gDistortMap), vec2(lfPolar, lfTimeShift), 0.0).b;

            lvTintColor = mix(lvTintColor, lvTintColor * 0.4, lCoreTex * lfCoronaBlend_DistFromCam * lfNoiseGlow);

            lvTintColor = mix(lvTintColor, lWhiteCoreColour, smoothstep(0.75, 0.85, lfStarCoreAlpha));

        }
        lvOutputCol = vec4( lvTintColor, lfStarCoreAlpha );

        // push to bright white at the centre, depending on the alpha value in the colour
        vec4 lvTargetCore = mix( lvOutputCol, vec4( lWhiteCoreColour, 1.0 ), lfWhiteCorePrep );
        lvOutputCol       = mix( lvOutputCol, lvTargetCore, max( lfWhiteCore, IN( mColour ).a ) );
    }
#endif

#ifdef D_PSTREAM_SOLARSYSTEM
    {
#if !defined(D_PLATFORM_SWITCH)
        // Cap the maximum brightness of stars whilst preserving the "impact" of brightness by adjusting the alpha threshold appropriately per-pixel
        // This prevents excessive bloom artifacts & adjusts the actual brightness of stars to more reasonable values (although this is very-far from being
        // physically based in any form). - A cap of 1.0 seems to work best which also simplifies the maths.
        {
            float lfMaxBrightness = max(lvOutputCol.r, max(lvOutputCol.g, lvOutputCol.b));

            if (lfMaxBrightness > 1.0)
            {
                lvOutputCol.a   *= lfMaxBrightness;
                lvOutputCol.rgb /= lfMaxBrightness;
            }
        }

        // Twinkle twinkle little stars...
        float lfRandomSeed = IN(mColour).a;
        float lfTime = sin(lUniforms.mpPerFrame.gfTime * 2.0 * lfRandomSeed + (lfRandomSeed * 33.0));
        lfTime = lfTime * 0.25 + 0.75;
        lvOutputCol.a *= lfTime;
#endif
    }
#endif

    // slow-in decay alpha away from camera
    float lfCoreBlur = lBlendValues.w;
    lvOutputCol.a = saturate(lvOutputCol.a - lfCoreBlur);
    if (lvOutputCol.a < (1.0 / 511.0))
    {
        discard;
    }
    else
    {
        FRAGMENT_COLOUR = lvOutputCol;
    }
}

#endif // D_PSTREAM_STARS


#ifdef D_PSTREAM_FIELD

//-----------------------------------------------------------------------------
///
///     frag for the general diffuse particle effect
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    float lfFieldRandom             = IN(mDepth_Field_Seed2).y;
    vec2  lvUV                      = IN(mUV);

    float lfParticleDiffuse         = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gFieldMap ), lvUV ).r;
    if (lfParticleDiffuse == 0.0)
    {
        discard;
    }
    float lfFalloff     = IN(mDepth_Field_Seed2).x;

    lfParticleDiffuse  *= lfFieldRandom;
    lfParticleDiffuse  -= (lfFalloff * 0.4);
    if (lfParticleDiffuse <= 0.0)
    {
        discard;
    }
    lfParticleDiffuse   = saturate( lfParticleDiffuse );

    float lfAlpha       = (lfParticleDiffuse * IN(mColour).a);
    FRAGMENT_COLOUR     = vec4( IN(mColour).rgb, lfAlpha );
}

#endif // D_PSTREAM_FIELD

#endif

#endif // D_PSTREAM