////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonGTAO.h
///     @author     User
///     @date       
///
///     @brief      CommonGTAO
///
///     Copyright (c) 2019 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONGTAO_H
#define D_COMMONGTAO_H

//-----------------------------------------------------------------------------
//      Include files
#include "Common/CommonUtils.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Functions 

#define sqrt_fast   sqrt_fast_0
#define rcp_fast    rcp_fast_0

float acos_fast(float x)
{
    // Eberly2014 - GPGPU Programming for Games and Science
    float res = -0.156583 * abs(x) + M_PI / 2.0;
    res *= sqrt_fast(1.0 - abs(x));
    return clamp(x >= 0.0 ? res : M_PI - res, 0.0, M_PI);
}

// Compute projection of disk of radius in screen space
float GetTexelRadius( float lfFov, float lfViewDepth, float lfFrameSizeY )
{
    const float kfAORadius          = 0.6;

    // lfFov is a quarter of the vertical fov; we need half, so double it
    // then double the tangent again
    const float kfRadiusScale       = lfFrameSizeY / ( tan( lfFov * 2.0 ) * 2.0 * 1080.0 );
    const float kfRadiusToScreen    = kfAORadius * 0.5 * kfRadiusScale;    
    const float kfRadiusTexels      = kfRadiusToScreen / lfViewDepth;    
    return kfRadiusTexels;
}

int GetTexelStepsNum( const float kfRadiusTexels, const int kiMaxRadiusTexels, const float kfOffset, const float kfOffsetStep )
{   
    const float kfOffsetStepInv = rcp_fast( kfOffsetStep );
    const int   kiTexelsStepNum = max( 1, int( round( ( kfRadiusTexels - kfOffset ) * kfOffsetStepInv ) ) );

    return min( kiMaxRadiusTexels, kiTexelsStepNum );
}

bool GetViewNormal(SAMPLER2DARG(lBuffer2Map), vec2 lFragCoordsVec2, mat4 lCameraMat4, out vec3 lViewNormalVec3)
{
    // Sample compressed normal
    vec4 lBuffer2Vec4 = Texture2DNoFiltering(SAMPLER2DPARAM(lBuffer2Map), lFragCoordsVec2);

#if !defined( D_PLATFORM_ORBIS )
    // Don't run on VR hidden area mask
    if (lBuffer2Vec4.x == 0.0 && lBuffer2Vec4.y == 0.0 && lBuffer2Vec4.z == 0.0)
        return false;
#endif

    // Do an inverse/transpose multiplication of the normal to bring it into camera/view space.
    vec3 lNormalVec3 = DecodeNormal(lBuffer2Vec4.xyz);

    lViewNormalVec3 = MUL(lNormalVec3, cast2mat3(lCameraMat4) );
    lViewNormalVec3.y = -lViewNormalVec3.y;
    return true;
}

bool GetViewPosition(SAMPLER2DARG(lDepthMap), vec2 lFragCoordsVec2, mat4 lInverseProjectionMat4, out vec3 lViewPositionVec3)
{
    // Sample depth and ignore sky
    float lfClipDepth = Texture2DNoFiltering(SAMPLER2DPARAM(lDepthMap), lFragCoordsVec2).x;
    if (lfClipDepth < 0.0000001)
        return false;

    // Homogeneous input
    vec4 lViewPositionVec4;
    lViewPositionVec4.xy = lFragCoordsVec2.xy;
    lViewPositionVec4.z  = lfClipDepth;
    lViewPositionVec4.w  = 1.0;

    // Transform into view space
    // TODO(don): Remove matrix multiplication
    lViewPositionVec4 = MUL(lInverseProjectionMat4, lViewPositionVec4);
    lViewPositionVec3 = lViewPositionVec4.xyz / lViewPositionVec4.w;
    return true;
}

void TestHorizon(SAMPLER2DARG(lDepthMap), mat4 lInverseProjectionMat4, vec2 lSampleCoordsVec2, vec3 lViewPosVec3, inout float lfHorizon)
{
    // Ignore sky samples
    vec3 lSamplePosVec3;
    if (!GetViewPosition(SAMPLER2DPARAM(lDepthMap), lSampleCoordsVec2, lInverseProjectionMat4, lSamplePosVec3))
        return;

    // [Paper] Equation 6 arccos expression in the camera plane instead of the view vector.
    // Only need to keep track of max horizon, decreasing VGPR use for GCN.

    // Increase range at long distances
    // without this neighbouring texels would go immediately out of range
    const float kfAORange       = 0.6 + 0.005 * ( -lViewPosVec3.z );
    const float kfAORangeInv    = 1.0 / kfAORange;

    vec3  lHypotenuseVec3 = lSamplePosVec3 - lViewPosVec3;
    float lfHypLength     = sqrt_fast(dot(lHypotenuseVec3, lHypotenuseVec3));    
    
    if ( lfHypLength > kfAORange ) 
        return;

    float lfHypInvLength = rcp_fast(lfHypLength);
    float lfNewHorizon   = lHypotenuseVec3.z * lfHypInvLength;
    
    // [Paper] Equation 9: Counter thin objects getting too much AO wth this blend heuristic.
    // Alternatives include depth-peeling but that seems to aggressive for the target hardware.
    // TODO(don): AORadius    

    // Old heuristic implementation (caused overly darkened features especially on grass)
    //const float AORadius = 1.0;
    //float lfThicknessBlend = saturate((1.0 - lfHypLength / AORadius) * 2.0);
    //lfHorizon = mix(lfHorizon, max(lfHorizon, lfNewHorizon), lfThicknessBlend);
    

    // New heuristic implementation    
    const float kfRatio = lfHypLength * kfAORangeInv;
    if ( lfNewHorizon > lfHorizon ) 
    { 
        float lfThicknessBlend  = saturate( ( 1.0 - kfRatio ) * 2.0 );
        lfHorizon = mix( lfHorizon, max( lfHorizon, lfNewHorizon ), lfThicknessBlend );
    }
    // Don't apply heuristic close to the sample to retain fine AO detail
    else if ( kfRatio > 0.15 )
    {
        // approximates an exponential average
        const float kfThicknessBlend  = saturate( kfRatio * 0.5 + 0.5 );            
        lfHorizon = mix( lfNewHorizon, lfHorizon, kfThicknessBlend );
    }    
}

vec4 GTAO(
    SAMPLER2DARG( lBuffer1Map ),
    SAMPLER2DARG( lBuffer2Map ),
    in vec2  lFragCoordsVec2,
    in vec4  lFoVValuesVec4,
    in vec4  lFrameBufferSizeVec4,
    in mat4  lCameraMat4,
    in mat4  lInverseProjectionMat4,
    in vec4  lClipPlanesVec4,
    in int   liFrameIndex
)
{
    const vec2 kTextureSizeVec2 = vec2(GetTexResolution(lBuffer1Map));
    const vec2 kTexelSizeVec2   = vec2(1.0, 1.0) / kTextureSizeVec2;

    // Snap GTAO frag coords to align with DEPTH buffer coords
    lFragCoordsVec2 -= kTexelSizeVec2 * fract( lFrameBufferSizeVec4.xy / kTextureSizeVec2 );

    // Reconstruct view position
    vec3 lViewPosVec3;
    if (!GetViewPosition(SAMPLER2DPARAM(lBuffer1Map), lFragCoordsVec2, lInverseProjectionMat4, lViewPosVec3))
        return float2vec4(1);

    // Decode view normal from gbuffer
    vec3 lViewNormalVec3;
    if (!GetViewNormal(SAMPLER2DPARAM(lBuffer2Map), lFragCoordsVec2, lCameraMat4, lViewNormalVec3))
        return float2vec4(1);

    // Counter backward facing normals	
    lViewNormalVec3.z = max(lViewNormalVec3.z, 0.0001);
    lViewNormalVec3   = normalize(lViewNormalVec3);
        
    // We need to know texel size and aspect ratio for stepping through it along a slice        
    const float kfTexelLength   = length( kTexelSizeVec2 );
    const vec2  kTexelRatioVec2 = vec2(kTextureSizeVec2.y / kTextureSizeVec2.x, 1.0);
    const uvec2 kTexelPosUVec2  = uvec2(lFragCoordsVec2 * lFrameBufferSizeVec4.xy);

    // Spatial noise offset, to be cleaned up by the spatial filter
    const float kfSpatialRotationNoise = float((((kTexelPosUVec2.x + kTexelPosUVec2.y) & 3) << 2) + (kTexelPosUVec2.x & 3)) / 16.0 * M_PI;

    // Temporal rotation offset
    //TF_BEGIN
    #ifdef D_PLATFORM_METAL
    const float kafFrameRotations[] = { 60.0 / 360 * M_PI, 300.0 / 360 * M_PI, 180.0 / 360 * M_PI, 240.0 / 360 * M_PI, 120.0 / 360 * M_PI, 0.0 / 360 * M_PI };
    #else
    STATIC_CONST float kafFrameRotations[] = { 60.0 / 360 * M_PI, 300.0 / 360 * M_PI, 180.0 / 360 * M_PI, 240.0 / 360 * M_PI, 120.0 / 360 * M_PI, 0.0 / 360 * M_PI };
    #endif
    //TF_END
    const float kfFrameRotation = kafFrameRotations[liFrameIndex % 6];

    // One slice of numerical integration of the outer integral
    const float kfPhi           = kfSpatialRotationNoise + kfFrameRotation;
    const vec2  kSliceDirVec2   = vec2(cos(kfPhi), sin(kfPhi));

#if defined ( D_PLATFORM_GLSL )
    const float kfSpatialOffsetNoise = float( ( kTexelPosUVec2.y - kTexelPosUVec2.x ) & 3 ) * 0.25;
#else
    const float kfSpatialOffsetNoise = (( kTexelPosUVec2.y - kTexelPosUVec2.x) & 3) * 0.25;
#endif
    vec2 lHorizonsVec2 = vec2(-1,-1);

#if defined (D_GTAO_LOW)
    const int   kiMaxRadiusTexels   = 8;
    const int   kiStepCoeff         = 2;
#elif defined (D_GTAO_MEDIUM)
    const int   kiMaxRadiusTexels   = 12;
    const int   kiStepCoeff         = 1;
#elif defined (D_GTAO_HIGH)
    const int   kiMaxRadiusTexels   = 24;
    const int   kiStepCoeff         = 1;
#endif

    const float kfRadiusTexels  = GetTexelRadius( lFoVValuesVec4.x, -lViewPosVec3.z, lFrameBufferSizeVec4.y );    
    const float kfOffset        = kfTexelLength * kfSpatialOffsetNoise;
    const float kfOffsetStep    = kfTexelLength * kiStepCoeff;
    vec2        lOffsetVec2     = kSliceDirVec2 * kfOffset      * kTexelRatioVec2;
    const vec2  kOffsetStepVec2 = kSliceDirVec2 * kfOffsetStep  * kTexelRatioVec2;
    const int   kiTexelStepsNum = GetTexelStepsNum( kfRadiusTexels, kiMaxRadiusTexels, kfOffset, kfOffsetStep );

    float tempOut;
    // Find the forward horizon
    for (int j = 0; j < kiTexelStepsNum; j++)
    {           
        lOffsetVec2 += kOffsetStepVec2;
        tempOut = lHorizonsVec2.x;
        TestHorizon(SAMPLER2DPARAM(lBuffer1Map), lInverseProjectionMat4, lFragCoordsVec2 + lOffsetVec2, lViewPosVec3, tempOut);        
        lHorizonsVec2.x = tempOut;
    }
    
    lOffsetVec2 = kSliceDirVec2 * kfSpatialOffsetNoise * kTexelSizeVec2;
    
    // Find the backward horizon
    for (int j = 0; j < kiTexelStepsNum; j++)
    {
        lOffsetVec2 += kOffsetStepVec2;
        tempOut = lHorizonsVec2.y;
        TestHorizon(SAMPLER2DPARAM(lBuffer1Map), lInverseProjectionMat4, lFragCoordsVec2 - lOffsetVec2, lViewPosVec3, tempOut);       
        lHorizonsVec2.y = tempOut;    
    }

    //return float2vec4( kiTexelStepsNum / float( kiMaxRadiusTexels ) );
    //return float2vec4( float(kiTexelStepsNum == kiMaxRadiusTexels) );

    // [Paper] Equation 6 completed for the found horizons.
    lHorizonsVec2.x = -acos_fast(lHorizonsVec2.x);
    lHorizonsVec2.y =  acos_fast(lHorizonsVec2.y);

    // Project view normal onto integration plane
    vec3 lBitangentVec3 = vec3(kSliceDirVec2.y, -kSliceDirVec2.x, 0.0);
    vec3 lProjNormalVec3 = lViewNormalVec3 - lBitangentVec3 * dot(lViewNormalVec3, lBitangentVec3);

    // Clamp horizons against the hemisphere around the surface normal.
    // [Presentation] Page 58
    float lfProjNormalLen = length(lProjNormalVec3);
    float lfProjNormalInvLen = 1.0 / lfProjNormalLen;
    float lfSinN = dot(lProjNormalVec3.xy, kSliceDirVec2) * lfProjNormalInvLen;
    float lfN = acos_fast(lfSinN) - M_PI / 2;
    lHorizonsVec2.x = lfN + max(lHorizonsVec2.x - lfN, -M_PI / 2.0);
    lHorizonsVec2.y = lfN + min(lHorizonsVec2.y - lfN,  M_PI / 2.0);

    // Inner arc integral can be analytically computed
    // [Paper] Equation 7
    float lfCosN = lProjNormalVec3.z * lfProjNormalInvLen;
    vec2 lfArcIntegralSub = -2.0 * lHorizonsVec2 * lfSinN + lfCosN - cos(2.0 * lHorizonsVec2 - lfN);
    return float2vec4(lfProjNormalLen * 0.25 * (lfArcIntegralSub.x + lfArcIntegralSub.y));
}

#endif