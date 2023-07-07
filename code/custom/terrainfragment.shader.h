////////////////////////////////////////////////////////////////////////////////
///
///     @file       TerrainFragment.h
///     @author     User
///     @date       
///
///     @brief      TerrainFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#define D_TERRAIN

// If we're using a simplified noise context and we're on switch
// just disable noise altogether
#if defined( D_SIMPLIFIED_NOISE ) && defined( D_PLATFORM_SWITCH )
#define D_DISABLE_NOISE
#endif

#if defined ( D_PLATFORM_SWITCH )
#define D_FORCE_HALF
#define D_FORCE_HALF_UV
#endif

//#if defined( D_PLATFORM_ORBIS ) && defined( D_USE_HALF_PRECISION )
//#pragma argument ( realtypes  )
//#pragma argument ( target=neo )
//#endif

#if defined ( D_PLATFORM_ORBIS ) && defined( D_STOCHASTIC_TERRAIN )

#pragma argument (O4; fastmath; scheduler=minpressure)

#if defined ( D_TERRAIN_T_SPLIT )
#pragma argument ( targetoccupancy_atallcosts=7 )
#else
#pragma argument ( targetoccupancy_atallcosts=5 )
#endif

#endif

#if defined ( D_PLATFORM_SWITCH ) && defined ( D_STOCHASTIC_TERRAIN )
#pragma optionNV(fastmath on)
#endif

#if !defined( D_REFLECT_WATER_UP ) && !defined( D_REFLECT_WATER ) && !defined( D_REFLECT_DUALP ) && !defined( D_WRITE_TEX_CACHE )
    #define D_FADE
#endif

#if defined( D_FADE ) && ( !(defined( D_TERRAIN_X_FACING ) || defined( D_TERRAIN_Y_FACING ) || defined( D_TERRAIN_Z_FACING )  || defined( D_TERRAIN_N_FACING ) || defined( D_STOCHASTIC_TERRAIN_NO_FADE ) ) )
    #define D_FADE_REALLY
#endif

#if !defined( D_PLATFORM_SWITCH ) && !defined( D_PLATFORM_XBOXONE ) && !defined( D_PLATFORM_IOS )
    #define D_LOCAL_FADE
#endif

#if (defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS )) && defined( D_DEFER )
#pragma PSSL_target_output_format(target 1 FMT_UNORM16_ABGR)
#endif

#if /*defined( D_ASTEROID ) ||*/ defined( D_REFLECT_WATER ) || defined( D_REFLECT_WATER_UP ) || defined( D_REFLECT_DUALP ) || defined(D_TERRAIN_EDITS)
#define _F50_DISABLE_POSTPROCESS
#endif

#ifdef GL_AMD_shader_explicit_vertex_parameter
#extension GL_AMD_shader_explicit_vertex_parameter : enable
#endif

//#if defined( D_REFLECT_WATER ) || defined( D_REFLECT_DUALP ) 
//#define D_LOW_QUALITY
//#endif

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"

#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"

//-----------------------------------------------------------------------------
//      Global Data



//
// In TerrainCommon we have our uniforms specific to terrain declared. PLUS, IMPORTANTLY we have the SRT buffer declared (gUniforms). 
// This MUST be included after CommonUniforms, but before all the other stuff that references gUniforms.
//

#include "Custom/TerrainCommon.h"

//
// Must be included after gUniforms is declared as they reference it.
//
#define D_TEXTURE_ARRAYS
#if defined( D_DEPTHONLY )
#include "Common/CommonGBuffer.shader.h"
#else
#include "Common/CommonTriPlanarTexturing.shader.h"
#ifdef D_DEFER
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonFragment.shader.h"
#include "Common/CommonGBuffer.shader.h"
#include "OutputDeferred.shader.h"

#else
//TF_BEGIN
#if !defined( D_FORWARD_RENDERER )
#define D_NO_SHADOWS
#endif
//TF_END
#ifdef D_TERRAIN_EDITS
#include "Common/CommonDepth.shader.h"  //indirectly included by the lighting shader, so need them still
#include "Common/CommonGBuffer.shader.h"
#else
#include "Common/CommonLighting.shader.h"
#endif
#include "OutputForward.shader.h"
#endif // D_DEFER
#endif // defined( D_DEPTHONLY ) 
#include "Common/CommonFade.shader.h" 
#include "Common/CommonNoise.shader.h" 
#include "Common/CommonFragment.shader.h" 
#include "Common/CommonTerrainConstants.shader.h"

//-----------------------------------------------------------------------------
//      Constants 

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------

#if defined( D_DEPTHONLY ) && ( !defined( D_FADE_REALLY ) || !defined( D_LOCAL_FADE ) )
    /* Empty Input */
    #if defined ( D_PLATFORM_IOS ) && defined ( D_FADE_REALLY )
    DECLARE_INPUT
        INPUT_SCREEN_POSITION    
    DECLARE_INPUT_END
    #endif
#else
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

#if !defined( D_DEPTHONLY ) || ( defined( D_FADE_REALLY ) && defined( D_LOCAL_FADE ) )
    INPUT( vec4, mLocalPositionVec4, TEXCOORD1 )
#endif
#if !defined( D_DEPTHONLY )
#if !defined ( D_TERRAIN_T_SPLIT )
    INPUT( vec4, mTileVec4, TEXCOORD2 )
#endif
#if defined( D_READ_TEX_CACHE ) || defined( D_WRITE_CACHE_FEEDBACK )
    INPUT( vec4, mTexCoordVec2_mTexBorderVec2, TEXCOORD3 )
#endif
    // We're using mfDistForFade for the water fade coefficient.   
    INPUT( vec4, mSmoothNormalVec3_mfDistForFade, TEXCOORD4 )
#if ( defined( D_TESS_SHADERS_PRESENT ) || defined( D_PLATFORM_OPENGL ) ) && ( defined( D_READ_TEX_CACHE ) || defined( D_WRITE_TEX_CACHE ) )
    INPUT( vec4, mTexCoordsDPDUVec4, TEXCOORD6 )
    INPUT( vec4, mTexCoordsDPDVVec4, TEXCOORD7 )
#endif
#endif // !defined( D_DEPTHONLY )
#if defined ( D_OUTPUT_MOTION_VECTORS ) || defined( D_FORWARD_RENDERER )
INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD5 )
INPUT( vec4, mPrevScreenPosition, TEXCOORD17 )
#endif
    
DECLARE_INPUT_END
#endif

//-----------------------------------------------------------------------------
//    Functions

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
#if defined( D_DEPTHONLY )
    #if ( !defined( D_FADE_REALLY ) || !defined( D_LOCAL_FADE ) ) && defined ( D_PLATFORM_METAL )
        VOID_MAIN_SRT
    #else
        VOID_MAIN_SRT
    #endif
#elif defined( D_FADE_REALLY )
    #if defined( D_OUTPUT_MOTION_VECTORS ) && defined(D_FORWARD_RENDERER)
        FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(HAS_MOTION_VECTORS)
    #elif defined(D_BLOOM) || defined(D_DOF)
        FRAGMENT_MAIN_COLOUR01_SRT
    #else
    FRAGMENT_MAIN_COLOUR_SRT
    #endif
#elif defined ( D_PLATFORM_SWITCH )
    FRAGMENT_MAIN_COLOUR_EARLYZ_SRT
#else
	#if defined( D_OUTPUT_MOTION_VECTORS ) && defined(D_FORWARD_RENDERER)
	    FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(HAS_MOTION_VECTORS)
	#elif defined(D_BLOOM) || defined(D_DOF)
		FRAGMENT_MAIN_COLOUR01_SRT
	#else
    FRAGMENT_MAIN_COLOUR_SRT
	#endif
#endif
{
#if defined( D_REFLECT_WATER )
    {
        vec3  lWorldUpVec3;
        float lfHeight;
        lWorldUpVec3 = GetWorldUp(  IN( mLocalPositionVec4 ).xyz + lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz,
                                    lUniforms.mpCommonPerMesh.gPlanetPositionVec4, lfHeight );

        if ( lfHeight < lUniforms.mpCustomPerMaterial.gWaterFogVec4.r - 0.5 )
        {
            discard;
        }
    }
#endif

//#if !defined ( D_DEPTHONLY )
//
//    vec3 lUpVector = normalize( IN( mWorldPositionVec4 ).xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz );
//    float lfScale = abs( dot( lUpVector, IN( mSmoothNormalVec3_mfDistForFade ).xyz ) );
//
//    #if !defined( D_ATTRIBUTES )
//
//    FRAGMENT_COLOUR0 = vec4( 0.0f, 1.0f * lfScale + 0.2f, 0.0f, 1.0f );
//
//    #else
//
//    FRAGMENT_COLOUR0 = vec4( 0.0f, 1.0f * lfScale + 0.2f, 0.0f, 1.0f );;
//    FRAGMENT_COLOUR1 = vec4( 0.0f, 0.0f, 0.0f, 1.0f );;
//    FRAGMENT_COLOUR2 = vec4( 0.0f, 0.0f, 0.0f, 1.0f );;
//    FRAGMENT_COLOUR3 = vec4( 0.0f, 0.0f, 0.0f, 1.0f );;
//
//    #endif
//
//#endif

#if defined( D_FADE_REALLY )
    {                
#if defined( D_PLATFORM_XBOXONE )
        CheckFadeFragCoord( 
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ),
            IN_SCREEN_POSITION,
            lUniforms.mpCommonPerMesh.gfFadeValue,
            vec3( 0.0, 0.0, 1.0 ) );
#elif defined ( D_PLATFORM_SWITCH ) || defined ( D_PLATFORM_IOS )
        CheckFadeFragCoordNoVR(
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ),
            IN_SCREEN_POSITION,
            lUniforms.mpCommonPerMesh.gfFadeValue,
            256.0 );
#else
        vec3 lOffsetVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

        CheckFadeLocalSpaceWithOffsetCoord(
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ),
            IN( mLocalPositionVec4 ).xyz,
            lOffsetVec3,
            lUniforms.mpCommonPerMesh.gfFadeValue );
#endif

    }
#endif

#if defined( D_DEPTHONLY )
    return;
#else

    vec3  lFragmentColourVec3;
#if defined ( D_FORCE_HALF )
    half3  lNormalVec3;
#else
    vec3  lNormalVec3;
#endif

    float lfTextureSmallScaleFactor = 1.0;
    float lfTextureLargeScaleFactor = 1.0;

#if defined ( D_FORCE_HALF )
    half lfSpecular = 0.0;
#else
    float lfSpecular = 0.0;
#endif

    //-----------------------------------------------------------------------------
    ///
    ///     Triplanar Texturing
    ///
    //-----------------------------------------------------------------------------

#if defined ( D_FORCE_HALF )
    half lfMetallic = 0.0;
    half lfHeight = 0.0;
#else
    float lfMetallic = 0.0;
    float lfHeight = 0.0;
#endif
    float lfSubsurface = 0.0;
    
    float lfGlow = 0.0;
    float lfScaleFade = 0.0;
    float lfWaterFade = IN( mSmoothNormalVec3_mfDistForFade ).w;


    vec3  lWorldSpaceNormalVec3;
    vec3  lWorldSpacePositionVec3;
    vec3  lPlanetSpacePositionVec3;
    vec3  lViewSpacePositionVec3;
    vec3  lPlanetSpaceOffsetVec3;
    vec3  lLocalSpacePositionVec3;

    uint  luSlopeIndex1 = 0;
    uint  luSlopeIndex2 = 0;
    

    {   
        {
            vec3  lWorldSpaceOffsetVec3;
            vec3  lViewSpaceOffsetVec3;

            lWorldSpaceNormalVec3       = normalize( IN( mSmoothNormalVec3_mfDistForFade ).xyz );        
            
            lWorldSpaceOffsetVec3       = lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz;
            lPlanetSpaceOffsetVec3      = lWorldSpaceOffsetVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
            lViewSpaceOffsetVec3        = lWorldSpaceOffsetVec3 - lUniforms.mpPerFrame.gViewPositionVec3;

            lLocalSpacePositionVec3     = IN( mLocalPositionVec4 ).xyz;
            lWorldSpacePositionVec3     = lLocalSpacePositionVec3 + lWorldSpaceOffsetVec3;
            lPlanetSpacePositionVec3    = lLocalSpacePositionVec3 + lPlanetSpaceOffsetVec3;
            lViewSpacePositionVec3      = lLocalSpacePositionVec3 + lViewSpaceOffsetVec3;
        }

/*
#ifdef D_ASTEROID
        {
            lPlanetSpacePositionVec3  = lLocalSpacePositionVec3;
            lWorldSpacePositionVec3   = lLocalSpacePositionVec3;
            lPlanetSpaceOffsetVec3    = float2vec3( 0.0 );
        }
#endif
*/

#if !defined( D_CACHE_COLOUR ) && !defined( D_CACHE_NORMAL ) && !defined( D_CACHE_MASK )
        float lfDistance = length( lViewSpacePositionVec3 );
#else 
        float lfDistance = 0.0;
#endif // !defined( D_CACHE_COLOUR ) && !defined( D_CACHE_NORMAL ) && !defined( D_CACHE_MASK ))

        bool  lbIsDetail = false;

#ifndef D_ASTEROID

#if defined( D_READ_TEX_CACHE ) || defined( D_TESS_SHADERS_PRESENT )
        if ( true ) // force fading between the first two stages 'cause that's all we cache
#else
        if ( lfDistance < kafTextureDistances[1] )
#endif // defined( D_READ_TEX_CACHE ) || defined( D_TESS_SHADERS_PRESENT )
        {
            lbIsDetail = true;
            lfTextureSmallScaleFactor = kafTextureScales[0];
            lfTextureLargeScaleFactor = kafTextureScales[1];
            lfScaleFade  = lfDistance             - kafTextureDistances[0];
            lfScaleFade /= kafTextureDistances[1] - kafTextureDistances[0];
            lfScaleFade  = lfScaleFade * 2.0 - 0.5;

            #if defined ( D_VALIDATE_TEX_CACHE_BLOCKS )
            lfScaleFade = 0.0f;
            #endif
        }
        else if ( lfDistance < kafTextureDistances[2] )
        {
            lfTextureSmallScaleFactor = kafTextureScales[1];
            lfTextureLargeScaleFactor = kafTextureScales[2];
            lfScaleFade  = lfDistance             - kafTextureDistances[1];
            lfScaleFade /= kafTextureDistances[2] - kafTextureDistances[1];
        }
        else if ( lfDistance < kafTextureDistances[3] )
        {
            lfTextureSmallScaleFactor = kafTextureScales[2];
            lfTextureLargeScaleFactor = kafTextureScales[3];
            lfScaleFade  = lfDistance             - kafTextureDistances[2];
            lfScaleFade /= kafTextureDistances[3] - kafTextureDistances[2];
        }
        else if ( lfDistance < kafTextureDistances[4] )
        {
            lfTextureSmallScaleFactor = kafTextureScales[3];
            lfTextureLargeScaleFactor = kafTextureScales[4];
            lfScaleFade  = lfDistance             - kafTextureDistances[3];
            lfScaleFade /= kafTextureDistances[4] - kafTextureDistances[3];
        }
        else if ( lfDistance < kafTextureDistances[5] )
        {
            // The further texture distances use a sigmoid function to reduce the area that we're rendering both the large and small textures at the same time.
            lfTextureSmallScaleFactor = kafTextureScales[4];
            lfTextureLargeScaleFactor = kafTextureScales[5];
            lfScaleFade  = lfDistance             - kafTextureDistances[4];
            lfScaleFade /= kafTextureDistances[5] - kafTextureDistances[4];
            lfScaleFade  = SigmoidFunction( lfScaleFade, kfCrossing, kfPower );
        }
        else if ( lfDistance < kafTextureDistances[6] )
        {
            lfTextureSmallScaleFactor = kafTextureScales[5];
            lfTextureLargeScaleFactor = kafTextureScales[6];
            lfScaleFade  = lfDistance             - kafTextureDistances[5];
            lfScaleFade /= kafTextureDistances[6] - kafTextureDistances[5];
            lfScaleFade  = SigmoidFunction( lfScaleFade, kfCrossing, kfPower );
        }
        else if ( lfDistance < kafTextureDistances[7] )
        {
            lfTextureSmallScaleFactor = kafTextureScales[6];
            lfTextureLargeScaleFactor = kafTextureScales[7];
            lfScaleFade  = lfDistance             - kafTextureDistances[6];
            lfScaleFade /= kafTextureDistances[7] - kafTextureDistances[6];
            lfScaleFade  = SigmoidFunction( lfScaleFade, kfCrossing, kfPower );
        }
        else
        {
            lfTextureSmallScaleFactor = kafTextureScales[7];
            lfTextureLargeScaleFactor = kafTextureScales[8];
            lfScaleFade  = lfDistance             - kafTextureDistances[7];
            lfScaleFade /= kafTextureDistances[8] - kafTextureDistances[7];
            lfScaleFade  = SigmoidFunction( lfScaleFade, kfCrossing, kfPower );
        }

        //lbIsDetail = lfDistance < kafTextureDistances[ 1 ];
        //lfScaleFade = lbIsDetail ? lfScaleFade * 2.0 - 0.5 : lfScaleFade;
        //lfScaleFade = lfDistance >= kafTextureDistances[ 5 ] ? SigmoidFunction( lfScaleFade, kfCrossing, kfPower ) : lfScaleFade;
        

        // Check whether we can cut the above tests down... this block of code is about as fast, so not going to put it in unless I can find something to do with it.
        //float lfOffset = 0.520837 * pow( lfDistance, 0.2713285 );
        //float lfOffset = floor( 0.225 * pow( lfDistance, 0.395 ));
        //lfOffset = max( lfOffset, 0.0f );
        //lfOffset = min( lfOffset, 7.0f );
        //int liOffset = int( lfOffset );
        //int liOffsetNext = liOffset + 1;

        //lbIsDetail = liOffset == 0;
        //lfTextureSmallScaleFactor = kafTextureScales[ liOffset ];
        //lfTextureLargeScaleFactor = kafTextureScales[ liOffsetNext ];
        //lfScaleFade = lfDistance - kafTextureDistances[ liOffset ];
        //lfScaleFade /= kafTextureDistances[ liOffsetNext ] - kafTextureDistances[ liOffset ];
        //lfScaleFade = lbIsDetail ? lfScaleFade * 2.0 - 0.5 : lfScaleFade;
        //lfScaleFade = liOffset < 4 ? lfScaleFade : SigmoidFunction( lfScaleFade, kfCrossing, kfPower );


        // Push into the constant...
        //lfTextureSmallScaleFactor = 1.0 / lfTextureSmallScaleFactor;
        //lfTextureLargeScaleFactor = 1.0 / lfTextureLargeScaleFactor;

#else

        lfTextureSmallScaleFactor = 1.0 / 256.0;
        lfTextureLargeScaleFactor = 1.0 / 256.0;
        lfScaleFade = 0.0;

#endif // D_ASTEROID

#ifndef D_READ_TEX_CACHE

        uvec4   lTileTextureIndicesVec4 = uvec4( 0, 0, 0, 0 );

        {
#if defined ( D_TERRAIN_T_SPLIT )

            lTileTextureIndicesVec4.z = uint( IN( mLocalPositionVec4 ).w );
            lTileTextureIndicesVec4.z = uint( round( mod( float(lTileTextureIndicesVec4.z), float(D_TERRAINCOLOURARRAY_SIZE) ) ) );

#else
            // lTileVec2 sometimes comes in negative which crashes PS4. Should be fixed
            // on CPU wherever it gets passed in from.
            lTileTextureIndicesVec4.z = uint( round( mod( max( IN( mTileVec4 ).x, 0.0 ), float(D_TERRAINCOLOURARRAY_SIZE) ) ) );
            lTileTextureIndicesVec4.w = uint( round( mod( max( IN( mTileVec4 ).y, 0.0 ), float(D_TERRAINCOLOURARRAY_SIZE) ) ) );
#endif

            luSlopeIndex1 = min( lTileTextureIndicesVec4.z / 4, 1u );
            luSlopeIndex2 = min( lTileTextureIndicesVec4.w / 4, 1u );

            uint id = lTileTextureIndicesVec4.z;
            lTileTextureIndicesVec4.z |= PackItemYChannel( id + 2 ) | PackItemZChannel( id + 1 ) | PackItemWChannel( id + 3 );
            lTileTextureIndicesVec4.x = lTileTextureIndicesVec4.z;

            //if( lTileTextureIndicesLarge1Vec4.x >= 20 )
            if ( id >= 20 )
            {
                // Substances read from a second atlas, don't have slope textures
                uint x = UnpackItemXChannel( lTileTextureIndicesVec4.z );
                lTileTextureIndicesVec4.z = x | PackItemYChannel( x ) | PackItemZChannel( x ) | PackItemWChannel( x );
                lTileTextureIndicesVec4.x = lTileTextureIndicesVec4.z;
            }
            else if ( id >= 16 )
            {
                uint x = lTileTextureIndicesVec4.z & 0x7c1f;
                x = ( x << 5 ) | x;

                lTileTextureIndicesVec4.z = x;
                lTileTextureIndicesVec4.x = x;
            }
            else if ( lbIsDetail )
            {
                // The nearest textures read from detail textures in the second half of the atlas
                uint x = UnpackItemXChannel( lTileTextureIndicesVec4.x );
                uint y = UnpackItemYChannel( lTileTextureIndicesVec4.x );
                uint z = UnpackItemZChannel( lTileTextureIndicesVec4.x );
                uint w = UnpackItemWChannel( lTileTextureIndicesVec4.x );
                lTileTextureIndicesVec4.x = PackItemXChannel( x + 8 ) | PackItemYChannel( y + 8 ) | PackItemZChannel( z + 8 ) | PackItemWChannel( w + 8 );
            }

#if !defined ( D_TERRAIN_T_SPLIT )

            id = lTileTextureIndicesVec4.w;
            lTileTextureIndicesVec4.w |= PackItemYChannel( id + 2 ) | PackItemZChannel( id + 1 ) | PackItemWChannel( id + 3 );
            lTileTextureIndicesVec4.y = lTileTextureIndicesVec4.w;

            if ( id >= 20 )
            {
                uint x = UnpackItemXChannel( lTileTextureIndicesVec4.w );
                lTileTextureIndicesVec4.w = x | PackItemYChannel( x ) | PackItemZChannel( x ) | PackItemWChannel( x );
                lTileTextureIndicesVec4.y = lTileTextureIndicesVec4.w;
            }
            else if ( id >= 16 )
            {
                uint x = lTileTextureIndicesVec4.w & 0x7c1f;
                x = ( x << 5 ) | x;

                lTileTextureIndicesVec4.w = x;
                lTileTextureIndicesVec4.y = x;
            }
            else if ( lbIsDetail )
            {
                uint x = UnpackItemXChannel( lTileTextureIndicesVec4.y );
                uint y = UnpackItemYChannel( lTileTextureIndicesVec4.y );
                uint z = UnpackItemZChannel( lTileTextureIndicesVec4.y );
                uint w = UnpackItemWChannel( lTileTextureIndicesVec4.y );
                lTileTextureIndicesVec4.y = PackItemXChannel( x + 8 ) | PackItemYChannel( y + 8 ) | PackItemZChannel( z + 8 ) | PackItemWChannel( w + 8 );
            }

#endif
        }

#if defined ( D_FORCE_HALF )
    #if !defined ( D_TERRAIN_T_SPLIT )
        // Tile blend
        half lfTile = IN( mTileVec4 ).z;
    #else
        // Tile blend
        half lfTile = 1.0;
    #endif
#else
    #if !defined ( D_TERRAIN_T_SPLIT )
        // Tile blend
        float lfTile = IN( mTileVec4 ).z;
    #else
        // Tile blend
        float lfTile = 1.0;
    #endif
#endif     

#if defined ( D_FORCE_HALF )
        half lfPatch;
#else
        // Using BC1 noise texture.
        float lfPatch;
#endif
        {
            float lfNoiseSample = texture3DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gPerlin3D ), lPlanetSpacePositionVec3 * lUniforms.mpCustomPerMaterial.gTileBlendScalesVec4.x, 0.0 ).g;      
#if defined ( D_FORCE_HALF )
            half lfPatch1       = saturate( lfNoiseSample ) * 2.0 - 1.0;
#else
            float lfPatch1      = saturate( lfNoiseSample ) * 2.0 - 1.0;
#endif

            lfNoiseSample       = texture3DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gPerlin3D ), lPlanetSpacePositionVec3 * lUniforms.mpCustomPerMaterial.gTileBlendScalesVec4.y, 0.0 ).g;
#if defined ( D_FORCE_HALF )
            half lfPatch2 = saturate( lfNoiseSample ) * 2.0 - 1.0;
#else
            float lfPatch2      = saturate( lfNoiseSample ) * 2.0 - 1.0;
#endif

            lfPatch             = lfPatch1 + lfPatch2 * lUniforms.mpCustomPerMaterial.gTileBlendScalesVec4.z;
            lfPatch             = 1.0 - saturate( lfPatch * 0.5 + 0.5 + lUniforms.mpCustomPerMaterial.gTileBlendScalesVec4.w );
        }

        // Blend to different texture on the slope
        //Skew the flat towards the top slightly, to show more slope [-1 : 1] . [-0.8 : 1] . Clamped to 0

#if defined ( D_FORCE_HALF )
        half lfSlope1 = 0.0;
        half lfSlope2 = 0.0; // Optimised out if not needed.
#else
        float lfSlope1 = 0.0;
#if !defined( D_TERRAIN_T_SPLIT )
        float lfSlope2 = 0.0;
#endif
#endif

        {
#if defined ( D_FORCE_HALF )
            half lfSlope;
#else
            float lfSlope;
#endif
            lfSlope = max( dot( normalize( lPlanetSpacePositionVec3 ), lWorldSpaceNormalVec3 ), 0.0 );

#if defined ( D_FORCE_HALF )
            lfSlope = AsinPolynomialApproximationHalf( lfSlope ) / ( 3.1415 * 0.5 );
#else
            lfSlope = AsinPolynomialApproximation( lfSlope ) / ( 3.1415 * 0.5 );
#endif

            //lfSlope1 = pow( lfSlope, lUniforms.mpCustomPerMaterial.gTerrainSlopeParamsVec4[luSlopeIndex1] );              
            //lfSlope1 = saturate( lfSlope1 );
            lfSlope1 = lfSlope;

            if ( UnpackItemXChannel( lTileTextureIndicesVec4.z ) >= 16 )
            {
                lfSlope1 = lfSlope * lfSlope;
            }

#if !defined( D_TERRAIN_T_SPLIT )
            //lfSlope2 = pow( lfSlope, lUniforms.mpCustomPerMaterial.gTerrainSlopeParamsVec4[luSlopeIndex2] );                
            //lfSlope2 = saturate( lfSlope2 );

            lfSlope2 = lfSlope;

            if ( UnpackItemXChannel( lTileTextureIndicesVec4.w ) >= 16 )
            {
                lfSlope2 = lfSlope * lfSlope;
            }
#endif
        }

#if defined ( D_STOCHASTIC_TERRAIN ) && defined ( D_TERRAIN_LOD ) && !defined( D_PLATFORM_SWITCH )
        lfPatch = SigmoidFunction( lfPatch, 0.5, kfPower );
#endif        

#if !defined( D_CACHE_COLOUR ) && !defined( D_CACHE_NORMAL ) && !defined( D_CACHE_MASK )
        {
            float lfCameraHeight = lUniforms.mpCustomPerMaterial.gTerrainDistancesVec4.x;
            lfCameraHeight = 1.0 - saturate( ( lfCameraHeight - 2000.0 ) / 2000.0 );

            lfPatch *= lfCameraHeight;
            lfSlope1 = mix( 1.0, lfSlope1, lfCameraHeight );
#if !defined( D_TERRAIN_T_SPLIT )
            lfSlope2 = mix( 1.0, lfSlope2, lfCameraHeight );
#endif
        }
#endif

#if defined ( D_TERRAIN_HEIGHT_LOD )
        lfPatch = 0.0;
        lfSlope1 = 1.0;
#if !defined( D_TERRAIN_T_SPLIT )
        lfSlope2 = 1.0;
#endif
#endif

#ifdef D_ASTEROID
        lfPatch = 0.0;
        lTileTextureIndicesVec4 = uvec4( 0, 0, 0, 0 );
#endif

#ifdef D_CACHE_FAR
        lfScaleFade = 1.0f;
#endif
               

        {        
            #if defined ( D_FORCE_HALF )
            const half kfOffset = 0.2;
            const half kfMidpoint = 0.5;
            half lfBlendDist;

            lfBlendDist = saturate( lfDistance - kafTextureDistances[ 1 ] ) / ( kafTextureDistances[ 3 ] - kafTextureDistances[ 1 ] );
            half2 lvBlendDist = half2( lfBlendDist, lfBlendDist );

            const half2 kvStart = half2( kfMidpoint - kfOffset, kfMidpoint - kfOffset );
            const half2 kvWidth = half2( 2.0 * kfOffset, 2.0 * kfOffset );

            half2 lvSlopes = half2( lfSlope1, lfSlope2 );
            half2 lvNearSlope = FastSmoothStepHalf( 
                kvStart,
                kvWidth,
                lvSlopes );
            lvSlopes = mix( lvNearSlope, lvSlopes, lvBlendDist );
            
            lfSlope1 = lvSlopes.x;
            lfSlope2 = lvSlopes.y;
            
            FRAGMENT_COLOUR0 = vec4( lfSlope1, lfSlope2, 0.0, 1.0 );
            #else
            float lfOffset = 0.2;
            float lfMidpoint = 0.5;
            float lfBlendDist;
            float lfNearSlope1;

            lfBlendDist = ( lfDistance - kafTextureDistances[ 1 ] ) / ( kafTextureDistances[ 3 ] - kafTextureDistances[ 1 ] );

            lfNearSlope1 = FastSmoothStep( lfMidpoint - lfOffset, 2.0 * lfOffset, lfSlope1 );
            lfSlope1 = mix( lfNearSlope1, lfSlope1, saturate( lfBlendDist ) );
            lfSlope1 = saturate( lfSlope1 );

            #if !defined( D_TERRAIN_T_SPLIT )
                float lfNearSlope2;

                lfNearSlope2 = FastSmoothStep( lfMidpoint - lfOffset, 2.0 * lfOffset, lfSlope2 );
                lfSlope2 = mix( lfNearSlope2, lfSlope2, saturate( lfBlendDist ) );
                lfSlope2 = saturate( lfSlope2 );
            #endif
            #endif // defined ( D_FORCE_HALF )
        }

#if defined (D_STOCHASTIC_TERRAIN)

        lFragmentColourVec3 = GetStochasticTileColourAndNormal(
            DEREF_PTR( lUniforms.mpCustomPerMaterial ),            
            lWorldSpaceNormalVec3,
            lPlanetSpaceOffsetVec3,
            lLocalSpacePositionVec3,
            lTileTextureIndicesVec4,
            lfPatch,
            lfSlope1,
#if !defined( D_TERRAIN_T_SPLIT )
            lfSlope2,
#endif
            lfTile,
            lNormalVec3,
            lfTextureSmallScaleFactor,
            lfTextureLargeScaleFactor,
            lfScaleFade,
            1.0 - lfWaterFade,
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gDiffuseMap ),
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gNormalMap ),
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gSubstanceDiffuseMap ),
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gSubstanceNormalMap ),
#if !defined( D_SIMPLIFIED_NOISE )
            SAMPLER2DPARAM_SRT(      lUniforms.mpCustomPerMaterial, gValueNoiseNorms2D ),
#else
            SAMPLER2DPARAM_SRT(      lUniforms.mpCustomPerMaterial, gValueNoiseNorms2D_VR ),
#endif
            lfSpecular,
            lfMetallic,
            lfHeight );

#else

        lFragmentColourVec3 = GetTileColourAndNormal(
            DEREF_PTR( lUniforms.mpCustomPerMaterial ),
            lWorldSpaceNormalVec3,
            lPlanetSpaceOffsetVec3,
            lLocalSpacePositionVec3,
            lTileTextureIndicesVec4,
            lfPatch,
            lfSlope1,
            lfSlope2,
            lfTile,
            lNormalVec3,
            lfTextureSmallScaleFactor,
            lfTextureLargeScaleFactor,
            lfScaleFade,
            1.0 - lfWaterFade,
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gDiffuseMap ),
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gNormalMap ),
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gSubstanceDiffuseMap ),
            SAMPLER2DARRAYPARAM_SRT( lUniforms.mpCustomPerMaterial, gSubstanceNormalMap ),
#if !defined( D_SIMPLIFIED_NOISE )
            SAMPLER2DPARAM_SRT(      lUniforms.mpCustomPerMaterial, gValueNoiseNorms2D ),
#else
            SAMPLER2DPARAM_SRT(      lUniforms.mpCustomPerMaterial, gValueNoiseNorms2D_VR ),
#endif
            lfSpecular,
            lfMetallic,
            lfHeight );

#endif // D_STOCHASTIC_TERRAIN

//#if defined ( D_TERRAIN_T_SPLIT )
//        lFragmentColourVec3 = vec3( 1.0, 0.0, 0.0 );
//#endif

#endif  // D_READ_TEX_CACHE

#ifdef D_READ_TEX_CACHE
        lNormalVec3 = lWorldSpaceNormalVec3;
#endif

#if defined( D_ASTEROID )	
        mat3 lNormalisedMat3;
        MAT3_SET_COLUMN( lNormalisedMat3, 0, normalize( lUniforms.mpCommonPerMesh.gWorldMat4[0].xyz ) );
        MAT3_SET_COLUMN( lNormalisedMat3, 1, normalize( lUniforms.mpCommonPerMesh.gWorldMat4[1].xyz ) );
        MAT3_SET_COLUMN( lNormalisedMat3, 2, normalize( lUniforms.mpCommonPerMesh.gWorldMat4[2].xyz ) );

        lNormalVec3 = normalize( MUL( lNormalisedMat3, lNormalVec3 ) );
#endif

    }

    float lfRoughness = lfSpecular * lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.x;



#if defined( D_READ_TEX_CACHE ) || defined( D_WRITE_CACHE_FEEDBACK )
    {
        ivec2 lResolution = ivec2( 32, 32 );

        vec2 lCacheCoords01 = IN( mTexCoordVec2_mTexBorderVec2 ).xy;

#ifndef D_PLATFORM_OPENGL
        lCacheCoords01.y = 1.0 - lCacheCoords01.y;
#endif

        vec2 lfDerivX = dFdx( lCacheCoords01.xy );
        vec2 lfDerivY = dFdy( lCacheCoords01.xy );

        vec4 lSparseStatus = lUniforms.mpCustomPerMesh.gSparseTextureStatusVec4;
        float lfMipBias = lSparseStatus[2];
        float lfAniso = lSparseStatus[3];

        float Px = sqrt( dot( lfDerivX, lfDerivX ) ) * float( lResolution.x ) * 256.0;
        float Py = sqrt( dot( lfDerivY, lfDerivY ) ) * float( lResolution.y ) * 256.0;

        float Pmax = max( Px, Py );
        float Pmin = min( Px, Py );

        float N = min( ceil( Pmax / Pmin ), lfAniso );

        float lfBaseMip = log2( Pmax / N );
        lfBaseMip -= min( 1.0, lfBaseMip * 0.25 ); // compensate for lost resolution in high mips due to gutters

        float lfDesiredMip = max( 0.0, min( 5.0, lfBaseMip + lfMipBias ) );

        uint liWriteSparse0 = uint( lSparseStatus[0] );
        float lfFadeThreshold = 1.0 / 20.0;

        if ( lfScaleFade < lfFadeThreshold )
        {
            lfScaleFade = 0.0;
        }

        if ( lfScaleFade > 1.0 - lfFadeThreshold )
        {
            lfScaleFade = 1.0;
        }


        // FEEDBACK WRITES
        // calling a function with a RwImage param is a horror show so do it here

#ifdef D_WRITE_CACHE_FEEDBACK
#if !defined( D_PLATFORM_XBOXGDK )
        if ( WaveIsFirstLane() )
#else
        if( __XB_GetLaneID() == __XB_S_FF1_U64( __XB_GetEntryActiveMask64() ) )
#endif
        {
            ivec2 lStatusCoords = ivec2( floor( vec2( lResolution.xy ) * lCacheCoords01 ) );
            int liHighDetailMip = int( floor( max( 0.0, lfDesiredMip - 0.5 ) ) );
            int lFbEnd = lResolution.x * lResolution.y * 2;

#ifdef D_PLATFORM_GLSL
            ivec4 lStoreVal = ivec4( 1, 0, 0, 0 );
#else
            int lStoreVal = 1;
#endif

            if ( lfScaleFade <= 1.0 - lfFadeThreshold )
            {
                uint liFeedbackIndex = liWriteSparse0 & 0xff;

                if ( liFeedbackIndex != 255 )
                {
                    ivec2 lFeedbackSample = lStatusCoords;
//TF_BEGIN
#if defined(D_PLATFORM_METAL)    
                    int indexFeedback = int(lFbEnd - 1 + liFeedbackIndex * UNIFORM(lUniforms.mpCustomPerMesh, gFeedbackMapTextureSize).x);
                    lUniforms.mpCustomPerMesh.gFeedbackMap[indexFeedback] = lStoreVal;
#else //TF_END
                    imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gFeedbackMap ), ivec2( lFbEnd - 1, liFeedbackIndex ), lStoreVal );
#endif
                    if ( liHighDetailMip < 5 )
                    {
                        lFeedbackSample.xy >>= liHighDetailMip;

                        lResolution = ivec2( 32, 32 );
                        lResolution.xy >>= liHighDetailMip;
                        int lMipStart = lFbEnd - lResolution.x * lResolution.y * 2;
//TF_BEGIN
#if defined(D_PLATFORM_METAL)    
                        indexFeedback = int((lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x) + liFeedbackIndex * UNIFORM(lUniforms.mpCustomPerMesh, gFeedbackMapTextureSize).x);
                        lUniforms.mpCustomPerMesh.gFeedbackMap[indexFeedback] = lStoreVal;
#else //TF_END
                        imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gFeedbackMap ),
                            ivec2( lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x, liFeedbackIndex ),
                            lStoreVal );
#endif

                        lFeedbackSample.xy /= 2;
                        lResolution.xy /= 2;

                        lMipStart = lFbEnd - lResolution.x * lResolution.y * 2;
//TF_BEGIN
#if defined(D_PLATFORM_METAL)    
                        indexFeedback = int((lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x) + liFeedbackIndex * UNIFORM(lUniforms.mpCustomPerMesh, gFeedbackMapTextureSize).x);
                        lUniforms.mpCustomPerMesh.gFeedbackMap[indexFeedback] = lStoreVal;
#else //TF_END
                        imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gFeedbackMap ),
                            ivec2( lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x, liFeedbackIndex ),
                            lStoreVal );
#endif
                    }
                }
            }

            if ( lfScaleFade >= lfFadeThreshold )
            {
                uint liFeedbackIndex = ( liWriteSparse0 >> 8 ) & 0xff;

                if ( liFeedbackIndex != 255 )
                {
                    ivec2 lFeedbackSample = lStatusCoords;
//TF_BEGIN
#if defined(D_PLATFORM_METAL)    
                    int indexFeedback = int(lFbEnd - 1 + liFeedbackIndex * UNIFORM(lUniforms.mpCustomPerMesh, gFeedbackMapTextureSize).x);
                    lUniforms.mpCustomPerMesh.gFeedbackMap[indexFeedback] = lStoreVal;
#else //TF_END
                    imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gFeedbackMap ), ivec2( lFbEnd - 1, liFeedbackIndex ), lStoreVal );
#endif

                    if ( liHighDetailMip < 5 )
                    {
                        lFeedbackSample.xy >>= liHighDetailMip;

                        lResolution = ivec2( 32, 32 );
                        lResolution.xy >>= liHighDetailMip;
                        int lMipStart = lFbEnd - lResolution.x * lResolution.y * 2;
//TF_BEGIN
#if defined(D_PLATFORM_METAL)    
                        indexFeedback = int((lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x) + liFeedbackIndex * UNIFORM(lUniforms.mpCustomPerMesh, gFeedbackMapTextureSize).x);
                        lUniforms.mpCustomPerMesh.gFeedbackMap[indexFeedback] = lStoreVal;
#else //TF_END
                        imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gFeedbackMap ),
                            ivec2( lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x, liFeedbackIndex ),
                            lStoreVal );
#endif

                        lFeedbackSample.xy /= 2;
                        lResolution.xy /= 2;

                        lMipStart = lFbEnd - lResolution.x * lResolution.y * 2;
//TF_BEGIN
#if defined(D_PLATFORM_METAL)    
                        indexFeedback = int((lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x) + liFeedbackIndex * UNIFORM(lUniforms.mpCustomPerMesh, gFeedbackMapTextureSize).x);
                        lUniforms.mpCustomPerMesh.gFeedbackMap[indexFeedback] = lStoreVal;
#else //TF_END
                        imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMesh, gFeedbackMap ),
                            ivec2( lMipStart + lFeedbackSample.y * lResolution.x + lFeedbackSample.x, liFeedbackIndex ),
                            lStoreVal );
#endif
                    }
                }
            }
        }
#endif // D_WRITE_CACHE_FEEDBACK


        vec3 lFragmentColourNearVec3 = vec3( 0.0, 0.0, 0.0 );
        vec4 lMasksNearVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );
        vec3 lNormalNearVec3 = vec3( 0.0, 0.0, 0.0 );

        vec3 lFragmentColourFarVec3 = vec3( 0.0, 0.0, 0.0 );
        vec4 lMasksFarVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );
        vec3 lNormalFarVec3 = vec3( 0.0, 0.0, 0.0 );

        float lfNearHeight;
        float lfFarHeight;

#ifdef D_READ_TEX_CACHE
#if defined( D_TESS_SHADERS_PRESENT ) || defined( D_PLATFORM_PC )
        vec2 lvBorder = vec2( IN( mTexCoordVec2_mTexBorderVec2 ).zw );
#ifndef D_PLATFORM_OPENGL
        lvBorder.y = -lvBorder.y;
#endif
#else
        vec2 lvBorder = vec2( 0, 0 );
#endif // defined( D_TESS_SHADERS_PRESENT ) || defined( D_PLATFORM_PC )

        if ( lfScaleFade < 1.0 - lfFadeThreshold )
        {
            ReadTextureCache(
                lCacheCoords01,
                lvBorder,
                lfDerivX,
                lfDerivY,
                lResolution,
                lfDesiredMip,
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseNearDiffMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseNearNormMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseNearStatusMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseNearStatusMap ),
                lFragmentColourNearVec3,
                lNormalNearVec3,
                lMasksNearVec4,
                lfNearHeight );
        }

        if ( lfScaleFade > lfFadeThreshold )
        {
            ReadTextureCache(
                lCacheCoords01,
                lvBorder,
                lfDerivX,
                lfDerivY,
                lResolution,
                lfDesiredMip,
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseFarDiffMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseFarNormMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseFarStatusMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gSparseFarStatusMap ),
                lFragmentColourFarVec3,
                lNormalFarVec3,
                lMasksFarVec4,
                lfFarHeight );
        }

        lFragmentColourVec3 = mix( lFragmentColourNearVec3, lFragmentColourFarVec3, lfScaleFade );
        lFragmentColourVec3 = GammaCorrectInput01( lFragmentColourVec3 );

        vec4 lMasksVec4 = mix( lMasksNearVec4, lMasksFarVec4, lfScaleFade );

#if !defined( D_TESS_SHADERS_PRESENT ) && !defined( D_PLATFORM_OPENGL )
        // can't get the texcoord derivative from the hull shader, must compute it ourselves

#if 0 // def GL_AMD_shader_explicit_vertex_parameter

// on AMD can use the simple subtract from the hull shader
        vec3 dPdU;

        if ( ( interpolateAtVertexAMD( mTexCoordVec2_mfDistForFade, 2 ).x - interpolateAtVertexAMD( mTexCoordVec2_mfDistForFade, 1 ).x ) == 0 )
        {
            dPdU = interpolateAtVertexAMD( mLocalPositionVec4, 2 ).xyz - interpolateAtVertexAMD( mLocalPositionVec4, 0 ).xyz;
            dPdU *= sign( interpolateAtVertexAMD( mTexCoordVec2_mfDistForFade, 2 ).x - interpolateAtVertexAMD( mTexCoordVec2_mfDistForFade, 0 ).x );
        }
        else
        {
            dPdU = interpolateAtVertexAMD( mLocalPositionVec4, 2 ).xyz - interpolateAtVertexAMD( mLocalPositionVec4, 1 ).xyz;
            dPdU *= sign( interpolateAtVertexAMD( mTexCoordVec2_mfDistForFade, 2 ).x - interpolateAtVertexAMD( mTexCoordVec2_mfDistForFade, 1 ).x );
        }
        dPdU = normalize( dPdU );

#else

// don't have the AMD feature, will nead to do some chain-rule acrobatics

        vec3 dPdX = dFdxFine( lWorldSpacePositionVec3 );
        vec3 dPdY = dFdyFine( lWorldSpacePositionVec3 );

        float dUdX = dFdxFine( lCacheCoords01.x );
        float dUdY = dFdyFine( lCacheCoords01.x );
        float dVdX = dFdxFine( lCacheCoords01.y );
        float dVdY = dFdyFine( lCacheCoords01.y );

        mat2 derivs;
        derivs[0][0] = dUdX;
        derivs[0][1] = dUdY;
        derivs[1][0] = dVdX;
        derivs[1][1] = dVdY;

        derivs = Inverse( derivs );

        float dXdU = derivs[0][0];
        float dYdU = derivs[1][0];

        float dXdV = derivs[0][1];
        float dYdV = derivs[1][1];

        vec3 dPdU = dPdX * dXdU + dPdY * dYdU;
        dPdU = normalize( dPdU );

#endif // #if 0 
        vec3 dPdV = normalize( cross( lNormalVec3, dPdU ) );

#else
        vec3 dPdU = IN( mTexCoordsDPDUVec4 ).xyz;
        vec3 dPdV = IN( mTexCoordsDPDVVec4 ).xyz;

#endif // !defined( D_TESS_SHADERS_PRESENT ) && !defined( D_PLATFORM_OPENGL )

        lNormalNearVec3 *= ( 1.0 - lfScaleFade );
        lNormalFarVec3 *= lfScaleFade;

        lNormalNearVec3 = vec3( lNormalNearVec3.x + lNormalFarVec3.x, lNormalNearVec3.y + lNormalFarVec3.y, lNormalNearVec3.z + lNormalFarVec3.z );

        lNormalNearVec3 = lNormalNearVec3.x * dPdU + lNormalNearVec3.y * dPdV;
        lNormalVec3 = normalize( lNormalVec3 + lNormalNearVec3 );

        lfMetallic = lMasksVec4.x;
        lfRoughness = lMasksVec4.y;
        lfSubsurface = 0.0; // lMasksVec4.z;
        lfGlow = 0.0; // lMasksVec4.w;
#endif // D_READ_TEX_CACHE
    }
#endif // defined( D_READ_TEX_CACHE ) || defined( D_WRITE_CACHE_FEEDBACK )

    // vec2 lCenter = GetViVjPerspCenter();
    // float lfW = 1.0 - lCenter.x - lCenter.y;
    // if( lCenter.x < (lUniforms.mpPerFrame.gTessSettingsVec4.w) || lCenter.y < (lUniforms.mpPerFrame.gTessSettingsVec4.w) || lfW < (lUniforms.mpPerFrame.gTessSettingsVec4.w) )
    // {
    //     lFragmentColourVec3 = vec3( 1 );
    // }

        //Terrain edits - do after the lighting
#ifdef D_TERRAIN_EDITS
    float lfEditBlend = IN(mTileVec4).w - 0.0001;  //Take off the bias?
#ifdef D_READ_TEX_CACHE
    //Force bind the diffuse map texture, which isn't used here, but is used by tesselation, by referencing it, but in a way that we will never actually read it
    if (lfEditBlend > 100.0)
    {
        //This case should never happen! This is just to force the gSparseNearDiffMap to be bound!
        vec4 lNoisyRubbish = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gSparseNearDiffMap), IN_SCREEN_POSITION.xy);
        lfEditBlend += lNoisyRubbish.x;
    }
#endif
    lfEditBlend = saturate(lfEditBlend);
    if (lfEditBlend > 0.0)
    {
        lfEditBlend *= lUniforms.mpCustomPerMaterial.gTerrainEditEffectsVec4.w;
        //Fade off with distance
        float lfDepth = ReverseZToLinearDepth(lUniforms.mpPerFrame.gClipPlanesVec4, IN_SCREEN_POSITION.z);
        lfEditBlend *= saturate(3.0 - (lfDepth * 0.04));

        //Do some screen-space effect
        vec3 lUp = lPlanetSpacePositionVec3;
        float lfUpDist = length(lUp);
        lfUpDist -= lUniforms.mpCustomPerMaterial.gTerrainEditEffectsVec4.x;

        vec3 lDrift = 1.5 * vec3(0.37654, 0.2245, 0.12356) * lUniforms.mpCustomPerMaterial.gTerrainEditEffectsVec4.x;

        //Calculate a bit of noise, to make the line wobbly
        vec3 position = lWorldSpacePositionVec3 + lDrift;
        position *= 0.9;
        vec3 posFloor = vec3(floor(position.x), floor(position.y), floor(position.z));

        float lfFloorWeight = posFloor.x + posFloor.y + posFloor.z;
        lfFloorWeight = mod(lfFloorWeight, 2.0);  //0 or 1
        float lfAltWeight = 1.0 - lfFloorWeight;  //1 or 0

        //Calc lerp factors...
        vec3 lDiff = position - posFloor;
        vec3 lAltDiff = vec3(1.0, 1.0, 1.0) - lDiff;
        //Bottom
        float bottom1 = (lDiff.x * lfAltWeight) + (lAltDiff.x * lfFloorWeight);
        float bottom2 = (lDiff.x * lfFloorWeight) + (lAltDiff.x * lfAltWeight);
        float bottom = (lDiff.z * bottom2) + (lAltDiff.z * bottom1);
        //Top
        //float top1 = bottom2;// (lDiff.x * lfFloorWeight) + (lAltDiff.x * lfAltWeight);
        //float top2 = bottom1;// (lDiff.x * lfAltWeight) + (lAltDiff.x * lfFloorWeight);
        float top = (lDiff.z * bottom1) + (lAltDiff.z * bottom2);
        //Total
        float total = (lDiff.y * bottom) + (lAltDiff.y * top);  //Quick perlin-esque random number generator

        lfUpDist += total * 0.25;

        float lfEffect = sin(lfUpDist * 6.0) - 0.8;
        lfEffect = saturate(max(lfEffect * 5.0, 0.0));
        lfEffect *= lfEffect * 0.7;

        float lfEffect2 = sin(3.0 + (lfUpDist * 18.0)) - 0.8;
        lfEffect2 = saturate(max(lfEffect2 * 5.0, 0.0));
        lfEffect2 *= lfEffect2 * 0.3;

        float lfFlatFade = saturate(dot(lNormalVec3, normalize(lUp)));
        lfFlatFade *= lfFlatFade * lfFlatFade;  //Fade out the lines when the surface is really flat, to stop a big pulse effect
        lfEffect *= 1.0 - lfFlatFade;
        lfEffect2 *= 1.0 - lfFlatFade;

        vec3 lBaseColour = vec3(2.5, 0.6, 0.0);
        float lfFresnel = (1.0 - saturate(dot(lNormalVec3, normalize(lUniforms.mpPerFrame.gViewPositionVec3 - lWorldSpacePositionVec3))));
        lBaseColour *= lfFresnel * lfFresnel;

        //lFragmentColourVec3 = mix(lFragmentColourVec3, lBaseColour, lfEditBlend);
        lFragmentColourVec3 = lBaseColour * lfEditBlend;

        //Add on the lines over the top
        //vec3 lLinesColour = 3.0 * vec3(0.7, 0.8, 2.0);
        vec3 lLinesColour = vec3(1.0, 0.4, 0.26);
        //lFragmentColourVec3 += lLinesColour * lfEffect * lfEditBlend;
        lFragmentColourVec3 = mix(lFragmentColourVec3, lLinesColour, lfEffect * lfEditBlend);

        //lFragmentColourVec3 += lLinesColour * lfEffect2 * lfEditBlend;
        lFragmentColourVec3 = mix(lFragmentColourVec3, lLinesColour, lfEffect2 * lfEditBlend);
    }
    else
    {
        //discard;  //We promised we wouldn't discard apparently
        lFragmentColourVec3 = vec3(0.0, 0.0, 0.0);
    }
#endif

    //-----------------------------------------------------------------------------
    ///
    ///     Distant Water Colouring
    ///
    //-----------------------------------------------------------------------------
#if !defined( D_CACHE_COLOUR ) && !defined( D_CACHE_NORMAL ) && !defined( D_CACHE_MASK )

#if !defined ( D_ASTEROID )

#if !defined ( D_STOCHASTIC_TERRAIN ) || ( defined( D_STOCHASTIC_TERRAIN ) && defined ( D_TERRAIN_T_SPLIT ) )

    // This recolours the terrain under a water layer.  We can live without the normal flattening for the stochastic terrain path, which blends across more than one tile.    
    
    if ( lfWaterFade > 0.0 )
    {
        lFragmentColourVec3  = mix( lFragmentColourVec3,    lUniforms.mpCustomPerMaterial.gWaterFogColourFarVec4.xyz, lfWaterFade );
        lfRoughness          = mix( lfRoughness,     0.35 * lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.x,      lfWaterFade );
    }

#endif

#endif

#if defined( D_REFLECT_DUALP ) || defined( D_REFLECT_WATER ) || defined( D_REFLECT_WATER_UP )
    {
        lFragmentColourVec3 *= ( lUniforms.mpCommonPerMesh.gLightColourVec4.xyz );
    }
#endif

#endif // !defined( D_CACHE_COLOUR ) && !defined( D_CACHE_NORMAL ) && !defined( D_CACHE_MASK )

    //-----------------------------------------------------------------------------
    ///
    ///		Output
    ///
    //-----------------------------------------------------------------------------

    {
        int liMaterialID = 0; //D_DETAILOVERLAY;

#ifdef _F10_NORECEIVESHADOW
        liMaterialID |= D_NORECEIVESHADOW;
#else

#endif // _F10_NORECEIVESHADOW


#ifdef D_NO_WATER
        liMaterialID |= D_NOWATER;
#endif

        /*
                if( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 0.0 )
                {
                    lFragmentColourVec3 = vec3( 1.0, 0.0, 0.0 );
                }
                else if( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 1.0 )
                {
                    lFragmentColourVec3 = vec3( 1.0, 1.0, 0.0 );
                }
                else if( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 2.0 )
                {
                    lFragmentColourVec3 = vec3( 0.0, 1.0, 0.0 );
                }
                else if( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 3.0 )
                {
                    lFragmentColourVec3 = vec3( 0.0, 1.0, 1.0 );
                }
                else if( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 4.0 )
                {
                    lFragmentColourVec3 = vec3( 0.0, 0.0, 1.0 );
                }
                else if( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 5.0 )
                {
                    lFragmentColourVec3 = vec3( 1.0, 0.0, 1.0 );
                }
                else
                {
                    lFragmentColourVec3 = vec3( 1.0, 1.0, 1.0 );
                }
        */

        vec4 lOutColours0Vec4 = vec4(0, 0, 0, 0);
        vec4 lOutColours1Vec4 = vec4(0, 0, 0, 0);
        vec4 lOutColours2Vec4 = vec4(0, 0, 0, 0);
        vec4 lOutColours3Vec4 = vec4(0, 0, 0, 0);
        vec4 lOutColours4Vec4 = vec4(0, 0, 0, 0);



#if defined( D_CACHE_COLOUR )

#ifdef D_CACHE_HEIGHT
        lOutColours0Vec4 = vec4( 0.0, 0.0, 0.0, lfHeight );
#else
        lOutColours0Vec4 = vec4( lFragmentColourVec3, 0.0 );
#endif

#elif  defined( D_CACHE_NORMAL )

#if !defined( D_TESS_SHADERS_PRESENT ) && !defined( D_PLATFORM_OPENGL )
        float dPdUInvLen = 1.0;
        float dPdVInvLen = 1.0;
        vec3 dPdU = normalize( dFdx( lWorldSpacePositionVec3 ) );
        vec3 dPdV = normalize( cross( lWorldSpaceNormalVec3, dPdU ) );
#else
        vec3 dPdU = IN( mTexCoordsDPDUVec4 ).xyz;
        vec3 dPdV = IN( mTexCoordsDPDVVec4 ).xyz;
        float dPdUInvLen = invsqrt( dot( dPdU, dPdU ) );
        float dPdVInvLen = invsqrt( dot( dPdV, dPdV ) );
        dPdU *= dPdUInvLen;
        dPdV *= dPdVInvLen;
#endif

        vec2 lEncDerivVec2 = vec2( dot( lNormalVec3, dPdU ), dot( lNormalVec3, dPdV ) );
        lEncDerivVec2.x *= dPdUInvLen;
        lEncDerivVec2.y *= dPdVInvLen;
        //lEncDerivVec2 *= length( IN( mSmoothNormalVec3 ).xyz );  // this removes a normalise() from the fragment side
        lEncDerivVec2 = sign( lEncDerivVec2 ) * sqrt( abs( lEncDerivVec2 ) * 0.25 );
        lEncDerivVec2 = lEncDerivVec2 * 0.5 + vec2( 0.5, 0.5 );

        lOutColours0Vec4.r = lEncDerivVec2.x;
        lOutColours0Vec4.g = lfMetallic;
        lOutColours0Vec4.b = lfRoughness;
        lOutColours0Vec4.a = lEncDerivVec2.y;

#elif  defined( D_CACHE_MASK )
        lOutColours0Vec4 = vec4( lfMetallic, lfRoughness, lfSubsurface, lfGlow );
#else

#ifndef D_READ_TEX_CACHE
        //lFragmentColourVec3 = vec3( lfNoise, lfNoise, lfNoise );
#endif

//TF_BEGIN

//TF_BEGIN
#ifdef D_TILED_LIGHTS
#ifndef D_PLATFORM_METAL
	int laLightIndices[D_TILE_MAX_LIGHT_COUNT];
#endif
    const ivec2 liThreadGlobalID = ivec2(IN_SCREEN_POSITION.xy - vec2(0.5, 0.5));
	const ivec2 liTileGlobalID = ivec2(liThreadGlobalID.x / D_TILE_WIDTH, liThreadGlobalID.y / D_TILE_HEIGHT);    
	//size of each tile in memory
	const int sizeOfTile = (D_TILE_WIDTH * D_TILE_HEIGHT+ 1);
	//num of tiles per row in image (without account for num of lights).
    const int liFBWidth = int(lUniforms.mpPerFrame.gFrameBufferSizeVec4.x);
	const int tilesPerWidth = (liFBWidth + (liFBWidth % D_TILE_WIDTH)) / D_TILE_WIDTH;
    //tile index
    const int luTileImageIndex =  liTileGlobalID.x + liTileGlobalID.y *  tilesPerWidth;
	//tile buffer start
	const int luTileBufferIndex = luTileImageIndex * sizeOfTile;
    
#ifndef D_PLATFORM_METAL
    ivec2 liTileBase = liTileGlobalID * ivec2(D_TILE_WIDTH, D_TILE_HEIGHT);
    int liVisibleLights = min(imageLoad(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gLightCluster), liTileBase).x, D_TILE_MAX_LIGHT_COUNT);
	for (int i = 1; i < liVisibleLights + 1; ++i)
	{
		laLightIndices[i - 1] = imageLoad(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gLightCluster),	liTileBase + ivec2(i % D_TILE_WIDTH, i / D_TILE_WIDTH)).x;
	}
#endif
#endif
//TF_END

        WriteOutput(
            lOutColours0Vec4,
            lOutColours1Vec4,
            lOutColours2Vec4,
            lOutColours3Vec4,
            lOutColours4Vec4,
            DEREF_PTR( lUniforms.mpPerFrame ),
            DEREF_PTR( lUniforms.mpCommonPerMesh ),
            DEREF_PTR( lUniforms.mpCustomPerMaterial ),
            vec4( lFragmentColourVec3, 1.0 ),
            lWorldSpacePositionVec3,
            lNormalVec3,
            liMaterialID,
            lfMetallic,
            lfRoughness,
            lfSubsurface,
            lfGlow,
#if defined( D_OUTPUT_MOTION_VECTORS ) || defined( D_FORWARD_RENDERER )
            IN( mScreenSpacePositionVec4 ).xyzw,
            IN( mPrevScreenPosition ).xyzw,
#else
            vec4( 0.0, 0.0, 0.0, 1.0 ),
            vec4( 0.0, 0.0, 0.0, 1.0 ),
#endif
#if !defined( D_ATTRIBUTES ) && !defined( _F07_UNLIT ) || defined( D_FORWARD_RENDERER )
            GetInverseWorldUpTransform( lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4 ),
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gShadowMap ),
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gCloudShadowMap ),
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gDualPMapBack ),
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gDualPMapFront ),
			//TF_BEGIN
#if defined(D_TILED_LIGHTS)
#ifdef D_PLATFORM_METAL
        SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gLightCluster),
		luTileBufferIndex,
#else
		liVisibleLights,
		laLightIndices,
#endif
#endif
            1.0, // AO always 1 for terrain
#endif
			//TF_END
			0.0, // temp. Going to be for per pixel depth of imposters
            false,
            -1.0
        );

#endif // defined( D_CACHE_COLOUR )

        /*
        if ( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 0.25 )
        {
            lOutColours0Vec4 = vec4( 1.0, 0.0, 0.0, 0.0 );
        }
        else
        if ( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.w == 0.5 )
        {
            lOutColours0Vec4 = vec4( 0.0, 1.0, 0.0, 0.0 );
        }
        */
        //lOutColours0Vec4 = vec4( lUniforms.mpCustomPerMesh.gTerrainLodParamsVec4.xyz, 0.0 );

#if 0
        float lfScaling = dot( lTerrainSpaceNormalVec3, vec3( 0.0, 1.0, 0.0 )  );
        lOutColours0Vec4 = vec4( 0.5 * lfScaling, 0.5 * lfScaling, 0.5 * lfScaling, 1.0 );
#endif

#if !defined( D_ATTRIBUTES )
        FRAGMENT_COLOUR = lOutColours0Vec4;
		//TF_BEGIN
#if defined(D_BLOOM) || defined(D_DOF)
		FRAGMENT_COLOUR1 = lOutColours1Vec4;
#endif

#if defined( D_OUTPUT_MOTION_VECTORS ) && defined(D_FORWARD_RENDERER)
#ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
#endif
        {
            FRAGMENT_COLOUR2 = lOutColours2Vec4;
        }
#endif
		//TF_END
#else

        FRAGMENT_COLOUR0 = lOutColours0Vec4;
        FRAGMENT_COLOUR1 = lOutColours1Vec4;
        FRAGMENT_COLOUR2 = lOutColours2Vec4;
        FRAGMENT_COLOUR3 = lOutColours3Vec4;

#endif // !defined( D_ATTRIBUTES )

#if defined ( D_VALIDATE_TEX_CACHE_BLOCKS ) && defined ( D_WRITE_TEX_CACHE )
        #if defined ( D_CACHE_NORMAL )
        FRAGMENT_COLOUR = vec4( 0.0, 0.0, 0.0, 1.0 );
        #else
        FRAGMENT_COLOUR = lUniforms.mpCustomPerMesh.gTexCacheBlockColourVec4;
        #endif
#endif

    }
#endif
}
