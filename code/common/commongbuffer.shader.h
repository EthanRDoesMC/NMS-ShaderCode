////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonGBuffer.h
///     @author     User
///     @date       
///
///     @brief      CommonLighting
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONGBUFFER_H
#define D_COMMONGBUFFER_H

//-----------------------------------------------------------------------------
//      Include files

//#define D_DETAILOVERLAY       ( 1 << 0 ) 
#define D_NOWATER             ( 1 << 0 )
#define D_NORECEIVESHADOW     ( 1 << 1 ) 
#define D_DISABLE_POSTPROCESS ( 1 << 2 ) 
#define D_UNLIT               ( 1 << 3 )
#define D_CLAMP_AA            ( 1 << 4 ) 
#define D_DISABLE_AMBIENT     ( 1 << 5 ) 
#define D_GLOW		          ( 1 << 6 ) 
#define D_CLAMPAMBIENT        ( 1 << 7 )
#define D_REFLECTIVE          ( 1 << 8 )
#define D_PARALLAX            ( 1 << 9 )

#include "Common/Common.shader.h"
#include "Common/CommonDepth.shader.h"


//-----------------------------------------------------------------------------
///
///     EncodeNormal
///
//-----------------------------------------------------------------------------
void
EncodeNormal(
    in  vec3 lNormal,
    out vec3 lBuffer )
{
    lBuffer = lNormal * 0.5 +  0.5;
}

void
EncodeNormal(
    in  vec3 lNormal,
    out vec4 lBuffer )
{
    lBuffer.xyz = lNormal * 0.5 +  0.5;
}

//-----------------------------------------------------------------------------
///
///     DecodeNormal
///
//-----------------------------------------------------------------------------
vec3
DecodeNormal(
    in vec3 lBuffer )
{
    return lBuffer * 2.0 - 1.0;
}

//-----------------------------------------------------------------------------
///
///     EncodeNormalHalf
///
//-----------------------------------------------------------------------------
void
EncodeNormalHalf(
    in  half3 lNormal,
    out half3 lBuffer)
{
    lBuffer = lNormal * 0.5 + 0.5;
}


//-----------------------------------------------------------------------------
///
///     DecodeNormalHalf
///
//-----------------------------------------------------------------------------
half3
DecodeNormalHalf(
    in half3 lBuffer)
{
    return lBuffer * 2.0 - 1.0;
}


//-----------------------------------------------------------------------------
///
///     EncodeUnormToX11
///
//-----------------------------------------------------------------------------
float
EncodeUnormToX11(
    float v )
{
    uint  u;
    uint  m;
    uint  e;
    
    // get the 10 bits we want to encode
    u       = uint( min( 1023.0, round( v * 1023.0 ) ) );

    // encode 6 in the X11 mantissa 
    // (for now stored in the first 6 of the 23 bit full float mantissa)
    m       = u & 0x3F;
    m       = m << 17;

    e       = u >> 6;
    e      += 127 - 15;
    e       = e << 23;

    v       = asfloat( int( e | m ) );

    return v;
}

//-----------------------------------------------------------------------------
///
///     EncodeUnormToX10
///
//-----------------------------------------------------------------------------
float
EncodeUnormToX10(
    float v )
{
    uint  u;
    uint  m;
    uint  e;
    
    // get the 10 bits we want to encode
    u       = uint( min( 511.0, round( v * 511.0 ) ) );

    // encode 5 in the X10 mantissa 
    // (for now stored in the first 5 of the 23 bit full float mantissa)
    m       = u & 0x1F;
    m       = m << 18;

    e       = u >> 5;
    e      += 127 - 15;
    e       = e << 23;

    v       = asfloat( int( e | m ) );

    return v;
}

//-----------------------------------------------------------------------------
///
///     DecodeUnormFromX11
///
//-----------------------------------------------------------------------------
float
DecodeUnormFromX11(
    float v )
{
    uint  u = asuint( v );
    uint  e = ( u >> 23 ) -  127 + 15;
    uint  m = ( u << 9  ) >> 26;
    
    e       = e << 6;
    v       = float ( e | m ) / 1023.0;

    return v;
}

//-----------------------------------------------------------------------------
///
///     DecodeUnormFromX10
///
//-----------------------------------------------------------------------------
float
DecodeUnormFromX10(
    float v )
{
    uint  u = asuint( v );
    uint  e = ( u >> 23 ) -  127 + 15;
    uint  m = ( u << 9  ) >> 27;
    
    e       = e << 5;
    v       = float ( e | m ) / 511.0;

    return v;
}

//-----------------------------------------------------------------------------
///
///     EncodeVec3ToR11G11B10
///
//-----------------------------------------------------------------------------
vec3
EncodeVec3ToR11G11B10(
    in  vec3  lVec3 )
{
    lVec3       = lVec3 * 0.5 + 0.5;
    lVec3.x     = EncodeUnormToX11( lVec3.x );
    lVec3.y     = EncodeUnormToX11( lVec3.y );
    lVec3.z     = EncodeUnormToX10( lVec3.z );

    return lVec3;
}

//-----------------------------------------------------------------------------
///
///     DecodeVec3FromR11G11B10
///
//-----------------------------------------------------------------------------
vec3
DecodeVec3FromR11G11B10(
    in  vec3  lVec3 )
{   
    lVec3.x = DecodeUnormFromX11( lVec3.x );
    lVec3.y = DecodeUnormFromX11( lVec3.y );
    lVec3.z = DecodeUnormFromX10( lVec3.z );
    lVec3   = lVec3 * 2.0 - 1.0;
    
    return lVec3;
}

//-----------------------------------------------------------------------------
///
///     EncodeMotion
///
//-----------------------------------------------------------------------------


#if defined( D_PLATFORM_VULKAN ) || defined( D_PLATFORM_XBOXONE ) || defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS )
vec2
EncodeMotion(
    in  vec2 lMotion )
{

    vec2 lMotionScaled = DSCREENSPACE_AS_RENDERTARGET_UVS(lMotion * 0.25);

    return lMotionScaled + 0.5;
}

#else

vec2
EncodeMotion(
    in  vec2 lMotion )
{
    // gamma 2.0 encoding to pack a reasonable value into RG8 targets
    // corresponding decode is in CommonPostProcess to save rebuilds

    vec2 lMotionScaled = DSCREENSPACE_AS_RENDERTARGET_UVS(lMotion * 8.0);

    //float lLength = length( lMotionScaled * vec2( 9.0 / 16.0, 1.0 ) );
    float lLength = lengthSquared( lMotionScaled );
    if( lLength > 0.0 )
    {
        lMotionScaled *= invsqrt(max(1.0, lLength));
        // set length to sqrt( the clamped length )
        lMotionScaled *= invsqrt( length( lMotionScaled ) );
    }

    // reserve 1.0 for signaling values`
    vec2 lMaxRealValue = float2vec2( 254.0 / 255.0 );

    return min( lMaxRealValue, lMotionScaled * ( 127.0 / 255.0 ) + 0.5 );
}

#endif
//-----------------------------------------------------------------------------
///
///     DecodeGBufferPosition
///
//-----------------------------------------------------------------------------
vec3
DecodeGBufferPosition(
    in  vec2  lScreenPosVec2,
    in  vec4  lClipPlanes,
    in  mat4  lInverseProjectionMat4,
    in  mat4  lInverseViewMat4,
    in  vec3  lViewPositionVec3,
    in  vec4  lBuffer1_Vec4,
    in  bool  lbWithIVP
)
{
    float lfDepth = FastDenormaliseDepth(lClipPlanes, DecodeDepthFromColour(lBuffer1_Vec4));
    if (lbWithIVP)
    {
        // fast path with lInverseProjectionMat4 as invView(zero position) * invProj
        return RecreatePositionFromDepthWithIVP(lfDepth, lScreenPosVec2, lViewPositionVec3, lInverseProjectionMat4, lClipPlanes);
    }
    else
    {
        return RecreatePositionFromDepth(lfDepth, lScreenPosVec2, lViewPositionVec3, lInverseProjectionMat4, lInverseViewMat4);
    }
}

//-----------------------------------------------------------------------------
///
///     DecodeGBuffer
///
//-----------------------------------------------------------------------------
void
DecodeGBuffer(
    in  vec2  lScreenPosVec2,
    in  vec4  lClipPlanes,
    in  mat4  lInverseProjectionMat4,
    in  mat4  lInverseViewMat4,
    in  vec3  lViewPositionVec3,
    in  vec4  lBuffer0_Vec4,
    in  vec4  lBuffer1_Vec4,
    in  vec4  lBuffer2_Vec4,
    in  vec4  lBuffer3_Vec4,
    in  bool  lbDecodePosition,
    in  bool  lbWithIVP,
    out vec3  lColourVec3,
    inout vec3  lPositionVec3,
    out vec3  lNormalVec3,
    out int   liMaterialID,
    out float lfRoughness,
    out float lfMetallic,
    out float lfSubsurface,
    out float lfGlow )
{
    if (lbDecodePosition)
    {
        lPositionVec3 = DecodeGBufferPosition(lScreenPosVec2, lClipPlanes, lInverseProjectionMat4, lInverseViewMat4, lViewPositionVec3, lBuffer1_Vec4, lbWithIVP);
    }

    lColourVec3 = lBuffer0_Vec4.rgb;
#if defined ( D_PLATFORM_SWITCH )
    lfGlow      = lBuffer0_Vec4.a * 4.0;
#else
    lfGlow      = lBuffer0_Vec4.a * lBuffer0_Vec4.a * 4.0;
#endif

    lNormalVec3       = DecodeNormal( lBuffer2_Vec4.xyz );

    liMaterialID      = int(lBuffer3_Vec4.r * 255.0 );
    lfRoughness       = lBuffer3_Vec4.g;
    lfMetallic        = lBuffer3_Vec4.b;
    lfSubsurface      = lBuffer3_Vec4.a;
}

void
DecodeGBufferHalf(
    in  vec2  lScreenPosVec2,
    in  vec4  lClipPlanes,
    in  mat4  lInverseProjectionMat4,
    in  mat4  lInverseViewMat4,
    in  vec3  lViewPositionVec3,
    in  half4  lBuffer0_Vec4,
    in  half4  lBuffer1_Vec4,
    in  half4  lBuffer2_Vec4,
    in  half4  lBuffer3_Vec4,
    in  bool  lbDecodePosition,
    in  bool  lbWithIVP,
    out half3  lColourVec3,
    inout vec3  lPositionVec3,
    out half3  lNormalVec3,
    out int   liMaterialID,
    out half lfRoughness,
    out half lfMetallic,
    out half lfSubsurface,
    out half lfGlow )
{
    if (lbDecodePosition)
    {
        lPositionVec3 = DecodeGBufferPosition(
            lScreenPosVec2,
            lClipPlanes,
            lInverseProjectionMat4,
            lInverseViewMat4,
            lViewPositionVec3,
            vec4( lBuffer1_Vec4 ),
            lbWithIVP );
    }

    lColourVec3 = lBuffer0_Vec4.rgb;
#if defined ( D_PLATFORM_SWITCH )
    lfGlow      = lBuffer0_Vec4.a * 4.0;
#else
    lfGlow      = lBuffer0_Vec4.a * lBuffer0_Vec4.a * 4.0;
#endif

    lNormalVec3       = DecodeNormalHalf( lBuffer2_Vec4.xyz );

    liMaterialID      = int(lBuffer3_Vec4.r * 255.0 );
    lfRoughness       = lBuffer3_Vec4.g;
    lfMetallic        = lBuffer3_Vec4.b;
    lfSubsurface      = lBuffer3_Vec4.a;
}

//-----------------------------------------------------------------------------
///
///     EncodeGBuffer
///
//-----------------------------------------------------------------------------
void
EncodeGBuffer(
    //in  vec4  lClipPlanes,
    //in  vec3  lViewPositionVec3,
    out vec4  lBuffer0_Vec4,
    out vec4  lBuffer1_Vec4,
    out vec4  lBuffer2_Vec4,
    out vec4  lBuffer3_Vec4,
    out vec4  lBuffer4_Vec4,
    in  vec3  lColourVec3,
    in  vec3  lPositionVec3,
    in  vec3  lNormalVec3,
    in  int   liMaterialID,
    in  float lfRoughness,
    in  float lfMetallic,
    in  float lfSubsurface,
    in  float lfGlow,
    in  vec2  lScreenSpaceMotionVec2,
    in  float lfPixelSelfShadowing
#ifdef D_OUTPUT_LINEARDEPTH
    ,
    in  float lfLinearDepth
#endif
    )
{
#if defined ( D_PLATFORM_SWITCH )
    lBuffer0_Vec4.xyz = lColourVec3;
    lBuffer0_Vec4.a = saturate(lfGlow * (1.0 / 4.0));
    lBuffer2_Vec4.w = 0.0;
#else
    if ( lfPixelSelfShadowing > -1.0 )
    {
        // If using parallax and self shadowing we can't be using glow.
        liMaterialID = liMaterialID & ~D_GLOW;
        liMaterialID |= D_PARALLAX;
    }
    lBuffer0_Vec4.xyz  = lColourVec3;
    lBuffer0_Vec4.a    = (liMaterialID & D_PARALLAX) != 0 ? lfPixelSelfShadowing : saturate( sqrt( lfGlow * ( 1.0 / 4.0 ) ) );
    lBuffer2_Vec4.w = float(liMaterialID >> 8) / 3.0;
#endif

    EncodeNormal( lNormalVec3, lBuffer2_Vec4 );
   
    lBuffer3_Vec4.r    = float( liMaterialID & 0xff ) / 255.0;
    lBuffer3_Vec4.g    = saturate(lfRoughness);
    lBuffer3_Vec4.b    = saturate(lfMetallic);
    lBuffer3_Vec4.a    = saturate(lfSubsurface);

    #ifdef D_OUTPUT_MOTION_VECTORS
    #ifdef D_PLATFORM_METAL
    if(HAS_MOTION_VECTORS)
    #endif
    {
    lBuffer1_Vec4      = vec4( EncodeMotion( lScreenSpaceMotionVec2 ), 0.0, 0.0 );
    }
    #ifdef D_PLATFORM_METAL
    else
    {        
        lBuffer1_Vec4      = vec4( lScreenSpaceMotionVec2, 0.0, 0.0 );
    }
    #endif
    #else
    lBuffer1_Vec4      = vec4( lScreenSpaceMotionVec2, 0.0, 0.0 );
    #endif
    #ifdef D_OUTPUT_LINEARDEPTH
    lBuffer4_Vec4 = EncodeDepthToColour(lfLinearDepth);
    #else   
    lBuffer4_Vec4 = vec4(0.0, 0.0, 0.0, 0.0);
    #endif
}

void
EncodeGBufferHalf(
    //in  vec4  lClipPlanes,
    //in  vec3  lViewPositionVec3,
    out half4  lBuffer0_Vec4,
    out half4  lBuffer1_Vec4,
    out half4  lBuffer2_Vec4,
    out half4  lBuffer3_Vec4,
    out half4  lBuffer4_Vec4,
    in  half3  lColourVec3,
    in  vec3  lPositionVec3,
    in  half3  lNormalVec3,
    in  int   liMaterialID,
    in  half lfRoughness,
    in  half lfMetallic,
    in  half lfSubsurface,
    in  half lfGlow,
    in  vec2  lScreenSpaceMotionVec2,
    in  half lfPixelSelfShadowing )
{
#if defined ( D_PLATFORM_SWITCH )
    lBuffer0_Vec4.xyz = lColourVec3;
    lBuffer0_Vec4.a = saturate(lfGlow * (1.0 / 4.0));
    lBuffer2_Vec4.w = 0.0;
#else
    if ( lfPixelSelfShadowing > -1.0 )
    {
        // If using parallax and self shadowing we can't be using glow.
        liMaterialID = liMaterialID & ~D_GLOW;
        liMaterialID |= D_PARALLAX;
    }
    lBuffer0_Vec4.xyz  = lColourVec3;
    lBuffer0_Vec4.a    = (liMaterialID & D_PARALLAX) != 0 ? lfPixelSelfShadowing : saturate( sqrt( lfGlow * ( 1.0 / 4.0 ) ) );
    lBuffer2_Vec4.w = half(float(liMaterialID >> 8) / 3.0);
#endif

#if defined ( D_PLATFORM_METAL )
    half3 lNormalOutVec3;
    EncodeNormalHalf(
        lNormalVec3,
        lNormalOutVec3 );
    lBuffer2_Vec4.xyz = lNormalOutVec3;
#else
    EncodeNormalHalf( lNormalVec3, lBuffer2_Vec4.xyz );
#endif  

    lBuffer3_Vec4.r    = half(float( liMaterialID & 0xff ) / 255.0);
    lBuffer3_Vec4.g    = saturate(lfRoughness);
    lBuffer3_Vec4.b    = saturate(lfMetallic);
    lBuffer3_Vec4.a    = saturate(lfSubsurface);

    #ifdef D_OUTPUT_MOTION_VECTORS
    lBuffer1_Vec4 = half4(
        half2( EncodeMotion(lScreenSpaceMotionVec2) ),
        half( 0.0 ),
        half( 0.0 ) );
    #else
    lBuffer1_Vec4      = half4( half2( lScreenSpaceMotionVec2 ), half( 0.0 ), half( 0.0 ) );
    #endif
    
    lBuffer4_Vec4      = half4(0.0, 0.0, 0.0, 0.0);
}

#endif
































