////////////////////////////////////////////////////////////////////////////////
///
///     @file       ShadowVertex.h
///     @author     User
///     @date       
///
///     @brief      ShadowVertexShader
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

#ifndef D_VERTEX
#define D_VERTEX
#endif
#define D_SHADOW_VERTEX

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "UberCommon.h"

#if defined( _F44_IMPOSTER )
    #include "Imposter.shader.h"
#endif
#include "Common/CommonVertex.shader.h"
#include "Common/CommonDynamicVertex.shader.h"
#include "Common/CommonPlanet.shader.h"

#if defined( _F02_SKINNED )
    #include "Common/CommonSkinning.shader.h"
#endif

#ifdef _F31_DISPLACEMENT
    #include "Common/CommonDisplacement.shader.h"
#endif



//-----------------------------------------------------------------------------
//      Global Data

#define D_PERSPECTIVE_SHADOWMAPx

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

    INPUT( vec4,       mkLocalPositionVec4,   POSITION0    )
#if !defined( D_TERRAIN )
    INPUT( vec4,       mkTexCoordsVec4,       TEXCOORD0    )
#endif
    INPUT( vec4,       mkLocalNormalVec4,     TEXCOORD1    )

#if defined( D_DECLARE_TANGENT ) && !defined( D_TERRAIN )
    INPUT( vec4,       mkTangentVec4,         TANGENT0     )
#endif

#if defined( _F21_VERTEXCOLOUR ) || defined( _F29_VBCOLOUR )
    INPUT( vec4,       mkColourVec4,          TEXCOORD2    )
#endif

#ifdef  D_SKINNING_UNIFORMS 
    INPUT( JOINT_TYPE, mkJointsVec4,          BLENDINDICES )
    INPUT( vec4,       mkWeightsVec4,         BLENDWEIGHT  )
#endif

#if 0 //def D_INSTANCE
    INPUT( vec4,       mkTransformMat4R0,     TEXCOORD3    )
    INPUT( vec4,       mkTransformMat4R1,     TEXCOORD4    )
    INPUT( vec4,       mkTransformMat4R2,     TEXCOORD5    )
    INPUT( vec4,       mkTransformMat4R3,     TEXCOORD6    )
    INPUT( vec4,       mkTransformMat4R4,     TEXCOORD7    )
    INPUT( vec4,       mkTransformMat4R5,     TEXCOORD8    )
#endif

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

    OUTPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )

#if !defined( D_TERRAIN )
    OUTPUT( vec4, mTexCoordsVec4,           TEXCOORD2 )
#endif

#if (defined( D_FADE ) && defined( D_INSTANCE )) || defined( _F44_IMPOSTER )
#if ENABLE_OCTAHEDRAL_IMPOSTERS
    OUTPUT( vec2, mImposterFrame, TEXCOORD4 )
#endif
    OUTPUT( vec3, mPixelZandW_mfFadeValueForInstance,  TEXCOORD1 )
#endif    
 
#ifdef _F60_ACUTE_ANGLE_FADE
    OUTPUT( vec4, mWorldPositionVec3_mfSpare, TEXCOORD3 )
    OUTPUT( vec3, mTangentSpaceNormalVec3, TEXCOORD6 )
#endif

DECLARE_OUTPUT_END

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
#ifdef D_INSTANCE
    VERTEX_MAIN_INSTANCED_SRT
#else
    VERTEX_MAIN_SRT
#endif
{
    vec4 lLocalPositionVec4;
 #ifdef D_CALC_WORLDPOSITION
    vec4 lWorldPositionVec4;
 #endif
    vec4 lScreenPositionVec4;
    mat4 lWorldMat4;
    vec3 lLocalNormalVec3;
    vec4 lShearParamsVec4 = float2vec4( 0.0 );

    
    lLocalPositionVec4 = IN( mkLocalPositionVec4 );
    lLocalPositionVec4.w = 1.0;


#ifdef D_INSTANCE
    sPerInstanceData lInstanceData = GETBUFFERDATA(lUniforms.mpCommonPerMesh, gaPerInstanceData, instanceID);

    lWorldMat4[0] = lInstanceData.mkTransformMat4R0;
    lWorldMat4[1] = lInstanceData.mkTransformMat4R1;
    lWorldMat4[2] = lInstanceData.mkTransformMat4R2;
    lWorldMat4[3] = lInstanceData.mkTransformMat4R3;

#ifdef D_SKINNING_UNIFORMS
        vec4 lExtraParams = lInstanceData.mkTransformMat4R4;

        vec3 lWindEigens0 = vec3(lWorldMat4[ 1 ].w, lWorldMat4[ 2 ].w, lWorldMat4[ 3 ].w );
        vec3 lWindEigens1 = lExtraParams.xyz;
#endif

    lShearParamsVec4 = lInstanceData.mkTransformMat4R5;

#ifdef D_FADE
    OUT( mPixelZandW_mfFadeValueForInstance ).z = lWorldMat4[ 0 ].w;
#endif        
    lWorldMat4[ 0 ].w = 0.0; // this matrix element is the instance fade value (PS4: may need to assign after the transpose-line)
    lWorldMat4[ 1 ].w      = 0.0; // this matrix element is one of the wind eigenvalue cosines
    lWorldMat4[ 2 ].w      = 0.0; // this matrix element is one of the wind eigenvalue cosines
    lWorldMat4[ 3 ].w      = 1.0; // this matrix element is one of the wind eigenvalue cosines

    lWorldMat4 = PLATFORM_TRANSPOSE( lWorldMat4 );
#else
    lWorldMat4 = lUniforms.mpCommonPerMesh.gWorldMat4;
#endif

#if !defined( D_TERRAIN )
     vec4 lTexCoordsVec4;
     lTexCoordsVec4 = IN( mkTexCoordsVec4 );
     lTexCoordsVec4.xy = IN(mkTexCoordsVec4).xy;
#endif
     
    //-----------------------------------------------------------------------------
    ///
    ///     Skinning
    ///
    //-----------------------------------------------------------------------------

    #if defined( _F02_SKINNED )
    #ifdef D_INSTANCE
    {
        vec4 lShearValues = lInstanceData.mkTransformMat4R5;

        lLocalPositionVec4.xyz = GetLdsWindPosition( DEREF_PTR( lUniforms.mpCommonPerMesh ), lLocalPositionVec4.xyz, IN( mkWeightsVec4 ), IN( mkJointsVec4 ), lWindEigens0, lWindEigens1, lShearValues );
    }
    #else
    {
        lLocalPositionVec4 = GetSkinPosition( DEREF_PTR( lUniforms.mpCommonPerMesh ), lLocalPositionVec4, IN( mkWeightsVec4 ), IN( mkJointsVec4 ) );
    }
    #endif
    #endif


 //   
    
    //-----------------------------------------------------------------------------
    ///
    ///     Vertex Rotation
    ///
    ////-----------------------------------------------------------------------------
    #ifdef _F17_VERTEX_ROTATION

        lLocalPositionVec4.xyz = ApplyRotation( lUniforms.mpPerFrame.gfTime, lLocalPositionVec4.xyz );

    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     Vertex Displacement
    ///
    //-----------------------------------------------------------------------------
    #ifdef _F31_DISPLACEMENT
    {
        //vec3 lDisplaceBasePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lDisplaceBasePositionVec3 = lWorldMat4[3].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;


        DisplaceVertex( lLocalPositionVec4,
                        lUniforms.mpPerFrame.gfTime + lDisplaceBasePositionVec3.x + lDisplaceBasePositionVec3.y + lDisplaceBasePositionVec3.z,
                        DEREF_PTR( lUniforms.mpCustomPerMaterial ));
    }
    #endif // _F31_DISPLACEMENT

#ifdef _F60_ACUTE_ANGLE_FADE
    OUT( mWorldPositionVec3_mfSpare ) = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, lLocalPositionVec4 );  
    OUT( mTangentSpaceNormalVec3 )    = normalize( CalcWorldVec( lUniforms.mpCommonPerMesh.gWorldNormalMat4, IN( mkLocalNormalVec4 ).xyz ) );
#endif

#if defined( _F44_IMPOSTER ) 
    vec3 lImposterLightPosition;
    if (OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
    {
        // Due to the way imposter vertex positioning works - we need to go with this "hack" - treating the directional light as a point
        // light in the far distance.
        lImposterLightPosition = lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz + (100000.0 * -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz);
    }
#endif

    //-----------------------------------------------------------------------------
    ///
    ///     Billboarding
    ///
    //-----------------------------------------------------------------------------	
    #ifdef _F19_BILLBOARD   
    {
        vec3 lWorldNormalVec3;
        mat4 lCameraMat4 = lUniforms.mpPerFrame.gViewMat4;
        #if defined( _F44_IMPOSTER ) || (defined( _F12_BATCHED_BILLBOARD ) && defined( D_SHADOW_VERTEX ) )

        //  Imposters only require a direction(Camera AT)
        lCameraMat4[2].xyz =  -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz;
#if defined( _F44_IMPOSTER ) 
        if (OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
        {
            lCameraMat4[3].xyz = lImposterLightPosition;
        }
#endif
        #endif        
        CalcBillboardValues( 
            lCameraMat4,
            lUniforms.mpCommonPerMesh.gWorldMat4,
            lLocalPositionVec4, 
            lUniforms.mpCustomPerMesh.gCustomParams01Vec4,
            lWorldMat4, 
            IN( mkTexCoordsVec4 ),
            lShearParamsVec4,
#if defined( _F44_IMPOSTER ) 
            OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4),
            lUniforms.mpCustomPerMaterial.gOctahedralImposterDataVec4.z,
#endif
            lLocalPositionVec4, 
            lWorldPositionVec4, 
            lLocalNormalVec3, 
            lWorldNormalVec3 );
            
        /*
        #ifdef _F12_BATCHED_BILLBOARD
        {
            // Move billboards closer to light so shadows as cast from top of billboard rather than center
            lWorldPositionVec4.xyz += normalize( -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz ) * lUniforms.mpCommonPerMaterial.gLightDirectionVec4.w;
        }	
        #endif
        */

        // TEST (hardcoded for the moment)
#ifdef _F12_BATCHED_BILLBOARD
        {
            // Move billboards closer to light so shadows as cast from top of billboard rather than center
            lWorldPositionVec4.xyz +=  /*normalize*/( -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz ) * -2.0;
        }
#endif
    }
    #elif defined(D_CALC_WORLDPOSITION)
    //-----------------------------------------------------------------------------
    ///
    ///     World Transform
    ///
    //-----------------------------------------------------------------------------    
    {
    #ifdef D_INSTANCE
        lWorldPositionVec4 = CalcWorldPosInstanced( lUniforms.mpCommonPerMesh.gWorldMat4, lWorldMat4, lLocalPositionVec4 );
    #else        
        lWorldPositionVec4 = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, lLocalPositionVec4 );
    #endif        
    }    
    #endif
    
    //-----------------------------------------------------------------------------
    ///
    ///     Wind
    ///
    //-----------------------------------------------------------------------------
    #ifdef _F15_WIND
    {
        mat3 lWorldUpMatrix = GetWorldUpTransform( lWorldPositionVec4.xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4 );        
        vec4 lPlanetRelPositionVec4 = lWorldPositionVec4 - vec4( lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz, 0.0 );
        lWorldPositionVec4 += CalcWindVec4( lPlanetRelPositionVec4, lWorldMat4, lWorldUpMatrix, lUniforms.mpPerFrame.gfTime );		
    }
    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     Texturing
    ///
    //-----------------------------------------------------------------------------
    {
#if defined( _F44_IMPOSTER ) && defined( _F11_ALPHACUTOUT )
        {
            vec3 lImposterAt = MAT4_GET_COLUMN(lWorldMat4, 2);
#if ENABLE_OCTAHEDRAL_IMPOSTERS
            if (OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
            {
                mat4 lInverseWorldMat4 = Inverse(lWorldMat4);

                vec2 lGridCoordinatePos;
                vec2 lTexCoords0;
                vec2 lImposterFrame0;
                vec3 lViewPositionVec3 = lImposterLightPosition;

                vec3 lLocalCameraPosVec3 = MUL( lInverseWorldMat4, vec4( lViewPositionVec3, 1.0 ) ).xyz;
                vec3 lProperLocalPosVec3 = MUL( lInverseWorldMat4, lWorldPositionVec4 ).xyz;
                ImposterVertexShadow(
                    // Inputs
                    lWorldPositionVec4, lProperLocalPosVec3,
                    lViewPositionVec3, lLocalCameraPosVec3, lImposterAt, lTexCoordsVec4.xy, lUniforms.mpCustomPerMaterial.gImposterDataVec4, lUniforms.mpCustomPerMaterial.gOctahedralImposterDataVec4,
                    // Outputs
                    lTexCoords0,
                    lImposterFrame0
                );
                OUT(mImposterFrame) = lImposterFrame0;
                lTexCoordsVec4.xy = lTexCoords0.xy;
            }
            else
#endif
            {

                float lfHeight;
                vec3 lWindingAxis = GetWorldUp(lWorldPositionVec4.xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4, lfHeight);

                lTexCoordsVec4.x = CalculateImposterTexCoordsU(lUniforms.mpCommonPerMesh.gLightPositionVec4.xyz, MAT4_GET_COLUMN(lUniforms.mpPerFrame.gCameraMat4, 3), lImposterAt, lWindingAxis, lTexCoordsVec4.xy, lUniforms.mpCustomPerMaterial.gImposterDataVec4);
                lTexCoordsVec4.xy = SCREENSPACE_AS_RENDERTARGET_UVS(lTexCoordsVec4.xy);
            }
        }
#endif

    }

#if !defined( D_TERRAIN )
        OUT( mTexCoordsVec4 ) = lTexCoordsVec4;
#endif

    // correct shadows for billboards (otherwise they cast rectangular shadows)
    #if defined( _F18_UVTILES ) && defined( _F11_ALPHACUTOUT )
    {
        vec2 lWorldPosVec2 = lWorldMat4[3].xz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xz;
            
        #ifdef _F12_BATCHED_BILLBOARD
            lWorldPosVec2 += IN( mkLocalPositionVec4 ).xz -  vec2( lTexCoordsVec4.x - 0.5,  0.0 );
        #endif

        // Divide the x pos down because in batched billboards this has some slight inaccuracies
        float lfPos = lWorldPosVec2.x / 8.0 + lWorldPosVec2.y * 10.0;

        lTexCoordsVec4.x /= lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;
        lTexCoordsVec4.x += floor( mod( lfPos, lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z ) ) / lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;

        lTexCoordsVec4.y /= lUniforms.mpCustomPerMesh.gUVScrollStepVec4.w;
        lTexCoordsVec4.y += floor( mod( lfPos, lUniforms.mpCustomPerMesh.gUVScrollStepVec4.w ) ) / lUniforms.mpCustomPerMesh.gUVScrollStepVec4.w;
            
        OUT( mTexCoordsVec4 ) = lTexCoordsVec4;
    }        
    #endif


    //-----------------------------------------------------------------------------
    ///
    ///     Screen Space Transform
    ///
    //-----------------------------------------------------------------------------
    {
    #ifdef D_CALC_WORLDPOSITION
        lScreenPositionVec4 = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );
    #else
        vec4 lWorldPositionVec4 = MUL(lUniforms.mpCommonPerMesh.gWorldMat4, lLocalPositionVec4);
        #ifdef D_INSTANCE
    	// Apply the two world matrices in turn, rather than combining the world matrices
    	lWorldPositionVec4 = MUL( lWorldMat4, lWorldPositionVec4 );
        lScreenPositionVec4 = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4);
        #else
        #if defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_SWITCH )
        lScreenPositionVec4 = CalcScreenPosFromLocal(lUniforms.mpCommonPerMesh.gWorldViewProjectionMat4, lLocalPositionVec4);
        #else
        lScreenPositionVec4 = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4);
        #endif  
        #endif
    #endif
      
        

#if defined( _F44_IMPOSTER )
    // Push back imposter depth by half its bounding box in shadowmap, so it doesn't cast on itself.
    float lfBoxDepth            = lUniforms.mpCustomPerMesh.gBboxDepthAndClips.x;
    lfBoxDepth                  = lUniforms.mpCustomPerMesh.gBboxDepthAndClips.z + ( lfBoxDepth / ( lUniforms.mpCustomPerMesh.gBboxDepthAndClips.w - lUniforms.mpCustomPerMesh.gBboxDepthAndClips.z ) );
#ifdef D_ENABLE_REVERSEZ_PROJECTION
    OUT( mPixelZandW_mfFadeValueForInstance ).x        = ( 0.5 - lScreenPositionVec4.z  * 0.5 ) - ( lfBoxDepth * 0.5 );
#else
    OUT( mPixelZandW_mfFadeValueForInstance ).x        = ( lScreenPositionVec4.z  * 0.5 + 0.5 ) + ( lfBoxDepth * 0.5 );
#endif
    OUT( mPixelZandW_mfFadeValueForInstance ).y        = lScreenPositionVec4.w;
#endif

    #ifndef D_PERSPECTIVE_SHADOWMAP
    
        //for ortho
        //we need to bias the depth from -0.5 to 0.5 range into 0 to 1 range
        //we also clamp objects behind the camera to a very small depth value
        //to avoid missing casters (pancaking)

        #if defined( D_TERRAIN ) && defined( D_FADE )
        float lfFade = lUniforms.mpCommonPerMesh.gfFadeValue;
        if (lfFade < 1.0)
        {
            float lfBias;
            if (lfFade < 0.0)
            {
                // Fade out
                lfBias = mix(0.005, 0.04, lfFade + 2.0);
            }
            else
            {
                // Fade in
                lfBias = mix(0.04, 0.005, lfFade);
            }

            lScreenPositionVec4.z += lfBias;
        }
        #endif
        lScreenPositionVec4.z = max( lScreenPositionVec4.z, -1.0 );

		#ifdef D_ENABLE_REVERSEZ_PROJECTION
		lScreenPositionVec4.z = 0.5 - (lScreenPositionVec4.z * 0.5);
		#endif

                
    #else

    #endif
    }   

    // gInverseModelMat4 has the combined camera, cameraview*projection which does not differ per cascade
    OUT( mScreenSpacePositionVec4 ) = lScreenPositionVec4;

    SCREEN_POSITION = lScreenPositionVec4;

}