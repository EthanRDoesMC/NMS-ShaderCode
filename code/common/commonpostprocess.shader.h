////////////////////////////////////////////////////////////////////////////////
///
///     @file       PostProcessFragment.h
///     @author     User
///     @date       
///
///     @brief      PostProcessFragment
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONPOSTPROCESS_H
#define D_COMMONPOSTPROCESS_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonDepth.shader.h"
#include "Common/CommonGBuffer.shader.h"
#if defined(D_SCREENEFFECT) || defined ( D_UI ) || defined( D_BINOCS )
#include "Common/Noise3D.glsl"
#include "Common/CommonFragment.shader.h"
#endif

//----------------------------------------------------------------------------- 
//      Global Data

STATIC_CONST float kfAvgLumR    = 0.5;
STATIC_CONST float kfAvgLumG    = 0.5;
STATIC_CONST float kfAvgLumB    = 0.5;
STATIC_CONST vec3  kvLumCoeff   = vec3( 0.2125, 0.7154, 0.0721 );
STATIC_CONST vec3  kvLumaCoeff  = vec3( 0.299,  0.587,  0.114 );

// Implementation of John Hable's Filmic tonemapper
// https://www.gdcvault.com/play/1012351/Uncharted-2-HDR
// https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting
// NOTE(sal): Not sure why Charlie decided to call this "Kodak" tonemapper.
// I think maybe it's because John Hable says he wanted to emulate how
// Kodak film behaves when exposed to light (slide 94).
STATIC_CONST float kfTonemapKodak_A = 0.22; //  shoulder strength
STATIC_CONST float kfTonemapKodak_B = 0.30; //  linear   strength
STATIC_CONST float kfTonemapKodak_C = 0.10; //  linear   angle
STATIC_CONST float kfTonemapKodak_D = 0.20; //  toe      strength
STATIC_CONST float kfTonemapKodak_E = 0.01; //  toe      numerator
STATIC_CONST float kfTonemapKodak_F = 0.30; //  toe      denominator
STATIC_CONST float kfTonemapKodak_W = 11.2; //  white    point

//-----------------------------------------------------------------------------
//    Functions

float
TonemapKodak(
    in float x,
    in float A,
    in float B,
    in float C,
    in float D,
    in float E,
    in float F )
{
    return ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - E / F;
}

vec3 
TonemapKodak(
    in vec3  x,
    in float A,
    in float B,
    in float C,
    in float D,
    in float E,
    in float F )
{
    return ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - E / F;
}

// NOTE(sal): These are not from John Hable's original presentation.
// I just inverted the function myself using wolfram alpha.
float
TonemapKodakInverse(
    in float x,
    in float A,
    in float B,
    in float C,
    in float D,
    in float E,
    in float F )
{
    return ( B * C * F - B * E - B * F * x ) / ( 2.0 * A * ( E + F * x - F ) ) -
            sqrt( ( -B * C * F + B * E + B * F * x ) * ( -B * C * F + B * E + B * F * x ) -
                  4.0 * D * F * F * x * ( A * E + A * F * x - A * F ) ) / ( 2.0 * A * ( E + F * x - F ) );
}

vec3
TonemapKodakInverse(
    in vec3  x,
    in float A,
    in float B,
    in float C,
    in float D,
    in float E,
    in float F )
{
    return ( B * C * F - B * E - B * F * x ) / ( 2.0 * A * ( E + F * x - F ) ) -
            sqrt( ( -B * C * F + B * E + B * F * x ) * ( -B * C * F + B * E + B * F * x ) -
                  4.0 * D * F * F * x * ( A * E + A * F * x - A * F ) ) / ( 2.0 * A * ( E + F * x - F ) );
}


float
TonemapKodak(
    in float x )
{
    return TonemapKodak( x, kfTonemapKodak_A, kfTonemapKodak_B, kfTonemapKodak_C, kfTonemapKodak_D, kfTonemapKodak_E, kfTonemapKodak_F );
}

vec3 
TonemapKodak(
    in vec3 x )
{
    return TonemapKodak( x, kfTonemapKodak_A, kfTonemapKodak_B, kfTonemapKodak_C, kfTonemapKodak_D, kfTonemapKodak_E, kfTonemapKodak_F );
}

float
TonemapKodak(
    in float x,
    in float w )
{
    return TonemapKodak( x ) / TonemapKodak( w );
}

vec3 
TonemapKodak(
    in vec3  x,
    in float w )
{
    return TonemapKodak( x ) / TonemapKodak( w );
}

float
TonemapKodakInverse(
    in float x )
{
    return TonemapKodakInverse( x, kfTonemapKodak_A, kfTonemapKodak_B, kfTonemapKodak_C, kfTonemapKodak_D, kfTonemapKodak_E, kfTonemapKodak_F );

}

vec3
TonemapKodakInverse(
    in vec3 x )
{
    return TonemapKodakInverse( x, kfTonemapKodak_A, kfTonemapKodak_B, kfTonemapKodak_C, kfTonemapKodak_D, kfTonemapKodak_E, kfTonemapKodak_F );
}

float
TonemapKodakInverse(
    in float x,
    in float w )
{
    return TonemapKodakInverse( x * TonemapKodak( w ) );

}

vec3
TonemapKodakInverse(
    in vec3  x,
    in float w )
{
    return TonemapKodakInverse( x * TonemapKodak( w ) );
}

vec3 
TonemapKodakYCgCo(
    in vec3 colIn )
{
    float lumIn  = colIn.x;
    float newLum =  TonemapKodak( lumIn );
    newLum *= 1.0 / TonemapKodak( 1.0 );

    return colIn * float2vec3( max( 0.001, newLum ) / max( 0.001, lumIn ) );
}

vec3 
TonemapKodakLum(
    in vec3 colIn )
{
    float lumIn  = 0.25 * ( colIn.r + colIn.b ) + 0.5 * colIn.g;
    float newLum = TonemapKodak( lumIn );

    return colIn * float2vec3( max( 0.001, newLum ) / max( 0.001, lumIn ) );
}

vec3 
TonemapUDK(
    in vec3 x )
{
    return x / (x + 0.187) * 1.035;;
}

vec3 
TonemapExposure(
    in vec3   lInputVec3,
    in float  lfExposure,
    in float  lfThreshold,
    in float  lfOffset  )
{
    vec3 lResultVec3;

    lResultVec3  = 1.0 - exp2( -lfExposure * lInputVec3.xyz );
    lResultVec3  = max( lResultVec3 - lfThreshold, 0.0 );
    lResultVec3 /= lfOffset + lResultVec3;

    return lResultVec3;
}



//-----------------------------------------------------------------------------
///
///     GetTex2DBilinear
///
///     @brief      GetTex2DBilinear
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec4 
GetTex2DBilinear( 
    SAMPLER2DARG( lTextureMap ), 
    in vec2       lTexCoordsVec2, 
    in vec2       lTexSizeVec2 )
{
    // Bilinear filtering function. Useful when hardware filtering is not available, e.g. for
    // floating point textures on ATI 1xx0 cards
    
    vec2 lCoord0Vec2 = lTexCoordsVec2 - 0.5 / lTexSizeVec2;
    vec2 lCoord1Vec2 = lTexCoordsVec2 + 0.5 / lTexSizeVec2;
    vec2 lWeightVec2 = fract( lCoord0Vec2 * lTexSizeVec2 );
    
    vec4 lBottomVec4 = mix( texture2D( lTextureMap, lCoord0Vec2 ),
                    texture2D( lTextureMap, vec2( lCoord1Vec2.x, lCoord0Vec2.y ) ),
                    lWeightVec2.x );

    vec4 lTopVec4 = mix( texture2D( lTextureMap, vec2( lCoord0Vec2.x, lCoord1Vec2.y ) ),
                    texture2D( lTextureMap, lCoord1Vec2 ),
                    lWeightVec2.x );
    
    return mix( lBottomVec4, lTopVec4, lWeightVec2.y );
}

//-----------------------------------------------------------------------------
///
///     BlurKawase
///
///     @brief      BlurKawase
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec4 
BlurKawase( 
    SAMPLER2DARG( lTextureMap ), 
    in vec2       lTexCoordsVec2, 
    in vec2       lRecipTexSizeVec2, 
    in float      lfIteration )
{
    // Function assumes that tex is using bilinear hardware filtering
    vec2 lUVVec2 = (lfIteration + 0.5) * lRecipTexSizeVec2;
    
    vec4 lColourVec4 = texture2DLod( lTextureMap, lTexCoordsVec2 + vec2( -lUVVec2.x, lUVVec2.y ), 0.0 );	// Top left
    lColourVec4 +=	   texture2DLod( lTextureMap, lTexCoordsVec2 + lUVVec2, 0.0);							// Top right
    lColourVec4 +=	   texture2DLod( lTextureMap, lTexCoordsVec2 + vec2( lUVVec2.x, -lUVVec2.y ), 0.0);	// Bottom right
    lColourVec4 +=     texture2DLod( lTextureMap, lTexCoordsVec2 - lUVVec2, 0.0);							// Bottom left
    
    return lColourVec4 * 0.25;
}

#if defined( D_PLATFORM_PROSPERO )

bool
HmdFovMask_isInvisible(in vec2 lFragCoordsVec2,
    in vec4 lFrustumTanFovVec4,
    in vec3 lVREyeInfoVec3
)
{
    return false;
}

#elif defined( D_PLATFORM_ORBIS)

bool
HmdFovMask_isInvisible(in vec2 lFragCoordsVec2,
            in vec4 lFrustumTanFovVec4,
            in vec3 lVREyeInfoVec3
)
{
    int eyeIndex = int(lVREyeInfoVec3.x);

    vec2 lVRTexCoords = lFragCoordsVec2;
    lVRTexCoords.x = (lVRTexCoords.x - lVREyeInfoVec3.y) * lVREyeInfoVec3.z;

    ivec2 originalRenderedResolution = ivec2(960, 1080);
    ivec2 originalRenderedScreenPos = ivec2(lVRTexCoords * vec2(960, 1080));

    float hFov = lFrustumTanFovVec4.x + lFrustumTanFovVec4.y;
    float vFov = lFrustumTanFovVec4.z + lFrustumTanFovVec4.w;
    float tanTop = lFrustumTanFovVec4.z;
    float leftToMiddleFov = eyeIndex ? lFrustumTanFovVec4.y : lFrustumTanFovVec4.x;

    ivec2 minHtileIndex = originalRenderedScreenPos / ivec2(8, 8);

    ivec2 minScreenPos = minHtileIndex * 8 - 1;
    ivec2 maxScreenPos = minHtileIndex * 8 + 8;

    vec2 minUv = (vec2(minScreenPos) + vec2(0.5f, 0.5f)) / vec2(originalRenderedResolution);
    vec2 maxUv = (vec2(maxScreenPos) + vec2(0.5f, 0.5f)) / vec2(originalRenderedResolution);

    vec2 maskCoordTL = vec2(0);
    vec2 maskCoordBR = vec2(0);

    maskCoordTL.x = (minUv.x - leftToMiddleFov / hFov) * (hFov / tanTop);
    maskCoordBR.x = (maxUv.x - leftToMiddleFov / hFov) * (hFov / tanTop);

    //top y == min.y , bottom y == max.y
    maskCoordTL.y = (minUv.y - tanTop / vFov) * (vFov / tanTop);
    maskCoordBR.y = (maxUv.y - tanTop / vFov) * (vFov / tanTop);

    bool bl = maskCoordTL.x*maskCoordTL.x + maskCoordBR.y * maskCoordBR.y > 1.0f;
    bool tl = maskCoordTL.x*maskCoordTL.x + maskCoordTL.y * maskCoordTL.y > 1.0f;
    bool tr = maskCoordBR.x*maskCoordBR.x + maskCoordTL.y * maskCoordTL.y > 1.0f;
    bool br = maskCoordBR.x*maskCoordBR.x + maskCoordBR.y * maskCoordBR.y > 1.0f;

    return bl && tl && tr && br;
}

#endif 

//-----------------------------------------------------------------------------
///
///     RGBToYCgCo
///
///     @brief      RGBToYCgCo
///
///     @param      vec3
///     @return     vec3.
///
//-----------------------------------------------------------------------------
vec3 
RGBToYCgCo(
    vec3 lRgbVec3 )
{
    float i1 = 0.25 * ( lRgbVec3.r + lRgbVec3.b );
    float i2 = 0.5 * lRgbVec3.g;

    float luminance = i1 + i2;
    float Cg = i2 - i1;
    float Co = 0.5 * ( lRgbVec3.r - lRgbVec3.b );

    //Cg += 0.5;
    //Co += 0.5;

    return vec3( luminance, Cg, Co );
}

//-----------------------------------------------------------------------------
///
///     YCgCoToRGB
///
///     @brief      YCgCoToRGB
///
///     @param      vec3
///     @return     vec3.
///
//-----------------------------------------------------------------------------
vec3 
YCgCoToRGB(
    vec3 lYCgCoVec3 )
{
    float Y = lYCgCoVec3.r;
    float Cg = lYCgCoVec3.g; // - 0.5;
    float Co = lYCgCoVec3.b; // - 0.5;

    float tmp = Y - Cg;
    float r = tmp + Co;
    float g = Y + Cg;
    float b = tmp - Co;

    return vec3( r, g, b );
}

//-----------------------------------------------------------------------------
///
///     EncodeUintToX11
///
//-----------------------------------------------------------------------------
float
EncodeUintToX11(
    uint  u )
{
    float v;
    uint  m;
    uint  e;    

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
///     EncodeUintToX10
///
//-----------------------------------------------------------------------------
float
EncodeUintToX10(
    uint  u )
{
    float v;
    uint  m;
    uint  e;    

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
///     DecodeUintFromX11
///
//-----------------------------------------------------------------------------
uint
DecodeUintFromX11(
    float v )
{
    uint  u = asuint( v );
    uint  e = ( u >> 23 ) -  127 + 15;
    uint  m = ( u << 9  ) >> 26;
    
    e       = e << 6;
    u       = e | m;

    return u;
}

//-----------------------------------------------------------------------------
///
///     DecodeUintFromX10
///
//-----------------------------------------------------------------------------
uint
DecodeUintFromX10(
    float v )
{
    uint  u = asuint( v );
    uint  e = ( u >> 23 ) -  127 + 15;
    uint  m = ( u << 9  ) >> 27;
    
    e       = e << 5;
    u       = e | m;

    return u;
}

//-----------------------------------------------------------------------------
///
///     EncodeVec2ToR11G11B10
///
//-----------------------------------------------------------------------------
vec3
EncodeVec2ToR11G11B10( 
    vec2 v )
{
    vec3  f3;
    uvec3 u3;

    u3.x = uint( min( 16383.0, round( v.x * 16383.0 ) ) );
    u3.y = uint( min( 16383.0, round( v.y * 16383.0 ) ) );

    u3.z = ( ( u3.x << 4 ) | ( u3.y & 0xf ) ) & 0xff;
    u3.x = u3.x >> 4;
    u3.y = u3.y >> 4;

    f3.x = EncodeUintToX11( u3.x );
    f3.y = EncodeUintToX11( u3.y );
    f3.z = EncodeUintToX10( u3.z );

    return f3;
}

//-----------------------------------------------------------------------------
///
///     DecodeVec2FromR11G11B10
///
//-----------------------------------------------------------------------------
vec2
DecodeVec2FromR11G11B10( 
    vec3 v )
{    
    vec2  v2;
    uvec3 u3;

    u3.x = DecodeUintFromX11( v.x );
    u3.y = DecodeUintFromX11( v.y );
    u3.z = DecodeUintFromX10( v.z );

    u3.x = ( u3.x << 4 ) | ( u3.z >> 4   );
    u3.y = ( u3.y << 4 ) | ( u3.z  & 0xf );

    v2   = vec2( u3.xy ) / 16383.0;

    return v2;
}

//-----------------------------------------------------------------------------
///
///     DecodeMotion
///
//-----------------------------------------------------------------------------

#if defined( D_PLATFORM_VULKAN ) || defined( D_PLATFORM_XBOXONE ) || defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_ORBIS )

vec2
DecodeMotion(
    in vec2 lBuffer )
{
    return ( lBuffer - 0.5 ) * 2.0;
}

float
ComputeMotionInverseSpeedSq(
    in vec2 lBuffer )
{
    vec2 lMotionScaled = DecodeMotion( lBuffer );
    return 1.0 / ( dot( lMotionScaled, lMotionScaled ) );
}

float
ComputeMotionInverseSpeed(
    in vec2 lBuffer )
{
    return sqrt( ComputeMotionInverseSpeedSq( lBuffer ) );
}

#elif defined( D_PLATFORM_METAL )

vec2
DecodeMotion(
    in vec2 lBuffer )
{

    return clamp(vec2(-lBuffer.x, lBuffer.y) * 0.25, -1.0f, 1.0f);
}

float
ComputeMotionInverseSpeedSq(
    in vec2 lBuffer )
{
    vec2 lMotionScaled = DecodeMotion( lBuffer );
    return 1.0 / ( dot( lMotionScaled, lMotionScaled ) );
}

float
ComputeMotionInverseSpeed(
    in vec2 lBuffer )
{
    return sqrt( ComputeMotionInverseSpeedSq( lBuffer ) );
}

#else

vec2
DecodeMotion(
    in vec2 lBuffer )
{

    {
        vec2 lMotionScaled = ( lBuffer - 127.0 / 255.0 );
        // length needs to be squared
        lMotionScaled *= length( lMotionScaled );

        #ifndef D_MOTION_VECTORS_ALREADY_RESOLVED
        lMotionScaled = ( lBuffer.x == 1.0 )? vec2(0,0) : lMotionScaled;
        #endif

        return lMotionScaled * float2vec2( 0.25 );
    }
}


float
ComputeMotionInverseSpeed(
    in vec2 lBuffer )
{
    {
        vec2 lMotionScaled = ( lBuffer - 127.0 / 255.0 );

        // lMotionScaled *= length( lMotionScaled );
        // lMotionScaled *= 0.25;

        // return 1.0 / ( 0.25 * dot( lMotionScaled, lMotionScaled ) );
        return 4.0 / ( dot( lMotionScaled, lMotionScaled ) );
    }
}

float
ComputeMotionInverseSpeedSq(
    in vec2 lBuffer )
{
    {
        return ComputeMotionInverseSpeed(lBuffer) * ComputeMotionInverseSpeed(lBuffer);
    }
}

#endif

vec2
GetDejitteredTexCoord(
    in vec2 lTexCoord,
    in vec3 lDejitterVec3 )
{
    return lTexCoord + DSCREENSPACE_AS_RENDERTARGET_UVS(lDejitterVec3.xy);
}

#ifdef D_PLATFORM_ORBIS

vec4 
Texture2DNoFiltering(
    SAMPLER2DARG( tex ),
    vec2 texcoord )
{
    // Setup the sampler descriptor
    sce::Gnm::Sampler sampler;
        sampler.init();
        sampler.setWrapMode(sce::Gnm::kWrapModeClampLastTexel, sce::Gnm::kWrapModeClampLastTexel, sce::Gnm::kWrapModeClampLastTexel );
        sampler.setXyFilterMode(sce::Gnm::kFilterModePoint, sce::Gnm::kFilterModePoint);
        sampler.setMipFilterMode(sce::Gnm::kMipFilterModeNone);

    // Create resource from descriptor
    SamplerState samp = SamplerState(sampler);
 
    vec4 color = tex.SampleLOD(samp, texcoord, 0.0); 
    return color;
}

#elif defined(D_PLATFORM_DX12)

vec4
Texture2DNoFiltering(
    SAMPLER2DARG(tex),
    vec2 texcoord)
{
    return texture2DLod(tex, texcoord, 0.0);
}

#elif defined(D_PLATFORM_METAL)

vec4
Texture2DNoFiltering(
    SAMPLER2DARG(tex),
    vec2 texcoord)
{
    return texture2DLod(tex, texcoord, 0.0);
}


#else

vec4 
Texture2DNoFiltering(
    SAMPLER2DARG( tex ),
    in vec2 texcoord )
{
    return texture2D( tex, texcoord );
}

#endif

//-----------------------------------------------------------------------------
///
///     GetPrevPosition
///
///     @brief      GetPrevPosition
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec2 GetPrevPosition(
    in vec2 lFragCoordsVec2,
    in vec4 lClipPlanesVec4,
    in mat4 lInverseProjectionMat4,
    in mat4 lInverseViewMat4,
    in mat4 lInverseViewProjectionMat4,
    in mat4 lViewProjectionMat4,
    in mat4 lPrevViewProjectionMat4,
    in vec3 lViewPositionVec3,
    in vec4 lMBlurSettingsVec4,
    in vec4 lFoVValuesVec4,
    in int  liAntiAliasingIndex,
    SAMPLER2DARG( lMotionMap ),
    SAMPLER2DARG( lDepthMap ),
    out float lfOutDepth,
    out float lfOutDepthNormalised,
    out float lfOutRcpSpeed,
    out vec2 lOutDelta,
    out vec2 lOutEncodedDelta,
    out bool lWantsColorClipAA )
{
    vec2 lScreenPosVec2;
    vec2 lReprojectScreenPosVec2;
    vec2 lReprojectFragCoordsVec2;
    vec4 lMotionBuffer = Texture2DNoFiltering( SAMPLER2DPARAM( lMotionMap ), lFragCoordsVec2 );
    vec2 lDelta = lMotionBuffer.xy;

    #ifndef D_MOTION_BUFFER_HAS_DEPTH
        float lfDepth = DecodeDepthFromColour( Texture2DNoFiltering(SAMPLER2DPARAM( lDepthMap ), lFragCoordsVec2 ) );
        lfOutDepthNormalised = lfDepth;
        lfDepth = FastDenormaliseDepth( lClipPlanesVec4, lfDepth );
        lfOutDepth = lfDepth;
    #else
        lfOutDepth = FastDenormaliseDepth( lClipPlanesVec4, lMotionBuffer.z ); 
        lfOutDepthNormalised = lMotionBuffer.z;
        lfOutRcpSpeed = lMotionBuffer.w;
    #endif

    #ifndef D_MOTION_VECTORS_ALREADY_RESOLVED
    if( lDelta.x < 1.0 )
    #endif
    {  
        // motion already computed by a friendly object shader
        lOutEncodedDelta = lDelta;

        #ifndef D_MOTION_BUFFER_HAS_DEPTH
        lfOutRcpSpeed = ComputeMotionInverseSpeed( lDelta ) * ( 1.0 / lMBlurSettingsVec4.z );
        #endif

        lDelta = DecodeMotion( lDelta );

        lReprojectFragCoordsVec2 = lFragCoordsVec2 + lDelta;

        lOutDelta = lDelta;
        lWantsColorClipAA = false;

        return lReprojectFragCoordsVec2;
    }

    #ifndef D_MOTION_VECTORS_ALREADY_RESOLVED

    // use gbuffer depth to reconstruct position in homogeneous clip space
    vec4 lPositionVec4;
    lPositionVec4.xyz = RecreatePositionFromDepthWithIVP(lfDepth, lFragCoordsVec2, lViewPositionVec3, lInverseViewProjectionMat4, lClipPlanesVec4);
    lPositionVec4.w = 1.0;

    lPositionVec4 = MUL( lPrevViewProjectionMat4, lPositionVec4 ); 

    lPositionVec4.xyz       /= lPositionVec4.w;
    lReprojectScreenPosVec2  = lPositionVec4.xy;

    // Convert the Reproj pos from NDC to UV space
    lReprojectFragCoordsVec2 = SCREENSPACE_AS_RENDERTARGET_UVS( lReprojectScreenPosVec2 ) * 0.5 + 0.5;

    // Convert the Frag pos from UV to NDC space
    lScreenPosVec2           = SCREENSPACE_AS_RENDERTARGET_UVS( lFragCoordsVec2 ) * 2.0 - 1.0;

    lDelta        = lReprojectFragCoordsVec2 - lFragCoordsVec2;
    lfOutRcpSpeed = 1.0 / ( length( lDelta ) * lMBlurSettingsVec4.z );
    lOutDelta     = lDelta;

    // Pass to EncodeMotion the pos diff in NDC space (as done in Uber)
    lOutEncodedDelta  = EncodeMotion( lReprojectScreenPosVec2 - lScreenPosVec2 );
    lWantsColorClipAA = ( lMotionBuffer.y > 0.75 );

    return lReprojectFragCoordsVec2;

    #endif
}

/*
//-----------------------------------------------------------------------------
///
///     RGBToHSL
///
///     @brief      RGBToHSL
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec3 
RGBToHSL(
    vec3 lColourVec3 )
{
    vec3 lHSLVec3 = vec3( 0.0, 0.0, 0.0 ); // init to 0 to avoid warnings ? (and reverse if + remove first part)
    
    float lfMin = min(min(lColourVec3.r, lColourVec3.g), lColourVec3.b);    //Min. value of RGB
    float lfMax = max(max(lColourVec3.r, lColourVec3.g), lColourVec3.b);    //Max. value of RGB
    float lfDelta = lfMax - lfMin;             //Delta RGB value

    lHSLVec3.z = (lfMax + lfMin) / 2.0; // Luminance

    if (lfDelta == 0.0)		//This is a gray, no chroma...
    {
        lHSLVec3.x = 0.0;	// Hue
        lHSLVec3.y = 0.0;	// Saturation
    }
    else                                    //Chromatic data...
    {
        if (lHSLVec3.z < 0.5)
            lHSLVec3.y = lfDelta / (lfMax + lfMin); // Saturation
        else
            lHSLVec3.y = lfDelta / (2.0 - lfMax - lfMin); // Saturation
        
        float lfDeltaR = (((lfMax - lColourVec3.r) / 6.0) + (lfDelta / 2.0)) / lfDelta;
        float lfDeltaG = (((lfMax - lColourVec3.g) / 6.0) + (lfDelta / 2.0)) / lfDelta;
        float lfDeltaB = (((lfMax - lColourVec3.b) / 6.0) + (lfDelta / 2.0)) / lfDelta;

        if (lColourVec3.r == lfMax )
            lHSLVec3.x = lfDeltaB - lfDeltaG; // Hue
        else if (lColourVec3.g == lfMax)
            lHSLVec3.x = (1.0 / 3.0) + lfDeltaR - lfDeltaB; // Hue
        else if (lColourVec3.b == lfMax)
            lHSLVec3.x = (2.0 / 3.0) + lfDeltaG - lfDeltaR; // Hue

        if (lHSLVec3.x < 0.0)
            lHSLVec3.x += 1.0; // Hue
        else if (lHSLVec3.x > 1.0)
            lHSLVec3.x -= 1.0; // Hue
    }

    return lHSLVec3;
}

//-----------------------------------------------------------------------------
///
///     HueToRGB
///
///     @brief      HueToRGB
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
float 
HueToRGB(
    float lf1, 
    float lf2, 
    float lfHue)
{
    if (lfHue < 0.0)
        lfHue += 1.0;
    else if (lfHue > 1.0)
        lfHue -= 1.0;
    float res;
    if ((6.0 * lfHue) < 1.0)
        res = lf1 + (lf2 - lf1) * 6.0 * lfHue;
    else if ((2.0 * lfHue) < 1.0)
        res = lf2;
    else if ((3.0 * lfHue) < 2.0)
        res = lf1 + (lf2 - lf1) * ((2.0 / 3.0) - lfHue) * 6.0;
    else
        res = lf1;
    return res;
}

//-----------------------------------------------------------------------------
///
///     HSLToRGB
///
///     @brief      HSLToRGB
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec3 
HSLToRGB(
    vec3 lHSLVec3 )
{
    vec3 lRGBVec3;
    
    if (lHSLVec3.y == 0.0)
        lRGBVec3 = vec3(lHSLVec3.z, lHSLVec3.z, lHSLVec3.z); // Luminance
    else
    {
        float lf2;
        
        if (lHSLVec3.z < 0.5)
            lf2 = lHSLVec3.z * (1.0 + lHSLVec3.y);
        else
            lf2 = (lHSLVec3.z + lHSLVec3.y) - (lHSLVec3.y * lHSLVec3.z);
            
        float lf1 = 2.0 * lHSLVec3.z - lf2;
        
        lRGBVec3.r = HueToRGB(lf1, lf2, lHSLVec3.x + (1.0/3.0));
        lRGBVec3.g = HueToRGB(lf1, lf2, lHSLVec3.x);
        lRGBVec3.b= HueToRGB(lf1, lf2, lHSLVec3.x - (1.0/3.0));
    }
    
    return lRGBVec3;
}

//-----------------------------------------------------------------------------
///
///     ContrastSaturationBrightness
///
///     @brief      ContrastSaturationBrightness
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%
vec3 
ContrastSaturationBrightness(
    vec3   lColourVec3, 
    float lfBRT, 
    float lfSaturation, 
    float lfCon )
{
    float              lfDot;

    // Increase or decrease these values to adjust r, g and b color channels separately
    vec3 lAverageLuminosityVec3 = vec3(kfAvgLumR, kfAvgLumG, kfAvgLumB);

    vec3 lBRTColourVec3 = lColourVec3 * lfBRT;

    lfDot = dot(lBRTColourVec3, lLumCoefficientVec3);
    vec3 lIntensityVec3 = vec3( lfDot, lfDot, lfDot );
    vec3 lSaturationColourVec3 = mix(lIntensityVec3, lBRTColourVec3, lfSaturation);
    vec3 lConColorVec3 = mix(lAverageLuminosityVec3, lSaturationColourVec3, lfCon);
    return lConColorVec3;

}*/

//-----------------------------------------------------------------------------
///
///     GetBulgeDeform
///
///     @brief      GetBulgeDeform
///
///     @param      vec2 lCoordsVec2
///     @return     vec2
///
//-----------------------------------------------------------------------------
vec2 
GetBulgeDeform(
    vec2 lCoordsVec2 )
{
    // get the right pixel for the current position
    vec2 lBulgeCoordsVec2;

    // put into -1 . 1 space
    lBulgeCoordsVec2   = lCoordsVec2*2.0 - 1.0;

    lBulgeCoordsVec2.y = lBulgeCoordsVec2.y * mix( 1.0, 1.1, sin(0.5*3.1416*(lBulgeCoordsVec2.x+1.0)) );
    lBulgeCoordsVec2.y -= 0.03;

    // put back into 0 . 1 space
    lBulgeCoordsVec2 = (lBulgeCoordsVec2 + 1.0)*0.5;

    return lBulgeCoordsVec2;
}



//-----------------------------------------------------------------------------
///
///     GetDofPower
/// 
///     @brief      Works out the DOF power to pass to DOF
///
//-----------------------------------------------------------------------------
float GetDofPower(
    vec2 lTexCoordsVec2,
    SAMPLER2DARG( lDepthMap ),
    SAMPLER2DARG( lBlurMask ),
    vec4 lDofParamsVec4,
    vec4 lClipPlanesVec4 )
{
    float lfPower = 0.0;

    if( lDofParamsVec4.x != 0.0 ) // DOF amount
    {
        vec4 lBuffer1ColourVec4 = texture2DLod( lDepthMap, lTexCoordsVec2, 0.0);
        //FRAGMENT_DEPTH              = lBuffer1ColourVec4.x;

        float lfLinearZ = FastDenormaliseDepth( lClipPlanesVec4, DecodeDepthFromColour( lBuffer1ColourVec4 ) );

        /*
        float lfNonLinearZ = lBuffer1ColourVec4.x;

        float lfLinearZ = ReverseZToLinearDepth( lClipPlanesVec4, lfNonLinearZ);
        */
        if( lDofParamsVec4.x >= 0.0 ) // DOF amount
        {
            vec2 lDistMaskedVec2;

            float lfNearPlane = lDofParamsVec4.z;
            lDistMaskedVec2.x = smoothstep( lfNearPlane, lfNearPlane - min( 15.0, lfNearPlane * 0.5 ), lfLinearZ ); // in this case edge0 > edge1 - glsl spec says this gives undefined behaviour... is this line correct?
            lDistMaskedVec2.y = smoothstep( lDofParamsVec4.y, lDofParamsVec4.y + lDofParamsVec4.w, lfLinearZ );

            lfPower = max( lDistMaskedVec2.x, lDistMaskedVec2.y );

            //// Blur Mask
            vec2 lDeformedCoordsVec2 = GetBulgeDeform( lTexCoordsVec2 );
            vec4 lBlurMaskVec4Warped = texture2DLod( lBlurMask, lDeformedCoordsVec2, 0.0);// texture2D( gBlurMask, lDeformedCoordsVec2 );
            vec4 lBlurMaskVec4 = texture2DLod( lBlurMask, lTexCoordsVec2, 0.0);
            lfPower = mix( 0.0, lfPower, lDofParamsVec4.x );
            lfPower = mix( lfPower, 1.0, max( lBlurMaskVec4Warped.r, lBlurMaskVec4.g ) );
        }
        else // if( lDofParamsVec4.x < 0.0 )
        {
            // new physically based DoF
            lDofParamsVec4.x = -lDofParamsVec4.x;

            // y-value is focus distance, z-value is f-stop, w-value is the film plane width
            // for simplicity figure the lens has a focal length of 1
            float lfCoC = 0.0;  // CoC diameter
            if( lDofParamsVec4.y > 100000000.0 )
            {
                // infinite focus distance
                lfCoC = 1.0 / ( lfLinearZ * lDofParamsVec4.z );
            }
            else
            {
                lfCoC = abs( lfLinearZ - lDofParamsVec4.y ) / lfLinearZ;
                lfCoC /= (lDofParamsVec4.z * ( lDofParamsVec4.y  - 0.5 ));
            }

            // now get the CoC radius in pixels - just assume 1920-pixel width half-sized
            lfCoC = abs( lfCoC * 480.0 / lDofParamsVec4.w );

            // consider CoC < 1 pix to be full focus
            lfCoC -= 1.0;

            // and normalise to 1.0 == 8 pix radius
            lfCoC = saturate( lfCoC * 0.125 );

            // apply strength val
            lfPower = lfCoC * lDofParamsVec4.x;
        }
    }

    return lfPower;
}

#if defined ( D_LUT )

vec3
ApplyColourLUT(
    in PerFrameUniforms      lPerFrameUniforms,
    in CustomPerMeshUniforms lPerMeshUniforms,
    in vec3                  lFragmentColour,
    in float                 lfDepth,
    in float                 lfLUTEffectAmount )
{
    float lfLinearDepth = FastDenormaliseDepth( lPerFrameUniforms.gClipPlanesVec4, lfDepth );
    float lfMixFar      = saturate(lPerMeshUniforms.gColourLUTParamsVec4.x * lfLinearDepth);
    float lfMixStorm    = saturate(lPerMeshUniforms.gColourLUTParamsVec4.y * lfLinearDepth);
    float lfMixEffect   = saturate(lPerMeshUniforms.gColourLUTParamsVec4.z * lfLinearDepth);
    vec3 lFragmentColourResult = vec3(0.0, 0.0, 0.0);

    vec3 lFragColBase   = texture3DLod( SAMPLER_GETLOCAL( lPerMeshUniforms, gColourLUTBase ),   lFragmentColour, 0.0 ).rgb;
    vec3 lFragColFar    = lfMixFar    > 0.003 ? texture3DLod( SAMPLER_GETLOCAL( lPerMeshUniforms, gColourLUTFar ),    lFragmentColour, 0.0 ).rgb : vec3(0.0, 0.0, 0.0);
    vec3 lFragColStorm  = lfMixStorm  > 0.003 ? texture3DLod( SAMPLER_GETLOCAL( lPerMeshUniforms, gColourLUTStorm ),  lFragmentColour, 0.0 ).rgb : vec3(0.0, 0.0, 0.0);
    vec3 lFragColEffect = lfMixEffect > 0.003 ? texture3DLod( SAMPLER_GETLOCAL( lPerMeshUniforms, gColourLUTEffect ), lFragmentColour, 0.0 ).rgb : vec3(0.0, 0.0, 0.0);

    lFragColFar    = mix( lFragColBase, lFragColFar,    lfMixFar);
    lFragColStorm  = mix( lFragColBase, lFragColStorm,  lfMixStorm );
    lFragColEffect = mix( lFragColBase, lFragColEffect, lfMixEffect * ( 1.0 - lfLUTEffectAmount ) );

    lFragmentColourResult =  lFragColFar    * lPerMeshUniforms.gColourLUTStrengthsVec4.x;
    lFragmentColourResult += lFragColStorm  * lPerMeshUniforms.gColourLUTStrengthsVec4.y;
    lFragmentColourResult += lFragColEffect * lPerMeshUniforms.gColourLUTStrengthsVec4.z;

    return lFragmentColourResult;
}

#endif


#if defined( D_LENSFLARE ) || defined( D_DEBANDFILTER )

//-----------------------------------------------------------------------------
///
///     Deband
/// 
///     @brief      Noise based deband filter.
///
//-----------------------------------------------------------------------------

vec2
DebandNrand(
    vec2 n)
{
    vec2 result;
    result.x = fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
    result.y = fract(cos(mod(123456789.0, 1e-7 + 256. * dot(vec2(23.1406926327792690, 2.6651441426902251), n))));
    return result;
}

vec3
DebandFilter(
    in PerFrameUniforms        lRenderTargetUniforms,
    in vec3                    lFragCol,
    in vec2                    lTexCoords,
    in float                   lfDepth,
    SAMPLER2DARG(lDepthMap),
    SAMPLER2DARG(lColourMap)
)
{
    if (lRenderTargetUniforms.gFoVValuesVec4.z != 2.0) // don't deband in VR
    {
        if (lfDepth >= 1.0)
        {
            vec2 invViewportSize = lRenderTargetUniforms.gFrameBufferSizeVec4.zw;
            vec2 rand = DebandNrand(lTexCoords + fract(lRenderTargetUniforms.gfTime));
            vec2 offsetCoords = lTexCoords + (floor((rand * vec2(31.0, 31.0)) - 15.0) * invViewportSize.xy);
            lfDepth = texture2DLod(lDepthMap, offsetCoords, 0.0).x;

            if (lfDepth >= 1.0)
            {
                vec3 lFragColRandomTap = texture2DLod(lColourMap, offsetCoords, 0.0).xyz;   //GammaCorrectInput
                // We take the maximum extent of the difference between all of the colours,
                // to try and take into account the perceptual difference between different Hues
                // this is to mitigate the fact that in certain scenarios (eg. green shades against
                // a blue background), dithering can be more noticeable.
                // A difference of squares approach was tested to try and account
                // for the ramp-off - but in testing for some reason a linear approach worked better.
                vec3 lVecDiff = abs(lFragCol - lFragColRandomTap);
                float lExtentDiff = lVecDiff.x + lVecDiff.y + lVecDiff.z;
                lFragCol = (lExtentDiff < ((7.0 / 255.0))) ? lFragColRandomTap : lFragCol;
            }
        }
    }
    return lFragCol;
}

#endif

#if defined(D_SCREENEFFECT) && defined ( D_PLATFORM_METAL )
//TF_BEGIN

STATIC_CONST vec3 kLumcoeff = vec3(0.299, 0.587, 0.114);

vec3
Scanline(
	vec2  uv,
	float angle,
	vec3  color,
	float size,
	float strength,
	float decay)
{
	uv[1] -= (0.5 + 0.5 * cos(mod(angle, 3.14*2.0) / 2.0));
	uv[1] *= 1000.0 * size;

	float col = 1.0 / uv[1];
	float damp = clamp(pow(abs(uv[0]), decay) + pow(abs(1.0 - uv[0]), decay), 0.0, 1.0);
	col -= damp * 0.2;
	col = clamp(col, 0.0, strength);
	return color * col;
}

vec3
GetColourSeparation(
	SAMPLER2DARG(lBufferMap),
	float lfDistance,
	vec2  lScreenCoordsVec2)
{
	vec3 lSeparateColourVec3;

	lSeparateColourVec3.r = texture2DLod(lBufferMap, vec2(lScreenCoordsVec2.x + lfDistance, lScreenCoordsVec2.y), 0.0).x;
	lSeparateColourVec3.g = texture2DLod(lBufferMap, vec2(lScreenCoordsVec2.x, lScreenCoordsVec2.y), 0.0).y;
	lSeparateColourVec3.b = texture2DLod(lBufferMap, vec2(lScreenCoordsVec2.x - lfDistance, lScreenCoordsVec2.y), 0.0).z;

	return lSeparateColourVec3;
}

vec2
GetColorSeparationRB(
    SAMPLER2DARG( lBufferMap ),
    float lfDistance,
    vec2  lScreenCoordsVec2 )
{
    vec2 lSeparateColourVec2;

    lSeparateColourVec2.x = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x + lfDistance, lScreenCoordsVec2.y ), 0.0 ).x;
    lSeparateColourVec2.y = texture2DLod( lBufferMap, vec2( lScreenCoordsVec2.x - lfDistance, lScreenCoordsVec2.y ), 0.0 ).z;

    return lSeparateColourVec2;
}

vec3
GetVignette(
	vec2  lScreenCoordsVec2)
{
	float lfStart = 0.85;
	float lfVignette = 16.0 * lScreenCoordsVec2.x * lScreenCoordsVec2.y * (1.0 - lScreenCoordsVec2.x) * (1.0 - lScreenCoordsVec2.y);

	lfVignette = lfVignette + lfStart;
	lfVignette *= lfVignette;
	lfVignette = clamp(lfVignette, 0.0, 1.0);

	return vec3(lfVignette, lfVignette, lfVignette);
}

vec3
GetScanlines(
	float lfTime,
	vec2  lScreenCoordsVec2)
{
	vec3 lEffectsColourVec3 = vec3(0.95, 1.05, 0.95);

	lEffectsColourVec3 *= 0.95 + 0.05  * sin(10.0 * lfTime + lScreenCoordsVec2.y * 1280.0); // Scan lines
	lEffectsColourVec3 *= 0.995 + 0.005 * sin(110.0 * lfTime);                                // Pulse

	return lEffectsColourVec3;
}

vec3 GetDropletColourAdd(
	SAMPLER2DARG			(lBufferMap),
	in PerFrameUniforms      lRenderTargetUniforms,
	in CustomPerMeshUniforms lMeshUniforms,
	in vec3 lInputColor,
	vec2 texcoord,
	vec2 lDeformedTexcoordVec2 )
{
	vec3 finalColour = lInputColor;
	float timer = lMeshUniforms.gWashParamsVec4.x;
	float resettimer = lMeshUniforms.gWashParamsVec4.y;
	float dropfade = clamp(resettimer*10.0, 0.0, 1.0);

	float grainsize = 25.0;
	//texture edge bleed removal
	float fade = 12.0;
	vec2 distortFade = vec2(0.0, 0.0);
	distortFade.x = clamp(lDeformedTexcoordVec2.x*fade, 0.0, 1.0);
	distortFade.x -= clamp(1.0 - (1.0 - lDeformedTexcoordVec2.x)*fade, 0.0, 1.0);
	distortFade.y = clamp(lDeformedTexcoordVec2.y*fade, 0.0, 1.0);
	distortFade.y -= clamp(1.0 - (1.0 - lDeformedTexcoordVec2.y)*fade, 0.0, 1.0);
	float dfade = 1.0 - pow((1.0 - distortFade.x*distortFade.y), 2.0);

	if (resettimer < 1.0
#ifdef D_SCREENEFFECT_VR
		&& lRenderTargetUniforms.gFoVValuesVec4.z != 2.0
#endif
		)  // don't do droplet effect if not running or in VR
	{
		vec2 wave = vec2(0.0, 0.0);
		vec2 wavecoordR;
		vec2 wavecoordG;
		vec2 wavecoordB;
		wave.x = sin((texcoord.x - texcoord.y*4.0) - timer * 1.5)*0.25;
		wave.x += cos((texcoord.y*8.0 - texcoord.x*12.0) + timer * 4.2)*0.5;
		wave.x += sin((texcoord.x*18.0 + texcoord.y*16.0) + timer * 3.5)*0.25;

		wave.y = sin((texcoord.x*4.0 + texcoord.x*5.0) + timer * 2.5)*0.25;
		wave.y += cos((texcoord.y*6.0 + texcoord.x*12.0) - timer * 2.5)*0.5;
		wave.y += sin((texcoord.x*22.0 - texcoord.y*24.0) + timer * 4.5)*0.25;

		wave = wave * dfade;
		wavecoordR = texcoord - wave * 0.002;
		wavecoordG = texcoord - wave * 0.003;
		wavecoordB = texcoord - wave * 0.004;
		vec3 wavecolor = vec3(0.0, 0.0, 0.0);
		wavecolor.r = texture2DLod(lBufferMap, wavecoordR, 0.0).r;
		wavecolor.g = texture2DLod(lBufferMap, wavecoordG, 0.0).g;
		wavecolor.b = texture2DLod(lBufferMap, wavecoordB, 0.0).b;
		finalColour = mix(wavecolor, finalColour, dropfade);


		if (dfade > 0.0 && dropfade > 0.0)
		{
			//_SCE_BREAK();
			float noiz = 0.0;
			noiz += snoise(vec3(lDeformedTexcoordVec2*vec2(1280.0 / 90.0, 720.0 / 250.0) + vec2(0.0, timer*1.2), 1.0 + timer * 0.2))*0.25;
			noiz += snoise(vec3(lDeformedTexcoordVec2*vec2(1280.0 / 1200.0, 720.0 / 1800.0) + vec2(0.0, timer*0.5), 3.0 + timer * 0.3))*0.75;

			float dropletmask = smoothstep(0.02 + resettimer, 0.03 + resettimer, noiz*0.5 + 0.5);
			dropletmask = clamp(dropletmask, 0.0, 1.0);
			if (dropletmask > 0.0)
			{
				float droplet;
				droplet = clamp(smoothstep(0.0 + resettimer, 0.5 + resettimer, noiz*0.5 + 0.5), 0.0, 1.0);
				droplet = pow(clamp(droplet, 0.0, 1.0), 0.1)*3.0;
				vec2 droplets = vec2(dFdx(lDeformedTexcoordVec2 + vec2(droplet, droplet)).r, dFdy(lDeformedTexcoordVec2 + vec2(droplet, droplet)).g);
				vec2 dropcoordR;
				vec2 dropcoordG;
				vec2 dropcoordB;
				droplets = droplets * dfade;
				dropcoordR = (texcoord - droplets * 1.2);
				dropcoordG = (texcoord - droplets * 1.3);
				dropcoordB = (texcoord - droplets * 1.5);
				vec3 dropletcolor = vec3(0.0, 0.0, 0.0);
				dropletcolor.r = texture2DLod(lBufferMap, dropcoordR, 0.0).r;
				dropletcolor.g = texture2DLod(lBufferMap, dropcoordG, 0.0).g;
				dropletcolor.b = texture2DLod(lBufferMap, dropcoordB, 0.0).b;

				finalColour = mix(finalColour, dropletcolor, dropletmask*dropfade);
			}
		}
	}

	return finalColour;

}

vec3 ScreenEffect(
	in PerFrameUniforms				lPerFrameUniforms,
	in CommonPerMeshUniforms		lCommonMeshUniforms,
	in CustomPerMeshUniforms		lMeshUniforms,
	SAMPLER2DARG					(lUIFullscreenEffect),
	SAMPLER2DARG					(lUIFullscreenNormal),
	SAMPLER2DARG					(lUIFullScreenRefraction),
	SAMPLER2DARG					(lUICamoEffect),
	SAMPLER2DARG					(lUICamoNormal),
	SAMPLER2DARG					(lBufferMap),
	in vec3                         lInputColor,
	in vec2                         lTextureCoordinatesVec2
)
{
	vec2 lCoordsVec2 = lTextureCoordinatesVec2;
	vec2 lScreenspacePositionVec2 = SCREENSPACE_AS_RENDERTARGET_UVS(lCoordsVec2);
	vec3 lFragmentColourVec3 = lInputColor;
	vec2 lDeformedCoordsVec2;

	bool  lbIsUIEnabled = lMeshUniforms.gUIDeformVec4.w > -0.5 ? true : false;
	lbIsUIEnabled = lbIsUIEnabled && (lPerFrameUniforms.gFoVValuesVec4.z == 1.0); // disable in VR or if EEngineSetting_VignetteAndScanlines sets this off

	bool  lbIsVignetteEnabled = lbIsUIEnabled;
	lbIsVignetteEnabled = lbIsVignetteEnabled || (lMeshUniforms.gUIDeformVec4.w < -1.0 ? true : false);
	lbIsVignetteEnabled = lbIsVignetteEnabled && (lPerFrameUniforms.gFoVValuesVec4.z != 2.0);

	// Get the scene colour
	lFragmentColourVec3 = GetDropletColourAdd(SAMPLER2DPARAM(lBufferMap), lPerFrameUniforms, lMeshUniforms, lFragmentColourVec3, lCoordsVec2, lScreenspacePositionVec2);

	lDeformedCoordsVec2 = GetBulgeDeform(lCoordsVec2);

	float lfOverallMagnitude = lMeshUniforms.gUIDeformVec4.x;

	if (lfOverallMagnitude > 0.0)
	{
	    float lfFlickerAmount = lMeshUniforms.gUIDeformVec4.y;
		lDeformedCoordsVec2.x +=
			((lfFlickerAmount * 0.1) + 2.0 * (max(sin(lPerFrameUniforms.gfTime + sin(lPerFrameUniforms.gfTime * 113.0)), 0.98) - 0.98)) *
			0.05 *
			sin(113.0 * lPerFrameUniforms.gfTime * sin(lDeformedCoordsVec2.y * 827.0)) *
			lfOverallMagnitude;
	}

#ifdef D_PLATFORM_OPENGL
	lDeformedCoordsVec2.y = 1.0 - lDeformedCoordsVec2.y;
#endif

#ifdef D_SCREENEFFECT_VR
	lTextureCoordinatesVec2.x = (lTextureCoordinatesVec2.x - lPerFrameUniforms.gVREyeInfoVec3.y) * lPerFrameUniforms.gVREyeInfoVec3.z;
#endif

#ifdef D_PLATFORM_OPENGL
	lTextureCoordinatesVec2.y = 1.0 - lTextureCoordinatesVec2.y;
#endif


	float lfVignetteCutoffLow = lMeshUniforms.gVignetteVec4.x;
	float lfVignetteCutoffHigh = lMeshUniforms.gVignetteVec4.y;
	float lfVignetteAdjust = lMeshUniforms.gVignetteVec4.z;

	float lfRedFlash = lMeshUniforms.gCriticalHitPointsVec4.x;
	float lfFullScreenDesat = lMeshUniforms.gCriticalHitPointsVec4.y;
	float lfWhiteFlash = lMeshUniforms.gCriticalHitPointsVec4.z;
	float lfCamouflage = lMeshUniforms.gCriticalHitPointsVec4.w;

	float lfRedVignetteStrength = lMeshUniforms.gUILowHealthVignetteVec4.x;
	float lfLowHealthPulseRate = lMeshUniforms.gUILowHealthVignetteVec4.y;
	float lfShieldDownScanline = lMeshUniforms.gUILowHealthVignetteVec4.z;
	float lfEffectProgression = lMeshUniforms.gUILowHealthVignetteVec4.w;

	// Colour separation
	if (lbIsUIEnabled)
	{
        float lfEffectStrength = 1.0 - saturate( lDeformedCoordsVec2.y * 4.5 );
        if ( lfEffectStrength > 0.003 )
        {
#if defined( D_PLATFORM_METAL )
            vec2 lSeparateColourVec2;
            lFragmentColourVec3.rgb = lFragmentColourVec3.rbg;
            lSeparateColourVec2 = GetColorSeparationRB( SAMPLER2DPARAM(lBufferMap) , 3.0 / 1280.0, lCoordsVec2 );
            lFragmentColourVec3.rg = mix( lFragmentColourVec3.rg, lSeparateColourVec2, lfEffectStrength );
            lFragmentColourVec3.rgb = lFragmentColourVec3.rbg;
#else
            float lfColourSeparationStrength = saturate(lfEffectStrength);
            vec3 lSeparateColourVec3 = GetColourSeparation(SAMPLER2DPARAM(lBufferMap), 3.0 / 1280.0, lCoordsVec2);
            lFragmentColourVec3 = mix(lFragmentColourVec3, lSeparateColourVec3, lfColourSeparationStrength);  
#endif

            vec3 lEffectColourVec3 = lFragmentColourVec3;
            vec3 lVignetteColourVec3 = GetVignette(lScreenspacePositionVec2);
#ifndef D_SCREENEFFECT_VR
            vec3 lScanlineVec3 = GetScanlines(lPerFrameUniforms.gfTime, lDeformedCoordsVec2);
            lEffectColourVec3 *= lScanlineVec3;
#endif
            lEffectColourVec3 *= lVignetteColourVec3;
            lFragmentColourVec3 = mix(lFragmentColourVec3, lEffectColourVec3, lfEffectStrength);
        }
	}


	// Full screen vignette
	float dFullScreen = distance(lScreenspacePositionVec2.xy, vec2(0.5, 0.5));
	float lfLowHealthVignette = 1.0 - smoothstep(0.3, 1.0, dFullScreen);

	if (lbIsVignetteEnabled)
	{
		//
		// Circular vignette. Only have it on the top of the screen
		//
		vec2 lNewCoord = lScreenspacePositionVec2.xy;
		lNewCoord.y = max(lNewCoord.y, 0.5);
		// VR side-by-side offset/scale
		//lNewCoord.x = (lNewCoord.x - lPerFrameUniforms.gVREyeInfoVec3.y) * lPerFrameUniforms.gVREyeInfoVec3.z;
		float d = distance(lNewCoord, vec2(0.5, 0.5));
		float lfCircleVignette = 1.0 - smoothstep(lfVignetteCutoffLow, lfVignetteCutoffHigh, d);

		//lFragmentColourVec3 = mix(vec3(0.0, 0.0, 0.0), lFragmentColourVec3, lfCircleVignette);
        lFragmentColourVec3 *= lfCircleVignette;
	}

	// Blend in the UI colour
	// HAZARD
	float lfHazardProgression = saturate(1.0 - lfVignetteAdjust);
	float lfExtraEffect = saturate(lfVignetteAdjust - 1.0);
	lfHazardProgression = mix(lfHazardProgression, 0.0, lfExtraEffect);

	if (lfHazardProgression < 1.0)
	{
		vec3 lFullScreenNormalVec3 = DecodeNormalMap(texture2D(lUIFullscreenNormal, lTextureCoordinatesVec2));

#ifdef D_SCREENEFFECT_VR
		float lfDistortionSeperateAmount = 16.0 / 1280.0;
		vec2 lDistortedBufferCoords = lTextureCoordinatesVec2.xy + lFullScreenNormalVec3.xy * lfDistortionSeperateAmount;
		lDistortedBufferCoords.x = clamp(lDistortedBufferCoords.x, 0.0, 1.0);
		lDistortedBufferCoords.x = (lDistortedBufferCoords.x / lPerFrameUniforms.gVREyeInfoVec3.z) + lPerFrameUniforms.gVREyeInfoVec3.y;
#else
		float lfDistortionSeperateAmount = 64.0 / 1280.0;
		vec2 lDistortedBufferCoords = lCoordsVec2.xy + lFullScreenNormalVec3.xy * lfDistortionSeperateAmount;
#endif
		vec2 lDistortedTextureCoords = lTextureCoordinatesVec2.xy + lFullScreenNormalVec3.xy * lfDistortionSeperateAmount;
		vec4 lUIFullscreenVec4 = texture2DLod(lUIFullscreenEffect, lDistortedTextureCoords, 0.0);

		float lfHazardVignette = 1.0 - smoothstep(0.3, 1.0, dFullScreen * 2.0);
		float lfAlphaMask = mix(1.0 - lfHazardVignette, 1.0, lfExtraEffect);
		float lfHeightmapIntensity = clamp(lUIFullscreenVec4.a - lfHazardProgression, 0.0, 1.0) * lfAlphaMask;
		lfHeightmapIntensity *= clamp(1.0 / (1.0 - lfHazardProgression), 0.0, 1.0);
		lfHeightmapIntensity = mix(lfHeightmapIntensity, 1.0, lfExtraEffect);

		float lfHazardIntensity = lfHeightmapIntensity * step(lfHazardProgression, lUIFullscreenVec4.a);
		lfHazardIntensity = clamp(lfHazardIntensity, 0.0, 1.0);
		lfHazardIntensity = mix(lfHazardIntensity, 1.0, lfExtraEffect);

		if (lfHazardIntensity > 0.003)
		{
			// Lighting
            // but I moved screen effects to linear space, which greatly simplifies our pipeline.
            // Here we use a combination of squared interpolation params and sqrt_fast_0 colours
            // to mimic the old gamma space blending and (cheaply) match the old look
            lFragmentColourVec3     = sqrt_fast_1( lFragmentColourVec3 );
            lFragmentColourVec3    *= 1.0 - lfHazardIntensity;

			// Lighting
			vec3 lCameraRightVec3 = MAT4_GET_COLUMN(lPerFrameUniforms.gCameraNoHeadTrackingMat4, 0);
			vec3 lCameraUpVec3 = MAT4_GET_COLUMN(lPerFrameUniforms.gCameraNoHeadTrackingMat4, 1);
			vec3 lCameraAtVec3 = MAT4_GET_COLUMN(lPerFrameUniforms.gCameraNoHeadTrackingMat4, 2);
             vec4 lUIRefraction = texture2D(lUIFullScreenRefraction, lTextureCoordinatesVec2);

			//ec3 lLightColLinear = GammaCorrectOutput(lCommonMeshUniforms.gLightColourVec4.rgb);
			//vec3 lLightCol = mix(lLightColLinear, vec3(1.0, 1.0, 1.0), 0.5);

			//vec3 lDistortedCol = texture2DLod(lBufferMap, lDistortedBufferCoords, 0.0).rgb;
#ifdef D_SCREENEFFECT_VR
			//lDistortedCol.rgb = TonemapKodak(lDistortedCol.rgb) / TonemapKodak(vec3(1.0, 1.0, 1.0));
#endif


			// We want always some kind of specular value, a fixed light direction and a 
			// view angle using the hemisphere pointing in the opposite direction will do the trick
            vec3 lLightDir = reflect( vec3( 0.0, 0.0, 1.0 ), lFullScreenNormalVec3.xyz );
			vec3 lViewDir = normalize(vec3((lCameraAtVec3.x), (lCameraAtVec3.y), -1.0));
            float specularReflection    = lUIRefraction.y * pow( saturate( dot( lLightDir, lViewDir ) ), 256.0 );
            lFragmentColourVec3    += float2vec3( specularReflection ) * lfHazardIntensity;

			//float specularReflection = lUIRefraction.y * pow(max(0.0, dot(reflect(lLightDir, lFullScreenNormalVec3.xyz), lViewDir)), 250.0);

			//vec3 lHazardColour = mix(lDistortedCol, lUIFullscreenVec4.rgb * lLightCol, lUIRefraction.x) + float2vec3(specularReflection);

			//lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, lHazardColour, lfHazardIntensity);
            vec3 lLightCol          = lUIFullscreenVec4.rgb * ( sqrt_fast_0( lCommonMeshUniforms.gLightColourVec4.rgb ) * 0.5 + float2vec3( 0.5 ) );
            vec3 lDistortedCol      = sqrt_fast_0( texture2DLod(lBufferMap ,lDistortedBufferCoords, 0.0 ).rgb );
            vec3 lHazardColour      = mix( lDistortedCol * ( 1.0 - ( 1.0 - lfHazardVignette ) * lfExtraEffect ), lLightCol, lUIRefraction.x );
            lFragmentColourVec3    += lHazardColour * lfHazardIntensity;
            lFragmentColourVec3    *= lFragmentColourVec3;
		}
	}

	// Shield down desaturation and overlay effect
	if (lfEffectProgression > 0.0)
	{
        float lfLowHealthVignette = 1.0 - smoothstep( 0.3, 1.0, dFullScreen );
        float lfLuma              = dot( lFragmentColourVec3.rgb, kLumcoeff );
        lFragmentColourVec3       = mix( lFragmentColourVec3.rgb, float2vec3( lfLuma ), lfEffectProgression * ( 1.0 - lfLowHealthVignette ) );
	}

	// Shield down scanline
	float lfScanLineValue = abs(lfShieldDownScanline);
	float lfScanLineIntensity = 1.0 - abs(lfScanLineValue - 0.5) * 2.0;
	if (lfScanLineIntensity > 0.0)
	{
		vec2 lScanlineCoords = lDeformedCoordsVec2;
		float lfScanLineDirection = sign(lfShieldDownScanline);

		lFragmentColourVec3.rgb += Scanline(lScanlineCoords, lfScanLineValue * 3.1416 * 2.0, vec3(0.0, 0.8, 1.0) * 1.0, 0.025 * lfScanLineDirection, 0.2, 3.0) * lfScanLineIntensity;
		lFragmentColourVec3.rgb += Scanline(lScanlineCoords, lfScanLineValue * 3.1416 * 2.0, vec3(1.0, 1.0, 1.0) * 3.0, 0.25  * lfScanLineDirection, 0.2, 3.0) * lfScanLineIntensity;
	}

	// Desaturation
	{
		float lumFullScreen = dot(lFragmentColourVec3.rgb, kLumcoeff);
		lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, float2vec3(lumFullScreen) * 0.8, lfFullScreenDesat);
	}

	// Camouflage
	if (lfCamouflage > 0.0)
	{
		vec4 lCamoCol = texture2DLod(lUICamoEffect, lTextureCoordinatesVec2, 0.0);
		float lfNormalBlend = 0.0;
		if (lfCamouflage < 1.0)
		{
			// We want the "tiles" of the camo full-screen effect to grow-outwards whilst finishing at there final alpha (determined by the red
			// channel of the colour texture) - we therefore have a sliding window that will lerp different alpha values from 0 to their target
			// value based on the value in the alpha texture & the fade in window.
			const float lfFadeInWindow = 0.07;
			const float lfInvFadeInWindow = 1.0 / lfFadeInWindow;
			lCamoCol.a = saturate(((lCamoCol.a - 1.0) + (lfCamouflage * (1.0 + lfFadeInWindow))) * lfInvFadeInWindow);
            lfNormalBlend   = lCamoCol.a;
            lCamoCol.a      = lCamoCol.r * lCamoCol.a;
			lCamoCol.rgb = vec3(0.2, 0.25, 1.0);
		}

		vec3 lCamoNormal = DecodeNormalMap(texture2D(lUICamoNormal, lTextureCoordinatesVec2));

#ifdef D_SCREENEFFECT_VR
		float lfDistortionSeperateAmount = 16.0 / 1280.0;
		vec2 lDistortedBufferCoords = lTextureCoordinatesVec2.xy + lCamoNormal.xy * lfDistortionSeperateAmount * lfNormalBlend;
		lDistortedBufferCoords.x = clamp(lDistortedBufferCoords.x, 0.0, 1.0);
		lDistortedBufferCoords.x = (lDistortedBufferCoords.x / lPerFrameUniforms.gVREyeInfoVec3.z) + lPerFrameUniforms.gVREyeInfoVec3.y;
#else
		float lfDistortionSeperateAmount = 64.0 / 1280.0;
		vec2 lDistortedBufferCoords = lCoordsVec2.xy + lCamoNormal.xy * lfDistortionSeperateAmount * lfNormalBlend;
#endif

		vec3 lDistortedCol = texture2DLod(lBufferMap, lDistortedBufferCoords, 0.0).rgb;

#ifdef D_SCREENEFFECT_VR
		lDistortedCol = TonemapKodak(lDistortedCol.rgb) / TonemapKodak(vec3(1.0, 1.0, 1.0));
		lDistortedCol += texture2DLod(lBufferMap, lCoordsVec2, 0.0).xyz;
		lDistortedCol = GammaCorrectOutput(lDistortedCol);
#endif

		lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, lDistortedCol, lfCamouflage);
		lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, lCamoCol.rgb, lCamoCol.a);
	}

	// Red flash
	lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, vec3(0.8, 0.0, 0.0), lfRedFlash * 0.4);

	// Red vignette
	float lfCriticHealthVignette = 1.0 - smoothstep(0.05, 1.0, dFullScreen);
	float lfRedVignettePulseAmount = max(0.0, sin(lPerFrameUniforms.gfTime * lfLowHealthPulseRate));
	lFragmentColourVec3.rgb += vec3(lfVignetteCutoffHigh * 0.7 + 0.3 * lfRedVignettePulseAmount, 0.0, 0.0) * lfRedVignetteStrength * (1.0 - lfCriticHealthVignette);

	// White flash
	//lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, vec3(0.0, 0.0, 0.0), lfWhiteFlash);

	// Flash from front end. This is independent from HUD/hazard flashes.
	lFragmentColourVec3.rgb = mix(lFragmentColourVec3.rgb, lMeshUniforms.gFrontendFlashColourVec4.xyz, lMeshUniforms.gFrontendFlashColourVec4.w);

	return lFragmentColourVec3;
}
//TF_END
#endif
#endif

