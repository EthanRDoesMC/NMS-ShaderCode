////////////////////////////////////////////////////////////////////////////////
///
///     @file       ScreenFragmentShader.h
///     @author     User
///     @date       
///
///     @brief      ScreenFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

//-----------------------------------------------------------------------------
//      Include files

#include "UberCommon.h"

#include "Common/CommonUniforms.shader.h"

#if defined( D_DEFER ) 
#include "Common/CommonGBuffer.shader.h"
#else
#include "Common/CommonLighting.shader.h"
#include "Common/CommonPlanet.shader.h"
#endif

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
DECLARE_INPUT

INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec4, mTexCoordsVec4, TEXCOORD0)

//#ifdef D_USES_WORLD_POSITION
#if defined( D_DEPTHONLY )
flat INPUT(vec4, mWorldPositionVec3_mfSpare, TEXCOORD1)
#else
INPUT(vec4, mWorldPositionVec3_mfSpare, TEXCOORD1)
#endif
//#endif

#ifdef _F21_VERTEXCOLOUR
INPUT(vec4, mColourVec4, TEXCOORD2)
#endif

#if !defined( _F01_DIFFUSEMAP ) || defined( _F22_TRANSPARENT_SCALAR )
INPUT(vec4, mMaterialVec4, TEXCOORD4)
#endif

#ifdef D_USES_VERTEX_NORMAL
INPUT(vec3, mTangentSpaceNormalVec3, TEXCOORD5)
#endif

#if !defined( D_DEPTHONLY ) && !defined( D_DEPTH_CLEAR )

#ifdef _F20_PARALLAXMAP
INPUT(vec3, mTangentSpaceEyeVec3, TEXCOORD6)
#endif

#if 0 // defined( D_OUTPUT_MOTION_VECTORS ) && defined( _F14_UVSCROLL )
INPUT(vec4, mPrevTexCoordsVec4, TEXCOORD8)
#endif

#if !defined( D_DEFER ) && !defined( _F07_UNLIT )
flat INPUT(mat3, mUpMatrixMat3, TEXCOORD9)
#endif

//#if defined( _F44_IMPOSTER )
//    INPUT( vec3, mShadowWorldPositionVec3,  TEXCOORD8 )
//#endif

#if defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
INPUT(vec3, mTangentMatRow1Vec3, TEXCOORD13)
INPUT(vec3, mTangentMatRow2Vec3, TEXCOORD14)
INPUT(vec3, mTangentMatRow3Vec3, TEXCOORD15)
#endif


#if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND )
INPUT(vec3, mCenteralNormalVec3, TEXCOORD16)
#endif



#ifdef D_OUTPUT_MOTION_VECTORS
INPUT_VARIANT(vec4, mPrevScreenPosition, TEXCOORD17, HAS_MOTION_VECTORS)
#endif

#endif


#ifdef D_USE_SCREEN_POSITION
INPUT(vec4, mScreenSpacePositionVec4, TEXCOORD18)
#endif

flat INPUT(vec3, mfFadeValueForInstance_mfLodIndex_mfShearMotionLength, TEXCOORD19)

//INPUT_FRONTFACING

DECLARE_INPUT_END



#ifdef D_DEFER
    #include "OutputDeferred.shader.h"
#else
    #include "OutputForward.shader.h"
#endif


//-----------------------------------------------------------------------------
//    Functions


//-----------------------------------------------------------------------------
///
///     Fragment Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if defined( D_DEPTH_CLEAR )         
FRAGMENT_MAIN_COLOUR_DEPTH_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{

#ifdef D_DEPTH_CLEAR
    FRAGMENT_COLOUR = vec4( 0.0, 0.0, 0.0, 0.0 );
    FRAGMENT_DEPTH = D_DEPTH_CLEARVALUE;
    return;
#endif

    vec4 lFragmentColourVec4;
    vec3 lNormalVec3 = vec3(0.0,1.0,0.0);

    #ifdef D_TEXCOORDS
    //{
        vec4 lTexCoordsVec4;
        lTexCoordsVec4 = IN( mTexCoordsVec4 );

        // WARNING THIS IS TEMPORARY AND ALSO SHIT
#ifdef D_PLATFORM_OPENGL
        lTexCoordsVec4.y = 1.0 - lTexCoordsVec4.y;
#endif

    //}
    #endif

    //-----------------------------------------------------------------------------
    ///     Diffuse
    //-----------------------------------------------------------------------------
    vec4 lDiffuseColourVec4;

    #ifdef _F01_DIFFUSEMAP
    {
        lDiffuseColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lTexCoordsVec4.xy );
        lDiffuseColourVec4.xyz = GammaCorrectInput( lDiffuseColourVec4.xyz );
        lDiffuseColourVec4.w = 1.0 - lDiffuseColourVec4.w;
    }
    #else
    {
        lDiffuseColourVec4 = IN( mMaterialVec4 );
    }
    #endif

    #ifdef _F21_VERTEXCOLOUR
    {
        lDiffuseColourVec4 *= IN( mColourVec4 );	
    }
    #endif

    lFragmentColourVec4 = lDiffuseColourVec4;

    //-----------------------------------------------------------------------------
    ///
    ///     Transparency
    ///
    //-----------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    ///
    ///     Output
    ///
    //-----------------------------------------------------------------------------	

    int liMaterialID = 0;

#ifdef _F10_NORECEIVESHADOW
    liMaterialID |= D_NORECEIVESHADOW;
#endif

#ifdef _F50_DISABLE_POSTPROCESS
    liMaterialID |= D_DISABLE_POSTPROCESS;
#endif

#ifdef _F07_UNLIT
    liMaterialID |= D_UNLIT;
#endif

//#if defined( _F57_DETAIL_OVERLAY )
//    liMaterialID |= D_DETAILOVERLAY;
//#endif

    liMaterialID |= D_CLAMP_AA;

    vec4 lOutColours0Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours1Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours2Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours3Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours4Vec4 = vec4(0, 0, 0, 0);

#if defined( D_TILED_LIGHTS )
    #if defined( D_PLATFORM_METAL )
    const device metal::atomic_int* liVisibleLights = nullptr;
    int laLightIndices;
    #else
    int liVisibleLights = 0;
    int laLightIndices[D_TILE_MAX_LIGHT_COUNT];
    #endif
#endif

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
        IN( mWorldPositionVec3_mfSpare ).xyz,
        lNormalVec3,
        liMaterialID,
        0.0,
        1.0,
        0.0,
        0.0,
        #ifdef D_USE_SCREEN_POSITION 
        IN( mScreenSpacePositionVec4 ).xyzw,
        #else
        float2vec4( 0.0 ),
        #endif 
        #ifdef D_OUTPUT_MOTION_VECTORS 
        HAS_MOTION_VECTORS ? IN( mPrevScreenPosition ).xyzw : float2vec4( 0.0 ) ,
        #else
        float2vec4( 0.0 ),
        #endif 
#if !defined( D_DEFER ) && !defined( _F07_UNLIT ) || defined( D_FORWARD_RENDERER )
        GetInverseWorldUpTransform(lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gShadowMap),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gCloudShadowMap),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gDualPMapBack),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gDualPMapFront),
#endif
#if defined( D_TILED_LIGHTS )
        liVisibleLights,
        laLightIndices,
        0.0,
#endif
        0.0,
        false,
        -1.0
        );

#if defined( D_LIT_WITH_MASK )
    {
        FRAGMENT_COLOUR0 = lOutColours0Vec4;
        FRAGMENT_COLOUR1 = vec4( 1.0, 0.0, 0.0, 1.0 );
    }
#elif !defined( D_ATTRIBUTES )
    FRAGMENT_COLOUR  = lOutColours0Vec4;
#else
    FRAGMENT_COLOUR0 = lOutColours0Vec4;
    FRAGMENT_COLOUR1 = lOutColours1Vec4;
    FRAGMENT_COLOUR2 = lOutColours2Vec4;
    FRAGMENT_COLOUR3 = lOutColours3Vec4;
#endif

}

