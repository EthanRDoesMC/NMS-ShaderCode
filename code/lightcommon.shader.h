

//-----------------------------------------------------------------------------
///
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///     Stuff that is only used for these materials.
//-----------------------------------------------------------------------------
struct CustomPerMeshUniforms
{
    vec4 gMaterialParamsVec4 MTL_ID(0);
    vec4 gMaterialSFXVec4;
    vec4 gMaterialSFXColVec4;

    vec4 gaPlanetPositionsVec4[ 6 ];
    vec4 gaPlanetColoursVec4[ 6 ];

    vec4 gSkyColourVec4;
    vec4 gHorizonColourVec4;
    vec4 gSunColourVec4;
    vec4 gScatteringParamsVec4;
    vec4 gSunPositionVec4;

    vec4 gSkyUpperParamsVec4;
    vec4 gSkyUpperColourVec4;
    vec4 gSkySolarColourVec4;
    vec4 gSkyGradientSpeedVec4;

    vec4 gFogParamsVec4;
    vec4 gFogColourVec4;
    vec4 gHeightFogParamsVec4;
    vec4 gHeightFogColourVec4;

    vec4 gWaterFogVec4;
    vec4 gWaterFogColourFarVec4;
    vec4 gWaterFogColourNearVec4;

    vec4 gSpaceHorizonColourVec4;
    vec4 gFogFadeHeightsVec4;
    vec4 gFogFadeHeights2Vec4;
    vec4 gFogFadeHeights3Vec4;
    vec4 gSpaceSkyColourVec4;
    vec4 gSpaceScatteringParamsVec4;

    vec4 gSpaceSkyColour1Vec4;
    vec4 gSpaceSkyColour2Vec4;
    vec4 gSpaceSkyColour3Vec4;
    vec4 gSpaceFogColourVec4;
    vec4 gSpaceFogColour2Vec4;
    vec4 gSpaceFogParamsVec4;

    vec4 gLightShaftParamsVec4;
    vec4 gLightTopColourVec4;
    vec4 gRainParametersVec4;

    vec4 gHueOverlayParamsVec4;
    vec4 gSaturationOverlayParamsVec4;
    vec4 gValueOverlayParamsVec4;

    vec4 gDebugLightColourVec4;
    vec4 gScreenSpaceShadowsParamsVec4;

BEGIN_SAMPLERBLOCK
    
	SAMPLER2D( gBufferMap );
    SAMPLER2D( gBuffer1Map );
    SAMPLER2D( gBuffer2Map );
    SAMPLER2D( gBuffer3Map );
    SAMPLER2D( gBuffer4Map );
    SAMPLER2D( gBuffer5Map );
    
#if !defined( D_RECOLOUR ) && !defined(D_COMBINE)
    SAMPLER2D(gCausticMap);
    SAMPLER2D(gCausticOffsetMap);
    SAMPLER2DSHADOW(gShadowMap);
    SAMPLER2D(gCloudShadowMap);
    SAMPLER2D(gDualPMapFront);
    SAMPLER2D(gDualPMapBack);
#endif

//TF_BEGIN
#if !defined(D_COMPUTE)
    #if defined(D_PLATFORM_METAL)
        RW_DATABUFFER(atomic_int, gLightCluster, D_LIGHT_CLUSTER_BUFFER_ID);
	    //RWIMAGE2D_INPUT(r32i,	gLightCluster, D_LIGHT_CLUSTER_BUFFER_ID );
        //vec4   gLightClusterSize;
    #else
        RWINTIMAGE2D( r32i, gLightCluster );
    #endif
#endif
//TF_END
#if defined( D_SPOTLIGHT ) || defined( D_SPOTLIGHT_MULTI )
    SAMPLER2DARRAY(gLightCookiesMap);
    SAMPLER3D(gLightVolNoise3D);
#endif

END_SAMPLERBLOCK

#ifndef D_COMPUTE
#define TEX_COORDS IN(mTexCoordsVec2)
#define READ_GBUFFER( structure, buf, coords ) texture2D( SAMPLER_GETMAP( structure, buf ), coords )

#elif defined(D_PLATFORM_METAL)

struct ComputeOutputUniforms
{
    vec4                 gOutTextureOffsetSize MTL_ID(0);
	//TF_BEGIN
#if defined( D_TILED_LIGHTS)
    RW_DATABUFFER(atomic_int, gLightCluster, D_LIGHT_CLUSTER_BUFFER_ID);
#else
	RWIMAGE2D_INPUT(float4, gOutTexture0, 10);
#endif
	//TF_END

    RWIMAGE2D_INPUT(float4, gOutTexture1, 11);
    RWIMAGE2D_INPUT(float4, gOutTexture2, 12);
    RWIMAGE2D_INPUT(float4, gOutTexture3, 13);
    RWIMAGE2D_INPUT(float4, gOutTexture4, 14);
    RWIMAGE2D_INPUT(float, gOutTextureDepth, 15);

};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define READ_GBUFFER( structure, buf, coords ) texelFetch( structure.buf, uvec2( dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) ), 0 )


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
    vec4                 gOutTextureOffsetSize;
    RW_Texture2D<float4> gOutTexture0; 
    RW_Texture2D<float4> gOutTexture1; 
    RW_Texture2D<float4> gOutTexture2; 
    RW_Texture2D<float4> gOutTexture3; 
    RW_Texture2D<float4> gOutTexture4;
    RW_Texture2D<float>  gOutTextureDepth;
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define READ_GBUFFER( structure, buf, coords ) (structure.buf[dispatchThreadID.xy + uvec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy)])

#elif defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS )

struct ComputeOutputUniforms
{
    vec4                gOutTextureOffsetSize;
    RWIMAGE2D( rgba32f, gOutTexture0 );
    RWIMAGE2D( rgba32f, gOutTexture1 );
    RWIMAGE2D( rgba32f, gOutTexture2 );
    RWIMAGE2D( rgba32f, gOutTexture3 );
    RWIMAGE2D( rgba32f, gOutTexture4 );
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define READ_GBUFFER( structure, buf, coords ) texture2D( SAMPLER_GETMAP( structure, buf ), coords )

#elif defined( D_PLATFORM_GLSL )

//TF_BEGIN
#if defined( D_TILED_LIGHTS)
RWINTIMAGE2D(r32i, gOutTexture0);
#else
RWIMAGE2D( rgba32f, gOutTexture0 );
#endif
//TF_END
RWIMAGE2D( rgba32f, gOutTexture1 );
RWIMAGE2D( rgba32f, gOutTexture2 );
RWIMAGE2D( rgba32f, gOutTexture3 );
RWIMAGE2D( rgba32f, gOutTexture4 );
RWIMAGE2D( r32f, gOutTextureDepth );

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )
#define READ_GBUFFER( structure, buf, coords ) textureLoadF( buf, ivec2( dispatchThreadID.xy ), 0 )


#else

RW_Texture2D<float4> gOutTexture0 : register(u0);
RW_Texture2D<float4> gOutTexture1 : register(u1);
RW_Texture2D<float4> gOutTexture2 : register(u2);
RW_Texture2D<float4> gOutTexture3 : register(u3);
RW_Texture2D<float4> gOutTexture4 : register(u4);

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )
#define READ_GBUFFER( structure, buf, coords ) (buf[dispatchThreadID.xy])

#endif


//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
    DECLARE_PTR( PerFrameUniforms, mpPerFrame )
    DECLARE_PTR( CustomPerMeshUniforms, mpCustomPerMesh )
    DECLARE_PTR( CommonPerMeshUniforms, mpCommonPerMesh )

#if defined( D_COMPUTE ) && ( defined( D_PLATFORM_ORBIS ) || ( defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS ) ) || defined( D_PLATFORM_METAL ) )
     DECLARE_PTR( ComputeOutputUniforms,        mpCmpOutPerMesh )   /* hack - marked 'per mesh' so it'll be alloced in cmd buf */
#endif
DECLARE_UNIFORMS_END
