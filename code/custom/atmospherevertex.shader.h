////////////////////////////////////////////////////////////////////////////////
///
///     @file       SkyVertex.h
///     @author     User
///     @date       
///
///     @brief      SkyVertexShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_VERTEX
#define D_VERTEX
#endif

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "Custom/AtmosphereCommon.h"

#include "Common/CommonVertex.shader.h"
#include "Common/CommonDepth.shader.h"
#if defined (D_SCATTERING_FORWARD)
//TF_BEGIN
#include "Common/CommonScattering.shader.h"
//TF_END
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

    INPUT( vec4, mkLocalPositionVec4, POSITION0 )
    INPUT( vec4, mkTexCoordsVec4,     TEXCOORD0     )
    INPUT( vec4, mkLocalNormalVec4,   TEXCOORD1     )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------
DECLARE_OUTPUT

    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE

    OUTPUT( vec4, mWorldPositionVec4,        TEXCOORD0 )
#if defined( D_RINGS ) || defined( D_RAINBOW )
    OUTPUT( vec4, mLocalPositionVec4, TEXCOORD1 )
#else
    OUTPUT( vec4, mTexCoordsVec4,            TEXCOORD1 )
    OUTPUT( vec4, mWorldNormalVec3_mfDistanceFromPlanet,  TEXCOORD2 )
#endif
#ifndef D_REFLECT    
    OUTPUT( vec4, mScreenSpacePositionVec4,  TEXCOORD3 )
#endif    

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
//    Functions

vec4 RotatePlanetDetail( 
    vec4  lPlanetToViewVec4,
    vec4  lPlanetToViewNormalVec4,
    vec4  lPlanetToViewTangentVec4,
    vec3  lLocalPosVec3 )
{   
#if defined( D_PLANET_NEAR )
    float lfSign = 1.0;
#else
    float lfSign = -1.0;
#endif

    mat4 lRotMat4;
    lRotMat4[ 0 ] = lPlanetToViewTangentVec4 * lfSign;
    lRotMat4[ 1 ] = lPlanetToViewVec4 * lfSign;
    lRotMat4[ 2 ] = lPlanetToViewNormalVec4;
    lRotMat4[ 3 ] = vec4( 0.0, 0.0, 0.0, 1.0 );

    return MUL( lRotMat4, vec4( lLocalPosVec3, 1.0 ) );
}

float GetHeight(
    SAMPLERCUBEARG(lHeightMap),
    vec3    lRotatedLocalVec3,
    vec3    lViewPositionVec3,
    vec3    lPlanetPositionVec3,
    float   lfPlanetRadius,
    float   lfHeightDivider )
{
    float lfHeight = 0.0;
    const float kfHeightThreshold = lfPlanetRadius + 1000.0;

    vec3 lViewDistVec3 = lViewPositionVec3 - lPlanetPositionVec3;
    if ( dot( lViewDistVec3, lViewDistVec3 ) > kfHeightThreshold * kfHeightThreshold )
    {
        lfHeight  = textureCube( lHeightMap, normalize( lRotatedLocalVec3 ) ).r;
        lfHeight *= lfHeightDivider / lfPlanetRadius;
    }

    return lfHeight;
}

vec3 Flatten(
    vec3    lWorldPositionVec3,
    vec3    lViewPositionVec3,
    vec3    lPlanetPositionVec3,
    float   lfPlanetRadius )
{
    const float kfFlattenStartDistance  =  6000.0;
    const float kfFlattenEndDistance    = 12000.0;
    const float kfFlattenHeight         =  2200.0;

    const float kfFlattenStartDistanceSquared = kfFlattenStartDistance * kfFlattenStartDistance;
    
    vec3  lViewDirVec3        = lViewPositionVec3 - lWorldPositionVec3;
    float lfViewLengthSquared = dot( lViewDirVec3, lViewDirVec3 );
    float lfViewHeight        = length( lViewPositionVec3 - lPlanetPositionVec3 ) - lfPlanetRadius;

    if ( lfViewLengthSquared > kfFlattenStartDistanceSquared || 
         lfViewHeight        < kfFlattenHeight )
    {
        vec3 lFlatWorldPositionNormVec3 = normalize( lWorldPositionVec3 - lPlanetPositionVec3 );
        vec3 lFlatWorldPositionVec3     = lFlatWorldPositionNormVec3 * ( lfPlanetRadius - 1.0 );

        float lfFlattenFactor   = 0.0;
        if ( lfViewLengthSquared > kfFlattenStartDistanceSquared )
        {
            lfFlattenFactor = ( sqrt( lfViewLengthSquared ) - kfFlattenStartDistance ) / ( kfFlattenEndDistance - kfFlattenStartDistance );
        }
        if ( lfViewHeight < kfFlattenHeight )
        {
            lfFlattenFactor = max( lfFlattenFactor, ( kfFlattenHeight - lfViewHeight ) / ( kfFlattenHeight ) );
        }

        lfFlattenFactor         = clamp( lfFlattenFactor, 0.0, 1.0 );
        lWorldPositionVec3      = mix( lWorldPositionVec3, lFlatWorldPositionVec3 + lPlanetPositionVec3, lfFlattenFactor );
    }

    return lWorldPositionVec3;
}

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
VERTEX_MAIN_SRT
{
    vec4 lWorldPositionVec4;
#if defined( D_PLANET )    
    vec4 lRotatedLocalVec4  = RotatePlanetDetail( lUniforms.mpCommonPerMesh.gPlanetToViewVec4,
                                                  lUniforms.mpCommonPerMesh.gPlanetToViewNormalVec4,
                                                  lUniforms.mpCommonPerMesh.gPlanetToViewTangentVec4,
                                                  IN( mkLocalPositionVec4 ).xyz );

    lWorldPositionVec4      = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, lRotatedLocalVec4 );
#else
    lWorldPositionVec4      = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, vec4( IN( mkLocalPositionVec4 ).xyz, 1.0 ) );
#endif

    // Resize sphere to correct radius
#if defined( D_CLOUD ) || defined( D_ATMOSPHERE_SHADOW ) || defined( D_PLANET )
    {
        float lfSize = 1.0;

    #if defined( D_ATMOSPHERE_SHADOW ) 
        lfSize = 1.005;
    #elif defined( D_CLOUD ) 
        lfSize = 1.01;
    #elif defined( D_PLANET_NEAR )
        float lfHeight = GetHeight( SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gWaterMap ),
                                    lRotatedLocalVec4.xyz,
                                    lUniforms.mpPerFrame.gViewPositionVec3,
                                    lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz,
                                    lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w,
                                    lUniforms.mpCustomPerMaterial.gTerrainDistancesVec4.y );        
        lfSize    = 1.0 + lfHeight;
    #endif

        lWorldPositionVec4.xyz  = normalize( lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) * ( lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w * lfSize );
        lWorldPositionVec4.xyz += lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

    #if defined( D_PLANET_NEAR )
        lWorldPositionVec4.xyz  = Flatten(  lWorldPositionVec4.xyz,
                                            lUniforms.mpPerFrame.gViewPositionVec3,
                                            lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz,
                                            lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w );
    #endif
    }
#endif

    OUT( mWorldPositionVec4 )       = lWorldPositionVec4;

#if defined( D_RINGS ) 
    OUT( mLocalPositionVec4 ).xyz                    = lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
    OUT( mLocalPositionVec4 ).w                      = length( IN( mkLocalPositionVec4 ).xyz );
#elif defined( D_RAINBOW )
    OUT( mLocalPositionVec4 )                        = vec4( IN( mkLocalPositionVec4 ).xyz, 1.0 );
#else
    OUT( mWorldNormalVec3_mfDistanceFromPlanet ).xyz = normalize( lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );
    OUT( mWorldNormalVec3_mfDistanceFromPlanet ).w   = length( lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;

    OUT( mTexCoordsVec4 )           = IN( mkTexCoordsVec4 );
#endif

    vec4 lScreenSpacePositionVec4 = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );

#ifdef D_REFLECT
    {
        SCREEN_POSITION = CalcDualParaboloidScreenPosition( lUniforms.mpPerFrame.gViewMat4, lWorldPositionVec4, lUniforms.mpPerFrame.gClipPlanesVec4.xy );
    }
#else
    {
        OUT( mScreenSpacePositionVec4 ) = lScreenSpacePositionVec4;
        SCREEN_POSITION = lScreenSpacePositionVec4;
        WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
    }
#endif
}
