////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonParticle.h
///     @author     User
///     @date       
///
///     @brief      CommonParticleShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONPARTICLE_H
#define D_COMMONPARTICLE_H

//-----------------------------------------------------------------------------
//      Include files


//-----------------------------------------------------------------------------
//      Global Data

#include "Common/CommonUniforms.shader.h"

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
//    Functions



// These *must* match the values found in TkParticleData.xml
STATIC_CONST uint EBillboardAlignment_Screen            = 0;
STATIC_CONST uint EBillboardAlignment_XLocal            = 1;
STATIC_CONST uint EBillboardAlignment_YLocal            = 2;
STATIC_CONST uint EBillboardAlignment_ZLocal            = 3;
STATIC_CONST uint EBillboardAlignment_NegativeXLocal    = 4;
STATIC_CONST uint EBillboardAlignment_NegativeYLocal    = 5;
STATIC_CONST uint EBillboardAlignment_NegativeZLocal    = 6;
STATIC_CONST uint EBillboardAlignment_ScreenWorld       = 7;

STATIC_CONST uint kFPUMantissa = 0x007FFFFF; // FPU mantissa
STATIC_CONST uint kFPUOne = 0x3F800000; // Binary value of 1.0 in IEEE

float RandFloat( uint luValue )
{
    luValue = (luValue ^ 61) ^ (luValue >> 16);
    luValue = luValue + (luValue << 3);
    luValue = luValue ^ (luValue >> 4);
    luValue = luValue * 0x1b873593;
    luValue = luValue ^ (luValue >> 15);

    uint luHash = luValue;

    //do the FPU magic shit to format it as a number between 0 and 1
    uint luMantissa = luHash & kFPUMantissa;
    uint luNormed = luMantissa | kFPUOne;
    float lfRes = asfloat( int( luNormed ) ) - 1.0f;
    return lfRes;
}

//-----------------------------------------------------------------------------
///
///     GetParticlePos
///
///     @brief      GetParticlePos
///
///     @param      void
///     @return     
///
//-----------------------------------------------------------------------------
vec4 
GetParticlePos(
    in PerFrameUniforms      lPerFrameUniforms,
    in CommonPerMeshUniforms lMeshUniforms,
    in vec3                  lVertexPositionVec3,
    in int                   liParticleID,
    in int                   liParticleCorner,
    in vec2                  lScale,
    in float                 lfRotation,
    in uint                  leBillboardAlignment,
    out mat3                 lOutTangentMat3,
    out float                lfOutAlphaMultiplier )
{
    
    if (leBillboardAlignment == EBillboardAlignment_Screen || leBillboardAlignment == EBillboardAlignment_ScreenWorld)
    {
        // we do the world transform for locally oriented particles, so no need for this here otherwise
        vec3 lRightVec3 = lMeshUniforms.gWorldMat4[0].xyz;
        lScale *= sqrt( dot( lRightVec3, lRightVec3 ) );
    }
    lfRotation *= 0.0174532925;

    // Particle Position
    vec3 lParticlePositionVec3    = lVertexPositionVec3; // ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaParticlePositionsVec4, liParticleID ).xyz;
    
    mat4  lCameraMat4 = mat4( 1.0, 0.0, 0.0, 0.0, 
                              0.0, 1.0, 0.0, 0.0,
                              0.0, 0.0, 1.0, 0.0,
                              0.0, 0.0, 0.0, 1.0 );

    lfOutAlphaMultiplier = 1.0;
    if (leBillboardAlignment == EBillboardAlignment_ScreenWorld)
    {
        lCameraMat4 = lPerFrameUniforms.gCameraNoHeadTrackingMat4;
        // Face camera in a world-aligned manner so that particles don't spin
        // with camera (face camera in a sphere).
        vec3 lCamAt = normalize(lCameraMat4[2].xyz);
        // We offset the sphere around which we make billboards face to be behind the camera so that we don't
        // see them "flipping" as you fly through them.
        float lfCameraOffset = 1000.0f;
        vec3 lAt = normalize( (lCameraMat4[3].xyz + lCamAt * lfCameraOffset) - lParticlePositionVec3 );
        vec3 lUp = normalize( vec3( RandFloat( uint(liParticleID)) - 0.5, RandFloat( uint(liParticleID + 1)) - 0.5, RandFloat( uint(liParticleID + 2)) - 0.5 ) );

        // We want to fade-out particles who's "pole" of rotation is aligned with the camera (as that is when they will seemingly "spin" a lot), which is why we randomise
        // the axis of rortaion in the first place. 
        lfOutAlphaMultiplier = 1.0 - abs(dot( lCamAt, lUp ));
        lfOutAlphaMultiplier *= lfOutAlphaMultiplier;
        vec3 lLeft = normalize(cross( lAt, lUp ));
        lUp = normalize(cross( lAt, lLeft ));
        lCameraMat4 = mat4( lLeft.x, lLeft.y, lLeft.z, 0.0,
                            lAt.x,   lAt.y,   lAt.z, 0.0,
                            lUp.x,   lUp.y,   lUp.z, 0.0,
                            0.0,     0.0,     0.0,   1.0 );

    }
    else
    {
        // Rotate to face camera matrix
#ifdef D_HEAVYAIR
        lCameraMat4 = lPerFrameUniforms.gCameraNoHeadTrackingMat4;
#else
        lCameraMat4 = lPerFrameUniforms.gCameraNoHeadTrackingMat4;
#endif

#ifndef D_HEAVYAIR
        if (lPerFrameUniforms.gFoVValuesVec4.z == 2.0)
#endif
        {
            vec3 lCamToParticleVec3 = lParticlePositionVec3 - lCameraMat4[3].xyz;

            lCamToParticleVec3 = lCamToParticleVec3 - (lCameraMat4[1].xyz * dot(lCameraMat4[1].xyz, lCamToParticleVec3));
            if (length(lCamToParticleVec3) > 0.0f)
            {
                lCamToParticleVec3 = normalize(lCamToParticleVec3);
            }
            float lfSinFacingAngle = dot( lCamToParticleVec3, lCameraMat4[2].xyz );            
            lfSinFacingAngle = clamp(lfSinFacingAngle, -1.0, 1.0);
            
            float lfCosFacingAngle = cos( asin( lfSinFacingAngle ) ) * sign( dot( lCamToParticleVec3, lCameraMat4[0].xyz ) );

            lCameraMat4 = MUL( lCameraMat4, mat4( lfSinFacingAngle, 0, -lfCosFacingAngle, 0,
                0, 1, 0, 0,
                lfCosFacingAngle, 0, lfSinFacingAngle, 0,
                0, 0, 0, 1 ) );
        }
    
        // lCameraMatrix[3] = vec4( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaParticlePositionsVec4, liParticleID ).xyz, 1.0 );
        // Once alex's math stuff is in this can be removed.
        lCameraMat4[3] = vec4( 0.0, 0.0, 0.0, 1.0 );
        lCameraMat4[0][3] = 0.0;
        lCameraMat4[1][3] = 0.0;
        lCameraMat4[2][3] = 0.0;
    }

    // Corner Position
    vec3 lCornerPositionVec3 = ARRAY_LOOKUP_FS(lMeshUniforms, mpCommonPerMesh, gaParticleCornersVec4, liParticleCorner).xyz;
    lCornerPositionVec3.xy *= lScale;
    

    mat4 lRotationMat4;
    {
        float lfSin = sin(lfRotation);
        float lfCos = cos(lfRotation);

        lRotationMat4 = mat4(lfCos, -lfSin, 0, 0,
            lfSin, lfCos, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
    }

    vec4 lRotatedPositionVec4;
    if (leBillboardAlignment == EBillboardAlignment_ScreenWorld)
    {
        mat4 lRotationMatrix = mat4(
            1,  0,  0,  0,
            0,  0,  1,  0,
            0, -1,  0,  0,
            0,  0,  0,  1 );

        lRotatedPositionVec4 = MUL( lRotationMatrix, vec4( lCornerPositionVec3, 1.0 ) );
    }
    else
    {
#ifndef D_HEAVYAIR
        // Offset the particle pivot point so that positions can be normalised from 0-1 (meaning from bottom to top)
        // and 0.5, 0.5 can be the centre.
        vec2 lfRotationOffset = (vec2(0.5, 0.5) - (lMeshUniforms.gParticlePivotOffsetVec2.xy)) * lScale * 2.0;
        lCornerPositionVec3 += vec3(lfRotationOffset, 0.0);
#endif

        lRotatedPositionVec4 = MUL(lRotationMat4, vec4(lCornerPositionVec3, 1.0));
    }
    
    vec4 lAlignedPositionVec4 = lRotatedPositionVec4;
    if (leBillboardAlignment == EBillboardAlignment_Screen || leBillboardAlignment == EBillboardAlignment_ScreenWorld) 
    {
        lAlignedPositionVec4 = MUL( lCameraMat4, lAlignedPositionVec4 );
    }
    else
    {
        if (leBillboardAlignment == EBillboardAlignment_XLocal)
        {
            mat4 lRotationMatrix = mat4(
                0,  0,  1,  0,
                0,  1,  0,  0,
               -1,  0,  0,  0,
                0,  0,  0,  1 );

            lAlignedPositionVec4 = MUL( lRotationMatrix, lAlignedPositionVec4 );
        }
        else if (leBillboardAlignment == EBillboardAlignment_YLocal)
        {
            mat4 lRotationMatrix = mat4(
                1, 0, 0, 0,
                0, 0, -1, 0,
                0, 1, 0, 0,
                0, 0, 0, 1);

            lAlignedPositionVec4 = MUL( lRotationMatrix, lAlignedPositionVec4 );
        }
        else if(leBillboardAlignment == EBillboardAlignment_ZLocal)
        {
            mat4 lRotationMatrix = mat4(
                -1,  0,  0,  0,
                 0,  1,  0,  0,
                 0,  0, -1,  0,
                 0,  0,  0,  1 );

            lAlignedPositionVec4 = MUL( lRotationMatrix, lAlignedPositionVec4 );
        }
        else if (leBillboardAlignment == EBillboardAlignment_NegativeXLocal)
        {
            mat4 lRotationMatrix = mat4(
                0,  0, -1,  0,
                0,  1,  0,  0,
                1,  0,  0,  0,
                0,  0,  0,  1 );

            lAlignedPositionVec4 = MUL( lRotationMatrix, lAlignedPositionVec4 );
        }
        else if (leBillboardAlignment == EBillboardAlignment_NegativeYLocal)
        {
            mat4 lRotationMatrix = mat4(
                1,  0,  0,  0,
                0,  0,  1,  0,
                0, -1,  0,  0,
                0,  0,  0,  1 );

            lAlignedPositionVec4 = MUL( lRotationMatrix, lAlignedPositionVec4 );
        }
        else if(leBillboardAlignment == EBillboardAlignment_NegativeZLocal)
        {
            mat4 lRotationMatrix = mat4(
                 1,  0,  0,  0,
                 0,  1,  0,  0,
                 0,  0,  1,  0,
                 0,  0,  0,  1 );

            lAlignedPositionVec4 = MUL( lRotationMatrix, lAlignedPositionVec4 );
        }
        
        // Apply local to world trasform of emitter object but then remove the translation as we get that from
        // the particle position values directly. This allows for the correct orientation of the particles - probably
        // could/should be simplified in the future.
        lAlignedPositionVec4 = MUL( lMeshUniforms.gWorldMat4, lAlignedPositionVec4 );
        lAlignedPositionVec4 -= vec4( MAT4_GET_COLUMN( lMeshUniforms.gWorldMat4, 3 ), 0.0);
    }


    vec4 lFinalPositionVec4 = vec4( lAlignedPositionVec4.xyz + lParticlePositionVec3, 1.0 );

    vec4 ldPdU = vec4( 1.0, 0.0, 0.0, 0.0 );
    vec4 ldPdV = vec4( 0.0, 1.0, 0.0, 0.0 );

    ldPdU = MUL( lCameraMat4, MUL( lRotationMat4, ldPdU ) );
    ldPdV = MUL( lCameraMat4, MUL( lRotationMat4, ldPdV ) );

    MAT3_SET_COLUMN( lOutTangentMat3, 0, ldPdU.xyz );
    MAT3_SET_COLUMN( lOutTangentMat3, 1, ldPdV.xyz );
    MAT3_SET_COLUMN( lOutTangentMat3, 2, MAT4_GET_COLUMN( lCameraMat4, 2 ); );

    return lFinalPositionVec4;    
}

#endif
