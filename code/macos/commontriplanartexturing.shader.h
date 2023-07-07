////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonTriplanarTexturing.h
///     @author     User
///     @date       
///
///     @brief      CommonTriplanarTexturing
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

//-----------------------------------------------------------------------------
//      Include files

#ifndef D_COMMONTRIPLANARTEXTURING
#define D_COMMONTRIPLANARTEXTURING

#define D_PLATFORM_METAL // dont commit
#define D_LOW_QUALITY

#include "Common/CommonFragment.shader.h"

#define D_USE_FAST_SMOOTH_INTERPOLATION
#define D_NO_Z_NORM

#if defined ( D_PLATFORM_ORBIS ) && !defined( D_PLATFORM_PROSPERO )
#define D_PACKED_NORMALS
#define D_PACKED_HEIGHTS
#define D_PACKED_SPECULAR
#endif

// Leaving this here as it maybe help with occupancy in the future,
// but for now it's not really necessary
#if 0
//#if defined ( D_TERRAIN_T_SPLIT ) && !defined( D_PLATFORM_PROSPERO )
#define D_USE_PACKED_COORDS
#endif

//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Functions

STATIC_CONST float kfBlendPower         = 64.0;
STATIC_CONST float kfWeightFloor        = 0.003;
STATIC_CONST float kfWrapScale          = 512.0;
STATIC_CONST float kfNoiseTextScale     = 0.3125;
STATIC_CONST float kfNormBlend          = 0.85;
STATIC_CONST float kfNoiseNormScale     = 0.50;
STATIC_CONST half  kfNoiseHeightScale   = 0.50;
STATIC_CONST half  kfNoiseSpecularScale = 0.30;

void
PackR8(
    inout  uint u,
    float f)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 0, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 0, u);
#else
    u = (u & 0x00ffffff) | uint(f * 255.0) << 24;
#endif
}

void
PackG8(
    inout  uint u,
    float f)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 1, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 1, u);
#else
    u = (u & 0xff00ffff) | uint(f * 255.0) << 16;
#endif
}

void
PackB8(
    inout  uint u,
    float f)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 2, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 2, u);
#else
    u = (u & 0xffff00ff) | uint(f * 255.0) << 8;
#endif
}

void
PackA8(
    inout  uint u,
    float f)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 3, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 3, u);
#else
    u = (u & 0xffffff00) | uint(f * 255.0);
#endif
}

half
UnpackR8(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte0(u)) / 255.0;
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte0(u) / 255.0;
#else
    return half(u >> 24) / 255.0;
#endif
}

half
UnpackG8(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte1(u)) / 255.0;
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte1(u) / 255.0;
#else
    return half((u >> 16) & 0xff) / 255.0;
#endif
}

half
UnpackB8(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte2(u)) / 255.0;
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte2(u) / 255.0;
#else
    return half((u >> 8) & 0xff) / 255.0;
#endif
}

half
UnpackA8(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte3(u)) / 255.0;
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte3(u) / 255.0;
#else
    return half(u & 0xff) / 255.0;
#endif
}

float
UnpackR8NoDiv(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte0(u));
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte0(u);
#else
    return half(u >> 24);
#endif
}

float
UnpackG8NoDiv(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte1(u));
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte1(u);
#else
    return half((u >> 16) & 0xff);
#endif
}

float
UnpackB8NoDiv(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte2(u));
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte2(u);
#else
    return half((u >> 8) & 0xff);
#endif
}

float
UnpackA8NoDiv(
    in uint u)
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    return half(UnpackByte3(u));
#elif defined ( D_PLATFORM_XBOXONE )
    return __XB_UnpackByte3(u);
#else
    return half(u & 0xff);
#endif
}
uint
PackToRGBA8(
    float a,
    float b,
    float c,
    float d)
{
    uint u;

#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    vec4 input = vec4( a, b, c, d );
    u = PackFloat4ToUInt( input );
#elif defined ( D_PLATFORM_XBOXONE )
    PackR8(u, a); PackG8(u, b); PackB8(u, c); PackA8(u, d);
#else
    u  = uint( d * 255.0 );
    u |= uint( c * 255.0 ) << 8;
    u |= uint( b * 255.0 ) << 16;
    u |= uint( a * 255.0 ) << 24;
#endif

    return u;
}

void
UnpackFromRGBA8(
    in  uint  u,
    out float a, out float b, out float c, out float d )
{
    a = UnpackR8(u);
    b = UnpackG8(u);
    c = UnpackB8(u);
    d = UnpackA8(u);
}

uint
MixRGBA8Pack(
    uint  u1,
    uint  u2,
    vec4  f4)
{
    uint  u = 0;
    float m;
    m = mix( UnpackR8NoDiv(u1), UnpackR8NoDiv(u2), f4.x ) / 255.0; PackR8( u, m );
    m = mix( UnpackG8NoDiv(u1), UnpackG8NoDiv(u2), f4.y ) / 255.0; PackG8( u, m );
    m = mix( UnpackB8NoDiv(u1), UnpackB8NoDiv(u2), f4.z ) / 255.0; PackB8( u, m );
    m = mix( UnpackA8NoDiv(u1), UnpackA8NoDiv(u2), f4.w ) / 255.0; PackA8( u, m );
    return u;
}


void  PackR16(inout uint u, float f) { u = (u & 0x0000ffff) | uint(f * 65535.0) << 16; }
void  PackG16(inout uint u, float f) { u = (u & 0xffff0000) | uint(f * 65535.0); }
void  PackRG16(inout uint u, vec2  f2) { u = (uint(f2.x * 65535.0) << 16) | (uint(f2.y * 65535.0)); }

half  UnpackR16(in uint u) { return half(u >> 16) / 65535.0; }
half  UnpackG16(in uint u) { return half(u & 0xffff) / 65535.0; }
vec2  UnpackRG16(in uint u) { return vec2(UnpackR16(u), UnpackG16(u)); }

#define PackItemXChannel(id) ((id) & 0x1f)
// uint
// PackItemXChannel(
//     uint id
// )
// {
//     return id & 0x1f;
// }

#define PackItemYChannel(id) (( (id) & 0x1f ) << 5)
// uint
// PackItemYChannel(
//     uint id
// )
// {
//     return ( id & 0x1f ) << 5;
// }

#define PackItemZChannel(id) (( (id) & 0x1f ) << 10)
// uint
// PackItemZChannel(
//     uint id
// )
// {
//     return ( id & 0x1f ) << 10;
// }

#define PackItemWChannel(id) (( (id) & 0x1f ) << 15)

// uint
// PackItemWChannel(
//     uint id
// )
// {
//     return ( id & 0x1f ) << 15;
// }

#define UnpackItemXChannel(id) ((id) & 0x1f)

// uint
// UnpackItemXChannel(
//     uint id
// )
// {
//     return id & 0x1f;
// }

#define UnpackItemYChannel(id) (( (id) >> 5 ) & 0x1f)
// uint
// UnpackItemYChannel(
//     uint id
// )
// {
//     return ( id >> 5 ) & 0x1f;
// }

#define UnpackItemZChannel(id) (( (id) >> 10 ) & 0x1f)

// uint
// UnpackItemZChannel(
//     uint id
// )
// {
//     return ( id >> 10 ) & 0x1f;
// }
#define UnpackItemWChannel(id) (( (id) >> 15 ) & 0x1f)
// uint
// UnpackItemWChannel(
//     uint id
// )
// {
//     return ( id >> 15 ) & 0x1f;
// }

vec3
GetTriPlanarColour(
    vec3  lNormalVec3,
    vec3  lWorldPositionVec3,
    vec2  lAnimationOffsetVec2,
    float lfScale,
    SAMPLER2DARG( lTexture ) )
{
#if defined( D_TERRAIN_X_FACING )
    vec3    m = vec3( 1.0, 0.0, 0.0 );
#elif defined( D_TERRAIN_Y_FACING )
    vec3    m = vec3( 0.0, 1.0, 0.0 );
#elif defined( D_TERRAIN_Z_FACING )
    vec3    m = vec3( 0.0, 0.0, 1.0 );
#else
    vec3    m = pow( abs( lNormalVec3 ), vec3( kfBlendPower, kfBlendPower, kfBlendPower ) );
#endif

    vec2  lCoord1Vec2 = lWorldPositionVec3.yz * lfScale + lAnimationOffsetVec2;
    vec2  lCoord2Vec2 = lWorldPositionVec3.zx * lfScale + lAnimationOffsetVec2;
    vec2  lCoord3Vec2 = lWorldPositionVec3.xy * lfScale + lAnimationOffsetVec2;

    vec3 lColour1Vec3 = texture2DLod( lTexture, lCoord1Vec2, 0.0 ).rgb;
    vec3 lColour2Vec3 = texture2DLod( lTexture, lCoord2Vec2, 0.0 ).rgb;
    vec3 lColour3Vec3 = texture2DLod( lTexture, lCoord3Vec2, 0.0 ).rgb;

    return ( lColour1Vec3 * m.x + lColour2Vec3 * m.y + lColour3Vec3 * m.z ) / ( m.x + m.y + m.z );
}

// mip-mapped variant of the above

vec3
GetTriPlanarColourMM(
    vec3  lNormalVec3,
    vec3  lWorldPositionVec3,
    vec2  lAnimationOffsetVec2,
    float lfScale,
    SAMPLER2DARG( lTexture ) )
{
#if defined( D_TERRAIN_X_FACING )
    vec3    m = vec3( 1.0, 0.0, 0.0 );
#elif defined( D_TERRAIN_Y_FACING )
    vec3    m = vec3( 0.0, 1.0, 0.0 );
#elif defined( D_TERRAIN_Z_FACING )
    vec3    m = vec3( 0.0, 0.0, 1.0 );
#else
    vec3    m = pow( abs( lNormalVec3 ), vec3( kfBlendPower, kfBlendPower, kfBlendPower ) );
#endif

#if 1
    vec2  lCoord1Vec2 = lWorldPositionVec3.yz * lfScale + lAnimationOffsetVec2;
    vec2  lCoord2Vec2 = lWorldPositionVec3.zx * lfScale + lAnimationOffsetVec2;
    vec2  lCoord3Vec2 = lWorldPositionVec3.xy * lfScale + lAnimationOffsetVec2;

    vec3 lColour1Vec3 = texture2DComputeGrad( lTexture, lCoord1Vec2 ).rgb;
    vec3 lColour2Vec3 = texture2DComputeGrad( lTexture, lCoord2Vec2 ).rgb;
    vec3 lColour3Vec3 = texture2DComputeGrad( lTexture, lCoord3Vec2 ).rgb;

    return ( lColour1Vec3 * m.x + lColour2Vec3 * m.y + lColour3Vec3 * m.z ) / ( m.x + m.y + m.z );
#else
    
    float lfInverseScale = 1.0 / ( m.x + m.y + m.z );
    m *= lfInverseScale;

    vec2  lCoord1Vec2 = lWorldPositionVec3.yz * lfScale + lAnimationOffsetVec2;
    vec3 lColourVec3 = texture2DComputeGrad( lTexture, lCoord1Vec2 ).rgb * m.x;

    vec2  lCoord2Vec2 = lWorldPositionVec3.zx * lfScale + lAnimationOffsetVec2;
    lColourVec3 += texture2DComputeGrad( lTexture, lCoord2Vec2 ).rgb * m.y;

    vec2 lCoord3Vec2 = lWorldPositionVec3.xy * lfScale + lAnimationOffsetVec2;
    lColourVec3 += texture2DComputeGrad( lTexture, lCoord3Vec2 ).rgb * m.z;

    return lColourVec3;

#endif
}


#ifdef D_TEXTURE_ARRAYS
//-----------------------------------------------------------------------------
///
///     GetTriPlanarNormal
///
///     @brief      GetTriPlanarNormal
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------

float
GetBias(
    float time,
    float bias )
{
    return ( time / ( ( ( ( 1.0 / bias ) - 2.0 )*( 1.0 - time ) ) + 1.0 ) );
}

half3
BlendNormalsUDN(    
    half3 lBaseNormal,
    half3 lDetailNormal,
    float lfDetailScale )
{        
    half3 n;
    half3 n1 = lBaseNormal;
    half3 n2 = lDetailNormal;
    n2.xy   *= half( lfDetailScale );
    //n        = normalize( half3( n1.xy + n2.xy, n1.z ) );
    n        = half3( n1.xy + n2.xy, n1.z );
    return n;
}

half3
BlendNormalsRNM(
    half3 lBaseNormal,
    half3 lDetailNormal)
{
    half3 n;
    half3 n1 = lBaseNormal;
    half3 n2 = lDetailNormal;
    n1.z  +=  1.0;
    n2.xy *= -1.0;
    n      = n1*dot(n1, n2)/n1.z - n2;
    return n;
}

vec3
BlendNormalsUDN_FP(
    vec3 lBaseNormal,
    vec3 lDetailNormal,
    float lfDetailScale)
{
    vec3 n;
    vec3 n1 = lBaseNormal;
    vec3 n2 = lDetailNormal;
    n2.xy *= lfDetailScale;
    //n        = normalize( vec3( n1.xy + n2.xy, n1.z ) );
    n = vec3(n1.xy + n2.xy, n1.z);
    return n;
}

vec3
BlendNormalsRNM_FP(
    vec3 lBaseNormal,
    vec3 lDetailNormal)
{
    vec3 n;
    vec3 n1 = lBaseNormal;
    vec3 n2 = lDetailNormal;
    n1.z += 1.0;
    n2.xy *= -1.0;
    n = n1 * dot(n1, n2) / n1.z - n2;
    return n;
}

#define GetSpecularNoiseCoeff( \
    lSpecularNoiseCoeff, \
    lCustomUniforms, \
    liIndex \
) \
{ \
    uint  luDiv = uint( liIndex ) / 4; \
    uint  luMod = uint( liIndex ) - luDiv * 4; \
    vec2  lSpecCoeff; \
    if ( luMod > 1 ) \
    { \
        lSpecCoeff = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaSpecularValuesVec4, luDiv ).zw;     \
    } \
    else \
    { \
        lSpecCoeff = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaSpecularValuesVec4, luDiv ).xy;     \
    } \
 \
    lSpecularNoiseCoeff = half( ( luMod & 0x1 ) == 0 ? lSpecCoeff.x : lSpecCoeff.y ); \
} while(0) \

half
GetSpecularNoiseCoeff2( 
    in  CustomPerMaterialUniforms lCustomUniforms,
    in  int                       liIndex )
{
    //vec4   lSpecularCoeffVec4 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaSpecularValuesVec4, liIndex / 4 );
    //return half( lSpecularCoeffVec4[ liIndex % 4 ] );

    // The code above, although seemingly simpler produces terrible assembly
    // in particular, it compiles into a buffer_load_dwordx4, eating up 4 vgprs like a mofo
    // The following code, although a bit wank, produces better assembly
    // compiling into buffer_load_dword, and taking up only 1 vgpr


#if !defined( D_SIMPLIFIED_NOISE )
    uint  luDiv = uint( liIndex ) / 4;
    uint  luMod = uint( liIndex ) - luDiv * 4;
    vec2  lSpecCoeff;
    if ( luMod > 1 )
    {
        lSpecCoeff = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaSpecularValuesVec4, luDiv ).zw;    
    }
    else
    {
        lSpecCoeff = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaSpecularValuesVec4, luDiv ).xy;    
    }

    return half( ( luMod & 0x1 ) == 0 ? lSpecCoeff.x : lSpecCoeff.y );
#else
    return 0.0;
#endif
}

#define GetTriPlanarNoiseNormal( \
    lNoiseNormal, \
    lCoordVec2, \
    lNoiseNormalMap, \
    lfHeight, \
    lfSpecular \
) \
{ \
    vec4  lLocalNormalAVec4 = texture2D( EVAL(lNoiseNormalMap), lCoordVec2 ).rgba; \
    lLocalNormalAVec4.rg    = lLocalNormalAVec4.rg * 2.0 - 1.0; \
    lfSpecular              =  ( 1.0 - half( lLocalNormalAVec4.b ) ) * kfNoiseSpecularScale; \
    lfHeight                =  ( half( lLocalNormalAVec4.a ) - 0.5 ) * kfNoiseHeightScale; \
    lLocalNormalAVec4.b     = 0.0; \
    lNoiseNormal = half3( lLocalNormalAVec4.rgb ); \
} while(0)

void GetTriPlanarNoiseNormal2(
    in half3 lNoiseNormal,
    in    vec2      lCoordVec2,
    SAMPLER2DARG(   lNoiseNormalMap ),
    out   half      lfHeight,
    out   half      lfSpecular )
{        
#if !defined( D_SIMPLIFIED_NOISE )
    vec4  lLocalNormalAVec4 = texture2D( lNoiseNormalMap, lCoordVec2 ).rgba;    
    lLocalNormalAVec4.rg    = lLocalNormalAVec4.rg * 2.0 - 1.0;
    lfSpecular              =  ( 1.0 - half( lLocalNormalAVec4.b ) ) * kfNoiseSpecularScale;
    lfHeight                =  ( half( lLocalNormalAVec4.a ) - 0.5 ) * kfNoiseHeightScale;
#else
    vec4  lLocalNormalAVec4;
    lLocalNormalAVec4.rg    = texture2D( lNoiseNormalMap, lCoordVec2 ).rg;    
    lfHeight                = 0.0;
    lfSpecular              = 0.0;
#endif
    lLocalNormalAVec4.b     = 0.0;

    lNoiseNormal = half3( lLocalNormalAVec4.rgb );
}
#define EVAL(X) X
#define GetTriPlanarNormalArray(normal, lCustomUniforms, lCoordVec2, lNormalMap, lSubstanceNormalMap, liIndex, lfHeight, lfDisplacement, lSpecular) \
{ \
    vec4 lTexValueVec4 = (liIndex < 16) ? \
    texture2DArray( EVAL(lNormalMap),          vec3( lCoordVec2, float( liIndex ) ) ) : \
    texture2DArray( EVAL(lSubstanceNormalMap), vec3( lCoordVec2, float( liIndex & 15 ) ) ); \
    float lfBrightness  = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).a; \
    float lfContrast    = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).a; \
    lfDisplacement      = lTexValueVec4.r; \
    lfHeight            = lTexValueVec4.r; \
    lfHeight           += lfBrightness; \
    lfHeight            = saturate( ( ( lfHeight - 0.5 ) * max( lfContrast + 1.0, 0.0 ) ) + 0.5 ); \
    lSpecular.x         = half(lTexValueVec4.b); \
    GetSpecularNoiseCoeff( lSpecular.y, lCustomUniforms, liIndex ); \
    normal = half3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 ); \
} while(0)

// half3
// GetTriPlanarNormalArray(
//     in  CustomPerMaterialUniforms lCustomUniforms,
//     in  vec2           lCoordVec2,
//     SAMPLER2DARRAYARG( lNormalMap ),
//     SAMPLER2DARRAYARG( lSubstanceNormalMap ),
//     in  int            liIndex,
//     out float          lfHeight,
//     out float          lfDisplacement,
//     out half2          lSpecular )
// {
//     vec4 lTexValueVec4;

//     // if ( 1)//liIndex < 16 )
//     // {
//     //     lTexValueVec4 = texture2DArray( lNormalMap, vec3( lCoordVec2, float( liIndex ) ) );
//     // }
//     // else
//     // {
//     //     lTexValueVec4 = texture2DArray( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ) );
//     // }

//     lTexValueVec4 = vec4(lCoordVec2, lCoordVec2);
    
//     float lfBrightness  = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).a;
//     float lfContrast    = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).a;

//     lfDisplacement      = lTexValueVec4.r;
//     lfHeight            = lTexValueVec4.r;
//     lfHeight           += lfBrightness;
//     lfHeight            = saturate( ( ( lfHeight - 0.5 ) * max( lfContrast + 1.0, 0.0 ) ) + 0.5 );

//     lSpecular.x         = half(lTexValueVec4.b);
//     lSpecular.y         = GetSpecularNoiseCoeff( lCustomUniforms, liIndex );

//     vec3 lLocalNormalAVec3 = vec3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
// #if !defined( D_NO_Z_NORM )
//     lLocalNormalAVec3.z = sqrt(saturate(1.0 - lLocalNormalAVec3.x*lLocalNormalAVec3.x - lLocalNormalAVec3.y*lLocalNormalAVec3.y));
// #endif

//     return half3(lLocalNormalAVec3);

// }

//-----------------------------------------------------------------------------
///
///     GetTriPlanarColour
///
///     @brief      GetTriPlanarColour
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec3
GetTriPlanarColourArray2(
    in vec2            lCoordVec2,
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    in int             liIndex,
    in vec3            lAverageHSVVec3,
    in vec3            lRecolourVec3 )
{
    vec3 lColourVec3;
    // if ( 1)//liIndex < 16 )
    // {
    //     lColourVec3 = texture2DArray( lDiffuseMap, vec3( lCoordVec2, float( liIndex ) ) ).rgb;
    // }
    // else
    // {
    //     lColourVec3 = texture2DArray( lSubstanceDiffuseMap, vec3( lCoordVec2, float( liIndex & 15 ) ) ).rgb;
    // }


    lColourVec3 = lCoordVec2.xyy;

    lColourVec3.r = fract( ( lColourVec3.r - lAverageHSVVec3.r ) + lRecolourVec3.r );
    lColourVec3.g = saturate( min( lRecolourVec3.g, lColourVec3.g ) );
    lColourVec3.b = saturate( ( lColourVec3.b - lAverageHSVVec3.b ) + lRecolourVec3.b );
    lColourVec3 = saturate( HSVToRGB( lColourVec3 ) );

    return lColourVec3;
}


#define GetTriPlanarColourArray( Colour, lCoordVec2, lDiffuseMap, lSubstanceDiffuseMap, liIndex, lAverageHSVVec3, lRecolourVec3 ) \
{ \
    if ( liIndex < 16 ) \
        Colour = texture2DArray( EVAL(lDiffuseMap), vec3( lCoordVec2, float( liIndex ) ) ).rgb; \
    else \
        Colour = texture2DArray( EVAL(lSubstanceDiffuseMap), vec3( lCoordVec2, float( liIndex & 15 ) ) ).rgb; \
    Colour.r = fract( ( Colour.r - lAverageHSVVec3.r ) + lRecolourVec3.r ); \
    Colour.g = saturate( min( lRecolourVec3.g, Colour.g ) ); \
    Colour.b = saturate( ( Colour.b - lAverageHSVVec3.b ) + lRecolourVec3.b ); \
    Colour = saturate( HSVToRGB( Colour ) ); \
} while(0)
//     SAMPLER2DARRAYARG( lDiffuseMap ),
//     SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
//     in int             liIndex,
//     in vec3            lAverageHSVVec3,
//     in vec3            lRecolourVec3 )
// {
//     vec3 lColourVec3;
//     // if ( 1)//liIndex < 16 )
//     // {
//     //     lColourVec3 = texture2DArray( lDiffuseMap, vec3( lCoordVec2, float( liIndex ) ) ).rgb;
//     // }
//     // else
//     // {
//     //     lColourVec3 = texture2DArray( lSubstanceDiffuseMap, vec3( lCoordVec2, float( liIndex & 15 ) ) ).rgb;
//     // }


//     lColourVec3 = lCoordVec2.xyy;

//     lColourVec3.r = fract( ( lColourVec3.r - lAverageHSVVec3.r ) + lRecolourVec3.r );
//     lColourVec3.g = saturate( min( lRecolourVec3.g, lColourVec3.g ) );
//     lColourVec3.b = saturate( ( lColourVec3.b - lAverageHSVVec3.b ) + lRecolourVec3.b );
//     lColourVec3 = saturate( HSVToRGB( lColourVec3 ) );

//     return lColourVec3;
// }

typedef half4 Half8[2];
typedef float4 Float8[2];

// struct Half8
// {
//     half4 m[2];
// };
// struct Float8
// {
//     float4 m[2];
// };

//-----------------------------------------------------------------------------
///
///     GetBlendedNormal
///
///     @brief      GetBlendedNormal
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
struct BlendedNormal
{
    half3 mNormal;
    Half8 mHeights;
    half  mSpecular;
    half  mSpecularCoeff;
    half  mDisplacement;
};
#if 0
BlendedNormal
GetBlendedNormal2(
    const  CustomPerMaterialUniforms lCustomUniforms,
    const    vec2         lTileCoordsVec2,
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    const    uint         lTile1UInt,
    const    uint         lTile2UInt,
    const half4        lhTileInfo
)
{

    BlendedNormal bn;

    #define lfHeight1       (bn.mHeights.m[0].x)
    #define lfHeight2       (bn.mHeights.m[0].y)
    #define lfHeight3       (bn.mHeights.m[0].z)
    #define lfHeight4       (bn.mHeights.m[0].w)
    #define lfHeight5       (bn.mHeights.m[1].x)
    #define lfHeight6       (bn.mHeights.m[1].y)
    #define lfHeight7       (bn.mHeights.m[1].z)
    #define lfHeight8       (bn.mHeights.m[1].w)
    #define lfSpecular      (bn.mSpecular)
    #define lfSpecularCoeff (bn.mSpecularCoeff)
    #define lfDisplacement  (bn.mDisplacement)

    #define lfPatch    lhTileInfo.x
    #define lfSlope2   lhTileInfo.z
    #define lfSlope1   lhTileInfo.y
    #define lfTileType lhTileInfo.w

#ifndef D_LOW_QUALITY

    const float lfThreshold = 1.0 / 20.0;
    const float lfOffset = 0.1;
    const float lfInverseOffset2 = 1.0 / ( lfOffset + lfOffset );
    const float lfDefaultHeight = 0.5;

    half3  lNormal5;
    half3  lNormal6;

    half2 lSpecular5;
    half2 lSpecular6;

    float lfHeightE;
    float lfHeightF;
    float lfHeightNoise;
    float lfDisplacementE;
    float lfDisplacementF;
    float lfDisplacementNoise;

    lfHeight1 = 0.0;
    lfHeight2 = 0.0;
    lfHeight3 = 0.0;
    lfHeight4 = 0.0;
    lfHeight5 = 0.0;
    lfHeight6 = 0.0;
    lfHeight7 = 0.0;
    lfHeight8 = 0.0;

    if ( lfTileType < 1.0 - lfThreshold )
    {
        half3  lNormal3;
        half3  lNormal4;

        float lfHeightC;
        float lfHeightD;
        float lfDisplacementC;
        float lfDisplacementD;

        half2 lSpecular3;
        half2 lSpecular4;

        if ( lfSlope1 < 1.0 - lfThreshold )
        {
            half3  lNormal1;
            half3  lNormal2;

            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;
            half2 lSpecular1;
            half2 lSpecular2;

            if ( lfPatch < 1.0 - lfThreshold )
            {
                 GetTriPlanarNormalArray(
                    lNormal1,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemXChannel( lTile1UInt ) ),
                    lfHeightA,
                    lfDisplacementA,
                    lSpecular1 );
            }
            else
            {
                lNormal1 = half3( 0.0, 0.0, 0.0 );

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;
                lSpecular1 = half2( 0.0, 0.0 );
            }

            if ( lfPatch > lfThreshold )
            {
                 GetTriPlanarNormalArray(lNormal2,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemYChannel( lTile1UInt ) ),
                    lfHeightB,
                    lfDisplacementB,
                    lSpecular2 );
            }
            else
            {
                lNormal2 = half3( 0.0, 0.0, 0.0 );

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;
                lSpecular2 = half2( 0.0, 0.0 );
            }

            // blend between height textures
            lfHeight1 = half(mix(lfHeightA, 1.0 - lfHeightB, lfPatch));
            half lfHeight = half(FastSmoothStep(lfHeight1 - lfOffset, 2.0f * lfOffset, lfPatch));

            lNormal3 = half3( mix( lNormal1, lNormal2, lfHeight ) );
            
            lSpecular3 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );

            // blend between height maps again to get map which can be combined with other height maps
            lfHeightC = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementC = mix( lfDisplacementA, lfDisplacementB, float(lfHeight) );
        }
        else
        {
            lNormal3 = half3(0.0, 0.0, 0.0);
            lfHeightC = lfDefaultHeight;
            lfDisplacementC = lfDefaultHeight;
            lSpecular3 = half2( 0.0, 0.0 );
        }

        if ( lfSlope1 > lfThreshold )
        {
            half3  lNormal1;
            half3  lNormal2;

            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;
            half2 lSpecular1;
            half2 lSpecular2;

            if ( lfPatch < 1.0 - lfThreshold )
            {
                GetTriPlanarNormalArray(lNormal1,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemZChannel( lTile1UInt ) ),
                    lfHeightA,
                    lfDisplacementA,
                    lSpecular1 );
            }
            else
            {
                lNormal1 = half3( 0.0, 0.0, 0.0 );

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;
                lSpecular1 = half2( 0.0, 0.0 );
            }

            if ( lfPatch > lfThreshold )
            {
                GetTriPlanarNormalArray(lNormal2
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemWChannel( lTile1UInt ) ),
                    lfHeightB,
                    lfDisplacementB,
                    lSpecular2 );
            }
            else
            {
                lNormal2 = half3(0.0, 0.0, 0.0);

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;
                lSpecular2 = half2( 0.0, 0.0 );
            }

            lfHeight2 = half(mix(lfHeightA, 1.0 - lfHeightB, lfPatch));
            float lfHeight = FastSmoothStep(lfHeight2 - lfOffset, 2.0f * lfOffset, lfPatch);
            lNormal4 = half3( mix( lNormal1, lNormal2, lfHeight ) );
            lSpecular4 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );
            
            lfHeightD = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementD = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
            lNormal4 = half3( 0.0, 0.0, 0.0 );

            lfHeightD = lfDefaultHeight;
            lfDisplacementD = lfDefaultHeight;
            lSpecular4 = half2( 0.0, 0.0 );
        }

        lfHeight5 = half( mix ( lfHeightC, 1.0 - lfHeightD, lfSlope1 ) );
        float lfHeight = FastSmoothStep(lfHeight5 - lfOffset, 2.0f * lfOffset, lfSlope1);
        
        lNormal5 = half3( mix( lNormal3, lNormal4, lfHeight ) );
        lSpecular5 = half2( mix( lSpecular3, lSpecular4, lfHeight ) );
        
        lfHeightE = mix( lfHeightC, lfHeightD, lfSlope1 );
        lfDisplacementE = mix( lfDisplacementC, lfDisplacementD, lfHeight );
    }
    else
    {
        lNormal5 = half3( 0.0, 0.0, 0.0 );

        lSpecular5 = half2( 0.0, 0.0 );

        lfHeightE = lfDefaultHeight;
        lfDisplacementE = lfDefaultHeight;
    }

    if ( lfTileType > lfThreshold )
    {
        half3  lNormal3;
        half3  lNormal4;

        float lfHeightC;
        float lfHeightD;
        float lfDisplacementC;
        float lfDisplacementD;

        half2 lSpecular3;
        half2 lSpecular4;

        if ( lfSlope2 < 1.0 - lfThreshold )
        {
            half3  lNormal1;
            half3  lNormal2;

            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;

            half2 lSpecular1;
            half2 lSpecular2;

            if ( lfPatch < 1.0 - lfThreshold )
            {
                GetTriPlanarNormalArray(lNormal1,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemXChannel( lTile2UInt ) ),
                    lfHeightA,
                    lfDisplacementA,
                    lSpecular1 );
            }
            else
            {
                lNormal1 = half3(0.0, 0.0, 0.0);

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;

                lSpecular1 = half2( 0.0, 0.0 );
            }

            if ( lfPatch > lfThreshold )
            {
                GetTriPlanarNormalArray(lNormal2,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemYChannel( lTile2UInt ) ),
                    lfHeightB,
                    lfDisplacementB,
                    lSpecular2 );
            }
            else
            {
                lNormal2 = half3(0.0, 0.0, 0.0);

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;
                lSpecular2 = half2( 0.0, 0.0 );
            }

            lfHeight3 = half( mix( lfHeightA, 1.0 - lfHeightB, lfPatch ) );
            float lfHeight = FastSmoothStep(lfHeight3 - lfOffset, 2.0f * lfOffset, lfPatch);
            lNormal3 = half3( mix( lNormal1, lNormal2, lfHeight ) );
            
            lSpecular3 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );
            
            lfHeightC = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementC = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
            lNormal3 = half3( 0.0, 0.0, 0.0 );

            lfHeightC = lfDefaultHeight;
            lfDisplacementC = lfDefaultHeight;

            lSpecular3 = half2( 0.0, 0.0 );
        }

        if ( lfSlope2 > lfThreshold )
        {
            half3  lNormal1;
            half3  lNormal2;

            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;

            half2 lSpecular1;
            half2 lSpecular2;

            if ( lfPatch < 1.0 - lfThreshold )
            {
                GetTriPlanarNormalArray(lNormal1,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemZChannel( lTile2UInt ) ),
                    lfHeightA,
                    lfDisplacementA,
                    lSpecular1 );
            }
            else
            {

                lNormal1 = half3(0.0, 0.0, 0.0);

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;

                lSpecular1 = half2( 0.0, 0.0 );
            }

            if ( lfPatch > lfThreshold )
            {
                GetTriPlanarNormalArray(lNormal2,
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    int( UnpackItemWChannel( lTile2UInt ) ),
                    lfHeightB,
                    lfDisplacementB,
                    lSpecular2 );
            }
            else
            {
                lNormal2 = half3(0.0, 0.0, 0.0);

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;
                lSpecular2 = half2( 0.0, 0.0 );
            }

            lfHeight4 = half( mix( lfHeightA, 1.0 - lfHeightB, lfPatch ) );
            float lfHeight = FastSmoothStep( lfHeight4 - lfOffset, 2.0f * lfOffset, lfPatch );
            
            lNormal4 = half3( mix( lNormal1, lNormal2, lfHeight ) );
            lSpecular4 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );

            lfHeightD = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementD = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {

            lNormal4 = half3(0.0, 0.0, 0.0);

            lfHeightD = lfDefaultHeight;
            lfDisplacementD = lfDefaultHeight;

            #if defined ( D_PACKED_SPECULAR )
            lSpecular4 = 0;
            #else
            lSpecular4 = half2( 0.0, 0.0 );
            #endif
        }

#if defined (D_STOCHASTIC_TERRAIN)
        float lfHeight = 0.0;
        {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )

            float lfPreScaledHeight = lfHeightC - lfHeightD * lfSlope2 - lfHeightC * lfSlope2 + lfSlope2;
            lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfSlope2 ) * lfInverseOffset2 );
            lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
            lfHeight6 += lfPreScaledHeight * lfScale;
#else
            float lfPreScaledHeight = mix( lfHeightC, 1.0 - lfHeightD, lfSlope2 );
            lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfSlope2 );
            lfHeight6 += lfPreScaledHeight * lfScale;
#endif
        }
#else
        #if defined ( D_PACKED_HEIGHTS )
        float lfTempHeight = half( mix( lfHeightC, 1.0 - lfHeightD, lfSlope2 ) );
        lfHeight6 = PackHeightsAB( lfTempHeight, lfTempHeight );
        float lfHeight = FastSmoothStep( lfTempHeight - lfOffset, 2.0f * lfOffset, lfSlope2 );
        #else
        lfHeight6 = half( mix( lfHeightC, 1.0 - lfHeightD, lfSlope2 ) );
        float lfHeight = FastSmoothStep( lfHeight6 - lfOffset, 2.0f * lfOffset, lfSlope2 );
        #endif
#endif

#if defined ( D_PACKED_NORMALS )

        vec3 lUnpackedNormal3;
        vec3 lUnpackedNormal4;
        vec3 lUnpackedNormal6;

        UnpackNormalsXYZ(
            lNormal3,
            lUnpackedNormal3.x,
            lUnpackedNormal3.y,
            lUnpackedNormal3.z);

        UnpackNormalsXYZ(
            lNormal4,
            lUnpackedNormal4.x,
            lUnpackedNormal4.y,
            lUnpackedNormal4.z);

        lUnpackedNormal6 = mix( lUnpackedNormal3, lUnpackedNormal4, lfHeight );

        lNormal6 = PackNormalsXYZ(
            lUnpackedNormal6.x,
            lUnpackedNormal6.y,
            lUnpackedNormal6.z);
#else
        lNormal6 = half3( mix( lNormal3, lNormal4, lfHeight ) );
#endif

        #if defined ( D_PACKED_SPECULAR )
        half2 lTempSpecularA;
        half2 lTempSpecularB;

        UnpackSpecular( lSpecular3, lTempSpecularA.x, lTempSpecularA.y );
        UnpackSpecular( lSpecular4, lTempSpecularB.x, lTempSpecularB.y );

        lTempSpecularA = half2( mix( lTempSpecularA, lTempSpecularB, lfHeight ) );

        lSpecular6 = PackSpecular( lTempSpecularA.x, lTempSpecularA.y );
        #else
        lSpecular6 = half2( mix( lSpecular3, lSpecular4, lfHeight ) );
        #endif
        
        lfHeightF = mix( lfHeightC, lfHeightD, lfSlope2 );
        lfDisplacementF = mix( lfDisplacementC, lfDisplacementD, lfHeight );
    }
    else
    {
#if defined ( D_PACKED_NORMALS )
        lNormal6 = uvec2(0, 0);
#else
        lNormal6 = half3(0.0, 0.0, 0.0);
#endif

        #if defined ( D_PACKED_SPECULAR )
        lSpecular6 = 0;
        #else
        lSpecular6 = half2( 0.0, 0.0 );
        #endif
        
        lfHeightF = lfDefaultHeight;
        lfDisplacementF = lfDefaultHeight;
    }

#if defined (D_STOCHASTIC_TERRAIN)
    float lfHeight = 0.0;
    {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )

        float lfPreScaledHeight = lfHeightE - lfHeightF * lfTileType - lfHeightE * lfTileType + lfTileType;
        lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfTileType ) * lfInverseOffset2 );
        lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
        lfHeight7 += lfPreScaledHeight * lfScale;
#else
        float lfPreScaledHeight = mix( lfHeightE, 1.0 - lfHeightF, lfTileType );
        lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfTileType );
        lfHeight7 += lfPreScaledHeight * lfScale;
#endif
    }
#else

    lfHeight7 = half( mix( lfHeightE, 1.0 - lfHeightF, lfTileType ) );
    float lfHeight = FastSmoothStep( lfHeight7 - lfOffset, 2.0f * lfOffset, lfTileType );
    lfHeight8 = half( mix( lfHeightE, lfHeightF, lfTileType ) );

#endif

    half2 lSpecular7 = half2( mix( lSpecular5, lSpecular6, lfHeight ) );
    half3 lNormal = half3( mix( lNormal5, lNormal6, lfHeight ) );

    //#if defined( D_NORMALBLEND )
    //    if ( IN_SCREEN_POSITION.x > 1024 )
    //    {
    //        lSpecular7.y = 0.0;
    //    }
    //#endif

#if !defined( D_STOCHASTIC_TERRAIN )
    lfDisplacement = mix( lfDisplacementE, lfDisplacementF, lfHeight );

    #if defined ( D_PACKED_SPECULAR )
    UnpackSpecular( lSpecular7, lfSpecular, lfSpecularCoeff );
    #else
    lfSpecular      = lSpecular7.x;
    lfSpecularCoeff = lSpecular7.y;
    #endif

#else
    lfDisplacement += mix( lfDisplacementE, lfDisplacementF, lfHeight ) * lfScale;
    
    #if defined ( D_PACKED_SPECULAR )
    float lfSpecularTemp;
    UnpackSpecular( lSpecular7, lfSpecularTemp, lfSpecularCoeff );
    lfSpecular      += lfSpecularTemp * lfScale;
    #else
    lfSpecular      += lSpecular7.x * lfScale;
    lfSpecularCoeff  = lSpecular7.y;
    #endif

#endif

#if defined( D_NO_Z_NORM )
    lNormal.z = sqrt( saturate( 1.0 - lNormal.x*lNormal.x - lNormal.y*lNormal.y ) );
#endif
    bn.mNormal = lNormal;
    return bn;

#else

#if defined (D_STOCHASTIC_TERRAIN)
    float lfHeight;
    float lfLocalDisplacement;

    #if defined ( D_PACKED_SPECULAR )
    uint lLocalSpecular;
    #else
    half2 lLocalSpecular;
    #endif

    half3 lNormal;
    GetTriPlanarNormalArray(lNormal,
        lCustomUniforms,
        lTileCoordsVec2,
        SAMPLER2DARRAYPARAM( lNormalMap ),
        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
        int( UnpackItemXChannel( lTile1UInt ) ),
        lfHeight,
        lfLocalDisplacement,      
        lLocalSpecular );    

    
    lfDisplacement  += lfLocalDisplacement * lfScale;

    #if defined ( D_PACKED_SPECULAR )
    float lfSpecularTemp;
    UnpackSpecular( lLocalSpecular, lfSpecularTemp, lfSpecularCoeff );
    lfSpecular      += lfSpecularTemp * lfScale;
    #else
    lfSpecular      += lLocalSpecular.x    * lfScale;
    lfSpecularCoeff  = lLocalSpecular.y;
    #endif
    
    lfHeight        *= lfScale;

    lfHeight1 += lfHeight;
    lfHeight2 += lfHeight;
    lfHeight3 += lfHeight;
    lfHeight4 += lfHeight;
    lfHeight5 += lfHeight;
    lfHeight6 += lfHeight;
    lfHeight7 += lfHeight;

#if defined( D_NO_Z_NORM )
    lNormal.z = sqrt( saturate( 1.0 - lNormal.x*lNormal.x - lNormal.y*lNormal.y ) );
#endif

    bn.mNormal = lNormal;
    return bn;
#else

    float lfHeight;
    half2 lSpecular;

    float lfDisplacementf = lfDisplacement;

    half3 lNormal;
    GetTriPlanarNormalArray(
        lNormal,
        lCustomUniforms,
        lTileCoordsVec2,
        SAMPLER2DARRAYPARAM( lNormalMap ),
        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
        int( UnpackItemXChannel( lTile1UInt ) ),
        lfHeight,
        lfDisplacementf,
        lSpecular );
    lfDisplacement = lfDisplacementf;

    lfSpecular = lSpecular.x;
    lfSpecularCoeff = lSpecular.y;
    
    lfHeight1 = lfHeight;
    lfHeight2 = lfHeight;
    lfHeight3 = lfHeight;
    lfHeight4 = lfHeight;
    lfHeight5 = lfHeight;
    lfHeight6 = lfHeight;
    lfHeight7 = lfHeight;
    lfHeight8 = lfHeight;

    vec3 lUnpackedNormal;
    lUnpackedNormal = vec3(lNormal);

    lUnpackedNormal.z = sqrt( saturate( 1.0 - lUnpackedNormal.x*lUnpackedNormal.x - lUnpackedNormal.y*lUnpackedNormal.y ) );
    bn.mNormal = half3(lUnpackedNormal);
    return bn;
#endif

#endif // LOW_QUALITY

#undef lfHeight1
#undef lfHeight2
#undef lfHeight3
#undef lfHeight4
#undef lfHeight5
#undef lfHeight6
#undef lfHeight7
#undef lfHeight8
#undef lfSpecular
#undef lfSpecularCoeff
#undef lfDisplacement



#undef lfPatch
#undef lfSlope2
#undef lfSlope1
#undef lfTileType

}
#endif

#define GetBlendedNormal( \
    lBlendedNormal, \
    lCustomUniforms, \
    lTileCoordsVec2, \
    lNormalMap, \
    lSubstanceNormalMap, \
    lTile1UInt, \
    lTile2UInt, \
    lhTileInfo, \
    lfSpecular, \
    lfSpecularCoeff, \
    lfDisplacement, \
    lfHeight1, \
    lfHeight2, \
    lfHeight3, \
    lfHeight4, \
    lfHeight5, \
    lfHeight6, \
    lfHeight7, \
    lfHeight8 \
) \
{ \
    float lfHeight; \
    half2 lSpecular; \
    float lfDisplacementf = lfDisplacement; \
 \
    half3 lNormal; \
    GetTriPlanarNormalArray( \
        lNormal, \
        lCustomUniforms, \
        lTileCoordsVec2, \
        EVAL( lNormalMap ), \
        EVAL( lSubstanceNormalMap ), \
        int( UnpackItemXChannel( lTile1UInt ) ), \
        lfHeight, \
        lfDisplacementf, \
        lSpecular ); \
    lfDisplacement = lfDisplacementf; \
 \
    lfSpecular = lSpecular.x; \
    lfSpecularCoeff = lSpecular.y; \
     \
    lfHeight1 = lfHeight; \
    lfHeight2 = lfHeight; \
    lfHeight3 = lfHeight; \
    lfHeight4 = lfHeight; \
    lfHeight5 = lfHeight; \
    lfHeight6 = lfHeight; \
    lfHeight7 = lfHeight; \
    lfHeight8 = lfHeight; \
 \
    vec3 lUnpackedNormal; \
    lUnpackedNormal = vec3(lNormal); \
    lUnpackedNormal.z = sqrt( saturate( 1.0 - lUnpackedNormal.x*lUnpackedNormal.x - lUnpackedNormal.y*lUnpackedNormal.y ) ); \
    lBlendedNormal = half3(lUnpackedNormal); \
} while (0)

#if 0
vec3
GetBlendedColour2(
    in  CustomPerMaterialUniforms lCustomUniforms,
    in    vec4         lTileCoordsVec22,
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    in    uint2         lTile1UInt2,
    in    uint2         lTile2UInt2,
    in    Float8       lfHeights,
    out   float        lfMetallic )
{

#define lfHeight1 (lfHeights.m[0][0])
#define lfHeight2 (lfHeights.m[0][1])
#define lfHeight3 (lfHeights.m[0][2])
#define lfHeight4 (lfHeights.m[0][3])
#define lfHeight5 (lfHeights.m[1][0])
#define lfHeight6 (lfHeights.m[1][1])
#define lfHeight7 (lfHeights.m[1][2])

    lfMetallic = 0.0;
#ifndef D_LOW_QUALITY
    const float lfThreshold = 1.0 / 20.0;
    vec3  lColour5 = vec3( 0.0, 0.0, 0.0 );
    vec3  lColour6 = vec3( 0.0, 0.0, 0.0 );
    float lfMetallic5 = 0.0;
    float lfMetallic6 = 0.0;

    if ( lfHeight7 < 1.0 - lfThreshold )
    {
        {
            if ( ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemXChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) *  ( liIndex >= 20 ? 1.0 : 0.0 );
                vec3 Colour;
                GetTriPlanarColourArray(Colour, lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 );
                lColour5 += ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) * Colour;
            }
            if ( lfHeight1 * ( 1.0 - lfHeight5 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemYChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += lfHeight1 * ( 1.0 - lfHeight5 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += lfHeight1 * ( 1.0 - lfHeight5 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
        {
            if ( ( 1.0 - lfHeight2 ) * lfHeight5 > lfThreshold )
            {
                int liIndex = int( UnpackItemZChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += ( 1.0 - lfHeight2 ) * lfHeight5  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += ( 1.0 - lfHeight2 ) * lfHeight5 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight2 * lfHeight5 > lfThreshold )
            {
                int liIndex = int( UnpackItemWChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += lfHeight2 * lfHeight5  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += lfHeight2 * lfHeight5 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
    }

    if ( lfHeight7 > lfThreshold )

    {
        {
            if ( ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemXChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight3 * ( 1.0 - lfHeight6 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemYChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += lfHeight3 * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += lfHeight3 * ( 1.0 - lfHeight6 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
        {
            if ( ( 1.0 - lfHeight4 ) * lfHeight6 > lfThreshold )
            {
                int liIndex = int( UnpackItemZChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += ( 1.0 - lfHeight4 ) * lfHeight6  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += ( 1.0 - lfHeight4 ) * lfHeight6 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
    }

    lfMetallic = mix( lfMetallic5, lfMetallic6, lfHeight7 );
    return       mix( lColour5, lColour6, lfHeight7 );
#else
    int liIndex = int( UnpackItemXChannel( lTile1UInt ) );
    
    vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
    vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
    vec3 Colour;
    GetTriPlanarColourArray(
        Colour,
        lTileCoordsVec2,
        SAMPLER2DARRAYPARAM( lDiffuseMap ),
        SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
        liIndex,
        lOriginalColourVec3,
        lRecolourVec3 );
    return Colour;
#endif

#undef lfHeight1
#undef lfHeight2
#undef lfHeight3
#undef lfHeight4
#undef lfHeight5
#undef lfHeight6
#undef lfHeight7

}
#endif

// METAL_FUNC
vec3
GetBlendedColourOldThingTemp(
    in  CustomPerMaterialUniforms lCustomUniforms,
    in    vec2         lTileCoordsVec2,
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    in    uint         lTile1UInt,
    in    uint         lTile2UInt,
    in    Float8       lfHeights,
    out   float        lfMetallic )
{

#define lfHeight1 (lfHeights.m[0][0])
#define lfHeight2 (lfHeights.m[0][1])
#define lfHeight3 (lfHeights.m[0][2])
#define lfHeight4 (lfHeights.m[0][3])
#define lfHeight5 (lfHeights.m[1][0])
#define lfHeight6 (lfHeights.m[1][1])
#define lfHeight7 (lfHeights.m[1][2])

    lfMetallic = 0.0;
#ifndef D_LOW_QUALITY
    const float lfThreshold = 1.0 / 20.0;
    vec3  lColour5 = vec3( 0.0, 0.0, 0.0 );
    vec3  lColour6 = vec3( 0.0, 0.0, 0.0 );
    float lfMetallic5 = 0.0;
    float lfMetallic6 = 0.0;

    if ( lfHeight7 < 1.0 - lfThreshold )
    {
        {
            if ( ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemXChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight1 * ( 1.0 - lfHeight5 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemYChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += lfHeight1 * ( 1.0 - lfHeight5 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += lfHeight1 * ( 1.0 - lfHeight5 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
        {
            if ( ( 1.0 - lfHeight2 ) * lfHeight5 > lfThreshold )
            {
                int liIndex = int( UnpackItemZChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += ( 1.0 - lfHeight2 ) * lfHeight5  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += ( 1.0 - lfHeight2 ) * lfHeight5 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight2 * lfHeight5 > lfThreshold )
            {
                int liIndex = int( UnpackItemWChannel( lTile1UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic5 += lfHeight2 * lfHeight5  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour5 += lfHeight2 * lfHeight5 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
    }

    if ( lfHeight7 > lfThreshold )
    {
        {
            if ( ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemXChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight3 * ( 1.0 - lfHeight6 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemYChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += lfHeight3 * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += lfHeight3 * ( 1.0 - lfHeight6 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
        {
            if ( ( 1.0 - lfHeight4 ) * lfHeight6 > lfThreshold )
            {
                int liIndex = int( UnpackItemZChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += ( 1.0 - lfHeight4 ) * lfHeight6  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += ( 1.0 - lfHeight4 ) * lfHeight6 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
    }

    lfMetallic = mix( lfMetallic5, lfMetallic6, lfHeight7 );
    return       mix( lColour5, lColour6, lfHeight7 );
#else
    int liIndex = int( UnpackItemXChannel( lTile1UInt ) );
    
    vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
    vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
    vec3 Colour;
    GetTriPlanarColourArray(
        Colour,
        lTileCoordsVec2,
        SAMPLER2DARRAYPARAM( lDiffuseMap ),
        SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
        liIndex,
        lOriginalColourVec3,
        lRecolourVec3 );
    return Colour;
#endif

#undef lfHeight1
#undef lfHeight2
#undef lfHeight3
#undef lfHeight4
#undef lfHeight5
#undef lfHeight6
#undef lfHeight7

}

#define _FastSmoothStep( \
    lResult, \
    lfStart, \
    lfWidth, \
    lfPosition) \
{ \
    float t = saturate((lfPosition - lfStart) / lfWidth); \
    lResult = (3.0f - 2.0f*t) * t*t; \
} while(0)

#if 1
// working low-quality version
#define GetBlendedColour( \
    lColour, \
    lCustomUniforms, \
    lTileCoordsVec2, \
    lDiffuseMap, \
    lSubstanceDiffuseMap, \
    lTile1UInt, \
    lTile2UInt, \
    lfHeights, \
    lfMetallic ) \
{ \
    int liIndex = int( UnpackItemXChannel( lTile1UInt ) ); \
    vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
    vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
    GetTriPlanarColourArray( \
        lColour, \
        lTileCoordsVec2, \
        EVAL(lDiffuseMap), \
        EVAL(lSubstanceDiffuseMap), \
        liIndex, \
        lOriginalColourVec3, \
        lRecolourVec3 ); \
} \
while(0)
#endif

#if 0
#define GetBlendedColour( \
    lColour, \
    lCustomUniforms, \
    lTileCoordsVec2, \
    lDiffuseMap, \
    lSubstanceDiffuseMap, \
    lTile1UInt, \
    lTile2UInt, \
    lfHeights, \
    lfMetallic ) \
{ \
    lfMetallic = 0.0; \
    const float lfThreshold = 1.0 / 20.0; \
    vec3  lColour5 = vec3( 0.0, 0.0, 0.0 ); \
    vec3  lColour6 = vec3( 0.0, 0.0, 0.0 ); \
    float lfMetallic5 = 0.0; \
    float lfMetallic6 = 0.0; \
    const half lfHeight1  = lfHeights[0][0]; \
    const half lfHeight2  = lfHeights[0][1]; \
    const half lfHeight3  = lfHeights[0][2]; \
    const half lfHeight4  = lfHeights[0][3]; \
    const half lfHeight5  = lfHeights[1][0]; \
    const half lfHeight6  = lfHeights[1][1]; \
    const half lfHeight7  = lfHeights[1][2]; \
    if ( lfHeight7 < 1.0 - lfThreshold ) \
    { \
        if ( ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) > lfThreshold ) \
        { \
            int liIndex = int( UnpackItemXChannel( lTile1UInt ) ); \
            vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
            vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
            lfMetallic5 += ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
            vec3 lArrayColour; \
            GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
            lColour5 += ( 1.0 - lfHeight1 ) * ( 1.0 - lfHeight5 ) * lArrayColour; \
        } \
        if ( lfHeight1 * ( 1.0 - lfHeight5 ) > lfThreshold ) \
        { \
            int liIndex = int( UnpackItemYChannel( lTile1UInt ) ); \
            vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
            vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
            lfMetallic5 += lfHeight1 * ( 1.0 - lfHeight5 )  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
            vec3 lArrayColour; \
            GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
            lColour5 += lfHeight1 * ( 1.0 - lfHeight5 ) * lArrayColour; \
        } \
        if ( ( 1.0 - lfHeight2 ) * lfHeight5 > lfThreshold ) \
        { \
            int liIndex = int( UnpackItemZChannel( lTile1UInt ) ); \
            vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
            vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
            lfMetallic5 += ( 1.0 - lfHeight2 ) * lfHeight5  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
            vec3 lArrayColour; \
            GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
            lColour5 += ( 1.0 - lfHeight2 ) * lfHeight5 * lArrayColour; \
        } \
        if ( lfHeight2 * lfHeight5 > lfThreshold ) \
        { \
            int liIndex = int( UnpackItemWChannel( lTile1UInt ) ); \
            vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
            vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
            lfMetallic5 += lfHeight2 * lfHeight5  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
            vec3 lArrayColour; \
            GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
            lColour5 += lfHeight2 * lfHeight5 * lArrayColour; \
        } \
    } \
    if ( lfHeight7 > lfThreshold ) \
    { \
        if ( ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) > lfThreshold ) \
        { \
            int liIndex = int( UnpackItemXChannel( lTile2UInt ) ); \
            vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
            vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
            lfMetallic6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
            vec3 lArrayColour; \
            GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
            lColour6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) * lArrayColour; \
        } \
        if ( lfHeight3 * ( 1.0 - lfHeight6 ) > lfThreshold ) \
        { \
            int liIndex = int( UnpackItemYChannel( lTile2UInt ) ); \
            vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
            vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
            lfMetallic6 += lfHeight3 * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
            vec3 lArrayColour; \
            GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
            lColour6 += lfHeight3 * ( 1.0 - lfHeight6 ) * lArrayColour; \
        } \
    } \
    lfMetallic = mix( lfMetallic5, lfMetallic6, float(lfHeight7) ); \
    lColour   =  mix( lColour5, lColour6, vec3(lfHeight7, lfHeight7, lfHeight7) ); \
} \
while(0)
#endif


    // if ( lfHeight7 > lfThreshold ) \
    // { \
    //     if ( ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) > lfThreshold ) \
    //     { \
    //         int liIndex = int( UnpackItemXChannel( lTile2UInt ) ); \
    //         vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
    //         vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
    //         lfMetallic6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
    //         vec3 lArrayColour; \
    //         GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
    //         lColour6 += ( 1.0 - lfHeight3 ) * ( 1.0 - lfHeight6 ) * lArrayColour; \
    //     } \
    //     if ( lfHeight3 * ( 1.0 - lfHeight6 ) > lfThreshold ) \
    //     { \
    //         int liIndex = int( UnpackItemYChannel( lTile2UInt ) ); \
    //         vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
    //         vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
    //         lfMetallic6 += lfHeight3 * ( 1.0 - lfHeight6 )  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
    //         vec3 lArrayColour; \
    //         GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
    //         lColour6 += lfHeight3 * ( 1.0 - lfHeight6 ) * lArrayColour; \
    //     } \
    //     if ( ( 1.0 - lfHeight4 ) * lfHeight6 > lfThreshold ) \
    //     { \
    //         int liIndex = int( UnpackItemZChannel( lTile2UInt ) ); \
    //         vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb; \
    //         vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb; \
    //         lfMetallic6 += ( 1.0 - lfHeight4 ) * lfHeight6  *  ( liIndex >= 20 ? 1.0 : 0.0 ); \
    //         vec3 lArrayColour; \
    //         GetTriPlanarColourArray( lArrayColour, lTileCoordsVec2, EVAL(lDiffuseMap), EVAL(lSubstanceDiffuseMap), liIndex, lOriginalColourVec3, lRecolourVec3 ); \
    //         lColour6 += ( 1.0 - lfHeight4 ) * lfHeight6 * lArrayColour; \
    //     } \
    // } \

vec3
GetTileColourAndNormal2(
    in  CustomPerMaterialUniforms lCustomUniforms,
    in  vec3           lFaceNormalVec3,
    in  vec3           lPlanetOffsetVec3,
    in  vec3           lLocalPositionVec3,
    in  uvec4          lTileTextureIndicesVec4,
    in  float          lfPatch,
    in  float          lfSlope1,
    in  float          lfSlope2,
    in  float          lfTileType,
    out vec3           lOutWorldNormalVec3,
    in  float          lfSmallScale,
    in  float          lfLargeScale,
    in  float          lfFade,
    in  float          lfWaterFade,
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    SAMPLER2DARG(      lNoiseNormalMap ),
    out float          lfSpecular,
    out float          lfMetallic,
    out float          lfHeight )
{

#if defined( D_TERRAIN_X_FACING )
    vec3    lWeightsVec3 = vec3( 1.0, 0.0, 0.0 );    
#elif defined( D_TERRAIN_Y_FACING )
    vec3    lWeightsVec3 = vec3( 0.0, 1.0, 0.0 );    
#elif defined( D_TERRAIN_Z_FACING )
    vec3    lWeightsVec3 = vec3( 0.0, 0.0, 1.0 );    
#else
    vec3    lWeightsVec3 = pow( abs( lFaceNormalVec3 ), vec3( kfBlendPower, kfBlendPower, kfBlendPower ) );    
#endif

    vec3    lTexCoordsVec3;
    vec3    lSmallTexCoordsVec3;
    vec3    lLargeTexCoordsVec3;
    vec3    lSmallNoiseTexCoordsVec3;
    vec3    lLargeNoiseTexCoordsVec3;
    float   lfWeightRecip = 1.0 / ( lWeightsVec3.x + lWeightsVec3.y + lWeightsVec3.z );
    vec3    lNormalVec3 = lFaceNormalVec3;

    lfMetallic = 0.0;
    lfHeight = 0.0;

    lSmallTexCoordsVec3  = mod( lLocalPositionVec3 * lfSmallScale, kfWrapScale / kfNoiseTextScale );
    lSmallTexCoordsVec3 += mod( lPlanetOffsetVec3  * lfSmallScale, kfWrapScale / kfNoiseTextScale );
    lLargeTexCoordsVec3  = mod( lLocalPositionVec3 * lfLargeScale, kfWrapScale / kfNoiseTextScale );
    lLargeTexCoordsVec3 += mod( lPlanetOffsetVec3  * lfLargeScale, kfWrapScale / kfNoiseTextScale );

    lSmallTexCoordsVec3  = mod( lSmallTexCoordsVec3, kfWrapScale / kfNoiseTextScale );
    lLargeTexCoordsVec3  = mod( lLargeTexCoordsVec3, kfWrapScale / kfNoiseTextScale );

    lSmallNoiseTexCoordsVec3 = lSmallTexCoordsVec3 * kfNoiseTextScale;
    lLargeNoiseTexCoordsVec3 = lLargeTexCoordsVec3 * kfNoiseTextScale;

    vec3 lWeightsVec3N = lWeightsVec3 * lfWeightRecip;

    // Normals + (Spec and Height)
    Float8 lafHeights;
    float lfDisplacement = 0.0;
#if 0
    {
        half3  lNormalSmallVec3;
        half3  lNormalLargeVec3;
        half3  lSmallMappedNormalVec3 = half3( 0.0, 0.0, 0.0 );
        half3  lLargeMappedNormalVec3 = half3( 0.0, 0.0, 0.0 );
        half3  lSpecularLargeVec3     = half3(0.0, 0.0, 0.0);
        half3  lSpecularSmallVec3     = half3(0.0, 0.0, 0.0);

        vec3   lDisplacementSmallVec3 = vec3( 0.0, 0.0, 0.0 );
        vec3   lDisplacementLargeVec3 = vec3( 0.0, 0.0, 0.0 );
        float  lfFadeThreshold = 1.0 / 20.0;
        half   lfSpecularCoeff = 0.0;
        half   lfHeightNoise;
        half   lfSpecularNoise;

        Half8 lHeightsLargeX = {0.5, 0.5};
        Half8 lHeightsLargeY = {0.5, 0.5};
        Half8 lHeightsLargeZ = {0.5, 0.5};

        Half8 lHeightsSmallX = {0.5, 0.5};
        Half8 lHeightsSmallY = {0.5, 0.5};
        Half8 lHeightsSmallZ = {0.5, 0.5};

        const half4 lhTileInfo = half4(lfPatch, lfSlope1, lfSlope2, lfTileType);

        #if 0
        if ( lfFade > lfFadeThreshold )
        {
            {
                BlendedNormal bn = GetBlendedNormal(
                    lCustomUniforms,
                    lLargeTexCoordsVec3.yz,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.z,
                    lTileTextureIndicesVec4.w,
                    lhTileInfo
                );

                lNormalLargeVec3 = bn.mNormal;
                lHeightsLargeX = bn.mHeights;
                lSpecularLargeVec3.x = bn.mSpecular;
                lfSpecularCoeff = bn.mSpecularCoeff;
                lDisplacementLargeVec3.x = bn.mDisplacement;

                half3 lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lLargeNoiseTexCoordsVec3.yz,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );

                half3 lPlaneNorm                = half3( lNormalVec3.y,lNormalVec3.z, abs( lNormalVec3.x ) );
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 );
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale );
                lNormalLargeVec3.z       *= sign( lNormalVec3.x );
                lNormalLargeVec3          = lNormalLargeVec3.zxy;
                lLargeMappedNormalVec3   += half( lWeightsVec3N.x ) * lNormalLargeVec3;
                lSpecularLargeVec3.x      = saturate( lSpecularLargeVec3.x     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementLargeVec3.x  = saturate( lDisplacementLargeVec3.x + lfHeightNoise );

            }
            
            #if 0
            if ( lWeightsVec3N.y > kfWeightFloor )
            {
                BlendedNormal bn = GetBlendedNormal(
                    lCustomUniforms,
                    lLargeTexCoordsVec3.zx,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.z,
                    lTileTextureIndicesVec4.w,
                    lhTileInfo
                );
                lNormalLargeVec3 = bn.mNormal;
                lHeightsLargeY = bn.mHeights;
                lSpecularLargeVec3.y = bn.mSpecular;
                lfSpecularCoeff = bn.mSpecularCoeff;
                lDisplacementLargeVec3.y = bn.mDisplacement;

                half3 lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lLargeNoiseTexCoordsVec3.zx,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );

                half3 lPlaneNorm                = half3( half2(lNormalVec3.zx), abs( lNormalVec3.y ) );
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 );
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale );
                lNormalLargeVec3.z       *= sign( lNormalVec3.y );
                lNormalLargeVec3          = lNormalLargeVec3.yzx;
                lLargeMappedNormalVec3   += half( lWeightsVec3N.y ) * lNormalLargeVec3;
                lSpecularLargeVec3.y      = saturate( lSpecularLargeVec3.y     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementLargeVec3.y  = saturate( lDisplacementLargeVec3.y + lfHeightNoise );
            }
            if ( lWeightsVec3N.z > kfWeightFloor )
            {
                BlendedNormal bn = GetBlendedNormal(
                    lCustomUniforms,
                    lLargeTexCoordsVec3.xy,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.z,
                    lTileTextureIndicesVec4.w,
                    lhTileInfo
                );
                lNormalLargeVec3 = bn.mNormal;
                lHeightsLargeZ = bn.mHeights;
                lSpecularLargeVec3.z = bn.mSpecular;
                lfSpecularCoeff = bn.mSpecularCoeff;
                lDisplacementLargeVec3.z = bn.mDisplacement;

                half3 lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lLargeNoiseTexCoordsVec3.xy,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );

                half3 lPlaneNorm                = half3( half2(lNormalVec3.xy), abs( lNormalVec3.z ) );
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 );
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale );
                lNormalLargeVec3.z       *= sign( lNormalVec3.z );
                lLargeMappedNormalVec3   += half( lWeightsVec3N.z ) * lNormalLargeVec3;
                lSpecularLargeVec3.z      = saturate( lSpecularLargeVec3.z     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementLargeVec3.z  = saturate( lDisplacementLargeVec3.z + lfHeightNoise );
            }
            #endif
        }
        else
        {
            lfFade = 0.0;
        }

        if ( lfFade < 1.0 - lfFadeThreshold )
        {
            if ( lWeightsVec3N.x > kfWeightFloor )
            {
                half lSpecX = lSpecularSmallVec3.x;
                half lDisplX = lDisplacementSmallVec3.x;
                BlendedNormal bn = GetBlendedNormal(
                    lCustomUniforms,
                    lSmallTexCoordsVec3.yz,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.x,
                    lTileTextureIndicesVec4.y,
                    lhTileInfo
                );

                lNormalSmallVec3 = bn.mNormal;
                lHeightsSmallX = bn.mHeights;
                lSpecularSmallVec3.x = bn.mSpecular;
                lfSpecularCoeff = bn.mSpecularCoeff;
                lDisplacementSmallVec3.x = bn.mDisplacement;

                half3 lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lSmallNoiseTexCoordsVec3.yz,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );

                half3 lPlaneNorm                = half3( half2(lNormalVec3.yz), abs( lNormalVec3.x ) );
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 );
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale );
                lNormalSmallVec3.z       *= sign( lNormalVec3.x );
                lNormalSmallVec3          = lNormalSmallVec3.zxy;
                lSmallMappedNormalVec3   += half( lWeightsVec3N.x ) * lNormalSmallVec3;
                lSpecularSmallVec3.x      = saturate( lSpecularSmallVec3.x     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementSmallVec3.x  = saturate( lDisplacementSmallVec3.x + lfHeightNoise );
            }
            if ( lWeightsVec3N.y > kfWeightFloor )
            {
                BlendedNormal bn = GetBlendedNormal(
                    lCustomUniforms,
                    lSmallTexCoordsVec3.zx,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.x,
                    lTileTextureIndicesVec4.y,
                    lhTileInfo
                );

                lNormalSmallVec3 = bn.mNormal;
                lHeightsSmallY = bn.mHeights;
                lSpecularSmallVec3.y = bn.mSpecular;
                lfSpecularCoeff = bn.mSpecularCoeff;
                lDisplacementSmallVec3.y = bn.mDisplacement;

                half3 lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lSmallNoiseTexCoordsVec3.zx,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );

                half3 lPlaneNorm                = half3( half2(lNormalVec3.zx), abs( lNormalVec3.y ) );
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 );
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale );
                lNormalSmallVec3.z       *= sign( lNormalVec3.y );
                lNormalSmallVec3          = lNormalSmallVec3.yzx;
                lSmallMappedNormalVec3   += half( lWeightsVec3N.y ) * lNormalSmallVec3;
                lSpecularSmallVec3.y      = saturate( lSpecularSmallVec3.y     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementSmallVec3.y  = saturate( lDisplacementSmallVec3.y + lfHeightNoise );
            }
            if ( lWeightsVec3N.z > kfWeightFloor )
            {
                BlendedNormal bn = GetBlendedNormal(
                    lCustomUniforms,
                    lSmallTexCoordsVec3.xy,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.x,
                    lTileTextureIndicesVec4.y,
                    lhTileInfo
                );

                lNormalSmallVec3 = bn.mNormal;
                lHeightsSmallZ = bn.mHeights;
                lSpecularSmallVec3.z = bn.mSpecular;
                lfSpecularCoeff = bn.mSpecularCoeff;
                lDisplacementSmallVec3.z = bn.mDisplacement;

                half3 lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lSmallNoiseTexCoordsVec3.xy,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );

                half3 lPlaneNorm                = half3( half2(lNormalVec3.xy), abs( lNormalVec3.z ) );
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 );
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale );
                lNormalSmallVec3.z       *= sign( lNormalVec3.z );
                lSmallMappedNormalVec3   += half( lWeightsVec3N.z ) * lNormalSmallVec3;
                lSpecularSmallVec3.z      = saturate( lSpecularSmallVec3.z     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementSmallVec3.z  = saturate( lDisplacementSmallVec3.z + lfHeightNoise );
            }
        }
        else
        {
            lfFade = 1.0;
        }
        #endif

        #if 0
        {

            for (int i = 0; i < 8; i++)
            {
                float lfHeightLarge = dot(vec3(lHeightsLargeX.m[i/4][i%4], lHeightsLargeY.m[i/4][i%4], lHeightsLargeZ.m[i/4][i%4]), lWeightsVec3) * lfWeightRecip;
                float lfHeightSmall = dot(vec3(lHeightsSmallX.m[i/4][i%4], lHeightsSmallY.m[i/4][i%4], lHeightsSmallZ.m[i/4][i%4]), lWeightsVec3) * lfWeightRecip;
                lafHeights.m[i/4][i%4] = mix(lfHeightSmall, lfHeightLarge, lfFade);
            }

            float lfDisplacementLarge = dot( lDisplacementLargeVec3, lWeightsVec3 ) * lfWeightRecip;
            float lfDisplacementSmall = dot( lDisplacementSmallVec3, lWeightsVec3 ) * lfWeightRecip;
            lfDisplacement = mix( lfDisplacementSmall, lfDisplacementLarge, lfFade );

            lfHeight = lfDisplacement;

            float lfWidth = 0.05;

            lafHeights.m[0][0] = FastSmoothStep( lafHeights.m[0][0] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights.m[0][1] = FastSmoothStep( lafHeights.m[0][1] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights.m[0][2] = FastSmoothStep( lafHeights.m[0][2] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights.m[0][3] = FastSmoothStep( lafHeights.m[0][3] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights.m[1][0] = FastSmoothStep( lafHeights.m[1][0] - lfWidth, 2.0f * lfWidth, lfSlope1 );
            lafHeights.m[1][1] = FastSmoothStep( lafHeights.m[1][1] - lfWidth, 2.0f * lfWidth, lfSlope2 );
            lafHeights.m[1][2] = FastSmoothStep( lafHeights.m[1][2] - lfWidth, 2.0f * lfWidth, lfTileType );
            lafHeights.m[1][3] = FastSmoothStep( lafHeights.m[1][3] - lfWidth, 2.0f * lfWidth, lfFade );

            float lfSpecularLarge = dot( vec3(lSpecularLargeVec3), lWeightsVec3 ) * lfWeightRecip;
            float lfSpecularSmall = dot( vec3(lSpecularSmallVec3), lWeightsVec3 ) * lfWeightRecip;

#if defined( D_CACHE_NORMAL )
            lSmallMappedNormalVec3 = ( lfFade > lfFadeThreshold ) ? lLargeMappedNormalVec3 : lSmallMappedNormalVec3;
#else

            // removing the height blend.. it doesn't work with caching
            lLargeMappedNormalVec3 *= half( lfFade ); //lafHeights[ 7 ];
            lSmallMappedNormalVec3 *= half( 1.0 - lfFade ); //lafHeights[ 7 ];
            lSmallMappedNormalVec3  = half3 (lSmallMappedNormalVec3.x + lLargeMappedNormalVec3.x, lSmallMappedNormalVec3.y + lLargeMappedNormalVec3.y, lSmallMappedNormalVec3.z + lLargeMappedNormalVec3.z);            
#endif            
			lOutWorldNormalVec3     = normalize( mix( lFaceNormalVec3, vec3(lSmallMappedNormalVec3), kfNormBlend * lfWaterFade ) );
            lfSpecular              = mix( lfSpecularSmall, lfSpecularLarge, lfFade );
        }
        #else
        lOutWorldNormalVec3     = lFaceNormalVec3;
        lfSpecular              = 0.5f;
        #endif
    }
    #endif
    lOutWorldNormalVec3     = lFaceNormalVec3;
    lfSpecular              = 0.5f;

    float lfFinalFade = lfFade;

    // World space position is lTilePositionVec3

    vec3    lTileColourSmallVec3 = vec3( 0.0, 0.0, 0.0 );
    vec3    lTileColourLargeVec3 = vec3( 0.0, 0.0, 0.0 );
    {
        // if ( lWeightsVec3N.x > kfWeightFloor )
        {
            vec3 Color0;
            GetBlendedColour(
                Color0,
                lCustomUniforms,
                lLargeTexCoordsVec3.zy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lafHeights,
                lfMetallic);
            lTileColourLargeVec3 += lWeightsVec3N.x * Color0;
            #if 1
            
            vec3 Color2;
            GetBlendedColour(
                Color2,
                lCustomUniforms,
                lLargeTexCoordsVec3.zx,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lafHeights,
                lfMetallic );
            lTileColourLargeVec3 += lWeightsVec3N.y * Color2;
            
            vec3 Color4;
            GetBlendedColour(
                Color4,
                lCustomUniforms,
                lLargeTexCoordsVec3.xy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lafHeights,
                lfMetallic );
            lTileColourLargeVec3 += lWeightsVec3N.z * Color4;

            vec3 Color1;
            GetBlendedColour(
                Color1,
                lCustomUniforms,
                lSmallTexCoordsVec3.zy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lafHeights,
                lfMetallic );
            lTileColourSmallVec3 += lWeightsVec3N.x * Color1;

            vec3 Color3;
            GetBlendedColour(
                Color3,
                lCustomUniforms,
                lSmallTexCoordsVec3.zx,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lafHeights,
                lfMetallic );
            lTileColourSmallVec3 += lWeightsVec3N.y * Color3;

            vec3 Color5;
            GetBlendedColour(
                Color5,
                lCustomUniforms,
                lSmallTexCoordsVec3.xy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lafHeights,
                lfMetallic );
            lTileColourSmallVec3 += lWeightsVec3N.z * Color5;
            #endif
        }
        // #endif
    }

    vec3 lTileColourVec3;
    lTileColourVec3 = mix( lTileColourSmallVec3, lTileColourLargeVec3, lfFade );
    lTileColourVec3 = GammaCorrectInput01( lTileColourVec3 );

    return lTileColourVec3;
}

#define GetTileColourAndNormal( \
    lTileColourVec3, \
    lCustomUniforms, \
    lFaceNormalVec3,  \
    lPlanetOffsetVec3,  \
    lLocalPositionVec3,  \
    lTileTextureIndicesVec4, \
    lfPatch, \
    lfSlope1, \
    lfSlope2, \
    lfTileType, \
    lOutWorldNormalVec3, \
    lfSmallScale, \
    lfLargeScale, \
    lfFade, \
    lfWaterFade, \
    lDiffuseMap, \
    lNormalMap, \
    lSubstanceDiffuseMap, \
    lSubstanceNormalMap, \
    lNoiseNormalMap, \
    lfSpecular, \
    lfMetallic, \
    lfHeight ) \
{ \
    vec3    lWeightsVec3 = pow( abs( lFaceNormalVec3 ), vec3( kfBlendPower, kfBlendPower, kfBlendPower ) ); \
 \
    vec3    lTexCoordsVec3; \
    vec3    lSmallTexCoordsVec3; \
    vec3    lLargeTexCoordsVec3; \
    const float   lfWeightRecip = 1.0 / ( lWeightsVec3.x + lWeightsVec3.y + lWeightsVec3.z ); \
    vec3    _lNormalVec3 = lFaceNormalVec3; \
    lfMetallic = 0.0; \
    lfSpecular = 0.5; \
    lfHeight = 0.0; \
    lOutWorldNormalVec3 = lFaceNormalVec3; \
 \
    lSmallTexCoordsVec3  = mod( lLocalPositionVec3 * lfSmallScale, kfWrapScale / kfNoiseTextScale ); \
    lSmallTexCoordsVec3 += mod( lPlanetOffsetVec3  * lfSmallScale, kfWrapScale / kfNoiseTextScale ); \
    lLargeTexCoordsVec3  = mod( lLocalPositionVec3 * lfLargeScale, kfWrapScale / kfNoiseTextScale ); \
    lLargeTexCoordsVec3 += mod( lPlanetOffsetVec3  * lfLargeScale, kfWrapScale / kfNoiseTextScale ); \
 \
    lSmallTexCoordsVec3  = mod( lSmallTexCoordsVec3, kfWrapScale / kfNoiseTextScale ); \
    lLargeTexCoordsVec3  = mod( lLargeTexCoordsVec3, kfWrapScale / kfNoiseTextScale ); \
 \
    const vec3 lSmallNoiseTexCoordsVec3 = lSmallTexCoordsVec3 * kfNoiseTextScale; \
    const vec3 lLargeNoiseTexCoordsVec3 = lLargeTexCoordsVec3 * kfNoiseTextScale; \
 \
    const vec3 lWeightsVec3N = lWeightsVec3 * lfWeightRecip; \
    float lfDisplacement = 0.0; \
    Float8 lafHeights; \
    { \
        half3  lNormalSmallVec3; \
        half3  lNormalLargeVec3; \
        half3  lSmallMappedNormalVec3 = half3( 0.0, 0.0, 0.0 ); \
        half3  lLargeMappedNormalVec3 = half3( 0.0, 0.0, 0.0 ); \
        half3  lSpecularLargeVec3     = half3(0.0, 0.0, 0.0); \
        half3  lSpecularSmallVec3     = half3(0.0, 0.0, 0.0); \
 \
        vec3   lDisplacementSmallVec3 = vec3( 0.0, 0.0, 0.0 ); \
        vec3   lDisplacementLargeVec3 = vec3( 0.0, 0.0, 0.0 ); \
        float  lfFadeThreshold = 1.0 / 20.0; \
        half   lfSpecularCoeff = 0.0; \
        half   lfHeightNoise; \
        half   lfSpecularNoise; \
 \
        Half8 lHeightsLargeX = {0.5, 0.5}; \
        Half8 lHeightsLargeY = {0.5, 0.5}; \
        Half8 lHeightsLargeZ = {0.5, 0.5}; \
 \
        Half8 lHeightsSmallX = {0.5, 0.5}; \
        Half8 lHeightsSmallY = {0.5, 0.5}; \
        Half8 lHeightsSmallZ = {0.5, 0.5}; \
        const half4 lhTileInfo = half4(lfPatch, lfSlope1, lfSlope2, lfTileType); \
        if ( lfFade > lfFadeThreshold ) \
        { \
            if ( lWeightsVec3N.x > kfWeightFloor ) \
            { \
                GetBlendedNormal( \
                    lNormalLargeVec3, \
                    lCustomUniforms, \
                    lLargeTexCoordsVec3.yz, \
                    SAMPLER2DARRAYPARAM( lNormalMap ), \
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ), \
                    lTileTextureIndicesVec4.z, \
                    lTileTextureIndicesVec4.w, \
                    lhTileInfo, \
                    lSpecularLargeVec3.x, \
                    lfSpecularCoeff, \
                    lDisplacementLargeVec3.x, \
                    lHeightsLargeX[0][0], lHeightsLargeX[0][1], lHeightsLargeX[0][2], lHeightsLargeX[0][3], \
                    lHeightsLargeX[1][0], lHeightsLargeX[1][1], lHeightsLargeX[1][2], lHeightsLargeX[1][3] \
                ); \
                half3 lNoiseNormalVec3; \
                GetTriPlanarNoiseNormal( \
                    lNoiseNormalVec3, \
                    lLargeNoiseTexCoordsVec3.yz, \
                    SAMPLER2DPARAM( lNoiseNormalMap ), \
                    lfHeightNoise,  \
                    lfSpecularNoise ); \
                half3 lPlaneNorm          = half3( lNormalVec3.y,lNormalVec3.z, abs( lNormalVec3.x ) ); \
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 ); \
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale ); \
                lNormalLargeVec3.z       *= sign( lNormalVec3.x ); \
                lNormalLargeVec3          = lNormalLargeVec3.zxy; \
                lLargeMappedNormalVec3   += half( lWeightsVec3N.x ) * lNormalLargeVec3; \
                lSpecularLargeVec3.x      = saturate( lSpecularLargeVec3.x     - lfSpecularNoise * half( lfSpecularCoeff ) ); \
                lDisplacementLargeVec3.x  = saturate( lDisplacementLargeVec3.x + lfHeightNoise ); \
            } \
            if ( lWeightsVec3N.y > kfWeightFloor ) \
            { \
                GetBlendedNormal( \
                    lNormalLargeVec3, \
                    lCustomUniforms, \
                    lLargeTexCoordsVec3.zx, \
                    SAMPLER2DARRAYPARAM( lNormalMap ), \
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ), \
                    lTileTextureIndicesVec4.z, \
                    lTileTextureIndicesVec4.w, \
                    lhTileInfo, \
                    lSpecularLargeVec3.y, \
                    lfSpecularCoeff, \
                    lDisplacementLargeVec3.y, \
                    lHeightsLargeY[0][0], lHeightsLargeY[0][1], lHeightsLargeY[0][2], lHeightsLargeY[0][3], \
                    lHeightsLargeY[1][0], lHeightsLargeY[1][1], lHeightsLargeY[1][2], lHeightsLargeY[1][3] \
                ); \
                half3 lNoiseNormalVec3; \
                GetTriPlanarNoiseNormal( \
                    lNoiseNormalVec3, \
                    lLargeNoiseTexCoordsVec3.zx, \
                    SAMPLER2DPARAM( lNoiseNormalMap ), \
                    lfHeightNoise,  \
                    lfSpecularNoise ); \
                half3 lPlaneNorm          = half3( half2(lNormalVec3.zx), abs( lNormalVec3.y ) ); \
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 ); \
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale ); \
                lNormalLargeVec3.z       *= sign( lNormalVec3.y ); \
                lNormalLargeVec3          = lNormalLargeVec3.yzx; \
                lLargeMappedNormalVec3   += half( lWeightsVec3N.y ) * lNormalLargeVec3; \
                lSpecularLargeVec3.y      = saturate( lSpecularLargeVec3.y     - lfSpecularNoise * half( lfSpecularCoeff ) ); \
                lDisplacementLargeVec3.y  = saturate( lDisplacementLargeVec3.y + lfHeightNoise ); \
            } \
            if ( lWeightsVec3N.z > kfWeightFloor ) \
            { \
                GetBlendedNormal( \
                    lNormalLargeVec3, \
                    lCustomUniforms, \
                    lLargeTexCoordsVec3.xy, \
                    SAMPLER2DARRAYPARAM( lNormalMap ), \
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ), \
                    lTileTextureIndicesVec4.z, \
                    lTileTextureIndicesVec4.w, \
                    lhTileInfo, \
                    lSpecularLargeVec3.z, \
                    lfSpecularCoeff, \
                    lDisplacementLargeVec3.z, \
                    lHeightsLargeZ[0][0], lHeightsLargeZ[0][1], lHeightsLargeZ[0][2], lHeightsLargeZ[0][3], \
                    lHeightsLargeZ[1][0], lHeightsLargeZ[1][1], lHeightsLargeZ[1][2], lHeightsLargeZ[1][3] \
                ); \
                half3 lNoiseNormalVec3; \
                GetTriPlanarNoiseNormal( \
                    lNoiseNormalVec3, \
                    lLargeNoiseTexCoordsVec3.xy, \
                    SAMPLER2DPARAM( lNoiseNormalMap ), \
                    lfHeightNoise,  \
                    lfSpecularNoise ); \
                half3 lPlaneNorm          = half3( half2(lNormalVec3.xy), abs( lNormalVec3.z ) ); \
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 ); \
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale ); \
                lNormalLargeVec3.z       *= sign( lNormalVec3.z ); \
                lLargeMappedNormalVec3   += half( lWeightsVec3N.z ) * lNormalLargeVec3; \
                lSpecularLargeVec3.z      = saturate( lSpecularLargeVec3.z     - lfSpecularNoise * half( lfSpecularCoeff ) ); \
                lDisplacementLargeVec3.z  = saturate( lDisplacementLargeVec3.z + lfHeightNoise ); \
            } \
        } \
        else \
        { \
            lfFade = 0.0; \
        } \
        if ( lfFade < 1.0 - lfFadeThreshold ) \
        { \
            if ( lWeightsVec3N.x > kfWeightFloor ) \
            { \
                GetBlendedNormal( \
                    lNormalSmallVec3, \
                    lCustomUniforms, \
                    lSmallTexCoordsVec3.yz, \
                    SAMPLER2DARRAYPARAM( lNormalMap ), \
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ), \
                    lTileTextureIndicesVec4.x, \
                    lTileTextureIndicesVec4.y, \
                    lhTileInfo, \
                    lSpecularSmallVec3.x, \
                    lfSpecularCoeff, \
                    lDisplacementSmallVec3.x, \
                    lHeightsSmallX[0][0], lHeightsSmallX[0][1], lHeightsSmallX[0][2], lHeightsSmallX[0][3], \
                    lHeightsSmallX[1][0], lHeightsSmallX[1][1], lHeightsSmallX[1][2], lHeightsSmallX[1][3] \
                ); \
                half3 lNoiseNormalVec3; \
                GetTriPlanarNoiseNormal( \
                    lNoiseNormalVec3, \
                    lSmallNoiseTexCoordsVec3.yz, \
                    SAMPLER2DPARAM( lNoiseNormalMap ), \
                    lfHeightNoise,  \
                    lfSpecularNoise ); \
                half3 lPlaneNorm          = half3( half2(lNormalVec3.yz), abs( lNormalVec3.x ) ); \
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 ); \
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale ); \
                lNormalSmallVec3.z       *= sign( lNormalVec3.x ); \
                lNormalSmallVec3          = lNormalSmallVec3.zxy; \
                lSmallMappedNormalVec3   += half( lWeightsVec3N.x ) * lNormalSmallVec3; \
                lSpecularSmallVec3.x      = saturate( lSpecularSmallVec3.x     - lfSpecularNoise * half( lfSpecularCoeff ) ); \
                lDisplacementSmallVec3.x  = saturate( lDisplacementSmallVec3.x + lfHeightNoise ); \
            } \
            if ( lWeightsVec3N.y > kfWeightFloor ) \
            { \
                GetBlendedNormal( \
                    lNormalSmallVec3, \
                    lCustomUniforms, \
                    lSmallTexCoordsVec3.zx, \
                    SAMPLER2DARRAYPARAM( lNormalMap ), \
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ), \
                    lTileTextureIndicesVec4.x, \
                    lTileTextureIndicesVec4.y, \
                    lhTileInfo, \
                    lSpecularSmallVec3.y, \
                    lfSpecularCoeff, \
                    lDisplacementSmallVec3.y, \
                    lHeightsSmallY[0][0], lHeightsSmallY[0][1], lHeightsSmallY[0][2], lHeightsSmallY[0][3], \
                    lHeightsSmallY[1][0], lHeightsSmallY[1][1], lHeightsSmallY[1][2], lHeightsSmallY[1][3] \
                ); \
                half3 lNoiseNormalVec3; \
                GetTriPlanarNoiseNormal( \
                    lNoiseNormalVec3, \
                    lSmallNoiseTexCoordsVec3.zx, \
                    SAMPLER2DPARAM( lNoiseNormalMap ), \
                    lfHeightNoise,  \
                    lfSpecularNoise ); \
                half3 lPlaneNorm          = half3( half2(lNormalVec3.zx), abs( lNormalVec3.y ) ); \
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 ); \
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale ); \
                lNormalSmallVec3.z       *= sign( lNormalVec3.y ); \
                lNormalSmallVec3          = lNormalSmallVec3.yzx; \
                lSmallMappedNormalVec3   += half( lWeightsVec3N.y ) * lNormalSmallVec3; \
                lSpecularSmallVec3.y      = saturate( lSpecularSmallVec3.y     - lfSpecularNoise * half( lfSpecularCoeff ) ); \
                lDisplacementSmallVec3.y  = saturate( lDisplacementSmallVec3.y + lfHeightNoise ); \
            } \
            if ( lWeightsVec3N.z > kfWeightFloor ) \
            { \
                GetBlendedNormal( \
                    lNormalSmallVec3, \
                    lCustomUniforms, \
                    lSmallTexCoordsVec3.xy, \
                    SAMPLER2DARRAYPARAM( lNormalMap ), \
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ), \
                    lTileTextureIndicesVec4.x, \
                    lTileTextureIndicesVec4.y, \
                    lhTileInfo, \
                    lSpecularSmallVec3.z, \
                    lfSpecularCoeff, \
                    lDisplacementSmallVec3.z, \
                    lHeightsSmallZ[0][0], lHeightsSmallZ[0][1], lHeightsSmallZ[0][2], lHeightsSmallZ[0][3], \
                    lHeightsSmallZ[1][0], lHeightsSmallZ[1][1], lHeightsSmallZ[1][2], lHeightsSmallZ[1][3] \
                ); \
                half3 lNoiseNormalVec3; \
                GetTriPlanarNoiseNormal( \
                    lNoiseNormalVec3, \
                    lSmallNoiseTexCoordsVec3.xy, \
                    SAMPLER2DPARAM( lNoiseNormalMap ), \
                    lfHeightNoise,  \
                    lfSpecularNoise ); \
                half3 lPlaneNorm                = half3( half2(lNormalVec3.xy), abs( lNormalVec3.z ) ); \
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 ); \
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale ); \
                lNormalSmallVec3.z       *= sign( lNormalVec3.z ); \
                lSmallMappedNormalVec3   += half( lWeightsVec3N.z ) * lNormalSmallVec3; \
                lSpecularSmallVec3.z      = saturate( lSpecularSmallVec3.z     - lfSpecularNoise * half( lfSpecularCoeff ) ); \
                lDisplacementSmallVec3.z  = saturate( lDisplacementSmallVec3.z + lfHeightNoise ); \
            } \
        } \
        else \
        { \
            lfFade = 1.0; \
        } \
 \
        for (int i = 0; i < 8; i++) \
        { \
            float lfHeightLarge = dot(vec3(lHeightsLargeX[i/4][i%4], lHeightsLargeY[i/4][i%4], lHeightsLargeZ[i/4][i%4]), lWeightsVec3) * lfWeightRecip; \
            float lfHeightSmall = dot(vec3(lHeightsSmallX[i/4][i%4], lHeightsSmallY[i/4][i%4], lHeightsSmallZ[i/4][i%4]), lWeightsVec3) * lfWeightRecip; \
            lafHeights[i/4][i%4] = mix(lfHeightSmall, lfHeightLarge, lfFade); \
        } \
 \
        float lfDisplacementLarge = dot( lDisplacementLargeVec3, lWeightsVec3 ) * lfWeightRecip; \
        float lfDisplacementSmall = dot( lDisplacementSmallVec3, lWeightsVec3 ) * lfWeightRecip; \
        lfDisplacement = mix( lfDisplacementSmall, lfDisplacementLarge, lfFade ); \
 \
        lfHeight = lfDisplacement; \
 \
        float lfWidth = 0.05; \
 \
        _FastSmoothStep(lafHeights[0][0],  lafHeights[0][0] - lfWidth, 2.0f * lfWidth, lfPatch ); \
        _FastSmoothStep(lafHeights[0][1],  lafHeights[0][1] - lfWidth, 2.0f * lfWidth, lfPatch ); \
        _FastSmoothStep(lafHeights[0][2],  lafHeights[0][2] - lfWidth, 2.0f * lfWidth, lfPatch ); \
        _FastSmoothStep(lafHeights[0][3],  lafHeights[0][3] - lfWidth, 2.0f * lfWidth, lfPatch ); \
        _FastSmoothStep(lafHeights[1][0],  lafHeights[1][0] - lfWidth, 2.0f * lfWidth, lfSlope1 ); \
        _FastSmoothStep(lafHeights[1][1],  lafHeights[1][1] - lfWidth, 2.0f * lfWidth, lfSlope2 ); \
        _FastSmoothStep(lafHeights[1][2],  lafHeights[1][2] - lfWidth, 2.0f * lfWidth, lfTileType ); \
        _FastSmoothStep(lafHeights[1][3],  lafHeights[1][3] - lfWidth, 2.0f * lfWidth, lfFade ); \
 \
        float lfSpecularLarge = dot( vec3(lSpecularLargeVec3), lWeightsVec3 ) * lfWeightRecip; \
        float lfSpecularSmall = dot( vec3(lSpecularSmallVec3), lWeightsVec3 ) * lfWeightRecip; \
        lLargeMappedNormalVec3 *= half( lfFade ); \
        lSmallMappedNormalVec3 *= half( 1.0 - lfFade ); \
        lSmallMappedNormalVec3  = half3 (lSmallMappedNormalVec3.x + lLargeMappedNormalVec3.x, lSmallMappedNormalVec3.y + lLargeMappedNormalVec3.y, lSmallMappedNormalVec3.z + lLargeMappedNormalVec3.z);             \
 \
        lOutWorldNormalVec3     = normalize( mix( lFaceNormalVec3, vec3(lSmallMappedNormalVec3), kfNormBlend * lfWaterFade ) ); \
        lfSpecular              = mix( lfSpecularSmall, lfSpecularLarge, lfFade ); \
 \
    } \
 \
    vec3    lTileColourSmallVec3 = vec3( 0.0, 0.0, 0.0 ); \
    vec3    lTileColourLargeVec3 = vec3( 0.0, 0.0, 0.0 ); \
    \
    if ( lWeightsVec3N.x > kfWeightFloor ) \
    { \
        vec3 BlendedColor;\
        GetBlendedColour(\
            BlendedColor,\
            lCustomUniforms,\
            lLargeTexCoordsVec3.zy,\
            SAMPLER2DARRAYPARAM( lDiffuseMap ),\
            SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),\
            lTileTextureIndicesVec4.z,\
            lTileTextureIndicesVec4.w,\
            lafHeights,\
            lfMetallic);\
        lTileColourLargeVec3 += lWeightsVec3N.x * BlendedColor;\
    } \
    if ( lWeightsVec3N.y > kfWeightFloor ) \
    { \
        vec3 Color;\
        GetBlendedColour(\
            Color,\
            lCustomUniforms,\
            lLargeTexCoordsVec3.zx,\
            SAMPLER2DARRAYPARAM( lDiffuseMap ),\
            SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),\
            lTileTextureIndicesVec4.z,\
            lTileTextureIndicesVec4.w,\
            lafHeights,\
            lfMetallic );\
        lTileColourLargeVec3 += lWeightsVec3N.y * Color;\
    } \
    if ( lWeightsVec3N.z > kfWeightFloor ) \
    { \
        vec3 Color;\
        GetBlendedColour( \
            Color, \
            lCustomUniforms, \
            lLargeTexCoordsVec3.xy, \
            SAMPLER2DARRAYPARAM( lDiffuseMap ), \
            SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), \
            lTileTextureIndicesVec4.z, \
            lTileTextureIndicesVec4.w, \
            lafHeights, \
            lfMetallic ); \
        lTileColourLargeVec3 += lWeightsVec3N.z * Color; \
    } \
    if ( lWeightsVec3N.x > kfWeightFloor ) \
    { \
        vec3 Color; \
        GetBlendedColour( \
            Color, \
            lCustomUniforms, \
            lSmallTexCoordsVec3.zy, \
            SAMPLER2DARRAYPARAM( lDiffuseMap ), \
            SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), \
            lTileTextureIndicesVec4.x, \
            lTileTextureIndicesVec4.y, \
            lafHeights, \
            lfMetallic ); \
        lTileColourSmallVec3 += lWeightsVec3N.x * Color; \
    } \
    if ( lWeightsVec3N.y > kfWeightFloor ) \
    { \
        vec3 Color; \
        GetBlendedColour( \
            Color, \
            lCustomUniforms, \
            lSmallTexCoordsVec3.zx, \
            SAMPLER2DARRAYPARAM( lDiffuseMap ), \
            SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), \
            lTileTextureIndicesVec4.x, \
            lTileTextureIndicesVec4.y, \
            lafHeights, \
            lfMetallic ); \
        lTileColourSmallVec3 += lWeightsVec3N.y * Color; \
    } \
    if ( lWeightsVec3N.z > kfWeightFloor ) \
    { \
        vec3 Color; \
        GetBlendedColour( \
            Color, \
            lCustomUniforms, \
            lSmallTexCoordsVec3.xy, \
            SAMPLER2DARRAYPARAM( lDiffuseMap ), \
            SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), \
            lTileTextureIndicesVec4.x, \
            lTileTextureIndicesVec4.y, \
            lafHeights, \
            lfMetallic ); \
        lTileColourSmallVec3 += lWeightsVec3N.z * Color; \
    } \
    lTileColourVec3 = mix( lTileColourSmallVec3, lTileColourLargeVec3, lfFade ); \
    lTileColourVec3 = GammaCorrectInput01( lTileColourVec3 ); \
} while(0)

    // lTileColourVec3 = (Color0 + Color1 + Color2 + Color3 + Color4 + Color5) / 6.0; \
#endif

#endif