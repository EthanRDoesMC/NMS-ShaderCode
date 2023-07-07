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
#if defined(D_PLATFORM_METAL)
#include "Common/CommonTriplanarTexturingApple.shader.h"
#elif !defined(D_COMMONTRIPLANARTEXTURING)
#define D_COMMONTRIPLANARTEXTURING

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
STATIC_CONST float kfNoiseNormScale     = 0.50;
STATIC_CONST half  kfNoiseHeightScale   = 0.50;
STATIC_CONST half  kfNoiseSpecularScale = 0.30;

#if defined ( D_FORCE_HALF )
STATIC_CONST half kfNormBlend = 0.85;
#else
STATIC_CONST float kfNormBlend = 0.85;
#endif

void
PackR8(
    inout  uint u,
#if defined ( D_FORCE_HALF )
    half f )
#else
    float f)
#endif
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 0, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 0, u);
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    u = bitfieldInsert( u, uint( f * 255.0 ), 24, 8 );
#else
    u = (u & 0x00ffffff) | uint(f * 255.0) << 24;
#endif
}

void
PackG8(
    inout  uint u,
#if defined ( D_FORCE_HALF )
    half f )
#else
    float f)
#endif
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 1, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 1, u);
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    u = bitfieldInsert( u, uint( f * 255.0 ), 16, 8 );
#else
    u = (u & 0xff00ffff) | uint(f * 255.0) << 16;
#endif
}

void
PackB8(
    inout  uint u,
#if defined ( D_FORCE_HALF )
    half f )
#else
    float f )
#endif
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 2, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 2, u);
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    u = bitfieldInsert( u, uint( f * 255.0 ), 8, 8 );
#else
    u = (u & 0xffff00ff) | uint(f * 255.0) << 8;
#endif
}

void
PackA8(
    inout  uint u,
#if defined ( D_FORCE_HALF )
    half f )
#else
    float f )
#endif
{
#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    u = PackFloatToByteOfUInt(f * 255.0, 3, u);
#elif defined ( D_PLATFORM_XBOXONE )
    u = __XB_PackF32ToU8(f * 255.0, 3, u);
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    u = bitfieldInsert( u, uint( f * 255.0 ), 0, 8 );
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
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    return half( bitfieldExtract( u, 24, 8 ) ) / 255.0;
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
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    return half( bitfieldExtract( u, 16, 8 ) ) / 255.0;
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
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    return half( bitfieldExtract( u, 8, 8 ) ) / 255.0;
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
#elif defined ( D_PLATFORM_SWITCH ) && defined ( D_FORCE_HALF )
    return half( bitfieldExtract( u, 0, 8 ) ) / 255.0;
#else
    return half(u & 0xff) / 255.0;
#endif
}

#if defined ( D_FORCE_HALF )
half
#else
float
#endif
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


#if defined ( D_FORCE_HALF )
half
#else
float
#endif
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

#if defined ( D_FORCE_HALF )
half
#else
float
#endif
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

#if defined ( D_FORCE_HALF )
half
#else
float
#endif
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
#if defined ( D_FORCE_HALF )
    half a,
    half b,
    half c,
    half d )
#else
    float a,
    float b,
    float c,
    float d )
#endif
{
    uint u;

#if defined ( D_PLATFORM_ORBIS ) || defined ( D_PLATFORM_PROSPERO )
    vec4 input = vec4( a, b, c, d );
    u = PackFloat4ToUInt( input );
#elif defined ( D_PLATFORM_XBOXONE ) || defined ( D_PLATFORM_SWITCH )
    PackR8(u, a);
    PackG8(u, b);
    PackB8(u, c);
    PackA8(u, d);
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
    #if defined ( D_FORCE_HALF )
    half4  f4 )
    #else
    vec4  f4 )
    #endif
{
    uint  u = 0;
    
    #if defined ( D_FORCE_HALF ) && defined ( D_PLATFORM_SWITCH )
    half m;
    m = max( min( mix( UnpackR8NoDiv( u1 ), UnpackR8NoDiv( u2 ), f4.x ), 255.0 ), 0.0 );
    u = bitfieldInsert( u, uint( m ), 24, 8 );
    
    m = max( min( mix( UnpackG8NoDiv( u1 ), UnpackG8NoDiv( u2 ), f4.y ), 255.0 ), 0.0 );
    u = bitfieldInsert( u, uint( m ), 16, 8 );

    m = max( min( mix( UnpackB8NoDiv( u1 ), UnpackB8NoDiv( u2 ), f4.z ), 255.0 ), 0.0 );
    u = bitfieldInsert( u, uint( m ), 8, 8 );

    m = max( min( mix( UnpackA8NoDiv( u1 ), UnpackA8NoDiv( u2 ), f4.w ), 255.0 ), 0.0 );
    u = bitfieldInsert( u, uint( m ), 0, 8 );
    #else
    float m;
    m = mix( UnpackR8NoDiv(u1), UnpackR8NoDiv(u2), f4.x ) / 255.0; PackR8( u, m );
    m = mix( UnpackG8NoDiv(u1), UnpackG8NoDiv(u2), f4.y ) / 255.0; PackG8( u, m );
    m = mix( UnpackB8NoDiv(u1), UnpackB8NoDiv(u2), f4.z ) / 255.0; PackB8( u, m );
    m = mix( UnpackA8NoDiv(u1), UnpackA8NoDiv(u2), f4.w ) / 255.0; PackA8( u, m );
    #endif

    return u;
}


void  PackR16(inout uint u, float f) { u = (u & 0x0000ffff) | uint(f * 65535.0) << 16; }
void  PackG16(inout uint u, float f) { u = (u & 0xffff0000) | uint(f * 65535.0); }
void  PackRG16(inout uint u, vec2  f2) { u = (uint(f2.x * 65535.0) << 16) | (uint(f2.y * 65535.0)); }

half  UnpackR16(in uint u) { return half(u >> 16) / 65535.0; }
half  UnpackG16(in uint u) { return half(u & 0xffff) / 65535.0; }
vec2  UnpackRG16(in uint u) { return vec2(UnpackR16(u), UnpackG16(u)); }

#if defined ( D_PACKED_NORMALS )

uint
PackNormalsXY(
    float lfNormalX,
    float lfNormalY)
{
    uint u;
    u = PackFloat2ToUInt(lfNormalX, lfNormalY);
    return u;
}

uvec2
PackNormalsXYZ(
    float lfNormalX,
    float lfNormalY,
    float lfNormalZ)
{
    uvec2 u;

    u.x = PackFloat2ToUInt(lfNormalX, lfNormalY);
    u.y = asuint(lfNormalZ);

    return u;
}

void
UnpackNormalsXY(
    in uint u,
    out float lfNormalX,
    out float lfNormalY)
{
    lfNormalX = f16tof32(u);
    lfNormalY = f16tof32(u >> 16);
}

void
UnpackNormalsXYZ(
    in uvec2 u,
    out float lfNormalX,
    out float lfNormalY,
    out float lfNormalZ)
{
    lfNormalX = f16tof32(u.x);
    lfNormalY = f16tof32(u.x >> 16);
    lfNormalZ = asfloat(u.y);
}

#endif

#if defined ( D_PACKED_HEIGHTS )

uint
PackHeightsAB(
    float lfHeightA,
    float lfHeightB)
{
    uint u;
    u = PackFloat2ToUInt( lfHeightA, lfHeightB );
    return u;
}

float
UnpackHeightA(
    in uint u)
{
    return f16tof32( u );
}

float
UnpackHeightB(
    in uint u)
{
    return f16tof32( u >> 16 );
}

void
UnpackHeightsABC(
    in uvec2 u,
    out float lfHeightA,
    out float lfHeightB,
    out float lfHeightC)
{
    lfHeightA = UnpackHeightA( u.x );
    lfHeightB = UnpackHeightB( u.x );
    lfHeightC = UnpackHeightA( u.y );
}

#endif

#if defined ( D_PACKED_SPECULAR )

uint
PackSpecular(
    float lfSpecularA,
    float lfSpecularB )
{
    uint u;
    u = PackFloat2ToUInt( lfSpecularA, lfSpecularB );
    return u;
}

void
UnpackSpecular(
    in uint u,
    out float lfSpecularA,
    out float lfSpecularB )
{
    lfSpecularA = f16tof32( u );
    lfSpecularB = f16tof32( u >> 16 );
}

#endif

uint
PackItemXChannel(
    uint id )
{
    return id & 0x1f;
}

uint
PackItemYChannel(
    uint id )
{
    return ( id & 0x1f ) << 5;
}

uint
PackItemZChannel(
    uint id )
{
    return ( id & 0x1f ) << 10;
}

uint
PackItemWChannel(
    uint id )
{
    return ( id & 0x1f ) << 15;
}

uint
UnpackItemXChannel(
    uint id )
{
#if defined ( D_PLATFORM_SWITCH )
    return bitfieldExtract( id, 0, 5 );
#else
    return id & 0x1f;
#endif
}

uint
UnpackItemYChannel(
    uint id
)
{
#if defined ( D_PLATFORM_SWITCH )
    return bitfieldExtract( id, 5, 5 );
#else
    return ( id >> 5 ) & 0x1f;
#endif
}

uint
UnpackItemZChannel(
    uint id
)
{
#if defined ( D_PLATFORM_SWITCH )
    return bitfieldExtract( id, 10, 5 );
#else
    return ( id >> 10 ) & 0x1f;
#endif
}

uint
UnpackItemWChannel(
    uint id
)
{
#if defined ( D_PLATFORM_SWITCH )
    return bitfieldExtract( id, 15, 5 );
#else
    return ( id >> 15 ) & 0x1f;
#endif
}

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

    m /= m.x + m.y + m.z;
    lWorldPositionVec3 *= lfScale;

    vec3 lColourVec3;
    vec2 lCoordVec2;

    lCoordVec2   = lWorldPositionVec3.yz + lAnimationOffsetVec2;
    lColourVec3  = texture2DComputeGrad( lTexture, lCoordVec2 ).rgb * m.x;
    lCoordVec2   = lWorldPositionVec3.zx + lAnimationOffsetVec2;
    lColourVec3 += texture2DComputeGrad( lTexture, lCoordVec2 ).rgb * m.y;
    lCoordVec2   = lWorldPositionVec3.xy + lAnimationOffsetVec2;
    lColourVec3 += texture2DComputeGrad( lTexture, lCoordVec2 ).rgb * m.z;

    return lColourVec3;
}


vec3
GetTriPlanarNormal(
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

    vec2  lCoord1Vec2 = vec2( lWorldPositionVec3.z, -lWorldPositionVec3.y ) * lfScale + lAnimationOffsetVec2;
    vec2  lCoord2Vec2 = vec2( lWorldPositionVec3.z, lWorldPositionVec3.x ) * lfScale + lAnimationOffsetVec2;
    vec2  lCoord3Vec2 = vec2( lWorldPositionVec3.x, -lWorldPositionVec3.y ) * lfScale + lAnimationOffsetVec2;

    vec3 lNormal1Vec3 = DecodeNormalMap( texture2D( lTexture, lCoord1Vec2 ) );
    vec3 lNormal2Vec3 = DecodeNormalMap( texture2D( lTexture, lCoord2Vec2 ) );
    vec3 lNormal3Vec3 = DecodeNormalMap( texture2D( lTexture, lCoord3Vec2 ) );

    lNormal1Vec3 = vec3( 0.0, lNormal1Vec3.x, lNormal1Vec3.y );  // YZ 
    lNormal2Vec3 = vec3( -lNormal2Vec3.x, 0.0, lNormal2Vec3.y );  // ZX
    lNormal3Vec3 = vec3( lNormal3Vec3.y, lNormal3Vec3.x, 0.0 );  // XY

    return ( lNormal1Vec3 * m.x + lNormal2Vec3 * m.y + lNormal3Vec3 * m.z ) / ( m.x + m.y + m.z );
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

half3
BlendNormalsRNM_HALF(
    half3 lBaseNormal,
    half3 lDetailNormal )
{
    half3 n;
    half3 n1 = lBaseNormal;
    half3 n2 = lDetailNormal;
    n1.z += 1.0;
    n2.xy *= -1.0;
    n = n1 * dot( n1, n2 ) / n1.z - n2;
    return n;
}


half
GetSpecularNoiseCoeff( 
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

half3
GetTriPlanarNoiseNormal(    
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

    return half3( lLocalNormalAVec4.rgb );
}

#if defined ( D_PACKED_NORMALS )
uvec2
#else
half3
#endif
GetTriPlanarNormalArray(
    in  CustomPerMaterialUniforms lCustomUniforms,
#if defined ( D_FORCE_HALF_UV )
    in  half2          lCoordVec2,
#else
    in  vec2           lCoordVec2,
#endif
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    in  int            liIndex,
#if defined ( D_FORCE_HALF )
    out half          lfHeight,
    out half          lfDisplacement,
#else
    out float          lfHeight,
    out float          lfDisplacement,
#endif
#if defined ( D_PACKED_SPECULAR )
    out uint           lSpecular )
#else
    out half2          lSpecular )
#endif
{
#if defined ( D_FORCE_HALF )
    half4 lTexValueVec4;
#else
    vec4 lTexValueVec4;
#endif

#if defined ( D_PLATFORM_SWITCH )
    lTexValueVec4 = texture2DArray( 
        liIndex < 16 ? lNormalMap : lSubstanceNormalMap,
        vec3( lCoordVec2, float( liIndex & 15 ) ) );
#else
    if ( liIndex < 16 )
    {
#ifdef D_CACHE_HEIGHT
        lTexValueVec4 = texture2DArrayLod( lNormalMap, vec3( lCoordVec2, float( liIndex ) ), 0 );
#else
        lTexValueVec4 = texture2DArray( lNormalMap, vec3( lCoordVec2, float( liIndex ) ) );
#endif
    }
    else
    {
#ifdef D_CACHE_HEIGHT
        lTexValueVec4 = texture2DArrayLod( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ), 0 );
#else
        lTexValueVec4 = texture2DArray( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ) );
#endif
    }
#endif
    
#if defined ( D_FORCE_HALF )
    half lfBrightness = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).a;
    half lfContrast = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).a;
#else
    float lfBrightness  = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).a;
    float lfContrast    = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).a;
#endif

    lfDisplacement      = lTexValueVec4.r;
    lfHeight            = lTexValueVec4.r;
    lfHeight           += lfBrightness;
    lfHeight            = saturate( ( ( lfHeight - 0.5 ) * max( lfContrast + 1.0, 0.0 ) ) + 0.5 );

#if defined ( D_PACKED_SPECULAR )
    lSpecular = PackSpecular( half(lTexValueVec4.b), GetSpecularNoiseCoeff(lCustomUniforms, liIndex) );
#else
    lSpecular.x         = half(lTexValueVec4.b);
    lSpecular.y         = GetSpecularNoiseCoeff( lCustomUniforms, liIndex );
#endif

#if defined ( D_PACKED_NORMALS )
    uvec2 lLocalNormal;

    vec3 lLocalNormalAVec3 = vec3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
    lLocalNormal.x = PackNormalsXY( lLocalNormalAVec3.x, lLocalNormalAVec3.y );
#if !defined( D_NO_Z_NORM )
    lLocalNormal.y = asuint(sqrt(saturate(1.0 - lLocalNormalAVec3.x*lLocalNormalAVec3.x - lLocalNormalAVec3.y*lLocalNormalAVec3.y)));
#endif

    return lLocalNormal;
#else

#if defined ( D_FORCE_HALF )
    half3 lLocalNormalAVec3 = half3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
#else
    vec3 lLocalNormalAVec3 = vec3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
#endif

#if !defined( D_NO_Z_NORM )
    lLocalNormalAVec3.z = sqrt(saturate(1.0 - lLocalNormalAVec3.x*lLocalNormalAVec3.x - lLocalNormalAVec3.y*lLocalNormalAVec3.y));
#endif

    return half3(lLocalNormalAVec3);
#endif
}

#if defined ( D_PACKED_NORMALS )
uvec2
#else
half3
#endif
GetTriPlanarNormalArrayPacked(
    in  CustomPerMaterialUniforms lCustomUniforms,
#if defined ( D_FORCE_HALF_UV )
    in  half2          lCoordVec2,
#else
    in  vec2           lCoordVec2,
#endif
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    in  int            liIndex,
    out uint           luHDS2 )
{
    #if defined ( D_FORCE_HALF )
    half4 lTexValueVec4;
    #else
    vec4 lTexValueVec4;
    #endif

#if defined ( D_PLATFORM_SWITCH )
    lTexValueVec4 = texture2DArray(
        liIndex < 16 ? lNormalMap : lSubstanceNormalMap,
        vec3( lCoordVec2, float( liIndex & 15 ) ) );
#else
    if ( liIndex < 16 )
    {
    #ifdef D_CACHE_HEIGHT
        lTexValueVec4 = texture2DArrayLod( lNormalMap, vec3( lCoordVec2, float( liIndex ) ), 0 );
    #else
        lTexValueVec4 = texture2DArray( lNormalMap, vec3( lCoordVec2, float( liIndex ) ) );
    #endif
    }
    else
    {
    #ifdef D_CACHE_HEIGHT
        lTexValueVec4 = texture2DArrayLod( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ), 0 );
    #else
        lTexValueVec4 = texture2DArray( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ) );
    #endif
    }
#endif

    PackG8( luHDS2, lTexValueVec4.r );
    PackB8( luHDS2, lTexValueVec4.b );

    #if defined ( D_FORCE_HALF )
    half lfBrightness = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).a;
    half lfContrast = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).a;
    half lfTemp = lTexValueVec4.r;
    #else
    float lfBrightness  = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).a;
    float lfContrast    = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).a;
    float lfTemp        = lTexValueVec4.r;
    #endif
    
    lfTemp             += lfBrightness;
    lfTemp              = saturate( ( ( lfTemp - 0.5 ) * max( lfContrast + 1.0, 0.0 ) ) + 0.5 );
    
    PackR8( luHDS2, lfTemp );

    lfTemp              = GetSpecularNoiseCoeff( lCustomUniforms, liIndex );

    PackA8( luHDS2, lfTemp );

#if defined ( D_PACKED_NORMALS )
    uvec2 lLocalNormal;

    vec3 lLocalNormalAVec3 = vec3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
    lLocalNormal.x = PackNormalsXY(lLocalNormalAVec3.x, lLocalNormalAVec3.y);
#if !defined( D_NO_Z_NORM )
    lLocalNormal.y = asuint(sqrt(saturate(1.0 - lLocalNormalAVec3.x*lLocalNormalAVec3.x - lLocalNormalAVec3.y*lLocalNormalAVec3.y)));
#endif

    return lLocalNormal;
#else

    #if defined ( D_FORCE_HALF )
    half3 lLocalNormalAVec3 = half3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
    #else
    vec3 lLocalNormalAVec3 = vec3( lTexValueVec4.g * 2.0 - 1.0, -lTexValueVec4.a * 2.0 + 1.0, 0.0 );
    #endif

#if !defined( D_NO_Z_NORM )
    lLocalNormalAVec3.z = sqrt(saturate(1.0 - lLocalNormalAVec3.x*lLocalNormalAVec3.x - lLocalNormalAVec3.y*lLocalNormalAVec3.y));
#endif

    return half3( lLocalNormalAVec3 );
#endif
}

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
GetTriPlanarColourArray(
#if defined ( D_FORCE_HALF_UV )
    in half2           lCoordVec2,
#else
    in vec2            lCoordVec2,
#endif
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    in int             liIndex,
#if defined ( D_FORCE_HALF )
    in half3            lAverageHSVVec3,
    in half3            lRecolourVec3 )
#else
    in vec3            lAverageHSVVec3,
    in vec3            lRecolourVec3 )
#endif
{
    #if defined ( D_FORCE_HALF )
    half3 lColourVec3;
    #else
    vec3 lColourVec3;
    #endif

#if defined ( D_PLATFORM_SWITCH )
    lColourVec3 = texture2DArray( 
        liIndex < 16 ? lDiffuseMap : lSubstanceDiffuseMap,
        vec3( lCoordVec2, float( liIndex & 15 ) ) ).rgb;
#else
    if ( liIndex < 16 )
    {
        lColourVec3 = texture2DArray( lDiffuseMap, vec3( lCoordVec2, float( liIndex ) ) ).rgb;
    }
    else
    {
        lColourVec3 = texture2DArray( lSubstanceDiffuseMap, vec3( lCoordVec2, float( liIndex & 15 ) ) ).rgb;
    }
#endif

    lColourVec3.r = fract( ( lColourVec3.r - lAverageHSVVec3.r ) + lRecolourVec3.r );
    lColourVec3.g = saturate( min( lRecolourVec3.g, lColourVec3.g ) );
    lColourVec3.b = saturate( ( lColourVec3.b - lAverageHSVVec3.b ) + lRecolourVec3.b );
    
    #if defined ( D_FORCE_HALF )
    lColourVec3 = saturate( HSVToRGBHalf( lColourVec3 ) );
    #else 
    lColourVec3 = saturate( HSVToRGB( lColourVec3 ) );
    #endif

    return lColourVec3;
}


vec3
GetTriPlanarNormalOnlyArray(
    in  CustomPerMaterialUniforms lCustomUniforms,
    in  vec2           lCoordVec2,
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    in  int            liIndex )
{
    vec2 lTexValueVec2;

    if ( liIndex < 16 )
    {
#ifdef D_CACHE_HEIGHT
        lTexValueVec2 = texture2DArrayLod( lNormalMap, vec3( lCoordVec2, float( liIndex ) ), 0 ).ga;
#else
        lTexValueVec2 = texture2DArray( lNormalMap, vec3( lCoordVec2, float( liIndex ) ) ).ga;
#endif
    }
    else
    {
#ifdef D_CACHE_HEIGHT
        lTexValueVec2 = texture2DArrayLod( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ), 0 ).ga;
#else
        lTexValueVec2 = texture2DArray( lSubstanceNormalMap, vec3( lCoordVec2, float( liIndex & 15 ) ) ).ga;
#endif
    }

    vec3 lLocalNormalAVec3 = vec3( lTexValueVec2.y * 2.0 - 1.0, lTexValueVec2.x * 2.0 - 1.0, 0.0 );
    lLocalNormalAVec3.z    = sqrt( max( 0.0, 1.0 - lLocalNormalAVec3.x*lLocalNormalAVec3.x - lLocalNormalAVec3.y*lLocalNormalAVec3.y ) );

    return lLocalNormalAVec3;
}


//-----------------------------------------------------------------------------
///
///     GetBlendedNormalSingleTileUnpacked
///
///     @brief      GetBlendedNormalSingleTileUnpacked
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if defined ( D_PLATFORM_SWITCH )
half3
GetBlendedNormalSingleTileUnpacked(
    in  CustomPerMaterialUniforms lCustomUniforms,
#if defined ( D_FORCE_HALF_UV )
    in    half2        lTileCoordsVec2,
#else
    in    vec2         lTileCoordsVec2,
#endif
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    in    uint        lTile1UInt,
    in    half        lfPatch,
    in    half        lfSlope1,
    inout   half      lfSpecular,
    inout   half      lfDisplacement,
    inout   uint      luPackedHeights,
    in      half      lfScale )
{
    const half lfThreshold = 1.0 / 20.0;
    const half lfOffset = 0.1;
    const half lfInverseOffset2 = 1.0 / ( lfOffset + lfOffset );
    const half lfDefaultHeight = 0.5;

    half3 lNormal5;
    half2 lfSpecular5;
    half lfHeightE;
    half lfDisplacementE;

    {
        half3  lNormal3;
        half3  lNormal4;

        half lfHeightC;
        half lfHeightD;
        half lfDisplacementC;
        half lfDisplacementD;
        half2 lfSpecular3;
        half2 lfSpecular4;

        if ( lfSlope1 < 1.0 - lfThreshold )
        {
            half3  lNormal1 = half3( 0.0, 0.0, 0.0 );
            half3  lNormal2 = half3( 0.0, 0.0, 0.0 );

            half lfHeightA = lfDefaultHeight;
            half lfHeightB = lfDefaultHeight;
            half lfDisplacementA = lfDefaultHeight;
            half lfDisplacementB = lfDefaultHeight;
            half2 lfSpecular1 = half2( 0.0, 0.0 );
            half2 lfSpecular2 = half2( 0.0, 0.0 );

            int liIdX = int( UnpackItemXChannel( lTile1UInt ) );
            bool lbReadLHS = lfPatch < 1.0 - lfThreshold;

            if ( lbReadLHS )
            {
                lNormal1 = GetTriPlanarNormalArray(
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    liIdX,
                    lfHeightA,
                    lfDisplacementA,
                    lfSpecular1 );
            }

            bool lbReadRHS = ( lfPatch > lfThreshold );

            if ( lbReadRHS )
            {
                int liIdY = int( UnpackItemYChannel( lTile1UInt ) );
                bool lbCopyLHS = ( liIdX == liIdY ) && lbReadLHS;

                if ( !lbCopyLHS )
                {
                    lNormal2 = GetTriPlanarNormalArray(
                        lCustomUniforms,
                        lTileCoordsVec2,
                        SAMPLER2DARRAYPARAM( lNormalMap ),
                        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                        liIdY,
                        lfHeightB,
                        lfDisplacementB,
                        lfSpecular2 );
                }
                else
                {
                    lNormal2 = lNormal1;
                    lfHeightB = lfHeightA;
                    lfDisplacementB = lfDisplacementA;
                    lfSpecular2 = lfSpecular1;
                }
            }

            half lfHeight = half( 0.0 );
            {
                half lfPreScaledHeight;

                // use height as a cutoff for smoothstep to get sharp transition between height maps
                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = half( saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 ) );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );

                half lfHeight1 = UnpackG8( luPackedHeights );
                lfHeight1 += lfPreScaledHeight * lfScale;
                PackG8( luPackedHeights, lfHeight1 );
            }

            lfSpecular3 = mix( lfSpecular1, lfSpecular2, lfHeight );
            lNormal3 = mix( lNormal1, lNormal2, lfHeight );

            // blend between height maps again to get map which can be combined with other height maps
            lfHeightC = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementC = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
            lfHeightC = lfDefaultHeight;
            lNormal3 = vec3( 0.0, 0.0, 0.0 );
            lfDisplacementC = lfDefaultHeight;
            lfSpecular3 = half2( 0.0, 0.0 );
        }

        if ( lfSlope1 > lfThreshold )
        {
            half3  lNormal1 = half3( 0.0, 0.0, 0.0 );
            half3  lNormal2 = half3( 0.0, 0.0, 0.0 );

            half lfHeightA = lfDefaultHeight;
            half lfHeightB = lfDefaultHeight;
            half lfDisplacementA = lfDefaultHeight;
            half lfDisplacementB = lfDefaultHeight;
            half2 lfSpecular1 = half2( 0.0, 0.0 );
            half2 lfSpecular2 = half2( 0.0, 0.0 );

            int liIdZ = int( UnpackItemZChannel( lTile1UInt ) );
            bool lbReadLHS = lfPatch < 1.0 - lfThreshold;

            if ( lbReadLHS )
            {
                lNormal1 = GetTriPlanarNormalArray(
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    liIdZ,
                    lfHeightA,
                    lfDisplacementA,
                    lfSpecular1 );
            }

            bool lbReadRHS = lfPatch > lfThreshold;

            if ( lbReadRHS )
            {
                int liIdW = int( UnpackItemWChannel( lTile1UInt ) );
                bool lbCopyLHS = ( liIdZ == liIdW ) && lbReadLHS;

                if ( !lbCopyLHS )
                {
                    lNormal2 = GetTriPlanarNormalArray(
                        lCustomUniforms,
                        lTileCoordsVec2,
                        SAMPLER2DARRAYPARAM( lNormalMap ),
                        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                        liIdW,
                        lfHeightB,
                        lfDisplacementB,
                        lfSpecular2 );
                }
                else
                {
                    lNormal2 = lNormal1;
                    lfHeightB = lfHeightA;
                    lfDisplacementB = lfDisplacementA;
                    lfSpecular2 = lfSpecular1;
                }
            }

            half lfHeight = half( 0.0 );
            {
                half lfPreScaledHeight;

                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = half( saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 ) );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );

                half lfHeight2 = UnpackB8( luPackedHeights );

                lfHeight2 += lfPreScaledHeight * lfScale;
                PackB8( luPackedHeights, lfHeight2 );
            }

            lfSpecular4 = mix( lfSpecular1, lfSpecular2, lfHeight );
            lNormal4 = mix( lNormal1, lNormal2, lfHeight );
            lfHeightD = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementD = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
            lNormal4 = vec3( 0.0, 0.0, 0.0 );
            lfHeightD = lfDefaultHeight;
            lfDisplacementD = lfDefaultHeight;
            lfSpecular4 = half2( 0.0, 0.0 );
        }

        half lfHeight = half( 0.0 );
        {
            half lfPreScaledHeight;

            lfPreScaledHeight = lfHeightC - lfHeightD * lfSlope1 - lfHeightC * lfSlope1 + lfSlope1;
            lfHeight = half( saturate( ( lfOffset - lfPreScaledHeight + lfSlope1 ) * lfInverseOffset2 ) );
            lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );

            half lfHeight5 = UnpackA8( luPackedHeights );

            lfHeight5 += lfPreScaledHeight * lfScale;
            PackA8( luPackedHeights, lfHeight5 );
        }

        lfSpecular5 = mix( lfSpecular3, lfSpecular4, lfHeight );
        lNormal5 = mix( lNormal3, lNormal4, lfHeight );
        lfHeightE = mix( lfHeightC, lfHeightD, lfSlope1 );
        lfDisplacementE = mix( lfDisplacementC, lfDisplacementD, lfHeight );
    }

    lfDisplacement += lfDisplacementE * lfScale;
    lfSpecular += lfSpecular5.x * lfScale;

#if defined( D_NO_Z_NORM )
    lNormal5.z = sqrt( saturate( 1.0 - lNormal5.x*lNormal5.x - lNormal5.y*lNormal5.y ) );
#endif

    return lNormal5;
}
#endif

//-----------------------------------------------------------------------------
///
///     GetBlendedNormalSingleTile
///
///     @brief      GetBlendedNormalSingleTile
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if defined ( D_FORCE_HALF )
half3
#else
vec3
#endif
GetBlendedNormalSingleTile(
    in  CustomPerMaterialUniforms lCustomUniforms,
    #if defined ( D_FORCE_HALF_UV )
    in    half2        lTileCoordsVec2,   
    #else
    in    vec2         lTileCoordsVec2,
    #endif
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    in    uint         lTile1UInt,
    #if defined ( D_FORCE_HALF )
    in    half        lfPatch,
    in    half        lfSlope1,
    inout   half      lfSpecular,
    inout   half      lfSpecularCoeff,
    inout   uint       luPackedHeights,
    in      half      lfScale )
    #else
    in    float        lfPatch,
    in    float        lfSlope1,
    inout   float      lfSpecular,
    inout   float      lfSpecularCoeff,
    inout   uint       luPackedHeights,
    in      float      lfScale )
    #endif
{
    #if defined ( D_FORCE_HALF )
    const half lfThreshold = 1.0 / 20.0;
    const half lfOffset = 0.1;
    const half lfInverseOffset2 = 1.0 / ( lfOffset + lfOffset );
    const half lfDefaultHeight = 0.5;
    #else
    const float lfThreshold = 1.0 / 20.0;
    const float lfOffset = 0.1;
    const float lfInverseOffset2 = 1.0 / ( lfOffset + lfOffset );
    const float lfDefaultHeight = 0.5;
    #endif

    half3 lNormal5;

    uint luPackedHDS2_E;    
    {
#if defined ( D_PACKED_NORMALS )
        uvec2  lNormal3;
        uvec2  lNormal4;
#else
        half3  lNormal3;
        half3  lNormal4;
#endif

        uint luPackedHDS2_C;
        uint luPackedHDS2_D;

        if ( lfSlope1 < 1.0 - lfThreshold )
        {
#if defined ( D_PACKED_NORMALS )
            uvec2  lNormal1 = uvec2( 0, 0 );
            uvec2  lNormal2 = uvec2( 0, 0 );
#else
            half3  lNormal1 = half3( 0.0, 0.0, 0.0 );
            half3  lNormal2 = half3( 0.0, 0.0, 0.0 );
#endif

            uint luPackedHDS2_A = 0;
            uint luPackedHDS2_B = 0;

            int liIdX = int( UnpackItemXChannel( lTile1UInt ) );
            bool lbReadLHS = lfPatch < 1.0 - lfThreshold;

            if ( lbReadLHS )
            {
                lNormal1 = GetTriPlanarNormalArrayPacked(
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    liIdX,
                    luPackedHDS2_A );

                lNormal2 = lNormal1;
                luPackedHDS2_B = luPackedHDS2_A;
            }
            else
            {
                luPackedHDS2_A = PackToRGBA8( lfDefaultHeight, lfDefaultHeight, 0.0, 0.0 );
            }

            bool lbReadRHS = ( lfPatch > lfThreshold );

            if ( lbReadRHS )
            {
                int liIdY = int( UnpackItemYChannel( lTile1UInt ) );
                bool lbCopyLHS = ( liIdX == liIdY ) && lbReadLHS;

                if ( !lbCopyLHS )
                {
                    lNormal2 = GetTriPlanarNormalArrayPacked(
                        lCustomUniforms,
                        lTileCoordsVec2,
                        SAMPLER2DARRAYPARAM( lNormalMap ),
                        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                        liIdY,
                        luPackedHDS2_B );
                }
                //else
                //{
                //    lNormal2        = lNormal1;
                //    luPackedHDS2_B  = luPackedHDS2_A;
                //}
            }
            else
            {
                luPackedHDS2_B = PackToRGBA8( lfDefaultHeight, lfDefaultHeight, 0.0, 0.0 );
            }

            half lfHeight = half(0.0);
            {
                #if defined ( D_FORCE_HALF )
                half lfHeightA = UnpackR8( luPackedHDS2_A );
                half lfHeightB = UnpackR8( luPackedHDS2_B );
                #else
                float lfHeightA = UnpackR8( luPackedHDS2_A );
                float lfHeightB = UnpackR8( luPackedHDS2_B );
                #endif

#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )                
                #if defined ( D_FORCE_HALF )
                half lfPreScaledHeight;
                #else
                float lfPreScaledHeight;
                #endif

                // use height as a cutoff for smoothstep to get sharp transition between height maps
                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = half ( saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 ) );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
#else
                float lfPreScaledHeight = mix( lfHeightA, 1.0 - lfHeightB, lfPatch );
                lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfPatch );
#endif
                #if defined ( D_FORCE_HALF )
                half lfHeight1;
                #else
                float lfHeight1;
                #endif

                lfHeight1 = UnpackG8( luPackedHeights );
                lfHeight1 += lfPreScaledHeight * lfScale;
                PackG8( luPackedHeights, lfHeight1 );
            }

#if defined ( D_PACKED_NORMALS )

            vec3 lUnpackedNormal1;
            vec3 lUnpackedNormal2;
            vec3 lUnpackedNormal3;

            UnpackNormalsXYZ(
                lNormal1,
                lUnpackedNormal1.x,
                lUnpackedNormal1.y,
                lUnpackedNormal1.z);

            UnpackNormalsXYZ(
                lNormal2,
                lUnpackedNormal2.x,
                lUnpackedNormal2.y,
                lUnpackedNormal2.z);

            lUnpackedNormal3 = mix( lUnpackedNormal1, lUnpackedNormal2, lfHeight );

            lNormal3 = PackNormalsXYZ(
                lUnpackedNormal3.x,
                lUnpackedNormal3.y,
                lUnpackedNormal3.z);
#else
            lNormal3 = mix( lNormal1, lNormal2, lfHeight );
#endif

            luPackedHDS2_C = MixRGBA8Pack( luPackedHDS2_A, luPackedHDS2_B, vec4( lfPatch, lfHeight, lfHeight, lfHeight ) );
        }
        else
        {
#if defined ( D_PACKED_NORMALS )
            lNormal3 = uvec2( 0, 0 );
#else
            lNormal3 = half3( 0.0, 0.0, 0.0 );
#endif

            luPackedHDS2_C = PackToRGBA8( lfDefaultHeight, lfDefaultHeight, 0.0, 0.0 );
        }

        if ( lfSlope1 > lfThreshold )
        {
#if defined ( D_PACKED_NORMALS )
            uvec2  lNormal1 = uvec2( 0, 0 );
            uvec2  lNormal2 = uvec2( 0, 0 );
#else
            half3  lNormal1 = half3( 0.0, 0.0, 0.0 );
            half3  lNormal2 = half3( 0.0, 0.0, 0.0 );
#endif

            uint luPackedHDS2_A = 0;
            uint luPackedHDS2_B = 0;

            int liIdZ = int( UnpackItemZChannel( lTile1UInt ) );
            bool lbReadLHS = lfPatch < 1.0 - lfThreshold;

            if ( lbReadLHS )
            {
                lNormal1 = GetTriPlanarNormalArrayPacked(
                    lCustomUniforms,
                    lTileCoordsVec2,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    liIdZ,
                    luPackedHDS2_A );

                lNormal2 = lNormal1;
                luPackedHDS2_B = luPackedHDS2_A;
            }
            else
            {
                luPackedHDS2_A = PackToRGBA8( lfDefaultHeight, lfDefaultHeight, 0.0, 0.0 );
            }

            bool lbReadRHS = lfPatch > lfThreshold;

            if ( lbReadRHS )
            {
                int liIdW = int( UnpackItemWChannel( lTile1UInt ) );
                bool lbCopyLHS = ( liIdZ == liIdW ) && lbReadLHS;

                if ( !lbCopyLHS )
                {
                    lNormal2 = GetTriPlanarNormalArrayPacked(
                        lCustomUniforms,
                        lTileCoordsVec2,
                        SAMPLER2DARRAYPARAM( lNormalMap ),
                        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                        liIdW,
                        luPackedHDS2_B );
                }
                //else
                //{
                //    lNormal2        = lNormal1;
                //    luPackedHDS2_B  = luPackedHDS2_A;
                //}
            }
            else
            {
                luPackedHDS2_B = PackToRGBA8( lfDefaultHeight, lfDefaultHeight, 0.0, 0.0 );
            }

            half lfHeight = half(0.0);
            {
                #if defined ( D_FORCE_HALF )
                half lfHeightA = UnpackR8( luPackedHDS2_A );
                half lfHeightB = UnpackR8( luPackedHDS2_B );
                #else
                float lfHeightA = UnpackR8( luPackedHDS2_A );
                float lfHeightB = UnpackR8( luPackedHDS2_B );
                #endif

#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )

                #if defined ( D_FORCE_HALF )
                half lfPreScaledHeight;
                #else
                float lfPreScaledHeight;
                #endif

                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = half( saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 ) );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
#else
                float lfPreScaledHeight = mix( lfHeightA, 1.0 - lfHeightB, lfPatch );
                lfHeight = FastSmoothStep(lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfPatch);
#endif
                #if defined ( D_FORCE_HALF )
                half lfHeight2 = UnpackB8( luPackedHeights );
                #else
                float lfHeight2 = UnpackB8( luPackedHeights );
                #endif
                
                lfHeight2 += lfPreScaledHeight * lfScale;
                PackB8( luPackedHeights, lfHeight2 );
            }

#if defined ( D_PACKED_NORMALS )

            vec3 lUnpackedNormal1;
            vec3 lUnpackedNormal2;
            vec3 lUnpackedNormal4;

            UnpackNormalsXYZ(
                lNormal1,
                lUnpackedNormal1.x,
                lUnpackedNormal1.y,
                lUnpackedNormal1.z );

            UnpackNormalsXYZ(
                lNormal2,
                lUnpackedNormal2.x,
                lUnpackedNormal2.y,
                lUnpackedNormal2.z );

            lUnpackedNormal4 = mix( lUnpackedNormal1, lUnpackedNormal2, lfHeight );

            lNormal4 = PackNormalsXYZ(
                lUnpackedNormal4.x,
                lUnpackedNormal4.y,
                lUnpackedNormal4.z );
#else
            lNormal4 = mix( lNormal1, lNormal2, lfHeight );
#endif

            luPackedHDS2_D = MixRGBA8Pack(
                luPackedHDS2_A,
                luPackedHDS2_B,
                #if defined ( D_FORCE_HALF )
                half4( lfPatch, lfHeight, lfHeight, lfHeight ) );
                #else
                vec4(lfPatch, lfHeight, lfHeight, lfHeight ) );
                #endif
        }
        else
        {
#if defined ( D_PACKED_NORMALS )
            lNormal4 = uvec2( 0, 0 );
#else
            lNormal4 = half3( 0.0, 0.0, 0.0 );
#endif

            luPackedHDS2_D = PackToRGBA8( lfDefaultHeight, lfDefaultHeight, 0.0, 0.0 );
        }

        half lfHeight = half(0.0);
        {
            #if defined ( D_FORCE_HALF )
            half lfHeightC = UnpackR8( luPackedHDS2_C );
            half lfHeightD = UnpackR8( luPackedHDS2_D );
            #else
            float lfHeightC = UnpackR8( luPackedHDS2_C );
            float lfHeightD = UnpackR8( luPackedHDS2_D );
            #endif

#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
            #if defined ( D_FORCE_HALF )
            half lfPreScaledHeight;
            #else
            float lfPreScaledHeight;
            #endif

            lfPreScaledHeight = lfHeightC - lfHeightD * lfSlope1 - lfHeightC * lfSlope1 + lfSlope1;
            lfHeight = half( saturate( ( lfOffset - lfPreScaledHeight + lfSlope1 ) * lfInverseOffset2 ) );
            lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
#else
            float lfPreScaledHeight = mix( lfHeightC, 1.0 - lfHeightD, lfSlope1 );
            lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfSlope1 );
#endif
            #if defined ( D_FORCE_HALF )
            half lfHeight5 = UnpackA8( luPackedHeights );
            #else
            float lfHeight5 = UnpackA8( luPackedHeights );
            #endif
            
            lfHeight5 += lfPreScaledHeight * lfScale;
            PackA8( luPackedHeights, lfHeight5 );
        }

#if defined ( D_PACKED_NORMALS )

        vec3 lUnpackedNormal3;
        vec3 lUnpackedNormal4;

        UnpackNormalsXYZ(
            lNormal3,
            lUnpackedNormal3.x,
            lUnpackedNormal3.y,
            lUnpackedNormal3.z );

        UnpackNormalsXYZ(
            lNormal4,
            lUnpackedNormal4.x,
            lUnpackedNormal4.y,
            lUnpackedNormal4.z );

        lNormal5 = half3( mix( lUnpackedNormal3, lUnpackedNormal4, lfHeight ) );
#else
        lNormal5 = mix( lNormal3, lNormal4, lfHeight );
#endif

        luPackedHDS2_E = MixRGBA8Pack(
            luPackedHDS2_C,
            luPackedHDS2_D,
            #if defined ( D_FORCE_HALF )
            half4( lfSlope1, lfHeight, lfHeight, lfHeight ) );
            #else 
            vec4(lfSlope1, lfHeight, lfHeight, lfHeight ) );
            #endif
    }

    //#if defined( D_NORMALBLEND )
    //    if ( IN_SCREEN_POSITION.x > 1024 )
    //    {
    //        lSpecular5.y = 0.0;
    //    }
    //#endif

    #if defined ( D_FORCE_HALF )
    half lfDisplacement;
    #else
    float lfDisplacement;
    #endif

    lfDisplacement = UnpackR8( luPackedHeights );
    lfDisplacement      += UnpackG8( luPackedHDS2_E  ) * lfScale;
    PackR8( luPackedHeights, lfDisplacement );

    lfSpecular          += UnpackB8( luPackedHDS2_E ) * lfScale;
    lfSpecularCoeff      = UnpackA8( luPackedHDS2_E );

#if defined( D_NO_Z_NORM )
    lNormal5.z = sqrt( saturate( 1.0 - lNormal5.x*lNormal5.x - lNormal5.y*lNormal5.y ) );
#endif

    return vec3(lNormal5);
}



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
half3
GetBlendedNormal(
    in  CustomPerMaterialUniforms lCustomUniforms,
#if defined ( D_FORCE_HALF_UV )
    in    half2        lTileCoordsVec2,
#else
    in    vec2         lTileCoordsVec2,
#endif
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    in    uint         lTile1UInt,
    in    uint         lTile2UInt,
    #if defined ( D_FORCE_HALF )
    in    half         lfPatch,
    in    half         lfSlope1,
    in    half         lfSlope2,
    in    half         lfTileType,
    #else
    in    float        lfPatch,
    in    float        lfSlope1,
    in    float        lfSlope2,
    in    float        lfTileType,
    #endif
#if defined (D_STOCHASTIC_TERRAIN)
    #if defined ( D_FORCE_HALF )
    inout   half      lfSpecular,
    inout   half      lfSpecularCoeff,
    inout   half      lfDisplacement,
    inout   half      lfHeight1,
    inout   half      lfHeight2,
    inout   half      lfHeight3,
    inout   half      lfHeight4,
    inout   half      lfHeight5,
    inout   half      lfHeight6,
    inout   half      lfHeight7,
    in    half        lfScale
    #else
    inout   float      lfSpecular,
    inout   float      lfSpecularCoeff,
    inout   float      lfDisplacement,
    inout   float      lfHeight1,
    inout   float      lfHeight2,
    inout   float      lfHeight3,
    inout   float      lfHeight4,
    inout   float      lfHeight5,
    inout   float      lfHeight6,
    inout   float      lfHeight7,
    in    float        lfScale
    #endif
#else
    out   float         lfSpecular,
    out   float         lfSpecularCoeff,
    out   float        lfDisplacement,
    #if defined (D_PACKED_HEIGHTS)
    out   uint         lfHeight1,
    out   uint         lfHeight2,
    out   uint         lfHeight3,
    out   uint         lfHeight4,
    out   uint         lfHeight5,
    out   uint         lfHeight6,
    out   uint         lfHeight7,
    out   uint         lfHeight8
    #else
    out   half         lfHeight1,
    out   half         lfHeight2,
    out   half         lfHeight3,
    out   half         lfHeight4,
    out   half         lfHeight5,
    out   half         lfHeight6,
    out   half         lfHeight7,
    out   half         lfHeight8
    #endif 
#endif
)
{

#if defined (D_STOCHASTIC_TERRAIN)
#if defined ( D_FORCE_HALF )
    half      lfHeight8 = 0.0;
#else
    float      lfHeight8 = 0.0;
#endif
#endif

#ifndef D_LOW_QUALITY

#if defined ( D_FORCE_HALF )
    const half lfThreshold = 1.0 / 20.0;
    const half lfOffset = 0.1;
    const half lfInverseOffset2 = 1.0 / ( lfOffset + lfOffset );
    const half lfDefaultHeight = 0.5;
#else
    const float lfThreshold = 1.0 / 20.0;
    const float lfOffset = 0.1;
    const float lfInverseOffset2 = 1.0 / ( lfOffset + lfOffset );
    const float lfDefaultHeight = 0.5;
#endif

#if defined ( D_PACKED_NORMALS )
    uvec2  lNormal5;
    uvec2  lNormal6;
#else
    half3  lNormal5;
    half3  lNormal6;
#endif

#if defined ( D_PACKED_SPECULAR )
    uint lSpecular5;
    uint lSpecular6;
#else
    half2 lSpecular5;
    half2 lSpecular6;
#endif

#if defined ( D_FORCE_HALF )
    half lfHeightE;
    half lfHeightF;
    half lfHeightNoise;
    half lfDisplacementE;
    half lfDisplacementF;
    half lfDisplacementNoise;
#else
    float lfHeightE;
    float lfHeightF;
    float lfHeightNoise;
    float lfDisplacementE;
    float lfDisplacementF;
    float lfDisplacementNoise;
#endif

#if !defined (D_STOCHASTIC_TERRAIN)
    #if defined (D_PACKED_HEIGHTS)
    lfHeight1 = 0;
    lfHeight2 = 0;
    lfHeight3 = 0;
    lfHeight4 = 0;
    lfHeight5 = 0;
    lfHeight6 = 0;
    lfHeight7 = 0;
    lfHeight8 = 0;
    #else
    lfHeight1 = 0.0;
    lfHeight2 = 0.0;
    lfHeight3 = 0.0;
    lfHeight4 = 0.0;
    lfHeight5 = 0.0;
    lfHeight6 = 0.0;
    lfHeight7 = 0.0;
    lfHeight8 = 0.0;
    #endif
#endif

    if ( lfTileType < 1.0 - lfThreshold )
    {
#if defined ( D_PACKED_NORMALS )
        uvec2  lNormal3;
        uvec2  lNormal4;
#else
        half3  lNormal3;
        half3  lNormal4;
#endif

#if defined ( D_FORCE_HALF )
        half lfHeightC;
        half lfHeightD;
        half lfDisplacementC;
        half lfDisplacementD;
#else
        float lfHeightC;
        float lfHeightD;
        float lfDisplacementC;
        float lfDisplacementD;
#endif

        #if defined ( D_PACKED_SPECULAR )
        uint lSpecular3;
        uint lSpecular4;
        #else
        half2 lSpecular3;
        half2 lSpecular4;
        #endif

        if ( lfSlope1 < 1.0 - lfThreshold )
        {
#if defined ( D_PACKED_NORMALS )
            uvec2  lNormal1;
            uvec2  lNormal2;
#else
            half3  lNormal1;
            half3  lNormal2;
#endif

            #if defined ( D_FORCE_HALF )
            half lfHeightA;
            half lfHeightB;
            half lfDisplacementA;
            half lfDisplacementB;
            #else
            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;
            #endif

            #if defined ( D_PACKED_SPECULAR )
            uint lSpecular1;
            uint lSpecular2;
            #else
            half2 lSpecular1;
            half2 lSpecular2;
            #endif

            if ( lfPatch < 1.0 - lfThreshold )
            {
                lNormal1 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal1 = uvec2( 0, 0 );
#else
                lNormal1 = half3( 0.0, 0.0, 0.0 );
#endif

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular1 = 0;
                #else
                lSpecular1 = half2( 0.0, 0.0 );
                #endif
            }

            if ( lfPatch > lfThreshold )
            {
                lNormal2 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal2 = uvec2( 0, 0 );
#else
                lNormal2 = half3( 0.0, 0.0, 0.0 );
#endif

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular2 = 0;
                #else
                lSpecular2 = half2( 0.0, 0.0 );
                #endif
            }

#if defined (D_STOCHASTIC_TERRAIN)
            #if defined ( D_FORCE_HALF )
            half lfHeight = 0.0;
            #else
            float lfHeight = 0.0;
            #endif
            {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )

                #if defined ( D_FORCE_HALF )
                half lfPreScaledHeight;
                #else
                float lfPreScaledHeight;
                #endif

                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
                lfHeight1 += lfPreScaledHeight * lfScale;
#else
                float lfPreScaledHeight = mix( lfHeightA, 1.0 - lfHeightB, lfPatch );
                lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfPatch );
                lfHeight1 += lfPreScaledHeight * lfScale;
#endif
            }
#else
            // blend between height textures
            #if defined (D_PACKED_HEIGHTS)
            float lfTempHeight = half(mix(lfHeightA, 1.0 - lfHeightB, lfPatch));
            lfHeight1 = PackHeightsAB(lfTempHeight, lfTempHeight);
            // use height as a cutoff for smoothstep to get sharp transition between height maps
            half lfHeight = half(FastSmoothStep(lfTempHeight - lfOffset, 2.0f * lfOffset, lfPatch));
            #else
            lfHeight1 = half(mix(lfHeightA, 1.0 - lfHeightB, lfPatch));
            half lfHeight = half(FastSmoothStep(lfHeight1 - lfOffset, 2.0f * lfOffset, lfPatch));
            #endif
#endif // D_STOCHASTIC_TERRAIN.

#if defined ( D_PACKED_NORMALS )

            vec3 lUnpackedNormal1;
            vec3 lUnpackedNormal2;
            vec3 lUnpackedNormal3;

            UnpackNormalsXYZ(
                lNormal1,
                lUnpackedNormal1.x,
                lUnpackedNormal1.y,
                lUnpackedNormal1.z);

            UnpackNormalsXYZ(
                lNormal2,
                lUnpackedNormal2.x,
                lUnpackedNormal2.y,
                lUnpackedNormal2.z);

            lUnpackedNormal3 = mix( lUnpackedNormal1, lUnpackedNormal2, lfHeight );

            lNormal3 = PackNormalsXYZ(
                lUnpackedNormal3.x,
                lUnpackedNormal3.y,
                lUnpackedNormal3.z);
#else
            lNormal3 = half3( mix( lNormal1, lNormal2, lfHeight ) );
#endif

            #if defined ( D_PACKED_SPECULAR )
            half2 lTempSpecularA;
            half2 lTempSpecularB;
            
            UnpackSpecular( lSpecular1, lTempSpecularA.x, lTempSpecularA.y );
            UnpackSpecular( lSpecular2, lTempSpecularB.x, lTempSpecularB.y );
            
            lTempSpecularA = half2( mix( lTempSpecularA, lTempSpecularB, lfHeight ) );

            lSpecular3 = PackSpecular( lTempSpecularA.x, lTempSpecularA.y );
            #else
            lSpecular3 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );
            #endif

            // blend between height maps again to get map which can be combined with other height maps
            lfHeightC = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementC = mix( lfDisplacementA, lfDisplacementB, float(lfHeight) );
        }
        else
        {
#if defined ( D_PACKED_NORMALS )
            lNormal3 = uvec2(0, 0);
#else
            lNormal3 = half3(0.0, 0.0, 0.0);
#endif

            lfHeightC = lfDefaultHeight;
            lfDisplacementC = lfDefaultHeight;

            #if defined ( D_PACKED_SPECULAR )
            lSpecular3 = 0;
            #else
            lSpecular3 = half2( 0.0, 0.0 );
            #endif
        }

        if ( lfSlope1 > lfThreshold )
        {
#if defined ( D_PACKED_NORMALS )
            uvec2  lNormal1;
            uvec2  lNormal2;
#else
            half3  lNormal1;
            half3  lNormal2;
#endif

            #if defined ( D_FORCE_HALF )
            half lfHeightA;
            half lfHeightB;
            half lfDisplacementA;
            half lfDisplacementB;
            #else
            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;
            #endif

            #if defined ( D_PACKED_SPECULAR )
            uint lSpecular1;
            uint lSpecular2;
            #else
            half2 lSpecular1;
            half2 lSpecular2;
            #endif

            if ( lfPatch < 1.0 - lfThreshold )
            {
                lNormal1 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal1 = uvec2( 0, 0 );
#else
                lNormal1 = half3( 0.0, 0.0, 0.0 );
#endif

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular1 = 0;
                #else
                lSpecular1 = half2( 0.0, 0.0 );
                #endif
            }

            if ( lfPatch > lfThreshold )
            {
                lNormal2 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal2 = uvec2(0, 0);
#else
                lNormal2 = half3(0.0, 0.0, 0.0);
#endif

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular2 = 0;
                #else
                lSpecular2 = half2( 0.0, 0.0 );
                #endif
            }

#if defined (D_STOCHASTIC_TERRAIN)
            #if defined ( D_FORCE_HALF )
            half lfHeight = 0.0;
            #else
            float lfHeight = 0.0;
            #endif
            {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
                #if defined ( D_FORCE_HALF )
                half lfPreScaledHeight;
                #else
                float lfPreScaledHeight;
                #endif

                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
                lfHeight2 += lfPreScaledHeight * lfScale;
#else
                float lfPreScaledHeight = mix( lfHeightA, 1.0 - lfHeightB, lfPatch );
                lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfPatch );
                lfHeight2 += lfPreScaledHeight  * lfScale;
#endif
            }
#else
            #if defined (D_PACKED_HEIGHTS)
            float lfTempHeight = half(mix(lfHeightA, 1.0 - lfHeightB, lfPatch));
            lfHeight2 = PackHeightsAB(lfTempHeight, lfTempHeight);
            float lfHeight = FastSmoothStep(lfTempHeight - lfOffset, 2.0f * lfOffset, lfPatch);
            #else
            lfHeight2 = half(mix(lfHeightA, 1.0 - lfHeightB, lfPatch));
            float lfHeight = FastSmoothStep(lfHeight2 - lfOffset, 2.0f * lfOffset, lfPatch);
            #endif
#endif // D_STOCHASTIC_TERRAIN

#if defined ( D_PACKED_NORMALS )

            vec3 lUnpackedNormal1;
            vec3 lUnpackedNormal2;
            vec3 lUnpackedNormal4;

            UnpackNormalsXYZ(
                lNormal1,
                lUnpackedNormal1.x,
                lUnpackedNormal1.y,
                lUnpackedNormal1.z );

            UnpackNormalsXYZ(
                lNormal2,
                lUnpackedNormal2.x,
                lUnpackedNormal2.y,
                lUnpackedNormal2.z );

            lUnpackedNormal4 = mix( lUnpackedNormal1, lUnpackedNormal2, lfHeight );

            lNormal4 = PackNormalsXYZ(
                lUnpackedNormal4.x,
                lUnpackedNormal4.y,
                lUnpackedNormal4.z );
#else
            lNormal4 = half3( mix( lNormal1, lNormal2, lfHeight ) );
#endif

            #if defined ( D_PACKED_SPECULAR )
            half2 lTempSpecularA;
            half2 lTempSpecularB;

            UnpackSpecular( lSpecular1, lTempSpecularA.x, lTempSpecularA.y );
            UnpackSpecular( lSpecular2, lTempSpecularB.x, lTempSpecularB.y );

            lTempSpecularA = half2( mix( lTempSpecularA, lTempSpecularB, lfHeight ) );

            lSpecular4 = PackSpecular( lTempSpecularA.x, lTempSpecularA.y );
            #else
            lSpecular4 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );
            #endif
            
            lfHeightD = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementD = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
#if defined ( D_PACKED_NORMALS )
            lNormal4 = uvec2( 0, 0 );
#else
            lNormal4 = half3( 0.0, 0.0, 0.0 );
#endif

            lfHeightD = lfDefaultHeight;
            lfDisplacementD = lfDefaultHeight;

            #if defined ( D_PACKED_SPECULAR )
            lSpecular4 = 0;
            #else 
            lSpecular4 = half2( 0.0, 0.0 );
            #endif
        }

#if defined (D_STOCHASTIC_TERRAIN)
        #if defined ( D_FORCE_HALF )
        half lfHeight = 0.0;
        #else
        float lfHeight = 0.0;
        #endif
        {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
            #if defined ( D_FORCE_HALF )
            half lfPreScaledHeight;
            #else
            float lfPreScaledHeight;
            #endif

            lfPreScaledHeight = lfHeightC - lfHeightD * lfSlope1 - lfHeightC * lfSlope1 + lfSlope1;
            lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfSlope1 ) * lfInverseOffset2 );
            lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
            lfHeight5 += lfPreScaledHeight * lfScale;
#else
            float lfPreScaledHeight = mix( lfHeightC, 1.0 - lfHeightD, lfSlope1 );
            lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfSlope1 );
            lfHeight5 += lfPreScaledHeight * lfScale;
#endif
        }
#else
        #if defined ( D_PACKED_HEIGHTS )
        float lfTempHeight = half( mix( lfHeightC, 1.0 - lfHeightD, lfSlope1 ) );
        lfHeight5 = PackHeightsAB( lfTempHeight, lfTempHeight );
        float lfHeight = FastSmoothStep(lfTempHeight - lfOffset, 2.0f * lfOffset, lfSlope1);
        #else
        lfHeight5 = half( mix ( lfHeightC, 1.0 - lfHeightD, lfSlope1 ) );
        float lfHeight = FastSmoothStep(lfHeight5 - lfOffset, 2.0f * lfOffset, lfSlope1);
        #endif
#endif // D_STOCHASTIC_TERRAIN

#if defined ( D_PACKED_NORMALS )

        vec3 lUnpackedNormal3;
        vec3 lUnpackedNormal4;
        vec3 lUnpackedNormal5;

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

        lUnpackedNormal5 = mix( lUnpackedNormal3, lUnpackedNormal4, lfHeight );

        lNormal5 = PackNormalsXYZ(
            lUnpackedNormal5.x,
            lUnpackedNormal5.y,
            lUnpackedNormal5.z);
#else
        lNormal5 = half3( mix( lNormal3, lNormal4, lfHeight ) );
#endif

        #if defined ( D_PACKED_SPECULAR )
        half2 lTempSpecularA;
        half2 lTempSpecularB;

        UnpackSpecular(lSpecular3, lTempSpecularA.x, lTempSpecularA.y);
        UnpackSpecular(lSpecular4, lTempSpecularB.x, lTempSpecularB.y);

        lTempSpecularA = half2(mix(lTempSpecularA, lTempSpecularB, lfHeight));

        lSpecular5 = PackSpecular(lTempSpecularA.x, lTempSpecularA.y);
        #else
        lSpecular5 = half2( mix( lSpecular3, lSpecular4, lfHeight ) );
        #endif
        
        lfHeightE = mix( lfHeightC, lfHeightD, lfSlope1 );
        lfDisplacementE = mix( lfDisplacementC, lfDisplacementD, lfHeight );
    }
    else
    {
#if defined ( D_PACKED_NORMALS )
        lNormal5 = uvec2( 0, 0 );
#else
        lNormal5 = half3( 0.0, 0.0, 0.0 );
#endif

        #if defined ( D_PACKED_SPECULAR )
        lSpecular5 = 0;
        #else
        lSpecular5 = half2( 0.0, 0.0 );
        #endif

        lfHeightE = lfDefaultHeight;
        lfDisplacementE = lfDefaultHeight;
    }

    if ( lfTileType > lfThreshold )
    {
#if defined ( D_PACKED_NORMALS )
        uvec2  lNormal3;
        uvec2  lNormal4;
#else
        half3  lNormal3;
        half3  lNormal4;
#endif

        #if defined ( D_FORCE_HALF )
        half lfHeightC;
        half lfHeightD;
        half lfDisplacementC;
        half lfDisplacementD;
        #else
        float lfHeightC;
        float lfHeightD;
        float lfDisplacementC;
        float lfDisplacementD;
        #endif

        #if defined ( D_PACKED_SPECULAR )
        uint lSpecular3;
        uint lSpecular4;
        #else
        half2 lSpecular3;
        half2 lSpecular4;
        #endif

        if ( lfSlope2 < 1.0 - lfThreshold )
        {
#if defined ( D_PACKED_NORMALS )
            uvec2  lNormal1;
            uvec2  lNormal2;
#else
            half3  lNormal1;
            half3  lNormal2;
#endif

            #if defined ( D_FORCE_HALF )
            half lfHeightA;
            half lfHeightB;
            half lfDisplacementA;
            half lfDisplacementB;
            #else
            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;
            #endif

            #if defined ( D_PACKED_SPECULAR )
            uint lSpecular1;
            uint lSpecular2;
            #else
            half2 lSpecular1;
            half2 lSpecular2;
            #endif

            if ( lfPatch < 1.0 - lfThreshold )
            {
                lNormal1 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal1 = uvec2(0, 0);
#else
                lNormal1 = half3(0.0, 0.0, 0.0);
#endif

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular1 = 0;
                #else
                lSpecular1 = half2( 0.0, 0.0 );
                #endif
            }

            if ( lfPatch > lfThreshold )
            {
                lNormal2 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal2 = uvec2(0, 0);
#else
                lNormal2 = half3(0.0, 0.0, 0.0);
#endif

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular2 = 0;
                #else
                lSpecular2 = half2( 0.0, 0.0 );
                #endif
            }

#if defined (D_STOCHASTIC_TERRAIN)
            #if defined ( D_FORCE_HALF )
            half lfHeight = 0.0;
            #else
            float lfHeight = 0.0;
            #endif
            {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
                #if defined ( D_FORCE_HALF )
                half lfPreScaledHeight;
                #else
                float lfPreScaledHeight;
                #endif

                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
                lfHeight3 += lfPreScaledHeight * lfScale;
#else
                float lfPreScaledHeight = mix( lfHeightA, 1.0 - lfHeightB, lfPatch );
                lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfPatch );
                lfHeight3 += lfPreScaledHeight * lfScale;
#endif
            }
#else
            #if defined (D_PACKED_HEIGHTS)
            float lfTempHeight = half( mix( lfHeightA, 1.0 - lfHeightB, lfPatch ) );
            lfHeight3 = PackHeightsAB( lfTempHeight, lfTempHeight );
            float lfHeight = FastSmoothStep(lfTempHeight - lfOffset, 2.0f * lfOffset, lfPatch);
            #else
            lfHeight3 = half( mix( lfHeightA, 1.0 - lfHeightB, lfPatch ) );
            float lfHeight = FastSmoothStep(lfHeight3 - lfOffset, 2.0f * lfOffset, lfPatch);
            #endif
#endif // D_STOCHASTIC_TERRAIN

#if defined ( D_PACKED_NORMALS )

            vec3 lUnpackedNormal1;
            vec3 lUnpackedNormal2;
            vec3 lUnpackedNormal3;

            UnpackNormalsXYZ(
                lNormal1,
                lUnpackedNormal1.x,
                lUnpackedNormal1.y,
                lUnpackedNormal1.z);

            UnpackNormalsXYZ(
                lNormal2,
                lUnpackedNormal2.x,
                lUnpackedNormal2.y,
                lUnpackedNormal2.z);

            lUnpackedNormal3 = mix( lUnpackedNormal1, lUnpackedNormal2, lfHeight );

            lNormal3 = PackNormalsXYZ(
                lUnpackedNormal3.x,
                lUnpackedNormal3.y,
                lUnpackedNormal3.z);
#else
            lNormal3 = half3( mix( lNormal1, lNormal2, lfHeight ) );
#endif

            #if defined ( D_PACKED_SPECULAR )
            half2 lTempSpecularA;
            half2 lTempSpecularB;

            UnpackSpecular(lSpecular1, lTempSpecularA.x, lTempSpecularA.y);
            UnpackSpecular(lSpecular2, lTempSpecularB.x, lTempSpecularB.y);

            lTempSpecularA = half2(mix(lTempSpecularA, lTempSpecularB, lfHeight));

            lSpecular3 = PackSpecular(lTempSpecularA.x, lTempSpecularA.y);
            #else
            lSpecular3 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );
            #endif
            
            lfHeightC = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementC = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
#if defined ( D_PACKED_NORMALS )
            lNormal3 = uvec2( 0, 0 );
#else
            lNormal3 = half3( 0.0, 0.0, 0.0 );
#endif

            lfHeightC = lfDefaultHeight;
            lfDisplacementC = lfDefaultHeight;

            #if defined ( D_PACKED_SPECULAR )
            lSpecular3 = 0;
            #else
            lSpecular3 = half2( 0.0, 0.0 );
            #endif
        }

        if ( lfSlope2 > lfThreshold )
        {
#if defined ( D_PACKED_NORMALS )
            uvec2  lNormal1;
            uvec2  lNormal2;
#else
            half3  lNormal1;
            half3  lNormal2;
#endif

            #if defined ( D_FORCE_HALF )
            half lfHeightA;
            half lfHeightB;
            half lfDisplacementA;
            half lfDisplacementB;
            #else
            float lfHeightA;
            float lfHeightB;
            float lfDisplacementA;
            float lfDisplacementB;
            #endif

            #if defined ( D_PACKED_SPECULAR )
            uint lSpecular1;
            uint lSpecular2;
            #else
            half2 lSpecular1;
            half2 lSpecular2;
            #endif

            if ( lfPatch < 1.0 - lfThreshold )
            {
                lNormal1 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal1 = uvec2(0, 0);
#else
                lNormal1 = half3(0.0, 0.0, 0.0);
#endif

                lfHeightA = lfDefaultHeight;
                lfDisplacementA = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular1 = 0;
                #else
                lSpecular1 = half2( 0.0, 0.0 );
                #endif
            }

            if ( lfPatch > lfThreshold )
            {
                lNormal2 = GetTriPlanarNormalArray(
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
#if defined ( D_PACKED_NORMALS )
                lNormal2 = uvec2(0, 0);
#else
                lNormal2 = half3(0.0, 0.0, 0.0);
#endif

                lfHeightB = lfDefaultHeight;
                lfDisplacementB = lfDefaultHeight;

                #if defined ( D_PACKED_SPECULAR )
                lSpecular2 = 0;
                #else
                lSpecular2 = half2( 0.0, 0.0 );
                #endif
            }

#if defined (D_STOCHASTIC_TERRAIN)
            #if defined ( D_FORCE_HALF )
            half lfHeight = 0.0;
            #else
            float lfHeight = 0.0;
            #endif
            {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
                #if defined ( D_FORCE_HALF )
                half lfPreScaledHeight;
                #else
                float lfPreScaledHeight;
                #endif

                lfPreScaledHeight = lfHeightA - lfHeightB * lfPatch - lfHeightA * lfPatch + lfPatch;
                lfHeight = saturate( ( lfOffset - lfPreScaledHeight + lfPatch ) * lfInverseOffset2 );
                lfHeight = lfHeight * lfHeight*( 3.0 - ( 2.0*lfHeight ) );
                lfHeight4 += lfPreScaledHeight * lfScale;
#else
                float lfPreScaledHeight = mix( lfHeightA, 1.0 - lfHeightB, lfPatch );
                lfHeight = FastSmoothStep( lfPreScaledHeight - lfOffset, 2.0f * lfOffset, lfPatch );
                lfHeight4 += lfPreScaledHeight * lfScale;
#endif
            }
#else
            #if defined (D_PACKED_HEIGHTS)
            float lfTempHeight = half( mix( lfHeightA, 1.0 - lfHeightB, lfPatch ) );
            lfHeight4 = PackHeightsAB( lfTempHeight, lfTempHeight );
            float lfHeight = FastSmoothStep( lfTempHeight - lfOffset, 2.0f * lfOffset, lfPatch );
            #else
            lfHeight4 = half( mix( lfHeightA, 1.0 - lfHeightB, lfPatch ) );
            float lfHeight = FastSmoothStep( lfHeight4 - lfOffset, 2.0f * lfOffset, lfPatch );
            #endif
#endif // D_STOCHASTIC_TERRAIN

#if defined ( D_PACKED_NORMALS )

            vec3 lUnpackedNormal1;
            vec3 lUnpackedNormal2;
            vec3 lUnpackedNormal4;

            UnpackNormalsXYZ(
                lNormal1,
                lUnpackedNormal1.x,
                lUnpackedNormal1.y,
                lUnpackedNormal1.z);

            UnpackNormalsXYZ(
                lNormal2,
                lUnpackedNormal2.x,
                lUnpackedNormal2.y,
                lUnpackedNormal2.z);

            lUnpackedNormal4 = mix( lUnpackedNormal1, lUnpackedNormal2, lfHeight );

            lNormal4 = PackNormalsXYZ(
                lUnpackedNormal4.x,
                lUnpackedNormal4.y,
                lUnpackedNormal4.z);
#else
            lNormal4 = half3( mix( lNormal1, lNormal2, lfHeight ) );
#endif

            #if defined ( D_PACKED_SPECULAR )
            half2 lTempSpecularA;
            half2 lTempSpecularB;

            UnpackSpecular(lSpecular1, lTempSpecularA.x, lTempSpecularA.y);
            UnpackSpecular(lSpecular2, lTempSpecularB.x, lTempSpecularB.y);

            lTempSpecularA = half2(mix(lTempSpecularA, lTempSpecularB, lfHeight));

            lSpecular4 = PackSpecular(lTempSpecularA.x, lTempSpecularA.y);
            #else
            lSpecular4 = half2( mix( lSpecular1, lSpecular2, lfHeight ) );
            #endif

            lfHeightD = mix( lfHeightA, lfHeightB, lfPatch );
            lfDisplacementD = mix( lfDisplacementA, lfDisplacementB, lfHeight );
        }
        else
        {
#if defined ( D_PACKED_NORMALS )
            lNormal4 = uvec2(0, 0);
#else
            lNormal4 = half3(0.0, 0.0, 0.0);
#endif

            lfHeightD = lfDefaultHeight;
            lfDisplacementD = lfDefaultHeight;

            #if defined ( D_PACKED_SPECULAR )
            lSpecular4 = 0;
            #else
            lSpecular4 = half2( 0.0, 0.0 );
            #endif
        }

#if defined (D_STOCHASTIC_TERRAIN)
        #if defined ( D_FORCE_HALF )
        half lfHeight = 0.0;
        #else
        float lfHeight = 0.0;
        #endif
        {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
            #if defined ( D_FORCE_HALF )
            half lfPreScaledHeight;
            #else
            float lfPreScaledHeight;
            #endif

            lfPreScaledHeight = lfHeightC - lfHeightD * lfSlope2 - lfHeightC * lfSlope2 + lfSlope2;
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
#endif // D_STOCHASTIC_TERRAIN

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
    #if defined ( D_FORCE_HALF )
    half lfHeight = 0.0;
    #else
    float lfHeight = 0.0;
    #endif
    {
#if defined ( D_USE_FAST_SMOOTH_INTERPOLATION )
        #if defined ( D_FORCE_HALF )
        half lfPreScaledHeight;
        #else
        float lfPreScaledHeight;
        #endif

        lfPreScaledHeight = lfHeightE - lfHeightF * lfTileType - lfHeightE * lfTileType + lfTileType;
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
    #if defined ( D_PACKED_HEIGHTS )
    float lfTempHeight = half( mix( lfHeightE, 1.0 - lfHeightF, lfTileType ));
    lfHeight7 = PackHeightsAB( lfTempHeight, lfTempHeight );
    float lfHeight = FastSmoothStep( lfTempHeight - lfOffset, 2.0f * lfOffset, lfTileType );
    lfTempHeight = half( mix( lfHeightE, lfHeightF, lfTileType ) );
    lfHeight8 = PackHeightsAB( lfTempHeight, lfTempHeight );
    #else
    lfHeight7 = half( mix( lfHeightE, 1.0 - lfHeightF, lfTileType ) );
    float lfHeight = FastSmoothStep( lfHeight7 - lfOffset, 2.0f * lfOffset, lfTileType );
    lfHeight8 = half( mix( lfHeightE, lfHeightF, lfTileType ) );
    #endif

#endif // D_STOCHASTIC_TERRAIN

    #if defined ( D_PACKED_SPECULAR )
    half2 lTempSpecularA;
    half2 lTempSpecularB;

    UnpackSpecular(lSpecular5, lTempSpecularA.x, lTempSpecularA.y);
    UnpackSpecular(lSpecular6, lTempSpecularB.x, lTempSpecularB.y);

    lTempSpecularA = half2(mix(lTempSpecularA, lTempSpecularB, lfHeight));

    uint lSpecular7 = PackSpecular(lTempSpecularA.x, lTempSpecularA.y);
    #else
    half2 lSpecular7 = half2( mix( lSpecular5, lSpecular6, lfHeight ) );
    #endif

#if defined ( D_PACKED_NORMALS )

    vec3 lUnpackedNormal5;
    vec3 lUnpackedNormal6;

    UnpackNormalsXYZ(
        lNormal5,
        lUnpackedNormal5.x,
        lUnpackedNormal5.y,
        lUnpackedNormal5.z);

    UnpackNormalsXYZ(
        lNormal6,
        lUnpackedNormal6.x,
        lUnpackedNormal6.y,
        lUnpackedNormal6.z);

    half3 lNormal = half3( mix( lUnpackedNormal5, lUnpackedNormal6, lfHeight ) );
#else
    half3 lNormal = half3( mix( lNormal5, lNormal6, lfHeight ) );
#endif

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

    return lNormal;

#else

#if defined (D_STOCHASTIC_TERRAIN)
    float lfHeight;
    float lfLocalDisplacement;

    #if defined ( D_PACKED_SPECULAR )
    uint lLocalSpecular;
    #else
    half2 lLocalSpecular;
    #endif

    half3 lNormal = GetTriPlanarNormalArray(
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

    return lNormal;
#else

    float lfHeight;

    #if defined ( D_PACKED_SPECULAR )
    uint lSpecular;
    #else
    half2 lSpecular;
    #endif

    #if defined ( D_PACKED_NORMALS )
    uint2 lNormal = GetTriPlanarNormalArray(
    #else
    half3 lNormal = GetTriPlanarNormalArray(
    #endif
        lCustomUniforms,
        lTileCoordsVec2,
        SAMPLER2DARRAYPARAM( lNormalMap ),
        SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
        int( UnpackItemXChannel( lTile1UInt ) ),
        lfHeight,
        lfDisplacement,
        lSpecular );

    #if defined ( D_PACKED_SPECULAR )
    UnpackSpecular( lSpecular, lfSpecular, lfSpecularCoeff );
    #else
    lfSpecular = lSpecular.x;
    lfSpecularCoeff = lSpecular.y;
    #endif

    #if defined (D_PACKED_HEIGHTS)
    lfHeight1 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight2 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight3 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight4 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight5 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight6 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight7 = PackHeightsAB( lfHeight, lfHeight );
    lfHeight8 = PackHeightsAB( lfHeight, lfHeight );
    #else
    lfHeight1 = half ( lfHeight );
    lfHeight2 = half( lfHeight );
    lfHeight3 = half( lfHeight );
    lfHeight4 = half( lfHeight );
    lfHeight5 = half( lfHeight );
    lfHeight6 = half( lfHeight );
    lfHeight7 = half( lfHeight );
    lfHeight8 = half( lfHeight );
    #endif

    vec3 lUnpackedNormal;

    #if defined ( D_PACKED_NORMALS )
    UnpackNormalsXYZ(
        lNormal,
        lUnpackedNormal.x,
        lUnpackedNormal.y,
        lUnpackedNormal.z);
    #else
    lUnpackedNormal = vec3(lNormal);
    #endif

#if defined( D_NO_Z_NORM )
    lUnpackedNormal.z = sqrt( saturate( 1.0 - lUnpackedNormal.x*lUnpackedNormal.x - lUnpackedNormal.y*lUnpackedNormal.y ) );
#endif

    return half3(lUnpackedNormal);
#endif

#endif // LOW_QUALITY
}


vec3
GetBlendedColour(
    in  CustomPerMaterialUniforms lCustomUniforms,
    in    vec2         lTileCoordsVec2,
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    in    uint         lTile1UInt,
    in    uint         lTile2UInt,
    in    float        lfHeight1,
    in    float        lfHeight2,
    in    float        lfHeight3,
    in    float        lfHeight4,
    in    float        lfHeight5,
    in    float        lfHeight6,
    in    float        lfHeight7,
    out   float        lfMetallic )
{

    lfMetallic = 0.0;
#ifndef D_LOW_QUALITY
    const float lfThreshold = 1.0 / 20.0;
    vec3  lColour5 = vec3( 0.0, 0.0, 0.0 );
    vec3  lColour6 = vec3( 0.0, 0.0, 0.0 );
    float lfMetallic5 = 0.0;
    float lfMetallic6 = 0.0;

    if ( lfHeight7 < 1.0 - lfThreshold )
    {
#ifdef D_PLATFORM_OPENGL
        if ( lfHeight5 < 1.0 - lfThreshold )
#endif
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
#ifdef D_PLATFORM_OPENGL
        if ( lfHeight5 > lfThreshold )
#endif
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
#ifdef D_PLATFORM_OPENGL
        if ( lfHeight6 < 1.0 - lfThreshold )
#endif
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
#ifdef D_PLATFORM_OPENGL
        if ( lfHeight6 > lfThreshold )
#endif
        {
            if ( ( 1.0 - lfHeight4 ) * lfHeight6 > lfThreshold )
            {
                int liIndex = int( UnpackItemZChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += ( 1.0 - lfHeight4 ) * lfHeight6  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += ( 1.0 - lfHeight4 ) * lfHeight6 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
#ifdef D_DEFER
            if ( lfHeight4 * lfHeight6 > lfThreshold )
            {
                int liIndex = int( UnpackItemWChannel( lTile2UInt ) );
                
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                lfMetallic6 += lfHeight4 * lfHeight6  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour6 += lfHeight4 * lfHeight6 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
#endif
        }
    }

    lfMetallic = mix( lfMetallic5, lfMetallic6, lfHeight7 );
    return       mix( lColour5, lColour6, lfHeight7 );
#else
    int liIndex = int( UnpackItemXChannel( lTile1UInt ) );
    
    vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
    vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
    return GetTriPlanarColourArray(
        lTileCoordsVec2,
        SAMPLER2DARRAYPARAM( lDiffuseMap ),
        SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
        liIndex,
        lOriginalColourVec3,
        lRecolourVec3 );
#endif
}

#if defined (D_STOCHASTIC_TERRAIN)

#if defined ( D_FORCE_HALF )
half3
#else
vec3
#endif
GetStochasticBlendedColour(
    in  CustomPerMaterialUniforms lCustomUniforms,
#if defined ( D_FORCE_HALF_UV )
    in    half2        lTileCoordsVec2,
#else
    in    vec2         lTileCoordsVec2,
#endif
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    in    uint         lTileUInt,
    #if defined ( D_FORCE_HALF )
    in    half         lfHeight1,
    in    half         lfHeight2,
    in    half         lfHeight3,
    out   half         lfMetallic )
    #else
    in    float        lfHeight1,
    in    float        lfHeight2,
    in    float        lfHeight3,
    out   float        lfMetallic )
    #endif
{
#if defined ( D_FORCE_HALF )
    const half lfThreshold = 1.0 / 20.0;
    half3  lColour = vec3( 0.0, 0.0, 0.0 );
#else
    const half lfThreshold = 1.0 / 20.0;
    vec3  lColour = vec3( 0.0, 0.0, 0.0 );
#endif

    lfMetallic = 0.0;

    {
        {
            if ( ( 1.0 - lfHeight2 ) * ( 1.0 - lfHeight1 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemXChannel( lTileUInt ) );

                #if defined ( D_FORCE_HALF )
                half3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                half3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #else
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #endif
                
                lfMetallic += ( 1.0 - lfHeight2 ) * ( 1.0 - lfHeight1 ) *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour += ( 1.0 - lfHeight2 ) * ( 1.0 - lfHeight1 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight2 * ( 1.0 - lfHeight1 ) > lfThreshold )
            {
                int liIndex = int( UnpackItemYChannel( lTileUInt ) );

                #if defined ( D_FORCE_HALF )
                half3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                half3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #else
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #endif
                
                lfMetallic += lfHeight2 * ( 1.0 - lfHeight1 )  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour += lfHeight2 * ( 1.0 - lfHeight1 ) * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
        {
            if ( ( 1.0 - lfHeight3 ) * lfHeight1 > lfThreshold )
            {
                int liIndex = int( UnpackItemZChannel( lTileUInt ) );

                #if defined ( D_FORCE_HALF )
                half3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                half3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #else
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #endif
                
                lfMetallic += ( 1.0 - lfHeight3 ) * lfHeight1  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour += ( 1.0 - lfHeight3 ) * lfHeight1 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
            if ( lfHeight3 * lfHeight1 > lfThreshold )
            {
                int liIndex = int( UnpackItemWChannel( lTileUInt ) );

                #if defined ( D_FORCE_HALF )
                half3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                half3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #else
                vec3 lOriginalColourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaAverageColoursVec4, liIndex ).rgb;
                vec3 lRecolourVec3 = ARRAY_LOOKUP_FS( lCustomUniforms, mpCustomPerMaterial, gaTerrainColoursVec4, liIndex ).rgb;
                #endif
                
                lfMetallic += lfHeight3 * lfHeight1  *  ( liIndex >= 20 ? 1.0 : 0.0 );
                lColour += lfHeight3 * lfHeight1 * GetTriPlanarColourArray( lTileCoordsVec2, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), liIndex, lOriginalColourVec3, lRecolourVec3 ).xyz;
            }
        }
    }

    return lColour;
}



//-----------------------------------------------------------------------------
///
///     GetStochasticTileColourAndNormal
///
///		Get the tile colour and normal, without doing a full blend across the
///     triplanar textures and tile textures.  We either use a stochastic method
///     keyed off the world position of the fragment or a height-as-alpha-test
///     to select the tile.
//-----------------------------------------------------------------------------
vec3
GetStochasticTileColourAndNormal(
    in  CustomPerMaterialUniforms lCustomUniforms,    
#if defined ( D_FORCE_HALF )
    in  half3          lFaceNormalVec3,
#else
    in  vec3           lFaceNormalVec3,
#endif
    in  vec3           lPlanetOffsetVec3,
    in  vec3           lLocalPositionVec3,
    in  uvec4          lTileTextureIndicesVec4,
#if defined ( D_FORCE_HALF )
    in  half           lfPatch,
    in  half           lfSlope1,
#if !defined( D_TERRAIN_T_SPLIT )
    in  half           lfSlope2,
#endif
    in  half           lfTileType,
#else
    in  float          lfPatch,
    in  float          lfSlope1,
#if !defined( D_TERRAIN_T_SPLIT )
    in  float          lfSlope2,
#endif
    in  float          lfTileType,
#endif
#if defined ( D_FORCE_HALF )
    out half3          lOutWorldNormalVec3,
#else
    out vec3           lOutWorldNormalVec3,
#endif
    in  float          lfSmallScale,
    in  float          lfLargeScale,
#if defined ( D_FORCE_HALF )
    in  half           lfFade,
    in  half           lfWaterFade,
#else
    in  float          lfFade,
    in  float          lfWaterFade,
#endif
    SAMPLER2DARRAYARG( lDiffuseMap ),
    SAMPLER2DARRAYARG( lNormalMap ),
    SAMPLER2DARRAYARG( lSubstanceDiffuseMap ),
    SAMPLER2DARRAYARG( lSubstanceNormalMap ),
    SAMPLER2DARG(      lNoiseNormalMap ),
#if defined ( D_FORCE_HALF )
    out half           lfSpecular,
    out half           lfMetallic,
    out half           lfHeight )
#else
    out float          lfSpecular,
    out float          lfMetallic,
    out float          lfHeight )
#endif
{
    
    bool lUsingWeightX = false;
    bool lUsingWeightY = false;

#if defined ( D_USE_PACKED_COORDS )
    uint luPackedCoordsSmall;
    uint luPackedCoordsLarge;

#if !defined( D_DISABLE_NOISE )
    uint luPackedCoordsNoiseSmall;
    uint luPackedCoordsNoiseLarge;
#endif

#else

#if defined ( D_FORCE_HALF_UV )
    half2 lLargeTextureCoordsVec2;
    half2 lSmallTextureCoordsVec2;
#else
    vec2 lLargeTextureCoordsVec2;
    vec2 lSmallTextureCoordsVec2;
#endif

#if !defined( D_DISABLE_NOISE )
    vec2 lSmallNoiseTexCoordsVec2;
    vec2 lLargeNoiseTexCoordsVec2;
#endif

#endif

    {    
#if defined( D_TERRAIN_X_FACING )
        vec3    lWeightsVec3N = vec3( 1.0, 0.0, 0.0 );
#elif defined( D_TERRAIN_Y_FACING )
        vec3    lWeightsVec3N = vec3( 0.0, 1.0, 0.0 );
#elif defined( D_TERRAIN_Z_FACING )
        vec3    lWeightsVec3N = vec3( 0.0, 0.0, 1.0 );
#else
        vec3    lWeightsVec3N = pow( abs( lFaceNormalVec3 ), vec3( kfBlendPower, kfBlendPower, kfBlendPower ) );
        lWeightsVec3N        /= ( lWeightsVec3N.x + lWeightsVec3N.y + lWeightsVec3N.z );
#endif        

        bool lbFacingX  = ( lWeightsVec3N.y < kfWeightFloor ) && ( lWeightsVec3N.z < kfWeightFloor );
        bool lbFacingY  = ( lWeightsVec3N.x < kfWeightFloor ) && ( lWeightsVec3N.z < kfWeightFloor );
        bool lbFacingZ  = ( lWeightsVec3N.x < kfWeightFloor ) && ( lWeightsVec3N.y < kfWeightFloor );

        // fallback necessary cause some triangles might change their facing direction due to the flattening
        // that occurs in the vertex shader
        bool lbNoFacing = !( lbFacingX || lbFacingY || lbFacingZ );

        if ( lbNoFacing )
        {
            // The shortened blend, works as well as a more correct blend in terms of seams for the lod'd version of the stochastic terrain!

            lbFacingX = ( lWeightsVec3N.x >= kfWeightFloor ) && ( lWeightsVec3N.x > lfPatch );
            lbFacingY = ( lWeightsVec3N.y >= kfWeightFloor ) && (( lWeightsVec3N.x + lWeightsVec3N.y ) > lfPatch );
            lbFacingZ = !lbFacingX && !lbFacingY;        
        }

        lUsingWeightX = lbFacingX;
        lUsingWeightY = !lbFacingX && lbFacingY;
     
#if defined ( D_USE_PACKED_COORDS )
        vec2 lSmallTextureCoordsVec2;
        vec2 lLargeTextureCoordsVec2;
#endif

#if defined ( D_FORCE_HALF_UV )
        half3 lSmallTextureCoordsVec3; 
        half3 lLargeTextureCoordsVec3;
#else
        vec3 lSmallTextureCoordsVec3;
        vec3 lLargeTextureCoordsVec3;
#endif

        lSmallTextureCoordsVec3   = mod( lLocalPositionVec3 * lfSmallScale, kfWrapScale / kfNoiseTextScale );
        lSmallTextureCoordsVec3  += mod( lPlanetOffsetVec3  * lfSmallScale, kfWrapScale / kfNoiseTextScale );
        lLargeTextureCoordsVec3   = mod( lLocalPositionVec3 * lfLargeScale, kfWrapScale / kfNoiseTextScale );
        lLargeTextureCoordsVec3  += mod( lPlanetOffsetVec3  * lfLargeScale, kfWrapScale / kfNoiseTextScale );

        lSmallTextureCoordsVec3   = mod( lSmallTextureCoordsVec3, kfWrapScale / kfNoiseTextScale );
        lLargeTextureCoordsVec3   = mod( lLargeTextureCoordsVec3, kfWrapScale / kfNoiseTextScale );

        if ( lbFacingX )
        {
            lSmallTextureCoordsVec2 = lSmallTextureCoordsVec3.yz;
            lLargeTextureCoordsVec2 = lLargeTextureCoordsVec3.yz;
        }
        else if ( lbFacingY )
        {
            lSmallTextureCoordsVec2 = lSmallTextureCoordsVec3.zx;
            lLargeTextureCoordsVec2 = lLargeTextureCoordsVec3.zx;
        }
        else
        {
            lSmallTextureCoordsVec2 = lSmallTextureCoordsVec3.xy;
            lLargeTextureCoordsVec2 = lLargeTextureCoordsVec3.xy;
        }

#if defined ( D_USE_PACKED_COORDS )
        PackRG16( luPackedCoordsSmall,      fract( lSmallTextureCoordsVec2  ) );
        PackRG16( luPackedCoordsLarge,      fract( lLargeTextureCoordsVec2  ) );

#if !defined( D_DISABLE_NOISE )
        lSmallNoiseTexCoordsVec2 = lSmallTextureCoordsVec2 * kfNoiseTextScale;
        lLargeNoiseTexCoordsVec2 = lLargeTextureCoordsVec2 * kfNoiseTextScale;

        PackRG16( luPackedCoordsNoiseSmall, fract( lSmallNoiseTexCoordsVec2 ) );
        PackRG16( luPackedCoordsNoiseLarge, fract( lLargeNoiseTexCoordsVec2 ) );
#endif

#endif
    }

    lfMetallic   = 0.0;
    lfHeight     = 0.0;
    lfSpecular   = 0.0;
    
    // Normals + (Spec and Height)

#if defined ( D_TERRAIN_T_SPLIT )
    #if defined ( D_FORCE_HALF )
    half lafHeights[ 3 ] = { 0.0, 0.0, 0.0 };
    #else
    float lafHeights[3] = { 0.0, 0.0, 0.0 };
    #endif
#else
    #if defined ( D_FORCE_HALF )
    half lafHeights[ 4 ] = { 0.0, 0.0, 0.0, 0.0 };
    #else
    float lafHeights[4] = { 0.0, 0.0, 0.0, 0.0 };
    #endif
#endif    
    
    bool lInterpolateLHS = true;
    bool lInterpolateRHS = true;    
    bool useTile2        = false;
    {        
#if defined ( D_TERRAIN_T_SPLIT )
        uint  luPackedHeights = 0;
#else
        #if defined ( D_FORCE_HALF )
        half lHeightsLargeSmall[ 7 ] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        #else
        float lHeightsLargeSmall[7] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        #endif
#endif

        #if defined ( D_FORCE_HALF )
        const half kfFadeThreshold = 1.0 / 20.0;
        #else
        const float kfFadeThreshold = 1.0 / 20.0;
        #endif

        if ( lfFade <= kfFadeThreshold )
        {
            lInterpolateLHS = false;
            lfFade = 0.0;
        }
        else
        if ( lfFade > 1.0 - kfFadeThreshold )
        {
            lInterpolateRHS = false;
            lfFade = 1.0;
        }                        
        
        #if defined ( D_FORCE_HALF )
        half3 lNormalMappedVec3 = half3( 0.0, 0.0, 0.0 );
        #else
        vec3  lNormalMappedVec3 = vec3( 0.0, 0.0, 0.0 );
        #endif

        if ( lInterpolateLHS )
        {
            #if defined ( D_FORCE_HALF )
            half lfSpecularCoeff = 0.0;
            half3  lNormalTileVec3;
            #else
            float lfSpecularCoeff = 0.0;
            vec3  lNormalTileVec3;
            #endif

#if defined ( D_TERRAIN_T_SPLIT )
            #if defined ( D_PLATFORM_SWITCH )
            lNormalTileVec3 = GetBlendedNormalSingleTileUnpacked(
            #else
            lNormalTileVec3 = GetBlendedNormalSingleTile(
            #endif
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsLarge ),
#else
                lLargeTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lNormalMap ),
                SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                lTileTextureIndicesVec4.z,
                lfPatch,
                lfSlope1,
                lfSpecular,
                #if defined ( D_PLATFORM_SWITCH )
                lfHeight,
                #else
                lfSpecularCoeff,
                #endif
                luPackedHeights,
                lfFade );
#else            
            lNormalTileVec3 = vec3(GetBlendedNormal(
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsLarge ),
#else
                lLargeTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lNormalMap ),
                SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lfPatch,
                lfSlope1,
                lfSlope2,
                lfTileType,
                lfSpecular,
                lfSpecularCoeff,
                lfHeight,
                lHeightsLargeSmall[0],
                lHeightsLargeSmall[1],
                lHeightsLargeSmall[2],
                lHeightsLargeSmall[3],
                lHeightsLargeSmall[4],
                lHeightsLargeSmall[5],
                lHeightsLargeSmall[6],
                lfFade ));
#endif

#if !defined( D_DISABLE_NOISE )
            half  lfHeightNoise;
            half  lfSpecularNoise;
            half3 lNormalNoiseVec3;

            lNormalNoiseVec3 = GetTriPlanarNoiseNormal(
#if defined ( D_USE_PACKED_COORDS )                
                UnpackRG16( luPackedCoordsNoiseLarge ),
#else
                lLargeTextureCoordsVec2 * kfNoiseTextScale,
#endif
                SAMPLER2DPARAM( lNoiseNormalMap ),
                lfHeightNoise, 
                lfSpecularNoise );
#endif // D_DISABLE_NOISE

            #if defined ( D_FORCE_HALF )
            half4 lPlaneNorm;
            if ( lUsingWeightX )        lPlaneNorm = half4( lFaceNormalVec3.yz, abs( lFaceNormalVec3.x ), lFaceNormalVec3.x < 0.0 ? -1.0 : 1.0 );
            else if ( lUsingWeightY )   lPlaneNorm = half4( lFaceNormalVec3.zx, abs( lFaceNormalVec3.y ), lFaceNormalVec3.y < 0.0 ? -1.0 : 1.0 );
            else                        lPlaneNorm = half4( lFaceNormalVec3.xy, abs( lFaceNormalVec3.z ), lFaceNormalVec3.z < 0.0 ? -1.0 : 1.0 );
            #else
            vec4 lPlaneNorm;
            if ( lUsingWeightX )        lPlaneNorm = vec4( lFaceNormalVec3.yz, abs( lFaceNormalVec3.x ), lFaceNormalVec3.x < 0.0 ? -1.0 : 1.0 );
            else if ( lUsingWeightY )   lPlaneNorm = vec4( lFaceNormalVec3.zx, abs( lFaceNormalVec3.y ), lFaceNormalVec3.y < 0.0 ? -1.0 : 1.0 );
            else                        lPlaneNorm = vec4( lFaceNormalVec3.xy, abs( lFaceNormalVec3.z ), lFaceNormalVec3.z < 0.0 ? -1.0 : 1.0 );
            #endif
            
#if !defined( D_DISABLE_NOISE )
            lNormalTileVec3             = BlendNormalsRNM_FP( lPlaneNorm.xyz,  lNormalTileVec3 );
            lNormalTileVec3             = BlendNormalsUDN_FP( lNormalTileVec3, vec3(lNormalNoiseVec3), kfNoiseNormScale ) * lfFade;
            lNormalTileVec3.z          *= lPlaneNorm.w;
            
            lfSpecular = saturate( lfSpecular - float( lfSpecularNoise ) * lfSpecularCoeff * lfFade );
#if defined( D_TERRAIN_T_SPLIT )
            PackR8( luPackedHeights, saturate( UnpackR8( luPackedHeights ) + lfHeightNoise * lfFade ) );
#else
            lfHeight   = saturate( lfHeight + float( lfHeightNoise ) * lfFade );
#endif

#else
            #if defined ( D_FORCE_HALF )
            lNormalTileVec3 = BlendNormalsRNM_HALF( lPlaneNorm.xyz, lNormalTileVec3 ) * lfFade;
            #else
            lNormalTileVec3 = BlendNormalsRNM_FP( lPlaneNorm.xyz,  lNormalTileVec3 ) * lfFade;
            #endif

            lNormalTileVec3.z          *= lPlaneNorm.w;
#endif // D_DISABLE_NOISE

            if ( lUsingWeightX )        lNormalMappedVec3 += lNormalTileVec3.zxy;
            else if ( lUsingWeightY )   lNormalMappedVec3 += lNormalTileVec3.yzx;
            else                        lNormalMappedVec3 += lNormalTileVec3;
        }


        if ( lInterpolateRHS )
        {
            #if defined ( D_FORCE_HALF )
            half lfSpecularCoeff = 0.0;
            half3  lNormalTileVec3;
            #else
            float lfSpecularCoeff = 0.0;
            vec3  lNormalTileVec3;
            #endif

#if defined ( D_TERRAIN_T_SPLIT )
            #if defined ( D_PLATFORM_SWITCH )
            lNormalTileVec3 = GetBlendedNormalSingleTileUnpacked(
            #else
            lNormalTileVec3 = GetBlendedNormalSingleTile(
            #endif
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsSmall ),
#else
                lSmallTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lNormalMap ),
                SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                lTileTextureIndicesVec4.x,
                lfPatch,
                lfSlope1,
                lfSpecular,
                #if defined ( D_PLATFORM_SWITCH )
                lfHeight,
                #else
                lfSpecularCoeff,
                #endif
                luPackedHeights,
                1.0 - lfFade );

#else
            lNormalTileVec3 = vec3(GetBlendedNormal(
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsSmall ),
#else
                lSmallTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lNormalMap ),
                SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lfPatch,
                lfSlope1,
                lfSlope2,
                lfTileType,
                lfSpecular,
                lfSpecularCoeff,
                lfHeight,
                lHeightsLargeSmall[0],
                lHeightsLargeSmall[1],
                lHeightsLargeSmall[2],
                lHeightsLargeSmall[3],
                lHeightsLargeSmall[4],
                lHeightsLargeSmall[5],
                lHeightsLargeSmall[6],
                1.0 - lfFade ));

#endif

#if !defined( D_DISABLE_NOISE )
            half  lfHeightNoise;
            half  lfSpecularNoise;
            half3 lNormalNoiseVec3;

            lNormalNoiseVec3 = GetTriPlanarNoiseNormal(
#if defined ( D_USE_PACKED_COORDS )                
                UnpackRG16( luPackedCoordsNoiseSmall ),
#else
                lSmallTextureCoordsVec2 * kfNoiseTextScale,
#endif
                SAMPLER2DPARAM( lNoiseNormalMap ),
                lfHeightNoise, 
                lfSpecularNoise );
#endif // D_DISABLE_NOISE

            #if defined ( D_FORCE_HALF )
            half4  lPlaneNorm;
            if ( lUsingWeightX )        lPlaneNorm = half4( lFaceNormalVec3.yz, abs( lFaceNormalVec3.x ), lFaceNormalVec3.x < 0.0 ? -1.0 : 1.0 );
            else if ( lUsingWeightY )   lPlaneNorm = half4( lFaceNormalVec3.zx, abs( lFaceNormalVec3.y ), lFaceNormalVec3.y < 0.0 ? -1.0 : 1.0 );
            else                        lPlaneNorm = half4( lFaceNormalVec3.xy, abs( lFaceNormalVec3.z ), lFaceNormalVec3.z < 0.0 ? -1.0 : 1.0 );
            #else
            vec4  lPlaneNorm;
            if ( lUsingWeightX )        lPlaneNorm = vec4( lFaceNormalVec3.yz, abs( lFaceNormalVec3.x ), lFaceNormalVec3.x < 0.0 ? -1.0 : 1.0 );
            else if ( lUsingWeightY )   lPlaneNorm = vec4( lFaceNormalVec3.zx, abs( lFaceNormalVec3.y ), lFaceNormalVec3.y < 0.0 ? -1.0 : 1.0 );
            else                        lPlaneNorm = vec4( lFaceNormalVec3.xy, abs( lFaceNormalVec3.z ), lFaceNormalVec3.z < 0.0 ? -1.0 : 1.0 );
            #endif

#if !defined( D_DISABLE_NOISE )
            lNormalTileVec3             = BlendNormalsRNM_FP( lPlaneNorm.xyz,  lNormalTileVec3 );
            lNormalTileVec3             = BlendNormalsUDN_FP( lNormalTileVec3, vec3(lNormalNoiseVec3), kfNoiseNormScale ) * ( 1.0 - lfFade );
            lNormalTileVec3.z          *= lPlaneNorm.w;

            lfSpecular = saturate( lfSpecular - float( lfSpecularNoise ) * lfSpecularCoeff * ( 1.0 - lfFade ) );
#if defined( D_TERRAIN_T_SPLIT )
            PackR8( luPackedHeights, saturate( UnpackR8( luPackedHeights ) + lfHeightNoise * ( 1.0 - lfFade ) ) );
#else
            lfHeight = saturate( lfHeight + float( lfHeightNoise ) * ( 1.0 - lfFade ) );
#endif

#else
            #if defined ( D_FORCE_HALF )
            lNormalTileVec3 = BlendNormalsRNM_HALF( lPlaneNorm.xyz,  lNormalTileVec3 ) * (1.0 - lfFade);
            #else
            lNormalTileVec3 = BlendNormalsRNM_FP( lPlaneNorm.xyz, lNormalTileVec3 ) * ( 1.0 - lfFade );
            #endif
            
            lNormalTileVec3.z          *= lPlaneNorm.w;
#endif // D_DISABLE_NOISE

            if ( lUsingWeightX )        lNormalMappedVec3 += lNormalTileVec3.zxy;
            else if ( lUsingWeightY )   lNormalMappedVec3 += lNormalTileVec3.yzx;
            else                        lNormalMappedVec3 += lNormalTileVec3;
        }
        
        lOutWorldNormalVec3   = normalize( mix( lFaceNormalVec3, lNormalMappedVec3, kfNormBlend * lfWaterFade ) );

#if defined ( D_TERRAIN_T_SPLIT )

        #if defined ( D_FORCE_HALF )
        const half kfWidth = 0.1;
        const half kfWidthScale = 2.0;

        lfHeight = UnpackR8( luPackedHeights );
        lafHeights[ 0 ] = UnpackA8( luPackedHeights );
        lafHeights[ 0 ] = FastSmoothStepHalf( lafHeights[ 0 ] - kfWidth, kfWidthScale * kfWidth, lfSlope1 );

        lafHeights[ 1 ] = UnpackG8( luPackedHeights );
        lafHeights[ 1 ] = FastSmoothStepHalf( lafHeights[ 1 ] - kfWidth, kfWidthScale * kfWidth, lfPatch );

        lafHeights[ 2 ] = UnpackB8( luPackedHeights );
        lafHeights[ 2 ] = FastSmoothStepHalf( lafHeights[ 2 ] - kfWidth, kfWidthScale * kfWidth, lfPatch );
        #else
        lfHeight = UnpackR8( luPackedHeights );

        float lfWidth = 0.1;

        // First path: height7, height5, height1, height2, tile1
        // Second path: height7, height6, height3, height4, tile2
        lafHeights[0] = float( UnpackA8( luPackedHeights ) );
        lafHeights[0] = FastSmoothStep( lafHeights[0] - lfWidth, 2.0f * lfWidth, lfSlope1 );

        lafHeights[1] = float( UnpackG8( luPackedHeights ) );
        lafHeights[1] = FastSmoothStep( lafHeights[1] - lfWidth, 2.0f * lfWidth, lfPatch );

        lafHeights[2] = float( UnpackB8( luPackedHeights ) );
        lafHeights[2] = FastSmoothStep( lafHeights[2] - lfWidth, 2.0f * lfWidth, lfPatch );
        #endif  // defined ( D_FORCE_HALF )
    }

#if defined ( D_FORCE_HALF )
    half3    lTileColourVec3;
    half3    lTileColourSmallVec3 = half3( 0.0, 0.0, 0.0 );
    half3    lTileColourLargeVec3 = half3( 0.0, 0.0, 0.0 );
#else
    vec3    lTileColourVec3;
    vec3    lTileColourSmallVec3 = vec3( 0.0, 0.0, 0.0 );
    vec3    lTileColourLargeVec3 = vec3( 0.0, 0.0, 0.0 );
#endif

    {
        if ( lInterpolateLHS )
        {
            uint lLargeTileUInt = lTileTextureIndicesVec4.z;

            lTileColourLargeVec3 = GetStochasticBlendedColour(
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsLarge ),
#else
                lLargeTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lLargeTileUInt,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lfMetallic ).xyz;
        }

        if ( lInterpolateRHS )
        {
            uint lSmallTileUInt = lTileTextureIndicesVec4.x;

            lTileColourSmallVec3 = GetStochasticBlendedColour(
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsSmall ),
#else
                lSmallTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lSmallTileUInt,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lfMetallic ).xyz;
        }
    }

#else

        #if defined ( D_FORCE_HALF )
        const half kfWidth = 0.1;
        const half kfWidthScale = 2.0;
        const half kfThreshold = 1.0 / 20.0;

        lafHeights[ 0 ] = FastSmoothStepHalf( lHeightsLargeSmall[ 6 ] - kfWidth, kfWidthScale * kfWidth, lfTileType );
        useTile2 = lafHeights[ 0 ] < 1.0 - kfThreshold ? false : true;

        const half2 kvWidth = half2( 0.1, 0.1 );
        const half2 kvWidthScaled = kvWidth * kfWidthScale;
        const half2 kvSlope1 = half2( lfSlope1, lfSlope1 );
        const half2 kvPatch = half2( lfPatch, lfPatch );

        half2 lvHeight1 = half2( lHeightsLargeSmall[ 5 ], lHeightsLargeSmall[ 4 ] );
        half2 lvHeight2 = half2( lHeightsLargeSmall[ 2 ], lHeightsLargeSmall[ 0 ] );
        half2 lvHeight3 = half2( lHeightsLargeSmall[ 3 ], lHeightsLargeSmall[ 1 ] );

        lvHeight1 = FastSmoothStepHalf( lvHeight1 - kfWidth, kvWidthScaled, kvSlope1 );
        lvHeight2 = FastSmoothStepHalf( lvHeight2 - kfWidth, kvWidthScaled, kvPatch );
        lvHeight3 = FastSmoothStepHalf( lvHeight3 - kfWidth, kvWidthScaled, kvPatch );

        lafHeights[ 1 ] = useTile2 ? lvHeight1.x : lvHeight1.y;
        lafHeights[ 2 ] = useTile2 ? lvHeight2.x : lvHeight2.y;
        lafHeights[ 3 ] = useTile2 ? lvHeight3.x : lvHeight3.y;

        #else

        float lfWidth = 0.1;
        //lafHeights[0] = smoothstep( lHeightsLargeSmall[6] - lfWidth, lHeightsLargeSmall[6] + lfWidth, lfTileType );
        lafHeights[0] = FastSmoothStep( lHeightsLargeSmall[6] - lfWidth, 2.0f * lfWidth, lfTileType );

        {
            const float lfThreshold = 1.0 / 20.0;

            // Using just the height to differentiate between tiles gives us hard edges, but more VGPRs back, and looks ok :).
            useTile2 = lafHeights[0] < 1.0 - lfThreshold ? false : true;
        }

        // First path: height7, height5, height1, height2, tile1
        // Second path: height7, height6, height3, height4, tile2
        lafHeights[1] = useTile2 ? lHeightsLargeSmall[5] : lHeightsLargeSmall[4];
        //lafHeights[1] = smoothstep( lafHeights[1] - lfWidth, lafHeights[1] + lfWidth, lfSlope1 );
        lafHeights[1] = FastSmoothStep(lafHeights[1] - lfWidth, 2.0f * lfWidth, lfSlope1);

        lafHeights[2] = useTile2 ? lHeightsLargeSmall[2] : lHeightsLargeSmall[0];
        //lafHeights[2] = smoothstep( lafHeights[2] - lfWidth, lafHeights[2] + lfWidth, lfPatch );
        lafHeights[2] = FastSmoothStep(lafHeights[2] - lfWidth, 2.0f * lfWidth, lfPatch);

        lafHeights[3] = useTile2 ? lHeightsLargeSmall[3] : lHeightsLargeSmall[1];
        //lafHeights[3] = smoothstep( lafHeights[3] - lfWidth, lafHeights[3] + lfWidth, lfPatch );
        lafHeights[3] = FastSmoothStep(lafHeights[3] - lfWidth, 2.0f * lfWidth, lfPatch );
        #endif // defined ( D_FORCE_HALF )
    }   

#if defined ( D_FORCE_HALF )
    half3    lTileColourVec3;
    half3    lTileColourSmallVec3 = half3( 0.0, 0.0, 0.0 );
    half3    lTileColourLargeVec3 = half3( 0.0, 0.0, 0.0 );
#else
    vec3    lTileColourVec3;
    vec3    lTileColourSmallVec3 = vec3( 0.0, 0.0, 0.0 );
    vec3    lTileColourLargeVec3 = vec3( 0.0, 0.0, 0.0 );
#endif
    {
        if ( lInterpolateLHS )
        {
            uint lLargeTileUInt = useTile2 ? lTileTextureIndicesVec4.w : lTileTextureIndicesVec4.z;

            lTileColourLargeVec3 = GetStochasticBlendedColour(
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsLarge ),
#else
                lLargeTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lLargeTileUInt,
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lfMetallic ).xyz;
        }

        if ( lInterpolateRHS )
        {
            uint lSmallTileUInt = useTile2 ? lTileTextureIndicesVec4.y : lTileTextureIndicesVec4.x;

            lTileColourSmallVec3 = GetStochasticBlendedColour(
                lCustomUniforms,
#if defined( D_USE_PACKED_COORDS )
                UnpackRG16( luPackedCoordsSmall ),
#else
                lSmallTextureCoordsVec2,
#endif
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lSmallTileUInt,
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lfMetallic ).xyz;
        }
    }

#endif // D_TERRAIN_T_SPLIT

    lTileColourVec3 = mix( lTileColourSmallVec3, lTileColourLargeVec3, lfFade );

#ifndef D_CACHE_COLOUR
    lTileColourVec3 = GammaCorrectInput01( lTileColourVec3 );
#endif

    return lTileColourVec3;
}

#endif


#if !defined( D_STOCHASTIC_TERRAIN )
vec3
GetTileColourAndNormal(
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
#if !defined( D_DISABLE_NOISE )
    vec3    lSmallNoiseTexCoordsVec3;
    vec3    lLargeNoiseTexCoordsVec3;
#endif
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

#if !defined( D_DISABLE_NOISE )
    lSmallNoiseTexCoordsVec3 = lSmallTexCoordsVec3 * kfNoiseTextScale;
    lLargeNoiseTexCoordsVec3 = lLargeTexCoordsVec3 * kfNoiseTextScale;
#endif

    vec3 lWeightsVec3N = lWeightsVec3 * lfWeightRecip;

    // Normals + (Spec and Height)
    float lafHeights[8] = { 0., 0., 0., 0., 0., 0., 0., 0. };
    float lfDisplacement = 0.0;
    {
        half3  lPlaneNorm;
        half3  lNormalSmallVec3;
        half3  lNormalLargeVec3;
#if !defined( D_DISABLE_NOISE )
        half3  lNoiseNormalVec3;
#endif
        half3  lSmallMappedNormalVec3 = half3( 0.0, 0.0, 0.0 );
        half3  lLargeMappedNormalVec3 = half3( 0.0, 0.0, 0.0 );
        half3  lSpecularLargeVec3     = half3(0.0, 0.0, 0.0);
        half3  lSpecularSmallVec3     = half3(0.0, 0.0, 0.0);

#if defined ( D_PACKED_HEIGHTS )

        const uint kuMidHeights = PackHeightsAB(0.5, 0.5);

        uvec2  lHeightsLargeVec3[8] =
        { uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights ) };
        uvec2  lHeightsSmallVec3[8] =
        { uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights )
        , uvec2( kuMidHeights, kuMidHeights ) };

#else
        half3  lHeightsLargeVec3[8] = { half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5) };
        half3  lHeightsSmallVec3[8] = { half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5), half3(0.5, 0.5, 0.5) };
#endif

        vec3   lDisplacementSmallVec3 = vec3( 0.0, 0.0, 0.0 );
        vec3   lDisplacementLargeVec3 = vec3( 0.0, 0.0, 0.0 );
        float  lfFadeThreshold = 1.0 / 20.0;
        float  lfSpecularCoeff = 0.0;
#if !defined( D_DISABLE_NOISE )
        half   lfHeightNoise;
        half   lfSpecularNoise;
#endif

        if ( lfFade > lfFadeThreshold )
        {            
            if ( lWeightsVec3N.x > kfWeightFloor )
            {           

#define GET_INDEX_LARGE(e, c) lHeightsLargeVec3[e][c]
                float specLargeX = lSpecularLargeVec3.x;
                float largeDisplX = lDisplacementLargeVec3.x;
                lNormalLargeVec3 = GetBlendedNormal(
                    lCustomUniforms,
                    lLargeTexCoordsVec3.yz,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.z,
                    lTileTextureIndicesVec4.w,
                    lfPatch,
                    lfSlope1,
                    lfSlope2,
                    lfTileType,
                    specLargeX,
                    lfSpecularCoeff,
                    largeDisplX,
#if defined ( D_PACKED_HEIGHTS )
                    GET_INDEX_LARGE(0, 1),
                    GET_INDEX_LARGE(1, 1),
                    GET_INDEX_LARGE(2, 1),
                    GET_INDEX_LARGE(3, 1),
                    GET_INDEX_LARGE(4, 1),
                    GET_INDEX_LARGE(5, 1),
                    GET_INDEX_LARGE(6, 1),
                    GET_INDEX_LARGE(7, 1));
#else
                    GET_INDEX_LARGE(0, 0),
                    GET_INDEX_LARGE(1, 0),
                    GET_INDEX_LARGE(2, 0),
                    GET_INDEX_LARGE(3, 0),
                    GET_INDEX_LARGE(4, 0),
                    GET_INDEX_LARGE(5, 0),
                    GET_INDEX_LARGE(6, 0),
                    GET_INDEX_LARGE(7, 0));
#endif
                    lSpecularLargeVec3.x = specLargeX;
                    lDisplacementLargeVec3.x = largeDisplX;
#if defined ( D_PACKED_HEIGHTS )
                for (uint i = 0; i < 8; ++i)
                {
                    float lfX = UnpackHeightB(lHeightsLargeVec3[i].y);
                    float lfY = UnpackHeightB(lHeightsLargeVec3[i].x);
                    lHeightsLargeVec3[i].x = PackHeightsAB(lfX, lfY);
                }
#endif

#if !defined( D_DISABLE_NOISE )
                lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lLargeNoiseTexCoordsVec3.yz,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );
#endif

                lPlaneNorm                = half3( lNormalVec3.y,lNormalVec3.z, abs( lNormalVec3.x ) );
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 );
#if !defined( D_DISABLE_NOISE )
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale );
#endif
                lNormalLargeVec3.z       *= half( sign( lNormalVec3.x ) );
                lNormalLargeVec3          = lNormalLargeVec3.zxy;
                lLargeMappedNormalVec3   += half( lWeightsVec3N.x ) * lNormalLargeVec3;

#if !defined( D_DISABLE_NOISE )
                lSpecularLargeVec3.x      = saturate( lSpecularLargeVec3.x     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementLargeVec3.x  = saturate( lDisplacementLargeVec3.x + lfHeightNoise );
#endif

            }
            if ( lWeightsVec3N.y > kfWeightFloor )
            {

#define GET_INDEX_LARGE(e, c) lHeightsLargeVec3[e][c]

                float lSpecY = lSpecularLargeVec3.y;
                float largeDisplY = lDisplacementLargeVec3.y;
                lNormalLargeVec3 = GetBlendedNormal(
                    lCustomUniforms,
                    lLargeTexCoordsVec3.zx,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.z,
                    lTileTextureIndicesVec4.w,
                    lfPatch,
                    lfSlope1,
                    lfSlope2,
                    lfTileType,
                    lSpecY,
                    lfSpecularCoeff,
                    largeDisplY,
                    GET_INDEX_LARGE(0, 1),
                    GET_INDEX_LARGE(1, 1),
                    GET_INDEX_LARGE(2, 1),
                    GET_INDEX_LARGE(3, 1),
                    GET_INDEX_LARGE(4, 1),
                    GET_INDEX_LARGE(5, 1),
                    GET_INDEX_LARGE(6, 1),
                    GET_INDEX_LARGE(7, 1));
                lSpecularLargeVec3.y = lSpecY;
                lDisplacementLargeVec3.y = largeDisplY;

#if defined ( D_PACKED_HEIGHTS )
                for (uint i = 0; i < 8; ++i)
                {
                    float lfX = UnpackHeightA(lHeightsLargeVec3[i].x);
                    float lfY = UnpackHeightB(lHeightsLargeVec3[i].y);
                    lHeightsLargeVec3[i].x = PackHeightsAB(lfX, lfY);
                }
#endif

#if !defined( D_DISABLE_NOISE )
                lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lLargeNoiseTexCoordsVec3.zx,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );
#endif

                lPlaneNorm                = half3( half2(lNormalVec3.zx), abs( lNormalVec3.y ) );
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 );
#if !defined( D_DISABLE_NOISE )
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale );
#endif
                lNormalLargeVec3.z       *= half( sign( lNormalVec3.y ) );
                lNormalLargeVec3          = lNormalLargeVec3.yzx;
                lLargeMappedNormalVec3   += half( lWeightsVec3N.y ) * lNormalLargeVec3;

#if !defined( D_DISABLE_NOISE )
                lSpecularLargeVec3.y      = saturate( lSpecularLargeVec3.y     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementLargeVec3.y  = saturate( lDisplacementLargeVec3.y + lfHeightNoise );
#endif
            }
            if ( lWeightsVec3N.z > kfWeightFloor )
            {

#define GET_INDEX_LARGE(e, c) lHeightsLargeVec3[e][c]

                float lSpecZ = lSpecularLargeVec3.z;
                float largeDisplZ = lDisplacementLargeVec3.z;
                lNormalLargeVec3 = GetBlendedNormal(
                    lCustomUniforms,
                    lLargeTexCoordsVec3.xy,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.z,
                    lTileTextureIndicesVec4.w,
                    lfPatch,
                    lfSlope1,
                    lfSlope2,
                    lfTileType,
                    lSpecZ,
                    lfSpecularCoeff,
                    largeDisplZ,
#if defined ( D_PACKED_HEIGHTS )
                    GET_INDEX_LARGE(0, 1),
                    GET_INDEX_LARGE(1, 1),
                    GET_INDEX_LARGE(2, 1),
                    GET_INDEX_LARGE(3, 1),
                    GET_INDEX_LARGE(4, 1),
                    GET_INDEX_LARGE(5, 1),
                    GET_INDEX_LARGE(6, 1),
                    GET_INDEX_LARGE(7, 1));
#else
                    GET_INDEX_LARGE(0, 2),
                    GET_INDEX_LARGE(1, 2),
                    GET_INDEX_LARGE(2, 2),
                    GET_INDEX_LARGE(3, 2),
                    GET_INDEX_LARGE(4, 2),
                    GET_INDEX_LARGE(5, 2),
                    GET_INDEX_LARGE(6, 2),
                    GET_INDEX_LARGE(7, 2));
#endif
                lSpecularLargeVec3.z = lSpecZ;
                lDisplacementLargeVec3.z = largeDisplZ;

//#if defined ( D_PACKED_HEIGHTS )
//                for (uint i = 0; i < 8; ++i)
//                {
//                    float lfZ = UnpackHeightB(lHeightsLargeVec3[i].y);
//                    lHeightsLargeVec3[i].y = PackHeightsAB(lfZ, lfZ);
//                }
//#endif

#if !defined( D_DISABLE_NOISE )
                lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lLargeNoiseTexCoordsVec3.xy,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );
#endif

                lPlaneNorm                = half3( half2(lNormalVec3.xy), abs( lNormalVec3.z ) );
                lNormalLargeVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalLargeVec3 );
#if !defined( D_DISABLE_NOISE )
                lNormalLargeVec3          = BlendNormalsUDN( lNormalLargeVec3, lNoiseNormalVec3, kfNoiseNormScale );
#endif
                lNormalLargeVec3.z       *= half( sign( lNormalVec3.z ) );
                lLargeMappedNormalVec3   += half( lWeightsVec3N.z ) * lNormalLargeVec3;

#if !defined( D_DISABLE_NOISE )
                lSpecularLargeVec3.z      = saturate( lSpecularLargeVec3.z     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementLargeVec3.z  = saturate( lDisplacementLargeVec3.z + lfHeightNoise );
#endif
            }
        }
        else
        {
            lfFade = 0.0;
        }

        if ( lfFade < 1.0 - lfFadeThreshold )
        {
            if ( lWeightsVec3N.x > kfWeightFloor )
            {

#define GET_INDEX_SMALL(e, c) lHeightsSmallVec3[e][c]

                float lSpecX = lSpecularSmallVec3.x;
                float lDisplX = lDisplacementSmallVec3.x;
                lNormalSmallVec3 = GetBlendedNormal(
                    lCustomUniforms,
                    lSmallTexCoordsVec3.yz,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.x,
                    lTileTextureIndicesVec4.y,
                    lfPatch,
                    lfSlope1,
                    lfSlope2,
                    lfTileType,
                    lSpecX,
                    lfSpecularCoeff,
                    lDisplX,
#if defined ( D_PACKED_HEIGHTS )
                    GET_INDEX_SMALL(0, 1),
                    GET_INDEX_SMALL(1, 1),
                    GET_INDEX_SMALL(2, 1),
                    GET_INDEX_SMALL(3, 1),
                    GET_INDEX_SMALL(4, 1),
                    GET_INDEX_SMALL(5, 1),
                    GET_INDEX_SMALL(6, 1),
                    GET_INDEX_SMALL(7, 1));
#else
                    GET_INDEX_SMALL(0, 0),
                    GET_INDEX_SMALL(1, 0),
                    GET_INDEX_SMALL(2, 0),
                    GET_INDEX_SMALL(3, 0),
                    GET_INDEX_SMALL(4, 0),
                    GET_INDEX_SMALL(5, 0),
                    GET_INDEX_SMALL(6, 0),
                    GET_INDEX_SMALL(7, 0));
#endif
                lSpecularSmallVec3.x = lSpecX;
                lDisplacementSmallVec3.x = lDisplX;

#if defined ( D_PACKED_HEIGHTS )
                for (uint i = 0; i < 8; ++i)
                {
                    float lfX = UnpackHeightB( lHeightsSmallVec3[i].y );
                    float lfY = UnpackHeightB( lHeightsSmallVec3[i].x );
                    lHeightsSmallVec3[i].x = PackHeightsAB( lfX, lfY );
                }
#endif

#if !defined( D_DISABLE_NOISE )
                lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lSmallNoiseTexCoordsVec3.yz,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );
#endif

                lPlaneNorm                = half3( half2(lNormalVec3.yz), abs( lNormalVec3.x ) );
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 );
#if !defined( D_DISABLE_NOISE )
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale );
#endif
                lNormalSmallVec3.z       *= half( sign( lNormalVec3.x ) );
                lNormalSmallVec3          = lNormalSmallVec3.zxy;
                lSmallMappedNormalVec3   += half( lWeightsVec3N.x ) * lNormalSmallVec3;

#if !defined( D_DISABLE_NOISE )
                lSpecularSmallVec3.x      = saturate( lSpecularSmallVec3.x     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementSmallVec3.x  = saturate( lDisplacementSmallVec3.x + lfHeightNoise );
#endif
            }
            if ( lWeightsVec3N.y > kfWeightFloor )
            {

#define GET_INDEX_SMALL(e, c) lHeightsSmallVec3[e][c]

                float lSpecY = lSpecularSmallVec3.y;
                float lDisplY = lDisplacementSmallVec3.y;
                lNormalSmallVec3 = GetBlendedNormal(
                    lCustomUniforms,
                    lSmallTexCoordsVec3.zx,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.x,
                    lTileTextureIndicesVec4.y,
                    lfPatch,
                    lfSlope1,
                    lfSlope2,
                    lfTileType,
                    lSpecY,
                    lfSpecularCoeff,
                    lDisplY,
                    GET_INDEX_SMALL(0, 1),
                    GET_INDEX_SMALL(1, 1),
                    GET_INDEX_SMALL(2, 1),
                    GET_INDEX_SMALL(3, 1),
                    GET_INDEX_SMALL(4, 1),
                    GET_INDEX_SMALL(5, 1),
                    GET_INDEX_SMALL(6, 1),
                    GET_INDEX_SMALL(7, 1));
                lSpecularSmallVec3.y = lSpecY;
                lDisplacementSmallVec3.y = lDisplY;

#if defined ( D_PACKED_HEIGHTS )
                for (uint i = 0; i < 8; ++i)
                {
                    float lfX = UnpackHeightA( lHeightsSmallVec3[i].x );
                    float lfY = UnpackHeightB( lHeightsSmallVec3[i].y );
                    lHeightsSmallVec3[i].x = PackHeightsAB( lfX, lfY );
                }
#endif

#if !defined( D_DISABLE_NOISE )
                lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lSmallNoiseTexCoordsVec3.zx,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );
#endif

                lPlaneNorm                = half3( half2(lNormalVec3.zx), abs( lNormalVec3.y ) );
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 );
#if !defined( D_DISABLE_NOISE )
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale );
#endif
                lNormalSmallVec3.z       *= half( sign( lNormalVec3.y ) );
                lNormalSmallVec3          = lNormalSmallVec3.yzx;
                lSmallMappedNormalVec3   += half( lWeightsVec3N.y ) * lNormalSmallVec3;

#if !defined( D_DISABLE_NOISE )
                lSpecularSmallVec3.y      = saturate( lSpecularSmallVec3.y     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementSmallVec3.y  = saturate( lDisplacementSmallVec3.y + lfHeightNoise );
#endif
            }
            if ( lWeightsVec3N.z > kfWeightFloor )
            {

#define GET_INDEX_SMALL(e, c) lHeightsSmallVec3[e][c]

                float lSpecZ = lSpecularSmallVec3.z;
                float lDisplZ = lDisplacementSmallVec3.z;
                lNormalSmallVec3 = GetBlendedNormal(
                    lCustomUniforms,
                    lSmallTexCoordsVec3.xy,
                    SAMPLER2DARRAYPARAM( lNormalMap ),
                    SAMPLER2DARRAYPARAM( lSubstanceNormalMap ),
                    lTileTextureIndicesVec4.x,
                    lTileTextureIndicesVec4.y,
                    lfPatch,
                    lfSlope1,
                    lfSlope2,
                    lfTileType,
                    lSpecZ,
                    lfSpecularCoeff,
                    lDisplZ,
#if defined ( D_PACKED_HEIGHTS )
                    GET_INDEX_SMALL(0, 1),
                    GET_INDEX_SMALL(1, 1),
                    GET_INDEX_SMALL(2, 1),
                    GET_INDEX_SMALL(3, 1),
                    GET_INDEX_SMALL(4, 1),
                    GET_INDEX_SMALL(5, 1),
                    GET_INDEX_SMALL(6, 1),
                    GET_INDEX_SMALL(7, 1));
#else
                    GET_INDEX_SMALL(0, 2),
                    GET_INDEX_SMALL(1, 2),
                    GET_INDEX_SMALL(2, 2),
                    GET_INDEX_SMALL(3, 2),
                    GET_INDEX_SMALL(4, 2),
                    GET_INDEX_SMALL(5, 2),
                    GET_INDEX_SMALL(6, 2),
                    GET_INDEX_SMALL(7, 2));
#endif
                lSpecularSmallVec3.z = lSpecZ;
                lDisplacementSmallVec3.z = lDisplZ;

//#if defined ( D_PACKED_HEIGHTS )
//                for (uint i = 0; i < 8; ++i)
//                {
//                    float lfZ = UnpackHeightB( lHeightsSmallVec3[i].y );
//                    lHeightsSmallVec3[i].y = PackHeightsAB( lfZ, lfZ );
//                }
//#endif

#if !defined( D_DISABLE_NOISE )
                lNoiseNormalVec3 = GetTriPlanarNoiseNormal(
                    lSmallNoiseTexCoordsVec3.xy,
                    SAMPLER2DPARAM( lNoiseNormalMap ),
                    lfHeightNoise, 
                    lfSpecularNoise );
#endif

                lPlaneNorm                = half3( half2(lNormalVec3.xy), abs( lNormalVec3.z ) );
                lNormalSmallVec3          = BlendNormalsRNM( lPlaneNorm,       lNormalSmallVec3 );
#if !defined( D_DISABLE_NOISE )
                lNormalSmallVec3          = BlendNormalsUDN( lNormalSmallVec3, lNoiseNormalVec3, kfNoiseNormScale );
#endif
                lNormalSmallVec3.z       *= half( sign( lNormalVec3.z ) );
                lSmallMappedNormalVec3   += half( lWeightsVec3N.z ) * lNormalSmallVec3;

#if !defined( D_DISABLE_NOISE )
                lSpecularSmallVec3.z      = saturate( lSpecularSmallVec3.z     - lfSpecularNoise * half( lfSpecularCoeff ) );
                lDisplacementSmallVec3.z  = saturate( lDisplacementSmallVec3.z + lfHeightNoise );
#endif
            }
        }
        else
        {
            lfFade = 1.0;
        }

        {
            for (int i = 0; i < 8; i++)
            {
#if defined ( D_PACKED_HEIGHTS )
                vec3 lHeightLarge;
                UnpackHeightsABC(lHeightsLargeVec3[i], lHeightLarge.x, lHeightLarge.y, lHeightLarge.z);
                vec3 lHeightSmall;
                UnpackHeightsABC(lHeightsSmallVec3[i], lHeightSmall.x, lHeightSmall.y, lHeightSmall.z);

                float lfHeightLarge = dot(lHeightLarge, lWeightsVec3) * lfWeightRecip;
                float lfHeightSmall = dot(lHeightSmall, lWeightsVec3) * lfWeightRecip;
                lafHeights[i] = mix(lfHeightSmall, lfHeightLarge, lfFade);
#else
                float lfHeightLarge = dot(vec3(lHeightsLargeVec3[i]), lWeightsVec3) * lfWeightRecip;
                float lfHeightSmall = dot(vec3(lHeightsSmallVec3[i]), lWeightsVec3) * lfWeightRecip;
                lafHeights[i] = mix(lfHeightSmall, lfHeightLarge, lfFade);
#endif
            }

            float lfDisplacementLarge = dot( lDisplacementLargeVec3, lWeightsVec3 ) * lfWeightRecip;
            float lfDisplacementSmall = dot( lDisplacementSmallVec3, lWeightsVec3 ) * lfWeightRecip;
            lfDisplacement = mix( lfDisplacementSmall, lfDisplacementLarge, lfFade );

            lfHeight = lfDisplacement;

            float lfWidth = 0.05;

            lafHeights[0] = FastSmoothStep( lafHeights[0] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights[1] = FastSmoothStep( lafHeights[1] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights[2] = FastSmoothStep( lafHeights[2] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights[3] = FastSmoothStep( lafHeights[3] - lfWidth, 2.0f * lfWidth, lfPatch );
            lafHeights[4] = FastSmoothStep( lafHeights[4] - lfWidth, 2.0f * lfWidth, lfSlope1 );
            lafHeights[5] = FastSmoothStep( lafHeights[5] - lfWidth, 2.0f * lfWidth, lfSlope2 );
            lafHeights[6] = FastSmoothStep( lafHeights[6] - lfWidth, 2.0f * lfWidth, lfTileType );
            lafHeights[7] = FastSmoothStep( lafHeights[7] - lfWidth, 2.0f * lfWidth, lfFade );

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
    }

    float lfFinalFade = lfFade;

#ifdef D_PLATFORM_GLSL

    vec3 lTileColourVec3 = vec3( 0.0, 0.0, 0.0 );

    {
        if ( lWeightsVec3N.x * lfFinalFade > kfWeightFloor )
        {
            lTileColourVec3 += lfFinalFade * lWeightsVec3N.x * GetBlendedColour( lCustomUniforms, lLargeTexCoordsVec3.yz, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), lTileTextureIndicesVec4.z, lTileTextureIndicesVec4.w, lafHeights[0], lafHeights[1], lafHeights[2], lafHeights[3], lafHeights[4], lafHeights[5], lafHeights[6], lfMetallic ).xyz;
        }
        if ( lWeightsVec3N.y * lfFinalFade > kfWeightFloor )
        {
            lTileColourVec3 += lfFinalFade * lWeightsVec3N.y * GetBlendedColour( lCustomUniforms, lLargeTexCoordsVec3.zx, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), lTileTextureIndicesVec4.z, lTileTextureIndicesVec4.w, lafHeights[0], lafHeights[1], lafHeights[2], lafHeights[3], lafHeights[4], lafHeights[5], lafHeights[6], lfMetallic ).xyz;
        }
        if ( lWeightsVec3N.z * lfFinalFade > kfWeightFloor )
        {
            lTileColourVec3 += lfFinalFade * lWeightsVec3N.z * GetBlendedColour( lCustomUniforms, lLargeTexCoordsVec3.xy, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), lTileTextureIndicesVec4.z, lTileTextureIndicesVec4.w, lafHeights[0], lafHeights[1], lafHeights[2], lafHeights[3], lafHeights[4], lafHeights[5], lafHeights[6], lfMetallic ).xyz;
        }

        if ( lWeightsVec3N.x * ( 1.0 - lfFinalFade ) > kfWeightFloor )
        {
            lTileColourVec3 += ( 1.0 - lfFinalFade ) * lWeightsVec3N.x * GetBlendedColour( lCustomUniforms, lSmallTexCoordsVec3.yz, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), lTileTextureIndicesVec4.x, lTileTextureIndicesVec4.y, lafHeights[0], lafHeights[1], lafHeights[2], lafHeights[3], lafHeights[4], lafHeights[5], lafHeights[6], lfMetallic ).xyz;
        }
        if ( lWeightsVec3N.y * ( 1.0 - lfFinalFade ) > kfWeightFloor )
        {
            lTileColourVec3 += ( 1.0 - lfFinalFade ) * lWeightsVec3N.y * GetBlendedColour( lCustomUniforms, lSmallTexCoordsVec3.zx, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), lTileTextureIndicesVec4.x, lTileTextureIndicesVec4.y, lafHeights[0], lafHeights[1], lafHeights[2], lafHeights[3], lafHeights[4], lafHeights[5], lafHeights[6], lfMetallic ).xyz;
        }
        if ( lWeightsVec3N.z * ( 1.0 - lfFinalFade ) > kfWeightFloor )
        {
            lTileColourVec3 += ( 1.0 - lfFinalFade ) * lWeightsVec3N.z * GetBlendedColour( lCustomUniforms, lSmallTexCoordsVec3.xy, SAMPLER2DARRAYPARAM( lDiffuseMap ), SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ), lTileTextureIndicesVec4.x, lTileTextureIndicesVec4.y, lafHeights[0], lafHeights[1], lafHeights[2], lafHeights[3], lafHeights[4], lafHeights[5], lafHeights[6], lfMetallic ).xyz;
        }
    }

#else // D_PLATFORM_GLSL

    // World space position is lTilePositionVec3

    vec3    lTileColourSmallVec3 = vec3( 0.0, 0.0, 0.0 );
    vec3    lTileColourLargeVec3 = vec3( 0.0, 0.0, 0.0 );
    {
        if ( lWeightsVec3N.x > kfWeightFloor )
        {
            lTileColourLargeVec3 += lWeightsVec3N.x * GetBlendedColour(
                lCustomUniforms,
                lLargeTexCoordsVec3.zy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lafHeights[4],
                lafHeights[5],
                lafHeights[6],
                lfMetallic ).xyz;

            lTileColourSmallVec3 += lWeightsVec3N.x * GetBlendedColour(
                lCustomUniforms,
                lSmallTexCoordsVec3.zy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lafHeights[4],
                lafHeights[5],
                lafHeights[6],
                lfMetallic ).xyz;
        }
        if ( lWeightsVec3N.y > kfWeightFloor )
        {
            lTileColourLargeVec3 += lWeightsVec3N.y * GetBlendedColour(
                lCustomUniforms,
                lLargeTexCoordsVec3.zx,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lafHeights[4],
                lafHeights[5],
                lafHeights[6],
                lfMetallic ).xyz;
            lTileColourSmallVec3 += lWeightsVec3N.y * GetBlendedColour(
                lCustomUniforms,
                lSmallTexCoordsVec3.zx,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lafHeights[4],
                lafHeights[5],
                lafHeights[6],
                lfMetallic ).xyz;
        }
        if ( lWeightsVec3N.z > kfWeightFloor )
        {
            lTileColourLargeVec3 += lWeightsVec3N.z * GetBlendedColour(
                lCustomUniforms,
                lLargeTexCoordsVec3.xy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.z,
                lTileTextureIndicesVec4.w,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lafHeights[4],
                lafHeights[5],
                lafHeights[6],
                lfMetallic ).xyz;
            lTileColourSmallVec3 += lWeightsVec3N.z * GetBlendedColour(
                lCustomUniforms,
                lSmallTexCoordsVec3.xy,
                SAMPLER2DARRAYPARAM( lDiffuseMap ),
                SAMPLER2DARRAYPARAM( lSubstanceDiffuseMap ),
                lTileTextureIndicesVec4.x,
                lTileTextureIndicesVec4.y,
                lafHeights[0],
                lafHeights[1],
                lafHeights[2],
                lafHeights[3],
                lafHeights[4],
                lafHeights[5],
                lafHeights[6],
                lfMetallic ).xyz;
        }
    }

    vec3 lTileColourVec3;


    lTileColourVec3 = mix( lTileColourSmallVec3, lTileColourLargeVec3, lfFade );
#endif // PLATFORM_GLSL

#ifndef D_CACHE_COLOUR
    lTileColourVec3 = GammaCorrectInput01( lTileColourVec3 );
#endif

    return lTileColourVec3;
}
#endif // D_STOCHASTIC_TERRAIN

#endif

#endif
