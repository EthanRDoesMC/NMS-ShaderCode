

//-----------------------------------------------------------------------------
///
///     CustomPerMeshUniforms
///
///     @brief      CustomPerMeshUniforms
///
///     Stuff that is only used for these types of meshes.
//-----------------------------------------------------------------------------
struct CustomPerMaterialUniforms
{
    // StartFadeNear, EndFadeNear, StartIncreaseFar, EndIncreaseFar
    vec4 gFadeDistancesVec4;
    // FarBrightnessMutiplier, EndFadeSpeed, TextureStrength, DistortionStrength
    vec4 gLaserParamsVec4;
    // FadeDistance, ScrollSpeed, NoiseScrollSpeed, MinimumDamageLevel
    vec4 gTextureParamsVec4;

BEGIN_SAMPLERBLOCK	
    SAMPLER2D( gDiffuseMap );
    SAMPLER2D( gNormalMap );
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAY(gBufferMap);
#else
    SAMPLER2D( gBufferMap );
#endif
    SAMPLER2D( gEffectMap );
    SAMPLER2D( gNoiseMap );
END_SAMPLERBLOCK

//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )            /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.
     DECLARE_PTR( CustomPerMaterialUniforms,    mpCustomPerMaterial )   /*: PER_MESH*/
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )       /*: PER_MESH*/
DECLARE_UNIFORMS_END
