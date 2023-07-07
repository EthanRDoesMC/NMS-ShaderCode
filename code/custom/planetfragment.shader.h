////////////////////////////////////////////////////////////////////////////////
///
///     @file       PlanettFragment.h
///     @author     User
///     @date       
///
///     @brief      SkyFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#ifdef D_PLATFORM_ORBIS
#pragma argument (O4; fastmath; scheduler=minpressure; targetoccupancy_atallcosts=5)
#endif

#if defined ( D_PLATFORM_SWITCH )
#pragma optionNV(fastmath on)
#endif

#define D_TEXTURE_ARRAYS
#define D_LOW_QUALITY

#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "Custom/AtmosphereCommon.h"


#include "Common/Common.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonScattering.shader.h"
#include "Common/CommonTriplanarTexturing.shader.h"

//TF_BEGIN
#if !defined( D_FORWARD_RENDERER ) 
#include "Common/CommonGBuffer.shader.h"
#include "OutputDeferred.shader.h"
#else
#include "Common/CommonLighting.shader.h"
#include "OutputForward.shader.h"
#endif
//TF_END

#if (defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS )) && defined( D_DEFER )
#pragma PSSL_target_output_format(target 1 FMT_UNORM16_ABGR)
#endif

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT( vec4, mWorldPositionVec4, TEXCOORD0 )
INPUT( vec4, mTexCoordsVec4, TEXCOORD1 )
INPUT( vec4, mWorldNormalVec3_mfDistanceFromPlanet, TEXCOORD2 )
#ifndef D_REFLECT
INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD3 )
#endif

DECLARE_INPUT_END

STATIC_CONST float HALFPI = 1.570796;

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main Fragment
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lFragmentColourVec4 = vec4( 1.0, 0.0, 0.0, 1.0 );

    float lfTile       = 0.0;
    float lfSlope      = 1.0;
    float lfPatch      = 0.0;
    float lfScaleFade  = 0.5;

    float lfSpecular   = 0.0;
    float lfSubsurface = 0.0;
    float lfMetallic   = 0.0;
    float lfGlow       = 0.0;
    float lfHeight     = 0.0;

    float kfTextureSmallScaleFactor = 1.0 / 16384.0;
    float kfTextureLargeScaleFactor = 1.0 / 32768.0;

    vec3  lNormalVec3;
    vec3  lTerrainSpaceNormalVec3;
    vec3  lTerrainSpacePositionVec3;
    vec3  lTerrainOffsetVec3;

    lTerrainSpaceNormalVec3       = normalize( IN( mWorldNormalVec3_mfDistanceFromPlanet ).xyz );
    lTerrainSpacePositionVec3     = IN( mWorldPositionVec4 ).xyz;
    lTerrainOffsetVec3            = -lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

    lfHeight          = textureCube( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gWaterMap ), lTerrainSpaceNormalVec3 ).r;    
    lfHeight         *= lUniforms.mpCustomPerMaterial.gTerrainDistancesVec4.y;
    float lfSeaLevel  = lUniforms.mpCustomPerMaterial.gWaterFogVec4.x;
    float lfWaterFade = float( lfSeaLevel > lfHeight );

    uvec4 lTileTextureIndicesVec4 = uvec4( 0x8421, 0x8421, 0x8421, 0x8421 );

    lFragmentColourVec4.xyz = GetTileColourAndNormal(
        DEREF_PTR( lUniforms.mpCustomPerMaterial ),
        lTerrainSpaceNormalVec3,
        float2vec3( 0.0 ),
        lTerrainSpacePositionVec3 + lTerrainOffsetVec3,
        lTileTextureIndicesVec4,
        lfTile,
        lfSlope, lfSlope,
        lfPatch,
        lNormalVec3,
        kfTextureSmallScaleFactor, 
        kfTextureLargeScaleFactor,
        lfScaleFade,
        1.0 - lfWaterFade,
        SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMesh, gDiffuseMap ),
        SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMesh, gNormalMap ),
        SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMesh, gDiffuseMap ),
        SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMesh, gNormalMap ),
        SAMPLER2DPARAM_SRT(      lUniforms.mpCustomPerMesh, gValueNoiseNorms2D ),
        lfSpecular,
        lfMetallic,
        lfHeight );

    mat3 lNormalisedMat3;
    MAT3_SET_COLUMN( lNormalisedMat3, 0, normalize( lUniforms.mpCommonPerMesh.gWorldMat4[ 0 ].xyz ) );
    MAT3_SET_COLUMN( lNormalisedMat3, 1, normalize( lUniforms.mpCommonPerMesh.gWorldMat4[ 1 ].xyz ) );
    MAT3_SET_COLUMN( lNormalisedMat3, 2, normalize( lUniforms.mpCommonPerMesh.gWorldMat4[ 2 ].xyz ) );

    lNormalVec3 = normalize( MUL( lNormalisedMat3, lNormalVec3 ) );
    
    {
        vec3 lFaceNormalVec3    = normalize( IN( mWorldPositionVec4 ).xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );

        lNormalVec3             = normalize( mix( lNormalVec3, lFaceNormalVec3, max( lfWaterFade - 0.1, 0.0 ) ) );

        //lfWaterFade         *= smoothstep( 0.9, 1.0, dot( lNormalVec3, lFaceNormalVec3 ) ) * 0.3 + 0.7;

        vec3 lLightColourVec3   = mix( ( lUniforms.mpCommonPerMesh.gLightColourVec4.xyz ), vec3( 1.0,1.0,1.0 ), lfWaterFade );

        lFragmentColourVec4.rgb = mix( lFragmentColourVec4.rgb, ( lUniforms.mpCustomPerMaterial.gWaterFogColourFarVec4.xyz ), lfWaterFade );
        lfSpecular              = mix( lfSpecular, 0.35, lfWaterFade );

        //float lfFoam = 1.0 - ( abs( lfWaterFade - 0.5 ) * 2.0 );
        //lFragmentColourVec4.rgb += vec3( lfFoam * 0.2 );
    }

    int   liMaterialID = 0; //D_DETAILOVERLAY;
    float lfRoughness  = lfSpecular * lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.x;

    vec4 lOutColours0Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours1Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours2Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours3Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours4Vec4 = vec4(0, 0, 0, 0);

    WriteOutput(
        lOutColours0Vec4,
        lOutColours1Vec4,
        lOutColours2Vec4,
        lOutColours3Vec4,
        lOutColours4Vec4,
        DEREF_PTR( lUniforms.mpPerFrame ),
        DEREF_PTR( lUniforms.mpCommonPerMesh ),
        DEREF_PTR( lUniforms.mpCustomPerMaterial ),
        lFragmentColourVec4,
        IN( mWorldPositionVec4 ).xyz,
        lNormalVec3,
        liMaterialID,
        lfMetallic,
        lfRoughness,
        lfSubsurface, 
        lfGlow,
        IN( mScreenSpacePositionVec4 ).xyzw,
        IN( mScreenSpacePositionVec4 ).xyzw,
#if !defined( D_ATTRIBUTES ) && !defined( _F07_UNLIT ) || defined( D_FORWARD_RENDERER )
        GetInverseWorldUpTransform( lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4 ),
//TF_BEGIN
#if defined( D_FORWARD_RENDERER )
       SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gShadowMap),
	   SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gCloudShadowMap),
	   SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gDualPMapBack),
	   SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gDualPMapFront),
#endif
        1.0,
#endif
//TF_END
		0.0, // temp. Going to be for per pixel depth of imposters
        false,
        -1.0
#if defined( D_OUTPUT_LINEARDEPTH )
        , FastNormaliseDepth(lUniforms.mpPerFrame.gClipPlanesVec4 , IN(mScreenSpacePositionVec4).w ) //Linear Depth!
#endif
        );

	//TF_BEGIN
#if !defined( D_ATTRIBUTES )
	//TF_END
    FRAGMENT_COLOUR  = lOutColours0Vec4;
#else
    FRAGMENT_COLOUR0 = lOutColours0Vec4;
    FRAGMENT_COLOUR1 = lOutColours1Vec4;
    FRAGMENT_COLOUR2 = lOutColours2Vec4;
    FRAGMENT_COLOUR3 = lOutColours3Vec4;
#if defined( D_OUTPUT_LINEARDEPTH )
    FRAGMENT_COLOUR4 = lOutColours4Vec4;
#endif
#endif

}

