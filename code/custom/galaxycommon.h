////////////////////////////////////////////////////////////////////////////////
///
///     @file       GalaxyCommon.h
///     @author     User
///     @date       
///
///     @brief      GalaxyCommon
///
///     Copyright (c) 2015 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"

#if defined( D_PLATFORM_METAL )
#include "Common/CommonUniforms.shader.h"
#else

STATIC_CONST  mat4 kFSQuadProj = mat4(  2.0 / 1280.0, 0.0, 0.0, 0.0,
                                        0.0, -2.0 / 720.0, 0.0, 0.0,
                                        0.0, 0.0, -1.0, 0.0,
                                       -1.0, 1.0, 0.0, 1.0);

struct PerFrameUniforms
{
    mat4    gViewProjectionMat4 MTL_ID(0);      // EShaderConst_ViewProjectionMat4
    mat4    gCameraMat4;                        // EShaderConst_CameraMat4
    mat4    gCameraNoHeadTrackingMat4;          // EShaderConst_CameraNoHeadTrackingMat4

#if defined( D_PLATFORM_ORBIS ) || defined(SM_CODE) || !defined(D_DEPTHONLY)
    mat4    gPrevViewProjectionMat4;            // EShaderConst_PrevViewProjectionMat4
    mat4    gInverseViewProjectionMat4;         // EShaderConst_InverseViewProjectionMat4
#endif

#if defined( D_PLATFORM_ORBIS ) || !defined( D_UBER )
    mat4    gViewMat4;                          // EShaderConst_ViewMat4
    mat4    gProjectionMat4;                    // EShaderConst_ProjectionMat4
    mat4    gInverseProjectionMat4;             // EShaderConst_InverseProjectionMat4
    mat4    gThisToPrevViewProjectionMat4;      // EShaderConst_ThisToPrevViewProjectionMat4
    mat4    gInverseViewMat4;                   // EShaderConst_InverseViewMat4
    mat4    gInverseProjectionSCMat4;           // EShaderConst_InverseProjectionSCMat4
    mat4    gInverseViewProjectionSCMat4;       // EShaderConst_InverseViewProjectionSCMat4

    mat4    gProjectionNoJitterMat4;                  // EShaderConst_ProjectionNoJitterMat4 
    mat4    gViewProjectionNoJitterMat4;              // EShaderConst_ViewProjectionNoJitterMat4
    mat4    gInverseViewProjectionNoJitterMat4;         // EShaderConst_InverseViewProjectionNoJitterMat4
    mat4    gPrevViewProjectionNoJitterMat4;            // EShaderConst_PrevViewProjectionNoJitterMat4
    mat4    gPrevInverseViewProjectionNoJitterMat4;     // EShaderConst_PrevInverseViewProjectionNoJitterMat4;
    mat4    gPrevInverseViewProjectionSCMat4;           // EShaderConst_PrevInverseViewProjectionSCMat4
    vec3    gPrevViewPositionVec3;                      // EShaderConst_PrevViewPositionVec3
	int     giAntiAliasingIndex;                         // EShaderConst_AntiAliasingIndex

    vec4    gMBlurSettingsVec4;                 // EShaderConst_BlurSettingsVec4
    vec4    gTaaSettingsVec4;                   // EShaderConst_TaaSettingsVec4
    vec4    gTessSettingsVec4;                  // EShaderConst_TessSettingsVec4
    vec4    gShellsSettingsVec4;                // EShaderConst_ShellsSettingsVec4
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

#if defined( D_PLATFORM_ORBIS ) || !defined( D_DEFER )
    vec4    gInverseWorldUpMatVec4[3];          // EShaderConst_InverseWorldUpMatVec4
    vec4    gShadowSizeVec4;  
    vec3    gShadowProjScaleVec3;               // EShaderConst_ShadowProjScaleVec3
    int     giFrameIndex;                       // EShaderConst_FrameIndex;
    vec4    gShadowFadeParamVec4;  
#endif
 #if defined(D_PLATFORM_ORBIS)
    vec4    gFrustumTanFovVec4;                 // EShaderConst_FrustumTanFovVec4 LRTB
#endif
};
#endif


struct TracingFsFxAndFsMapUniforms
{ 
//TF_BEGIN
// Unused
  //vec4    gTraceOrigin ;
  //vec4    gTraceDir;
 //TF_END

	vec4    gTracePDX MTL_ID(0);
	vec4    gTracePDY;
	vec4    gTraceScreenCenter;

	vec4    gGoalCenterDir;
	vec4    gGoalCenterPerpU;
	vec4    gGoalCenterPerpV;

	vec4    gAnoStreakConfig1;
	vec4    gAnoStreakConfig2;
	vec4    gAnoStreakConfig3;

	vec4    gGalacticScale;
	vec4    gInterest;

 //TF_BEGIN
 // Unused
 // vec4    gTiming;
 //TF_END

	vec4    gScreen;
//TF_BEGIN
// Unused
//  vec4    gOriginSS;
	vec4	gIterations;
//TF_END
	vec4    gSunCoreConfig;
	vec4    gSunCoreColour;

	vec4    gBGCellConfig;
	vec4    gBGColourScales;
	vec4    gBGColourConfig;
	vec4    gBGLensFlareColour;
	vec4    gBGLensFlareSpread;

	vec4    gNebulaeStepRange_AlphaPow;
	vec4    gCompositeControlBVCG;
	vec4    gNebulaeTraceConfig;
	vec4    gVignetteLensFlareConfig;

BEGIN_SAMPLERBLOCK

    SAMPLER2D(gNebulaeMap);
    SAMPLER2D(gAtmosMap);
    SAMPLER2D(gNoiseMap);

END_SAMPLERBLOCK




#ifndef D_COMPUTE

#define TEX_COORDS IN(mTexCoordsVec2)

#elif defined(D_PLATFORM_METAL)

//-----------------------------------------------------------------------------
///
///     ComputeOutputUniforms
///
///     @brief      ComputeOutputUniforms
///
///     Refs to output textures for compute quad shaders
//-----------------------------------------------------------------------------


struct ComputeOutputUniforms
{
    vec4                 gOutTextureOffsetSize MTL_ID(0);
    RWIMAGE2D( rgba32f, gOutTexture0 );
    RWIMAGE2D( rgba32f, gOutTexture1 );
    RWIMAGE2D( rgba32f, gOutTexture2 );
    RWIMAGE2D( rgba32f, gOutTexture3 );
    RWIMAGE2D( rgba32f, gOutTexture4 );
};
#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )


#elif defined(D_PLATFORM_ORBIS)

//-----------------------------------------------------------------------------
///
///     ComputeOutputUniforms
///
///     @brief      ComputeOutputUniforms
///
///     Refs to output textures for compute quad shaders
//-----------------------------------------------------------------------------

struct ComputeOutputUniforms
{
    vec4                 gOutTextureOffsetSize;
    RW_Texture2D<float4> gOutTexture0;
    RW_Texture2D<float4> gOutTexture1;
    RW_Texture2D<float4> gOutTexture2;
    RW_Texture2D<float4> gOutTexture3;
    RW_Texture2D<float4> gOutTexture4;
};
#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

#elif defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS )

struct ComputeOutputUniforms
{
    vec4               gOutTextureOffsetSize;
    RWIMAGE2D(rgba32f, gOutTexture0);
    RWIMAGE2D(rgba32f, gOutTexture1);
    RWIMAGE2D(rgba32f, gOutTexture2);
    RWIMAGE2D(rgba32f, gOutTexture3);
    RWIMAGE2D(rgba32f, gOutTexture4);
    RWIMAGE2D(r32f, gOutTextureDepth);
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

#elif defined( D_PLATFORM_GLSL )

RWIMAGE2D( rgba32f, gOutTexture0 );
RWIMAGE2D( rgba32f, gOutTexture1 );
RWIMAGE2D( rgba32f, gOutTexture2 );
RWIMAGE2D( rgba32f, gOutTexture3 );
RWIMAGE2D( rgba32f, gOutTexture4 );

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#elif defined D_PLATFORM_SWITCH

RWIMAGE2D( rgba32f, gOutTexture0 );
RWIMAGE2D( rgba32f, gOutTexture1 );
RWIMAGE2D( rgba32f, gOutTexture2 );
RWIMAGE2D( rgba32f, gOutTexture3 );
RWIMAGE2D( rgba32f, gOutTexture4 );

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#else

RW_Texture2D<float4> gOutTexture0 : register(u0);
RW_Texture2D<float4> gOutTexture1 : register(u1);
RW_Texture2D<float4> gOutTexture2 : register(u2);
RW_Texture2D<float4> gOutTexture3 : register(u3);
RW_Texture2D<float4> gOutTexture4 : register(u4);

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#endif

struct ColourPalettes
{
    vec4    gLargeAreaPrimaryLUT MTL_ID(0) [10];
    vec4    gLargeAreaSecondaryLUT[10];
};



DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,          mpPerFrame )       /*: PER_MATERIAL*/
     DECLARE_PTR( TracingFsFxAndFsMapUniforms,  mpGalaxyMapPerMesh )
     DECLARE_PTR( ColourPalettes,       mpColourPalettesPerMesh  )
#if (defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_SWITCH ) || defined( D_PLATFORM_METAL ) ) && defined D_COMPUTE
     DECLARE_PTR(ComputeOutputUniforms, mpCmpOutPerMesh)   /* hack - marked 'per mesh' so it'll be alloced in cmd buf */
#endif
DECLARE_UNIFORMS_END

#ifndef M_PI
#define M_PI 3.141592653589793
#endif