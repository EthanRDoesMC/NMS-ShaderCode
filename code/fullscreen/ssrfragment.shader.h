////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///
///     @file       SSRFragment.shader.h
///     @author     strgiu
///     @date       
///
///     @brief      SSRFragmement
///
///     Copyright (c) 2021 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonPostProcess.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Fullscreen/SSRCommon.shader.h"
#include "Fullscreen/SSRUtils.shader.h"

#if defined( D_PLATFORM_ORBIS )
#pragma argument (O4; fastmath; scheduler=minpressure)
#ifndef D_SSR_MARCH
// unrolled D_SSR_MARCH causes very slow compile times :(
#pragma argument(unrollallloops)
#endif
#endif


// ------------------------------------------------------------------------------------------------
// D_SSR_MARCH
// ------------------------------------------------------------------------------------------------
#ifdef D_SSR_MARCH


#include "Common/CommonGBuffer.shader.h"

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

bool
GetSSRHit(
    in      PerFrameUniforms         lPerFrameUniforms,
    in      CustomPerMeshUniforms    lPerMeshUniforms,
    SAMPLER2DARG(   lgDepthHighZMap ),
    SAMPLER2DARG(   lgDepthLowZMap ),
    in      vec3        lPosViewVec3,
    in      vec3        lViewDirVec3,
    in      vec3        lNormDirVec3,
    inout   float       lfPDF,
    in      float       lfRoughness,
    out     float       lfVisibility,
    out     vec2        lHitUVsVec2,
    out     vec3        lHitDirVec3 )
{
    lNormDirVec3                = mix( lViewDirVec3, lNormDirVec3, saturate( pow( 0.96 + dot( lViewDirVec3, lNormDirVec3 ), 0.475 ) ) );

    vec3    lReflDirVec3        = reflect( -lViewDirVec3, lNormDirVec3 );
    vec3    lPosStartVec3       = lPosViewVec3;
    vec3    lPosEndVec3         = lPosStartVec3.xyz + ( lReflDirVec3 * MAX_MARCH_DISTANCE );

    lfVisibility                = smoothstep( 0.0, 0.2, dot( -lViewDirVec3, lReflDirVec3 ) );
    lHitDirVec3                 = lReflDirVec3;
    lHitUVsVec2                 = float2vec2( 0.0 );

    bool    lbHit               = false;

    if ( lfVisibility > VISIBILITY_THRESHOLD )
    {
        lbHit = RunMarch(
                    lPerFrameUniforms,
                    SAMPLER2DPARAM( lgDepthHighZMap ),
                    SAMPLER2DPARAM( lgDepthLowZMap ),
                    lfRoughness,
                    lPosStartVec3,  lPosEndVec3,
                    lHitUVsVec2 );
    }

    lfPDF       = EncodeGGX_PDF( lfPDF, lfRoughness, lbHit );

    return lbHit;
}

FRAGMENT_MAIN_COLOUR012_SRT
{
    vec2  lUVsVec2          = TEX_COORDS;
    vec2  lFrameSizeVec2    = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    vec4  lNormBuff         = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2 );
    float lfMaterialID      = lNormBuff.a;
    int   liMaterialID      = int( lfMaterialID * 3.0 ) << 8;
    float lfVisibility      = 0.0;

    if ( ( liMaterialID & D_REFLECTIVE ) == 0 || dFdx( lfMaterialID ) != 0.0 || dFdy( lfMaterialID ) != 0.0 )
    {
        discard;
    }

    vec2  lHitUVsVec2;
    mat3  lViewMat3         = asmat3( lUniforms.mpPerFrame.gViewMat4 );
    vec3  lNormDirWSVec3    = normalize( DecodeNormal( lNormBuff.xyz ) );
    vec3  lNormDirVSVec3    = MUL( lViewMat3, lNormDirWSVec3 );
    vec4  lDepthBuff        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2, 0.0 );
    float lfDepth           = DecodeDepthFromColour( lDepthBuff );

    vec3  lWorldPosVec3     = RecreatePositionFromRevZDepth(
                                lfDepth, lUVsVec2,
                                lUniforms.mpPerFrame.gViewPositionVec3,
                                lUniforms.mpPerFrame.gInverseViewProjectionMat4 );
    vec3  lViewPosVec3      = MUL( lUniforms.mpPerFrame.gViewMat4, vec4( lWorldPosVec3, 1.0 ) ).xyz;
    vec3  lViewDirVec3      = normalize( -lViewPosVec3 );

    vec2  lPixelVec2        = floor( lUVsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );
    float lfRoughness       = texture2D(    SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsVec2 ).g;
    float lfVariance        = texture2D(    SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lUVsVec2 ).r;
    
    #if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness            *= ROUGHNESS_SCALE_OVERRIDE;
    #endif

    float lfPDF;
    vec3  lHitDirVec3;
    vec3  lMNormDirVec3     = SampleMicrofacetNormal(
                                SAMPLER2DPARAM_SRT(     lUniforms.mpCustomPerMesh, gSobolMap),
                                SAMPLER2DARRAYPARAM_SRT(lUniforms.mpCustomPerMesh, gScrambleArray0Map),
                                SAMPLER2DARRAYPARAM_SRT(lUniforms.mpCustomPerMesh, gRankArray0Map),
                                lNormDirVSVec3,
                                lPixelVec2,
                                lViewDirVec3,
                                lfRoughness,
                                lfVariance,
                                lUniforms.mpPerFrame.giFrameIndex,
                                lfPDF );

    bool  lbHit             = GetSSRHit(
                                DEREF_PTR(lUniforms.mpPerFrame),
                                DEREF_PTR(lUniforms.mpCustomPerMesh),
                                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBufferMap),
                                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gBuffer1Map),
                                lViewPosVec3,
                                lViewDirVec3,
                                lMNormDirVec3,
                                lfPDF,
                                lfRoughness,
                                lfVisibility,
                                lHitUVsVec2,
                                lHitDirVec3 );
    
    vec3 lRadianceVec3      = GetRadiance(
                                DEREF_PTR( lUniforms.mpPerFrame ),
                                DEREF_PTR( lUniforms.mpCustomPerMesh ),
                                lFrameSizeVec2,
                                lUVsVec2,
                                lHitUVsVec2,
                                lfVisibility,
                                lfRoughness,
                                lWorldPosVec3,
                                lNormDirWSVec3,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer5Map ),
                                PROBE_PARAMS_SRT );

    WRITE_FRAGMENT_COLOUR0( vec4( lRadianceVec3,  0.0 ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lHitUVsVec2,    0.0, 0.0 ) );
    WRITE_FRAGMENT_COLOUR2( vec4( lfPDF,          0.0, 0.0, 0.0 ) );
}

#endif

// ------------------------------------------------------------------------------------------------
// D_SSR_RADIANCE
// ------------------------------------------------------------------------------------------------
#ifdef D_SSR_RADIANCE

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{     
    vec2  lUVsVec2          = TEX_COORDS;
    vec2  lFrameSizeVec2    = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    mat3  lViewMat3         = asmat3( lUniforms.mpPerFrame.gViewMat4 );
    vec4  lDepthBuff        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2, 0.0 );
    vec4  lNormBuff         = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsVec2, 0.0 );
    vec4  lMaterialBuff     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2, 0.0 );
    float lfDepth           = DecodeDepthFromColour( lDepthBuff );
    vec3  lNormWorldVec3    = normalize( DecodeNormal( lNormBuff.xyz ) );
    float lfRoughness       = lMaterialBuff.g;
        
#if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness            *= ROUGHNESS_SCALE_OVERRIDE;
#endif

    vec3  lWorldPosVec3;
    
    lWorldPosVec3           = RecreatePositionFromRevZDepth(
                                lfDepth, lUVsVec2,
                                lUniforms.mpPerFrame.gViewPositionVec3,
                                lUniforms.mpPerFrame.gInverseViewProjectionMat4 );

    vec2  lHitUVsVec2       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsVec2,    0.0 ).xy;
    vec4  lHitDepthBuff     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lHitUVsVec2, 0.0 );
    float lfHitVisibility   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), lUVsVec2,    0.0 ).r;
    
    vec3  lRadianceVec3     = GetRadiance(  DEREF_PTR( lUniforms.mpPerFrame ),
                                            DEREF_PTR( lUniforms.mpCustomPerMesh ),
                                            lFrameSizeVec2,
                                            lUVsVec2,           lHitUVsVec2,
                                            lfHitVisibility,    lfRoughness,
                                            lWorldPosVec3,      lNormWorldVec3,
                                            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer6Map ),
                                            PROBE_PARAMS_SRT );

    WRITE_FRAGMENT_COLOUR( vec4( lRadianceVec3, 1.0 ) );
}

#endif

// ------------------------------------------------------------------------------------------------
// D_SSR_RESOLVE
// ------------------------------------------------------------------------------------------------
#ifdef D_SSR_RESOLVE

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)

DECLARE_INPUT_END

struct Hit
{
    vec3  mRadianceVec3;
    vec3  mReflRayVec3;
    float mfPDF;
    bool  mbValid;
};

struct LocalProps
{
    vec3  mViewPosVec3;
    vec3  mViewDirVec3;
    vec3  mNormDirVec3;
    float mfDepth;
    float mfRoughness;
};

float
GetDepth(
    vec2  lUVsVec2,
    SAMPLER2DARG( lDepthMap ) )
{
    vec4  lDepthBuff    = texture2DLod( lDepthMap, lUVsVec2, 0.0 );
    float lfDepth       = DecodeDepthFromColour( lDepthBuff );
    
    return lfDepth;
}

vec3
GetViewPos(
    vec2  lUVsVec2,
    float lfDepth, 
    mat4  lInverseProjSCMat4 )
{
    vec3  lViewPosVec3  = RecreateViewPositionFromRevZDepthSC(
                            lfDepth, lUVsVec2, lInverseProjSCMat4 );

    return lViewPosVec3;
}

vec3
GetNormalDir(
    vec2 lUVsVec2,
    mat4 lViewMat4,
    SAMPLER2DARG( lNormalMap ) )
{
    mat3  lViewMat3     = asmat3( lViewMat4 );
    vec4  lNormBuff     = texture2DLod( lNormalMap, lUVsVec2, 0.0 );
    vec3  lNormDirVec3  = normalize( MUL( lViewMat3, DecodeNormal( lNormBuff.xyz ) ) );

    return lNormDirVec3;
}

vec3
GetAlbedo(
    vec2 lUVsVec2,
    SAMPLER2DARG( lAlbedoMap ) )
{
    vec3  lAlbedoVec3 = texture2DLod( lAlbedoMap, lUVsVec2, 0.0 ).rgb;

    return lAlbedoVec3;
}

float
GetVariance(
    vec2 lUVsVec2,
    SAMPLER2DARG( lVarianceMap ) )
{
    float  lfVariance = texture2DLod( lVarianceMap, lUVsVec2, 0.0 ).r;

    return lfVariance;
}

float
GetRoughness(
    vec2 lUVsVec2,
    SAMPLER2DARG( lMaterialMap ) )
{
    vec4  lMaterialBuff = texture2DLod( lMaterialMap, lUVsVec2, 0.0 );
    float lfRoughness   = lMaterialBuff.g;
    #if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness        *= ROUGHNESS_SCALE_OVERRIDE;
    #endif

    return lfRoughness;
}

float
GetPDF(
    in  vec2  lUVsVec2,
    out bool  lbValidHit,
    SAMPLER2DARG( lRcpPDFMap ) )
{
    float  lfPDF = texture2DLod( lRcpPDFMap, lUVsVec2, 0.0 ).r;
    lfPDF        = DecodeGGX_PDF( lfPDF, lbValidHit );
    return lfPDF;
}

LocalProps
GetLocalProps(
    vec3  lViewPosVec3,
    vec3  lViewDirVec3,
    vec3  lNormDirVec3,
    float lfDepth,
    float lfRoughness )
{
    LocalProps lLocalProps;

    lLocalProps.mViewPosVec3 = lViewPosVec3;
    lLocalProps.mViewDirVec3 = lViewDirVec3;
    lLocalProps.mNormDirVec3 = lNormDirVec3;
    lLocalProps.mfDepth      = lfDepth;
    lLocalProps.mfRoughness  = lfRoughness;

    return lLocalProps;
}

Hit
GetReflectionHit(
    bool  lbPrimary,
    vec3  lLocalPosVec3,
    vec2  lUVsVec2,
    mat4  lInverseProjSCMat4,
    SAMPLER2DARG( lRadianceMap ),
    SAMPLER2DARG( lUVsMap ),
    SAMPLER2DARG( lPDFMap ),
    SAMPLER2DARG( lDepthMap ) )
{
    Hit     lHit;

    lHit.mfPDF          = texture2DLod( lPDFMap,   lUVsVec2,    0.0 ).r;
    lHit.mfPDF          = DecodeGGX_PDF( lHit.mfPDF,  lHit.mbValid );

    if ( !lHit.mbValid )
    {
        lHit.mRadianceVec3  = !lbPrimary ? float2vec3( 0.0 ) : texture2DLod( lRadianceMap, lUVsVec2, 0.0 ).rgb;
        lHit.mfPDF          = 1.0;
        return lHit;
    }

    vec2    lHitUVsVec2;
    vec3    lHitPosVec3;
    vec4    lHitDepthBuff;
    float   lfHitDepth;

    lHitUVsVec2         = texture2DLod( lUVsMap,   lUVsVec2,    0.0 ).xy;
    lHitDepthBuff       = texture2DLod( lDepthMap, lHitUVsVec2, 0.0 );
    lfHitDepth          = DecodeDepthFromColour( lHitDepthBuff );
    lHitPosVec3         = RecreateViewPositionFromRevZDepthSC(
                            lfHitDepth, lHitUVsVec2, lInverseProjSCMat4 );
    lHit.mReflRayVec3   = normalize( lHitPosVec3 - lLocalPosVec3 );
    lHit.mRadianceVec3  = texture2DLod( lRadianceMap, lUVsVec2, 0.0 ).rgb;
    return lHit;
}

float
CalculateHitWeight(
    Hit         lHit,
    LocalProps  lLocalProps )
{
    float lfWeight = 1.0;

    lfWeight  = CalculateSampleBRDFWeight(
                    lLocalProps.mViewDirVec3,
                    lLocalProps.mNormDirVec3,
                    lHit.mReflRayVec3,
                    lHit.mfPDF,
                    lLocalProps.mfRoughness );

    lfWeight /= ( 1.0 + Luminance( lHit.mRadianceVec3 ) );
    
    return lfWeight;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lUVsVec2          = TEX_COORDS.xy;
    vec2  lTexelStepVec2    = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    uvec2 lPixelVec2        = uvec2( floor( lUVsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) );

    float lfDepth           = GetDepth(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer6Map ) );

    vec3  lViewPosVec3      = GetViewPos(
                                lUVsVec2, lfDepth,
                                lUniforms.mpPerFrame.gInverseProjectionSCMat4 );

    vec3  lViewDirVec3      = normalize( -lViewPosVec3 );

    float lfVariance        = GetVariance(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ) );

    vec3  lNormDirVec3      = GetNormalDir(
                                lUVsVec2, lUniforms.mpPerFrame.gViewMat4,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer4Map ) );

    float lfRoughness      = GetRoughness(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer5Map ) );

    LocalProps lLocalProps  = GetLocalProps( lViewPosVec3, lViewDirVec3, lNormDirVec3, lfDepth, lfRoughness );
    
    Hit   lHit              = GetReflectionHit(
                                true,
                                lLocalProps.mViewPosVec3,
                                lUVsVec2,
                                lUniforms.mpPerFrame.gInverseProjectionSCMat4,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap  ),
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ),
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ),
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer6Map ) );

    float lfWeightSum       = 0.0;
    vec3  lRadianceSumVec3  = float2vec3( 0.0 );
    float lfWeight          = CalculateHitWeight( lHit, lLocalProps );
    Hit   lPrimaryHit       = lHit;

    lfWeightSum            += lfWeight;
    lRadianceSumVec3       += lHit.mRadianceVec3 * lfWeight;

    if ( !ShouldResolve( lLocalProps.mfRoughness ) && lHit.mbValid )
    {
        WRITE_FRAGMENT_COLOUR( vec4( lHit.mRadianceVec3, 1.0 ) );
        return;
    }

#ifdef D_PLATFORM_METAL
    const int    kiReuseCount    = ReuseCount(  RAY_REUSE_NUM_MAX, lfVariance, lfRoughness );
    const float  kfReuseRadius   = ReuseRadius( RAY_REUSE_RAD_MIN, RAY_REUSE_RAD_MAX, lfVariance, lfRoughness, lfDepth );
#else
    STATIC_CONST int    kiReuseCount    = ReuseCount(  RAY_REUSE_NUM_MAX, lfVariance, lfRoughness );
    STATIC_CONST float  kfReuseRadius   = ReuseRadius( RAY_REUSE_RAD_MIN, RAY_REUSE_RAD_MAX, lfVariance, lfRoughness, lfDepth );
#endif
    int     liIterations = 0;
    int     liHits       = 0;

    for (;  liIterations < RAY_REUSE_NUM_MAX; ++liIterations )
    {
        #if 0
        vec2  lRandVec2         = SampleRandomVec2(
                                    lPixelVec2,
                                    uint( lUniforms.mpPerFrame.giFrameIndex * RAY_REUSE_NUM_MAX + liIterations ), 8, 4,
                                    SAMPLER2DPARAM_SRT(     lUniforms.mpCustomPerMesh, gSobolMap ),
                                    SAMPLER2DARRAYPARAM_SRT(lUniforms.mpCustomPerMesh, gScrambleArray2Map ),
                                    SAMPLER2DARRAYPARAM_SRT(lUniforms.mpCustomPerMesh, gRankArray2Map ) );
        #else
        vec2 lRandVec2          = Rand1SPPDenoiserInput(
                                    uvec2( lPixelVec2 ),
                                    uint( lUniforms.mpPerFrame.giFrameIndex * RAY_REUSE_NUM_MAX + liIterations ) );
        #endif
        lRandVec2               = SampleDisk( lRandVec2 );

        vec2  lCurrUVsVec2      = lUVsVec2 + lRandVec2 * lTexelStepVec2 * kfReuseRadius;
        
        Hit lCurrHit            = GetReflectionHit(
                                    false,
                                    lLocalProps.mViewPosVec3,
                                    lCurrUVsVec2,
                                    lUniforms.mpPerFrame.gInverseProjectionSCMat4,
                                    SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap  ),
                                    SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ),
                                    SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ),
                                    SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer6Map ) );

        if ( !lCurrHit.mbValid )
        {
            lfWeightSum += 0.1;
            continue;
        }
        if ( !lHit.mbValid )
        { 
            lHit = lCurrHit;
        }

        float lfCurrRoughness   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), lUVsVec2, 0.0 ).g;

        #if defined( D_OVERRIDE_ROUGHNESS_SCALE )
        lfCurrRoughness        *= ROUGHNESS_SCALE_OVERRIDE;
        #endif

        float lfRoughnessWeight = GetEdgeStoppingRoughnessWeight(
                                    lLocalProps.mfRoughness,
                                    lfCurrRoughness * lfCurrRoughness, 0.0, 0.5 + lfVariance * 0.15 );

        float lfReuseWeight     = ReuseWeight( lRandVec2, kfReuseRadius, lfVariance, lfRoughness, lHit.mRadianceVec3, lCurrHit.mRadianceVec3 );

        float lfBRDFWeight      = CalculateHitWeight( lCurrHit, lLocalProps );

        lfWeight                = lfBRDFWeight * lfReuseWeight * lfRoughnessWeight;
        lfWeightSum            += lfWeight;
        lRadianceSumVec3       += lCurrHit.mRadianceVec3 * lfWeight;
        
        if ( ++liHits >= kiReuseCount ) break;
    }

    
    vec3  lOutRadianceVec3;
    lOutRadianceVec3    = lRadianceSumVec3 / lfWeightSum;
    lOutRadianceVec3   /= ( 1.0 - Luminance( lOutRadianceVec3 ) );

    if ( !lPrimaryHit.mbValid )
    {
        float lfFade        = saturate( ( liIterations - liHits ) / float( kiReuseCount ) );
        lOutRadianceVec3    = mix( lOutRadianceVec3, lPrimaryHit.mRadianceVec3, lfFade );
    }

    WRITE_FRAGMENT_COLOUR( vec4( lOutRadianceVec3, 1.0 ) );
}

#endif

// ------------------------------------------------------------------------------------------------
// D_SSR_TEMPORAL
// ------------------------------------------------------------------------------------------------
#ifdef D_SSR_TEMPORAL

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

vec2
GetParallaxRpjUVs(
    in  PerFrameUniforms lPerFrame,
    in  vec2             lUVsVec2,
    in  vec2             lHitUVsVec2,
    SAMPLER2DARG(        lDepthMap ) )
{
    float lfSurfaceDepth    = DecodeDepthFromColour( texture2DLod( lDepthMap, lUVsVec2,    0.0 ) );
    float lfHitDepth        = DecodeDepthFromColour( texture2DLod( lDepthMap, lHitUVsVec2, 0.0 ) );

    vec3  lSurfacePosVec3   = RecreatePositionFromRevZDepth(
                                lfSurfaceDepth, lUVsVec2, lPerFrame.gViewPositionVec3, lPerFrame.gInverseViewProjectionMat4 );

    vec3  lHitPosVec3       = RecreatePositionFromRevZDepth(
                                lfHitDepth,  lHitUVsVec2, lPerFrame.gViewPositionVec3, lPerFrame.gInverseViewProjectionMat4 );

    vec3  lParallaxPosVec3  = lSurfacePosVec3 + length( lHitPosVec3 - lSurfacePosVec3 ) * normalize( lSurfacePosVec3 - lPerFrame.gViewPositionVec3 );

    vec4  lParallaxUVsVec4  = MUL( lPerFrame.gPrevViewProjectionMat4, vec4( lParallaxPosVec3, 1.0 ) );
    lParallaxUVsVec4.xyz   /= lParallaxUVsVec4.w;
    lParallaxUVsVec4.xy     = SCREENSPACE_AS_RENDERTARGET_UVS( lParallaxUVsVec4.xy * 0.5 + 0.5 );

    return lParallaxUVsVec4.xy;
}

vec3
KNN(
    vec3 center_RGB,
    vec3 block3x3_YCgCo[ 9 ] )
{

    vec3    neighbour;
    vec3    center_YCgCo    = RGBToYCgCo( center_RGB );
    vec3    result          = float2vec3( 0.0 );
    float   counter         = 0.0;
    
    float   weight;
    float   weight_sum      = 0.0;


    float   rcp_sigma_g     = 0.5;
    float   rcp_sigma_c     = 0.1;
    float   threshold       = 0.0;

    float   area            = 9.0;
    float   lerp_factor     = 1.0;

    float   x;
    float   y;

    for ( int ii = 0; ii < 9; ++ii )
    {
        neighbour   = block3x3_YCgCo[ ii ];
        weight      = dot( center_YCgCo - neighbour, center_YCgCo - neighbour );
        x           = float( ii % 3 ) - 1.0;
        y           = float( ii / 3 ) - 1.0;
        weight      = exp( -( weight * rcp_sigma_c + ( x * x + y * y ) * rcp_sigma_g ) );
        weight     /= 1.0 + neighbour.x;
        counter    += int( weight > threshold );
        weight_sum += weight;

        result     += neighbour * weight;
    }

    result     /= weight_sum;
    lerp_factor = ( counter > ( threshold * area ) ) ? 1.0 - lerp_factor : lerp_factor;
    result      = mix( result, center_YCgCo, lerp_factor );

    return YCgCoToRGB( result );
}

vec2
ComputeBlendWeights(
    float lfSpeed,
    float lfVariance,
    float lfRoughness )
{
    lfRoughness *= 0.875;

    float lfSpeedBlend      = lfSpeed * mix( 1.0, 0.25, lfRoughness );

    float lfRadBlendMax     = mix( 0.25, saturate( 1.0 - lfVariance ) * 0.015,    lfRoughness * lfRoughness );
    float lfRadClipMin      = mix( 1.0,  saturate( 1.0 - lfVariance ) * 0.50,     lfRoughness * lfRoughness );
    float lfRadClipBlend    = mix( lfRadClipMin,    1.0,           saturate( lfSpeedBlend ) );
    float lfRadianceBlend   = mix( 0.025,           lfRadBlendMax, saturate( lfSpeedBlend ) );

    return vec2( lfRadClipBlend, lfRadianceBlend );
}

float
ComputeSpeed(
    vec2  lMotPrlxVec2,
    float lfSpeedPrev )
{
    float lfSpeed;
    lfSpeed         = length( lMotPrlxVec2 );
    lfSpeed         = lfSpeed * SPEED_FACTOR;
    lfSpeedPrev     = lfSpeedPrev;
    lfSpeed         = max( lfSpeed, lfSpeedPrev );
    lfSpeed         = max( lfSpeed, lfSpeed * lfSpeed );

    return lfSpeed;
}

float
GetMotion(
    vec2    lMotionVec2,
    float   lfMotionPrev )
{    
    float   lfMotion        = length( lMotionVec2 );
    lfMotionPrev            = lfMotionPrev  * step( MOTION_THRESHOLD, lfMotionPrev );
    lfMotion                = lfMotion      * step( MOTION_THRESHOLD, lfMotion     );
    lfMotion                = mix( max( lfMotionPrev, lfMotion * MOTION_TOLERANCE ), lfMotion, MOTION_HYSTERIS );

    return lfMotion;
}

float
ComputeVariance(
    float lfVarPrev,
    float lfVarSpatial,
    float lfSpeed,
    vec3  lRadCurrVec3,
    vec3  lRadPrevVec3 )
{
    float lfLumCurr = Luminance( lRadCurrVec3 );
    float lfLumPrev = Luminance( lRadPrevVec3 );

    float lfAvg;
    float lfVarCurr, lfVarFinal;

    lfAvg           = ( lfLumCurr + lfLumPrev ) * 0.5;
    lfVarCurr       = sqr( lfLumCurr - lfAvg ) + sqr( lfLumPrev - lfAvg );
    lfVarCurr       = lfVarCurr / ( 1.0 + lfVarCurr );
    lfVarCurr       = max( lfVarSpatial * 0.001, lfVarCurr / max( 1.0, lfSpeed * 2.0 ) );

    lfVarCurr       = sqrt( lfVarCurr * VARIANCE_FACTOR );
    lfVarFinal      = mix( max( lfVarCurr * VARIANCE_TOLERANCE, lfVarPrev ), lfVarCurr, VARIANCE_HYSTERIS );
    lfVarFinal      = min( lfVarFinal, 254.0 / 255.0 );

    return lfVarFinal;
}

FRAGMENT_MAIN_COLOUR012_SRT
{
    vec2  lUVsVec2          = TEX_COORDS;
    vec2  lUVsDejitVec2     = GetDejitteredTexCoord( lUVsVec2, lUniforms.mpPerFrame.gDeJitterVec3 );
    vec2  lTexSizeVec2      = vec2( GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer7Map ) ) );
    vec2  lHitUVsVec2       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2, 0.0 ).xy;
    vec2  lMotionBuffVec2   = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer6Map ), lUVsVec2 ).xy;
    vec2  lMotionVec2       = DecodeMotion( lMotionBuffVec2  );
    vec2  lUVsRpjVec2       = lUVsVec2 + lMotionVec2;    
    vec2  lUVsRpjPrlxVec2   = GetParallaxRpjUVs( DEREF_PTR( lUniforms.mpPerFrame ), lUVsVec2, lHitUVsVec2,
                                                 SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer7Map ) );
    vec2  lMotPrlxVec2      = lUVsRpjPrlxVec2 - lUVsVec2;
    vec3  lRadCurrVec3      = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map  ), lUVsVec2 ).rgb;

    if ( !CheckInside( lUVsRpjPrlxVec2 ) )
    {
        WRITE_FRAGMENT_COLOUR0( vec4( lRadCurrVec3,  1.0 ) );
        WRITE_FRAGMENT_COLOUR1( vec4( 1.0, 0.0, 0.0, 1.0 ) );
        WRITE_FRAGMENT_COLOUR2( vec4( 1.0, 0.0, 0.0, 1.0 ) );
        return;
    }

    vec3  lRadPrevVec3      = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsRpjPrlxVec2,  0.0 ).rgb;

    if ( lRadPrevVec3.x <= 0.0 )
    {
        WRITE_FRAGMENT_COLOUR0( vec4( lRadCurrVec3,  1.0 ) );
        WRITE_FRAGMENT_COLOUR1( vec4( 1.0, 0.0, 0.0, 1.0 ) );
        WRITE_FRAGMENT_COLOUR2( vec4( 1.0, 0.0, 0.0, 1.0 ) );
        return;
    }

    vec3  lRadFinalVec3;
    
    float lfVarianceFinal;
    float lfVariancePrev    = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lUVsRpjPrlxVec2,  0.0 ).r;
    float lfMotionPrev      = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lUVsVec2,         0.0 ).r;
    float lfRoughness       = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), lUVsVec2,         0.0 ).g;


    #if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness            *= ROUGHNESS_SCALE_OVERRIDE;
    #endif

    vec3  lRad_3x3Block[9];
    Gather3x3_YCgCo( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsDejitVec2, lRad_3x3Block );

    float lfVarSpatial;
    //float lfSpeed           = ComputeSpeed( lMotPrlxVec2, lfSpeedPrev );
    float lfMotion          = GetMotion( lMotionVec2, lfMotionPrev );
    float lfSpeed           = lfMotion * SPEED_FACTOR;
    vec3  lRadPrevClipVec3  = ClipColor( lRadCurrVec3, lRadPrevVec3, lRad_3x3Block, lfRoughness * lfRoughness, lfVarSpatial );
    vec2  lBlendVec2        = ComputeBlendWeights( lfSpeed, lfVariancePrev, lfRoughness );

    lRadCurrVec3            = KNN( lRadCurrVec3, lRad_3x3Block );

    lRadPrevClipVec3        = mix( lRadPrevVec3,     lRadPrevClipVec3, lBlendVec2.x );
    lRadFinalVec3           = mix( lRadPrevClipVec3, lRadCurrVec3,     lBlendVec2.y );
    lRadFinalVec3           = max( lRadFinalVec3, 1.0e-16 );

    lfVarianceFinal         = ComputeVariance( lfVariancePrev, lfVarSpatial, lfSpeed, lRadFinalVec3, lRadPrevVec3 );

    WRITE_FRAGMENT_COLOUR0( vec4( lRadFinalVec3,   1.0 ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lfVarianceFinal, 0.0, 0.0, 1.0 ) );
    WRITE_FRAGMENT_COLOUR2( vec4( lfMotion,        0.0, 0.0, 1.0 ) );
}

#endif


// ------------------------------------------------------------------------------------------------
// D_SSR_MEDIAN
// ------------------------------------------------------------------------------------------------
#ifdef D_SSR_MEDIAN

//#pragma argument (O0)

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

/*
#define s2(a, b)                            temp = a; a = min(a, b); b = max(temp, b);
#define t2(a, b)                            s2(v[a], v[b]);
#define t24(a, b, c, d, e, f, g, h)         t2(a, b); t2(c, d); t2(e, f); t2(g, h); 
#define t25(a, b, c, d, e, f, g, h, i, j)   t24(a, b, c, d, e, f, g, h);  t2(i, j);

// A Fast, Small-Radius GPU Median Filter by Morgan McGuire
// https://casual-effects.com/research/McGuire2008Median/index.html
FRAGMENT_MAIN_COLOUR_SRT
{
    const vec2 uv         = TEX_COORDS;
    const vec2 texel_size = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    
    vec4 v[25];

    // Add the pixels which make up our window to the pixel array.

    for (int dX = -2; dX <= 2; ++dX)
    {
        for (int dY = -2; dY <= 2; ++dY)
        {
            vec2 offset = vec2(float(dX), float(dY));
		    
            // If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
            // pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
            // bottom right pixel of the window at pixel[N-1].
            v[(dX + 2) * 5 + (dY + 2)] = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), uv + offset * texel_size );
        }
    }
    
    vec4 temp;
    t25( 0,  1,  3,  4,  2,  4,  2,  3,  6,  7);
    t25( 5,  7,  5,  6,  9,  7,  1,  7,  1,  4);
    t25(12, 13, 11, 13, 11, 12, 15, 16, 14, 16);
    t25(14, 15, 18, 19, 17, 19, 17, 18, 21, 22);
    t25(20, 22, 20, 21, 23, 24,  2,  5,  3,  6);
    t25( 0,  6,  0,  3, 4,   7,  1,  7,  1,  4);
    t25(11, 14,  8, 14, 8,  11, 12, 15,  9, 15);
    t25( 9, 12, 13, 16, 10, 16, 10, 13, 20, 23);
    t25(17, 23, 17, 20, 21, 24, 18, 24, 18, 21);
    t25(19, 22,  8, 17, 9,  18,  0, 18,  0,  9);
    t25(10, 19,  1, 19, 1,  10, 11, 20,  2, 20);
    t25( 2, 11, 12, 21, 3,  21,  3, 12, 13, 22);
    t25( 4, 22,  4, 13, 14, 23,  5, 23,  5, 14);
    t25(15, 24,  6, 24, 6,  15,  7, 16,  7, 19);
    t25( 3, 11,  5, 17, 11, 17,  9, 17,  4, 10);
    t25( 6, 12,  7, 14, 4,   6,  4,  7, 12, 14);
    t25(10, 14,  6,  7, 10, 12,  6, 10,  6, 17);
    t25(12, 17,  7, 17, 7,  10, 12, 18,  7, 12);
    t24(10, 18, 12, 20, 10, 20, 10, 12);

    WRITE_FRAGMENT_COLOUR( v[12] );
}
*/

#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges

FRAGMENT_MAIN_COLOUR_SRT
{
    const vec2 uv         = TEX_COORDS;
    const vec2 texel_size = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    
    vec3 v[9];

    // Add the pixels which make up our window to the pixel array.
    for(int dX = -1; dX <= 1; ++dX) 
    {
      for(int dY = -1; dY <= 1; ++dY) 
      {
        vec2 offset = vec2(float(dX), float(dY));	    
        // If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
        // pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
        // bottom right pixel of the window at pixel[N-1].
        v[(dX + 1) * 3 + (dY + 1)] = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), uv + offset * texel_size, 0.0 ).rgb;
      }
    }

    vec3 temp;
    
    // Starting with a subset of size 6, remove the min and max each time
    mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
    mnmx5(v[1], v[2], v[3], v[4], v[6]);
    mnmx4(v[2], v[3], v[4], v[7]);
    mnmx3(v[3], v[4], v[8]);
    WRITE_FRAGMENT_COLOUR( vec4( v[4], 1.0 ) );
}

#endif

// ------------------------------------------------------------------------------------------------
// D_SSR_APPLY
// ------------------------------------------------------------------------------------------------
#ifdef D_SSR_APPLY

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

vec3
GetViewDir(
    vec2  lUVsVec2,
    mat4  lInverseProjSCMat4,
    SAMPLER2DARG( lDepthMap ) )
{
    vec4  lDepthBuff    = texture2DLod( lDepthMap, lUVsVec2, 0.0 );
    float lfDepth       = DecodeDepthFromColour( lDepthBuff );
    vec3  lViewPosVec3  = RecreateViewPositionFromRevZDepthSC(
                            lfDepth, lUVsVec2, lInverseProjSCMat4 );
    vec3  lViewVec3     = normalize( -lViewPosVec3 );

    return lViewVec3;
}

vec3
GetNormalDir(
    vec2 lUVsVec2,
    mat4 lViewMat4,
    SAMPLER2DARG( lNormalMap ) )
{
    mat3  lViewMat3     = asmat3( lViewMat4 );
    vec4  lNormBuff     = texture2DLod( lNormalMap, lUVsVec2, 0.0 );
    vec3  lNormDirVec3  = normalize( MUL( lViewMat3, DecodeNormal( lNormBuff.xyz ) ) );

    return lNormDirVec3;
}

vec3
GetAlbedo(
    vec2 lUVsVec2,
    SAMPLER2DARG( lAlbedoMap ) )
{
    vec3  lAlbedoVec3 = texture2DLod( lAlbedoMap, lUVsVec2, 0.0 ).rgb;

    return lAlbedoVec3;
}

vec3
GetReflection(
    vec2 lUVsVec2,
    SAMPLER2DARG( lReflectionMap ) )
{
    vec3  lReflectionVec3 = texture2DLod( lReflectionMap, lUVsVec2, 0.0 ).rgb;

    return lReflectionVec3;
}

vec2
GetMaterial(
    vec2 lUVsVec2,
    SAMPLER2DARG( lMaterialMap ) )
{
    vec4  lMaterialBuff = texture2DLod( lMaterialMap, lUVsVec2, 0.0 );
    float lfRoughness   = lMaterialBuff.g;
    float lfMetallic    = lMaterialBuff.b;
    #if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness        *= ROUGHNESS_SCALE_OVERRIDE;
    #endif

    return vec2( lfRoughness, lfMetallic );
}

float
GetRoughness(
    vec2 lUVsVec2,
    SAMPLER2DARG( lMaterialMap ) )
{
    vec4  lMaterialBuff = texture2DLod( lMaterialMap, lUVsVec2, 0.0 );
    float lfRoughness   = lMaterialBuff.g;
    #if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness        *= ROUGHNESS_SCALE_OVERRIDE;
    #endif

    return lfRoughness;
}

FRAGMENT_MAIN_COLOUR_SRT
{
    const vec2 lUVsVec2     = TEX_COORDS;

    vec3  lRadianceVec3     = GetReflection(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBufferMap  ) );
    
    vec3  lViewDirVec3      = GetViewDir(
                                lUVsVec2, lUniforms.mpPerFrame.gInverseProjectionSCMat4,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer4Map ) );

    vec3  lAlbedoVec3       = GetAlbedo(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ) );

    vec3  lNormDirVec3      = GetNormalDir(
                                lUVsVec2, lUniforms.mpPerFrame.gViewMat4,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ) );

    float lfRoughness       = GetRoughness(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ) );

    // TODO(sal): hook up metalness and specular colour
    #if 0
    vec2  lMaterialVec2     = GetMaterial(
                                lUVsVec2,
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ) );

    float DielectricSpec    = 0.04;
    float lfRoughness       = lMaterialVec2.x;
    float lfMetallic        = lMaterialVec2.y;
    vec3  lSpecColourVec3   = DielectricSpec - ( DielectricSpec * lfMetallic ) + lAlbedoVec3 * lfMetallic;
    #endif

    lRadianceVec3          *= EnvBRDFApprox( float2vec3( 1.0 ), lfRoughness, dot( lNormDirVec3, lViewDirVec3 ) );
    lRadianceVec3          *= 3.0;
    lRadianceVec3          /= 1.0 + lRadianceVec3;
    lRadianceVec3           = max( lRadianceVec3, 0.0 );

    WRITE_FRAGMENT_COLOUR( vec4( lRadianceVec3, 1.0 ) );
}

#endif

// ------------------------------------------------------------------------------------------------
// D_PROBE_REFLECTIONS
// ------------------------------------------------------------------------------------------------
#ifdef D_PROBE_REFLECTIONS

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
#if defined( D_LOCAL_REFLECTIONS )
    vec2  lUVsVec2          = TEX_COORDS;
    vec4  lDepthBuff        = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lUVsVec2, 0.0 );
    vec4  lNormBuff         = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lUVsVec2, 0.0 );
    vec4  lMaterialBuff     = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lUVsVec2, 0.0 );
    float lfDepth           = DecodeDepthFromColour( lDepthBuff );
    vec3  lNormWorldVec3    = normalize( DecodeNormal( lNormBuff.xyz ) );
    float lfRoughness       = lMaterialBuff.g;
        
#if defined( D_OVERRIDE_ROUGHNESS_SCALE )
    lfRoughness            *= ROUGHNESS_SCALE_OVERRIDE;
#endif

    vec3  lWorldPosVec3;
    vec3  lCubeRadianceVec3;

    lWorldPosVec3       = RecreatePositionFromRevZDepth(
                            lfDepth, lUVsVec2,
                            lUniforms.mpPerFrame.gViewPositionVec3,
                            lUniforms.mpPerFrame.gInverseViewProjectionMat4 );

    lCubeRadianceVec3   = LocalCubemapReflection(
                            DEREF_PTR( lUniforms.mpPerFrame ),
                            DEREF_PTR( lUniforms.mpCustomPerMesh ),
                            lWorldPosVec3,
                            lNormWorldVec3,
                            lfRoughness,
                            PROBE_PARAMS_SRT );

    lCubeRadianceVec3  *= LOCAL_REFLECTION_STRENGTH_BIAS;
#else
    vec3  lCubeRadianceVec3;
    lCubeRadianceVec3   = float2vec3( 0.0 );
#endif

    WRITE_FRAGMENT_COLOUR( vec4( lCubeRadianceVec3, 1.0 ) );
}

#endif