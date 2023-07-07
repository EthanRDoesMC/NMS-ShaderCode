//-----------------------------------------------------------------------------
///
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///     Stuff that is only used for these types of meshes.
//-----------------------------------------------------------------------------
struct CustomPerMaterialUniforms
{
    vec4 gMinPixelSize_Glow;
    vec4 gUvScrollStep_DistortionStep;
    vec4 gDistortionScale_Unused;


    BEGIN_SAMPLERBLOCK
        SAMPLER2D( gDiffuseMap );

#if defined(_F14_UVSCROLL)
    SAMPLER2D( gAlphaMap );
#endif

#if defined( _F31_DISPLACEMENT )
    SAMPLER2D( gDistortionMap );
#endif

    END_SAMPLERBLOCK

    //
    // This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
    //
        DECLARE_UNIFORMS
            //DECLARE_PTR( CommonPerMaterialUniforms, mpCommonPerMaterial )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.  
    DECLARE_PTR( PerFrameUniforms, mpPerFrame )   /*: PER_MATERIAL*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.  
        DECLARE_PTR( CommonPerMeshUniforms, mpCommonPerMesh )       /*: PER_MESH*/ // sematics currently crash the compiler so the parser is hardcoded to look for names.  
        DECLARE_PTR( CustomPerMaterialUniforms, mpCustomPerMaterial )       /*: PER_MESH*/
        DECLARE_UNIFORMS_END
