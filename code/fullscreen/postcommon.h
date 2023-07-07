////////////////////////////////////////////////////////////////////////////////
///
///     @file       PostCommon.h
///     @author     User
///     @date       
///
///     @brief      PostCommon
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"


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
    vec4 gDoFParamsVec4 MTL_ID(0);
    vec4 gHDRParamsVec4;
    vec4 gHBAOParamsVec4;
    vec4 gThresholdParamsVec4;
    vec4 gCustomParamsVec4;
    vec4 gBlurParamsVec4;
    vec4 gColourLUTParamsVec4;
    vec4 gColourLUTStrengthsVec4;
#if defined ( D_SHARPEN )
    vec4 gSharpenStrength;
#elif defined ( D_IBL )
    vec4 gIBLParamsVec4;
#else
    vec4 gTextureSizeMipLevelVec4;
#endif
#if defined ( D_UI ) || defined( D_SCREENEFFECT ) || defined( D_BINOCS ) || defined( D_SCREEN_FADE )
    vec4   gUIColourVec4;
    vec4   gUIDeformVec4;
    vec4   gVignetteVec4;
    vec4   gUILowHealthVignetteVec4;
    vec4   gWashParamsVec4;
    vec4   gCriticalHitPointsVec4;
    vec4   gFrontendFlashColourVec4;
#else
    vec4 gSkyUpperParamsVec4;
    vec4 gLightShaftColourBottomVec4;
    vec4 gLightShaftColourTopVec4;
#endif
#if defined ( D_ACES_PARAMS )
    vec4 gAcesCurveParams[9];
#endif
    vec4 gHDRParams2Vec4;

#if defined( D_LENSFLARE )
    vec4 gLensDirtDistortionVec4;
    vec4 gLensHaloDistortionVec4;
#endif
#if defined( D_DEBUG_QUAD_SPLIT )
    vec4 gDebugSplitVec4;
    vec4 gDebugZoomVec4;
#endif

BEGIN_SAMPLERBLOCK


//TF_BEGIN
#if defined(D_MSAA_RESOLVE)
	SAMPLER2DMS( gBufferMap );
	SAMPLER2DMS( gBuffer1Map );
    #if defined ( D_PLATFORM_METAL )
    SAMPLER2DMSIREG(uint, gStencilBuffer);
    #endif
#else
    SAMPLER2D( gBufferMap );
    SAMPLER2D( gBuffer1Map );
    #if defined ( D_PLATFORM_METAL )
    ISAMPLER2D( uint, gStencilBuffer );
    #endif
#endif
//TF_END
    SAMPLER2D( gBuffer2Map );
    SAMPLER2D( gBuffer3Map );
    SAMPLER2D( gBuffer4Map );

  #if defined( D_INT_SAMPLERS )
    SAMPLER2DI( gBuffer0iMap );
    SAMPLER2DI( gBuffer1iMap );
    SAMPLER2DI( gBuffer2iMap );
    SAMPLER2DI( gBuffer3iMap );
  #endif

  #if defined( D_UINT_SAMPLERS )
    SAMPLER2DU( gBuffer0uMap );
    SAMPLER2DU( gBuffer1uMap );
    SAMPLER2DU( gBuffer2uMap );
    SAMPLER2DU( gBuffer3uMap );
  #endif


    // UI shader stuff
  #if defined ( D_UI ) || defined( D_SCREENEFFECT ) || defined( D_BINOCS )
#if !defined( D_FORWARD )
    SAMPLER2D( gUIMap );
#else
	//TF_BEGIN
	SAMPLER2D(gBuffer5Map);
	//TF_END
#endif
    SAMPLER2D( gUIFullscreenEffect );
    SAMPLER2D( gUIFullscreenNormal );
    SAMPLER2D( gUICamoEffect );
    SAMPLER2D( gUICamoNormal );
#if !defined( D_FORWARD )
    SAMPLER2D( gUIZoomEffect );
    SAMPLER2D( gUISurveyingEffect );
    SAMPLER2D( gUIMissionSurveyingEffect );
#endif
    SAMPLER2D( gUIFullScreenRefraction );

  #else // defined ( D_UI ) || defined( D_SCREENEFFECT ) || defined( D_BINOCS )

    #if defined( D_REFLECTION_PROBE )
    SAMPLER2D( gProbeCubemapFlat );
    #else
    SAMPLER2D( gBuffer5Map );
    #endif

    #if defined ( D_IBL )
    SAMPLER2D(gDualPMapFront);
    SAMPLER2D(gDualPMapBack);
    SAMPLER2D(gIBLIndoorMapFront);
    SAMPLER2D(gIBLIndoorMapBack);
    #else
    SAMPLER2D(gBlurMask);
    #endif

    #if defined( D_LENSFLARE )
    SAMPLER2D(gLensStar);
    SAMPLER2D(gLensNoise);
    SAMPLER2D(gLensDirtHalo);
    SAMPLER2D(gLensDirtGhost);
    SAMPLER2D(gLensDirtAna);
    #endif

    #endif

    #if defined( D_LUT )
    SAMPLER3D(gColourLUTBase);
    SAMPLER3D(gColourLUTFar);
    SAMPLER3D(gColourLUTStorm);
    SAMPLER3D(gColourLUTEffect);
  #endif

  #if defined( D_DEBUG_SHADER )
    #if defined( D_DEBUG_HEX_SPLIT )
    SAMPLER2D( gBuffer6Map );
    SAMPLER2D( gBuffer7Map );
    SAMPLER2D( gBuffer8Map );
    SAMPLER2D( gBuffer9Map );
    SAMPLER2D( gBuffer10Map );
    SAMPLER2D( gBuffer11Map );
    SAMPLER2D( gBuffer12Map );
    SAMPLER2D( gBuffer13Map );
    SAMPLER2D( gBuffer14Map );
    #elif defined( D_DEBUG_QUAD_SPLIT )
    SAMPLER2D( gBuffer0Map );
    SAMPLER2D( gBuffer000Map );
    SAMPLER2D( gBuffer001Map );
    SAMPLER2D( gBuffer002Map );
    SAMPLER2D( gBuffer003Map );
    SAMPLER2D( gBuffer100Map );
    SAMPLER2D( gBuffer101Map );
    SAMPLER2D( gBuffer102Map );
    SAMPLER2D( gBuffer103Map );
    SAMPLER2D( gBuffer200Map );
    SAMPLER2D( gBuffer201Map );
    SAMPLER2D( gBuffer202Map );
    SAMPLER2D( gBuffer203Map );
    SAMPLER2D( gBuffer300Map );
    SAMPLER2D( gBuffer301Map );
    SAMPLER2D( gBuffer302Map );
    SAMPLER2D( gBuffer303Map );
    #endif
  #endif

  #if defined( D_BILATERAL_REJECTMAP )
    #if defined ( D_PLATFORM_METAL )
        RWIMAGE2D_INPUT( int, gBilatRejectMap, D_BILATERAL_REJECT_MAP_BUFFER_ID );
    #elif defined ( D_PLATFORM_GLSL )
        RWINTIMAGE2D( r8i, gBilatRejectMap );
    #else
        RWINTIMAGE2D( int, gBilatRejectMap);
    #endif
  #endif

  #if defined( D_DEPTH_REPRJ_FRWD_RW )
    #if defined ( D_PLATFORM_METAL )
        RWIMAGE2D_INPUT( uint, gDepthReprjFrwd, D_DEPTH_REPRJ_FRWD_BUFFER_ID );
    #else    
        RWUINTIMAGE2D( r32ui, gDepthReprjFrwd );
    #endif
  #endif

  #if defined( D_DEPTH_REPRJ_BKWD_RW )
    #if defined ( D_PLATFORM_METAL )
        RWIMAGE2D_INPUT( uint, gDepthReprjBkwd, D_DEPTH_REPRJ_BKWD_BUFFER_ID );
    #else    
        RWUINTIMAGE2D( r32ui, gDepthReprjBkwd );
    #endif
  #endif

  #if defined( D_CHECKERBOARD )
    #if defined ( D_PLATFORM_METAL )
	RWIMAGE2D_INPUT( r16ui, gCbMask, D_CB_MASK_BUFFER_ID );
	#else
	RWUINTIMAGE2D( r8ui, gCbMask );		
	#endif
    SAMPLER2DMS( gBuffer0MsMap );
    SAMPLER2DMS( gBuffer1MsMap );
    SAMPLER2DMS( gBuffer2MsMap );
    SAMPLER2DMS( gBuffer3MsMap );
  #endif

END_SAMPLERBLOCK

#ifndef D_COMPUTE

#define READ_GBUFFER( structure, buf, coords ) texture2D( SAMPLER_GETMAP( structure, buf ), coords )
#define TEX_COORDS IN(mTexCoordsVec2)
#define TEX_COORDS_HT IN(mTexCoordsVec2)

#elif defined(D_PLATFORM_METAL)

struct ComputeOutputUniforms
{
    vec4                 gOutTextureOffsetSize MTL_ID(0);
    RWIMAGE2D_INPUT(float4, gOutTexture0, 10);
    RWIMAGE2D_INPUT(float4, gOutTexture1, 11);
    RWIMAGE2D_INPUT(float4, gOutTexture2, 12);
    RWIMAGE2D_INPUT(float4, gOutTexture3, 13);
    RWIMAGE2D_INPUT(float4, gOutTexture4, 14);
    RWIMAGE2D_INPUT(float, gOutTextureDepth, 15);
    RWIMAGE2D_INPUT(uint,  gOutTextureStencil, 16);
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define TEX_COORDS_HT ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )


#elif defined D_PLATFORM_ORBIS

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
    RW_Texture2D<float4> gOutTexture0;
    RW_Texture2D<float4> gOutTexture1; 
    RW_Texture2D<float4> gOutTexture2; 
    RW_Texture2D<float4> gOutTexture3; 
    RW_Texture2D<float4> gOutTexture4; 
    RW_Texture2D<float>  gOutTextureDepth;
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define TEX_COORDS_HT ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

#elif defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS )

struct ComputeOutputUniforms
{
    vec4                gOutTextureOffsetSize;
    RWIMAGE2D( rgba32f, gOutTexture0 ); 
    RWIMAGE2D( rgba32f, gOutTexture1 ); 
    RWIMAGE2D( rgba32f, gOutTexture2 ); 
    RWIMAGE2D( rgba32f, gOutTexture3 ); 
    RWIMAGE2D( rgba32f, gOutTexture4 ); 
    RWIMAGE2D( r32f,  gOutTextureDepth );
};

#define READ_GBUFFER( structure, buf, coords ) textureLoadF( buf, ivec2( dispatchThreadID.xy ), 0 )
#define TEX_COORDS    ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define TEX_COORDS_HT ( ( vec2( dispatchThreadID.xy ) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

#elif defined( D_PLATFORM_GLSL )

    RWIMAGE2D( rgba32f, gOutTexture0 ); 
    RWIMAGE2D( rgba32f, gOutTexture1 ); 
    RWIMAGE2D( rgba32f, gOutTexture2 ); 
    RWIMAGE2D( rgba32f, gOutTexture3 ); 
    RWIMAGE2D( rgba32f, gOutTexture4 ); 
    RWIMAGE2D( r32f,  gOutTextureDepth );
	RWIMAGE2D( r32f,  gOutTextureStencil);

#define READ_GBUFFER( structure, buf, coords ) textureLoadF( buf, ivec2( dispatchThreadID.xy ), 0 )
#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )
#define TEX_COORDS_HT ( ( vec2( dispatchThreadID.xy ) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#else

RW_Texture2D<float4> gOutTexture0 : register(u0);
RW_Texture2D<float4> gOutTexture1 : register(u1);
RW_Texture2D<float4> gOutTexture2 : register(u2);
RW_Texture2D<float4> gOutTexture3 : register(u3);
RW_Texture2D<float4> gOutTexture4 : register(u4);
RW_Texture2D<float>  gOutTextureDepth : register(u5);
#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )
#define TEX_COORDS_HT ( ( vec2( dispatchThreadID.xy ) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#endif

//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )       /*: PER_MESH*/
     DECLARE_PTR( CustomPerMeshUniforms,        mpCustomPerMesh )       /*: PER_MESH*/
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )       /*: PER_MESH*/

#if defined( D_COMPUTE ) && ( defined( D_PLATFORM_ORBIS ) || ( defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS ) )  || defined( D_PLATFORM_METAL ) )
     DECLARE_PTR( ComputeOutputUniforms,        mpCmpOutPerMesh )   /* hack - marked 'per mesh' so it'll be alloced in cmd buf */
#endif
DECLARE_UNIFORMS_END
