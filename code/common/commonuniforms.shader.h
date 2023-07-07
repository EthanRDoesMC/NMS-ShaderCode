////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonUniforms.h
///     @author     User
///     @date       
///
///     @brief      CommonUniforms
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


#ifndef D_COMMONUNIFORMS2_H
#define D_COMMONUNIFORMS2_H


STATIC_CONST  mat4 kFSQuadProj = mat4(  2.0 / 1280.0, 0.0, 0.0, 0.0,
                                        0.0, -2.0 / 720.0, 0.0, 0.0,
                                        0.0, 0.0, -1.0, 0.0,
                                       -1.0, 1.0, 0.0, 1.0);

#ifdef D_PLATFORM_IOS
#define D_MAX_LIGHT_COUNT 128
#else
#define D_MAX_LIGHT_COUNT 512
#endif

struct sPerInstanceData
{
    vec4 mkTransformMat4R0;
    vec4 mkTransformMat4R1;
    vec4 mkTransformMat4R2;
    vec4 mkTransformMat4R3;
    vec4 mkTransformMat4R4;
    vec4 mkTransformMat4R5;
};

struct sSkinningData
{
    vec4   gaSkinMatrixRowsVec4[75 * 3];
    vec4   gaPrevSkinMatrixRowsVec4[75 * 3];
};

//TF_BEGIN
/// Tiled lighting defines
#define D_TILE_WIDTH 16
#define D_TILE_HEIGHT 16
#define D_TILE_SIZE (D_TILE_WIDTH * D_TILE_HEIGHT)
#define D_TILE_MAX_LIGHT_COUNT 64
//TF_END

#if defined( D_SHADOW ) && !defined( D_PLATFORM_ORBIS ) && !defined( D_FORWARD_RENDERER ) && !defined( D_PLATFORM_METAL )

struct PerFrameUniforms
{
    mat4    gViewMat4 MTL_ID(0);                          // EShaderConst_ViewMat4
    mat4    gViewProjectionMat4;                // EShaderConst_ViewProjectionMat4
    mat4    gCameraMat4;                        // EShaderConst_CameraMat4
    vec3    gViewPositionVec3;                  // EShaderConst_ViewPositionVec3
    float   gfTime;                             // EShaderConst_Time
    vec4    gFrameBufferSizeVec4;

#if defined( D_FORWARD_RENDERER )
//TF_BEGIN
	vec4    gShadowSizeVec4;
	vec3    gShadowProjScaleVec3;               // EShaderConst_ShadowProjScaleVec3
	int     giDisableAmbientAllowed;            // EShaderConst_DisableAmbientAllowed
	vec4    gShadowFadeParamVec4;
	vec4	gRainParametersVec4;
//TF_END
#endif
};

#elif defined( D_TERRAIN ) && !defined( D_PLATFORM_ORBIS )  && !defined( D_FORWARD_RENDERER ) && !defined( D_PLATFORM_METAL )

struct PerFrameUniforms
{
    mat4    gViewMat4 MTL_ID(0);                          // EShaderConst_ViewMat4
    mat4    gViewProjectionMat4;                // EShaderConst_ViewProjectionMat4
    vec3    gViewPositionVec3;                  // EShaderConst_ViewPositionVec3
    float   gfTime;                             // EShaderConst_Time
    vec4    gClipPlanesVec4;                    // EShaderConst_ClipPlanesVec4
    vec4    gTessSettingsVec4;                  // EShaderConst_TessSettingsVec4
    vec4    gFrameBufferSizeVec4;
#if defined( D_FORWARD_RENDERER )
	//TF_BEGIN
	vec4    gShadowSizeVec4;
	vec3    gShadowProjScaleVec3;               // EShaderConst_ShadowProjScaleVec3
	int     giDisableAmbientAllowed;            // EShaderConst_DisableAmbientAllowed
	vec4    gShadowFadeParamVec4;
	vec4	gRainParametersVec4;
	//TF_END
#endif
#if defined( D_TILED_LIGHTS )
	vec4	 gLocalLightDataMultiVec4[D_MAX_LIGHT_COUNT];			// EShaderConst_LocalLightDataMultiVec4
	vec4	 gLocalLightPosMultiVec4[D_MAX_LIGHT_COUNT];			// EShaderConst_LocalLightPosMultiVec4
#endif
	//TF_END
};

#else

struct PerFrameUniforms
{
    mat4    gViewProjectionMat4 MTL_ID(0);                // EShaderConst_ViewProjectionMat4
    mat4    gCameraMat4;                        // EShaderConst_CameraMat4
    mat4    gCameraNoHeadTrackingMat4;          // EShaderConst_CameraNoHeadTrackingMat4

#if defined( D_PLATFORM_ORBIS ) || !defined(D_DEPTHONLY)
    mat4    gPrevViewProjectionMat4;            // EShaderConst_PrevViewProjectionMat4
    mat4    gInverseViewProjectionMat4;         // EShaderConst_InverseViewProjectionMat4
#endif

#if defined( D_PLATFORM_ORBIS ) || ( (!defined( D_UBER )) && (!defined( D_PARTICLE_UNIFORMS ))) || defined( D_FORWARD_RENDERER ) || defined( D_PLATFORM_METAL )
    mat4    gViewMat4;                          // EShaderConst_ViewMat4
    mat4    gProjectionMat4;                    // EShaderConst_ProjectionMat4
    mat4    gInverseProjectionMat4;             // EShaderConst_InverseProjectionMat4
    mat4    gThisToPrevViewProjectionMat4;      // EShaderConst_ThisToPrevViewProjectionMat4
    mat4    gInverseViewMat4;                   // EShaderConst_InverseViewMat4
    mat4    gInverseProjectionSCMat4;           // EShaderConst_InverseProjectionSCMat4
    mat4    gInverseViewProjectionSCMat4;       // EShaderConst_InverseViewProjectionSCMat4

    mat4    gProjectionNoJitterMat4;                    // EShaderConst_ProjectionNoJitterMat4 
    mat4    gViewProjectionNoJitterMat4;                // EShaderConst_ViewProjectionNoJitterMat4
    mat4    gInverseViewProjectionNoJitterMat4;         // EShaderConst_InverseViewProjectionNoJitterMat4
    mat4    gPrevViewProjectionNoJitterMat4;            // EShaderConst_PrevViewProjectionNoJitterMat4
    mat4    gPrevInverseViewProjectionNoJitterMat4;     // EShaderConst_PrevInverseViewProjectionNoJitterMat4;
    mat4    gPrevInverseViewProjectionSCMat4;           // EShaderConst_PrevInverseViewProjectionSCMat4;
    vec3    gPrevViewPositionVec3;                      // EShaderConst_PrevViewPositionVec3
    int     giAntiAliasingIndex;                         // EShaderConst_AntiAliasingIndex
    vec4    gMBlurSettingsVec4;                 // EShaderConst_BlurSettingsVec4
    vec4    gTaaSettingsVec4;                   // EShaderConst_TaaSettingsVec4
    vec4    gTessSettingsVec4;                  // EShaderConst_TessSettingsVec4
    vec4    gShellsSettingsVec4;                // EShaderConst_ShellsSettingsVec4
#endif
#if defined( D_PLATFORM_ORBIS ) || (!defined( D_UBER )) 
    vec4    gFoVValuesVec4;                     // EShaderConst_FoVValuesVec4
#endif

    vec3    gViewPositionVec3;                  // EShaderConst_ViewPositionVec3
    float   gfTime;                             // EShaderConst_Time
    vec3    gVREyeInfoVec3;                     // EShaderConst_VREyeInfoVec3;
    float   gfPrevTime;                         // EShaderConst_PrevTime
    vec4    gClipPlanesVec4;                    // EShaderConst_ClipPlanesVec4
    vec3    gDeJitterVec3;                      // EShaderConst_DeJitterVec3
    int     giDisableAmbientAllowed;            // EShaderConst_DisableAmbientAllowed
    vec4    gFrameBufferSizeVec4;  

#if defined( D_PLATFORM_ORBIS ) || (!defined( D_DEFER ) && !defined( D_DEPTHONLY ))
    vec4    gInverseWorldUpMatVec4[3];          // EShaderConst_InverseWorldUpMatVec4
    vec4    gShadowSizeVec4;  
    vec3    gShadowProjScaleVec3;               // EShaderConst_ShadowProjScaleVec3
    int     giFrameIndex;                       // EShaderConst_FrameIndex;
    vec4    gShadowFadeParamVec4;
#endif
#if defined(D_PLATFORM_ORBIS)
    vec4    gFrustumTanFovVec4;                 // EShaderConst_FrustumTanFovVec4 LRTB
#endif
	//TF_BEGIN
#if defined( D_TILED_LIGHTS ) || defined( D_PLATFORM_METAL )
	// Light Data (x - packed10 direction, y - packed8 color, z - falloff type, w - fov)
	vec4	 gLocalLightDataMultiVec4[D_MAX_LIGHT_COUNT];			// EShaderConst_LocalLightDataMultiVec4
	vec4	 gLocalLightPosMultiVec4[D_MAX_LIGHT_COUNT];		    // EShaderConst_LocalLightPosMultiVec4
#endif
#if defined(D_MSAA_RESOLVE) || defined( D_PLATFORM_METAL )
	int		gMSAASamples;						// EShaderConst_MSAASamples
#endif
#if defined( D_PLATFORM_METAL )
    float   gfMipLodBias;                       // EShaderConst_MipLodBias

    // atmospheric scattering
    vec4 gaFramePlanetPositionsVec4[ 6 ];
    vec4 gaFramePlanetColoursVec4[ 6 ];

    vec4 gFrameSkyUpperParamsVec4;
    vec4 gFrameSkyUpperColourVec4;
    vec4 gFrameSkySolarColourVec4;
    vec4 gFrameSkyGradientSpeedVec4;

    vec4 gFrameSkyColourVec4;
    vec4 gFrameHorizonColourVec4;
    vec4 gFrameSunColourVec4;
    vec4 gFrameScatteringParamsVec4;
    vec4 gFrameSunPositionVec4;

    vec4 gFrameFogFadeHeightsVec4;
    vec4 gFrameFogFadeHeights2Vec4;
    vec4 gFrameFogFadeHeights3Vec4;
    vec4 gFrameFogParamsVec4;
    vec4 gFrameFogColourVec4;
    
    vec4 gFrameHeightFogColourVec4;
    vec4 gFrameSpaceScatteringParamsVec4;
    vec4 gFrameSpaceFogParamsVec4;

    vec4 gFrameWaterFogVec4;
    vec4 gFrameHeightFogParamsVec4;
    vec4 gFrameSpaceHorizonColourVec4;
    vec4 gFrameSpaceSkyColourVec4;

    vec4 gFrameSpaceSkyColour1Vec4;
    vec4 gFrameSpaceSkyColour2Vec4;
    vec4 gFrameSpaceSkyColour3Vec4;
    vec4 gFrameSpaceFogColourVec4;
    vec4 gFrameSpaceFogColour2Vec4;


#endif
//TF_END
};

#endif

struct CommonPerMeshUniforms
{    
    mat4    gWorldMat4 MTL_ID(0);                            // EShaderConst_WorldMat4
#if defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_SWITCH )
    mat4    gWorldViewProjectionMat4;              // EShaderConst_WorldViewProjectionMat4
#endif

#if !defined( D_SHADOW ) || defined( _F60_ACUTE_ANGLE_FADE )
	mat4    gWorldNormalMat4;                      // EShaderConst_WorldNormalMat4
#endif

#if defined( D_AABB )
    vec4    gAabbMinVec4;
    vec4    gAabbMaxVec4;
#endif

#if defined( D_INSTANCE ) && defined( D_INSTANCE_BASE_INDEX )
    uint    guInstBaseIndex;
#endif

#if defined( D_FADE ) && !defined( D_INSTANCE )
    float   gfFadeValue;                           // EShaderConst_FadeValue
#else
    float   fdFadeValueDummy;
#endif
    int     giShaderContextVariant;
    int     giLodIndex;

    // These are planet specific. Should they be here?
    vec4 gPlanetPositionVec4;

#if defined( D_PLANET )
    vec4 gPlanetToViewVec4;
    vec4 gPlanetToViewNormalVec4;
    vec4 gPlanetToViewTangentVec4;
#endif

#if !defined( D_DEPTHONLY )

#if !defined( D_DEFER ) && !defined( D_TERRAIN_EDITS ) && !defined( D_PARTICLE_UNIFORMS ) && !defined( D_SHADOW )
    // These shouldn't be per mesh, the should be per rendertarget. BUT we need to add them to the enum
    // in egShader and SetPErRenderTargetUniforms for that to work and we're trying to do a build for sony
    // so that will have to wait. (also probably need a way of setting per RT uniforms from Game).
#if  !defined( _F50_DISABLE_POSTPROCESS ) || defined( D_FORWARD_RENDERER )
    vec4    gScanParamsPosVec4;
    vec4    gScanParamsCfg1Vec4;
    vec4    gScanParamsCfg2Vec4;
    vec4    gScanParamsCfg3Vec4;
    vec4    gScanColourVec4;
#endif
    mat4    gaShadowMat4[3];                  // EShaderConst_ShadowMat4
#endif

    #if defined( D_SPOTLIGHT_MULTI )
     vec4    gLightColourMultiVec4[D_MAX_LIGHT_COUNT];              // EShaderConst_LightColourVec4
     vec4    gLightCustomParamsMultiVec4[D_MAX_LIGHT_COUNT];        // EShaderConst_LightCustomParamsMultiVec4

    #endif
     vec4    gLightColourVec4;                   // EShaderConst_LightColourVec4
     vec4    gLightCustomParamsVec4;             // EShaderConst_LightCustomParamsVec4
   

#if !defined( D_TERRAIN )
    mat4    gWorldMotionMat4;                      // EShaderConst_WorldMotionMat4
#if defined( D_DEFERRED_DECAL ) || defined( D_PLATFORM_METAL )
    mat4    gInverseModelMat4;                     // EShaderConst_InverseModelMat4
#else
    mat4    gInverseModelMat4Dummy;
#endif 
#endif

#endif

#ifdef D_IBL
    vec4    gGenericParam0Vec4;                    // EShaderConst_GenericParam0Vec4
    vec4    gGenericParam1Vec4;                    // EShaderConst_GenericParam1Vec4
#else
    vec4    gUserDataVec4;
#endif

#if defined( _F20_PARALLAXMAP )
    vec4 gParallaxMapDataVec4;
#endif

#if defined( _F30_REFRACTION )
    vec4 gRefractionParamsVec4;
#endif

#if defined( _F15_WIND)
    vec4 gWindParams1Vec4; // this would likely have to go in material uniforms so we can change it per tree
    vec4 gWindParams2Vec4; // this would likely have to go in material uniforms so we can change it per tree
#endif

#if ( ( (defined( D_DEFER ) && defined( _F20_PARALLAXMAP ) ) || !defined( D_DEFER )) && !defined( D_TERRAIN_EDITS ) && !defined ( D_SHADOW ) ) || defined( _F44_IMPOSTER ) || defined( _F19_BILLBOARD )
    vec4    gLightPositionVec4;                 // EShaderConst_LightPositionVec4
    vec4    gLightDirectionVec4;                // EShaderConst_LightDirectionVec4
    vec4    gLightOriginVec4;
#endif

#if defined( D_INSTANCE ) && ( defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_METAL ) )
    REGULARBUFFER( sPerInstanceData, gaPerInstanceData, D_PER_INSTANCE_BUFFER_ID );
#endif
#if defined( D_INSTANCE_BASE_INDEX ) && ( defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_METAL ) )
    RW_REGULARBUFFER( uint, gaPerInstanceCull, D_PER_INSTANCE_CULL_BUFFER_ID );
#endif


#if defined( D_SKINNING_UNIFORMS ) && ( defined( D_VERTEX ) || defined( D_PLATFORM_SWITCH ) || defined( D_PLATFORM_METAL ) )
    vec4   gaSkinMatrixRowsVec4[75 * 3];
    vec4   gaPrevSkinMatrixRowsVec4[75 * 3];
#endif    //  have particle shader use a particlecommon instead of uber, and put these into it.
#if defined( D_PARTICLE_UNIFORMS )
    vec2    gParticlePivotOffsetVec2;                    // EShaderConst_ParticlePivotOffsetVec2
    float   gfParticleSoftFadeStrength;                  // EShaderConst_ParticleSoftFadeStrength
    int     guParticleSettingBits;                       // EShaderConst_ParticleSettingBits
    vec4    gaParticleCornersVec4[4];                    // EShaderConst_ParticleCornersVec4
    vec4    gaParticlePositionsVec4[32];                 // EShaderConst_ParticlePositionsVec4
    vec4    gaParticleSizeAndRotationsVec4[32];          // EShaderConst_ParticleSizeAndRotationsVec4
    vec4    gaParticleNormalsVec4[32];                   // EShaderConst_ParticleNormalsVec4
    vec4    gaParticleColoursVec4[32];                   // EShaderConst_ParticleColoursVec4
#endif

#if defined( D_SPOTLIGHT ) || defined( D_SPOTLIGHT_MULTI )
#if defined( D_SPOTLIGHT_MULTI )
    vec4 gSpotlightPositionMultiVec4[D_MAX_LIGHT_COUNT];
    vec4 gSpotlightDirectionMultiVec4[D_MAX_LIGHT_COUNT];
    vec4 gSpotlightUpMultiVec4[D_MAX_LIGHT_COUNT];
#elif !defined( D_SPOTLIGHT_MULTI )
    vec4 gSpotlightPositionVec4;
    vec4 gSpotlightDirectionVec4;
    vec4 gSpotlightUpVec4;
#endif
#endif
#if defined( D_LINE_UNIFORMS )
    vec4 gRuntimeLaserParamsVec4;
#endif

//#if defined( D_SKINNING_UNIFORMS ) && defined( D_VERTEX ) && defined ( D_PLATFORM_ORBIS )
//    REGULARBUFFER(sSkinningData, gaSkinningData, 1);
//#endif

};


#endif

