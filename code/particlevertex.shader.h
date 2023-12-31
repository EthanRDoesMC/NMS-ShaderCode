////////////////////////////////////////////////////////////////////////////////
///
///     @file       ParticleVertex.h
///     @author     User
///     @date       
///
///     @brief      ParticleVertex
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


#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "UberCommon.h"

#include "Common/Common.shader.h"
#include "Common/CommonParticle.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonDynamicVertex.shader.h"

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

    INPUT( vec4, mkTexCoordsVec4, TEXCOORD0 )

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

    flat OUTPUT( vec2,  mParticleIdX_AlphaMultiplierY,  TEXCOORD0 )
	OUTPUT( vec4,   mTexCoordsVec4,           TEXCOORD1 )
    OUTPUT( vec4,   mScreenSpacePositionVec4, TEXCOORD2 )
#ifdef _F13_UVANIMATION
    OUTPUT( float,  mfAnimBlendValue,         TEXCOORD3)
#endif
#ifndef _F07_UNLIT
	OUTPUT( vec3,   mWorldNormalVec3,         TEXCOORD4 )
#ifdef _F03_NORMALMAP
#if defined(D_PLATFORM_METAL)
    flat OUTPUT(vec3, mTangentSpaceMat3_col0,  TEXCOORD5 )
    flat OUTPUT(vec3, mTangentSpaceMat3_col1,  TEXCOORD6 )
    flat OUTPUT(vec3, mTangentSpaceMat3_col2,  TEXCOORD7 )
#else
    flat OUTPUT(mat3, mTangentSpaceMat3,  TEXCOORD5 )
#endif
#endif
#endif

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END


//-----------------------------------------------------------------------------
//    Functions

mat3
RotationAboutAxis(
    float lRadians, 
    vec3   lVec ) 
{
    float s;
    float c;
    float oneMinusC;
    float xy;
    float yz;
    float zx;

    s = sin( lRadians );
    c = cos( lRadians );

    xy = ( lVec.x * lVec.y );
    yz = ( lVec.y * lVec.z );
    zx = ( lVec.z * lVec.x );
    
    oneMinusC = ( 1.0 - c );

    return mat3(
        vec3( (((lVec.x*lVec.x) * oneMinusC) + c), ((xy*oneMinusC) + (lVec.z*s)),       ((zx*oneMinusC) - (lVec.y*s))),
        vec3( ((xy*oneMinusC) - (lVec.z*s)),       (((lVec.y*lVec.y) * oneMinusC) + c), ((yz*oneMinusC) + (lVec.x*s))),
        vec3( ((zx*oneMinusC) + (lVec.y*s)),       ((yz*oneMinusC) - (lVec.x*s)),       (((lVec.z*lVec.z) * oneMinusC) + c)));
}

vec2
AnimateTexture(
    vec2  lTexCoord,
    vec2  lTexSize,
    float lfFrameNumber )
{
    vec2  lMappedUV;
    float lfFrameY = floor( (lfFrameNumber + 0.5) * lTexSize.x );

    lMappedUV.x = lTexCoord.x       * lTexSize.x + fract( (lfFrameNumber + 0.5) * lTexSize.x ) - lTexSize.x*0.5;
    lMappedUV.y = (1.0-lTexCoord.y) * lTexSize.y + fract( (lfFrameY      + 0.5) * lTexSize.y ) - lTexSize.y*0.5;
    lMappedUV.y = 1.0 - lMappedUV.y;

    return lMappedUV;
}

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Main
///
///     @param      void
///     @return     Fresnel
///
//-----------------------------------------------------------------------------
VERTEX_MAIN_SRT
{
    vec4 lParticlePositionVec4;
    vec4 lParticleViewPositionVec4;
	    
    
    float lfParticleID = IN( mkTexCoordsVec4 ).z;
    int  liParticleID     = int( lfParticleID );
    int  liParticleCorner = int( IN( mkTexCoordsVec4 ).w );

    vec4 lTexCoordsVec4 = IN( mkTexCoordsVec4 ).xyxy;

    vec2  lAnimFrameSize = lUniforms.mpCustomPerMesh.gUVScrollStepVec4.xy;
    float lfAnimSpeed    = lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;

    float lfTimeElapsed = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID ).z;
    float lfFrameNumber = floor( lfTimeElapsed * lfAnimSpeed );
    float lfAnimBlendValue = fract( lfTimeElapsed * lfAnimSpeed );
    bool lbAlphaThresholdIsSet = false;

#if !defined( D_HEAVYAIR )

#ifdef _F03_NORMALMAP
    // We need to make sure we flip the normals to match any flipping of the uv coordinates.
    // So we construct an identity matrix that is post-multiplied into the tangent space transform matrix that we can then use
    // to correctly flip the x/y components of the normal vectors.
    mat3 lNormalFlipMat3 = mat3( vec3( 1.0, 0.0, 0.0 ), vec3( 0.0, 1.0, 0.0 ), vec3( 0.0, 0.0, 1.0 ) );
    bool lNormalsCanBeFlipped = false;
#endif

    // "flip" the uv coordinates based on bitfield settings - the bitfield values
    // have to match what is set in the C++ code in TkParticleNode::SetPerDrawUniforms
    // lpEmitter.mpEmitterData.mUCoordinate.meCoordinateOrientation == ECoordinateOrientation_Random
    if ((lUniforms.mpCommonPerMesh.guParticleSettingBits & 1) != 0 )
    {
#ifdef _F03_NORMALMAP
        lNormalsCanBeFlipped = true;
#endif
        if ( liParticleID % 2 == 0 )
        {
            lTexCoordsVec4.x = 1.0 - lTexCoordsVec4.x;
#ifdef _F03_NORMALMAP
            lNormalFlipMat3[0][0] = -1.0;
#endif
        }
    }

    // lpEmitter.mpEmitterData.mVCoordinate.meCoordinateOrientation == ECoordinateOrientation_Random
    if ((lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 1)) != 0)
    {
#ifdef _F03_NORMALMAP
        lNormalsCanBeFlipped = true;
#endif
        if ( ( liParticleID / 2 ) % 2 == 0)
        {
            lTexCoordsVec4.y = 1.0 - lTexCoordsVec4.y;
#ifdef _F03_NORMALMAP
            lNormalFlipMat3[1][1] = -1.0;
#endif
        }
    }

    // (lpEmitter.mpEmitterData.meFlipbookPlaybackRate == EFlipbookPlaybackRate_OnceToCompletion
    if ((lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 2)) != 0)
    {
        float lfTotalFrames = (1.0 / (lAnimFrameSize.x * lAnimFrameSize.y)) - 1.0;
        lfFrameNumber = floor(lfTimeElapsed * lfTotalFrames);
        lfAnimBlendValue = fract(lfTimeElapsed * lfTotalFrames);
    }

    // (lpEmitter.mpEmitterData.meFlipbookPlaybackRate == EFlipbookPlaybackRate_Random
    if ((lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 3)) != 0)
    {
        float lfTotalFrames = (1.0 / (lAnimFrameSize.x * lAnimFrameSize.y)) - 1.0;
        lfAnimBlendValue = 0.0f;
        lfFrameNumber = floor( lfTimeElapsed * lfTotalFrames);
    }

    uint leBillboardAlignment = uint((lUniforms.mpCommonPerMesh.guParticleSettingBits >> 4) & 7);

    lbAlphaThresholdIsSet = (lUniforms.mpCommonPerMesh.guParticleSettingBits & (1 << 7)) != 0;

#else
    uint leBillboardAlignment = EBillboardAlignment_Screen;
#endif // defined( D_HEAVYAIR )
    
    //-----------------------------------------------------------------------------
    ///
    ///     Texture Coords
    ///
    //-----------------------------------------------------------------------------
    #ifdef _F13_UVANIMATION
    {
        vec2 lTexCoordsVec2 = lTexCoordsVec4.xy;
        lTexCoordsVec4.xy = AnimateTexture( lTexCoordsVec2, lAnimFrameSize, lfFrameNumber );
        lTexCoordsVec4.zw = AnimateTexture( lTexCoordsVec2, lAnimFrameSize, lfFrameNumber + 1.0 );
        
        OUT( mfAnimBlendValue )   = lfAnimBlendValue;
		OUT( mTexCoordsVec4 )   = lTexCoordsVec4;
    }
    #elif defined( _F14_UVSCROLL )
    { 
        lTexCoordsVec4.zw  = lTexCoordsVec4.xy;
        lTexCoordsVec4 += UNIFORM( mpCustomPerMesh, gUVScrollStepVec4 ) * lfTimeElapsed;

        OUT( mTexCoordsVec4 ) = lTexCoordsVec4;
    }
    #elif defined( _F18_UVTILES )
    {
        // Take the particle start time to decide on the UV section
        float lfPos = liParticleID * 1.0;
        lTexCoordsVec4.x /= lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;
        lTexCoordsVec4.x += floor( mod( lfPos, lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z ) ) / lUniforms.mpCustomPerMesh.gUVScrollStepVec4.z;

        OUT( mTexCoordsVec4 ) = lTexCoordsVec4;
    }
    #else
	{		
        lTexCoordsVec4.x = (lTexCoordsVec4.x / lUniforms.mpCustomPerMesh.gMultiTextureVec4.x);
#if !defined(D_HEAVYAIR)
        lTexCoordsVec4.x   += mod( lfParticleID, lUniforms.mpCustomPerMesh.gMultiTextureVec4.x ) / lUniforms.mpCustomPerMesh.gMultiTextureVec4.x ;
#else
		// UnstableParticelIDs: Due to the way that particles are populated for heavy air, we need to look up a "stable" particle id for looking up any
        // particle properties so that they track the same "logical" particle  instance.
        float lfStableParticleID = ARRAY_LOOKUP_FP(lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID).w;
        lTexCoordsVec4.x += mod( lfStableParticleID, lUniforms.mpCustomPerMesh.gMultiTextureVec4.x ) / lUniforms.mpCustomPerMesh.gMultiTextureVec4.x;
#endif

		OUT( mTexCoordsVec4 ) = lTexCoordsVec4;
	}
	#endif
	
    //-----------------------------------------------------------------------------
    ///
    ///     Position
    ///
    //-----------------------------------------------------------------------------
	//calculate position and billboard
    mat3 lTangentMat3;
    vec2 lScale;
    {
        float lfScaleX = ARRAY_LOOKUP_FP(lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID).x;
        lScale = vec2(lfScaleX, lfScaleX);

#if !defined(D_HEAVYAIR)
        if(!lbAlphaThresholdIsSet)
        {
            float lfScaleY = ARRAY_LOOKUP_FP(lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID).w;
            lScale.y = lfScaleY;
        }
#endif
    }
    float lfRotation = ARRAY_LOOKUP_FP(lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID).y;
    vec3 lvLocalPosition = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticlePositionsVec4, liParticleID ).xyz;


    #ifdef SM_CODE
    {
        float lfGlow = 0.0;
        float lfSkGlobalTime = lUniforms.mpPerFrame.gfTime;
        float lfOutScale = 1.0;
        float lfOutRotation = lfRotation;
        float lfOutGlow = 0.0;
        vec3 lvOutLocalPosition = lvLocalPosition;
        vec4 lFragmentColour = float2vec4(0.0);
        vec4 lVertexColour = float2vec4(0.0);
        vec4 lFragmentNormal = float2vec4(0.0);
        vec2 lTexCoords = float2vec2(0.0);

        vec3 lSkNodePositionVec3 = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lSkPlanetPositionVec3 = float2vec3(0.0);
        vec3 lSkSunPositionVec3    = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz * 100000.0;

        SM_CODE
        lScale *= lfOutScale;
        lfRotation = lfOutRotation;
        lvLocalPosition = lvOutLocalPosition;
    }
    #endif

    float lfParticlePosAlphaMultiplier;
    lParticlePositionVec4 = GetParticlePos( DEREF_PTR(lUniforms.mpPerFrame), DEREF_PTR(lUniforms.mpCommonPerMesh), lvLocalPosition, liParticleID, liParticleCorner, lScale, lfRotation, leBillboardAlignment, lTangentMat3, lfParticlePosAlphaMultiplier );

    float lfAlphaMultiplier = lParticlePositionVec4.w * lfParticlePosAlphaMultiplier;
	
	//alpha was previously stored in position w in the case of stretchy particles
	//reset to 0 for vertex transform.
	lParticlePositionVec4.w = 1.0;

    //-----------------------------------------------------------------------------
    ///
    ///     Normal mapping
    ///
    //-----------------------------------------------------------------------------
    #ifndef _F07_UNLIT
    {
        vec3 lCameraRightVec3 = MAT4_GET_COLUMN( UNIFORM( mpPerFrame, gCameraMat4 ), 0 );
        vec3 lCameraUpVec3 = MAT4_GET_COLUMN( UNIFORM( mpPerFrame, gCameraMat4 ), 1 );        
        vec3 lCameraAtVec3 = MAT4_GET_COLUMN( UNIFORM( mpPerFrame, gCameraMat4 ), 2 );

        float  lfRadius = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, liParticleID ).x;
        vec3   lMidPointVec3 = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticlePositionsVec4, liParticleID ).xyz;
        vec3   lSphereApexVec3 = lMidPointVec3 - (lCameraAtVec3 * lfRadius);
        vec3   lWorldNormalVec3 = normalize( lParticlePositionVec4.xyz - lSphereApexVec3 );

        OUT( mWorldNormalVec3 ) = lWorldNormalVec3;

#ifdef _F03_NORMALMAP
        {

#ifndef D_HEAVYAIR
            if (lNormalsCanBeFlipped)
            {
                lTangentMat3 = MUL( lTangentMat3, lNormalFlipMat3 );
            }
#endif
#ifdef D_PLATFORM_METAL
            OUT( mTangentSpaceMat3_col0 ) = lTangentMat3[0];
            OUT( mTangentSpaceMat3_col1 ) = lTangentMat3[1];
            OUT( mTangentSpaceMat3_col2 ) = lTangentMat3[2];
#else
            OUT( mTangentSpaceMat3 ) = lTangentMat3;
#endif
        }
        #endif
    }
    #endif
	
    //-----------------------------------------------------------------------------
    ///
    ///     Transform
    ///
    //-----------------------------------------------------------------------------
	
	vec4 lScreenSpacePositionVec4 = MUL( UNIFORM( mpPerFrame, gViewProjectionMat4 ), lParticlePositionVec4 );
    lScreenSpacePositionVec4.xy  -= UNIFORM( mpPerFrame, gDeJitterVec3.xy ) * lScreenSpacePositionVec4.w;

    OUT( mParticleIdX_AlphaMultiplierY ) = vec2( lfParticleID, lfAlphaMultiplier);
	OUT( mScreenSpacePositionVec4 ) = lScreenSpacePositionVec4;
    SCREEN_POSITION          = lScreenSpacePositionVec4;
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
	
}