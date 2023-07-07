////////////////////////////////////////////////////////////////////////////////
///
///     @file       DoFFragment.h
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

#include "Common/Defines.shader.h"

struct CustomPerMaterialUniforms
{
    vec4 gMaterialParamsVec4 MTL_ID(0);

    vec4 gWaveScaleVec4;
    vec4 gWaveSpeedVec4;
    vec4 gWindDirectionVec4;
    vec4 gWaterSurfaceParamsVec4;
    vec4 gFresnelParamsVec4;
    vec4 gFoamParamsVec4;
    vec4 gFoamColourVec4;
    vec4 gSkyUpperColourVec4;

    // Fog
    vec4 gSkyColourVec4;
    vec4 gHorizonColourVec4;
    vec4 gSunColourVec4;
    vec4 gWaterColourBaseVec4;
    vec4 gWaterColourAddVec4;
    vec4 gWaterFogColourNearVec4;
    vec4 gWaterFogColourFarVec4;
    vec4 gWaterFogVec4;
    vec4 gHeightFogParamsVec4;
    vec4 gHeightFogColourVec4;
    vec4 gSpaceHorizonColourVec4;
    vec4 gFogColourVec4;
    vec4 gFogParamsVec4;
    vec4 gScatteringParamsVec4;
    vec4 gSpaceSkyColour1Vec4;
    vec4 gSpaceSkyColour2Vec4;
    vec4 gSpaceSkyColour3Vec4;
    vec4 gSpaceSkyColourVec4;
    vec4 gFogFadeHeightsVec4;
    vec4 gSunPositionVec4;
    vec4 gSpaceScatteringParamsVec4;
    vec4 gLightTopColourVec4;
    
BEGIN_SAMPLERBLOCK
    SAMPLER2DSHADOW( gShadowMap );

    SAMPLER3D( gNoiseMap );

    SAMPLER2D( gDualPMapFront );
    SAMPLER2D( gDualPMapBack );
    SAMPLER2D( gCloudShadowMap );

    SAMPLER2D( gNormalMap );
    SAMPLER2D( gLargeNormalMap );
    SAMPLER2D( gFoamNormalMap );

    SAMPLER2D( gBufferMap );
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAY( gBuffer1Map );
#else
    SAMPLER2D( gBuffer1Map );
#endif
    SAMPLER2D( gReflectionMap );
    SAMPLER2D( gFoamMap );
    SAMPLER2D( gFoamHeightMap );
#ifdef D_FADE // defined( D_INSTANCED ) || defined( D_TERRAIN )
    SAMPLER2D( gFadeNoiseMap );
#endif

#if !defined ( D_PLATFORM_METAL )
    #ifdef D_WRITE_LINEARDEPTH
        #if defined ( D_PLATFORM_SWITCH )
            RWIMAGE2D(rgba32f, gRWDepthLinear);
        #else
            RWIMAGE2D(rgba32f, gRWDepthLinear);
        #endif
    #endif
#endif

BEGIN_IMAGEBLOCK

#if defined ( D_PLATFORM_METAL )
    #ifdef D_WRITE_LINEARDEPTH
        RWIMAGE2D_INPUT(rgba32f, gRWDepthLinear, D_DEPTH_LINEAR_BUFFER_ID);
    #endif
#endif

END_IMAGEBLOCK

// We need to support compute in here also.
// HACK: When we work out if we need per material uniforms, then we can choose to include this from PostCommon.h

#ifndef D_COMPUTE

#define TEX_COORDS IN( mTexCoordsVec2 )

#elif defined ( D_COMPUTE )

#if defined D_PLATFORM_ORBIS

struct ComputeOutputUniforms
{
    vec4 gOutTextureOffsetSize MTL_ID(0);
    RW_Texture2D<float4> gOutTexture0;
    RW_Texture2D<float4> gOutTexture1;
    RW_Texture2D<float4> gOutTexture2;
    RW_Texture2D<float4> gOutTexture3;
    RW_Texture2D<float4> gOutTexture4;
    RW_Texture2D<float>  gOutTextureDepth;
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy ) + vec2( 0.5, 0.5 ) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

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
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy ) + vec2( 0.5, 0.5 ) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )


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
    RWIMAGE2D( r32f, gOutTextureDepth );

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2( 0.5, 0.5 ) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#else

    RW_Texture2D<float4> gOutTexture0 : register( u0 );
    RW_Texture2D<float4> gOutTexture1 : register( u1 );
    RW_Texture2D<float4> gOutTexture2 : register( u2 );
    RW_Texture2D<float4> gOutTexture3 : register( u3 );
    RW_Texture2D<float4> gOutTexture4 : register( u4 );
    RW_Texture2D<float>  gOutTextureDepth : register( u5 );

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2( 0.5, 0.5 ) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#endif // D_PLATFORM_ORBIS

#endif // defined ( D_COMPUTE )

DECLARE_UNIFORMS
    DECLARE_PTR( PerFrameUniforms,          mpPerFrame )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.
    DECLARE_PTR( CommonPerMeshUniforms,     mpCommonPerMesh )       /*: PER_MESH*/

    DECLARE_PTR( CustomPerMaterialUniforms, mpCustomPerMaterial )   /*: PER_MATERIAL*/

#if defined( D_COMPUTE ) && ( defined( D_PLATFORM_ORBIS ) || ( defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS ) ) || defined( D_PLATFORM_METAL ) )
    DECLARE_PTR( ComputeOutputUniforms, mpCmpOutPerMesh )   /* hack - marked 'per mesh' so it'll be alloced in cmd buf */
#endif

DECLARE_UNIFORMS_END
