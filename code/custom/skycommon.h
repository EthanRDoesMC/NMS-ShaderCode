////////////////////////////////////////////////////////////////////////////////
///
///     @file       CloudCommon.h
///     @author     User
///     @date       
///
///     @brief      CloudCommon
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
///     CustomPerMaterialUniforms
///
///     @brief      CustomPerMaterialUniforms
///
///   
//----------------------------------------------------------------------------- 

struct CustomPerMeshUniforms
{
    vec4 gDummyVec4 MTL_ID(0);
};

struct CustomPerMaterialUniforms 
{
    vec4 gSpaceSkyColour1Vec4 MTL_ID(0);
    vec4 gSpaceSkyColour2Vec4;
    vec4 gSpaceSkyColour3Vec4;
    vec4 gSpaceCloudColourVec4;
    vec4 gSpaceSunColourVec4;

    vec4 gSpaceNebulaColour1Vec4;
    vec4 gSpaceNebulaColour2Vec4;
    vec4 gSpaceNebulaColour3Vec4;
    vec4 gSpaceNebulaColour4Vec4;

    vec4 gSpaceNormalParamsVec4;
    vec4 gSpaceNebulaParamsVec4;

    vec4 gSunPositionVec4;
    vec4 gScatteringParamsVec4;

//TF_BEGIN
#if defined(D_BLOOM)
	vec4 gHDRParamsVec4;
#endif
//TF_END
#if !defined(D_SKY_CUBE)

BEGIN_SAMPLERBLOCK
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAY(gBufferMap);
#else
    SAMPLER2D(gBufferMap);
#endif
    SAMPLERCUBE(gNebulaMap);
    SAMPLERCUBE(gNebulaMask);
    SAMPLER2D(gNoiseMap);
    SAMPLER2DSHADOW(gShadowMap);
    SAMPLER2D(gDualPMapFront);
    SAMPLER2D(gDualPMapBack);
END_SAMPLERBLOCK
#else
	//TF_BEGIN
BEGIN_SAMPLERBLOCK
	SAMPLERCUBE(gSkyCubeMap);
END_SAMPLERBLOCK
	//TF_END
#endif

//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS

    DECLARE_PTR( PerFrameUniforms,          mpPerFrame )           /*: PER_MATERIAL*/
    DECLARE_PTR( CustomPerMaterialUniforms, mpCustomPerMaterial )  /*: PER_MATERIAL*/

    DECLARE_PTR( CommonPerMeshUniforms,     mpCommonPerMesh )      /*: PER_MESH*/
    DECLARE_PTR( CustomPerMeshUniforms, mpCustomPerMesh )          /*: PER_MESH*/

DECLARE_UNIFORMS_END
