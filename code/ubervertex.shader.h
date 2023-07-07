////////////////////////////////////////////////////////////////////////////////
///
///     @file       UberVertexShader.h
///     @author     User
///     @date       
///
///     @brief      UberVertexShader
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

#include "UberCommon.h"

//
// Have to include things that reference the global under here. Things defined above may be parameters to functions in the following includes.
//
#if defined( _F44_IMPOSTER ) || defined( D_IMPOSTER )
    #include "Imposter.shader.h"
#endif
#include "Common/CommonVertex.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonDynamicVertex.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonPlanet.shader.h"

#ifdef D_SKINNING_UNIFORMS
    #include "Common/CommonSkinning.shader.h"
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

    INPUT( vec4,        mkLocalPositionVec4,  POSITION0     )
    INPUT( vec4,        mkTexCoordsVec4,      TEXCOORD0     )
    INPUT( vec4,        mkLocalNormalVec4,    TEXCOORD1     )

#ifdef D_DECLARE_TANGENT
    INPUT( vec4,        mkTangentVec4,        TANGENT0      )
#endif

#if defined( _F21_VERTEXCOLOUR ) || defined( _F29_VBCOLOUR )
    INPUT( vec4,        mkColourVec4,         TEXCOORD2     )
#endif

#ifdef D_SKINNING_UNIFORMS
    INPUT( JOINT_TYPE,  mkJointsVec4,         BLENDINDICES  )
    INPUT( vec4,        mkWeightsVec4,        BLENDWEIGHT   )
#endif

/*#ifdef D_INSTANCE
    INPUT( vec4,        mkTransformMat4R0,    TEXCOORD3     )
    INPUT( vec4,        mkTransformMat4R1,    TEXCOORD4     )
    INPUT( vec4,        mkTransformMat4R2,    TEXCOORD5     )
    INPUT( vec4,        mkTransformMat4R3,    TEXCOORD6     )
    INPUT( vec4,        mkTransformMat4R4,    TEXCOORD7     )
    INPUT( vec4,        mkTransformMat4R5,    TEXCOORD8     )
#endif*/

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

    OUTPUT( vec4,  mTexCoordsVec4,            TEXCOORD0 )

#ifdef D_USES_WORLD_POSITION
    OUTPUT( vec4, mWorldPositionVec3_mfSpare, TEXCOORD1 ) 
#endif

#if !defined( D_DEPTH_CLEAR )

#if (defined ( _F21_VERTEXCOLOUR ) || defined( _F45_VERTEX_BLEND ) || defined( _F33_SHELLS )) && !defined ( D_DEPTHONLY )
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
        OUTPUT( vec3,  mTangentSpaceLightDirVec3, TEXCOORD7 )
#endif

#if 0 //defined( D_OUTPUT_MOTION_VECTORS )  && defined( _F14_UVSCROLL )
    OUTPUT( vec4, mPrevTexCoordsVec4,         TEXCOORD8 )
#endif

#if !defined( D_DEFER ) && !defined( _F07_UNLIT ) && !defined( D_DEPTHONLY ) && !defined( D_PLATFORM_METAL )
    flat OUTPUT( mat3, mUpMatrixMat3,     TEXCOORD9)
#endif

//#if defined( _F44_IMPOSTER )
//    OUTPUT( vec3, mShadowWorldPositionVec3,   TEXCOORD8 )
//#endif

#if defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
    OUTPUT( vec3,  mTangentMatRow1Vec3,       TEXCOORD13 )
    OUTPUT( vec3,  mTangentMatRow2Vec3,       TEXCOORD14 )
    OUTPUT( vec3,  mTangentMatRow3Vec3,       TEXCOORD15 )
#endif
    

#if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND ) || defined( D_DEFERRED_DECAL )
    flat OUTPUT( vec3,  mCenteralNormalVec3,	      TEXCOORD16 )
#endif
   
#if defined( D_OUTPUT_MOTION_VECTORS ) 
    OUTPUT_VARIANT( vec4,   mPrevScreenPosition,      TEXCOORD17, HAS_MOTION_VECTORS)
#endif

#endif // D_DEPTHONLY
#endif // D_DEPTH_CLEAR

#if defined ( D_USE_SCREEN_POSITION )
    OUTPUT( vec4, mScreenSpacePositionVec4,   TEXCOORD18 )
#endif

    flat OUTPUT( vec3, mfFadeValueForInstance_mfLodIndex_mfShearMotionLength, TEXCOORD19 )

#if defined ( D_SK_USE_LOCAL_POSITION )
    OUTPUT( vec4, mLocalPositionVec4,   TEXCOORD20 )
#endif

#if ENABLE_OCTAHEDRAL_IMPOSTERS && defined(_F44_IMPOSTER)
        OUTPUT( vec4, mImposterData0, TEXCOORD21 )
        flat OUTPUT( vec4, mImposterViewNormal, TEXCOORD24 )
        OUTPUT( vec4, mImposterFrameXY_FrameProjectonVecZW0, TEXCOORD25 )
        OUTPUT( vec4, mImposterFrameXY_FrameProjectonVecZW1, TEXCOORD26 )
        OUTPUT( vec4, mImposterFrameXY_FrameProjectonVecZW2, TEXCOORD27 )
#if defined ( D_INSTANCE ) && !defined ( D_PLATFORM_METAL )
        flat OUTPUT( mat3, mWorldNormalMat3, TEXCOORD28 )
#endif
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

#if defined D_PLATFORM_DX12 && !defined D_PLATFORM_XBOXONE
vec3 fix_normal(vec3 v)
{
    return normalize((v - int3(v > 0.5)) * 2);
}
#else
vec3 fix_normal(vec3 v)
{
    return normalize(v);
}
#endif


#if !defined(_F19_BILLBOARD) && defined(_F15_WIND) && defined(_F21_VERTEXCOLOUR) 
vec4
DoVertexWind(
    vec4 lWindParamsVec4,
    vec4 lObjectPositionWorldSpaceVec4,
    vec4 lVertexPositionWorldSpaceVec4,
    vec4 lVertexPositionObjectSpaceVec4,
    vec4 lNormalWorldSpaceVec4,
    vec4 lMasksVec4,
    float lfTime,
    in CommonPerMeshUniforms lMeshUniforms )
{    
    // TODOs:
    // - Do all in object space so we can just wiggle things in major axes.
    // - Make direction of movement take into account wind direction more.
    // - Application-side - make wind more dynamic so it affects the previous point more.
    // - Make animation offset work (based on texture channel)
    // - Nicer noise/movenent. Profile it too. Use noise texture with a few octaves, rather than loads of sin/cos?
    // - Branch movement is crap, how to make that nicer without need joint info.
    // - How to deal with height of tree? Or do we care? Just have max movement capped.
    // - Remove all hardcoded values and pass in from app.
    // - Fixup normals after movement
    // - lod out on distance

    const float kfAnimationOffsetMax = 5.0;
    float lfAnimationOffset = lWindParamsVec4.w * kfAnimationOffsetMax;
    vec4 lWorldPositionVec4 = lVertexPositionWorldSpaceVec4;
    
    // Base wind movement
    /*
    vec3 lWindDirectionVec3 = normalize( lMeshUniforms.gWindParams1Vec4.xyz ); // this should be the game wind vector passed in for this position (with some added noise here)
    float lfWindStrength    = ( sin( lfTime ) + 1.0f ) * 0.5;
    lfWindStrength          = lfWindStrength * lfAnimationOffset; // TODO this isn't how to use the offset!
    lWindDirectionVec3      *= lfWindStrength;
    */
    vec3 lWindDirectionVec3 = lMeshUniforms.gWindParams2Vec4.xyz; // this should be the game wind vector passed in for this position
    float lfWindStrength = (sin( lfTime ) + 1.0f) * 0.5;
    lfWindStrength = lfWindStrength * lfAnimationOffset;
    lWindDirectionVec3 *= 5.0f;// lfWindStrength;
    

    //----------------------------------------------------------------------------------------------------------
    // Trunk Movement
    //----------------------------------------------------------------------------------------------------------
    

    // Get height of this instance. TODO should get this from data somehow. Vertex colour? Or upload from bbox
    float lfObjectPositionHeight = lObjectPositionWorldSpaceVec4.y ;

    // how much to scale the trunk movement (multiplied by 0-1 value representing height of vertex)
    float lfMovementScale   = lWindParamsVec4.x; 

    // Get 0-1 height value, clamped, and multiply up a bit so only the top moves.
    float lfHeight  = clamp( ( lVertexPositionObjectSpaceVec4.y ) / 25.0 , 0.0, 1.0  );
    lfHeight *= lfHeight * lfHeight;

    // Generate some noise for movement. Currently this doesn't actually take into account wind direction!
    float lfPeriod  = sin( lfTime + ( lObjectPositionWorldSpaceVec4.x * 0.1 ) ) * cos( lfTime ) * sin( lfTime + 0.2 ) * cos( lfTime + 0.03 ) * sin( lfTime + 1.1 ) * cos( lfTime + 1.3 );
    lfPeriod  = lfPeriod * 3.0;
    float lfSinTime = lfPeriod * lfMovementScale * lfHeight;

    //lWorldPositionVec4 +=  vec4( lfSinTime, 0.0, lfSinTime, 0.0 ) ;
    lWorldPositionVec4.xyz +=  lWindDirectionVec3 * lfSinTime;


    //----------------------------------------------------------------------------------------------------------
    // Branch movement
    //----------------------------------------------------------------------------------------------------------

    // get this from vertex colour
    vec3 lDelta = lVertexPositionObjectSpaceVec4.xyz;// IN(mkLocalPositionVec4).xyz;
    lDelta.y = 0.0;
    float lfDistanceFromTrunk = length(lDelta);

    lfDistanceFromTrunk = clamp(lfDistanceFromTrunk, 0.0, 10.0);
    /*
    lfSinTimeBranch = lfSinTimeBranch * sin(lUniforms.mpPerFrame.gfTime);
    lfSinTimeBranch *= lfDistanceFromTrunk;
    //float lfSinTimeBranch = lfPeriod * 1.0 * lfDistanceFromTrunk;

    lLocalPositionVec4 = lLocalPositionVec4 + vec4(0.0, lfSinTimeBranch, 0.0, 0.0);
    */
        
    float lfBranchMovementScale = lMasksVec4.b;
    float lfAnimationMaskOffest = lMasksVec4.g;

    // lfAnimationMaskOffest offset the branch animation within a tree (so all branches on a single tree don't move in sync), 
    // then also use world position so all trees aren't in sync.
    float lfSinTimeBranch = lfPeriod * lfBranchMovementScale* sin( lfTime + lfAnimationMaskOffest + lObjectPositionWorldSpaceVec4.z)* lWindParamsVec4.x* lfDistanceFromTrunk;
   // use lfperiod or but mask it out like above.
    lfSinTimeBranch *= 0.1;


    // TODO this could be a rotation rather than just a translation, so the branch pivots.
    lWorldPositionVec4 += vec4( 0.0, lfSinTimeBranch, 0.0, 0.0 ) ;
    

    //----------------------------------------------------------------------------------------------------------
    // Leaf movement
    //----------------------------------------------------------------------------------------------------------
    // TODO maybe scale up movement based on distnce
    const float kfMaxLeafDistance     = lWindParamsVec4.y;    // max distance in world units that the vertex can move.
    const float kfLeafFrequencyScale  = lWindParamsVec4.z;    // movement frequency scale.
    const float lfLeafMovementScale   = lMasksVec4.r; // how much this vertex is allowed to move (comes from vertex colour mask)
    const float lfLowFreqNoise        = sin(  lfTime * kfLeafFrequencyScale + lVertexPositionWorldSpaceVec4.x );
    const float lfMedFreqNoise        = sin( (lfTime * kfLeafFrequencyScale + lVertexPositionWorldSpaceVec4.x ) * 5.0 ) * 0.75;
    const float lfHighFreqNoise       = sin( (lfTime * kfLeafFrequencyScale + lVertexPositionWorldSpaceVec4.x ) * 10.0 ) * 0.5;
    const float lfLeafMovement        = lfLowFreqNoise * lfMedFreqNoise * lfHighFreqNoise; // do this with small noise texture, move it thorugh object space
    
    // Currently just flutters up and down (with tiny x/z movement). Make something nicer here?
    // TODO, this was in local space, but now in world space, which means can't just move in Y (as up changes depending where you are)
    lWorldPositionVec4 += vec4( lfLeafMovement * 0.1, lfLeafMovement, lfLeafMovement * 0.1, 0.0 ) * lfLeafMovementScale * kfMaxLeafDistance;

    return lWorldPositionVec4;
}
#endif

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
    mat4 lWorldMat4;
    mat4 lPrevWorldMat4;
    vec4 lLocalPositionVec4;
    vec4 lPrevLocalPositionVec4;
    vec4 lWorldPositionVec4 = float2vec4( 0.0 );
    vec4 lPrevWorldPositionVec4 = float2vec4( 0.0 );
    vec4 lScreenSpacePositionVec4;
    vec3 lLocalNormalVec3;
    vec3 lWorldNormalVec3;
    vec3 lLocalTangentVec3;
    vec4 lTexCoordsVec4 = IN(mkTexCoordsVec4);
    vec4 lPrevTexCoordsVec4;
    vec4 lShearParamsVec4 = float2vec4( 0.0 );
    float lShearMotionLength = 0.0;

    lLocalPositionVec4   = IN( mkLocalPositionVec4 );
    lPrevLocalPositionVec4 = lLocalPositionVec4;
    lLocalNormalVec3     = fix_normal(IN( mkLocalNormalVec4 ).xyz);
    
    #ifdef D_DECLARE_TANGENT
    {
        lLocalTangentVec3 = normalize( IN( mkTangentVec4 ).xyz );
    }
    #endif

    #ifdef D_INSTANCE
        sPerInstanceData lInstanceData = GETBUFFERDATA(lUniforms.mpCommonPerMesh, gaPerInstanceData, instanceID);

        lWorldMat4[ 0 ]        = lInstanceData.mkTransformMat4R0;
        lWorldMat4[ 1 ]        = lInstanceData.mkTransformMat4R1;
        lWorldMat4[ 2 ]        = lInstanceData.mkTransformMat4R2;
        lWorldMat4[ 3 ]        = lInstanceData.mkTransformMat4R3;

        OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).x  = lWorldMat4[ 0 ].w;  // + lUniforms.mpCommonPerMesh.gfFadeValue

        #ifdef D_SKINNING_UNIFORMS
            vec4 lExtraParams = lInstanceData.mkTransformMat4R4;

        vec3 lWindEigens0   = vec3(lWorldMat4[ 1 ].w, lWorldMat4[ 2 ].w, lWorldMat4[ 3 ].w );
        vec3 lWindEigens1   = lExtraParams.xyz;
        #endif

        lShearParamsVec4 = lInstanceData.mkTransformMat4R5;

        lWorldMat4[ 0 ].w   = 0.0; // this matrix element is the instance fade value (PS4: may need to assign after the transpose-line)
        lWorldMat4[ 1 ].w   = 0.0; // this matrix element is one of the wind eigenvalue cosines
        lWorldMat4[ 2 ].w   = 0.0; // this matrix element is one of the wind eigenvalue cosines
        lWorldMat4[ 3 ].w   = 1.0; // this matrix element is one of the wind eigenvalue cosines

        lWorldMat4          = PLATFORM_TRANSPOSE( lWorldMat4 );
        lPrevWorldMat4      = lWorldMat4;

      #if defined( D_OUTPUT_MOTION_VECTORS )
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
		{
        #if !defined( D_SKINNING_UNIFORMS )
        lPrevWorldMat4[ 1 ].xyz -= lShearParamsVec4.x * lWorldMat4[ 0 ].xyz;
        lPrevWorldMat4[ 1 ].xyz -= lShearParamsVec4.y * lWorldMat4[ 2 ].xyz;

        lPrevWorldMat4[ 1 ].xyz += lShearParamsVec4.z * lWorldMat4[ 0 ].xyz;
        lPrevWorldMat4[ 1 ].xyz += lShearParamsVec4.w * lWorldMat4[ 2 ].xyz;

        lShearMotionLength = lengthSquared( lShearParamsVec4.xy - lShearParamsVec4.zw );
        #else
        lShearMotionLength = max( lengthSquared( lShearParamsVec4.xy ), lengthSquared( lShearParamsVec4.zw ) );
        #endif
		}
      #endif

        vec4 lLodParams = lInstanceData.mkTransformMat4R4;
        OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).y = lLodParams.w;

    #else
        OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).y = 0.0;

        #ifdef D_FADE
            OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).x = lUniforms.mpCommonPerMesh.gfFadeValue;
        #else
            OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).x = 1.0;
        #endif

        lWorldMat4 = lUniforms.mpCommonPerMesh.gWorldMat4;

        #ifdef D_OUTPUT_MOTION_VECTORS 
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            lPrevWorldMat4 = lUniforms.mpCommonPerMesh.gWorldMotionMat4;
        }
        #ifdef D_PLATFORM_METAL
        else
        {
            lPrevWorldMat4 = lWorldMat4;
        }
        #endif
        #else
            lPrevWorldMat4 = lWorldMat4;
        #endif
    #endif    
  
    
    //-----------------------------------------------------------------------------
    ///
    ///     Skinning
    ///
    //-----------------------------------------------------------------------------

    #ifdef D_SKINNING_UNIFORMS
        #ifdef D_INSTANCE
        {
            vec4 lShearValues = lInstanceData.mkTransformMat4R5;

            lLocalPositionVec4.xyz = GetLdsWindPosition( DEREF_PTR( lUniforms.mpCommonPerMesh ), lLocalPositionVec4.xyz, IN( mkWeightsVec4 ), IN( mkJointsVec4 ), lWindEigens0, lWindEigens1, lShearValues );
        }
        #else
        {
            mat4 lSkinningMat4;
            mat4 lPrevSkinningMat4;
            mat3 lSkinningMat3;
            vec4 lSkinnedPosition;
 
            lSkinningMat4 = CalcSkinningMat( DEREF_PTR( lUniforms.mpCommonPerMesh ), IN( mkWeightsVec4 ), IN( mkJointsVec4 ) );
            lSkinningMat3 = GetSkinningMatVec( lSkinningMat4 );
 
            lSkinnedPosition   = GetSkinPosition( lLocalPositionVec4, lSkinningMat4 );
            lLocalNormalVec3   = GetSkinVector( lLocalNormalVec3,   lSkinningMat3 );
        
            #ifdef D_OUTPUT_MOTION_VECTORS 
            #ifdef D_PLATFORM_METAL
            if(HAS_MOTION_VECTORS)
            #endif
            {
                lPrevSkinningMat4 = CalcPrevSkinningMat( DEREF_PTR( lUniforms.mpCommonPerMesh ), IN( mkWeightsVec4 ), IN( mkJointsVec4 ) );
                lPrevLocalPositionVec4 = GetSkinPosition( lLocalPositionVec4, lPrevSkinningMat4 );
            }
            #ifdef D_PLATFORM_METAL
            else
            {
                lPrevLocalPositionVec4 = lSkinnedPosition;
            }
            #endif
            #else
            lPrevLocalPositionVec4 = lSkinnedPosition;
            #endif

            lLocalPositionVec4 = lSkinnedPosition;

            #ifdef D_DECLARE_TANGENT
            {
                lLocalTangentVec3 = GetSkinVector( lLocalTangentVec3, lSkinningMat3 );
            }
            #endif
        }
        #endif
    #endif


    //-----------------------------------------------------------------------------
    ///
    ///     Vertex Displacement
    ///
    //-----------------------------------------------------------------------------

    vec3 lLocalPositionNoDisplace     = lLocalPositionVec4.xyz;
    vec3 lPrevLocalPositionNoDisplace = lPrevLocalPositionVec4.xyz;
    vec3 lLocalNormalNoDisplace       = lLocalNormalVec3;

    vec3 lLocalPositionWithDisplace     = lLocalPositionVec4.xyz;
    vec3 lPrevLocalPositionWithDisplace = lPrevLocalPositionVec4.xyz;
    vec3 lLocalNormalWithDisplace       = lLocalNormalVec3;

    #ifdef _F31_DISPLACEMENT
    {
        //vec3 lDisplaceBasePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[3].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lDisplaceBasePositionVec3 = lWorldMat4[3].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

        DisplaceVertexAndNormal(	
            lLocalPositionVec4,
            lLocalNormalVec3,
            lLocalTangentVec3, 
            lUniforms.mpPerFrame.gfTime + ( lUniforms.mpCustomPerMaterial.gWaveOneAmpAndPosOffsetVec4.w * ( lDisplaceBasePositionVec3.x + lDisplaceBasePositionVec3.y + lDisplaceBasePositionVec3.z ) ),
            DEREF_PTR( lUniforms.mpCustomPerMaterial ) );

        lLocalPositionWithDisplace     = lLocalPositionVec4.xyz;
        lLocalNormalWithDisplace       = lLocalNormalVec3;

        #ifdef D_OUTPUT_MOTION_VECTORS
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            mat3   laJacobianMat3[2];
            vec3   laNewVertexVec3[2];
            vec3   laFallOffRecipVec3[2];
            float lafFallOffScale[2];
            
            DisplaceVertex( lPrevLocalPositionVec4,
                            laNewVertexVec3,
                            laFallOffRecipVec3,
                            lafFallOffScale,
                            lUniforms.mpPerFrame.gfPrevTime + ( lUniforms.mpCustomPerMaterial.gWaveOneAmpAndPosOffsetVec4.w * (lDisplaceBasePositionVec3.x + lDisplaceBasePositionVec3.y + lDisplaceBasePositionVec3.z)),
                            DEREF_PTR( lUniforms.mpCustomPerMaterial ) );

            lPrevLocalPositionWithDisplace = lPrevLocalPositionVec4.xyz;
        }
        #endif
    }

    #ifdef D_DISREGARD_DISPLACEMENT
     lLocalPositionVec4.xyz     = lLocalPositionNoDisplace;
     lPrevLocalPositionVec4.xyz = lPrevLocalPositionNoDisplace;
     lLocalNormalVec3           = lLocalNormalNoDisplace;
    #endif

    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     ShaderMill
    ///
    //-----------------------------------------------------------------------------
    #ifdef SM_CODE
    {
        float lfSkGlobalTime = lUniforms.mpPerFrame.gfTime;
        vec4 lSkUserData = lUniforms.mpCommonPerMesh.gUserDataVec4;

        #if defined( _F21_VERTEXCOLOUR ) || defined( _F29_VBCOLOUR )
        vec4 lSkVertexColour = IN(mkColourVec4);
        #else
        vec4 lSkVertexColour = float2vec4(0.0);
        #endif

        vec4 lSkFeaturesVec4 = float2vec4(0.0);

        vec3 lSkLocalPositionVec3 = lLocalPositionVec4.xyz;
        vec3 lSkWorldPositionVec3 = lWorldPositionVec4.xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lSkNodePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

        vec3 lSkPositionDisplacement = lLocalPositionWithDisplace - lLocalPositionNoDisplace;
        vec3 lSkNormalDisplacement   = lLocalNormalWithDisplace - lLocalNormalNoDisplace;

        vec3 lSkPlanetPositionVec3 = float2vec3(0.0);
        vec3 lSkSunPositionVec3    = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz * 100000.0;
        vec2 lSkBiomeDataVec2      = lUniforms.mpCustomPerMaterial.gBiomeDataVec4.xy;

        vec3 lfOutLocalPosition = lSkLocalPositionVec3;
        vec3 lvOutShellsOffset  = float2vec3(0.0);
        vec4 lfOutColour        = float2vec4(0.0);
        vec3 lOutNormal         = float2vec3(0.0);
        float lfOutMetallic     = 0.0;
        float lfOutRoughness    = 0.0;
        float lfOutSubsurface   = 0.0;
        float lfOutGlow         = 0.0;
        float lfShellHeight     = 0.0;

        vec4 lColourVec4   = float2vec4(0.0);
        vec3 lSkInNormal   = lLocalNormalVec3;
        vec3 lSkInWorldNormal = lLocalNormalVec3;                
        #ifdef D_INSTANCE
            lSkInWorldNormal   = CalcWorldVecInstanced( lUniforms.mpCommonPerMesh.gWorldNormalMat4, lWorldMat4, lLocalNormalVec3 );
        #else
            lSkInWorldNormal   = CalcWorldVec( lUniforms.mpCommonPerMesh.gWorldNormalMat4, lLocalNormalVec3 );
        #endif  
            
        vec3 lSkInWorldNormal1 = lSkInWorldNormal;     
        vec3 lSkInWorldNormal2 = lSkInWorldNormal;   
        
        vec3 lSkCameraPositionVec3 = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lSkCameraDirectionVec3 = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 );
        float lfMetallic   = 0.0;
        float lfRoughness  = 0.0;
        float lfSubsurface = 0.0;
        float lfGlow       = 0.0;

        vec2 lSkUv1 = lTexCoordsVec4.xy;
        vec2 lSkUv2 = lTexCoordsVec4.zw;
        vec2 lSkUvScreen = vec2( 0.0, 0.0 );

        {
            SM_CODE
        }

        #if defined ( D_SK_USE_LOCAL_POSITION )
            lLocalPositionVec4.xyz = lfOutLocalPosition;
            OUT(mLocalPositionVec4) = lLocalPositionVec4;
        #endif

        #ifdef D_OUTPUT_MOTION_VECTORS
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            lSkPositionDisplacement = lPrevLocalPositionWithDisplace - lPrevLocalPositionNoDisplace;
            lSkLocalPositionVec3 = lPrevLocalPositionVec4.xyz;
            lSkWorldPositionVec3 = lPrevWorldPositionVec4.xyz;
            lSkNodePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMotionMat4[3].xyz;
            lfOutLocalPosition = lSkLocalPositionVec3;
            lfSkGlobalTime = lUniforms.mpPerFrame.gfPrevTime;
            {
                SM_CODE
                lPrevLocalPositionVec4.xyz = lfOutLocalPosition;
            }
        }
        #endif
    }
    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     Billboarding
    ///
    //-----------------------------------------------------------------------------
    #ifdef _F19_BILLBOARD   

    {
#if defined( _F44_IMPOSTER ) 
        bool lbOctahedralImpostersEnabled = OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4);
#endif
        CalcBillboardValues( 
            lUniforms.mpPerFrame.gCameraNoHeadTrackingMat4,
            lUniforms.mpCommonPerMesh.gWorldMat4,
            lLocalPositionVec4, 
            lUniforms.mpCustomPerMesh.gCustomParams01Vec4,
            lWorldMat4, 
            IN( mkTexCoordsVec4 ),
            lShearParamsVec4,
#if defined( _F44_IMPOSTER ) 
            lbOctahedralImpostersEnabled,
            lUniforms.mpCustomPerMaterial.gOctahedralImposterDataVec4.z,
#endif
            lLocalPositionVec4, 
            lWorldPositionVec4, 
            lLocalNormalVec3, 
            lWorldNormalVec3 );

        #ifdef D_OUTPUT_MOTION_VECTORS
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            vec3 lDummyNormal1;
            vec3 lDummyNormal2;

            if( lShearMotionLength > 0.0 )
            {
                CalcBillboardValues( 
                    lUniforms.mpPerFrame.gCameraNoHeadTrackingMat4,
                    lUniforms.mpCommonPerMesh.gWorldMotionMat4,
                    lPrevLocalPositionVec4, 
                    lUniforms.mpCustomPerMesh.gCustomParams01Vec4,
                    lPrevWorldMat4, 
                    IN( mkTexCoordsVec4 ),
                    lShearParamsVec4.zwxy,
#if defined( _F44_IMPOSTER ) 
                    lbOctahedralImpostersEnabled,
                    lUniforms.mpCustomPerMaterial.gOctahedralImposterDataVec4.z,
#endif
                    lPrevLocalPositionVec4, 
                    lPrevWorldPositionVec4, 
                    lDummyNormal1, 
                    lDummyNormal2 );
            }
            else
            {
                lPrevLocalPositionVec4 = lLocalPositionVec4;
                lPrevWorldPositionVec4 = lWorldPositionVec4;
            }
        }
        #endif
    }
    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     World Transform
    ///
    //-----------------------------------------------------------------------------
    #ifdef D_USES_WORLD_POSITION
        OUT( mWorldPositionVec3_mfSpare ) = vec4(0.0, 0.0, 0.0, 0.0);
    #endif

    #if 0//defined(_F15_WIND) && defined(_F21_VERTEXCOLOUR)


        vec3 lInstancePosition = MAT4_GET_COLUMN( lWorldMat4, 3 ).xyz;
        vec4 lWindParamsVec4   = lUniforms.mpCommonPerMesh.gWindParams1Vec4;

        const float kfAnimationOffsetMax = 5.0;
        float lfAnimationOffset = lWindParamsVec4.w * kfAnimationOffsetMax;



        
    #endif

    #ifndef _F19_BILLBOARD
    {
        #ifdef D_INSTANCE

            lWorldPositionVec4 = CalcWorldPosInstanced( lUniforms.mpCommonPerMesh.gWorldMat4, lWorldMat4, lLocalPositionVec4 );
            lWorldNormalVec3   = normalize( CalcWorldVecInstanced( lUniforms.mpCommonPerMesh.gWorldNormalMat4, lWorldMat4, lLocalNormalVec3 ) );       
            

            // no point in computing this on skinned props, as for those the previous transform is just set to whatever the current one is
            #if defined( D_OUTPUT_MOTION_VECTORS ) && !defined( D_SKINNING_UNIFORMS )
            #ifdef D_PLATFORM_METAL
            if(HAS_MOTION_VECTORS)
            #endif
            {
                #if defined( _F31_DISPLACEMENT )
                lPrevWorldPositionVec4 = CalcWorldPosInstanced( lUniforms.mpCommonPerMesh.gWorldMat4, lPrevWorldMat4, lPrevLocalPositionVec4 );
                #else
                lPrevWorldPositionVec4 = lShearMotionLength > 0.0 ?
                                            CalcWorldPosInstanced( lUniforms.mpCommonPerMesh.gWorldMat4, lPrevWorldMat4, lPrevLocalPositionVec4 ) :
                                            lWorldPositionVec4;
                #endif
            }
            #ifdef D_PLATFORM_METAL
            else
            {
                lPrevWorldPositionVec4 = lWorldPositionVec4;
            }
            #endif
            #else
            lPrevWorldPositionVec4 = lWorldPositionVec4;
            #endif
        #else
            lWorldPositionVec4 = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, lLocalPositionVec4 );
            lWorldNormalVec3   = normalize( CalcWorldVec( lUniforms.mpCommonPerMesh.gWorldNormalMat4, lLocalNormalVec3 ) );

            #if defined( D_OUTPUT_MOTION_VECTORS )
            #ifdef D_PLATFORM_METAL
            if(HAS_MOTION_VECTORS)
            #endif
            {
                lPrevWorldPositionVec4 = CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMotionMat4, lPrevLocalPositionVec4 );
            }
            #ifdef D_PLATFORM_METAL
            else
            {
                lPrevWorldPositionVec4 = lWorldPositionVec4;
            }
            #endif
            #else
            lPrevWorldPositionVec4 = lWorldPositionVec4;
            #endif
        #endif
    }
    #endif

        

    #if 0// !defined(_F19_BILLBOARD) && defined(_F15_WIND) && defined(_F21_VERTEXCOLOUR)
    {
        lWorldPositionVec4.xyz = DoVertexWind(                                         
            lWindParamsVec4, 
            vec4( lInstancePosition, 0.0 ), 
            lWorldPositionVec4,
            lLocalPositionVec4, 
            vec4( lWorldNormalVec3, 0.0 ), 
            IN( mkColourVec4 ), 
            lUniforms.mpPerFrame.gfTime,
            DEREF_PTR( lUniforms.mpCommonPerMesh ) ).xyz;

        #if defined( D_OUTPUT_MOTION_VECTORS ) && !defined( D_SKINNING_UNIFORMS )        
        {
            lPrevWorldPositionVec4.xyz = DoVertexWind(
                lWindParamsVec4,
                vec4(lInstancePosition, 0.0),
                lPrevWorldPositionVec4,
                lPrevLocalPositionVec4,
                vec4(lWorldNormalVec3, 0.0),
                IN(mkColourVec4),
                lUniforms.mpPerFrame.gfPrevTime,
                DEREF_PTR(lUniforms.mpCommonPerMesh)).xyz;
            /*
            #ifdef D_INSTANCE
            lWorldPositionVec4 = CalcWorldPosInstanced(lUniforms.mpCommonPerMesh.gWorldMat4, lWorldMat4, lWorldPositionVec4);
            lPrevWorldPositionVec4 = CalcWorldPosInstanced(lUniforms.mpCommonPerMesh.gWorldMat4, lPrevWorldMat4, lPrevWorldPositionVec4);
            #else
            lPrevWorldPositionVec4 = CalcWorldPos(lUniforms.mpCommonPerMesh.gWorldMotionMat4, lPrevLocalPositionVec4);
            #endif
            */

        }
        #endif
        
    }
    #endif

    #if defined( _F36_DOUBLESIDED ) && defined( D_USES_WORLD_POSITION ) && !defined( D_DEPTHONLY )
    {
        #ifdef D_NORMALMAPPED
            OUT( mWorldPositionVec3_mfSpare ).w = IN( mkTangentVec4 ).w > 0.01 ? 1.0 : -1.0;
        #endif
    }
    #endif

    #ifdef D_USES_WORLD_POSITION
    {
        OUT( mWorldPositionVec3_mfSpare ).xyz = lWorldPositionVec4.xyz;
    }
    #endif


    /*
    Nothing uses this. All wind is currently done with biome settings (cpu instance code) or vertex displacment.
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
    */
    //-----------------------------------------------------------------------------
    ///
    ///     Planet Curvature
    ///
    //-----------------------------------------------------------------------------
    #if !defined( _F38_NO_DEFORM ) && defined( D_USES_WORLD_POSITION )
    {
        OUT( mWorldPositionVec3_mfSpare ).xyz = lWorldPositionVec4.xyz;
    }
    #endif
     
    #if !defined( D_DEPTH_CLEAR )

    #if !defined( D_DEPTHONLY ) 

        #ifdef D_USES_VERTEX_NORMAL
        {
            OUT( mTangentSpaceNormalVec3 ) = lWorldNormalVec3;
        }
        #endif
        //-----------------------------------------------------------------------------
        ///
        ///     Foliage
        ///
        //-----------------------------------------------------------------------------
        #if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND )
            //OUT( mCenteralNormalVec3 ) = CalcWorldVec( vec3( 0.0, 1.0, 0.0 ) );
            //OUT( mCenterPositionVec3 ) = MAT4_GET_COLUMN( lWorldMat4, 3 ).xyz;
            OUT( mCenteralNormalVec3 ) = normalize( MAT4_GET_COLUMN( lWorldMat4, 1 ).xyz );
        #endif

        #if defined( D_DEFERRED_DECAL ) && defined( D_FORWARD_RENDERER )
            OUT( mCenteralNormalVec3 ) = normalize( MAT4_GET_COLUMN( lWorldMat4, 2 ).xyz );
        #endif
   
        //-----------------------------------------------------------------------------
        ///
        ///     Normal Mapping
        ///
        //-----------------------------------------------------------------------------

    #ifdef D_NORMALMAPPED
    	{
        vec3 lWorldTangentVec3;
        vec3 lWorldBitangentVec3;
        vec3 lWorldTangentNormalVec3;

            #if defined( _F19_BILLBOARD )
            {
                lWorldTangentVec3       =  MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 0 );
                lWorldBitangentVec3     = -MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 1 );        
                lWorldTangentNormalVec3 =  MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 );

                #ifdef D_IMPOSTER
                    lWorldTangentNormalVec3 = -lWorldTangentNormalVec3;
                #endif

            }
            #else
            {
                #ifdef D_INSTANCE
                    lWorldTangentVec3   = CalcWorldVecInstanced( lUniforms.mpCommonPerMesh.gWorldNormalMat4, lWorldMat4, lLocalTangentVec3 );
                #else
                    lWorldTangentVec3   = CalcWorldVec( lUniforms.mpCommonPerMesh.gWorldNormalMat4, lLocalTangentVec3 );
                #endif  
          
                // This is a bit of a hack. Can't figure out why W comes through as unorm on ps4 (so 1 comes out as 0.333). This gets around it :S
                float lfHandedness = IN( mkTangentVec4 ).w > 0.01 ? 1.0 : -1.0;
                lWorldBitangentVec3 = cross( lWorldNormalVec3, lWorldTangentVec3 ) * lfHandedness;

                // normalize tangent frame to achieve scale-independent results of normal mapping
                lWorldTangentNormalVec3 = lWorldNormalVec3;
                lWorldTangentVec3       = lWorldTangentVec3;
                lWorldBitangentVec3     = lWorldBitangentVec3;
            }
            #endif
                
#if defined( _F56_MATCH_GROUND ) && defined( _F36_DOUBLESIDED ) && defined( D_USES_VERTEX_NORMAL ) && defined( D_PLATFORM_SWITCH )
            //mat3 lTangentSpaceMat;

            //lTangentSpaceMat[0] = normalize(lWorldTangentVec3);
            //lTangentSpaceMat[1] = normalize(lWorldBitangentVec3);
            //lTangentSpaceMat[2] = normalize(lWorldTangentNormalVec3);
            OUT(mTangentSpaceNormalVec3) = lWorldNormalVec3;
#else
            OUT( mTangentMatRow1Vec3 ) = normalize(lWorldTangentVec3);
            OUT( mTangentMatRow2Vec3 ) = normalize(lWorldBitangentVec3);
            OUT( mTangentMatRow3Vec3 ) = normalize(lWorldTangentNormalVec3);
            
#endif
            
            #ifdef _F20_PARALLAXMAP
            {
                // figure out how to reorder stuff so parallax comes after this which happens further down(or this happens before parallax)
                lScreenSpacePositionVec4 = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );

                vec3 T   = normalize(lWorldTangentVec3);
                vec3 B   = normalize(lWorldBitangentVec3);
                vec3 N   = normalize(lWorldTangentNormalVec3);
                mat3 TBN = transpose(mat3(T, B, N));
                
                //
                // Get light direction in tangent space for self shadowing in fragment shader.
                //
                vec3 lLightDirectionTangentSpaceVec3 = normalize( lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz );
                lLightDirectionTangentSpaceVec3 = MUL( TBN, lLightDirectionTangentSpaceVec3 );
                OUT( mTangentSpaceLightDirVec3 ) = lLightDirectionTangentSpaceVec3;
                
//                OUT( mTangentSpaceViewPosVec3 )     = MUL( TBN, vec3(lUniforms.mpPerFrame.gViewPositionVec3) );
//                OUT( mTangentSpaceFragPosVec3 )     = MUL( TBN, vec3(lWorldPositionVec4 ) );
                
                vec3 lWorldSpaceView = normalize( lWorldPositionVec4.xyz - lUniforms.mpPerFrame.gViewPositionVec3 );
                OUT( mTangentSpaceEyeVec3 )     = MUL( TBN, lWorldSpaceView );                
            }
            #endif
        
        }
        #endif
       
        #if !defined( D_DEFER ) && !defined( _F07_UNLIT ) && !defined( D_DEPTHONLY ) && !defined( D_PLATFORM_METAL )
            mat3 lUpMatrix;
            MAT3_SET_COLUMN( lUpMatrix, 0, lUniforms.mpPerFrame.gInverseWorldUpMatVec4[ 0 ].xyz );
            MAT3_SET_COLUMN( lUpMatrix, 1, lUniforms.mpPerFrame.gInverseWorldUpMatVec4[ 1 ].xyz );
            MAT3_SET_COLUMN( lUpMatrix, 2, lUniforms.mpPerFrame.gInverseWorldUpMatVec4[ 2 ].xyz );
            OUT( mUpMatrixMat3 ) = lUpMatrix;
        #endif


    #endif // !defined( D_DEPTHONLY )
            

            //-----------------------------------------------------------------------------
            ///
            ///     Vertex Colour
            ///
            //-----------------------------------------------------------------------------
        #if defined ( _F21_VERTEXCOLOUR ) && defined ( D_DEPTHONLY )
            {
                OUT( mWorldPositionVec3_mfSpare ).w = IN( mkColourVec4 ).w;
            }
        #elif defined ( _F21_VERTEXCOLOUR ) || defined( _F45_VERTEX_BLEND )
            {
                OUT(mColourVec4) = IN(mkColourVec4);
            }
        #elif defined ( _F33_SHELLS ) && !defined ( D_DEPTHONLY )
            {
                OUT(mColourVec4) = vec4(1.0,1.0,1.0,1.0);
            }
        #endif

            //-----------------------------------------------------------------------------
            ///
            ///     Material Colour
            ///
            ////-----------------------------------------------------------------------------

        #if !defined( _F01_DIFFUSEMAP )
            {
                OUT(mMaterialVec4) = vec4(GammaCorrectInput(lUniforms.mpCustomPerMaterial.gMaterialColourVec4.xyz), lUniforms.mpCustomPerMaterial.gMaterialColourVec4.a);
            }
        #endif

    #endif      // !defined( D_DEPTH_CLEAR )

    //-----------------------------------------------------------------------------
    ///
    ///     Texturing
    ///
    //-----------------------------------------------------------------------------
    {
        #ifdef _F16_DIFFUSE2MAP
            lTexCoordsVec4.zw = IN(mkTexCoordsVec4).zw;
        #else
            lTexCoordsVec4.zw = lTexCoordsVec4.xy;
        #endif

        vec4 lFlippedScrollingUVVec4;

        #ifdef _F13_UVANIMATION
        {
            lFlippedScrollingUVVec4 = lUniforms.mpCustomPerMesh.gUVScrollStepVec4;

			float lfTime	= lUniforms.mpPerFrame.gfTime;
			float lfStepMul = min( int( ( 1.0f / lFlippedScrollingUVVec4.x ) * ( 1.0f / lFlippedScrollingUVVec4.y ) ), 10000.0f );			
			float lfFrameNumber = ( sign( lFlippedScrollingUVVec4.w ) * floor( lFlippedScrollingUVVec4.w ) ) + ( ( 1.0f - sign( lFlippedScrollingUVVec4.w ) ) * mod( floor( lfTime * lFlippedScrollingUVVec4.z ), lfStepMul ) );

            lTexCoordsVec4.x += fract( lfFrameNumber * lFlippedScrollingUVVec4.x );
            lTexCoordsVec4.y += fract( floor( lfFrameNumber * lFlippedScrollingUVVec4.x ) * lFlippedScrollingUVVec4.y );

            lTexCoordsVec4.zw = lTexCoordsVec4.xy;
        }
        #elif defined( _F14_UVSCROLL )
        {
            lFlippedScrollingUVVec4 = lUniforms.mpCustomPerMesh.gUVScrollStepVec4;

            #if defined( D_OUTPUT_MOTION_VECTORS )
            #ifdef D_PLATFORM_METAL
            if(HAS_MOTION_VECTORS)
            #endif
            {
                lPrevTexCoordsVec4 = lTexCoordsVec4 + lFlippedScrollingUVVec4 * lUniforms.mpPerFrame.gfPrevTime;
            }
            #endif

            lTexCoordsVec4 += lFlippedScrollingUVVec4 * lUniforms.mpPerFrame.gfTime;
        }
        #elif defined( _F18_UVTILES )
        {
            //bodge to make tiling based on position work for objects that are scaled differently without relying on world position
            vec3 lWorldPosVec3 = normalize( lWorldMat4[3].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) * lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;

            
            #ifdef _F12_BATCHED_BILLBOARD
                lWorldPosVec3.xz += IN( mkLocalPositionVec4 ).xz -  vec2( lTexCoordsVec4.x - 0.5,  0.0 );
                lWorldPosVec3.xz /= 8.0;
            #endif

            // Divide the x pos down because in batched billboards this has some slight inaccuracies
            float lfPos = ( lWorldPosVec3.x + lWorldPosVec3.y + lWorldPosVec3.z ) * 10.0;

            lTexCoordsVec4.x /= lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;
            lTexCoordsVec4.y /= lUniforms.mpCustomPerMesh.gUVScrollStepVec4.w;
            lTexCoordsVec4.x += floor( mod( lfPos, lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z ) ) / lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;

            lTexCoordsVec4.y += floor( mod( lfPos, lUniforms.mpCustomPerMesh.gUVScrollStepVec4.w ) ) / lUniforms.mpCustomPerMesh.gUVScrollStepVec4.w;
            

        }        
        #endif

        #if defined( _F44_IMPOSTER )
        {
            vec3 lImposterAt = MAT4_GET_COLUMN(lWorldMat4, 2);
#if ENABLE_OCTAHEDRAL_IMPOSTERS
            if(OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
            {
                mat4 lInverseWorldMat4 = Inverse(lWorldMat4);
                vec3 lViewPositionVec3 = MAT4_GET_COLUMN(lUniforms.mpPerFrame.gCameraMat4, 3);
                vec3 lLocalCameraPosVec3 = MUL(lInverseWorldMat4, vec4(lViewPositionVec3, 1.0)).xyz;

                // For some reason using the local position here causes issues with octohedral imposters, so we
                // have to calculate it again from the inverse world transform matrix, not sure why though.
                // - Tom Read Cutting
                //vec3 lProperLocalPosVec3 = lLocalPositionVec4.xyz;
                vec3 lProperLocalPosVec3 = MUL(lInverseWorldMat4, lWorldPositionVec4).xyz;
                vec2 lGridCoordinatePos;
                vec4 lBlendWeights;
                vec3 lNormal;
                bool lbFrameBlendEnabled;
                bool lbDepthReprojectionEnabled;
                vec2 lTexCoords0;
                vec2 lTexCoords1;
                vec2 lTexCoords2;
                vec2 lImposterFrame0;
                vec2 lImposterFrame1;
                vec2 lImposterFrame2;
                vec2 lFrameProjection0;
                vec2 lFrameProjection1;
                vec2 lFrameProjection2;
                ImposterVertex(
                    // Inputs
                    lWorldPositionVec4, lProperLocalPosVec3,
                    lViewPositionVec3, lLocalCameraPosVec3, lImposterAt, lTexCoordsVec4.xy, lUniforms.mpCustomPerMaterial.gImposterDataVec4, lUniforms.mpCustomPerMaterial.gOctahedralImposterDataVec4, lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4,
                    // Outputs
                    lGridCoordinatePos, lNormal, lBlendWeights, lbFrameBlendEnabled, lbDepthReprojectionEnabled,
                    lTexCoords0, lTexCoords1, lTexCoords2,
                    lImposterFrame0, lImposterFrame1, lImposterFrame2,
                    lFrameProjection0, lFrameProjection1, lFrameProjection2
                );
                lTexCoordsVec4 = vec4(lTexCoords0, lTexCoords1);

                OUT(mImposterData0) = vec4(lTexCoords2, lBlendWeights.xy);

                OUT(mImposterFrameXY_FrameProjectonVecZW0) = vec4(lImposterFrame0, lFrameProjection0);
                OUT(mImposterFrameXY_FrameProjectonVecZW1) = vec4(lImposterFrame1, lFrameProjection1);
                OUT(mImposterFrameXY_FrameProjectonVecZW2) = vec4(lImposterFrame2, lFrameProjection2);

    #ifdef D_INSTANCE
                {
                    mat3 lWorldNormalMat3 = CalculateWorldInstancedNormalMat( lWorldMat4 );
                    lNormal = MUL( lUniforms.mpCommonPerMesh.gWorldNormalMat4, vec4( lNormal, 1.0 ) ).xyz;
                    lNormal = MUL( lWorldNormalMat3, lNormal );

                    #if !defined ( D_PLATFORM_METAL )
                    OUT( mWorldNormalMat3 ) = lWorldNormalMat3;
                    #endif
                }
    #else
                lNormal = CalcWorldVec(lUniforms.mpCommonPerMesh.gWorldNormalMat4, lNormal);
    #endif  
                float lfScale = sqrt(lWorldMat4[0][0] * lWorldMat4[0][0] + lWorldMat4[0][1] * lWorldMat4[0][1] + lWorldMat4[0][2] * lWorldMat4[0][2]);
                lfScale = 1.0;

                // encode a bit of information in the sign of each component of the normal vector
                lNormal.x = EncodeBitInNormalFloat( lNormal.x, lbFrameBlendEnabled );
                lNormal.y = EncodeBitInNormalFloat( lNormal.y, lbDepthReprojectionEnabled );

                OUT(mImposterViewNormal) = vec4(lNormal, lfScale);

                lScreenSpacePositionVec4 = CalcScreenPosFromWorld(lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4);
#ifdef D_INSTANCE
                lScreenSpacePositionVec4.z = CalcScreenPosFromLocal( lUniforms.mpPerFrame.gViewProjectionMat4, CalcWorldPosInstanced( lUniforms.mpCommonPerMesh.gWorldMat4, lWorldMat4, vec4( 0.0,0.0,0.0,1.0 ) ) ).z;
#else
                lScreenSpacePositionVec4.z = CalcScreenPosFromLocal( lUniforms.mpPerFrame.gViewProjectionMat4, CalcWorldPos( lUniforms.mpCommonPerMesh.gWorldMat4, vec4( 0.0,0.0,0.0,1.0 ) ) ).z;
#endif
            }
            else
#endif
            {
            //vec3 lWindingAxis   = GetWorldUp( lWorldPositionVec4.xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4 );
            float lfHeight;
            vec3 lWindingAxis = GetWorldUp(lWorldPositionVec4.xyz, lUniforms.mpCommonPerMesh.gPlanetPositionVec4, lfHeight);


            lTexCoordsVec4.x = CalculateImposterTexCoordsU( MAT4_GET_COLUMN( lWorldMat4, 3 ), MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 3 ), lImposterAt, lWindingAxis, lTexCoordsVec4.xy, lUniforms.mpCustomPerMaterial.gImposterDataVec4 );
            lTexCoordsVec4.xy = SCREENSPACE_AS_RENDERTARGET_UVS(lTexCoordsVec4.xy);
        }
        }
        #endif
        
        //#ifdef D_TEXCOORDS
            OUT( mTexCoordsVec4 ) = lTexCoordsVec4;
            #if 0 //defined( D_OUTPUT_MOTION_VECTORS ) && defined( _F14_UVSCROLL )
                OUT( mPrevTexCoordsVec4 ) = lPrevTexCoordsVec4;
            #endif
        //#endif
    }



    //-----------------------------------------------------------------------------
    ///
    ///     Screen Space Transform
    ///
    //-----------------------------------------------------------------------------
    {
        #if defined( _F44_IMPOSTER )
        if(!OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
        #endif
        {
        lScreenSpacePositionVec4 = CalcScreenPosFromWorld( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );
        }
        #ifdef D_REMOVE_TAA_JITTER
            lScreenSpacePositionVec4.xy -= lUniforms.mpPerFrame.gDeJitterVec3.xy * lScreenSpacePositionVec4.w * 2.0f;
        #endif

        #if defined ( D_USE_SCREEN_POSITION )
            OUT( mScreenSpacePositionVec4 ) = lScreenSpacePositionVec4;
        #endif

        #ifdef D_OUTPUT_MOTION_VECTORS
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            OUT( mPrevScreenPosition ) =  CalcScreenPosFromWorld( lUniforms.mpPerFrame.gPrevViewProjectionMat4, lPrevWorldPositionVec4 );
        }
        #endif

        #if defined( D_OUTPUT_MOTION_VECTORS ) && defined( D_INSTANCE )
        #ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
        #endif
        {
            OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).z = lShearMotionLength;
        }
        #ifdef D_PLATFORM_METAL
        else
        {
            OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).z = 0.0;
        }
        #endif
        #else
            OUT( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).z = 0.0;
        #endif

        #if defined( D_IMPOSTER_VERTEX )

            lScreenSpacePositionVec4.z = max(lScreenSpacePositionVec4.z, -1.0);

            #ifdef D_ENABLE_REVERSEZ_PROJECTION
                lScreenSpacePositionVec4.z = 0.5 - (lScreenSpacePositionVec4.z * 0.5);
            #endif

            lScreenSpacePositionVec4.w = 1.0;
            SCREEN_POSITION = lScreenSpacePositionVec4;
            WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
            //OUT( mfLogZ ) = abs( lScreenSpacePositionVec4.z );

        #else

            #if defined( _F34_GLOW )
                lScreenSpacePositionVec4.z = lScreenSpacePositionVec4.z + 0.000001;
            #endif

            //    OUT( mfLogZ ) = lScreenSpacePositionVec4.z;

            SCREEN_POSITION = lScreenSpacePositionVec4;
            WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);

        #endif
    }

}

