////////////////////////////////////////////////////////////////////////////////
///
///     @file       ParticleFragment.h
///     @author     User
///     @date       
///
///     @brief      ParticleFragment
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifdef _F14_UVSCROLL
#define D_DECLARE_TIME
#define D_UV_DYNAMIC
#endif

#ifdef _F13_UVANIMATION
#define D_DECLARE_TIME
#define D_UV_DYNAMIC
#endif

#ifdef _F18_UVTILES
#define D_UV_DYNAMIC
#endif

#define D_PARTICLE_UNIFORMS

//-----------------------------------------------------------------------------
//      Include files

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "UberCommon.h"

#include "Common/Common.shader.h"
#include "Common/CommonParticle.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonFragment.shader.h"

#ifdef SM_CODE
#include "Common/ShaderMillDefines.shader.h"
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

    flat INPUT( vec2,   mParticleIdX_AlphaMultiplierY,  TEXCOORD0 )
    INPUT( vec4,   mTexCoordsVec4,		     	TEXCOORD1 )
    INPUT( vec4,   mScreenSpacePositionVec4,	TEXCOORD2 )
#ifdef _F13_UVANIMATION
    INPUT( float, mfAnimBlendValue,             TEXCOORD3 )
#endif
#ifndef _F07_UNLIT
    INPUT( vec3,   mWorldNormalVec3,			TEXCOORD4 )
#ifdef _F03_NORMALMAP
#ifdef D_PLATFORM_METAL
    flat INPUT(vec3, mTangentSpaceMat3_col0,     TEXCOORD5 )
    flat INPUT(vec3, mTangentSpaceMat3_col1,     TEXCOORD6 )
    flat INPUT(vec3, mTangentSpaceMat3_col2,     TEXCOORD7 )
#else
    flat INPUT(mat3, mTangentSpaceMat3,     TEXCOORD5 )
#endif
#endif

#endif

DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     ComputeLightColour
///
///     @brief      ComputeLightColour
///
///     @param      void
///     @return     
///
//-----------------------------------------------------------------------------
vec3
ComputeLightColour(
    in vec3  lLightDirectionVec3,
    in vec3  lLightColourVec3,
    in vec3  lNormalVec3 )
{
    float lfLightDirDotN;
    float lfLightColour;

    lfLightDirDotN = -dot( lNormalVec3, lLightDirectionVec3 );
        
    // back bit
    //float lfBackLightMin = 0.05;
    //float lfBackLight    = min( 0.5, lfBackLightMin + (1.0 + lfLightColour)*(0.5-lfBackLightMin));
        
    lfLightColour = (lfLightDirDotN + 1.0) * 0.5;
    //lfLightColour = max( lfLightColour, lfBackLight );
 
    return vec3(lfLightColour * lLightColourVec3);
}

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main
///
///     @param      void
///     @return     
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec4 lFragmentColourVec4;
    vec4 lTextureColourVec4;
    vec3 lNormalVec3;
    int liParticleID = int( IN( mParticleIdX_AlphaMultiplierY ).x );
     
    //-----------------------------------------------------------------------------
    ///
    ///     Colour
    ///
    //-----------------------------------------------------------------------------

    vec2 lTexCoords = vec2( IN(mTexCoordsVec4).x, 1.0 - IN(mTexCoordsVec4).y );
    lFragmentColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lTexCoords); // vec4(1.0, 1.0, 1.0, 1.0);
    lTextureColourVec4 = lFragmentColourVec4;

    vec2  lAnimFrameSize = lUniforms.mpCustomPerMesh.gUVScrollStepVec4.xy;
    float lfAnimSpeed    = lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;

    float lfStartTime   = 0.0; //ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID).z;
    float lfTimeElapsed = lUniforms.mpPerFrame.gfTime - lfStartTime;
    float lfFrameNumber = floor( lfTimeElapsed * lfAnimSpeed );

    float lfGlow = 0.0;

    //-----------------------------------------------------------------------------
    ///
    ///     Colour
    ///
    //-----------------------------------------------------------------------------
    //calculate colour and fade alpha if we have stretched the particle
    vec4 lInputColourVec4 = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleColoursVec4, liParticleID );
    lInputColourVec4.xyz = GammaCorrectInput( lInputColourVec4.xyz );
    lInputColourVec4.a *= IN( mParticleIdX_AlphaMultiplierY ).y;
    lInputColourVec4.a = min( lInputColourVec4.a, 1.0 );

#ifdef _F13_UVANIMATION
    vec2 lNextFrameTexCoords = vec2( IN( mTexCoordsVec4 ).z, 1.0 - IN( mTexCoordsVec4 ).w );
    vec4 lNextFrameColour    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lNextFrameTexCoords);
    lFragmentColourVec4 = mix( lFragmentColourVec4, lNextFrameColour, IN( mfAnimBlendValue ) );
#endif

    #ifdef SM_CODE
    {
        float lfSkGlobalTime = lUniforms.mpPerFrame.gfTime;
        float lfScale = 0.0;
        float lfRotation = 0.0;
        float lfOutScale = 0.0;
        float lfOutRotation = 0.0;
        float lfOutGlow = 0.0;
        vec3 lvLocalPosition =  float2vec3(0.0);
        vec3 lvOutLocalPosition = float2vec3(0.0);
        vec4 lVertexColour = lInputColourVec4;
        vec4 lFragmentColour = lFragmentColourVec4;
        vec4 lFragmentNormal = lFragmentColourVec4;

        vec3 lSkNodePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lSkPlanetPositionVec3 = float2vec3(0.0);
        vec3 lSkSunPositionVec3    = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz * 100000.0;

        SM_CODE
        lFragmentColourVec4 = lFragmentColour;
        lfGlow = lfOutGlow;
    }
    #endif


#ifdef _F33_ALPHACUTOUT
    if( lFragmentColourVec4.a < 1.0 )
    {
        discard;
    }
#endif
 
    lFragmentColourVec4 *= lInputColourVec4;

    //-----------------------------------------------------------------------------
    ///
    ///     Soft Particles
    ///
    //-----------------------------------------------------------------------------
    #if 0
    float lWindowCoordZ = IN(mScreenSpacePositionVec4).z / IN(mScreenSpacePositionVec4).w;
    lWindowCoordZ = lWindowCoordZ * 0.5 + 0.5;
    float lfLinearZParticle = lWindowCoordZ;
    #else
    float lfLinearZParticle = IN(mScreenSpacePositionVec4).w;
    #endif

    
    #ifdef D_SOFT_PARTICLES			

        #if defined( D_HEAVYAIR ) && !defined( _F02_SKINNED )
        lFragmentColourVec4.a *= lUniforms.mpCustomPerMesh.gHeavyAirFadeOutVec4.r;		
        lFragmentColourVec4.a *= saturate(lfLinearZParticle * 0.07);
        if (lFragmentColourVec4.a < (1.0 / 384.0) )
        {
            discard;
        }
        #endif
    #endif

        {
#ifdef _F53_COLOURISABLE
            // Change HUE
            vec3 lOriginalHSVVec3 = RGBToHSV( lFragmentColourVec4.xyz );
            vec3 lTintHSVVec3 = RGBToHSV( GammaCorrectInput( lUniforms.mpCommonPerMesh.gUserDataVec4.xyz ) );

            vec3 lHSVColourVec3 = lOriginalHSVVec3;
            lHSVColourVec3.x = lTintHSVVec3.x;
            lHSVColourVec3.y = lTintHSVVec3.y;  //Not working? Try copying across the saturation too. If that fixes it, the problem is the source is too white/doesn't have enough saturation

            if (lTintHSVVec3.y > 0)
            {
                lFragmentColourVec4.rgb = HSVToRGB( lHSVColourVec3 );
            }
#endif
        }


    //-----------------------------------------------------------------------------
    ///
    ///     Normal Mapping and Lighting
    ///
    //-----------------------------------------------------------------------------
    vec3 lUnlitColourVec3 = lFragmentColourVec4.rgb;

    #ifndef _F07_UNLIT
    {
        vec3 lLightColourVec3;
        vec2 lTexCoordsVec2;

        #ifdef _F03_NORMALMAP
        #ifdef D_PLATFORM_METAL
        mat3 mTangentSpaceMat3 = mat3(IN(mTangentSpaceMat3_col0), IN(mTangentSpaceMat3_col1), IN(mTangentSpaceMat3_col2));
        #endif
        {
            vec3 lNormalMapVec3;

            #if defined(_F14_UVSCROLL) && !defined(_F13_UVANIMATION)
                // Scroll UV's of mWorldNormalVec3 map independently
                lTexCoordsVec2 = vec2( IN( mTexCoordsVec4 ).z, 1.0 - IN( mTexCoordsVec4 ).w );
            #else
                lTexCoordsVec2 = vec2( IN( mTexCoordsVec4 ).x, 1.0 - IN( mTexCoordsVec4 ).y );
            #endif

            lNormalVec3 = DecodeNormalMap( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gNormalMap ), lTexCoordsVec2  ) );
            #ifdef D_PLATFORM_METAL
                lNormalVec3 = MUL(  mTangentSpaceMat3, lNormalVec3 );
            #else
                lNormalVec3 = MUL(  IN(mTangentSpaceMat3), lNormalVec3 );     
            #endif

            #if defined( _F30_REFRACTION )

                if( lTextureColourVec4.a == 0.0 )
                {
                    discard;
                }

                // This part could potentially (or maybe should?) be hardcoded on platforms where particle refractions aren't supported at all (i.e. Switch),
                // but this needs testing in that branch - so will do that there.
                if ((lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 8)) != 0)
                {
                    // Refractions are disabled
                    lFragmentColourVec4.rgb = lTextureColourVec4.rgb;
                    lFragmentColourVec4.a = lTextureColourVec4.a * lInputColourVec4.a * (1.0 - lUniforms.mpCommonPerMesh.gRefractionParamsVec4.w);
                }
                else
                {
                    //
                    // Refraction
                    //            
                    vec2 lScreenPos = SCREENSPACE_AS_RENDERTARGET_UVS( (IN( mScreenSpacePositionVec4 ).xy / IN( mScreenSpacePositionVec4 ).w) * 0.5 + 0.5 );

                    float lRefractionEdgeToneDown = 1.0;
                    if ((lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 9)) != 0)
                    {
                        vec2 lEdgeTownDownFactor = abs( lScreenPos - vec2( 0.5, 0.5 ) ); // middle = 0.0, edges = 0.5
                        lEdgeTownDownFactor *= 2.0; // middle = 0.0, edges = 1.0
                        lEdgeTownDownFactor = vec2( 0.9, 0.9 ) - lEdgeTownDownFactor; // middle = 1.0, edges = 0.0
                        lEdgeTownDownFactor = saturate( 9.0 * (lEdgeTownDownFactor) ); // scale 1.0 to 0 on edges
                        lEdgeTownDownFactor = lEdgeTownDownFactor * lEdgeTownDownFactor;// *(1.0 - ((1.0 - lEdgeTownDownFactor) * (1.0 - lEdgeTownDownFactor)));
                        lRefractionEdgeToneDown = saturate( min( lEdgeTownDownFactor.x, lEdgeTownDownFactor.y ) + 0.0001 );
                    }

                    lScreenPos = lScreenPos + lNormalVec3.xy * lUniforms.mpCommonPerMesh.gRefractionParamsVec4.xy;

#ifdef D_PLATFORM_METAL
				//TF_BEGIN
				uvec4 lStencilVec4 = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gStencilBuffer), lScreenPos);
				if ((lStencilVec4.r & 0x10))
				{
					discard;
				}
				//TF_END
#endif
                    // Slightly boost refraction colour artificially (gRefractionParamsVec4.z).
#ifdef D_PLATFORM_ORBIS
                    vec4  lRTColour = lUniforms.mpCommonPerMesh.gRefractionParamsVec4.z * texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gBufferMap ), vec3( lScreenPos, lUniforms.mpPerFrame.gVREyeInfoVec3.x ) );
#else
                    vec4  lRTColour = lUniforms.mpCommonPerMesh.gRefractionParamsVec4.z * texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gBufferMap ), lScreenPos );
#endif                             
                    if (lRefractionEdgeToneDown != 1.0)
                    {
                        lFragmentColourVec4.rgb = mix( lTextureColourVec4.rgb, lRTColour.rgb, 1.0 - saturate( lUniforms.mpCommonPerMesh.gRefractionParamsVec4.w / lRefractionEdgeToneDown ) );
                        lFragmentColourVec4.a = lTextureColourVec4.a * lInputColourVec4.a * lRefractionEdgeToneDown;
                    }
                    else
                    {

                        lFragmentColourVec4.rgb = mix( lRTColour.rgb, lTextureColourVec4.rgb, lUniforms.mpCommonPerMesh.gRefractionParamsVec4.w );
                        lFragmentColourVec4.a = lTextureColourVec4.a * lInputColourVec4.a;
                    }
                }
            #endif
        }
        #else
        {
            lNormalVec3 = normalize( IN( mWorldNormalVec3 ) );            
        }
        #endif

        #ifndef _F30_REFRACTION

            lLightColourVec3 = ComputeLightColour(lUniforms.mpCommonPerMesh.gLightDirectionVec4.rgb, lUniforms.mpCommonPerMesh.gLightColourVec4.rgb, lNormalVec3);

            //add a little ambient, because steep normals produce black and it don't blend well with almost opaque particles
            lLightColourVec3 += 0.2;

            lFragmentColourVec4.rgb *= lLightColourVec3;

        #endif // ! _F30_REFRACTION

    }
    #endif

    #ifdef _F34_GLOW
    {
        lfGlow = mix( lfGlow, lfGlow * sqrt( lfGlow ), saturate( lfGlow - 1 ) );
        lFragmentColourVec4.rgb += lUnlitColourVec3 * lfGlow;
    }
    #endif
    
    //-----------------------------------------------------------------------------
    ///
    ///     Soft Particles - depth buffer
    ///
    //-----------------------------------------------------------------------------
    #ifdef D_SOFT_PARTICLES
        //float lfMaxRGB = max3(lFragmentColourVec4.r, lFragmentColourVec4.g, lFragmentColourVec4.b);
        //if ((lFragmentColourVec4.a * lfMaxRGB) < (1.0 / 384.0))
        //{
        //    discard;
        //}

#ifndef D_PLATFORM_OPENGL
        vec2 lWindowCoordsVec2 = IN_SCREEN_POSITION.xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
#else
        vec2 lWindowCoordsVec2 = (IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w) * 0.5 + 0.5;
        //lWindowCoordsVec2.x = (lWindowCoordsVec2.x - lUniforms.mpPerFrame.gVREyeInfoVec3.y);
        lWindowCoordsVec2 = SCREENSPACE_AS_RENDERTARGET_UVS(lWindowCoordsVec2);
#endif

#ifdef D_PLATFORM_ORBIS
        float lfLinearZDepthBuffer = texture2DArray(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gDepthBuffer), vec3(lWindowCoordsVec2, lUniforms.mpPerFrame.gVREyeInfoVec3.x ) ).r;
#else
#ifdef D_DEPTH_LINEAR_NORM
		float lfLinearZDepthBuffer = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gDepthBuffer), lWindowCoordsVec2).r;
		lfLinearZDepthBuffer = FastDenormaliseDepth(lUniforms.mpPerFrame.gClipPlanesVec4, lfLinearZDepthBuffer);
#else
        float lfLinearZDepthBuffer = texture2D( SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gDepthBuffer),  lWindowCoordsVec2 ).r;
#endif
#endif
#if defined(D_FULL_RES_PARTICLES)
        // If particles are rendered at full resolution, the sample from DEPTH_LINEAR which stores normalised depth values
       lfLinearZDepthBuffer = FastDenormaliseDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfLinearZDepthBuffer );
#endif
#ifdef D_PLATFORM_METAL
//TF_BEGIN
        float lfDepthFade = saturate((lfLinearZDepthBuffer - lfLinearZParticle) * lUniforms.mpCommonPerMesh.gfParticleSoftFadeStrength);
//TF_END
#else
        float lfDepthFade = saturate((lfLinearZDepthBuffer - lfLinearZParticle) * lUniforms.mpCommonPerMesh.gfParticleSoftFadeStrength.x);
#endif
        #ifdef _F02_SKINNED

            if( lfLinearZParticle >= lfLinearZDepthBuffer )
            {
                lFragmentColourVec4.a = 0.0;
            }

        #else

            lFragmentColourVec4.a *= lfDepthFade;
            lFragmentColourVec4.a  = min( lfDepthFade, lFragmentColourVec4.a );	
            // visualise depth fade
            //lFragmentColourVec4.rgba = vec4(1.0, 1.0, 1.0, 1.0 - lfDepthFade);

        #endif

    #else

        lFragmentColourVec4.a   *= clamp((lfLinearZParticle / 5.0) - 0.5, 0.0, 1.0);
        
   #endif

#ifndef D_HEAVYAIR
    {
        bool lbAlphaThresholdIsSet = (lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 7)) != 0;
        if (lbAlphaThresholdIsSet)
        {
            float lfParticleAlphaThreshold = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID ).w;

            if (lfParticleAlphaThreshold != 0.0)
            {
                if (lfParticleAlphaThreshold >= 1.0)
                {
                    lFragmentColourVec4.a = 0.0;
                }
                else
                {
                    lFragmentColourVec4.a = max( 0.0, lFragmentColourVec4.a - lfParticleAlphaThreshold ) / (1.0 - lfParticleAlphaThreshold);
                }
            }
        }
    }
#endif

   FRAGMENT_COLOUR = vec4( lFragmentColourVec4 );
}