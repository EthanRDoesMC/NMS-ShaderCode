////////////////////////////////////////////////////////////////////////////////
///
///     @file       UberFragmentShader.h
///     @author     User
///     @date       
///
///     @brief      UberFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

//#define D_USES_VERTEX_NORMAL

//-----------------------------------------------------------------------------
//      Include files

#include "UberCommon.h"

#if defined( _F44_IMPOSTER ) || defined( D_IMPOSTER )
    #include "Imposter.shader.h"
#endif
//#include "Common/Noise3d.glsl"
#include "Common/Common.shader.h"
#include "Common/CommonFragment.shader.h"
#include "Common/CommonDepth.shader.h"
//#include "Common/CommonNoise.shader.h"
#if defined( D_DEFER ) 
#include "Common/CommonGBuffer.shader.h"
#else
#include "Common/CommonLighting.shader.h"
#endif

#if defined( D_FADE ) 
    #include "Common/CommonFade.shader.h"
#endif

#ifdef SM_CODE
    #include "Common/ShaderMillDefines.shader.h"
#endif

#if defined ( D_PLATFORM_SWITCH )
#pragma optionNV(fastmath on)
#endif

#if (defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS )) && defined( D_DEFER )
#pragma PSSL_target_output_format(target 1 FMT_UNORM16_ABGR)
#endif

//-----------------------------------------------------------------------------
//      Global Data

//
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

    INPUT( vec4, mTexCoordsVec4,            TEXCOORD0 )

#ifdef D_USES_WORLD_POSITION
    INPUT( vec4, mWorldPositionVec3_mfSpare,TEXCOORD1 )
#endif

#if !defined(D_DEPTH_CLEAR)

#if (defined ( _F21_VERTEXCOLOUR ) || defined( _F45_VERTEX_BLEND ) || defined( _F33_SHELLS )) && !defined( D_DEPTHONLY )
    INPUT( vec4, mColourVec4,               TEXCOORD2 )
#endif

#ifdef _F30_REFRACTION_MAP
    INPUT( vec4, mProjectedTexCoordsVec4,   TEXCOORD3 )
#endif

#if !defined( _F01_DIFFUSEMAP )
    INPUT( vec4,   mMaterialVec4,           TEXCOORD4 )
#endif

#ifdef D_USES_VERTEX_NORMAL
    INPUT( vec3, mTangentSpaceNormalVec3, TEXCOORD5 )
#endif

#if !defined( D_DEPTHONLY )

#ifdef _F20_PARALLAXMAP
    INPUT( vec3, mTangentSpaceEyeVec3,      TEXCOORD6 )
    INPUT( vec3,  mTangentSpaceLightDirVec3, TEXCOORD7 )
#endif

#if 0 // defined( D_OUTPUT_MOTION_VECTORS ) && defined( _F14_UVSCROLL )
    INPUT( vec4, mPrevTexCoordsVec4,        TEXCOORD8 )
#endif

#if !defined( D_DEFER ) && !defined( _F07_UNLIT ) && !defined( D_DEPTHONLY ) && !defined(D_PLATFORM_METAL)
    flat INPUT( mat3,  mUpMatrixMat3,        TEXCOORD9 )
#endif

//#if defined( _F44_IMPOSTER )
//    INPUT( vec3, mShadowWorldPositionVec3,  TEXCOORD8 )
//#endif

#if defined( _F03_NORMALMAP )  || defined( _F42_DETAIL_NORMAL )
    INPUT( vec3,   mTangentMatRow1Vec3,     TEXCOORD13 )
    INPUT( vec3,   mTangentMatRow2Vec3,     TEXCOORD14 )
    INPUT( vec3,   mTangentMatRow3Vec3,     TEXCOORD15 )
#endif


#if defined( _F58_USE_CENTRAL_NORMAL ) || defined( _F56_MATCH_GROUND ) || defined( D_DEFERRED_DECAL )
    flat INPUT( vec3, mCenteralNormalVec3,		TEXCOORD16 )
#endif

#ifdef D_OUTPUT_MOTION_VECTORS
    INPUT_VARIANT( vec4,   mPrevScreenPosition,      TEXCOORD17, HAS_MOTION_VECTORS)
#endif
    
#endif
#endif

#if defined ( D_USE_SCREEN_POSITION )
    INPUT( vec4,   mScreenSpacePositionVec4, TEXCOORD18 )
#endif

    flat INPUT( vec3, mfFadeValueForInstance_mfLodIndex_mfShearMotionLength, TEXCOORD19 )

#if defined ( D_SK_USE_LOCAL_POSITION )
    INPUT( vec4, mLocalPositionVec4,   TEXCOORD20 )
#endif

#if ENABLE_OCTAHEDRAL_IMPOSTERS && defined(_F44_IMPOSTER)
        INPUT( vec4, mImposterData0, TEXCOORD21)
        flat INPUT( vec4, mImposterViewNormal, TEXCOORD24 )
        INPUT( vec4, mImposterFrameXY_FrameProjectonVecZW0, TEXCOORD25 )
        INPUT( vec4, mImposterFrameXY_FrameProjectonVecZW1, TEXCOORD26 )
        INPUT( vec4, mImposterFrameXY_FrameProjectonVecZW2, TEXCOORD27 )
#if defined ( D_INSTANCE ) && !defined ( D_PLATFORM_METAL )
        flat INPUT( mat3, mWorldNormalMat3, TEXCOORD28 )
#endif
#endif

#ifdef SM_INTERP
    #define SM_INTERP_VAL( v, n, t ) INPUT( v, n, t )
    SM_INTERP
    #undef SM_INTERP_VAL
#endif

    INPUT_FRONTFACING

DECLARE_INPUT_END

#ifdef D_DEFER
#include "OutputDeferred.shader.h"
#elif !defined( D_DEPTHONLY )
#include "OutputForward.shader.h"
#endif

#if defined( _F62_DETAIL_ALPHACUTOUT )
    STATIC_CONST float kfAlphaThreshold    = 0.1;
    STATIC_CONST float kfAlphaThresholdMax = 0.5;
#elif defined( _F33_SHELLS )
    STATIC_CONST float kfAlphaThreshold    = 0.05;
    STATIC_CONST float kfAlphaThresholdMax = 0.3;
#elif defined( _F11_ALPHACUTOUT )
    STATIC_CONST float kfAlphaThreshold    = 0.45;
    STATIC_CONST float kfAlphaThresholdMax = 0.8;
#else
    STATIC_CONST float kfAlphaThreshold    = 0.0001;
#endif

#if defined( D_PLATFORM_PC )
    STATIC_CONST vec4 gaDebugLodColours[ 5 ] =
    {
        vec4( 1.0, 0.0, 0.0, 1.0 ),
        vec4( 0.0, 1.0, 0.0, 1.0 ),
        vec4( 0.0, 0.0, 1.0, 1.0 ),
        vec4( 1.0, 1.0, 1.0, 1.0 ),
        vec4( 0.0, 0.0, 0.0, 1.0 )
    };
#endif

//-----------------------------------------------------------------------------
//    Functions

#ifdef D_DEFERRED_DECAL

//#ifdef D_PLATFORM_SWITCH
//    precision highp sampler;
//#endif

vec2
GetDecalTexCoords(
    in PerFrameUniforms            lPerFrameUniforms,
    in CommonPerMeshUniforms       lPerMeshUniforms,
#ifdef D_PLATFORM_ORBIS
    SAMPLER2DARRAYARG(          lTexture ),
#else
    SAMPLER2DARG(               lTexture ),
#endif
    in vec4                     lScreenSpacePositionVec4,
    out vec3                    lWorldPositionVec3 )
{
    vec3 lTexCoords;    
    vec2 lGBufferCoords = SCREENSPACE_AS_RENDERTARGET_UVS(( lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w ) * 0.5 + 0.5);

    // Deferred Pos is in view space
#ifdef D_PLATFORM_ORBIS
    float lfDepth = FastDenormaliseDepth( lPerFrameUniforms.gClipPlanesVec4, DecodeDepthFromColour( texture2DArray( lTexture, vec3(lGBufferCoords, lPerFrameUniforms.gVREyeInfoVec3.x ) ) ) );
#else
    float lfDepth = FastDenormaliseDepth( lPerFrameUniforms.gClipPlanesVec4, DecodeDepthFromColour( texture2D(lTexture, lGBufferCoords) ) );
#endif
    lWorldPositionVec3 = RecreatePositionFromDepthWithIVP( lfDepth, lGBufferCoords, lPerFrameUniforms.gViewPositionVec3 , lPerFrameUniforms.gInverseViewProjectionMat4, lPerFrameUniforms.gClipPlanesVec4 );
    
    vec3  lObjectSpaceVec3   = MUL( lPerMeshUniforms.gInverseModelMat4, vec4( lWorldPositionVec3, 1.0 ) ).xyz;
    //vec3  lObjectSpaceVec3   = MUL( inverse( lPerMeshUniforms.gWorldMat4 ), vec4( lWorldPositionVec3, 1.0 ) ).xyz;

    //float lfMaxScale = max( length( lPerMeshUniforms.gWorldMat4[0] ), max( length( lPerMeshUniforms.gWorldMat4[1] ), length( lPerMeshUniforms.gWorldMat4[2] ) ) );

    lTexCoords.xyz = (lObjectSpaceVec3.xyz) + 0.5;
    lTexCoords.y   = 1.0 - lTexCoords.y;
    //lTexCoords.xy *= 1.0 / (lfMaxScale * lfMaxScale);
    
    if ( lTexCoords.x <  0.0 ||
         lTexCoords.x >= 1.0 ||
         lTexCoords.y <  0.0 ||
         lTexCoords.y >= 1.0 ||
         lTexCoords.z <  0.0 ||
         lTexCoords.z >= 1.0 )
    {
        discard;
    }

    return lTexCoords.xy;
}
#endif

mat3
GetDecalTangentSpaceMatrix(
    in vec3 worldPosition,
    in mat4 lCameraMat4 )
{
    vec3 ddxWp    = dFdx( worldPosition );
    vec3 ddyWp    = dFdy( worldPosition );
    vec3 normal   = normalize( cross( ddyWp, ddxWp ) );
    vec3 binormal = normalize( ddxWp );
    vec3 tangent  = normalize( ddyWp );

    mat3 View;
    mat3 tangentToView;

    View[0] = lCameraMat4[0].xyz;
    View[1] = lCameraMat4[1].xyz;
    View[2] = lCameraMat4[2].xyz;

#ifndef D_PLATFORM_OPENGL
    tangentToView[0] = MUL( -tangent, View );
    tangentToView[1] = MUL( binormal, View );
    tangentToView[2] = MUL( -normal , View );
#else
    tangentToView[0] = MUL( tangent, View );
    tangentToView[1] = MUL( binormal, View );
    tangentToView[2] = MUL( normal , View );
#endif

    return tangentToView;
}

mat3 
GetCotangentFrame( vec3 lPositionVec3, vec3 lViewPosition, vec2 uv )
{    
    vec3 ddxWp = dFdx( lPositionVec3 );

#ifndef D_PLATFORM_OPENGL
    vec3 ddyWp = -dFdy( lPositionVec3 );
#else
    vec3 ddyWp = dFdy( lPositionVec3 );
#endif

    vec3 lVertexNormal = normalize( cross( ddyWp, ddxWp ) );
    vec3 lViewDirVec3 = -normalize( lViewPosition - lPositionVec3.xyz );

    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( lViewDirVec3 );
    vec3 dp2 = dFdy( lViewDirVec3 );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );

    // solve the linear system
    vec3 dp2perp = cross( dp2, lVertexNormal );
    vec3 dp1perp = cross( lVertexNormal, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame 
    float invmax = invsqrt(max(dot(T, T), dot(B, B)));
    return mat3( T * invmax, B * invmax, -lVertexNormal );

}

#if defined( _F56_MATCH_GROUND ) && !defined( D_DEPTHONLY )

// all platforms should be using this path now
#if 1//defined( D_PLATFORM_SWITCH )

vec3
GetGrassColour(
    in CommonPerMeshUniforms     lCommonUniforms,
    in CustomPerMaterialUniforms lCustomUniforms,
    in vec3                     lTextureColourVec3,
    in vec3                      lWorldPositionVec3,
    in half                      lfDistance)
{
    vec3 lTexturePosition = lWorldPositionVec3 - lCommonUniforms.gPlanetPositionVec4.xyz;

    // Using BC1 noise texture.
    vec4 lTileBlendScalesVec4 = lCustomUniforms.gTileBlendScalesVec4;
    half lfNoiseSample = texture3DLod(SAMPLER_GETLOCAL(lCustomUniforms, gPerlin3D), lTexturePosition * lTileBlendScalesVec4.x, 0.0).g;
    half lfPatch1 = lfNoiseSample * 2.0 - 1.0;

    lfNoiseSample = texture3DLod(SAMPLER_GETLOCAL(lCustomUniforms, gPerlin3D), lTexturePosition * lTileBlendScalesVec4.y, 0.0).g;
    half lfPatch2 = lfNoiseSample * 2.0 - 1.0;

    half lfPatch = lfPatch1 + lfPatch2 * lTileBlendScalesVec4.z;
    lfPatch = 1.0 - saturate(lfPatch * 0.5 + 0.5 + lTileBlendScalesVec4.w);
    lfPatch = FastSmoothStepHalf(0.5 - 0.12, 0.24, lfPatch);

    half  lfTexVal = max( lTextureColourVec3.r, max( lTextureColourVec3.g, lTextureColourVec3.b ) );

    vec3 lColour1 = vec3( lCustomUniforms.gTerrainColour1Vec4.rgb );
    vec3 lColour2 = vec3( lCustomUniforms.gTerrainColour2Vec4.rgb );

    lfTexVal    = saturate( lfTexVal * 2.0 ) - 1.0;
    lfTexVal    = 1.0 +     lfTexVal * lfDistance;

    lColour1    = mix( lColour1, lColour2, lfPatch ) * lfTexVal;

    return vec3 ( GammaCorrectInput01Half( half3 ( lColour1 ) ) );
}

#else

vec3
GetGrassColour(
    in CommonPerMeshUniforms     lCommonUniforms,
    in CustomPerMaterialUniforms lCustomUniforms,
    in vec3                      lTextureColourVec3,
    in vec3                      lWorldPositionVec3,
    in float                     lfDistance )
{
   vec3 lTexturePosition = lWorldPositionVec3 - lCommonUniforms.gPlanetPositionVec4.xyz;

    // Original patch noise check.
    //vec4  lfNoiseSample  = texture3DLod( SAMPLER_GETLOCAL(lCustomUniforms, gPerlin3D), lTexturePosition * lCustomUniforms.gTileBlendScalesVec4.x, 0.0 );
    //float lfPatch1       = saturate( ( ( lfNoiseSample.r + lfNoiseSample.g + lfNoiseSample.b + lfNoiseSample.a ) / 4.0 ) ) * 2.0 - 1.0;
    //lfNoiseSample        = texture3DLod( SAMPLER_GETLOCAL(lCustomUniforms, gPerlin3D), lTexturePosition * lCustomUniforms.gTileBlendScalesVec4.y, 0.0 );
    //float lfPatch2       = saturate( ( ( lfNoiseSample.r + lfNoiseSample.g + lfNoiseSample.b + lfNoiseSample.a ) / 4.0 ) ) * 2.0 - 1.0;

    // Using BC1 noise texture.
    float lfNoiseSample  = texture3DLod( SAMPLER_GETLOCAL(lCustomUniforms, gPerlin3D), lTexturePosition * lCustomUniforms.gTileBlendScalesVec4.x, 0.0 ).g;
    float lfPatch1       = saturate( lfNoiseSample ) * 2.0 - 1.0;

    lfNoiseSample        = texture3DLod( SAMPLER_GETLOCAL(lCustomUniforms, gPerlin3D), lTexturePosition * lCustomUniforms.gTileBlendScalesVec4.y, 0.0 ).g;
    float lfPatch2       = saturate( lfNoiseSample ) * 2.0 - 1.0;

    float lfPatch        = lfPatch1 + lfPatch2 * lCustomUniforms.gTileBlendScalesVec4.z;
    lfPatch              = 1.0 - saturate( lfPatch * 0.5 + 0.5 + lCustomUniforms.gTileBlendScalesVec4.w );
    lfPatch              = smoothstep( 0.5 - 0.12, 0.5 + 0.12, lfPatch );

    vec3 lOriginalColour = RGBToHSV( lTextureColourVec3 );

    vec3 lColour1 = lCustomUniforms.gTerrainColour1Vec4.rgb;
    vec3 lColour2 = lCustomUniforms.gTerrainColour2Vec4.rgb;

    float lfValue1 = saturate( lOriginalColour.z * 2.0 ) * lColour1.z;
    float lfValue2 = saturate( lOriginalColour.z * 2.0 ) * lColour2.z;

    lColour1.z = mix( lColour1.z, lfValue1, lfDistance );
    lColour2.z = mix( lColour2.z, lfValue2, lfDistance );

    lColour1 = HSVToRGB( lColour1 );
    lColour2 = HSVToRGB( lColour2 );

    lColour1 = mix( lColour1, lColour2, lfPatch );

    lColour1 = GammaCorrectInput( lColour1 );

    return lColour1;
}

#endif // defined( D_PLATFORM_SWITCH )

#endif

#ifdef _F20_PARALLAXMAP

vec2
GetParallaxTexCoords(
    SAMPLER2DARG( lDepthTexture ),
    vec2        lTexCoords,
    vec3        lViewDirTanSpace,
    vec3        lLightDirTanSpace,
    vec4        lParallaxDataVec4,
    out float   lfInShadow )
{
    float   lfHeightScale   = lParallaxDataVec4.w;
    int     liNumLayers     = int(lParallaxDataVec4.x);
    int     liVersion       = int(lParallaxDataVec4.y);
    float   lfShadowFactor  = lParallaxDataVec4.z;
    lfInShadow = 0.0;

    // liVersion 0 = parallax map, no shadow
    // liVersion 1 = parallax map, self shadow
    // liVersion 2 = basic normal map - useful for comparisons. (use ngui - graphics.parallax)

    if ( liVersion==2 )
    {
        // Don't do anything. This is useful to switch back to normal mapping for performance comparisons.
        return lTexCoords;  
    }    
    else// if ( liVersion==2 )
    {
        //
        // do fewer samples as we become more perpendicular.
        //
        const int kiMinIterations = 5;    
        float lfDot = max(dot(vec3(0.0, 0.0, 1.0), -lViewDirTanSpace), 0.0);
        lfDot *= lfDot * lfDot;

        // If doing the shadows then don't scale down the number of samples or the shadows don't work correctly.
        float liFinalNumLayers = liVersion==1 ? liNumLayers : kiMinIterations + mix( liNumLayers, 1, lfDot );
        //float liFinalNumLayers = kiMinIterations + mix( liNumLayers, 1, lfDot );
                              
        const float kfLayerDepth  = 1.0 / liFinalNumLayers;        
        float lfCurrentLayerDepth = 0.0;
        
        // Calculate texcoord delta between each layer.
        lViewDirTanSpace.x  = -lViewDirTanSpace.x;
        vec2 lRay           = lViewDirTanSpace.xy * lfHeightScale; 
        vec2 lDeltaVec2     = lRay / liFinalNumLayers;// liNumLayers;

        vec2  lCurrentTexCoordsVec2         = lTexCoords;
        float lfCurrentDepthMap             = 1.0 - texture2D(lDepthTexture, lCurrentTexCoordsVec2).r;
        float lfCachedCurrentDepthMapValue  = lfCurrentDepthMap;

        int liNumSamples = 0;
        while( lfCurrentLayerDepth < lfCurrentDepthMap )
        {
            // Move along ray and increment depth.
            lCurrentTexCoordsVec2       -= lDeltaVec2;
            lfCurrentDepthMap                   = 1.0 - texture2D(lDepthTexture, lCurrentTexCoordsVec2).r;

            lfCurrentLayerDepth         += kfLayerDepth;  

            // Use this to step back up when doing self shadow.
            liNumSamples++;

        }

        // get texture coordinates before collision (reverse operations)
        vec2 lPrevTexCoordsVec2 = lCurrentTexCoordsVec2 + lDeltaVec2;

        // get depth after and before collision for linear interpolation
        float lfDepthBefore = 1.0 - texture2D(lDepthTexture, lPrevTexCoordsVec2).r - lfCurrentLayerDepth + kfLayerDepth; // shoud be able to cache this
        float lfDepthAfter  = lfCurrentDepthMap - lfCurrentLayerDepth;

        // Lerp between before/after values.
        float   lfWeight            = lfDepthAfter / (lfDepthAfter - lfDepthBefore);
        vec2    lFinalTexCoordsVec2 = lPrevTexCoordsVec2 * lfWeight + lCurrentTexCoordsVec2 * (1.0 - lfWeight);
        
        // If not doing self shadow version return now.
        if ( liVersion==0 )
        {
            lfInShadow = 0.0;
            return lFinalTexCoordsVec2;
        }

        //
        // Self shadowing. Same as before except along light direction.
        //
        
        // only do this if the light is shining on the surface....
        // do we want this? It makes for weird perf variation in the same scene...

        float lfCurrentShadowLayerDepth = 0.0;
        lLightDirTanSpace.y     = -lLightDirTanSpace.y;        
        lRay                    = -lLightDirTanSpace.xy * lfHeightScale; 
        lDeltaVec2              = lRay / liFinalNumLayers; //liNumLayers;

        lCurrentTexCoordsVec2   = lFinalTexCoordsVec2;
        int     liCounter       = 1;
        float   lfShadow        = 0.0;

        // Same as before except going the other way and along the light direction instead of view.
        while( liCounter < liNumSamples )
        {
            lCurrentTexCoordsVec2       += lDeltaVec2;
            lfCurrentDepthMap           = 1.0 - texture2D(lDepthTexture, lCurrentTexCoordsVec2).r; 
            lfCurrentShadowLayerDepth   += kfLayerDepth;  

            // gather shadow values and average later.
            lfShadow = lfCurrentShadowLayerDepth > lfCurrentDepthMap ? lfShadow + 1.0 : lfShadow;

            ++liCounter;
        }

        lfInShadow = saturate( lfShadow / lfShadowFactor );
        
        //lfInShadow =  lfShadow > 0.01 ? 1.0 : 0.0; // hard shadow
        return lFinalTexCoordsVec2;  
    }

//todo: 
  //    - do line interesction test for final iteration  
  // - figure out decent control over shadows - maybe pass in the divisor from metadata?  
  // - fix cloud shadows

    
    return vec2( 0.0, 0.0 );
}
#endif

#if defined( D_ENV_OVERLAY )
void
ApplyEnviromentalOverlay(
    SAMPLER2DARG( lOverlayDiffuse ),
    SAMPLER2DARG( lOverlayNormal  ),
    SAMPLER2DARG( lOverlayMasks   ),
    in    vec3  lWorldPositionVec3,
    in    float lfTriplanarBlendPower,
    in    float lfFinalBlendFactor,
    inout vec3  lColourVec3,
    inout vec3  lNormalVec3,
    inout float lfRoughness )
{
    STATIC_CONST float kfThreshold = 0.005;

    vec3  m         = pow( abs( lNormalVec3 ), float2vec3( lfTriplanarBlendPower ) );
    vec3  factor    = m / ( m.x + m.y + m.z );

    vec2  lCoord1Vec2   = lWorldPositionVec3.yz;
    vec2  lCoord2Vec2   = lWorldPositionVec3.zx;
    vec2  lCoord3Vec2   = lWorldPositionVec3.xy;

    // TODO(sal): get normals blending sorted
    //vec3  lNormal1Vec3  = float2vec3( 0.0 );
    //vec3  lNormal2Vec3  = float2vec3( 0.0 );
    //vec3  lNormal3Vec3  = float2vec3( 0.0 );

    vec3  lOvlColourVec3 = float2vec3( 0.0 );
    float lfOvlRoughness = 0.0;

    if ( factor.x > kfThreshold )
    {
        lOvlColourVec3 += factor.x * texture2DComputeGrad( lOverlayDiffuse, lCoord1Vec2 ).rgb;
        lfOvlRoughness += factor.x * texture2DComputeGrad( lOverlayMasks,   lCoord1Vec2 ).r;
    }
    if ( factor.y > kfThreshold )
    {
        lOvlColourVec3 += factor.y * texture2DComputeGrad( lOverlayDiffuse, lCoord2Vec2 ).rgb;
        lfOvlRoughness += factor.y * texture2DComputeGrad( lOverlayMasks,   lCoord2Vec2 ).r;
    }
    if ( factor.z > kfThreshold )
    {
        lOvlColourVec3 += factor.z * texture2DComputeGrad( lOverlayDiffuse, lCoord3Vec2 ).rgb;
        lfOvlRoughness += factor.z * texture2DComputeGrad( lOverlayMasks,   lCoord3Vec2 ).r;
    }

    lColourVec3 = mix( lColourVec3, lOvlColourVec3, lfFinalBlendFactor );
    lfRoughness = mix( lfRoughness, lfOvlRoughness, lfFinalBlendFactor );
}
#endif

//-----------------------------------------------------------------------------
///
///     Fragment Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if defined( D_DEPTH_CLEAR ) || IMPOSTER_ENABLE_FRAGMENT_DEPTH_WRITING
    #if defined( D_OUTPUT_MOTION_VECTORS ) && defined( D_FORWARD_RENDERER )
        #if defined ( IMPOSTER_ENABLE_FRAGMENT_DEPTH_WRITING )
            FRAGMENT_MAIN_COLOUR012_DEPTH_SRT_VARIANT(HAS_MOTION_VECTORS)
        #else
            FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(HAS_MOTION_VECTORS)
        #endif
    #else
        FRAGMENT_MAIN_COLOUR_DEPTH_SRT
    #endif
#else
    #if defined( D_DEPTHONLY )
        #if defined( _F11_ALPHACUTOUT ) && defined( D_PLATFORM_PROSPERO )
        VOID_MAIN_REZ_SRT
        #else
        VOID_MAIN_SRT
        #endif
    #else
        //TF_BEGIN
        #if defined( D_OUTPUT_MOTION_VECTORS ) && defined( D_FORWARD_RENDERER )
        FRAGMENT_MAIN_COLOUR012_SRT_VARIANT(HAS_MOTION_VECTORS)
        #elif defined(D_BLOOM) || defined(D_DOF)
        FRAGMENT_MAIN_COLOUR01_SRT
        #elif defined( D_FADE )  && !defined( D_UI_OVERLAY )
        FRAGMENT_MAIN_COLOUR_SRT
        #else
        FRAGMENT_MAIN_COLOUR_EARLYZ_SRT
        #endif
    //TF_END
    #endif
#endif
{
#if defined( D_IMPOSTER ) && defined( _F09_TRANSPARENT ) && ( defined( _F13_UVANIMATION ) || defined( _F14_UVSCROLL ) )

    discard;

#else

    //-----------------------------------------------------------------------------
    ///
    ///     Fading
    ///
    //-----------------------------------------------------------------------------
    #if  defined( D_FADE )  && !defined( D_UI_OVERLAY )
    {   
        #if defined( _F44_IMPOSTER )
        vec4 lTexCoords = IN( mTexCoordsVec4 ) * lUniforms.mpCustomPerMaterial.gImposterDataVec4.x;
        #else
        vec4 lTexCoords = IN( mTexCoordsVec4 );
        #endif
            
        #if defined ( D_PLATFORM_SWITCH ) || defined ( D_PLATFORM_IOS )
        CheckFadeFragCoordNoVR(
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ),
            IN_SCREEN_POSITION,
            IN( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).x,
            256.0 );
        #else
        CheckFadeWorldSpaceCoord(
            SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gFadeNoiseMap),
            IN(mWorldPositionVec3_mfSpare).xyz,
            IN(mfFadeValueForInstance_mfLodIndex_mfShearMotionLength).x );
        #endif
    }
    #endif

#ifdef D_DEPTH_CLEAR
    FRAGMENT_COLOUR = vec4( 0.0, 0.0, 0.0, 0.0 );
    FRAGMENT_DEPTH = D_DEPTH_CLEARVALUE;
    return;
#else

    //-----------------------------------------------------------------------------
    ///
    ///     Diffuse Maps
    ///
    //-----------------------------------------------------------------------------
    vec4  lColourVec4;
    vec3  lNormalVec3         = vec3( 0.0, 1.0, 0.0 );
#ifdef D_USES_WORLD_POSITION
    vec3  lWorldPositionVec3  = IN( mWorldPositionVec3_mfSpare ).xyz;
#endif

#ifdef _F63_DISSOLVE
    float lDissolveEdge = 2.0f;
    if( lUniforms.mpCustomPerMaterial.gDissolveDataVec4.w != 0.0f )
    {
        //float dissolveDist = abs( dot( lUniforms.mpCustomPerMaterial.gDissolveDataVec4.xyz, lWorldPositionVec3 ) ) - abs( lUniforms.mpCustomPerMaterial.gDissolveDataVec4.w );
        float dissolveDist = dot(lUniforms.mpCustomPerMaterial.gDissolveDataVec4.xyz, lWorldPositionVec3) - lUniforms.mpCustomPerMaterial.gDissolveDataVec4.w;

        //if( ( lUniforms.mpCustomPerMaterial.gDissolveDataVec4.w < 0.0f && dissolveDist < 0.0f ) || ( lUniforms.mpCustomPerMaterial.gDissolveDataVec4.w >= 0.0f && dissolveDist >= 0.0f ) )
        if(dissolveDist > 0.0f)
        {
            lDissolveEdge = dissolveDist / 0.05f;

            if( lDissolveEdge > 1.0f )
            {
                discard;
            }
        }
    }
#endif

    vec4  lPrevScreenPosition = vec4( 0.0, 0.0, 0.0, 0.0 );    
    float lfDetailBlend       = 0.0;

    #ifdef _F55_MULTITEXTURE    
    float lfMultiTextureIndex = float(asint( lUniforms.mpCommonPerMesh.gUserDataVec4.w ) >> 24);
    #endif

    vec4 lTexCoordsVec4 = IN( mTexCoordsVec4 );
    #ifdef D_TEXCOORDS
    //{

        #ifdef D_DEFERRED_DECAL
        {
            lTexCoordsVec4 = GetDecalTexCoords( 
                                DEREF_PTR( lUniforms.mpPerFrame ),
                                DEREF_PTR( lUniforms.mpCommonPerMesh ),
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial,gBufferMap ),
                                IN( mScreenSpacePositionVec4 ),
                                lWorldPositionVec3 ).xyxy;
        }
        #endif

        float lfPixelSelfShadowing = 0.0;
        #if defined(_F20_PARALLAXMAP) && !defined( D_DEPTHONLY )
        {
           
            //vec3 lTangentSpaceViewVec3 = normalize( IN( mTangentSpaceViewPosVec3 ) - IN( mTangentSpaceFragPosVec3 ) );
            vec3 lTangentSpaceViewVec3 = normalize( IN( mTangentSpaceEyeVec3 ) );
                      
            lTexCoordsVec4 = GetParallaxTexCoords( 
                                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gParallaxMap ),
#ifdef _F16_DIFFUSE2MAP
                                lTexCoordsVec4.zw, 
#else
                                lTexCoordsVec4.xy, 
#endif
                                lTangentSpaceViewVec3,
                                IN( mTangentSpaceLightDirVec3 ),
                                lUniforms.mpCommonPerMesh.gParallaxMapDataVec4,
                                lfPixelSelfShadowing ).xyxy;
        }
        #endif

    #if defined( D_IMPOSTER ) && defined( _F12_BATCHED_BILLBOARD )
        if (lUniforms.mpCommonPerMesh.giShaderContextVariant == 0)
        {
            lTexCoordsVec4.y = 1.0 - lTexCoordsVec4.y;
        }
    #endif


    //}
    #endif

#if defined(_F44_IMPOSTER)
    vec4 lImposterDiffuse;
    vec4 lImposterMask;
    vec3 lImposterNormal;
    if (OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
    {
#if ENABLE_OCTAHEDRAL_IMPOSTERS
        {
            vec2 lTexCoords0 = lTexCoordsVec4.xy;
            vec2 lTexCoords1 = lTexCoordsVec4.zw;
            vec2 lTexCoords2 = IN( mImposterData0 ).xy;
            vec3 lBlendWeights = vec3( IN( mImposterData0 ).zw, 1.0);
            lBlendWeights.z = 1.0 - (lBlendWeights.x + lBlendWeights.y);
            bool lbFrameBlendEnabled = true;
            bool lbDepthReprojectionEnabled = true;

            vec3 lImposterViewNormal = IN( mImposterViewNormal ).xyz;

            float lfDecodedValue = lImposterViewNormal.x;            
            DecodeBitFromNormalFloat(
                lImposterViewNormal.x,
                lfDecodedValue,
                lbFrameBlendEnabled );
            lImposterViewNormal.x = lfDecodedValue;

            lfDecodedValue = lImposterViewNormal.y;
            DecodeBitFromNormalFloat(
                lImposterViewNormal.y,
                lfDecodedValue,
                lbDepthReprojectionEnabled );
            lImposterViewNormal.y = lfDecodedValue;

            vec2 lImposterFrame0 = IN(mImposterFrameXY_FrameProjectonVecZW0).xy;
            vec2 lImposterFrame1 = IN(mImposterFrameXY_FrameProjectonVecZW1).xy;
            vec2 lImposterFrame2 = IN(mImposterFrameXY_FrameProjectonVecZW2).xy;
            vec2 lFrameProjection0 = IN(mImposterFrameXY_FrameProjectonVecZW0).zw;
            vec2 lFrameProjection1 = IN(mImposterFrameXY_FrameProjectonVecZW1).zw;
            vec2 lFrameProjection2 = IN(mImposterFrameXY_FrameProjectonVecZW2).zw;
            mat3 lWorldNormalMat3;

#if defined ( D_INSTANCE ) && !defined ( D_PLATFORM_METAL )
            {
                lWorldNormalMat3 = IN( mWorldNormalMat3 );
            }
#else
            {
                MAT3_SET_COLUMN( lWorldNormalMat3, 0, lUniforms.mpCommonPerMesh.gWorldNormalMat4[0].xyz );
                MAT3_SET_COLUMN( lWorldNormalMat3, 1, lUniforms.mpCommonPerMesh.gWorldNormalMat4[1].xyz );
                MAT3_SET_COLUMN( lWorldNormalMat3, 2, lUniforms.mpCommonPerMesh.gWorldNormalMat4[2].xyz );
            }
#endif
            float lfImposterDepth;
            ImposterFrag(
                lBlendWeights.xyz,
                lUniforms.mpCustomPerMaterial.gImposterDataVec4, lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4,
                lImposterViewNormal,
                lWorldNormalMat3, lbFrameBlendEnabled, lbDepthReprojectionEnabled,
                lTexCoords0, lTexCoords1, lTexCoords2,
                lImposterFrame0, lImposterFrame1, lImposterFrame2,
                lFrameProjection0, lFrameProjection1, lFrameProjection2,
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gDiffuseMap), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gMasksMap), SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gNormalMap),
                lImposterDiffuse, lImposterMask, lImposterNormal, lfImposterDepth
            );

#if IMPOSTER_ENABLE_FRAGMENT_DEPTH_WRITING
            FRAGMENT_DEPTH = LinearToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, ( lfImposterDepth * IN( mImposterViewNormal ).w ) + ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, IN_SCREEN_POSITION.z ) );
#endif
        }


#endif
    }
    else
    {
#if IMPOSTER_ENABLE_FRAGMENT_DEPTH_WRITING
        FRAGMENT_DEPTH = IN_SCREEN_POSITION.z;
#endif
    }
#endif

    #if defined( _F01_DIFFUSEMAP ) && !defined( _F47_REFLECTION_PROBE )
    {
#if defined(_F44_IMPOSTER)
        if (OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
        {
            lColourVec4 = lImposterDiffuse;
        }
        else
#endif
        {
            #ifdef _F55_MULTITEXTURE
                    lColourVec4 = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gDiffuseMap ), vec3(lTexCoordsVec4.xy, lfMultiTextureIndex));
                    //lColourVec4 = vec4(1.0, 0.0, 0.0, 1.0);
            #else
                    lColourVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gDiffuseMap ), lTexCoordsVec4.xy);
            #endif
        }

        #if !defined( _F09_TRANSPARENT ) && !defined( _F11_ALPHACUTOUT ) && !defined( _F33_SHELLS ) && !defined( _F45_VERTEX_BLEND )
        {
            // There is not a transparent texture
            lColourVec4.a = 1.0;
        }
        #endif

        #if defined( _F05_INVERT_ALPHA )
        {            
            lColourVec4.a = 1.0 - lColourVec4.a;
        }
        #endif
    }
    #elif !defined( _F47_REFLECTION_PROBE )
    {
        lColourVec4 = IN( mMaterialVec4 );
    }
    #endif

    #ifdef D_MASKS
    //{
        vec4 lMasks;
        #if defined(_F44_IMPOSTER)
        if (OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
        {
            lMasks = lImposterMask;
        }
        else
        #endif
        { 
            #ifdef _F55_MULTITEXTURE
                lMasks = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gMasksMap ), vec3(lTexCoordsVec4.xy, lfMultiTextureIndex) );
            #else
                lMasks = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gMasksMap ), lTexCoordsVec4.xy);
            #endif
        }
        
    //}
    #endif

    #ifdef D_FEATURES
    //{
        #ifdef _F55_MULTITEXTURE
            vec4 lFeatures = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gFeaturesMap ), vec3( lTexCoordsVec4.xy, lfMultiTextureIndex ) );
        #else
            vec4 lFeatures = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gFeaturesMap ), lTexCoordsVec4.xy );
        #endif
    //}
    #endif

    #ifdef _F53_COLOURISABLE
    {
        #ifdef _F54_COLOURMASK
        {
            // Tint by mask
            float lfColourMask  = lMasks.b;
            int   liMaskIdx     = int( round( lfColourMask * 4.0 ) ) - 1;

            if ( liMaskIdx >= 0 )
            {
                int lPackedColour = asint( lUniforms.mpCommonPerMesh.gUserDataVec4[ liMaskIdx ] );
                
                vec3 lTintColourVec3;
                lTintColourVec3.x = float( ( lPackedColour & 0xFF0000 ) >> 16 ) / 255.0;
                lTintColourVec3.y = float( ( lPackedColour & 0x00FF00 ) >>  8 ) / 255.0;
                lTintColourVec3.z = float( ( lPackedColour & 0x0000FF )       ) / 255.0;

                lColourVec4.rgb *= lTintColourVec3;
            }
        }
        #else
        {
            // Change HUE
            vec3  lOriginalHSVVec3 = RGBToHSV(lColourVec4.xyz);
            vec3  lTintHSVVec3     = RGBToHSV( GammaCorrectInput01( lUniforms.mpCommonPerMesh.gUserDataVec4.xyz ) );

            vec3  lHSVColourVec3 = lOriginalHSVVec3;
            lHSVColourVec3.x    = lTintHSVVec3.x;
            lHSVColourVec3.y    = lTintHSVVec3.y;  //Not working? Try copying across the saturation too. If that fixes it, the problem is the source is too white/doesn't have enough saturation

            if( lTintHSVVec3.y > 0.0 )
            {
                lColourVec4.rgb = HSVToRGB( lHSVColourVec3 );
            }
        }
        #endif
    }
    #endif
    
    #ifdef _F16_DIFFUSE2MAP
    //{
    vec4 lDiffuse2Vec4;

        lDiffuse2Vec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gDiffuse2Map ), lTexCoordsVec4.zw );
        lfDetailBlend = lDiffuse2Vec4.a;

        #if defined( _F45_VERTEX_BLEND ) && !defined ( D_DEPTHONLY )
        {
            float lfHeight0 = lColourVec4.a;
            float lfHeight1 = lDiffuse2Vec4.a;

            float lfHeightDiff = ( lfHeight1 - lfHeight0 );

            float lBlend = IN( mColourVec4 ).r * 2.0 - 1.0 + lfHeightDiff;
            #if !defined( _F58_USE_CENTRAL_NORMAL )
                lBlend *= lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.w;
            #endif
            lfDetailBlend = smoothstep( -1.0, 1.0, lBlend );

            lColourVec4.a = 1.0;
        }
        #endif

        #ifndef _F17_MULTIPLYDIFFUSE2MAP
        {
            lColourVec4.rgb = mix( lColourVec4.rgb, lDiffuse2Vec4.rgb, lfDetailBlend );
        }
        #endif

    //}
    #endif

    #if defined( _F42_DETAIL_NORMAL ) && !defined( D_DETAIL )
        vec4  lDetailNormalVec4 = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDetailNormalMap ), lTexCoordsVec4.zw );
    #endif

    #ifdef _F33_SHELLS
    {
        float lfFurNoiseScale      = lUniforms.mpCustomPerMesh.gCustomParams02Vec4.x;
        float lfFurNoiseThickness  = lUniforms.mpCustomPerMesh.gCustomParams02Vec4.y;
        float lfFurNoiseTurbulence = lUniforms.mpCustomPerMesh.gCustomParams02Vec4.z;
        float lfFurTurbulenceScale = lUniforms.mpCustomPerMesh.gCustomParams01Vec4.w;

        if( lfFurNoiseScale > 0.0 )
        {
            vec2 lBnTexCoords = lTexCoordsVec4.xy * lfFurNoiseScale;

            if( lfFurNoiseTurbulence > 0.0 )
            {
                #if defined ( D_DEPTHONLY )
                lfFurNoiseTurbulence *= (1.0 - IN( mWorldPositionVec3_mfSpare ).w);
                #else
                lfFurNoiseTurbulence *= (1.0 - IN( mColourVec4 ).a);
                #endif

                vec2 lTurbulenceCoords = lBnTexCoords * lfFurTurbulenceScale;

                lBnTexCoords.x -= lfFurNoiseTurbulence * 0.5;
                lBnTexCoords.y -= lfFurNoiseTurbulence * 0.5;

                lBnTexCoords.x += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ), lTurbulenceCoords ).r * lfFurNoiseTurbulence;
                lBnTexCoords.y += texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ), (lTurbulenceCoords+vec2(0.2,0.4)) ).r * lfFurNoiseTurbulence;
            }

            float lfFurNoise = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gFadeNoiseMap ), lBnTexCoords ).r * 2.0;
            lfFurNoise = ( mix( lfFurNoiseThickness, 1.0 + lfFurNoiseThickness, lfFurNoise ) );
            lColourVec4 *= lfFurNoise;
        }
    }
    #endif

    #if defined ( _F21_VERTEXCOLOUR ) && !defined( _F45_VERTEX_BLEND ) && !defined( D_DISREGARD_VERTCOLOUR ) && defined ( D_DEPTHONLY )
    {
        lColourVec4.a *= IN( mWorldPositionVec3_mfSpare ).w;
    }
    #elif defined ( _F21_VERTEXCOLOUR ) && !defined( _F45_VERTEX_BLEND ) && !defined( D_DISREGARD_VERTCOLOUR ) && !defined(_F15_WIND)
    {
        lColourVec4 *= IN( mColourVec4 );
    }
    #elif defined( _F33_SHELLS ) && !defined ( D_DEPTHONLY )
    {
        lColourVec4.a = saturate( lColourVec4.a - (1.0 - IN( mColourVec4 ).a) );
    }
    #endif

    #if defined( D_OUTPUT_MOTION_VECTORS )

#ifdef D_PLATFORM_METAL
        if(HAS_MOTION_VECTORS)
#endif
        {
        lPrevScreenPosition = IN( mPrevScreenPosition );

            #if 0 // defined( D_TEXCOORDS ) && defined( _F14_UVSCROLL )
            {
                // note! this works, but I have had to turn it off - fast UV scrolling effects in
                // a few places are causing the motion blur to go nuts. Left here to enable possibly
                // a material flag to be added someday.

                // compute texture UV change
                vec2 lvTextureSpaceTexMotion = IN( mPrevTexCoordsVec4 ).xy - IN( mTexCoordsVec4 ).xy;

                // texture UV change is approximately (first-order taylor, no perspective)
                // dU = ddx(U) * dX + ddy(U) * dY
                // dV = ddx(V) * dX + ddy(V) * dY

                float dU = lvTextureSpaceTexMotion.x;
                float dV = lvTextureSpaceTexMotion.y;

                float ddxU = dFdx( lTexCoordsVec4.x );
                float ddyU = dFdy( lTexCoordsVec4.x );

                float ddxV = dFdx( lTexCoordsVec4.y );
                float ddyV = dFdy( lTexCoordsVec4.y );

                // now solve for dX and dY using Cramer's rule
                float lfDenom = ddxU * ddyV - ddxV * ddyU;

                float dX = ( dU * ddyV - dV * ddyU ) / lfDenom;
                float dY = ( ddxU * dV - ddxV * dU ) / lfDenom;

                // and again, first-order Taylor to estimate prev screen pos accounting for UV
                vec4 lTexPositionChange = dFdx( lPrevScreenPosition ) * dX + dFdy( lPrevScreenPosition ) * dY;
                // lPrevScreenPosition += lTexPositionChange;
            }
    #endif
        }

    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     Transparency
    ///
    //-----------------------------------------------------------------------------

    #if defined( D_LIT_FORWARD )
    if (lUniforms.mpCommonPerMesh.giShaderContextVariant == 1)
    {
        float lfDistance = length((lUniforms.mpCustomPerMaterial.gPlaneSpotPositionVec4).xyz - lWorldPositionVec3);
        float lfAngle = 1.5707963268 * saturate(lfDistance * 0.125);  //Radius 8 (1/0.125)
        lColourVec4.a *= cos(lfAngle);
    }
    #endif
 
    #ifdef _F22_TRANSPARENT_SCALAR
    {
        lColourVec4.a *= lUniforms.mpCustomPerMaterial.gMaterialColourVec4.a;
    }
    #endif

    // Discard fully transparent pixels
    #if !defined(SM_CODE) && ( defined( _F09_TRANSPARENT ) || defined( _F22_TRANSPARENT_SCALAR ) || defined( _F11_ALPHACUTOUT ) )
    {
        #if (!defined(D_ZEQUALS)) || (defined( _F44_IMPOSTER ) && defined( D_PLATFORM_SWITCH ))
        if( lColourVec4.a < kfAlphaThreshold )
        {
            discard;
        }        
        #endif  

        #ifdef _F11_ALPHACUTOUT
        {
            #if defined( _F33_SHELLS )
                // I think if we have got here with shells (ie. not discarded based on alpha above,
                // then we can just output 1.0 as it's not going to be blended anyway. (otherwise
                // it uses the alpha to blend on the discovery page which normally wouldn't happen
                // with alphacutout).
                lColourVec4.a = 1.0;
            #else
                lColourVec4.a = smoothstep( kfAlphaThreshold, kfAlphaThresholdMax, lColourVec4.a );
            #endif
        }
        #endif
    }
    #endif

#if defined( _F60_ACUTE_ANGLE_FADE ) && !defined( D_IMPOSTER ) && !defined( _F44_IMPOSTER )
    vec3 lViewDirVec3 = normalize( lUniforms.mpPerFrame.gViewPositionVec3 - IN( mWorldPositionVec3_mfSpare ).xyz );

    float lfNdotCam = 1.0 - acos( abs( dot( lViewDirVec3, normalize( IN( mTangentSpaceNormalVec3 ).xyz ) ) ) ) / ( 3.14 * 0.5 );

    if( lfNdotCam < 0.5 )
    {
        CheckFade(
            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial,gFadeNoiseMap ),
            IN( mScreenSpacePositionVec4 ),
            lfNdotCam * 2.0,
            lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy,
            vec3(0.0,0.0,1.0) );
    }
#endif

#if defined( D_DEPTHONLY )

#if defined( D_PLATFORM_OPENGL )
    FRAGMENT_COLOUR  = vec4( 0.0 );
#endif

#else

    //-----------------------------------------------------------------------------
    ///
    ///     Detail Maps
    ///
    //-----------------------------------------------------------------------------

    #ifdef D_DETAIL
    //{
        float lfImageIndex      = floor( lMasks.a * 10.0 );
        float lfIsBlendImage    = ceil( clamp( lUniforms.mpCustomPerMesh.gCustomParams02Vec4.z - lfImageIndex, 0.0, 1.0 ) );
        float lfIsMultiplyImage = 1.0 - lfIsBlendImage;

        lfImageIndex = mix( lfImageIndex, lUniforms.mpCustomPerMesh.gCustomParams02Vec4.z + (lfImageIndex - (10.0 - lUniforms.mpCustomPerMesh.gCustomParams02Vec4.w)), lfIsMultiplyImage );

        float lfDetailScale         = mix( lUniforms.mpCustomPerMesh.gCustomParams02Vec4.x, lUniforms.mpCustomPerMesh.gCustomParams02Vec4.y, lfIsBlendImage );
        vec4  lDetailNormalVec4     = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gDetailNormalMap ), vec3((lTexCoordsVec4.xy * lfDetailScale), lfImageIndex ) );

        float lfBlendSize = (step( 0.5, 1.0 - lMasks.b ) * 0.4) + 0.1;
        float lfBlendHeightMin = clamp( 1.0 - lDetailNormalVec4.r, 0.05, 0.95 );
        float lfBlendHeightMax = min( lfBlendHeightMin + lfBlendSize, 0.95 );

        lfDetailBlend = smoothstep( lfBlendHeightMin, lfBlendHeightMax, lMasks.b );

        #if defined( _F41_DETAIL_DIFFUSE ) // || defined( _F39_METALLIC_MASK ) || defined( _F35_GLOW_MASK )
        {
            vec4   lDetailDiffuseVec4 = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gDetailDiffuseMap ), vec3((lTexCoordsVec4.xy * lfDetailScale), lfImageIndex ) );

            //#ifdef _F41_DETAIL_DIFFUSE
            {
                lColourVec4.rgb = mix( lColourVec4.xyz, lDetailDiffuseVec4.xyz,                   lfIsBlendImage    * lfDetailBlend );
                lColourVec4.rgb = mix( lColourVec4.xyz, lColourVec4.xyz * lDetailDiffuseVec4.xyz, lfIsMultiplyImage * lfDetailBlend );
            }
            //#endif

            // Disable detail metallic and glow; will probably need a detail mask at some point???
            /*
            #if  !defined( _F07_UNLIT ) && defined( _F39_METALLIC_MASK )
                lHighAlpha = mix( lHighAlpha, GetUpperValue( lDetailDiffuseVec4.a ), lfIsBlendImage * lfDetailBlend );
                lHighAlpha = mix( lHighAlpha, GetUpperValue( lDetailDiffuseVec4.a ) * lHighAlpha, lfIsMultiplyImage * lfDetailBlend );
            #endif

            #if defined( _F34_GLOW ) && defined( _F35_GLOW_MASK ) && !defined( _F09_TRANSPARENT )
                lLowAlpha = mix( lLowAlpha, GetLowerValue( lDetailDiffuseVec4.a ), lfIsBlendImage * lfDetailBlend );
                lLowAlpha = mix( lLowAlpha, GetLowerValue( lDetailDiffuseVec4.a ) * lLowAlpha, lfIsMultiplyImage * lfDetailBlend );
            #endif
            */
        }
        #endif
    //}
    #endif

    #if defined( _F24_AOMAP )
    {
        lColourVec4.rgb *= lMasks.r;
    }
    #endif

    #ifdef _F17_MULTIPLYDIFFUSE2MAP
    {
        lColourVec4.rgb *= lDiffuse2Vec4.r;
    }
    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     Normals
    ///
    //-----------------------------------------------------------------------------
    vec3 lNormal1Vec3;
    vec3 lNormal2Vec3;

    #if defined( _F56_MATCH_GROUND ) && defined( D_USES_VERTEX_NORMAL ) && defined( _F36_DOUBLESIDED ) && defined( D_PLATFORM_SWITCH )
    {
        lNormalVec3 = normalize( IN(mTangentSpaceNormalVec3) );
    }
    #elif defined( _F03_NORMALMAP )
    //{
        #ifdef _F43_NORMAL_TILING
        {    
            lTexCoordsVec4.xy *= lUniforms.mpCustomPerMesh.gCustomParams01Vec4.z;
        }
        #endif
        
        #if defined( _F44_IMPOSTER )
            vec3 lNormalTexVec3;
            if(OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
            {
                lNormalTexVec3 = lImposterNormal;
            }
            else
            {
#if !defined ( D_PLATFORM_SWITCH )
                vec4 lTexColour = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gNormalMap), lTexCoordsVec4.xy, 1.0);
#else
                vec4 lTexColour = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gNormalMap), lTexCoordsVec4.xy);
#endif
                lNormalTexVec3 = -normalize(lTexColour.xyz * 2.0 - 1.0);
            }
        #else
            #ifdef _F55_MULTITEXTURE
                    vec4 lTexColour = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gNormalMap ), vec3(lTexCoordsVec4.xy, lfMultiTextureIndex ));
            #else
                    vec4 lTexColour = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial,gNormalMap ), lTexCoordsVec4.xy );
            #endif 
        
        vec3 lNormalTexVec3 = DecodeNormalMap( lTexColour );
        #endif
            
         lNormalVec3 = lNormalTexVec3;
    //}
    #elif defined( D_USES_VERTEX_NORMAL )
    {
        lNormalVec3 = IN( mTangentSpaceNormalVec3 );
    }
    #endif
    lNormal1Vec3 = lNormalVec3;
    lNormal2Vec3 = lNormalVec3;

    #ifdef _F42_DETAIL_NORMAL
    {
        vec3 lDetailNormalVec3 = DecodeNormalMap( lDetailNormalVec4 );

        #ifdef D_DETAIL
        lDetailNormalVec3   = mix( lDetailNormalVec3, lNormalVec3, 0.5 );
        lDetailNormalVec3.z = lNormalVec3.z;
        #endif

        lNormal2Vec3 = lDetailNormalVec3;
        lNormalVec3 = mix( lNormalVec3, lDetailNormalVec3, lfDetailBlend );
    }
    #endif
    
    #if defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL )
    mat3 lTangentSpaceMat;
	vec3 lTangentSpaceNormal = lNormalVec3;

    #if defined( _F56_MATCH_GROUND ) && defined( _F36_DOUBLESIDED ) && defined( D_USES_VERTEX_NORMAL ) && defined( D_PLATFORM_SWITCH )

    #else
    {

        #ifdef _F52_DECAL_NORMAL
        {
            lTangentSpaceMat = GetCotangentFrame( lWorldPositionVec3, lUniforms.mpPerFrame.gViewPositionVec3, lTexCoordsVec4.xy );
        }
        #else
        {
            lTangentSpaceMat[0] = IN( mTangentMatRow1Vec3 );
            lTangentSpaceMat[1] = IN( mTangentMatRow2Vec3 );
            lTangentSpaceMat[2] = IN( mTangentMatRow3Vec3 );
        }
        #endif


        #if defined( _F36_DOUBLESIDED ) && defined (_F18_UVTILES)
        // Tree with flat leaves which use multiple textures have different lighting
        vec3 lFaceNormalVec3;
        {
            vec3 lEyeVec3 = normalize(lUniforms.mpPerFrame.gViewPositionVec3 - lWorldPositionVec3);

            vec3 ddxWp = dFdx(lWorldPositionVec3);
            vec3 ddyWp = dFdy(lWorldPositionVec3);
            lFaceNormalVec3 = normalize(cross(ddyWp, ddxWp));

            if (dot(lFaceNormalVec3, lEyeVec3) < 0.0)
            {
                lFaceNormalVec3 = reflect(IN(mTangentSpaceNormalVec3), ddxWp);
            }
            else
            {
                lFaceNormalVec3 = IN(mTangentSpaceNormalVec3);
            }
        }
        lTangentSpaceMat[ 1 ] = cross( lFaceNormalVec3, lTangentSpaceMat[ 0 ] ) * IN( mWorldPositionVec3_mfSpare ).w;
        lTangentSpaceMat[ 2 ] = lFaceNormalVec3;
        #endif // #if defined( _F36_DOUBLESIDED ) && defined (_F18_UVTILES)

#if defined(_F44_IMPOSTER)
        if (!OCTAHEDRAL_IMPOSTERS_ENABLED(lUniforms.mpCustomPerMaterial.gImposterQualitySettingsVec4))
#endif
        {
            lNormalVec3 = normalize( MUL( lTangentSpaceMat, lNormalVec3 ) ); 
            lNormal1Vec3 = normalize( MUL( lTangentSpaceMat, lNormal1Vec3 ) );
            lNormal2Vec3 = normalize( MUL( lTangentSpaceMat, lNormal2Vec3 ) );
            
#if defined( _F36_DOUBLESIDED ) && !defined (_F18_UVTILES)
            {
                if (!FRAGMENT_FRONTFACE)
                {
                    lNormalVec3 = -lNormalVec3;
                }
            }
#endif
		}
    }
	#endif // defined( _F56_MATCH_GROUND ) && defined( _F36_DOUBLESIDED ) && defined( D_USES_VERTEX_NORMAL ) && defined( D_PLATFORM_SWITCH )
    #endif // defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL )

    #if defined( _F47_REFLECTION_PROBE )
    {
        #if defined( D_PLATFORM_PC )
        mat3  lWorldMat3        = mat3( lUniforms.mpCommonPerMesh.gWorldMat4 );
        #elif defined(D_PLATFORM_METAL)
        mat3  lWorldMat3        = cast2mat3( lUniforms.mpCommonPerMesh.gWorldMat4 );
        #else
        mat3  lWorldMat3        = (mat3)lUniforms.mpCommonPerMesh.gWorldMat4;
        #endif

        #if defined( D_USE_SCREEN_POSITION )
        vec4 lScreenPositionVec4 = IN( mScreenSpacePositionVec4 );
        vec3 lViewPosVec3        = RecreatePositionFromRevZDepth( 
                                        lScreenPositionVec4.z  / lScreenPositionVec4.w,
                                        lScreenPositionVec4.xy / lScreenPositionVec4.w * 0.5 + 0.5,
                                        float2vec3( 0.0 ),
                                        lUniforms.mpPerFrame.gInverseViewProjectionMat4 );

        #elif defined( D_USES_WORLD_POSITION )
        vec3 lViewPosVec3        = lWorldPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3;
        #endif

        vec3 lReflVec3           = normalize( reflect( normalize( lViewPosVec3 ), lNormalVec3 ) );
        lReflVec3                = MUL( transpose( lWorldMat3 ), lReflVec3 ) * vec3( -1.0, 1.0, -1.0 );
        lColourVec4              = textureCubeLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gDiffuseMap ), lReflVec3, 0.0 );
    }
    #endif

    //-----------------------------------------------------------------------------
    ///
    ///     Lighting
    ///
    //-----------------------------------------------------------------------------
    
    float lfRoughness  = 1.0;
    float lfMetallic   = 0.0;
    float lfSubsurface = 0.0;

    #ifndef _F07_UNLIT
    {
        #ifndef _F44_IMPOSTER

            #ifdef _F25_ROUGHNESS_MASK
            {
                lfRoughness = lMasks.g;

                #ifdef D_DETAIL
                {
                    lfRoughness  = mix( lfRoughness, lDetailNormalVec4.b,               lfIsBlendImage    * smoothstep( lfBlendHeightMin, lfBlendHeightMax, lMasks.b ) );
                    lfRoughness  = mix( lfRoughness, lDetailNormalVec4.b * lfRoughness, lfIsMultiplyImage * smoothstep( lfBlendHeightMin, lfBlendHeightMax, lMasks.b ) );
                }
                #endif

                lfRoughness = 1.0 - lfRoughness;
            }
            #endif

            lfRoughness *= lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.x;

        #else
        
            lfRoughness = lMasks.g;

        #endif

        #ifndef _F44_IMPOSTER

            #ifdef _F39_METALLIC_MASK
            {
                lfMetallic = lMasks.r;

                #ifdef D_DETAIL
                {
                    lfMetallic = mix( lfMetallic, lDetailNormalVec4.b,               lfIsBlendImage    * smoothstep( lfBlendHeightMin, lfBlendHeightMax, lMasks.b ) );
                    lfMetallic = mix( lfMetallic, lDetailNormalVec4.b * lfRoughness, lfIsMultiplyImage * smoothstep( lfBlendHeightMin, lfBlendHeightMax, lMasks.b ) );
                }
                #endif        
            }
            #else 
            {
                lfMetallic = lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.z;
            }
            #endif

        #else

            lfMetallic = lMasks.b;

        #endif

        #if defined( _F40_SUBSURFACE_MASK ) && !defined( D_PLATFORM_METAL )
        {
            lfSubsurface = lMasks.r;
        }
        #endif

        #if defined( _F58_USE_CENTRAL_NORMAL ) && defined( _F03_NORMALMAP )
        {
            float lfDistance = length(lWorldPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3);
            lfDistance       = ( lfDistance - 20.0 ) / 30.0;
            lfDistance       = 1.0 - saturate( lfDistance );
            lNormalVec3      = mix( normalize( IN( mCenteralNormalVec3 ).xyz ), lNormalVec3, lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.w * lfDistance );
            //lNormalVec3 = normalize( IN( mCenteralNormalVec3 ).xyz );
        }
        #endif

    }
    #endif

    float lfGlow = 0.0;
    #if defined( _F34_GLOW ) && !defined( D_NO_GLOW )
    {
        #if defined( _F35_GLOW_MASK )
        {
            lfGlow = lUniforms.mpCustomPerMesh.gCustomParams01Vec4.y * lMasks.a;
        }
        #else
        {
            lfGlow = lUniforms.mpCustomPerMesh.gCustomParams01Vec4.y;
        }
        #endif
    }
    #endif 
    
    
    #if defined( _F56_MATCH_GROUND ) && !defined( D_DEPTHONLY )

        float lfDistance = length( lWorldPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3 );
        lfDistance       = ( lfDistance - 20.0 ) / 30.0;
        lfDistance       = 1.0 - saturate( lfDistance );
        lColourVec4.rgb = GetGrassColour( 
            DEREF_PTR( lUniforms.mpCommonPerMesh ), 
            DEREF_PTR( lUniforms.mpCustomPerMaterial ),
            lColourVec4.rgb,
            lWorldPositionVec3,
            lfDistance );
    #endif

    #if defined( D_ENV_OVERLAY ) && defined( D_FEATURES )
        #ifdef _F55_MULTITEXTURE
            // TODO(sal): :)
        #else
            int  liOvlIdx       = int(lUniforms.mpCustomPerMaterial.gBiomeDataVec4.z);
            
            ApplyEnviromentalOverlay(
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gOverlayDiffuseMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gOverlayNormalMap ),
                SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gOverlayMasksMap ),
                lWorldPositionVec3, 64.0, lFeatures[liOvlIdx], lColourVec4.rgb, lNormalVec3, lfRoughness );
        #endif
    #endif
        
    #ifdef SM_CODE
    {
        float lfSkGlobalTime = lUniforms.mpPerFrame.gfTime;
        vec4 lSkUserData = lUniforms.mpCommonPerMesh.gUserDataVec4;

        #if defined ( _F21_VERTEXCOLOUR ) || defined( _F45_VERTEX_BLEND )
        vec4 lSkVertexColour = IN(mColourVec4);
        #else
        vec4 lSkVertexColour = float2vec4(0.0);
        #endif

        #if defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL )
        vec3 lSkInNormal     = lTangentSpaceNormal;
        vec3 lOutNormal      = lTangentSpaceNormal;
        #else
        vec3 lSkInNormal     = lNormalVec3;
        vec3 lOutNormal      = lNormalVec3;
        #endif
        
        vec3 lSkInWorldNormal1 = lNormal1Vec3;     
        vec3 lSkInWorldNormal2 = lNormal2Vec3;     
        vec3 lSkInWorldNormal = lNormalVec3;     

        #if defined( _F33_SHELLS )
        float lfShellHeight   = IN( mColourVec4 ).a;
        #else
        float lfShellHeight   = 0.0;
        #endif

        #if defined( D_SK_USE_LOCAL_POSITION )
        vec3 lSkLocalPositionVec3 = IN(mLocalPositionVec4).xyz;
        #else
        vec3 lSkLocalPositionVec3 = float2vec3(0.0);
        #endif

        #if defined( D_SM_FEATURES_MAP )
        vec4 lSkFeaturesVec4 = lFeatures;
        #else
        vec4 lSkFeaturesVec4 = float2vec4(0.0);
        #endif

        vec3 lSkWorldPositionVec3 = lWorldPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lSkNodePositionVec3  = lUniforms.mpCommonPerMesh.gWorldMat4[ 3 ].xyz - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;

        vec3 lSkPlanetPositionVec3 = float2vec3(0.0);
        vec3 lSkSunPositionVec3    = lUniforms.mpCustomPerMaterial.gSunPositionVec4.xyz * 100000.0;
        vec2 lSkBiomeDataVec2      = lUniforms.mpCustomPerMaterial.gBiomeDataVec4.xy;

        vec3 lSkCameraPositionVec3  = lUniforms.mpPerFrame.gViewPositionVec3 - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
        vec3 lSkCameraDirectionVec3 = MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 );

        vec3 lSkPositionDisplacement = float2vec3(0.0);
        vec3 lSkNormalDisplacement   = float2vec3(0.0);

        vec3 lfOutLocalPosition   = float2vec3(0.0);
        vec3 lvOutShellsOffset    = float2vec3(0.0);
        vec4 lfOutColour          = lColourVec4;
        float lfOutMetallic       = lfMetallic;
        float lfOutRoughness      = lfRoughness;
        float lfOutSubsurface     = lfSubsurface;
        float lfOutGlow           = lfGlow;
        vec2 lSkUv1               = lTexCoordsVec4.xy;
        vec2 lSkUv2               = lTexCoordsVec4.zw;
#if defined( D_USE_SCREEN_POSITION )
        vec2 lSkUvScreen          = SCREENSPACE_AS_RENDERTARGET_UVS( (IN( mScreenSpacePositionVec4 ).xy / IN( mScreenSpacePositionVec4 ).w) * 0.5 + 0.5 );
#else
        vec2 lSkUvScreen          = vec2(0.0,  0.0);
#endif

        SM_CODE

        lColourVec4  = lfOutColour;
        lfMetallic   = lfOutMetallic;
        lfRoughness  = lfOutRoughness;
        lfSubsurface = lfOutSubsurface;
        lfGlow       = max( lfOutGlow, 0.0 );

        #if defined( _F03_NORMALMAP ) || defined( _F42_DETAIL_NORMAL )
        lNormalVec3 = normalize( MUL( lTangentSpaceMat, lOutNormal ) );
        
        #if defined( _F36_DOUBLESIDED )
        {
            if (!FRAGMENT_FRONTFACE)
            {
                lNormalVec3 = -lNormalVec3;
            }
        }
        #endif
        
        #if defined( _F58_USE_CENTRAL_NORMAL ) && defined( _F03_NORMALMAP )
        {
            float lfDistance = length(lWorldPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3);
            lfDistance       = ( lfDistance - 20.0 ) / 30.0;
            lfDistance       = 1.0 - saturate( lfDistance );
            lNormalVec3      = mix( normalize( IN( mCenteralNormalVec3 ).xyz ), lNormalVec3, lUniforms.mpCustomPerMaterial.gMaterialParamsVec4.w * lfDistance );
            //lNormalVec3 = normalize( IN( mCenteralNormalVec3 ).xyz );
        }
        #endif
        #endif

        // Discard fully transparent pixels
        #if ( defined( _F09_TRANSPARENT ) || defined( _F22_TRANSPARENT_SCALAR ) || defined( _F11_ALPHACUTOUT ) )
        {
            #if (!defined(D_ZEQUALS)) || (defined( _F44_IMPOSTER ) && defined( D_PLATFORM_SWITCH ))
            if( lColourVec4.a < kfAlphaThreshold )
            {
                discard;
            }
            #endif  

            #ifdef _F11_ALPHACUTOUT
            {
                lColourVec4.a = smoothstep( kfAlphaThreshold, kfAlphaThresholdMax, lColourVec4.a );
            }
            #endif
        }
        #endif
    }
    #endif
    //-----------------------------------------------------------------------------
    ///
    ///     Write Output
    ///
    //-----------------------------------------------------------------------------

    int liMaterialID = 0;
    
    #if defined( _F34_GLOW ) && !defined( D_NO_GLOW ) && !defined( _F09_TRANSPARENT )
        liMaterialID |= D_GLOW;
    #endif

    #ifdef _F44_IMPOSTER
        if( lMasks.a > 0.5 )
        {
            liMaterialID = D_CLAMPAMBIENT;
        }
    #endif

    #ifdef _F10_NORECEIVESHADOW
        liMaterialID |= D_NORECEIVESHADOW;
    #endif

    #ifdef _F50_DISABLE_POSTPROCESS
        liMaterialID |= D_DISABLE_POSTPROCESS;
    #endif

    #ifdef _F07_UNLIT
        liMaterialID |= D_UNLIT;
    #endif

    //#if defined( _F57_DETAIL_OVERLAY )
    //    liMaterialID |= D_DETAILOVERLAY;
    //#endif

    #if defined( _F61_CLAMP_AMBIENT )
        liMaterialID |= D_CLAMPAMBIENT;
    #endif

    #ifdef _F49_DISABLE_AMBIENT
        if ( lUniforms.mpPerFrame.giDisableAmbientAllowed > 0  )
        {
            liMaterialID |= D_DISABLE_AMBIENT;
        }
    #endif

    #if defined( D_OUTPUT_MOTION_VECTORS ) && defined( D_SKINNING_UNIFORMS )
#ifdef D_PLATFORM_METAL
    if(HAS_MOTION_VECTORS)
#endif
    {
        if( IN( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).z > 0.0 )
        {
            liMaterialID |= D_CLAMP_AA;
        }
    }
    #endif

    #if defined( _F14_UVSCROLL )
        liMaterialID |= D_CLAMP_AA;
    #endif

    #if defined( _F08_REFLECTIVE )
        liMaterialID |= D_REFLECTIVE;
    #endif

    vec4 lOutColours0Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours1Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours2Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours3Vec4 = vec4(0, 0, 0, 0);
    vec4 lOutColours4Vec4 = vec4(0, 0, 0, 0);

#if defined( D_PLATFORM_PC )

    #if defined( D_INSTANCE )
    // Debug lod colouring. See giLodOverride in EgInstancedModelNode.cpp.
    // We only have 1 float in which to store useful info, so store either normal index, or index + 10 so
    // in the shader we can tell whether to render normally or debug. (see mLodIndexVec4 in unberfragment and vertex).
    int liLodIndex  = int( IN( mfFadeValueForInstance_mfLodIndex_mfShearMotionLength ).y );
    lColourVec4     = liLodIndex < 10 ? lColourVec4 : gaDebugLodColours[ liLodIndex - 10 ];
    #else
    int liLodIndex  = lUniforms.mpCommonPerMesh.giLodIndex;
    lColourVec4     = liLodIndex < 0 ? lColourVec4 : gaDebugLodColours[ liLodIndex ];
    #endif
#endif // defined( D_PLATFORM_PC )

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

	float lAO = 1.0;
#if defined(D_MASKS)
#if defined(_F24_AOMAP)

    lAO = lMasks.r;
#endif
#endif
//TF_END

#ifdef D_DEFERRED_DECAL
#if !defined( _F51_DECAL_DIFFUSE )
    lColourVec4.rgb = float2vec3(1.0f);
    lfRoughness = 1.0f;
    lfMetallic = 1.0f;
#endif
#if !defined( _F03_NORMALMAP )
    lNormalVec3 = normalize( IN( mCenteralNormalVec3 ).xyz );
    vec3 lEyeVec3 = normalize( lUniforms.mpPerFrame.gViewPositionVec3 - lWorldPositionVec3 );
    if( dot(  lNormalVec3, lEyeVec3 ) < 0.0 )
    {         
        lNormalVec3 = -lNormalVec3;
    }
#endif
#endif

    WriteOutput(
        lOutColours0Vec4,
        lOutColours1Vec4,
        lOutColours2Vec4,
        lOutColours3Vec4,
        lOutColours4Vec4,
        DEREF_PTR(lUniforms.mpPerFrame),
        DEREF_PTR(lUniforms.mpCommonPerMesh),
        DEREF_PTR(lUniforms.mpCustomPerMaterial),
        lColourVec4,
        lWorldPositionVec3,
        lNormalVec3,
        liMaterialID,
        lfMetallic,
        lfRoughness,
        lfSubsurface,
        lfGlow,
#ifdef D_USE_SCREEN_POSITION   
        IN(mScreenSpacePositionVec4).xyzw,
#else
        float2vec4(0.0),
#endif
#ifdef D_OUTPUT_MOTION_VECTORS   
        HAS_MOTION_VECTORS ? lPrevScreenPosition : float2vec4(0.0),
#else
        float2vec4(0.0),
#endif
#if !defined( D_DEFER ) && !defined( _F07_UNLIT ) && !defined( D_DEPTHONLY ) || defined( D_FORWARD_RENDERER )


        #ifdef D_PLATFORM_METAL
        GetInverseWorldUpTransform(lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4),
        #else
            #if !defined( D_DEFER ) && !defined( _F07_UNLIT ) && !defined( D_DEPTHONLY )
                IN(mUpMatrixMat3),
            #else
                mat3( 0.0 ),
            #endif
        #endif
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gShadowMap),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gCloudShadowMap),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gDualPMapBack),
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial,gDualPMapFront),
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
		lAO, 
#endif
        IN_SCREEN_POSITION.z,
		//TF_END
        FRAGMENT_FRONTFACE,
#if defined(_F20_PARALLAXMAP)
        lfPixelSelfShadowing
#else
        -1.0
#endif
        );

#if defined( _F30_REFRACTION )
    vec3  lViewDirVec3 = normalize(lWorldPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3);
    // A fresnel glow that is just currently used for camouflage/invisibility on the player character
    // We still want this part of the effect to work even on platforms with refraction disabled (as it an important
    // part of the effect), therefore we ignore D_REFRACT here.
    // This could be turned into a separate material flag if needed for perf reasons. But at the time of writing this comment
    // using the unused components of the refraction params uniform for this was deemed to having the winning tradeoffs as
    // material flags are in short supply.
    if (lUniforms.mpCommonPerMesh.gRefractionParamsVec4.z > 0.0)
    {
        float lfFresnelSharpness = lUniforms.mpCommonPerMesh.gRefractionParamsVec4.z;
        float lfBase = 1 - dot(lViewDirVec3, lNormalVec3);
        float lfExponential = pow(lfBase, 5.0);
        lOutColours0Vec4.a = saturate(lfExponential + lfFresnelSharpness * (1 - lfExponential)) * lUniforms.mpCommonPerMesh.gRefractionParamsVec4.w;
    }
#endif


#if defined( D_REFRACT )


    #if defined( _F30_REFRACTION )
    {
        vec3  lRefrDirVec3;
        vec3  lEncodedRefrDirVec3;
        float lfEta;

        lfEta           = lUniforms.mpCommonPerMesh.gRefractionParamsVec4.y;
        lRefrDirVec3    = refract( lViewDirVec3, lNormalVec3, lfEta );
        lRefrDirVec3   *= lUniforms.mpCommonPerMesh.gRefractionParamsVec4.x;

        #if defined( _F32_REFRACTION_MASK )
        lRefrDirVec3   *= lMasks.r;
        #endif

        // Ideally we'd use an RGB10A2 buffer to store the direction vector
        // instead we use an R11G11B10 buffer, as we can share it with
        // other existing ones. Small float encoding for the refraction
        // direction vector is kinda crap, so we encode the unorm representation
        // as a float and decode as unorm again on the other end;
        // should have the same precision as RGB10A2, except for the
        // blue channel, which will have 9 bit precision instead of ten :(
        // (the topmost exponent bit in each channel is forced to 0 to
        // avoid encoding inf or nan values )
        // we could really using format agnostic aliasing of render targets
        // memory...
        lEncodedRefrDirVec3 = EncodeVec3ToR11G11B10( lRefrDirVec3 );
        #if !defined( D_REFRACT_HIGH )
        FRAGMENT_COLOUR0 = lOutColours0Vec4;
        FRAGMENT_COLOUR1 = vec4( 1.0, 0.0, 0.0, lOutColours0Vec4.a );
        FRAGMENT_COLOUR2 = vec4( lEncodedRefrDirVec3, 1.0 );
        FRAGMENT_COLOUR3 = vec4( IN_SCREEN_POSITION.z, 0.0, 0.0, 1.0 );
        #else
        FRAGMENT_COLOUR0 = vec4( lOutColours0Vec4.rgb, 1.0 );
        FRAGMENT_COLOUR1 = vec4( lOutColours0Vec4.a,   0.0, 0.0, 1.0 );
        FRAGMENT_COLOUR2 = lOutColours0Vec4;
        FRAGMENT_COLOUR3 = vec4( 1.0, 0.0, 0.0, lOutColours0Vec4.a );
        FRAGMENT_COLOUR4 = vec4( lEncodedRefrDirVec3,  1.0 );
        FRAGMENT_COLOUR5 = vec4( IN_SCREEN_POSITION.z, 0.0, 0.0, 1.0 );
        #endif
    }
    #else
    {
        #if !defined( D_REFRACT_HIGH )
        FRAGMENT_COLOUR0 = lOutColours0Vec4;
        FRAGMENT_COLOUR1 = vec4( 1.0, 0.0, 0.0, lOutColours0Vec4.a );
        FRAGMENT_COLOUR2 = vec4( EncodeVec3ToR11G11B10( float2vec3( 0.0 ) ), 1.0 );
        FRAGMENT_COLOUR3 = vec4( IN_SCREEN_POSITION.z, 0.0, 0.0, 1.0 );
        #else
        FRAGMENT_COLOUR0 = vec4( lOutColours0Vec4.rgb, 1.0 );
        FRAGMENT_COLOUR1 = vec4( lOutColours0Vec4.a,   0.0, 0.0, 1.0 );
        FRAGMENT_COLOUR2 = lOutColours0Vec4;
        FRAGMENT_COLOUR3 = vec4( 1.0, 0.0, 0.0, lOutColours0Vec4.a );
        FRAGMENT_COLOUR4 = vec4( EncodeVec3ToR11G11B10( float2vec3( 0.0 ) ),  1.0 );
        FRAGMENT_COLOUR5 = vec4( IN_SCREEN_POSITION.z, 0.0, 0.0, 1.0 );
        #endif
    }
    #endif
#elif !defined( D_ATTRIBUTES )    

#if !defined( D_PLATFORM_METAL )
    if ( lOutColours0Vec4.a < ( 1.0 / 255.0 ) )
    {
        // this pixel will have no effect on the framebuffer -
        // don't write the stencil buffer plz
        discard;
    }
#endif

    #if defined( D_DEPTH_BASED_EFFECT )

    #if defined( D_UI_OVERLAY ) || defined( _F09_TRANSPARENT )
    if ( lUniforms.mpCustomPerMaterial.gUITransparencyVec4.x > 0.0 )
    #endif
    {
        vec2  lScreenPos  = SCREENSPACE_AS_RENDERTARGET_UVS( ( IN( mScreenSpacePositionVec4 ).xy / IN( mScreenSpacePositionVec4 ).w ) * 0.5 + 0.5 );
        #ifdef D_PLATFORM_ORBIS
        float lfDepthBuf  = DecodeDepthFromColour( texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gBufferMap ), vec3( lScreenPos, lUniforms.mpPerFrame.gVREyeInfoVec3.x ) ) );
        #else
        float lfDepthBuf  = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gBufferMap ), lScreenPos ) );
        #endif

        lfDepthBuf        = FastDenormaliseDepth(  lUniforms.mpPerFrame.gClipPlanesVec4, lfDepthBuf );
        float lfDepthThis = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, IN_SCREEN_POSITION.z );

        #if defined( D_UI_OVERLAY )
        {
            float   lfThresh  = 22.0;
            float   lfInterp  = 13.0;
            float   lfMinDist = 50.0;
            bool    lbTransp  = ( lfDepthThis > lfDepthBuf ) && ( lfDepthThis < lfThresh );

            if ( lbTransp )
            {
                float lfCoeff         = 1.0 - smoothstep( lfThresh - lfInterp, lfThresh, lfDepthThis );
                lOutColours0Vec4.a    = mix( lOutColours0Vec4.a, min( lOutColours0Vec4.a, 0.2 ), lfCoeff );
                lOutColours0Vec4.rgb *= float2vec3( lOutColours0Vec4.a );
            }
        }
        #elif defined( D_OCCLUDED )
        {
            float lfInvBlendRange   = 1.0 / 8.0;
            float lfMaxAlpha        = 0.15;
            float lfMinAlpha        = 0.06;
            float lfInterp          = saturate( ( lfDepthThis - lfDepthBuf ) * lfInvBlendRange );

            lOutColours0Vec4.a      = lOutColours0Vec4.a * mix( lfMaxAlpha, lfMinAlpha, lfInterp );
        }
        #else        
        {
            #if defined( _F06_BRIGHT_EDGE )
            {
                float lfDepthDiff     = min( 1.0, abs( lfDepthThis - lfDepthBuf ) * 0.33 );
                lfDepthDiff           = smoothstep( 0.0, 1.0 - 1.0e-3, lfDepthDiff );
                float lfEdgeCoeff     = 0.25 / ( lfDepthDiff + 1.0e-3 );
                lOutColours0Vec4.rgb *= lfEdgeCoeff;
            }
            #endif

            #if defined( _F09_TRANSPARENT )
            {
                float lfTransParam  = lUniforms.mpCustomPerMaterial.gUITransparencyVec4.x;
                float lfDepthFade   = saturate( ( lfDepthBuf - lfDepthThis ) * lfTransParam );
                lOutColours0Vec4.a *= lfDepthFade;
                lOutColours0Vec4.a  = min( lfDepthFade, lOutColours0Vec4.a );

            }
            #endif

            #if !defined( _F06_BRIGHT_EDGE ) && !defined( _F09_TRANSPARENT )
                #error Unexpected flag combination
            #endif
        }
        #endif
    }
    #endif

    #if defined( D_LIT_WITH_MASK )
    float lfAlpha    = saturate( ( ( DEREF_PTR( lUniforms.mpCustomPerMaterial ) ).gMaterialSFXColVec4.a * 2.0f ) );
    FRAGMENT_COLOUR0 = lOutColours0Vec4;
    FRAGMENT_COLOUR1 = vec4( lfAlpha, 0.0, 0.0, 1.0 );
    #else
        #if ( defined( D_OUTPUT_MOTION_VECTORS ) && defined( D_FORWARD_RENDERER ) ) || defined( D_BLOOM ) || defined( D_DOF )
            FRAGMENT_COLOUR0  = lOutColours0Vec4;
        #else
    FRAGMENT_COLOUR  = lOutColours0Vec4;
    #endif
        //TF_BEGIN
        #if defined( D_BLOOM ) || defined( D_DOF )
	       FRAGMENT_COLOUR1 = lOutColours1Vec4;
        #endif
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

#ifdef _F63_DISSOLVE

    if( lDissolveEdge <= 1.0f )
    {
        lOutColours0Vec4 = mix( lOutColours0Vec4, vec4( 1.0, 1.0, 1.0, 1.0 ), clamp( lDissolveEdge, 0.0f, 1.0f ) );

        liMaterialID |= D_UNLIT;
        lOutColours3Vec4.r = float( liMaterialID ) / 255.0f;
    }
#endif

    #ifdef D_DEFERRED_DECAL
    {
        #ifdef _F51_DECAL_DIFFUSE
        {
            FRAGMENT_COLOUR0 = vec4( lOutColours0Vec4.xyz, lColourVec4.a );
            //FRAGMENT_COLOUR0 = vec4( lTexCoordsVec4.x, lTexCoordsVec4.y, 0.0, lColourVec4.a );
            //FRAGMENT_COLOUR0 = vec4( 1.0, 1.0, 1.0, 1.0 );
        }
        #else
        {
            FRAGMENT_COLOUR0 = float2vec4(0.0); // Discard Colour
        }
        #endif

        //FRAGMENT_COLOUR1 = lOutColours1Vec4;
        FRAGMENT_COLOUR1 = float2vec4(0.0);
        
        #ifdef _F03_NORMALMAP
        {
            FRAGMENT_COLOUR2 = vec4( lOutColours2Vec4.xyz, lColourVec4.a );
        }
        #else
        {
            FRAGMENT_COLOUR2 = float2vec4(0.0); // Discard Colour
        }
        #endif

        FRAGMENT_COLOUR3 = lOutColours3Vec4;
    }
    #else
    {
        FRAGMENT_COLOUR0 = lOutColours0Vec4;
        FRAGMENT_COLOUR1 = lOutColours1Vec4;
        FRAGMENT_COLOUR2 = lOutColours2Vec4;
        FRAGMENT_COLOUR3 = lOutColours3Vec4;
    }
    #endif
    
#endif

#endif

#endif
// D_DEPTH_CLEAR

#endif



// defined( D_IMPOSTER ) && defined( _F09_TRANSPARENT ) && ( defined( _F13_UVANIMATION ) || defined( _F14_UVSCROLL ) )
}
 
