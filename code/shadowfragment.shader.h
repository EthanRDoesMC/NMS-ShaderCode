////////////////////////////////////////////////////////////////////////////////
///
///     @file       ShadowFragment.h
///     @author     User
///     @date       
///
///     @brief      ShadowFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 

#if defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL ) || defined( _F20_PARALLAXMAP )
#define D_NORMALMAPPED
#endif

#if defined( D_NORMALMAPPED ) || defined( _F27_VBTANGENT ) || defined( _F31_DISPLACEMENT )
#define D_DECLARE_TANGENT
#endif

#ifdef _F31_DISPLACEMENT
#define D_DECLARE_TIME
#endif

#if defined( _F01_DIFFUSEMAP ) || defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL ) || defined( _F41_DETAIL_DIFFUSE )
#define D_TEXCOORDS
#endif

#define D_USES_WORLD_POSITION

#ifndef _F03_NORMALMAP
#if defined( _F04_ENVMAP ) || !defined( _F07_UNLIT )
#define D_USES_VERTEX_NORMAL
#endif
#endif

#if defined( _F02_SKINNED )
#define D_SKINNING_UNIFORMS
#endif

#define D_DEPTHONLY
#define D_SHADOW

#if defined( _F19_BILLBOARD ) || defined( _F12_BATCHED_BILLBOARD ) || defined( _F15_WIND ) || defined( _F44_IMPOSTER )
#define D_CALC_WORLDPOSITION
#endif

#if defined( _F60_ACUTE_ANGLE_FADE )
#define D_FADE
#endif

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "UberCommon.h"

#if defined( _F44_IMPOSTER )
#include "Imposter.shader.h"
#endif
#include "Common/CommonDepth.shader.h"

#if defined( D_FADE )
#include "Common/CommonFade.shader.h"
#endif

#if defined( _F02_SKINNED )
    //#include "Common/CommonSkinning.shader.h"
#endif

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )

#if !defined( D_TERRAIN )
    INPUT( vec4, mTexCoordsVec4,           TEXCOORD2 )
#endif

#if (defined( D_FADE ) && defined( D_INSTANCE )) || defined( _F44_IMPOSTER )
#if ENABLE_OCTAHEDRAL_IMPOSTERS
    INPUT( vec2, mImposterFrame, TEXCOORD4 )
#endif
    INPUT( vec3, mPixelZandW_mfFadeValueForInstance,    TEXCOORD1 )
#endif    


#ifdef _F60_ACUTE_ANGLE_FADE
    INPUT( vec4, mWorldPositionVec3_mfSpare, TEXCOORD3 )
    INPUT( vec3, mTangentSpaceNormalVec3,    TEXCOORD6 )
#endif


DECLARE_INPUT_END


#if defined( D_PLATFORM_OPENGL )
FRAGMENT_MAIN_COLOUR_SRT
#else
#if defined(_F44_IMPOSTER )
VOID_MAIN_DEPTH_SRT
#elif defined( D_FADE )

#if defined( _F01_DIFFUSEMAP ) && defined( D_PLATFORM_PROSPERO )
    VOID_MAIN_REZ_SRT
#else
    VOID_MAIN_SRT
#endif

#else
VOID_MAIN_COLOUR_EARLYZ_SRT
#endif
#endif
{ 

#ifdef D_FADE    
    
       CheckFade(
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ),
            IN( mScreenSpacePositionVec4 ),
#ifdef  D_INSTANCE             
           IN( mPixelZandW_mfFadeValueForInstance ).z,
#else
           lUniforms.mpCommonPerMesh.gfFadeValue,
#endif 
           lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy,
           vec3(0.0, 0.0, 1.0)
         );    
       

#endif

#if ENABLE_OCTAHEDRAL_IMPOSTERS
    if(OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
    {
        vec2 lTexCoords = IN( mTexCoordsVec4 ).xy;
        lTexCoords.x = ((lTexCoords.x - 0.5) * lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4.x) + 0.5;

        if (lTexCoords.x < 0.0 || lTexCoords.x >= 1.0 || lTexCoords.y < 0.0 || lTexCoords.y >= 1.0)
        {
            discard;
        }

        lTexCoords = lUniforms.mpCustomPerMaterial.gImposterDataVec4.y * (lTexCoords + IN( mImposterFrame ));
        vec4 lDiffuseColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lTexCoords );

        if (lDiffuseColourVec4.a < 0.5) // needs to be alpharef passed into constant buffer
        {
            discard;
        }

        float lfImposterDepth = IN(mPixelZandW_mfFadeValueForInstance.x);
        FRAGMENT_DEPTH = lfImposterDepth;
    }
    else
#endif
    {
        #ifdef _F01_DIFFUSEMAP
        {
            #if defined( _F11_ALPHACUTOUT )
            {
                vec4 lTexCoordsVec4 = IN(mTexCoordsVec4);
                vec4 lDiffuseColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lTexCoordsVec4.xy);

                if ( lDiffuseColourVec4.a < 0.5 ) // needs to be alpharef passed into constant buffer
                {
                    discard;
                }
            }
            #endif 
        }
        #endif
    
        #if defined( _F60_ACUTE_ANGLE_FADE )
        vec3 lViewDirVec3 = normalize(lUniforms.mpPerFrame.gViewPositionVec3 - IN(mWorldPositionVec3_mfSpare).xyz);

        float lfNdotCam = 1.0 - acos(abs(dot(lViewDirVec3, normalize(IN(mTangentSpaceNormalVec3).xyz)))) / (3.14 * 0.5);

        if (lfNdotCam < 0.5)
        {
            CheckFade(
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gFadeNoiseMap),
                IN(mScreenSpacePositionVec4),
                lfNdotCam * 2.0,
                lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy,
                vec3(0.0, 0.0, 1.0));
        }

        #endif

        #if defined( _F44_IMPOSTER )
        // Output modified imposter depth to avoid crap self shadowing artefacts.
        //vec4 lTexCoordsImp = IN( mTexCoordsVec4 );
        
        //lTexCoordsImp *= lUniforms.mpCustomPerMaterial.gImposterDataVec4.x;

        float lfImposterDepth = IN( mPixelZandW_mfFadeValueForInstance.x );
        FRAGMENT_DEPTH = lfImposterDepth;
        #endif
    }

#if defined( D_PLATFORM_OPENGL )
    float lfShadowAlpha = 1.0;
    FRAGMENT_COLOUR = vec4(1.0, 0.0, 1.0, lfShadowAlpha);
#endif    
    
}
