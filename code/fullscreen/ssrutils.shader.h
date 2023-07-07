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
#ifndef SSR_UTILS_H
#define SSR_UTILS_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUtils.shader.h"
#include "Fullscreen/SSRSettings.h"

//-----------------------------------------------------------------------------
///
///     SSR Utility Functions
///
//-----------------------------------------------------------------------------

#if !defined( D_PLATFORM_GLSL )
// TODO(sal) move these to a more general include file (Defines.shader.h???)
bool2 
lessThan(
    vec2 a,
    vec2 b )
{
    return a < b;
}
bool3 
lessThan(
    vec3 a,
    vec3 b )
{
    return a < b;
}

bool2 
greaterThan(
    vec2 a,
    vec2 b )
{
    return a > b;
}

bool3 
greaterThan(
    vec3 a,
    vec3 b )
{
    return a > b;
}
#endif

mat3
asmat3(
    mat4 mat )
{
    #if defined( D_PLATFORM_PC )
    return mat3( mat );
    #elif defined(D_PLATFORM_METAL)
    return cast2mat3(mat);
    #else
    return (mat3)mat;
    #endif
}

vec3 
sqr(
    vec3  value )
{ 
    return value * value;
}

float
sqr_dist(
    vec3 a,
    vec3 b )
{
    return dot( a - b, a - b );
}

float
sqr(
    float value )
{
    return value * value; 
}

float
Luminance(
    vec3 lColour )
{
    return RGBToYCgCo( lColour ).x;
}

float
LuminanceFast(
    vec3 lColour )
{
    return dot( lColour, vec3( 0.2126, 0.7152, 0.0722 ) );
}

bool
CheckInside(
    vec2 lUVsVec2 )
{
    return all( greaterThan( lUVsVec2.xy, float2vec2( 0.0 ) ) ) && 
           all( lessThan(    lUVsVec2.xy, float2vec2( 1.0 ) ) );
}

float
GetEdgeStoppingRoughnessWeight(
    float roughness_p,
    float roughness_q,
    float sigma_min,
    float sigma_max )
{
    return sqrt( saturate( 1.0 - smoothstep( sigma_min, sigma_max, abs( roughness_p - roughness_q ) ) ) );
}

vec4
GetUVCoordsFromView(
    in PerFrameUniforms lPerFrameUniforms,
    in vec3             lPosViewVec3 )
{
    vec4    lUVsVec4    = vec4( lPosViewVec3, 1.0 );
    lUVsVec4            = MUL( lPerFrameUniforms.gProjectionMat4, lUVsVec4 );
    lUVsVec4.xyz       /= lUVsVec4.w;
    lUVsVec4.xy         = SCREENSPACE_AS_RENDERTARGET_UVS( lUVsVec4.xy * 0.5 + 0.5 );

    return lUVsVec4;
}

vec3 
FragToUVsSpace(
    in PerFrameUniforms lPerFrameUniforms,
    in vec2             lTexSizeVec2,
    in vec3             lPosFragVec3 )
{
    vec3 lPosUVsVec3;
    lPosUVsVec3.xy  = SCREENSPACE_AS_RENDERTARGET_UVS( lPosFragVec3.xy / lTexSizeVec2 );
    lPosUVsVec3.z   = lPosFragVec3.z;

    return lPosUVsVec3;
}


// Building an Orthonormal Basis, Revisited
// http://www.jcgt.org/published/0006/01/01/
mat3 
CreateTBN(
    vec3 Z )
{
    const float sign    = Z.z >= 0 ? 1 : -1;
    const float a       = -1.0 / ( sign + Z.z );
    const float b       = Z.x * Z.y * a;
    vec3  X             = vec3( 1.0 + sign * Z.x * Z.x * a, sign * b, -sign * Z.x );
    vec3  Y             = vec3( b, sign + Z.y * Z.y * a, -Z.y );

    return mat3(X, Y, Z);
}

//-----------------------------------------------------------------------------
///
///     SSR Sampling Methods
///
//-----------------------------------------------------------------------------

uvec3 
Rand3DPCG16(
    ivec3 p )
{
    uvec3 v = uvec3( p );

    v = v * 1664525u + 1013904223u;
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;
    v.x += v.y*v.z;
    v.y += v.z*v.x;
    v.z += v.x*v.y;

    return v >> 16u;
}

vec2 
Rand1SPPDenoiserInput(
    uvec2   PixelPos, 
    uint    luFrameIdx )
{
    vec2 E;

    uvec2 Random = Rand3DPCG16( ivec3( PixelPos.x, PixelPos.y, int(luFrameIdx) ) ).xy;
    E = vec2( Random ) / 65536.0; // equivalent to Hammersley16(0, 1, Random).
 
    return E;
}

vec4 
ImportanceSampleGGX(
    vec2    lXi,
    float   a2 )
{
    float Phi      = 2 * M_PI * lXi.x;
    float CosTheta = sqrt( (1 - lXi.y) / ( 1 + (a2 - 1) * lXi.y ) );
    float SinTheta = sqrt(  1 - CosTheta * CosTheta );
    
    vec3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    
    float d = ( CosTheta * a2 - CosTheta ) * CosTheta + 1;
    float D = a2 / ( M_PI*d*d );
    float PDF = D * CosTheta;
    
    return vec4( H, PDF );
}

// Blue Noise Sampler by Eric Heitz. Returns a value in the range [0, 1].
vec2
SampleBlueNoise( 
    uvec2               pixel,
    uint                sample_index,
    uint                sample_seq_size_log,
    uint                dimension,
    SAMPLER2DARG(       sobol_map ),
    SAMPLER2DARRAYARG(  scramble_array_map ),
    SAMPLER2DARRAYARG(  rank_array_map ) )
{   
    // Wrap arguments
    pixel.x             = pixel.x & 127;
    pixel.y             = pixel.y & 127;
    sample_index        = sample_index & 255;
    dimension           = dimension & 0xfe;

    ivec2 sobol_coords;
    ivec3 tile_coords;

    uvec2 sobol;
    uvec2 scramble;

    uint  rank;
    uint  ranked_smp_idx;

    vec2  value;

    tile_coords     = ivec3( pixel.x, pixel.y, int(sample_seq_size_log) );

    rank            = uint(  textureLoadArrayF( rank_array_map,     tile_coords, 0 ).r  * 255.0 );
    scramble        = uvec2( textureLoadArrayF( scramble_array_map, tile_coords, 0 ).rg * 255.0 );

    // xor index based on optimized ranking
    ranked_smp_idx  = sample_index ^ rank;

    sobol_coords.x  = int(dimension);
    sobol_coords.y  = int(ranked_smp_idx);
    sobol.x         = uint( textureLoadF( sobol_map, sobol_coords, 0 ).r  * 255.0 );

    sobol_coords.x  = int(dimension + 1);
    sobol.y         = uint( textureLoadF( sobol_map, sobol_coords, 0 ).r  * 255.0 );
    
    value           = vec2( sobol ^ scramble );

    // Convert to float and return
    return ( value + 0.5f ) / 256.0f;
}

vec2 
SampleRandomVec2(
    uvec2               pixel,
    uint                seq_idx,
    uint                seq_size_log,
    uint                dimension,
    SAMPLER2DARG(       sobol_map ),
    SAMPLER2DARRAYARG(  scramble_array_map ),
    SAMPLER2DARRAYARG(  rank_array_map ) )
{
    vec2    u   = SampleBlueNoise(  pixel, seq_idx, seq_size_log, dimension,
                                    SAMPLER2DPARAM( sobol_map ),
                                    SAMPLER2DARRAYPARAM( scramble_array_map ),
                                    SAMPLER2DARRAYPARAM( rank_array_map ) );

            u   = vec2( fract( u.x + (seq_idx & 0xFFFF) * M_GOLDEN_RATIO ),
                        fract( u.y + (seq_idx & 0xFFFF) * M_GOLDEN_RATIO ) );

    return u;
}

vec2 
SampleDisk(
    vec2 Xi )
{
    float   phi     = 2 * M_PI * Xi.x;
    float   radius  = sqrt(Xi.y);

    return  radius * vec2( cos( phi ), sin( phi ) );
}

vec2 
SampleDisk(
    vec2  Xi,
    float r )
{
    float   phi     = 2 * M_PI * Xi.x;
    float   radius  = sqrt(Xi.y) + r;


    return  radius * vec2( cos( phi ), sin( phi ) );
}

// Smith term for GGX
// [Smith 1967, "Geometrical shadowing of a random rough surface"]
float 
Vis_Smith(
    float a2,
    float NoV,
    float NoL )
{
    float Vis_SmithV = NoV + sqrt( NoV * ( NoV - NoV * a2 ) + a2 );
    float Vis_SmithL = NoL + sqrt( NoL * ( NoL - NoL * a2 ) + a2 );
    return 1.0 / ( Vis_SmithV * Vis_SmithL );
}

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float
D_GGX( 
    float a2, 
    float NoH )
{
    float d = ( NoH * a2 - NoH ) * NoH + 1; // 2 mad
    return a2 / ( M_PI * d*d );             // 4 mul, 1 rcp
}

vec3 
EnvBRDFApprox( 
    vec3 SpecularColor,
    float Roughness,
    float NoV )
{
    // [ Lazarov 2013, "Getting More Physical in Call of Duty: Black Ops II" ]
    // Adaptation to fit our G term.
    const vec4  c0   = vec4( -1, -0.0275, -0.572, 0.022 );
    const vec4  c1   = vec4( 1, 0.0425, 1.04, -0.04 );
    const vec4  r    = Roughness * c0 + c1;
    const float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
    const vec2  AB   = vec2( -1.04, 1.04 ) * a004 + r.zw;
    
    return SpecularColor * AB.x + AB.y;
}

// http://jcgt.org/published/0007/04/01/paper.pdf by Eric Heitz
// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
vec3
SampleGGX_VNDF(
    vec3  Ve,
    float alpha_x,
    float alpha_y,
    float U1, 
    float U2 )
{
    // Section 3.2: transforming the view direction to the hemisphere configuration
    vec3  Vh    = normalize( vec3( alpha_x * Ve.x, alpha_y * Ve.y, Ve.z ) );
    
    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3  T1    = lensq > 0 ? vec3( -Vh.y, Vh.x, 0 ) * invsqrt( lensq ) : vec3( 1, 0, 0 );
    vec3  T2    = cross( Vh, T1 );
    
    // Section 4.2: parameterization of the projected area
    float r     = sqrt( U1 );
    float phi   = 2.0 * M_PI * U2;
    float t1    = r * cos( phi );
    float t2    = r * sin( phi );
    float s     = 0.5 * ( 1.0 + Vh.z );
    t2          = ( 1.0 - s ) * sqrt( 1.0 - t1 * t1 ) + s * t2;
    
    // Section 4.3: reprojection onto hemisphere
    vec3 Nh     = t1 * T1 + t2 * T2 + sqrt( max( 0.0, 1.0 - t1 * t1 - t2 * t2 ) ) * Vh;

    // Section 3.4: transforming the normal back to the ellipsoid configuration
    vec3 Ne     = normalize( vec3( alpha_x * Nh.x, alpha_y * Nh.y, max( 0.0, Nh.z ) ) );

    return Ne;
}

vec3 
Sample_GGX_VNDF_Ellipsoid( 
    vec3   Ve,
    float  alpha_x,
    float  alpha_y,
    float  U1,
    float  U2 )
{
    return SampleGGX_VNDF( Ve, alpha_x, alpha_y, U1, U2 );
}

vec3 
Sample_GGX_VNDF_Hemisphere( 
    vec3  Ve,
    float alpha,
    float U1,
    float U2 )
{
    return Sample_GGX_VNDF_Ellipsoid( Ve, alpha, alpha, U1, U2 );
}

float
Compute_GGX_VNDF_rcpPDF(
    vec3  Ve,
    vec3  Ne,
    float alpha )
{
    float Ns_dot_Ve = Ve.z;
    float Ns_dot_Ne = Ne.z;
    float Ve_dot_Ne = dot( Ve, Ne );

    float a2        = alpha * alpha;
    float d         = ( Ns_dot_Ne * a2 - Ns_dot_Ne ) * Ns_dot_Ne + 1.0;
    float D         = a2 / ( M_PI*d*d );

    /*
    float G_SmithV  = 2.0 * Ns_dot_Ve / ( Ns_dot_Ve + sqrt( Ns_dot_Ve * ( Ns_dot_Ve  - Ns_dot_Ve  * a2 ) + a2 ) );
    float rcpPDF    = 4.0 * Ns_dot_Ve * Ve_dot_Ne / ( G_SmithV * Ve_dot_Ne * D );
    */

    // G_Smith denominator
    float G_SmithVDen   = Ns_dot_Ve + sqrt( Ns_dot_Ve * ( Ns_dot_Ve - Ns_dot_Ve * a2 ) + a2 );
    float rcpPDF        = 2.0 * G_SmithVDen / D;
    
    return rcpPDF;
}

float
ComputeDynamicBias(
    float lfBiasMin,
    float lfBiasMax,
    float lfVariance,
    float lfRoughness )
{
    float lfVarScale        = mix( 1.0, 0.05, sqrt( lfVariance ) );
    float lfRoughScale      = smoothstep( MIRROR_THRESHOLD, 1.0, pow( lfRoughness, 0.75 ) );
    float lfBias            = mix( lfBiasMax, lfBiasMin, lfRoughScale * lfVarScale );
    return lfBias;
}

vec3 
SampleMicrofacetNormal(
    SAMPLER2DARG(       sobol_map ),
    SAMPLER2DARRAYARG(  scramble_array_map ),
    SAMPLER2DARRAYARG(  rank_array_map ),
    in  vec3            lNormalVec3,
    in  vec2            lPixelCoordsVec2,
    in  vec3            lViewDirVec3,
    in  float           lfRoughness,
    in  float           lfVariance,
    in  int             liFrameIndex,
    out float           lfPDF )
{    
    float lfAlpha           = lfRoughness * lfRoughness;
    float lfBias            = ComputeDynamicBias( GGX_BIAS_MIN, GGX_BIAS_MAX, lfVariance, lfRoughness );

    mat3  lTBNMat3          = CreateTBN( lNormalVec3 );
    vec3  lViewTBNVec3      = MUL( lViewDirVec3, lTBNMat3 );
    
    vec3  lMNormVec3;
    vec4  lMNormTBNPDFVec4;
    bool  lbValidMNorm      = false;

    for ( int ii = 0; ii < GGX_RUNS_MAX; ++ii )
    {
        #if 0
        vec2  lXi               = SampleRandomVec2( 
                                    uvec2(lPixelCoordsVec2),
                                    uint(liFrameIndex * GGX_RUNS_MAX + ii), 8, 0,
                                    SAMPLER2DPARAM(      sobol_map ),
                                    SAMPLER2DARRAYPARAM( scramble_array_map ),
                                    SAMPLER2DARRAYPARAM( rank_array_map ) );
        #else
        vec2  lXi               = Rand1SPPDenoiserInput( 
                                    uvec2( lPixelCoordsVec2), 
                                    uint( liFrameIndex * GGX_RUNS_MAX + ii ) );
        #endif

        #if defined( GGX_VNDF )
        vec3 lMNormTBNVec3      = Sample_GGX_VNDF_Hemisphere(
                                            lViewTBNVec3,
                                            lfAlpha,
                                            lXi.x, lXi.y );
        lfRcpPDF                = Compute_GGX_VNDF_rcpPDF( lViewTBNVec3, lMNormTBNVec3, lfAlpha );
        #endif

        lXi.y                   = mix( lXi.y, 0.0, lfBias );
        lMNormTBNPDFVec4        = ImportanceSampleGGX( lXi, lfAlpha * lfAlpha );

        lbValidMNorm            = dot( lMNormTBNPDFVec4.xyz, lViewTBNVec3 ) > 0.0;

        if ( lbValidMNorm ) break;
    }

    lMNormTBNPDFVec4    = !lbValidMNorm ? vec4( 0.0, 0.0, 1.0, M_1_PI / lfAlpha  ) : lMNormTBNPDFVec4;
    lfPDF               = lMNormTBNPDFVec4.w;
    lMNormVec3          = MUL( lTBNMat3, lMNormTBNPDFVec4.xyz );

    return lMNormVec3;
}

//-----------------------------------------------------------------------------
///
///     SSR March Functions
///
//-----------------------------------------------------------------------------

// Intersection function based on:
// https://sakibsaikia.github.io/graphics/2016/12/26/Screen-Space-Reflection-in-Killing-Floor-2.html
// most other intersection samples in the wild end up causing precision issues
// which in turns causes incorrect marching and blocking artifacts
vec3 
IntersectCellBoundary(
    vec3  r,
    vec3  d,
    float eps,
    vec2  lCellIndexVec2,
    vec2  lCellCountVec2 )
{
    vec3 RaySample = r;
    vec3 RayDir    = d;

    // Size of current mip 
    vec2 MipSize      = lCellCountVec2;

    // UV converted to index in the mip
    vec2 MipCellIndex = lCellIndexVec2;

    vec2 Eps;
    Eps.x = eps * max( MipSize.x, MipSize.y ) * ( RayDir.x > 0 ? 1.0 : -1.0 );
    Eps.y = eps * max( MipSize.x, MipSize.y ) * ( RayDir.y > 0 ? 1.0 : -1.0 );

    //
    // Find the cell boundary UV based on the direction of the ray
    // Take floor() or ceil() depending on the sign of RayDir.xy
    //
    vec2 BoundaryUV;
    BoundaryUV.x  = RayDir.x > 0 ? ceil( MipCellIndex.x ) : floor( MipCellIndex.x );
    BoundaryUV.y  = RayDir.y > 0 ? ceil( MipCellIndex.y ) : floor( MipCellIndex.y );
    BoundaryUV   += Eps;

    //
    // We can now represent the cell boundary as being formed by the intersection of 
    // two lines which can be represented by 
    //
    // x = BoundaryUV.x
    // y = BoundaryUV.y
    //
    // Intersect the parametric equation of the Ray with each of these lines
    //
    vec2 t = ( BoundaryUV - RaySample.xy * MipSize ) / ( MipSize * RayDir.xy );

    // Pick the cell intersection that is closer, and march to that cell	
    int idx    = int( abs(t.x) > abs(t.y) );	
    RaySample += t[idx] * RayDir;

    return RaySample;
}

float 
GetHiZSample(
    in PerFrameUniforms lPerFrameUniforms,
    SAMPLER2DARG(       lgHiZDepthMap ),
    vec2                lUVsVec2,
    int                 liHiZMipLevel )
{    
    vec4    lHiZDepthBuffVe4    = texture2DLod( lgHiZDepthMap, lUVsVec2, float( liHiZMipLevel ) );
    float   lfHiZDepth = DecodeDepthFromColour(lHiZDepthBuffVe4);
    return  lfHiZDepth;
}

vec2 
GetCell( 
    vec2 lRayUVsVec2,
    vec2 lCellCountVec2 )
{
    return lRayUVsVec2 * vec2( lCellCountVec2 );
}


vec2 
GetCellCount(
    SAMPLER2DARG( lgHiZDepthMap ),
    int liHiMipLevel )
{
    return vec2( GetTexResolutionLod( lgHiZDepthMap, liHiMipLevel ) );
}

bool 
CrossedCellBoundary(
    vec2 lCellIdxOneVec2,
    vec2 lCellIdxTwoVec2 )
{
    return  int(floor(lCellIdxOneVec2.x)) != int(floor(lCellIdxTwoVec2.x)) ||
            int(floor(lCellIdxOneVec2.y)) != int(floor(lCellIdxTwoVec2.y));
}

float 
GetThickness(
    float lfDepthRay,
    float lfRoughnessBias )
{
    // Nothing special going on here; just a heuristic I came up with
    // to scale thickness based on the ray's depth
    float lfThickness   = lfDepthRay * lfDepthRay * lfDepthRay * HI_Z_THICKNESS_SCALE;
    return clamp( lfThickness, HI_Z_THICKNESS_MIN, HI_Z_THICKNESS_MAX ) * lfRoughnessBias;
}

float
GetThicknessRoughnessBias(
    float lfRoughness )
{
    return max( 1.0, lfRoughness * lfRoughness * lfRoughness * HI_Z_THICKNESS_ROUGH_BIAS );
}

vec3
InitialStep(
    SAMPLER2DARG( lgHiZDepthMap ),
    vec3  lRayPUVsVec3,
    vec3  lRayDUVsNormVec3,
    float lfEps )
{
    vec2  lCellCountVec2    = GetCellCount( SAMPLER2DPARAM( lgHiZDepthMap ), 0 );
    vec2  lOldCellIdxVec2   = GetCell( lRayPUVsVec3.xy, lCellCountVec2 );
    float lfShiftedEps      = lfEps + 1.0 / min( lCellCountVec2.x, lCellCountVec2.y );
    vec3  lCrossRayPUVsVec3 = IntersectCellBoundary( lRayPUVsVec3, lRayDUVsNormVec3, lfShiftedEps, lOldCellIdxVec2, lCellCountVec2 );
    
    return lCrossRayPUVsVec3;
}

void
GetRay(
    in  vec4  lStartPosUVsVec4,
    in  vec4  lEndPosUVsVec4,
    out vec3  lRayPUVsVec3,
    out vec3  lRayDUVsNormVec3,
    out float lfRayDLength )
{    
    vec3 lRayDUVsVec3;

    lRayPUVsVec3    = lStartPosUVsVec4.xyz;
    lRayDUVsVec3    = lEndPosUVsVec4.xyz - lRayPUVsVec3;
    lRayDUVsVec3    = lRayDUVsVec3.xyz / abs( lRayDUVsVec3.z );

    // scale vector such that z is -1.0f (maximum depth range)
    lfRayDLength        = length( lRayDUVsVec3 );
    lRayDUVsNormVec3    = lRayDUVsVec3 / lfRayDLength;
}

// Based on:
// GPU Pro 1 - 1.4.2, pag. 127 - Quadtree Displacement Mapping with Height Blending - Michal Drobot
// GPU Pro 5 - 4.5.3, pag. 174 - Hi-Z Screen-Space Cone-Traced Reflections          - Yasin Uludag
bool
RunHiZMarch(
    in PerFrameUniforms lPerFrameUniforms,
    SAMPLER2DARG(   lgHiZDepthMap ),
    SAMPLER2DARG(   lgLowZDepthMap ),
    in   float      lfRoughness,
    in   vec4       lStartPosUVsVec4,
    in   vec4       lEndPosUVsVec4,
    out  vec4       lHitPosFragVec4 )
    //out  float      lfIterations )
{
    const vec2  kHiZTexSizeVec2 = vec2( GetTexResolutionLod( lgHiZDepthMap, 0 ) );
    const float kfHiZEps        = HI_Z_EPS / max( kHiZTexSizeVec2.x, kHiZTexSizeVec2.y );

    bool  lbHit         = true;

    // allow for higher thickness on rougher reflections
    float lfRoughBias   = GetThicknessRoughnessBias( lfRoughness );

    uint  luIterations  = 0;
    int   liHiZMiplevel = HI_Z_START_LEVEL;

    float lfRayDLength;
    vec3  lRayPUVsVec3, lRayDUVsNormVec3;
    GetRay( lStartPosUVsVec4, lEndPosUVsVec4, lRayPUVsVec3, lRayDUVsNormVec3, lfRayDLength );
    
    lRayPUVsVec3 = InitialStep( SAMPLER2DPARAM( lgHiZDepthMap ), lRayPUVsVec3, lRayDUVsNormVec3, kfHiZEps );

    for (; luIterations < HI_Z_MAX_STEPS; ++luIterations )
    {
        // get the cell number of the current ray
        const vec2 lCellCountVec2   = GetCellCount( SAMPLER2DPARAM( lgHiZDepthMap ), liHiZMiplevel );
        vec2       lOldCellIdxVec2  = GetCell( lRayPUVsVec3.xy, lCellCountVec2 );

        // get the minimum depth plane in which the current ray resides
        float lfHiZ                 = GetHiZSample( lPerFrameUniforms, SAMPLER2DPARAM( lgHiZDepthMap ), lRayPUVsVec3.xy, liHiZMiplevel );
        
        bool  lbShouldCross         = false;

        if ( lRayPUVsVec3.z > lfHiZ )
        {
            vec3 lRayPUVsCachedVec3 = lRayPUVsVec3;
            lRayPUVsVec3            = lRayPUVsVec3 + lRayDUVsNormVec3 * lfRayDLength * ( lRayPUVsVec3.z - lfHiZ );

            // get the new cell number as well
            vec2 lNewCellIdxVec2    = GetCell( lRayPUVsVec3.xy, lCellCountVec2 );
            lbShouldCross           = CrossedCellBoundary( lOldCellIdxVec2, lNewCellIdxVec2 );
            lRayPUVsVec3            = lbShouldCross ? lRayPUVsCachedVec3 : lRayPUVsVec3;
        }
        else
        {
            float lfLowZ            = GetHiZSample( lPerFrameUniforms, SAMPLER2DPARAM( lgLowZDepthMap ), lRayPUVsVec3.xy, liHiZMiplevel );
            lfLowZ                 -= GetThickness( lRayPUVsVec3.z, lfRoughBias );
            lbShouldCross           = lRayPUVsVec3.z < lfLowZ;
        }

        // if the new cell number is different from the old cell number, or below low z, a cell was crossed
        if ( lbShouldCross )
        {
            // intersect the boundary of that cell instead, and go up a level for taking a larger step next iteration
            lRayPUVsVec3   = IntersectCellBoundary( lRayPUVsVec3, lRayDUVsNormVec3, kfHiZEps, lOldCellIdxVec2, lCellCountVec2 );
            liHiZMiplevel += 2;
        }

        liHiZMiplevel = min( HI_Z_MAX_LEVEL, liHiZMiplevel - 1 );
        lbHit         = CheckInside( lRayPUVsVec3.xy );

        if ( liHiZMiplevel < HI_Z_STOP_LEVEL || !lbHit ) break;
    }

    lbHit           = lbHit && luIterations < HI_Z_MAX_STEPS;
    lHitPosFragVec4 = vec4( lRayPUVsVec3, 1.0 );

    //lfIterations    = saturate( luIterations / float( HI_Z_MAX_STEPS ) );

    return lbHit;
}

bool
RunMarch(
    in  PerFrameUniforms    lPerFrameUniforms,
    SAMPLER2DARG(           lgDepthHighZMap ),
    SAMPLER2DARG(           lgDepthLowZMap ),
    in  float               lfRoughness,
    in  vec3                lPosStartViewVec3,
    in  vec3                lPosEndViewVec3,
    out vec2                lUVHitVec2 )
{    
    // Store whether we hit anything along the high-z march
    bool    lbHitHiZPass    = false;

    // Transform start and end point to screen space.
    vec4    lStartUVsVec4   = GetUVCoordsFromView( lPerFrameUniforms, lPosStartViewVec3 );
    vec4    lEndUVsVec4     = GetUVCoordsFromView( lPerFrameUniforms, lPosEndViewVec3   );
    
    vec4    lHiZUVsVec4;
    {
        lbHitHiZPass        = RunHiZMarch(  lPerFrameUniforms,
                                            SAMPLER2DPARAM( lgDepthHighZMap ),
                                            SAMPLER2DPARAM( lgDepthLowZMap ),
                                            lfRoughness,
                                            lStartUVsVec4,  lEndUVsVec4, lHiZUVsVec4 );
    }

    lUVHitVec2              = lHiZUVsVec4.xy;

    return lbHitHiZPass;
}

//-----------------------------------------------------------------------------
///
///     SSR Radiance Functions
///
//-----------------------------------------------------------------------------
float 
CalculateRadiusIsoscelesTriangleCone(
    float lfConeBaseLength, 
    float lfConeHeightLength )
{
    float   b       = lfConeBaseLength;
    float   h       = lfConeHeightLength;
    float   b_sqr   = b * b;
    float   h_sqr_4 = h * h * 4.0;
    float   num     = b * ( sqrt(b_sqr + h_sqr_4 ) - b );
    float   denom   = h * 4.0;
    
    return num / denom;
}

vec4 
ConeSampleWeightedColor(
    SAMPLER2DARG(   lDiffuseMap ),
    vec2            lSamplePosVec2, 
    float           lfMipChannel, 
    float           lfGloss )
{
    vec3 lPreIntegratedColourVec3 = GammaCorrectInput( texture2DLod( lDiffuseMap, lSamplePosVec2, lfMipChannel ).rgb );
    return vec4( lPreIntegratedColourVec3 * lfGloss, lfGloss);
}

float 
ShortenConeLengthByDiameter(
    float lfAdjacentLength, 
    float lfDiameter )
{
    // subtract the diameter of the in-circle to get the next cone length
    return lfAdjacentLength - lfDiameter;
}

float 
SpecularPowerToConeAngle( 
    float lfSpecularPower )
{
    // based on phong distribution model
    // GPU Pro 5 pag. 168    
    const float kfXi    = 0.244;
    float       lfExp   = 1.0 / ( lfSpecularPower + 1.0 );
    float       lfAngle = acos( pow( kfXi, lfExp ) );
    
    return      mix( lfAngle, 0.0, saturate( lfExp / 1024.0 - 1.0 ) );
}

float 
RoughnessToSpecularPower(
    float lfRoughness )
{
    return 2.0 / pow( lfRoughness, 4.0 ) - 2.0;
}

float
CalculateMipBias(
    vec2  lStartUVsVec2,
    vec2  lEndUVsVec2,
    vec2  lTexSizeVec2,
    float lfRoughness )
{

    float lfSpecularPower       = RoughnessToSpecularPower( lfRoughness * lfRoughness );
    float lfConeTanHalfTheta    = tan( SpecularPowerToConeAngle( lfSpecularPower ) * 0.5 );
    float lfConeHeightLength    = length( ( lEndUVsVec2 - lStartUVsVec2 ) * lTexSizeVec2 );
    float lfConeBaseLenght      = 2.0 * lfConeTanHalfTheta * lfConeHeightLength;
    float lfRadius              = CalculateRadiusIsoscelesTriangleCone( lfConeBaseLenght, lfConeHeightLength );
    float lfMipBias             = clamp( log( lfRadius + 1.0 ) * MIP_BIAS_FACTOR + 0.7, 0.0, MIP_BIAS_MAX );

    return lfMipBias;
}

float
CalculateLocalMipBias(
    float lfRayLength,
    float lfRoughness )
{
    float lfSpecularPower       = RoughnessToSpecularPower( lfRoughness * lfRoughness );
    float lfConeTanHalfTheta    = tan( SpecularPowerToConeAngle( lfSpecularPower ) * 0.5 );
    float lfConeHeightLength    = mix( 0.0, 512.0, lfRoughness * lfRoughness );
    float lfConeBaseLenght      = 2.0 * lfConeTanHalfTheta * lfConeHeightLength;
    float lfRadius              = CalculateRadiusIsoscelesTriangleCone( lfConeBaseLenght, lfConeHeightLength );
    float lfMipBias             = clamp( log( lfRadius + 1.0 ) * MIP_BIAS_FACTOR_LOCAL + 0.7, 0.0, MIP_BIAS_LOCAL_MAX );

    return lfMipBias;
}

#if defined( D_SSR_USES_PROBES )

#if defined( D_LOCAL_REFLECTIONS )

struct AABB
{
    vec3 box_min;
    vec3 box_max;
    int  box_idx;
    
    vec3 pix_pos;
    vec3 pix_dir;
};

AABB
BuildAABB(
    vec3 pos,
    vec3 dir,
    vec3 extent,
    int  idx )
{
    AABB aabb;
    
    extent /= 2.0;
    
    aabb.pix_pos    = pos;
    aabb.pix_dir    = dir;
    aabb.box_min    = -extent;
    aabb.box_max    = +extent;
    aabb.box_idx    = idx;

    return aabb;
}


bool
ValidAABB(
    AABB aabb )
{
    bool valid = aabb.box_max.x - aabb.box_min.x > AABB_MIN_EXTENT  &&
                 all( greaterThan( aabb.pix_pos, aabb.box_min ) )   &&
                 all( lessThan(    aabb.pix_pos, aabb.box_max ) );
    
    return valid;
}


AABB
GetClosestAABB(
    PerFrameUniforms        lPerFrame,
    CustomPerMeshUniforms   lCustomPerMesh,
    vec3                    pix_pos,
    vec3                    pix_dir )
{
    AABB    aabb_closest;
    float   dist_closest;

    dist_closest            = 3.4e38;
    aabb_closest.box_idx    = -1;

    for ( int ii = 0; ii < D_MAX_PROBES_COUNT; ++ii )
    {
        vec3  aabb_pos      = lCustomPerMesh.gaProbePositionsVec4[ ii ].xyz;
        vec3  aabb_extent   = lCustomPerMesh.gaProbeExtentsVec4  [ ii ].xyz;
        mat3  aabb_mat      = asmat3( lCustomPerMesh.gaProbeMat4 [ ii ] );
        vec3  aabb_pos_diff = pix_pos - aabb_pos;
        vec3  aabb_pix_pos  = MUL( aabb_mat, aabb_pos_diff );
        float aabb_dist     = dot( aabb_pix_pos, aabb_pix_pos );
        AABB  aabb          = BuildAABB( aabb_pix_pos, float2vec3( 0.0 ), aabb_extent, ii );
        bool  valid         = ValidAABB( aabb );
        
        if ( valid && aabb_dist < dist_closest )
        {
            dist_closest    = aabb_dist;
            aabb_closest    = aabb;
        }
    }

    mat3  aabb_mat       = asmat3( lCustomPerMesh.gaProbeMat4[ aabb_closest.box_idx ] );
    aabb_closest.pix_dir = MUL( aabb_mat, pix_dir );

    return aabb_closest;
}

float
GetAABBFade(
    AABB aabb )
{
    vec3  diff      = min( abs( aabb.pix_pos - aabb.box_min ), abs( aabb.pix_pos - aabb.box_max ) );
    float edge_dist = min( min( diff.x, diff.y ), diff.z );
    float fade      = saturate( edge_dist / AABB_FADE_DIST );

    return fade * fade;
}

vec3
CorrectLocalCubemap(
    AABB aabb )
{
    // Find the ray intersection with box plane
    vec3 inv_dir        = 1.0 / aabb.pix_dir;
    vec3 t_plane_min    = (aabb.box_min - aabb.pix_pos) * inv_dir;
    vec3 t_plane_max    = (aabb.box_max - aabb.pix_pos) * inv_dir;

    // Get largest intersection values (we are not interested in negative values)
    vec3 t_max          = max(t_plane_min, t_plane_max);

    // Get closest
    float t            = min(min( t_max.x, t_max.y), t_max.z);

    // Get the intersection position
    vec3 hit_pos       = aabb.pix_pos + aabb.pix_dir * t;

    // Calculate corrected version
    vec3 local_dir     = hit_pos * vec3( -1.0, 1.0, -1.0 );

    return local_dir;
}
vec3
SampleFromCubemap(
    vec3    lDirVec3,
    int     liIdx,
    float   lfMip,
    PROBE_ARGS )
{
    vec3    lColourVec3 = float2vec3( 0.0 );

    PROBE_SELECT(   textureCubeLod( lProbe00Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe01Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe02Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe03Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe04Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe05Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe06Map, lDirVec3, lfMip ).rgb,
                    textureCubeLod( lProbe07Map, lDirVec3, lfMip ).rgb );

//#define D_DEBUG_PROBE_COVERAGE
#if defined( D_DEBUG_PROBE_COVERAGE )
    PROBE_SELECT(   vec3( 1.0, 0.0, 0.0 ),
                    vec3( 0.0, 1.0, 0.0 ),
                    vec3( 0.0, 0.0, 1.0 ),
                    vec3( 1.0, 0.0, 1.0 ),
                    vec3( 1.0, 1.0, 0.0 ),
                    vec3( 0.0, 1.0, 1.0 ),
                    vec3( 0.5, 0.5, 1.0 ),
                    vec3( 0.5, 0.7, 0.7 ) );
#endif

    return lColourVec3;
}

vec3
LocalCubemapReflection(
    PerFrameUniforms        lPerFrame,
    CustomPerMeshUniforms   lCustomPerMesh,
    vec3                    lWorldPosVec3,
    vec3                    lNormDirVec3,
    float                   lfRoughness,
    PROBE_ARGS )
{
    vec3 lReflRadVec3   = float2vec3( 0.0 );
    vec3 lRayDirVec3    = normalize( lWorldPosVec3 - lPerFrame.gViewPositionVec3 );
    vec3 lReflDirVec3   = reflect( lRayDirVec3, lNormDirVec3 );
    AABB lAABB          = GetClosestAABB( lPerFrame, lCustomPerMesh, lWorldPosVec3, lReflDirVec3 );

    if ( lAABB.box_idx >= 0 )
    {
        vec3 lLocalReflDir  = CorrectLocalCubemap( lAABB );
        float lfMipBias     = CalculateLocalMipBias( length( lLocalReflDir ), lfRoughness );
        lReflRadVec3        = SampleFromCubemap( lLocalReflDir, lAABB.box_idx, lfMipBias, PROBE_PARAMS );
        float lfFade        = GetAABBFade( lAABB );
        lReflRadVec3        = mix( float2vec3( 0.0 ), lReflRadVec3, lfFade ); 
        lReflRadVec3       *= max( 1.0, lfMipBias * MIP_STRENGHT_BIAS_LOCAL );
    }
    return lReflRadVec3;
}

#endif // D_LOCAL_REFLECTIONS

vec3
GetRadiance( 
    PerFrameUniforms        lPerFrame,
    CustomPerMeshUniforms   lPerMesh,
    vec2                    lTexSizeVec2,
    vec2                    lUVsVec2,
    vec2                    lHitUVsVec2,
    float                   lfHitVisibility,
    float                   lfRoughness,
    vec3                    lWorldPosVec3,
    vec3                    lNormDirVec3,
    SAMPLER2DARG(           lRadianceMap ),
    PROBE_ARGS )
{
    lfHitVisibility        *= smoothstep( 0.0, 0.35, 1.0 - abs( lHitUVsVec2.x - 0.5 ) * 2.0 );
    lfHitVisibility        *= smoothstep( 0.0, 0.35, 1.0 - abs( lHitUVsVec2.y - 0.5 ) * 2.0 );
    
    vec3  lOutRadianceVec3  = float2vec3( 0.0 );
    vec3  lSSRRadianceVec3  = float2vec3( 0.0 );

    if ( lfHitVisibility >= VISIBILITY_THRESHOLD )
    {
        float lfMipBias         = CalculateMipBias( lUVsVec2, lHitUVsVec2, lTexSizeVec2, lfRoughness );
        lSSRRadianceVec3        = texture2DLod( lRadianceMap, lHitUVsVec2, lfMipBias ).rgb;
        lSSRRadianceVec3       *= SSR_STRENGTH_BIAS * max( 1.0, lfMipBias * MIP_STRENGHT_BIAS );
        lOutRadianceVec3        = lSSRRadianceVec3;
    }
#if !defined( D_LOCAL_REFLECTIONS )
    else
    {
        return float2vec3( 0.0 );
    }
#endif

#if defined( D_LOCAL_REFLECTIONS )
    if ( lfHitVisibility < 1.0 )
    {
        vec3 lCubeRadianceVec3;

        lCubeRadianceVec3   = LocalCubemapReflection(
                                    lPerFrame,     lPerMesh,
                                    lWorldPosVec3, lNormDirVec3,
                                    lfRoughness,   PROBE_PARAMS );

        lCubeRadianceVec3  *= LOCAL_REFLECTION_STRENGTH_BIAS;
        lOutRadianceVec3    = mix( lCubeRadianceVec3, lSSRRadianceVec3, lfHitVisibility );
    }
#else
    lOutRadianceVec3 *= lfHitVisibility;
#endif
    lOutRadianceVec3  = max( lOutRadianceVec3, 0.0 );

    return lOutRadianceVec3;
}

#endif // D_SSR_USES_PROBES

//-----------------------------------------------------------------------------
///
///     SSR Resolve Functions
///
//-----------------------------------------------------------------------------
bool
IsMirror(
    float lfRoughness )
{
    return lfRoughness < MIRROR_THRESHOLD;
}


bool
ShouldResolve(
    float lfRoughness )
{
    return !IsMirror( lfRoughness );
}

float
EncodeGGX_PDF(
    float lfPDF,
    float lfRoughness,
    bool  lbHit )
{
    if ( !lbHit ) return 0.0;
    if ( IsMirror( lfRoughness ) ) return 1.0;

    // only valid if the cutoff for mirror reflections
    // is 0.146 or higher
    return max( sqrt( lfPDF ) / 1.5, 1.0 / 255.0 );
}

float
DecodeGGX_PDF(
    in  float lfEcdPDF,
    out bool  lbValid )
{
    lbValid = lfEcdPDF > 0.0;
    return 2.25 * lfEcdPDF;
}

float
CalculateSampleBRDFWeight(
    vec3  lViewDirVec3,
    vec3  lNormDirVec3,
    vec3  lReflDirVec3,
    float lfPDF,
    float lfRoughness )
{
    // BRDF Weight
    vec3  N = lNormDirVec3;
    vec3  L = lReflDirVec3;
    vec3  V = lViewDirVec3;
    vec3  H = normalize( L + V );

    float A     = lfRoughness * lfRoughness;
    float A2    = A * A;
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float NdotL = saturate(dot(N, L));

    // Calculate BRDF where Fresnel = 1
    float G             = Vis_Smith( A2, NdotV, NdotL );
    float D             = D_GGX( A2, NdotH );
    float LocalBRDF     = G * D * NdotL;
    float weight        = LocalBRDF / lfPDF;

    return weight;
}


int
ReuseCount(
    int   liReuseCount,
    float lfVariance,
    float lfRoughness )
{
    liReuseCount  =  clamp( int( liReuseCount * lfVariance * 2.0 * lfRoughness ), 4, liReuseCount );

    return liReuseCount;
}

float
ReuseRadius(
    float lfReuseRadMin,
    float lfReuseRadMax,
    float lfVariance,
    float lfRoughness,
    float lfDepth )
{
    //if ( lfVariance == 1.0 ) return lfReuseRadMax;

    float lfVarFactor   = lfVariance * 4.0;
    float lfRoughFactor = mix( 0.1, 1.0, lfRoughness * lfRoughness );
    float lfReuseRadius = mix( lfReuseRadMin, lfReuseRadMax, saturate( lfRoughFactor * lfVarFactor ) );

    return lfReuseRadius;
}

float
ReuseWeight(
    vec2  lDiskVec2,
    float lfReuseRad,
    float lfVariance,
    float lfRoughness,
    vec3  lHitRadiance,
    vec3  lCurrHitRadiance )
{
    //if ( lfVariance == 1.0 ) return 1.0;

    float sigma     = lfVariance * 2.5 * mix( 0.1, 1.0, lfRoughness * lfRoughness );
    float sigmaSqr  = sigma * sigma;
    vec2  diskSqr   = lDiskVec2 * lDiskVec2;
    vec2  diskExp   = diskSqr / sigmaSqr;
    float radWeight = dot( lHitRadiance - lCurrHitRadiance, lHitRadiance - lCurrHitRadiance ) / 0.015;
    
    return exp(  - ( diskExp.x + diskExp.y + radWeight ) );
}


//-----------------------------------------------------------------------------
///
///     SSR Temporal Functions
///
//-----------------------------------------------------------------------------
vec3 
clip_aabb(
    vec3 q,
    vec3 aabb_min,
    vec3 aabb_max )
{
    // note: only clips towards aabb center (but fast!)
    vec3 p_clip = 0.5 * (aabb_max + aabb_min);
    vec3 e_clip = 0.5 * (aabb_max - aabb_min);

    vec3 v_clip = q - p_clip;
    vec3 v_unit = e_clip / v_clip;
    vec3 a_unit = abs(v_unit);
    float ma_unit = saturate( min(a_unit.x, min(a_unit.y, a_unit.z)) );

    return p_clip + v_clip * ma_unit;
}

vec3 
ClipColor(
    in  vec3    currentColorRGB,
    in  vec3    historyColorRGB,
    in  vec3    block3x3_YCgCo[9],
    in  float   gammaScale,
    out float   variance )
{        
    // 0 1 2
    // 3
    vec3 t0 = block3x3_YCgCo[0].rgb;
    vec3 t1 = block3x3_YCgCo[1].rgb;
    vec3 t2 = block3x3_YCgCo[2].rgb;
    vec3 t3 = block3x3_YCgCo[3].rgb;
    //     5
    // 6 7 8
    vec3 b0 = block3x3_YCgCo[5].rgb;
    vec3 b1 = block3x3_YCgCo[6].rgb;
    vec3 b2 = block3x3_YCgCo[7].rgb;
    vec3 b3 = block3x3_YCgCo[8].rgb;

    vec3 currentColorYCgCo;
    currentColorYCgCo = block3x3_YCgCo[4].rgb;

    vec3 historyColorYCgCo;
    historyColorYCgCo = RGBToYCgCo(historyColorRGB);

    // do variance clipping
    vec3  mu_9      = ( t0               + t1               + t2               + t3               + b0                + b1               + b2               + b3               + currentColorYCgCo )               / 9.0;
    vec3  vr_9      = ( sqr( t0 - mu_9 ) + sqr( t1 - mu_9 ) + sqr( t2 - mu_9 ) + sqr( t3 - mu_9 ) + sqr( b0 - mu_9 )  + sqr( b1 - mu_9 ) + sqr( b2 - mu_9 ) + sqr( b3 - mu_9 ) + sqr( currentColorYCgCo - mu_9 ) ) / 8.0;
    vec3  sd_9      = sqrt( vr_9 );

    variance        = vr_9.x / ( 1.0 + vr_9.x );

    vec3  mu        = mu_9;
    vec3  sigma     = sd_9;
    float gamma     = mix( 4.0, 8.0, gammaScale );

    vec3  minimum   = mu - gamma * sigma;
    vec3  maximum   = mu + gamma * sigma;

    vec3  newHistoryColor   = YCgCoToRGB( clip_aabb( historyColorYCgCo, minimum, maximum ) );

    return newHistoryColor;
}

void
Gather3x3_YCgCo(
    SAMPLER2DARG(   lThisFrameSSRMap ),
    vec2            lUVsVec2,
    inout vec3      block3x3_YCgCo[9] )
{
    // Get the size of a texel
    vec2  lTextureSizeVec2  = vec2( GetTexResolution( lThisFrameSSRMap ) );
    vec2  lTexelSizeVec2    = 1.0 / lTextureSizeVec2;
    float lfU               = lTexelSizeVec2.x;
    float lfV               = lTexelSizeVec2.y;

    // Sample the "rounded" 3x3 neighbourhood of the current solution
    block3x3_YCgCo[ 0 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2(-lfU, -lfV) ).rgb );
    block3x3_YCgCo[ 1 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2(   0, -lfV) ).rgb );
    block3x3_YCgCo[ 2 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2( lfU, -lfV) ).rgb );
    block3x3_YCgCo[ 3 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2(-lfU,    0) ).rgb );
    block3x3_YCgCo[ 4 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2(   0,    0) ).rgb );
    block3x3_YCgCo[ 5 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2( lfU,    0) ).rgb );
    block3x3_YCgCo[ 6 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2(-lfU,  lfV) ).rgb );
    block3x3_YCgCo[ 7 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2(   0,  lfV) ).rgb );
    block3x3_YCgCo[ 8 ] = RGBToYCgCo( texture2D( lThisFrameSSRMap, lUVsVec2 + vec2( lfU,  lfV) ).rgb );
}

#endif // SSR_UTILS_H