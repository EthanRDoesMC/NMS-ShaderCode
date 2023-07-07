////////////////////////////////////////////////////////////////////////////////
///
///     @file       UberCommon.h
///     @author     User
///     @date       
///
///     @brief      UberCommon
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "Common/Defines.shader.h"

#define D_UBER

// Griff - working around SDK5 shader compiler bug with billboard/displacement vertex positions
#if defined( D_PLATFORM_ORBIS ) && defined( D_VERTEX )
#if defined( _F12_BATCHED_BILLBOARD ) || defined(_F31_DISPLACEMENT)
#pragma argument (O2; nofastmath; scheduler=minpressure)
#else
#pragma argument (O4; fastmath; scheduler=minpressure)
#endif
#elif defined ( D_PLATFORM_ORBIS )
#pragma argument (O4; fastmath; scheduler=minpressure)
#endif

#ifdef D_PLATFORM_SWITCH
#undef _F20_PARALLAXMAP
#undef _F30_REFRACTION
#undef _F32_REFRACTION_MASK
#undef D_TILED_LIGHTS
#endif

#ifdef _F20_PARALLAXMAP
    #define _F03_NORMALMAP
#endif

#if defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL )
    #define D_NORMALMAPPED
#endif

#if defined( D_NORMALMAPPED ) || defined( _F27_VBTANGENT ) || defined( _F31_DISPLACEMENT )
    #define D_DECLARE_TANGENT
#endif

#if defined( _F25_ROUGHNESS_MASK )  || defined( _F41_DETAIL_DIFFUSE )   || defined( _F42_DETAIL_NORMAL ) || \
    defined( _F24_AOMAP )           || defined( _F40_SUBSURFACE_MASK )  || defined( _F54_COLOURMASK )    || \
    defined( _F44_IMPOSTER )        || defined( _F35_GLOW_MASK )        || defined( _F39_METALLIC_MASK ) || \
    defined( _F32_REFRACTION_MASK )
#define D_MASKS
#endif

#if defined( _F04_FEATURESMAP ) || defined( D_SM_FEATURES_MAP )
#define D_FEATURES
#endif

#if ( defined( _F15_WIND ) || defined( _F19_BILLBOARD ) ) && !defined( D_DEPTHONLY ) && !defined( D_DEPTH_CLEAR ) && !defined( _F12_BATCHED_BILLBOARD ) && !defined( _F44_IMPOSTER )
    #define D_OUTPUT_MOTION_VECTORS
#endif

#if ( defined( _F31_DISPLACEMENT ) && !defined( D_DEPTHONLY ) && !defined( D_DEPTH_CLEAR ) )
    #define D_OUTPUT_MOTION_VECTORS
#endif

#if defined( _F01_DIFFUSEMAP ) || defined( _F03_NORMALMAP ) || defined( _F20_PARALLAXMAP ) || defined( D_IMPOSTERMASKS ) || defined(D_MASKS)
    #define D_TEXCOORDS
#endif

#if defined( _F07_UNLIT ) || defined( D_DEFER )  
#define D_NO_SHADOWS
#endif

#if !defined _F03_NORMALMAP && (defined( _F04_ENVMAP ) || !defined( _F07_UNLIT ) || defined(_F30_REFRACTION) || defined(SM_USE_WORLDNORMAL))
    #define D_USES_VERTEX_NORMAL
#endif

#if (defined( _F58_USE_CENTRAL_NORMAL ) && !defined( D_DEPTHONLY )) || defined( _F60_ACUTE_ANGLE_FADE ) || ( defined( _F36_DOUBLESIDED ) && !defined( D_DEPTHONLY ) )
    #define D_USES_VERTEX_NORMAL
#endif

#if ((defined( _F42_DETAIL_NORMAL ) && !defined( _F16_DIFFUSE2MAP ) && !defined( D_SM_NORMAL2 )) || defined( _F41_DETAIL_DIFFUSE )) 
    #define D_DETAIL
#endif

#if defined( _F02_SKINNED )
    #define D_SKINNING_UNIFORMS
#endif


#if defined( D_IMPOSTER_COLOUR ) || defined( D_IMPOSTER_NORMAL ) || defined( D_IMPOSTER_MASK ) || defined( D_IMPOSTER_VERTEX )
	#define D_IMPOSTER
#endif

#if !defined( D_DEFER ) ||defined(_F09_TRANSPARENT) || defined( D_DEFERRED_DECAL ) || defined( D_OUTPUT_MOTION_VECTORS ) || ( defined( _F60_ACUTE_ANGLE_FADE ) && !defined( D_IMPOSTER ) )
    #define D_USE_SCREEN_POSITION
#endif

#if defined( D_UI_OVERLAY ) || defined( D_OCCLUDED ) || ( ( defined( _F06_BRIGHT_EDGE ) || defined( _F09_TRANSPARENT ) ) && !defined( D_DEFER ) )
    #define D_DEPTH_BASED_EFFECT
#endif


#if (!defined( D_IMPOSTER ) && !defined( D_ZEQUALS )) || defined( _F60_ACUTE_ANGLE_FADE )
	#define D_FADE
#endif

#if !defined( D_DEPTHONLY ) || ( defined( D_DEPTHONLY ) && defined( _F60_ACUTE_ANGLE_FADE ) && !defined( D_IMPOSTER ) ) || defined( _F63_DISSOLVE ) || ( defined( D_FADE )  && !defined( D_UI_OVERLAY ) )
#define D_USES_WORLD_POSITION
#endif

#ifdef _F31_DISPLACEMENT
    #define D_DECLARE_TIME
#endif

#if defined( _F57_ENV_OVERLAY ) && !defined( D_DEPTHONLY ) && !defined( D_DEPTH_CLEAR )
    #define D_ENV_OVERLAY
#endif

//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonUniforms.shader.h"

//-----------------------------------------------------------------------------
///
///     CustomPerMeshUniforms
///
///     @brief      CustomPerMeshUniforms
///
///     Stuff that is only used for these types of meshes.
//-----------------------------------------------------------------------------
struct CustomPerMeshUniforms
{      
    vec4 gUVScrollStepVec4 MTL_ID(0);
    vec4 gCustomParams01Vec4;

#if defined( D_SHADOW )
    
    vec4    gBboxDepthAndClips;

#else

#if defined( D_PARTICLE_UNIFORMS )
    vec4 gHeavyAirFadeOutVec4;
    vec4 gMultiTextureVec4;
#else
    vec4 gCustomParams02Vec4;
    vec4 gObjectColourVec4;

#endif

#endif
};
 
//-----------------------------------------------------------------------------
///
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///     Stuff that is only used for these materials.
//-----------------------------------------------------------------------------
struct CustomPerMaterialUniforms
{
#if defined(_F44_IMPOSTER)
    vec4 gImposterDataVec4 MTL_ID(0);
    vec4 gOctahedralImposterDataVec4;
    vec4 gImposterQualitySettingsVec4;
#endif


#ifdef _F31_DISPLACEMENT
    vec4 gWaveOneAmpAndPosOffsetVec4;
    vec4 gWaveOneFrequencyVec4;
    vec4 gWaveOneFallOffAndSpeedVec4;

    vec4 gWaveTwoAmplitudeVec4;
    vec4 gWaveTwoFrequencyVec4;
    vec4 gWaveTwoFallOffAndSpeedVec4;
#endif

    vec4 gSunPositionVec4;

#if defined( D_SHADOW )

BEGIN_SAMPLERBLOCK

    #ifdef _F47_REFLECTION_PROBE
        SAMPLER2D(gDiffuseMap);
    #else

    #ifdef _F55_MULTITEXTURE
        SAMPLER2DARRAY(gDiffuseMap);
    #else
        SAMPLER2D(gDiffuseMap);
    #endif
    
    #endif // _F47_REFLECTION_PROBE

#else

    // Lighting
    // [ Roughness | NonMetalSpecularScale | Metallic | 0 ]
    vec4 gMaterialParamsVec4;
    vec4 gMaterialColourVec4;                  
    vec4 gMaterialSFXVec4;
    vec4 gMaterialSFXColVec4;
    vec4 gBiomeDataVec4;

#if !defined( D_PARTICLE_UNIFORMS )

#if !defined( D_DEPTHONLY )
    vec4 gTileBlendScalesVec4;
    vec4 gTerrainColour1Vec4;
    vec4 gTerrainColour2Vec4;
#endif


#if !defined(D_DEFER) && !defined( D_DEPTHONLY )

    vec4 gHueOverlayParamsVec4;
    vec4 gSaturationOverlayParamsVec4;
    vec4 gValueOverlayParamsVec4;

    vec4 gAverageColour1Vec4;            
    vec4 gAverageColour2Vec4;   

    vec4 gRecolour1Vec4;
    vec4 gRecolour2Vec4; 

    vec4 gSkyColourVec4;
    vec4 gHorizonColourVec4;
    vec4 gSunColourVec4;
    vec4 gWaterFogColourNearVec4;
    vec4 gWaterFogColourFarVec4;
    vec4 gWaterFogVec4;
    vec4 gHeightFogParamsVec4;
    vec4 gHeightFogColourVec4;
    vec4 gSpaceHorizonColourVec4;
    vec4 gFogColourVec4;
    vec4 gFogParamsVec4;
    vec4 gScatteringParamsVec4;

    vec4 gFogFadeHeightsVec4;
    vec4 gSpaceScatteringParamsVec4;
        
    vec4 gSkyUpperColourVec4;
    vec4 gLightTopColourVec4;
#endif

#if defined( D_LIT_FORWARD ) 
    vec4 gPlaneSpotPositionVec4;
#endif

#if defined( _F63_DISSOLVE )
    vec4 gDissolveDataVec4;
#endif

#if defined(D_UI_OVERLAY) || defined(_F09_TRANSPARENT)
    vec4 gUITransparencyVec4;
#endif
	//TF_BEGIN
#if defined(D_BLOOM)
	vec4 gHDRParamsVec4;
#endif
#if defined(D_DOF)
	vec4 gDoFParamsVec4;
#endif
	//TF_END
#endif

BEGIN_SAMPLERBLOCK  

#ifdef _F47_REFLECTION_PROBE
    SAMPLERCUBE(gDiffuseMap);
    SAMPLER2D(gNormalMap);
#else

#ifdef _F55_MULTITEXTURE
    SAMPLER2DARRAY( gDiffuseMap );
    SAMPLER2DARRAY( gNormalMap );
#else
    SAMPLER2D( gDiffuseMap );
    SAMPLER2D( gNormalMap );
#endif

#endif // _F47_REFLECTION_PROBE


#ifdef _F20_PARALLAXMAP
    SAMPLER2D( gParallaxMap );
#endif

#if !defined( D_PARTICLE_UNIFORMS ) || defined( D_SM_DIFFUSE2 )
    SAMPLER2D(gDiffuse2Map);
#endif

#if !defined( D_PARTICLE_UNIFORMS ) 

//#ifdef _F41_DETAIL_DIFFUSE
    SAMPLER2DARRAY( gDetailDiffuseMap );
//#endif

//#ifdef D_DETAIL
#if defined( D_DETAIL ) && !defined( D_SM_NORMAL2 )
    SAMPLER2DARRAY( gDetailNormalMap );
#else
    SAMPLER2D( gDetailNormalMap );
#endif
//#endif

//#ifdef D_MASKS
#ifdef _F55_MULTITEXTURE
    SAMPLER2DARRAY( gMasksMap );
#else
    SAMPLER2D( gMasksMap );
#endif
//#endif

#if defined( D_FEATURES )
#ifdef _F55_MULTITEXTURE
    SAMPLER2DARRAY( gFeaturesMap );
#else
    SAMPLER2D( gFeaturesMap );
#endif
#endif

#if !defined( D_DEPTHONLY )

#if (!defined( D_RECOLOUR ) && !defined( D_COMBINE ) && !defined( D_DEFER ) ) || defined( D_SM_LIGHT_MAPS )
    SAMPLER2D(gCausticMap);
    SAMPLER2D(gCausticOffsetMap);
    SAMPLER2DSHADOW(gShadowMap);
    SAMPLER2D(gCloudShadowMap);
    SAMPLER2D(gDualPMapFront);
    SAMPLER2D(gDualPMapBack);
#endif

#if defined( D_ENV_OVERLAY )
    SAMPLER2D(gOverlayDiffuseMap);
    SAMPLER2D(gOverlayNormalMap);
    SAMPLER2D(gOverlayMasksMap);
#endif

    SAMPLER2D(gNoiseMap);
    SAMPLER3D(gPerlin3D);

#endif // D_DEPTHONLY 

#else
    // move to particle common
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAY(gDepthBuffer);
#else
    SAMPLER2D(gDepthBuffer);
    //TF_BEGIN
    #if defined(D_PLATFORM_METAL)	
    ISAMPLER2D( uint, gStencilBuffer );
    #endif
    //TF_END
#endif

#ifndef _F07_UNLIT
    SAMPLER2D(gDualPMapFront);
    SAMPLER2D(gDualPMapBack);
#endif

#endif

#endif


#if defined( D_FADE ) || defined( D_NOISE_MAP ) || defined( _F33_SHELLS ) || defined( D_PLATFORM_SWITCH )
    SAMPLER2D(gFadeNoiseMap);
#endif

#if defined( _F09_TRANSPARENT ) || defined( D_DEFERRED_DECAL ) || defined( D_UI_OVERLAY ) || defined( D_OCCLUDED ) || defined( D_DRAW_INDEX ) || defined( _F06_BRIGHT_EDGE  ) || ( defined( _F30_REFRACTION ) && defined( D_PARTICLE_UNIFORMS ) ) || defined( D_RAIN )
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAY(gBufferMap);
#else
    SAMPLER2D(gBufferMap);
#endif
#endif

//TF_BEGIN
#if defined(D_TILED_LIGHTS)
    #if defined(D_PLATFORM_METAL)
        RW_DATABUFFER(atomic_int, gLightCluster, D_LIGHT_CLUSTER_BUFFER_ID);
#if !defined(D_PLATFORM_IOS)
        SAMPLER2DARRAY(gLightCookiesMap);
#endif
    #else
        RWINTIMAGE2D( r32i, gLightCluster );
    #endif
#endif
//TF_END

END_SAMPLERBLOCK

#if defined( D_INSTANCE ) && !defined( D_PLATFORM_ORBIS ) && !defined( D_PLATFORM_METAL ) && !defined ( D_FRAGMENT )
REGULARBUFFER(sPerInstanceData, gaPerInstanceData, 0);
#endif

//#if defined( D_SKINNING_UNIFORMS ) && defined( D_VERTEX )  && !defined( D_PLATFORM_ORBIS )
//REGULARBUFFER(sSkinningData, gaSkinningData, 1);
//#endif
//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
 //    DECLARE_PTR( CommonPerMaterialUniforms,    mpCommonPerMaterial )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.

#if  defined( D_PLATFORM_SWITCH) && !defined( D_SKINNING_UNIFORMS ) && defined( D_DEPTHONLY )
     DECLARE_PTR(CommonPerMeshUniforms, mpCommonPerMesh)       /*: PER_MESH*/
     DECLARE_PTR(CustomPerMaterialUniforms, mpCustomPerMaterial)   /*: PER_MATERIAL*/
     DECLARE_PTR(CustomPerMeshUniforms, mpCustomPerMesh)       /*: PER_MESH*/
#else
     DECLARE_PTR(CustomPerMaterialUniforms, mpCustomPerMaterial)   /*: PER_MATERIAL*/
     DECLARE_PTR(CustomPerMeshUniforms, mpCustomPerMesh)       /*: PER_MESH*/
     DECLARE_PTR(CommonPerMeshUniforms, mpCommonPerMesh)       /*: PER_MESH*/
#endif

DECLARE_UNIFORMS_END






