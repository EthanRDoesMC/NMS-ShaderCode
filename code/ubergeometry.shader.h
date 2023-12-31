////////////////////////////////////////////////////////////////////////////////
///
///     @file       UberHullShader.h
///     @author     User
///     @date       
///
///     @brief      UberHullShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "UberCommon.h"


//
// Have to include things that reference the global under here. Things defined above may be parameters to functions in the following includes.
//
#include "Common/CommonVertex.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonDynamicVertex.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Imposter.shader.h"

#ifdef D_SKINNING_UNIFORMS
    //#include "Common/CommonSkinning.shader.h"
#endif

#ifdef _F31_DISPLACEMENT
    #include "Common/CommonDisplacement.shader.h"
#endif

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

    INPUT( vec4,  mTexCoordsVec4,            TEXCOORD0 )

#ifdef D_USES_WORLD_POSITION
    // mfSpare has IN( mkColourVec4 ).a for _F21_VERTEXCOLOUR when D_DEPTHONLY
    //      and IN( mkTangentVec4 ).w otherwise
    INPUT( vec4, mWorldPositionVec3_mfSpare, TEXCOORD1 ) 
#endif

#if !defined( D_DEPTH_CLEAR )

#if (defined ( _F21_VERTEXCOLOUR ) || defined( _F33_SHELLS )) && !defined ( D_DEPTHONLY )
    INPUT( vec4,  mColourVec4,               TEXCOORD2 )
#endif

#if !defined( _F01_DIFFUSEMAP )
    INPUT( vec4,  mMaterialVec4,             TEXCOORD4 ) 
#endif

#ifdef D_USES_VERTEX_NORMAL
    INPUT( vec3, mTangentSpaceNormalVec3, TEXCOORD5 )
#endif

#if !defined( D_DEPTHONLY )


#ifdef _F20_PARALLAXMAP
    INPUT( vec3,  mTangentSpaceEyeVec3,      TEXCOORD6 )
#endif

#if 0 //defined( D_OUTPUT_MOTION_VECTORS )  && defined( _F14_UVSCROLL )
    INPUT( vec4, mPrevTexCoordsVec4,         TEXCOORD8 )
#endif

#if !defined( D_DEFER ) && !defined( _F07_UNLIT )
    flat INPUT( mat3, mUpMatrixMat3,     TEXCOORD7)
#endif

//#if defined( _F44_IMPOSTER )
//    INPUT( vec3, mShadowWorldPositionVec3,   TEXCOORD8 )
//#endif

#if defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
    INPUT( vec3,  mTangentMatRow1Vec3,       TEXCOORD10 )
    INPUT( vec3,  mTangentMatRow2Vec3,       TEXCOORD11 )
    INPUT( vec3,  mTangentMatRow3Vec3,       TEXCOORD12 )
#endif
    

#if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND )
    INPUT( vec3,  mCenteralNormalVec3,       TEXCOORD13 )
    //INPUT( vec3,  mCenterPositionVec3,       TEXCOORD16 )
#endif
   
#if defined( D_OUTPUT_MOTION_VECTORS ) 
    INPUT_VARIANT( vec4,   mPrevScreenPosition,      TEXCOORD14, HAS_MOTION_VECTORS )
#endif

#endif // D_DEPTHONLY
#endif // D_DEPTH_CLEAR

#if defined ( D_USE_SCREEN_POSITION )
    INPUT( vec4, mScreenSpacePositionVec4,   TEXCOORD15 )
#endif

    flat INPUT( vec3, mfFadeValueForInstance_mfLodIndex_mfShearMotionLength, TEXCOORD16 )

#ifdef SM_INTERP
    #define SM_INTERP_VAL( v, n, t ) INPUT( v, n, t )
    SM_INTERP
    #undef SM_INTERP_VAL
#endif

#if defined ( D_SK_USE_LOCAL_POSITION )
    INPUT( vec4, mLocalPositionVec4,   TEXCOORD17 )
#endif

DECLARE_INPUT_END

DECLARE_INPUT_PER_VERTEX_DESCRIPTOR

    INPUT_SCREEN_POSITION_REDECLARED

DECLARE_INPUT_PER_VERTEX_DESCRIPTOR_END

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

    OUTPUT( vec4,  mTexCoordsVec4,            TEXCOORD0 )

#ifdef D_USES_WORLD_POSITION
    // mfSpare has IN( mkColourVec4 ).a for _F21_VERTEXCOLOUR when D_DEPTHONLY
    //      and IN( mkTangentVec4 ).w otherwise
    OUTPUT( vec4, mWorldPositionVec3_mfSpare, TEXCOORD1 ) 
#endif

#if !defined( D_DEPTH_CLEAR )

#if (defined ( _F21_VERTEXCOLOUR ) || defined( _F33_SHELLS )) && !defined ( D_DEPTHONLY )
    OUTPUT( vec4,  mColourVec4,               TEXCOORD2 )
#endif

#if !defined( _F01_DIFFUSEMAP )
    OUTPUT( vec4,  mMaterialVec4,             TEXCOORD4 ) 
#endif

#ifdef D_USES_VERTEX_NORMAL
    OUTPUT( vec3, mTangentSpaceNormalVec3, TEXCOORD5 )
#endif

#if !defined( D_DEPTHONLY )


#ifdef _F20_PARALLAXMAP
    OUTPUT( vec3,  mTangentSpaceEyeVec3,      TEXCOORD6 )
#endif

#if 0 //defined( D_OUTPUT_MOTION_VECTORS )  && defined( _F14_UVSCROLL )
    OUTPUT( vec4, mPrevTexCoordsVec4,         TEXCOORD8 )
#endif

#if !defined( D_DEFER ) && !defined( _F07_UNLIT ) && !defined(D_PLATFORM_METAL)
    flat OUTPUT( mat3, mUpMatrixMat3,     TEXCOORD7)
#endif

//#if defined( _F44_IMPOSTER )
//    OUTPUT( vec3, mShadowWorldPositionVec3,   TEXCOORD8 )
//#endif

#if defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
    OUTPUT( vec3,  mTangentMatRow1Vec3,       TEXCOORD10 )
    OUTPUT( vec3,  mTangentMatRow2Vec3,       TEXCOORD11 )
    OUTPUT( vec3,  mTangentMatRow3Vec3,       TEXCOORD12 )
#endif
    

#if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND )
    OUTPUT( vec3,  mCenteralNormalVec3,       TEXCOORD13 )
    //OUTPUT( vec3,  mCenterPositionVec3,       TEXCOORD16 )
#endif
   
#if defined( D_OUTPUT_MOTION_VECTORS ) 
    OUTPUT_VARIANT( vec4,   mPrevScreenPosition,      TEXCOORD14, HAS_MOTION_VECTORS )
#endif

#endif // D_DEPTHONLY
#endif // D_DEPTH_CLEAR

#if defined ( D_USE_SCREEN_POSITION )
    OUTPUT( vec4, mScreenSpacePositionVec4,   TEXCOORD15 )
#endif

    flat OUTPUT( vec3, mfFadeValueForInstance_mfLodIndex_mfShearMotionLength, TEXCOORD16 )

#if defined ( D_SK_USE_LOCAL_POSITION )
    OUTPUT( vec4, mLocalPositionVec4,   TEXCOORD17 )
#endif

#ifdef SM_INTERP
    #define SM_INTERP_VAL( v, n, t ) OUTPUT( v, n, t )
    SM_INTERP
    #undef SM_INTERP_VAL
#endif

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Geometry Main
///
///     @brief      Geometry Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------


#ifdef _F33_SHELLS
#define MAX_SHELLS 8 
#else
#define MAX_SHELLS 1
#endif

GEOMETRY_MAIN_SRT( 3 * MAX_SHELLS )
{
	GEOMETRY_MAIN_DECLARE_OUTPUT
	
    vec4 lVertexScreenPosition[3];
    vec4 lColourVec4[3];
    vec4 lScreenSpacePositionVec4[3];
    vec4 lPrevScreenPosition[3];
    vec4 lShellDirection[3];
    
    float lfScreenSpaceHeight0 = lUniforms.mpCustomPerMesh.gCustomParams02Vec4.w / IN_SCREEN_POSITION( 0 ).w;
    float lfScreenSpaceHeight1 = lUniforms.mpCustomPerMesh.gCustomParams02Vec4.w / IN_SCREEN_POSITION( 1 ).w;
    float lfScreenSpaceHeight2 = lUniforms.mpCustomPerMesh.gCustomParams02Vec4.w / IN_SCREEN_POSITION( 2 ).w;
    float lfMaxHeight          = max(lfScreenSpaceHeight0, max(lfScreenSpaceHeight1, lfScreenSpaceHeight2));
    
    // lod based on distance rather than screensize of triangle. Having different numbers of shells
    // for different triangles crashes ps4 pro. (ideally would do basedon screensize of object).
    vec3    lWorldPos       = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
    vec3    lViewPos        = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
    float   lfDistance      = abs( length( lWorldPos - lViewPos ) );

#ifdef D_PLATFORM_PROSPERO
    float   lfLodScale;
    //if (lUniforms.mpPerFrame.gFoVValuesVec4.z == 2.0)
    {
        vec3 lCamPos = MAT4_GET_COLUMN(lUniforms.mpPerFrame.gCameraMat4, 3);
        vec3 lToCamera = lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz - lCamPos;
        float lfDistToCamera = length(lToCamera);
        lfLodScale = 1.0 - saturate( (lfDistToCamera - 4.0) / 4.0);
    }
#else
    float   kfMaxDistance   = 40.0; // todo: take from data. This is as usual a last minute fix.
    float   lfLodScale      = 1.0 - smoothstep( 0.4, 1.0, saturate( lfDistance / kfMaxDistance ) );
#endif
    int     liNumShells     = max( 1, int( 8.0 * lfLodScale ) );

    //int liNumShells =  8;//max( min(8, int(lfMaxHeight*800.0)), 1 );
    //int liNumShells =  min(8, int(lfMaxHeight*800.0));
    float lfAdvance = (1.0 / (liNumShells + 2) ); 
    float lfShellHeight = lfAdvance;     
  
    for(int ii=0; ii<3; ++ii)
    {
        lVertexScreenPosition[ii] = IN_SCREEN_POSITION( ii );

        #if defined D_USE_SCREEN_POSITION
        lScreenSpacePositionVec4[ii] = IN( mScreenSpacePositionVec4, ii );
        #endif

        #if defined( D_OUTPUT_MOTION_VECTORS ) 
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            lPrevScreenPosition[ii] = IN( mPrevScreenPosition, ii );
        }
        #endif


        #ifdef _F33_SHELLS

        #ifdef _F21_VERTEXCOLOUR
        lColourVec4[ii] = IN( mColourVec4, ii );
        #else
        lColourVec4[ii] = vec4( 1.0, 1.0, 1.0, 1.0 );
        lColourVec4[ii].a -= lfAdvance;
        #endif

        #ifdef D_USES_VERTEX_NORMAL
        lShellDirection[ii] = vec4( IN( mTangentSpaceNormalVec3, ii ), 0.0 );
        #elif defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
        lShellDirection[ii] = vec4( IN( mTangentMatRow3Vec3, ii ), 0.0 );
        #else
        lShellDirection[ii] = vec4( 0.0, 0.0, 0.0, 0.0 );
        #endif
        lShellDirection[ii] = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lShellDirection[ii] );
        lShellDirection[ii] *= ( lUniforms.mpCustomPerMesh.gCustomParams02Vec4.w * lfAdvance );

        // advance the first shell once so it doesn't just coincide with the main mesh
        lVertexScreenPosition[ii] += lShellDirection[ii];

        #if defined D_USE_SCREEN_POSITION
            lScreenSpacePositionVec4[ii] += lShellDirection[ii];
        #endif

        #if defined( D_OUTPUT_MOTION_VECTORS ) 
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            lPrevScreenPosition[ii] += lShellDirection[ii];
        }
        #endif

        #endif // _F33_SHELLS
    }


    for(int jj=0; jj<liNumShells; ++jj)
    {

        // Emit the triangle
        for(int ii=0; ii<3; ++ii)
        {


            OUT_SCREEN_POSITION        = lVertexScreenPosition[ii];
            // COPY_SCREEN_SLICE(ii)

            OUT(  mTexCoordsVec4             ) = IN(  mTexCoordsVec4, ii             );

            #ifdef D_USES_WORLD_POSITION
                OUT(  mWorldPositionVec3_mfSpare ) = IN(  mWorldPositionVec3_mfSpare, ii );
            #endif

            #if !defined(D_DEPTH_CLEAR)

            #if (defined ( _F21_VERTEXCOLOUR ) || defined( _F33_SHELLS )) && !defined( D_DEPTHONLY )
                OUT(   mColourVec4  ) = lColourVec4[ii];
            #endif

            #if !defined( _F01_DIFFUSEMAP )
                OUT(    mMaterialVec4            ) = IN(    mMaterialVec4, ii            );
            #endif

            #ifdef D_USES_VERTEX_NORMAL
                OUT(  mTangentSpaceNormalVec3  ) = IN(  mTangentSpaceNormalVec3, ii  );
            #endif

            #if !defined( D_DEPTHONLY )

            #ifdef _F20_PARALLAXMAP
                OUT(  mTangentSpaceEyeVec3       ) = IN(  mTangentSpaceEyeVec3, ii       );
            #endif

            #if 0 // defined( D_OUTPUT_MOTION_VECTORS ) && defined( _F14_UVSCROLL )
                OUT(  mPrevTexCoordsVec4         ) = IN(  mPrevTexCoordsVec4, ii         );
            #endif

            #if !defined( D_DEFER ) && !defined( _F07_UNLIT )
                OUT(   mUpMatrixMat3         ) = IN(   mUpMatrixMat3, ii         );
            #endif

            //#if defined( _F44_IMPOSTER )
            //    OUT(  mShadowWorldPositionVec3   ) = IN(  mShadowWorldPositionVec3, ii   );
            //#endif

            #if defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
                OUT(    mTangentMatRow1Vec3      ) = IN(    mTangentMatRow1Vec3, ii      );
                OUT(    mTangentMatRow2Vec3      ) = IN(    mTangentMatRow2Vec3, ii      );
                OUT(    mTangentMatRow3Vec3      ) = IN(    mTangentMatRow3Vec3, ii      );
            #endif


            #if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND )
                OUT(  mCenteralNormalVec3        ) = IN(  mCenteralNormalVec3, ii        );
            #endif



            #ifdef D_OUTPUT_MOTION_VECTORS
            #ifdef D_PLATFORM_METAL
            if(HAS_MOTION_VECTORS)
            #endif
            {
                OUT(    mPrevScreenPosition  ) = lPrevScreenPosition[ii];
            }
            #endif
                
            #endif
            #endif

            #if defined ( D_USE_SCREEN_POSITION )
                OUT(  mScreenSpacePositionVec4  ) = lScreenSpacePositionVec4[ii];
            #endif

            #if defined ( D_SK_USE_LOCAL_POSITION )
                OUT( mLocalPositionVec4 ) = IN(  mLocalPositionVec4, ii        );
            #endif

            OUT(  mfFadeValueForInstance_mfLodIndex_mfShearMotionLength  ) = IN(  mfFadeValueForInstance_mfLodIndex_mfShearMotionLength, ii  );

            #ifdef SM_INTERP
                #define SM_INTERP_VAL( v, n, t ) OUT( n ) = IN( n, ii )
                SM_INTERP
                #undef SM_INTERP_VAL
            #endif

            #ifdef SM_CODE
                {
                    float lfSkGlobalTime = lUniforms.mpPerFrame.gfTime;
                    vec4 lSkUserData = lUniforms.mpCommonPerMesh.gUserDataVec4;

                    vec4 lSkVertexColour = lColourVec4[ii];

                    #if defined ( D_SK_USE_LOCAL_POSITION )
                        vec3 lSkLocalPositionVec3 = IN(mLocalPositionVec4, ii).xyz;
                        vec3 lfOutLocalPosition = lSkLocalPositionVec3;
                    #else
                        vec3 lSkLocalPositionVec3 = float2vec3(0.0);
                        vec3 lfOutLocalPosition = float2vec3(0.0);
                    #endif
                    vec3 lSkWorldPositionVec3 = IN(  mWorldPositionVec3_mfSpare, ii ).xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;;
                    vec3 lSkNodePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;;

                    vec3 lSkPlanetPositionVec3 = float2vec3(0.0);
                    vec3 lSkSunPositionVec3    = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz * 100000.0;
                    vec3 lSkCameraPositionVec3 = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
                    vec3 lSkCameraDirectionVec3 = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 );

                    vec3 lSkPositionDisplacement = float2vec3(0.0);
                    vec3 lSkNormalDisplacement   = float2vec3(0.0);

                    vec3 lvOutShellsOffset  = float2vec3(0.0);
                    vec4 lfOutColour      = float2vec4(0.0);        
                    vec3 lOutNormal       = float2vec3(0.0);
                    float lfOutMetallic   = 0.0;
                    float lfOutRoughness  = 0.0;
                    float lfOutSubsurface = 0.0;
                    float lfOutGlow       = 0.0;

                    //vec4 lColourVec4   = float2vec4(0.0);
                    #ifdef D_USES_VERTEX_NORMAL
                        vec3 lSkInNormal        = IN( mTangentSpaceNormalVec3, ii );
                        vec3 lSkInWorldNormal   = IN( mTangentSpaceNormalVec3, ii );
                    #elif defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
                        vec3 lSkInNormal        = IN( mTangentMatRow3Vec3, ii );
                        vec3 lSkInWorldNormal   = IN( mTangentMatRow3Vec3, ii );
                    #else
                        vec3 lSkInNormal        = float2vec3( 0.0, 0.0, 0.0 );
                        vec3 lSkInWorldNormal   = float2vec3( 0.0, 0.0, 0.0 );
                    #endif

                    vec3 lSkInWorldNormal1 = lSkInWorldNormal;     
                    vec3 lSkInWorldNormal2 = lSkInWorldNormal;   

                    float lfMetallic   = 0.0;
                    float lfRoughness  = 0.0;
                    float lfSubsurface = 0.0;
                    float lfGlow       = 0.0;

                    vec2 lSkUv1 = IN( mTexCoordsVec4, ii ).xy;
                    vec2 lSkUv2 = IN( mTexCoordsVec4, ii ).zw;

                    {
                        SM_CODE
                        vec4 lTransformedShellsOffset = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, vec4( lvOutShellsOffset * lfShellHeight, 0.0 ) );
                        OUT_SCREEN_POSITION = lVertexScreenPosition[ii] + lTransformedShellsOffset;

                        #if defined ( D_USE_SCREEN_POSITION )
                            OUT(  mScreenSpacePositionVec4  ) = lScreenSpacePositionVec4[ii] + lTransformedShellsOffset;
                        #endif
                    }

                    #ifdef D_OUTPUT_MOTION_VECTORS
                    #ifdef D_PLATFORM_METAL
                    if(HAS_MOTION_VECTORS)
                    #endif
                    {
                        lSkLocalPositionVec3 = float2vec3(0.0);
                        lSkNodePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMotionMat4[3].xyz;
                        lfOutLocalPosition = lSkLocalPositionVec3;
                        lfSkGlobalTime = lUniforms.mpPerFrame.gfPrevTime;
                        {
                            SM_CODE
                            vec4 lTransformedShellsOffset = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gPrevViewProjectionMat4, vec4( lvOutShellsOffset * lfShellHeight, 0.0 ) );

                            OUT(    mPrevScreenPosition  ) = lPrevScreenPosition[ii] + lTransformedShellsOffset;
                        }
                    }
                    #endif
                }
                #endif


            WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);

            EMIT_VERTEX;
        }

        END_PRIMITIVE;


        for(int ii=0; ii<3; ++ii)
        {
            // Smaller intervals towards the end of the shell.
            float lfCurrentDisplacement = 1.0f - ( float(jj) / liNumShells );
            lVertexScreenPosition[ii] += lShellDirection[ii] * lfCurrentDisplacement;
            
   
            #if defined D_USE_SCREEN_POSITION
            lScreenSpacePositionVec4[ii] += lShellDirection[ii];
            #endif

            #if defined( D_OUTPUT_MOTION_VECTORS ) 
            #ifdef D_PLATFORM_METAL
            if(HAS_MOTION_VECTORS)
            #endif
            {
                lPrevScreenPosition[ii] += lShellDirection[ii];
            }
            #endif

            #ifdef _F33_SHELLS
            lColourVec4[ii].a = saturate( lColourVec4[ii].a - lfAdvance );
            #endif

            lfShellHeight += lfAdvance;
        }
    }

}

