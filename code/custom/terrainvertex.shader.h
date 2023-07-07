////////////////////////////////////////////////////////////////////////////////
///
///     @file       TerrainVertex.h
///     @author     User
///     @date       
///
///     @brief      TerrainVertexShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#if !defined(D_WATER)
#define D_TERRAIN
#endif

#define D_OCTAHEDRON_NORMALS

#if !defined( D_REFLECT_WATER_UP ) && !defined( D_REFLECT_WATER ) && !defined( D_REFLECT_DUALP ) && !defined( D_WRITE_TEX_CACHE )
    #define D_FADE
#endif

#if defined( D_FADE ) && ( !(defined( D_TERRAIN_X_FACING ) || defined( D_TERRAIN_Y_FACING ) || defined( D_TERRAIN_Z_FACING )  || defined( D_TERRAIN_N_FACING ) || defined( D_STOCHASTIC_TERRAIN_NO_FADE ) ) )
    #define D_FADE_REALLY
#endif

#if !defined( D_PLATFORM_SWITCH ) && !defined( D_PLATFORM_XBOXONE ) && !defined( D_PLATFORM_IOS )
    #define D_LOCAL_FADE
#endif

#if /*defined( D_ASTEROID ) ||*/ defined( D_REFLECT_WATER ) || defined( D_REFLECT_WATER_UP ) || defined( D_REFLECT_DUALP )
#define _F50_DISABLE_POSTPROCESS
#endif

#if defined ( D_PLATFORM_SWITCH ) || defined ( D_PLATFORM_IOS )
#define D_DISABLE_TERRAIN_UVS
#define D_DISABLE_TERRAIN_ENCODE
#endif

//-----------------------------------------------------------------------------
//      Include files

#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"




//
// In TerrainCommon we have our uniforms specific to terrain declared. PLUS, IMPORTANTLY we have the SRT buffer declared (gUniforms). 
// This MUST be included after CommonUniforms, but before all the other stuff that references gUniforms.
//
#include "Custom/TerrainCommon.h"


//
// Have to include things that reference the global gUniforms under here. Things defined above may be parameters to functions in the following includes.
//
#include "Common/CommonVertex.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonTriplanarTexturing.shader.h"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------

#ifdef D_MESH

    DECLARE_INPUT

    INPUT( vec4, mkLocalPositionVec4, POSITION0 )
    //INPUT( vec4, mkCustom1Vec4,       TEXCOORD0 )

        
        INPUT( vec4, mkLocalNormalVec4,   TEXCOORD1 )

#ifdef D_DECLARE_TANGENT
    INPUT( vec4, mkTangentVec4, TANGENT0 )
#endif

DECLARE_INPUT_END

#else

DECLARE_INPUT

    INPUT( vec4, mkLocalPositionVec4, POSITION0 )
    INPUT( vec4, mkCustom1Vec4,       BLENDINDICES  )
#ifdef D_OCTAHEDRON_NORMALS    
    #if defined ( D_DISABLE_TERRAIN_UVS )
    INPUT( vec2, mkLocalNormalVec4,     TEXCOORD1 )
    #else
    INPUT( vec4, mkLocalNormalVec4,     TEXCOORD1       )
    #endif
#else
    INPUT( vec3, mkFaceNormalVec3,    TEXCOORD1       )
    INPUT( vec3, mkSmoothNormalVec3,  TEXCOORD2       )
#endif    
#ifdef D_PLATFORM_OPENGL

    INPUT( vec4, mkCustom2Vec4,       BLENDWEIGHT  )
#endif

DECLARE_INPUT_END

#endif


//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------

#if defined( D_DEPTHONLY ) && !defined( D_TESS_SHADERS_PRESENT ) && ( !defined( D_FADE_REALLY ) || !defined( D_LOCAL_FADE ) )
    /* Empty Output */
    #if defined ( D_PLATFORM_IOS ) && defined ( D_FADE_REALLY )
    DECLARE_OUTPUT
        OUTPUT_SCREEN_POSITION
    DECLARE_OUTPUT_END
    #endif
#else

DECLARE_OUTPUT
    
    OUTPUT_SCREEN_POSITION
#if ! defined( D_TESS_SHADERS_PRESENT )
    OUTPUT_SCREEN_SLICE
#endif
#if !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT ) || ( defined( D_FADE_REALLY ) && defined( D_LOCAL_FADE ) )
    OUTPUT( vec4,   mLocalPositionVec4,              TEXCOORD1 )
#endif
#if !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT )

#if !defined ( D_TERRAIN_T_SPLIT )
    OUTPUT( vec4,   mTileVec4,                       TEXCOORD2 )
#endif
#if defined( D_READ_TEX_CACHE ) || defined( D_WRITE_CACHE_FEEDBACK ) ||  defined( D_WRITE_TEX_CACHE )|| defined( D_APPLY_DISPLACEMENT ) || defined ( D_TESS_ON_AMD )
    OUTPUT( vec4,   mTexCoordVec2_mTexBorderVec2,    TEXCOORD3 )
#endif
    // We're using mfDistForFade for the water fade coefficient.   
    OUTPUT( vec4,   mSmoothNormalVec3_mfDistForFade, TEXCOORD4 )
#if defined( D_PLATFORM_OPENGL ) && !defined( D_TESS_SHADERS_PRESENT ) && ( defined( D_READ_TEX_CACHE ) || defined( D_WRITE_TEX_CACHE ) )
    OUTPUT( vec4,   mTexCoordsDPDUVec4,              TEXCOORD6 )
    OUTPUT( vec4,   mTexCoordsDPDVVec4,              TEXCOORD7 )
#endif
#endif // !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT )
#if defined ( D_OUTPUT_MOTION_VECTORS ) || defined ( D_APPLY_DISPLACEMENT ) || defined ( D_WRITE_CACHE_FEEDBACK )
    OUTPUT( vec4, mScreenSpacePositionVec4,        TEXCOORD5 )
#if  defined( D_FORWARD_RENDERER )
    OUTPUT( vec4, mPrevScreenPosition,             TEXCOORD17 )
#endif
#endif

DECLARE_OUTPUT_END
#endif

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Vertex Main
///
///     @brief      Vertex Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------

#if defined( D_PLATFORM_ORBIS ) && ( !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT ) )
#pragma argument(indirectdraw)
#endif

VERTEX_MAIN_SRT
{    
    vec4 lWorldPositionVec4;        
    vec4 lScreenSpacePositionVec4;
    
    vec3   lLocalPositionVec3;
    vec3   lSmoothNormalVec3;
    //vec3   lFaceNormalVec3;
    vec4   lTiles1Vec4;

    lLocalPositionVec3   = IN( mkLocalPositionVec4 ).xyz;

#ifdef D_MESH

    //lFaceNormalVec3       = IN( mkLocalNormalVec4 ).xyz;
    lSmoothNormalVec3     = IN( mkLocalNormalVec4 ).xyz;

#else

#ifdef D_OCTAHEDRON_NORMALS
    #if defined ( D_DISABLE_TERRAIN_ENCODE )
    lSmoothNormalVec3 = vec3( IN( mkLocalNormalVec4 ).x, IN( mkLocalNormalVec4 ).y, IN( mkLocalPositionVec4 ).w );
    #elif defined ( D_DISABLE_TERRAIN_UVS )
    lSmoothNormalVec3    = OctahedronNormalDecode( IN( mkLocalNormalVec4 ).xy );
    #else
    //lFaceNormalVec3      = OctahedronNormalDecode( IN( mkLocalNormalVec4 ).xy );
    lSmoothNormalVec3    = OctahedronNormalDecode( IN( mkLocalNormalVec4 ).zw );
    #endif
#else
    //lFaceNormalVec3      = IN( mkFaceNormalVec3 ).xyz;
    lSmoothNormalVec3    = IN( mkSmoothNormalVec3 ).xyz;
#endif

#endif

#ifdef D_MESH
    lTiles1Vec4          = vec4( 1.0, 1.0, 1.0, 1.0 );
#else
    lTiles1Vec4          = IN( mkCustom1Vec4 );
#endif

#if defined( D_PLATFORM_OPENGL ) && !defined( D_MESH ) && !defined( D_TESS_SHADERS_PRESENT ) && ( defined( D_READ_TEX_CACHE ) || defined( D_WRITE_TEX_CACHE ) )
    vec3 lDpDu = OctahedronNormalDecode( IN( mkCustom2Vec4 ).zw );

    lDpDu -= lSmoothNormalVec3 * dot( lSmoothNormalVec3, lDpDu );
    lDpDu = normalize( lDpDu );

    vec3 lDpDv = normalize( cross( lSmoothNormalVec3, lDpDu ) );

    OUT( mTexCoordsDPDUVec4 ) = vec4( lDpDu, 0.0 );
    OUT( mTexCoordsDPDVVec4 ) = vec4( lDpDv, 0.0 );
#endif

    //-----------------------------------------------------------------------------
    ///
    ///     World Transform
    ///
    //-----------------------------------------------------------------------------
#if defined( D_ASTEROID ) || defined( D_MESH ) || defined( D_TESS_SHADERS_PRESENT )
    lWorldPositionVec4          = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, vec4( lLocalPositionVec3, 1.0 ) );
#else
    lWorldPositionVec4.xyz      = lLocalPositionVec3 + lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz;
    lWorldPositionVec4.w        = 1.0;
#endif

#if defined ( D_OUTPUT_MOTION_VECTORS ) || defined ( D_APPLY_DISPLACEMENT ) || defined ( D_WRITE_CACHE_FEEDBACK )
    OUT(mScreenSpacePositionVec4) = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4);
#if defined( D_FORWARD_RENDERER )
    OUT(mPrevScreenPosition) = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gPrevViewProjectionMat4, lWorldPositionVec4);
#endif
#endif

#if !defined( D_ASTEROID ) && !defined( D_MESH ) && !defined ( D_TERRAIN_LOD )

    float lfFlattenUniform = 0.0;

#if !defined(D_CUSTOM_TILES)
    lfFlattenUniform = lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.z;
#endif

#if !defined( D_WRITE_TEX_CACHE )

    // Flatten terrain down as we go to space... D_TERRAIN_LOD starts using this from about 2000m above the planet surface.
    //float lfRadius = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w + lUniforms.mpCustomPerMaterial.gWaterFogVec4.r * ( 1.0 - lfFlattenUniform );
    //float lfWorldHeight = length( lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );

    const float kfFlattenStartDistance = 6000.0;
    const float kfFlattenEndDistance = 10000.0;
    const float kfFlattenStartDistanceSquared = kfFlattenStartDistance * kfFlattenStartDistance;
    
    vec3 lViewDirVec3 = lUniforms.mpPerFrame.gViewPositionVec3 - lWorldPositionVec4.xyz;
    float lfViewLengthSquared = dot( lViewDirVec3, lViewDirVec3 );

    if ( lfViewLengthSquared > kfFlattenStartDistanceSquared )
    {
        vec3 lFlatWorldPositionNormVec3 = normalize( lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );
        vec3 lFlatWorldPositionVec3 = lFlatWorldPositionNormVec3 * ( lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w - 1.0 );

        float lfFlattenFactor = ( sqrt( lfViewLengthSquared ) - kfFlattenStartDistance ) / ( kfFlattenEndDistance - kfFlattenStartDistance );
        lfFlattenFactor = clamp( lfFlattenFactor, 0.0, 1.0 );
        lWorldPositionVec4.xyz = mix( lWorldPositionVec4.xyz, lFlatWorldPositionVec3 + lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz, lfFlattenFactor );
        lSmoothNormalVec3 = mix( lSmoothNormalVec3, lFlatWorldPositionNormVec3, lfFlattenFactor );
    }
#endif

#endif // !defined( D_ASTEROID ) && !defined( D_MESH )


#if !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT ) || ( defined( D_FADE_REALLY ) && defined( D_LOCAL_FADE ) )
    OUT( mLocalPositionVec4 ).xyz   = lLocalPositionVec3;
    OUT( mLocalPositionVec4 ).w     = 1.0;
#endif


    //-----------------------------------------------------------------------------
    ///
    ///     Screen Transform
    ///
    //-----------------------------------------------------------------------------
    #ifdef D_REFLECT_DUALP
    {
        vec3 lReflectPosition       = GetWorldUp( lWorldPositionVec4.xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4 ) * lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w + lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        lScreenSpacePositionVec4    = CalcDualParaboloidScreenPosition( 
                                        lUniforms.mpPerFrame.gViewMat4, 
                                        vec4(lReflectPosition.x, lReflectPosition.y, lReflectPosition.z, lWorldPositionVec4.w), 
                                        lUniforms.mpPerFrame.gClipPlanesVec4.xy);
    }
    #else
    {
        lScreenSpacePositionVec4 = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );
    }    
    #endif

#if !defined(D_DEPTHONLY) || defined( D_TESS_SHADERS_PRESENT )

    //-----------------------------------------------------------------------------
    ///
    ///     Normals
    ///
    //-----------------------------------------------------------------------------

    #if defined ( D_DISABLE_TERRAIN_UVS )
    vec2 lTexCoords = vec2( 0.0, 0.0 );
    #else
    vec2 lTexCoords = IN( mkLocalNormalVec4 ).xy;
    #endif
    
    vec2 lvBorder = vec2( 0.0, 0.0 );

    #if defined( D_PLATFORM_OPENGL ) && !defined( D_TESS_SHADERS_PRESENT ) && !defined( D_MESH )
    vec2 lvQuadCenter = IN( mkCustom2Vec4 ).xy;
    #ifdef D_WRITE_TEX_CACHE
    vec2 lResolution = lUniforms.mpCustomPerMesh.gTextureCoordsAdjustVec4.xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    vec2 lInvRes = vec2( 1.0, 1.0 ) / vec2( lResolution );
    vec2 lOnePixelIn = sign( lvQuadCenter - lTexCoords ) * lInvRes;
    lvBorder = 0.45 * lOnePixelIn;
    #else
    vec2 lInvRes = vec2( 1.0 / 8192.0, 1.0 / 8192.0 );
    vec2 lOnePixelIn = sign( lvQuadCenter - lTexCoords ) * lInvRes;
    lvBorder = 0.5 * lOnePixelIn;
    #endif
    lTexCoords += lvBorder;
    #endif

#if defined( D_READ_TEX_CACHE ) || defined( D_WRITE_CACHE_FEEDBACK ) || defined( D_WRITE_TEX_CACHE ) || defined( D_APPLY_DISPLACEMENT ) || defined ( D_TESS_ON_AMD )
    OUT( mTexCoordVec2_mTexBorderVec2 ) = vec4( lTexCoords.x, lTexCoords.y, lvBorder.x, lvBorder.y );
#endif
#if !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT ) || defined ( D_TERRAIN_T_SPLIT )
    float lfWaterFadeCoeff = lTiles1Vec4.w < 0.0 ? 1.0 : 0.0;
    if (  lfWaterFadeCoeff > 0.0 )
    {
        lfWaterFadeCoeff = 0.0;
        float lfCameraHeight = lUniforms.mpCustomPerMaterial.gTerrainDistancesVec4.x;
        if ( ( lfCameraHeight > 500.0 ) )
        {
            lfWaterFadeCoeff                 = saturate( ( lfCameraHeight - 700.0 ) / 200.0 + 0.5 );
            vec3  lFlatWorldPositionNormVec3 = normalize( lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );
            lSmoothNormalVec3                = mix( lSmoothNormalVec3, lFlatWorldPositionNormVec3, max( lfWaterFadeCoeff - 0.1, 0.0 ) );
        }
    }

    OUT( mSmoothNormalVec3_mfDistForFade ) = vec4( lSmoothNormalVec3, lfWaterFadeCoeff );
#endif
    
    //----------------------------------------------------------------------------- 
    ///
    ///     Terrain Values
    ///
    //-----------------------------------------------------------------------------

    // Indices passed in from dualcontouring
    float lIndex1 = lTiles1Vec4.x;
    float lIndex2 = lTiles1Vec4.y;

#if defined ( D_TERRAIN_T_SPLIT )
    OUT( mLocalPositionVec4 ).w = lTiles1Vec4.z > 0.5 ? lIndex2 : lIndex1;
#else

    // And this is an optimization to save a LOT of time in GetBlendedColour
#ifdef D_LOW_QUALITY
    if ( lTiles1Vec4.z > 0.5 )
    {
        float lfSwap    = lIndex1;
        lIndex1         = lIndex2;
        lIndex2         = lfSwap;
    }
#endif
   

#ifdef D_MESH
    {
        lIndex1 = lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.z;
        lIndex2 = lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w;
        //vec3 lUpDirection = GetWorldUp( lWorldPositionVec4.xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4 ) ;
        vec3 lUpDirection = vec3 ( 0.0, 1.0, 0.0 );
        //Skew the flat towards the top slightly, to show more slope [-1 : 1] . [-0.8 : 1] . Clamped to 0
        lTiles1Vec4.z = max( (0.9 * ( 1.0 + dot( lUpDirection, normalize( lSmoothNormalVec3 )))) - 0.8, 0.0 );  //Tile blend
        lTiles1Vec4.w = 0.0;  //Edited/Underwater
    }
#endif

#if !defined ( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT )
    OUT( mTileVec4.x )        = lIndex1;
    OUT( mTileVec4.y )        = lIndex2;
    OUT( mTileVec4.z )        = lTiles1Vec4.z;
    OUT( mTileVec4.w )        = lTiles1Vec4.w;
#endif

#endif // D_TERRAIN_T_SPLIT

#endif // !defined( D_DEPTHONLY ) || defined( D_TESS_SHADERS_PRESENT )

    //-----------------------------------------------------------------------------
    ///
    ///     Output
    ///
    //-----------------------------------------------------------------------------
    #if defined( D_WRITE_TEX_CACHE )
    lTexCoords = lTexCoords * lUniforms.mpCustomPerMesh.gTextureCoordsAdjustVec4.xy + lUniforms.mpCustomPerMesh.gTextureCoordsAdjustVec4.zw;
    #ifdef D_PLATFORM_OPENGL
    lTexCoords.y = 1.0 - lTexCoords.y;
    #endif

    SCREEN_POSITION = vec4( lTexCoords * 2.0 - 1.0, 0.5, 1.0 );
#if ! defined( D_TESS_SHADERS_PRESENT )
    WRITE_SCREEN_SLICE(0);
#endif
    #else

    SCREEN_POSITION = lScreenSpacePositionVec4;
#if ! defined( D_TESS_SHADERS_PRESENT )
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
#endif
    #endif
}



