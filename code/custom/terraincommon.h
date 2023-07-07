//-----------------------------------------------------------------------------
///
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///   
//----------------------------------------------------------------------------- 
struct CustomPerMaterialUniforms
{
    // Fog
    vec4 gWaterFogVec4 MTL_ID(0);
#if !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT )

    vec4 gSkyColourVec4;
    vec4 gHorizonColourVec4;
    vec4 gSunColourVec4;
    vec4 gWaterFogColourNearVec4;
    vec4 gWaterFogColourFarVec4;
    vec4 gTerrainEditEffectsVec4;

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
    vec4 gTerrainDistancesVec4;

    vec4 gSkyUpperColourVec4;
    vec4 gMaterialParamsVec4;
#if defined ( D_PLATFORM_SWITCH )
    half4 gaAverageColoursVec4[ D_TERRAINCOLOURARRAY_SIZE + 1 ];
    half4 gaTerrainColoursVec4[ D_TERRAINCOLOURARRAY_SIZE + 1 ];
    half4 gaSpecularValuesVec4[ D_TERRAINCOLOURARRAY_SIZE_FLOAT ];
#else
    vec4 gaAverageColoursVec4[ D_TERRAINCOLOURARRAY_SIZE ];
    vec4 gaTerrainColoursVec4[ D_TERRAINCOLOURARRAY_SIZE ];
    vec4 gaSpecularValuesVec4[ D_TERRAINCOLOURARRAY_SIZE_FLOAT ];
#endif

    vec4 gAverageColour1Vec4; 
    vec4 gAverageColour2Vec4; 

    vec4 gDebugColourVec4;

    vec4 gHueOverlayParamsVec4;
    vec4 gSaturationOverlayParamsVec4;
    vec4 gValueOverlayParamsVec4;

    vec4 gTileBlendScalesVec4;

    vec4 gLightTopColourVec4;

	//TF_BEGIN
#if defined(D_BLOOM)
	vec4 gHDRParamsVec4;
#endif
#if defined(D_DOF)
	vec4 gDoFParamsVec4;
#endif
	//TF_END

BEGIN_SAMPLERBLOCK

#if !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT )
    SAMPLER3DREG(gPerlin3D, 5);
#if !defined( D_SIMPLIFIED_NOISE )
    SAMPLER2DREG(gValueNoiseNorms2D,    23);
#else
    SAMPLER2DREG(gValueNoiseNorms2D_VR, 20);
#endif
    SAMPLER2DARRAYREG(gNormalMap, 7);
    SAMPLER2DARRAYREG(gSubstanceNormalMap, 9);
    SAMPLER2DARRAYREG(gDiffuseMap, 6);
    SAMPLER2DARRAYREG(gSubstanceDiffuseMap, 8);
#endif

    SAMPLER2DREG(gNoiseMap, 4);

#if !defined( D_RECOLOUR ) && !defined( D_COMBINE ) && !defined( D_DEFER ) && !defined( D_DEPTHONLY ) && !defined(D_TERRAIN_EDITS)
    SAMPLER2D(gCausticMap);
    SAMPLER2D(gCausticOffsetMap);
    SAMPLER2DSHADOW(gShadowMap);
    SAMPLER2D(gCloudShadowMap);
    SAMPLER2D(gDualPMapFront);
    SAMPLER2D(gDualPMapBack);
#endif


//TF_BEGIN
#if defined(D_TILED_LIGHTS) && defined(D_PLATFORM_METAL) && !defined(D_PLATFORM_IOS)
    SAMPLER2DARRAY(gLightCookiesMap);
#endif
//TF_END

#else

BEGIN_SAMPLERBLOCK

#endif

#if defined( D_FADE )
    SAMPLER2DREG( gFadeNoiseMap, 14 );
#endif

END_SAMPLERBLOCK

#if defined ( D_WRITE_TEX_CACHE ) || defined ( D_READ_TEX_CACHE )
#define D_VALIDATE_TEX_CACHE_BLOCKSx
#endif


//-----------------------------------------------------------------------------
///
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///   
//-----------------------------------------------------------------------------
struct CustomPerMeshUniforms
{
    vec4 gTerrainLodParamsVec4 MTL_ID(0);
    vec4 gSparseTextureStatusVec4;
    
#ifdef D_WRITE_TEX_CACHE
    vec4 gTextureCoordsAdjustVec4;
    vec4 gTextureCoordsBorderVec4;
#endif

#if defined ( D_VALIDATE_TEX_CACHE_BLOCKS )
    vec4 gTexCacheBlockColourVec4;
#endif

BEGIN_SAMPLERBLOCK

#ifdef D_READ_TEX_CACHE

    SAMPLER2DREG( gSparseNearDiffMap, 15 );
    SAMPLER2DREG( gSparseNearNormMap, 16 );

    SAMPLER2DREG( gSparseNearStatusMap, 17 );

    SAMPLER2DREG( gSparseFarStatusMap, 19 );

    SAMPLER2DREG( gSparseFarDiffMap, 21 );
    SAMPLER2DREG( gSparseFarNormMap, 22 );

#endif

BEGIN_IMAGEBLOCK

//TF_BEGIN
#if defined(D_TILED_LIGHTS)
    #if defined(D_PLATFORM_METAL)
        RW_DATABUFFER(atomic_int, gLightCluster, D_LIGHT_CLUSTER_BUFFER_ID);
    #else
	    RWINTIMAGE2D(r32i, gLightCluster);
    #endif
#endif
//TF_END

#ifdef D_WRITE_CACHE_FEEDBACK
//TF_BEGIN
#if !defined(D_COMPUTE)
    #if defined(D_PLATFORM_METAL)
        device RW_DATABUFFER(int, gFeedbackMap,     6);
        vec4   gFeedbackMapTextureSize;
    #elif defined ( D_PLATFORM_SWITCH )
        uniform
        RWINTIMAGE2D( r32i, gFeedbackMap );	
    #else
        RWINTIMAGE2D( r32i, gFeedbackMap );
    #endif
#endif
//TF_END
#endif
    
END_IMAGEBLOCK

//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )       /*: PER_MESH*/

     DECLARE_PTR( CustomPerMaterialUniforms,    mpCustomPerMaterial )   /*: PER_MATERIAL*/
     DECLARE_PTR( CustomPerMeshUniforms,        mpCustomPerMesh )       /*: PER_MESH*/
DECLARE_UNIFORMS_END

//

uint
StoreCompressedSign( uint luAccumulate, uint luSignValue )
{
    return (luAccumulate & 0xfff) | ( ( 0x1 & luSignValue ) << 12 );
}

uint
StoreCompressedArrayIdLo( uint luAccumulate, float luValue )
{
    // Pack to 3f range... id should be less than 64.
    uint luId = uint( luValue );
    return ( luAccumulate & 0xffffffc0 ) | ( luId & 0x3f );
}

uint StoreCompressedArrayIdHi( uint luAccumulate, float luValue )
{
    // Our colour array range is very specific, so we can 
    // pack an id somewhere outside the range:
    // D_TERRAINCOLOURARRAY_SIZE == 23
    // const float lfScale = 64.0;
    uint luId = uint( luValue );
    return ( luAccumulate & 0x103f ) | ( (luId & 0x3f ) << 6 );
}

bool
DecompressSign( uint luValue )
{
    return ( luValue & 0x1000 ) == 0x1000;
}

uint
DecompressArrayIdLo( uint luValue )
{
    return ( luValue & 0x3f );
}

uint
DecompressArrayIdHi( uint luValue )
{
    luValue &= 0xfc0;
    luValue >>= 6;
    return ( luValue );
}
