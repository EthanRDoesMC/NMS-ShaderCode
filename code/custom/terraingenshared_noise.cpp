//------------------------------------------------------3-----------------------
//Terrain gen cpp code shared between CPU and GPU
//This can be included from a single c++ file in c++ code
//-----------------------------------------------------------------------------

#ifndef _TGENSHARED_NOISE_CPP_
#define _TGENSHARED_NOISE_CPP_
#include "Custom/TerrainGenShared_Noise.h"
#include "Custom/SharedBegin.inl"

//TK_DISABLE_OPTIMIZATION

//-----------------------------------------------------------------------------
//Initial tables and small functions used to generate core simplex noise
//-----------------------------------------------------------------------------

//Simplex noise permutation lookup table (repeated to allow for indexing between 256-511)
STATIC_CONST 
#if defined( __cplusplus )
unsigned char
#else
int 
#endif
gNoisePermutationTable[256] = 
{
	151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
};

//Macro to use permutation table to generate a random number from 3 integers 
#define SIMPLEX_NOISE_HASH(x,y,z) gNoisePermutationTable[((x) + gNoisePermutationTable[((y) + gNoisePermutationTable[((z)&255)])&255])&255]


//Lookup table for grad3 vectors (12 edges of a cube)
STATIC_CONST vec3 gNoiseGrad3LUT[16] =
{
    vec3(1.0f, 0.0f, 1.0f),
    vec3(0.0f, 1.0f, 1.0f), // 12 cube edges
    vec3(-1.0f, 0.0f, 1.0f),
    vec3(0.0f, -1.0f, 1.0f),
    vec3(1.0f, 0.0f, -1.0f),
    vec3(0.0f, 1.0f, -1.0f),
    vec3(-1.0f, 0.0f, -1.0f),
    vec3(0.0f, -1.0f, -1.0f),
    vec3(1.0f, -1.0f, 0.0f),
    vec3(1.0f, 1.0f, 0.0f),
    vec3(-1.0f, 1.0f, 0.0f),
    vec3(-1.0f, -1.0f, 0.0f),
    vec3(1.0f, 0.0f, 1.0f),
    vec3(-1.0f, 0.0f, 1.0f), // 4 repeats to make 16
    vec3(0.0f, 1.0f, -1.0f),
    vec3(0.0f, -1.0f, -1.0f)
};

#if defined( __cplusplus )

#ifdef D_PLATFORM_PC
// the instrinsics in the other version are sse4.1. If want that on PC we'd need a check in here if it supports that instruction set, which would make it 
// slow enough that it's not worth it.
TKINLINE float Round(const float val)
{
    return round(val);
}
#else
TKINLINE float Round(const float val)
{
    return _mm_cvtss_f32(_mm_round_ps(_mm_set1_ps(val), _MM_FROUND_NO_EXC));
}
#endif

TKINLINE float ClampPrecision(const float _val, const int _bits)
{
    float lfScale = float(1 << _bits);
    float x = (_val * lfScale) + 0.5f;
    sInt64 n = (sInt64)x;
    float r = (float)n;
    return (r - (r > x)) / lfScale;
}

#else

TKINLINE float ClampPrecision(const float _f, const int _bits)
{
    return ldexp(round(ldexp(_f, float(_bits))), float(-_bits));
}
TKINLINE float Round(const float val)
{
    return round(val);
}

#endif


//Uses lookup table to get a gradient for a given hash
vec3 NoiseGrad3(int hash)
{
    int h = hash & 15;
    return gNoiseGrad3LUT[h];
}
void NoiseGrad3(int hash, OUTPARAM(float) x, OUTPARAM(float) y, OUTPARAM(float) z)
{
    vec3 res = NoiseGrad3(hash);
    x = res.x;
    y = res.y;
    z = res.z;
}

//Constants for noise calculation
STATIC_CONST float F3 = 1.0f / 3.0f;
STATIC_CONST float G3 = 1.0f / 6.0f; // Very nice and simple unskew factor, too

#if 0 //def D_PLATFORM_NX64

TKINLINE int IntFloor(float f)
{
    return floorf(f);
}
#else

TKINLINE int IntFloor( float f )
{
#if ISPC
    int i = (int)f;
    return ( i - select( ( i > f ), 1, 0 ) );
#else
    int i = (int)f;
    return ( i - int( i > f ) );
#endif
}
#endif

//-----------------------------------------------------------------------------
//Main noise 3d function with derivatives
//-----------------------------------------------------------------------------
TKFORCEINLINE
float
Noise3d(float x, float y, float z, OUTPARAM(float) dnoise_dx, OUTPARAM(float) dnoise_dy, OUTPARAM(float) dnoise_dz, OUTPARAM(vec4) dbg)
{
    float n0, n1, n2, n3; /* Noise contributions from the four simplex corners */
    float noise;          /* Return value */
    float gx0, gy0, gz0, gx1, gy1, gz1; /* Gradients at simplex corners */
    float gx2, gy2, gz2, gx3, gy3, gz3;
    float x1, y1, z1, x2, y2, z2, x3, y3, z3;
    float t0, t1, t2, t3, t20, t40, t21, t41, t22, t42, t23, t43;
    float temp0, temp1, temp2, temp3;

    /* Skew the input space to determine which simplex cell we're in */
    float s = (x + y + z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int ii, i = (int)IntFloor( xs );
    int jj, j = (int)IntFloor( ys );
    int kk, k = (int)IntFloor( zs );

    float t = (float)(i + j + k)*G3;
    float X0 = i - t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; /* The x,y,z distances from the cell origin */
    float y0 = y - Y0;
    float z0 = z - Z0;

    /* For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    * Determine which simplex we are in. */

    bool g0 = (x0 >= y0), l0 = !g0;
    bool g1 = (y0 >= z0), l1 = !g1;
    bool g2 = (z0 >= x0), l2 = !g2;
    int i1 = int(g0) & int(l2);   /* Offsets for second corner of simplex in (i,j,k) coords */
    int j1 = int(g1) & int(l0);
    int k1 = int(g2) & int(l1);
    int i2 = int(g0) | int(l2);   /* Offsets for third corner of simplex in (i,j,k) coords */
    int j2 = int(g1) | int(l0);
    int k2 = int(g2) | int(l1);    

    /* A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    * a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    * a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    * c = 1/6.   */

    x1 = x0 - i1 + G3; /* Offsets for second corner in (x,y,z) coords */
    y1 = y0 - j1 + G3;
    z1 = z0 - k1 + G3;
    x2 = x0 - i2 + 2.0f * G3; /* Offsets for third corner in (x,y,z) coords */
    y2 = y0 - j2 + 2.0f * G3;
    z2 = z0 - k2 + 2.0f * G3;
    x3 = x0 - 1.0f + 3.0f * G3; /* Offsets for last corner in (x,y,z) coords */
    y3 = y0 - 1.0f + 3.0f * G3;
    z3 = z0 - 1.0f + 3.0f * G3;
    ii = i & 255;
    jj = j & 255;
    kk = k & 255;

    /* Calculate the contribution from the four corners */
    t0 = max( 0.0f, 0.6f - x0*x0 - y0*y0 - z0*z0 );
    {
        NoiseGrad3(SIMPLEX_NOISE_HASH(ii, jj, kk), gx0, gy0, gz0);
        t20 = t0 * t0;
        t40 = t20 * t20;
        n0 = t40 * (gx0 * x0 + gy0 * y0 + gz0 * z0);
    }

    t1 = max( 0.0f, 0.6f - x1*x1 - y1*y1 - z1*z1 );
    {
        NoiseGrad3(SIMPLEX_NOISE_HASH(ii + i1, jj + j1, kk + k1), gx1, gy1, gz1);
        t21 = t1 * t1;
        t41 = t21 * t21;
        n1 = t41 * (gx1 * x1 + gy1 * y1 + gz1 * z1);
    }

    t2 = max( 0.0f, 0.6f - x2*x2 - y2*y2 - z2*z2 );
    {
        NoiseGrad3(SIMPLEX_NOISE_HASH(ii + i2, jj + j2, kk + k2), gx2, gy2, gz2);
        t22 = t2 * t2;
        t42 = t22 * t22;
        n2 = t42 * (gx2 * x2 + gy2 * y2 + gz2 * z2);
    }

    t3 = max( 0.0, 0.6f - x3*x3 - y3*y3 - z3*z3 );
    {
        NoiseGrad3(SIMPLEX_NOISE_HASH(ii + 1, jj + 1, kk + 1), gx3, gy3, gz3);
        t23 = t3 * t3;
        t43 = t23 * t23;
        n3 = t43 * (gx3 * x3 + gy3 * y3 + gz3 * z3);
    }

    /*  Add contributions from each corner to get the final noise value.
    * The result is scaled to return values in the range [-1,1] */
    noise = 28.0f * (n0 + n1 + n2 + n3);

    /* Compute derivative, if requested by supplying non-null pointers
    * for the last three arguments */
    {
        /*  A straight, unoptimised calculation would be like:
        *     *dnoise_dx = -8.0f * t20 * t0 * x0 * dot(gx0, gy0, gz0, x0, y0, z0) + t40 * gx0;
        *    *dnoise_dy = -8.0f * t20 * t0 * y0 * dot(gx0, gy0, gz0, x0, y0, z0) + t40 * gy0;
        *    *dnoise_dz = -8.0f * t20 * t0 * z0 * dot(gx0, gy0, gz0, x0, y0, z0) + t40 * gz0;
        *    *dnoise_dx += -8.0f * t21 * t1 * x1 * dot(gx1, gy1, gz1, x1, y1, z1) + t41 * gx1;
        *    *dnoise_dy += -8.0f * t21 * t1 * y1 * dot(gx1, gy1, gz1, x1, y1, z1) + t41 * gy1;
        *    *dnoise_dz += -8.0f * t21 * t1 * z1 * dot(gx1, gy1, gz1, x1, y1, z1) + t41 * gz1;
        *    *dnoise_dx += -8.0f * t22 * t2 * x2 * dot(gx2, gy2, gz2, x2, y2, z2) + t42 * gx2;
        *    *dnoise_dy += -8.0f * t22 * t2 * y2 * dot(gx2, gy2, gz2, x2, y2, z2) + t42 * gy2;
        *    *dnoise_dz += -8.0f * t22 * t2 * z2 * dot(gx2, gy2, gz2, x2, y2, z2) + t42 * gz2;
        *    *dnoise_dx += -8.0f * t23 * t3 * x3 * dot(gx3, gy3, gz3, x3, y3, z3) + t43 * gx3;
        *    *dnoise_dy += -8.0f * t23 * t3 * y3 * dot(gx3, gy3, gz3, x3, y3, z3) + t43 * gy3;
        *    *dnoise_dz += -8.0f * t23 * t3 * z3 * dot(gx3, gy3, gz3, x3, y3, z3) + t43 * gz3;
        */
        temp0 = t20 * t0 * (gx0 * x0 + gy0 * y0 + gz0 * z0);
        dnoise_dx = temp0 * x0;
        dnoise_dy = temp0 * y0;
        dnoise_dz = temp0 * z0;
        temp1 = t21 * t1 * (gx1 * x1 + gy1 * y1 + gz1 * z1);
        dnoise_dx += temp1 * x1;
        dnoise_dy += temp1 * y1;
        dnoise_dz += temp1 * z1;
        temp2 = t22 * t2 * (gx2 * x2 + gy2 * y2 + gz2 * z2);
        dnoise_dx += temp2 * x2;
        dnoise_dy += temp2 * y2;
        dnoise_dz += temp2 * z2;
        temp3 = t23 * t3 * (gx3 * x3 + gy3 * y3 + gz3 * z3);
        dnoise_dx += temp3 * x3;
        dnoise_dy += temp3 * y3;
        dnoise_dz += temp3 * z3;
        dnoise_dx *= -8.0f;
        dnoise_dy *= -8.0f;
        dnoise_dz *= -8.0f;
        dnoise_dx += t40 * gx0 + t41 * gx1 + t42 * gx2 + t43 * gx3;
        dnoise_dy += t40 * gy0 + t41 * gy1 + t42 * gy2 + t43 * gy3;
        dnoise_dz += t40 * gz0 + t41 * gz1 + t42 * gz2 + t43 * gz3;
        dnoise_dx *= 28.0f; /* Scale derivative to match the noise scaling */
        dnoise_dy *= 28.0f;
        dnoise_dz *= 28.0f;
    }

    float res = noise;

    dbg = vec4(res,dnoise_dx,dnoise_dy,dnoise_dz);

    return res;
}


//Main noise 3d function with derivatives
//-----------------------------------------------------------------------------
#if defined( __cplusplus )

/*  _mm_sum4 - return [a.sum() b.sum() c.sum() d.sum()] */
inline __m128 _mm_sum4(__m128 a, __m128 b, __m128 c, __m128 d) {
    /* [a0+a2 c0+c2 a1+a3 c1+c3 */
    __m128 s1 = _mm_add_ps(_mm_unpacklo_ps(a, c), _mm_unpackhi_ps(a, c));
    /* [b0+b2 d0+d2 b1+b3 d1+d3 */
    __m128 s2 = _mm_add_ps(_mm_unpacklo_ps(b, d), _mm_unpackhi_ps(b, d));
    /* [a0+a2 b0+b2 c0+c2 d0+d2]+
       [a1+a3 b1+b3 c1+c3 d1+d3] */
    return _mm_add_ps(_mm_unpacklo_ps(s1, s2), _mm_unpackhi_ps(s1, s2));
}

STATIC_CONST __m128i vec_mask255 = _mm_set_epi32(255, 255, 255, 0);
STATIC_CONST __m128i vec_mask1   = _mm_set_epi32(1, 1, 1, 0);
STATIC_CONST __m128  vec_G3   = { 0.0f, 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f };
STATIC_CONST __m128  vec_G3x2 = { 0.0f, 2.0f / 6.0f, 2.0f / 6.0f, 2.0f / 6.0f };
STATIC_CONST __m128  vec_G3x3 = { 0.0f, (3.0f / 6.0f) - 1.0f, (3.0f / 6.0f) - 1.0f, (3.0f / 6.0f) - 1.0f };
STATIC_CONST __m128  vec_minus8 = { -8.0f, -8.0f, -8.0f, -8.0f };
STATIC_CONST __m128  vec_28     = { 28.0f, 28.0f, 28.0f, 28.0f };
STATIC_CONST __m128  vec_0point6    = { 0.6f, 0.6f, 0.6f, 0.6f };

STATIC_CONST __m128  gNoiseGrad3LUT_SSE[16] =
{
    { 0.0f,1.0f, 0.0f, 1.0f },
    { 0.0f,0.0f, 1.0f, 1.0f }, // 12 cube edges
    { 0.0f,-1.0f, 0.0f, 1.0f },
    { 0.0f,0.0f, -1.0f, 1.0f },
    { 0.0f,1.0f, 0.0f, -1.0f },
    { 0.0f,0.0f, 1.0f, -1.0f },
    { 0.0f,-1.0f, 0.0f, -1.0f },
    { 0.0f,0.0f, -1.0f, -1.0f },
    { 0.0f,1.0f, -1.0f, 0.0f },
    { 0.0f,1.0f, 1.0f, 0.0f },
    { 0.0f,-1.0f, 1.0f, 0.0f },
    { 0.0f,-1.0f, -1.0f, 0.0f },
    { 0.0f,1.0f, 0.0f, 1.0f },
    { 0.0f,-1.0f, 0.0f, 1.0f }, // 4 repeats to make 16
    { 0.0f,0.0f, 1.0f, -1.0f },
    { 0.0f,0.0f, -1.0f, -1.0f }
};

__m128  gNoiseGrad3LUTPermuted_SSE[256];
unsigned char gNoisePermutationTable65k[65536];

static int InitPerm(void)
{
    for (sInt32 i = 0; i < 256; i++)
    {
        sUInt32 luPerm = gNoisePermutationTable[i];
        gNoiseGrad3LUTPermuted_SSE[i] = gNoiseGrad3LUT_SSE[luPerm & 15];
    }
    for (sInt32 i = 0; i < 65536; i++)
    {
        int z = i >> 8;
        int y = i & 0xff;
        gNoisePermutationTable65k[i] = gNoisePermutationTable[(y + gNoisePermutationTable[z])&255];
    }
    return 0;
}

static int gDummyInitPerm = InitPerm();

TKFORCEINLINE __m128 NoiseGrad3_SSE(int hash)
{
    return gNoiseGrad3LUT_SSE[hash & 15];
}

TKFORCEINLINE __m128 NoiseGrad3Simplex_SIMD(int x, int y, int z)
{
    return gNoiseGrad3LUTPermuted_SSE[((x)+gNoisePermutationTable[((y)+gNoisePermutationTable[(z&255)])&255]) & 255];
    //return gNoiseGrad3LUTPermuted_SSE[((x)+gNoisePermutationTable65k[(y&255)|((z&255)<<8)])&255];
}


#if 0

TKFORCEINLINE __m128i hsum_epi32_sse2(__m128i x)
{
    __m128i shuf = _mm_shuffle_epi32(x, _MM_SHUFFLE(2, 3, 0, 1));
    __m128i sums = _mm_add_epi32(x, shuf);
    return _mm_add_epi32(sums, _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(shuf), _mm_castsi128_ps(sums))));
}

TKFORCEINLINE __m128 hsum_ps_sse2(__m128 x)
{
    __m128 shuf = _mm_shuffle_ps(x, x, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(x, shuf);
    return _mm_add_ps(sums, _mm_movehl_ps(shuf, sums));
}

TKFORCEINLINE
float
Noise3dAVX(float x, float y, float z, OUTPARAM(float) dnoise_dx, OUTPARAM(float) dnoise_dy, OUTPARAM(float) dnoise_dz)
{
    float s = (x + y + z)*F3;
    __m128 _xyz = _mm_set_ps(z, y, x, 0.0f);
    __m128 _xyzs = _mm_add_ps(_xyz, _mm_set_ps(s, s, s, 0.0f));
    __m128 _s_int = _mm_round_ps(_xyzs, _MM_FROUND_NO_EXC | _MM_FROUND_FLOOR);
    __m128i ijk = _mm_cvtps_epi32(_s_int);
    __m128i _t = hsum_epi32_sse2(ijk);
    ijk = _mm_and_si128(ijk, vec_mask255);
    __m128 _XYZ0 = _mm_sub_ps(_s_int, _mm_mul_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(_t, _MM_SHUFFLE(1, 1, 1, 0))), vec_G3));
    __m128 vec_v0 = _mm_sub_ps(_xyz, _XYZ0);
    __m128i vec_g = _mm_castps_si128(_mm_cmpge_ps(vec_v0, _mm_shuffle_ps(vec_v0, vec_v0, _MM_SHUFFLE(1, 3, 2, 0))));
    vec_g = _mm_and_si128(vec_g, vec_mask1);
    __m128i vec_l = _mm_xor_si128(vec_g, vec_mask1);
    __m128i vec_lshuf = _mm_shuffle_epi32(vec_l, _MM_SHUFFLE(2, 1, 3, 0));
    __m128i vec_ijk1 = _mm_and_si128(vec_g, vec_lshuf);
    __m128i vec_ijk2 = _mm_or_si128(vec_g, vec_lshuf);
    __m128 vec_v1 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_cvtepi32_ps(vec_ijk1)), vec_G3);
    __m128 vec_v2 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_cvtepi32_ps(vec_ijk2)), vec_G3x2);
    __m128 vec_v3 = _mm_add_ps(vec_v0, vec_G3x3);
    int ii = _mm_extract_epi32(ijk, 1);
    int jj = _mm_extract_epi32(ijk, 2);
    int kk = _mm_extract_epi32(ijk, 3);
    __m128 vec_v0_2 = _mm_mul_ps(vec_v0, vec_v0);
    __m128 vec_v1_2 = _mm_mul_ps(vec_v1, vec_v1);
    __m128 vec_v2_2 = _mm_mul_ps(vec_v2, vec_v2);
    __m128 vec_v3_2 = _mm_mul_ps(vec_v3, vec_v3);
    __m128 vec_t = _mm_sum4(vec_v3_2, vec_v2_2, vec_v1_2, vec_v0_2);
    vec_t = _mm_sub_ps(vec_0point6, vec_t);
    vec_t = _mm_max_ps(_mm_set1_ps(0.0f), vec_t);
    vec_ijk1 = _mm_add_epi32(vec_ijk1, ijk);
    vec_ijk2 = _mm_add_epi32(vec_ijk2, ijk);
    int i1 = _mm_extract_epi32(vec_ijk1, 1);
    int j1 = _mm_extract_epi32(vec_ijk1, 2);
    int k1 = _mm_extract_epi32(vec_ijk1, 3);
    int i2 = _mm_extract_epi32(vec_ijk2, 1);
    int j2 = _mm_extract_epi32(vec_ijk2, 2);
    int k2 = _mm_extract_epi32(vec_ijk2, 3);
    __m128 vec_g0 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii, jj, kk));
    __m128 vec_g1 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(i1, j1, k1));
    __m128 vec_g2 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(i2, j2, k2));
    __m128 vec_g3 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + 1, jj + 1, kk + 1));
    __m128 vec_t2 = _mm_mul_ps(vec_t, vec_t);
    __m128 vec_t4 = _mm_mul_ps(vec_t2, vec_t2);
    __m128 dotn0 = _mm_mul_ps(vec_g0, vec_v0);
    __m128 dotn1 = _mm_mul_ps(vec_g1, vec_v1);
    __m128 dotn2 = _mm_mul_ps(vec_g2, vec_v2);
    __m128 dotn3 = _mm_mul_ps(vec_g3, vec_v3);
    __m128 temp = _mm_sum4(dotn3, dotn2, dotn1, dotn0);
    __m128 tempn = _mm_mul_ps(temp, vec_t4);
    __m128 shuf = _mm_shuffle_ps(tempn, tempn, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(tempn, shuf);
    float noise = _mm_cvtss_f32(_mm_add_ss(sums, _mm_movehl_ps(shuf, sums))) * 28.0f;
    {
        __m128 dnoise;
        temp = _mm_mul_ps(temp, vec_t);
        temp = _mm_mul_ps(temp, vec_t2);
#if 1
        dnoise = _mm_mul_ps(vec_v0, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)));
        dnoise = _mm_add_ps(dnoise, _mm_mul_ps(vec_v1, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2))));
        dnoise = _mm_add_ps(dnoise, _mm_mul_ps(vec_v2, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1))));
        dnoise = _mm_add_ps(dnoise, _mm_mul_ps(vec_v3, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0))));
        dnoise = _mm_mul_ps(dnoise, vec_minus8);
        vec_g0 = _mm_mul_ps(vec_g0, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(3, 3, 3, 3)));
        vec_g0 = _mm_add_ps(vec_g0, _mm_mul_ps(vec_g1, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(2, 2, 2, 2))));
        vec_g0 = _mm_add_ps(vec_g0, _mm_mul_ps(vec_g2, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(1, 1, 1, 1))));
        vec_g0 = _mm_add_ps(vec_g0, _mm_mul_ps(vec_g3, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(0, 0, 0, 0))));
        dnoise = _mm_add_ps(dnoise, vec_g0);
        dnoise = _mm_mul_ps(dnoise, vec_28);
#else
        dnoise = _mm_mul_ps(vec_v0, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)));
        dnoise = _mm_fmadd_ps(vec_v1, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2)), dnoise);
        dnoise = _mm_fmadd_ps(vec_v2, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1)), dnoise);
        dnoise = _mm_fmadd_ps(vec_v3, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0)), dnoise);
        dnoise = _mm_mul_ps(dnoise, vec_minus8);
        dnoise = _mm_fmadd_ps(vec_g0, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(3, 3, 3, 3)), dnoise);
        dnoise = _mm_fmadd_ps(vec_g1, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(2, 2, 2, 2)), dnoise);
        dnoise = _mm_fmadd_ps(vec_g2, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(1, 1, 1, 1)), dnoise);
        dnoise = _mm_fmadd_ps(vec_g3, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(0, 0, 0, 0)), dnoise);
        dnoise = _mm_mul_ps(dnoise, vec_28);
#endif
        dnoise_dx = _mm_cvtss_f32(_mm_shuffle_ps(dnoise, dnoise, _MM_SHUFFLE(1, 1, 1, 1)));
        dnoise_dy = _mm_cvtss_f32(_mm_shuffle_ps(dnoise, dnoise, _MM_SHUFFLE(2, 2, 2, 2)));
        dnoise_dz = _mm_cvtss_f32(_mm_shuffle_ps(dnoise, dnoise, _MM_SHUFFLE(3, 3, 3, 3)));
    }
    return noise;
}


double ldBenchMarkVar = 0.0;
TKNOINLINE void BenchNoise(void)
{
    for (int j = 1; j < 25000000; j++)
    {
        float dx, dy, dz;
        ldBenchMarkVar += (double)GPU::Noise3dAVX(float(j), float(j & 8 ? j - 123 : j + 123), float(j & 4 ? j - 321 : j + 321), dx, dy, dz);
        ldBenchMarkVar += (double)dx;
        ldBenchMarkVar += (double)dy;
        ldBenchMarkVar += (double)dz;
    }
}

#elif defined( D_PLATFORM_NX64 ) || defined ( D_PLATFORM_ARM64 )

TKFORCEINLINE
float
Noise3dNEON(float x, float y, float z, OUTPARAM(float) dnoise_dx, OUTPARAM(float) dnoise_dy, OUTPARAM(float) dnoise_dz)
{
    float s = (x + y + z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int i = (int)IntFloor(xs);
    int j = (int)IntFloor(ys);
    int k = (int)IntFloor(zs);
    float t = (float)(i + j + k)*G3;
    float X0 = i - t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; /* The x,y,z distances from the cell origin */
    float y0 = y - Y0;
    float z0 = z - Z0;
    bool g0 = (x0 >= y0), l0 = !g0;
    bool g1 = (y0 >= z0), l1 = !g1;
    bool g2 = (z0 >= x0), l2 = !g2;
    int i1 = g0 & l2;   /* Offsets for second corner of simplex in (i,j,k) coords */
    int j1 = g1 & l0;
    int k1 = g2 & l1;
    int i2 = g0 | l2;   /* Offsets for third corner of simplex in (i,j,k) coords */
    int j2 = g1 | l0;
    int k2 = g2 | l1;
    __m128 vec_v0 = { 0.0f, x0, y0, z0 };
    
#if defined ( D_PLATFORM_NX64 )
    __m128i vec_ijk1 = { 0, i1, j1, k1 };
    __m128i vec_ijk2 = { 0, i2, j2, k2 };
#else
    __m128i vec_ijk1 = { 0, 0 };
    vec_ijk1 = vsetq_lane_s32( i1, vec_ijk1, 1 );
    vec_ijk1 = vsetq_lane_s32( j1, vec_ijk1, 2 );
    vec_ijk1 = vsetq_lane_s32( k1, vec_ijk1, 3 );
    
    __m128i vec_ijk2 = { 0, 0 };
    vec_ijk2 = vsetq_lane_s32( i2, vec_ijk2, 1 );
    vec_ijk2 = vsetq_lane_s32( j2, vec_ijk2, 2 );
    vec_ijk2 = vsetq_lane_s32( k2, vec_ijk2, 3 );
#endif
    __m128 vec_v1 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_cvtepi32_ps(vec_ijk1)), vec_G3);
    __m128 vec_v2 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_cvtepi32_ps(vec_ijk2)), vec_G3x2);
    __m128 vec_v3 = _mm_add_ps(vec_v0, vec_G3x3);
    /* Calculate the contribution from the four corners */
    __m128 vec_v0_2 = vmulq_f32(vec_v0, vec_v0);
    __m128 vec_v1_2 = vmulq_f32(vec_v1, vec_v1);
    __m128 vec_v2_2 = vmulq_f32(vec_v2, vec_v2);
    __m128 vec_v3_2 = vmulq_f32(vec_v3, vec_v3);
    __m128 vec_t = { vaddvq_f32(vec_v3_2), vaddvq_f32(vec_v2_2), vaddvq_f32(vec_v1_2), vaddvq_f32(vec_v0_2) };
    vec_t = vsubq_f32(vec_0point6, vec_t);
    vec_t = vmaxq_f32(_mm_set1_ps(0.0f), vec_t);
    __m128 vec_g0 = NoiseGrad3Simplex_SIMD(i, j, k);
    __m128 vec_g1 = NoiseGrad3Simplex_SIMD(i + i1, j + j1, k + k1);
    __m128 vec_g2 = NoiseGrad3Simplex_SIMD(i + i2, j + j2, k + k2);
    __m128 vec_g3 = NoiseGrad3Simplex_SIMD(i + 1, j + 1, k + 1);
    __m128 vec_t2 = vmulq_f32(vec_t, vec_t);
    __m128 vec_t4 = vmulq_f32(vec_t2, vec_t2);
    __m128 dotn0 = vmulq_f32(vec_g0, vec_v0);
    __m128 dotn1 = vmulq_f32(vec_g1, vec_v1);
    __m128 dotn2 = vmulq_f32(vec_g2, vec_v2);
    __m128 dotn3 = vmulq_f32(vec_g3, vec_v3);
    __m128 temp = { vaddvq_f32(dotn3), vaddvq_f32(dotn2), vaddvq_f32(dotn1), vaddvq_f32(dotn0) };
    __m128 tempn = vmulq_f32(temp, vec_t4);
    float noise = vaddvq_f32(tempn) * 28.0f;
    __m128 dnoise;
    temp = vmulq_f32(temp, vec_t);
    temp = vmulq_f32(temp, vec_t2);
    dnoise = vmulq_f32(vec_v0, vdupq_lane_f32(vget_high_f32(temp), 1));
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_high_f32(temp), 0), vec_v1);
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_low_f32(temp), 1), vec_v2);
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_low_f32(temp), 0), vec_v3);
    dnoise = vmulq_f32(dnoise, vec_minus8);
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_high_f32(vec_t4), 1), vec_g0);
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_high_f32(vec_t4), 0), vec_g1);
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_low_f32(vec_t4), 1), vec_g2);
    dnoise = vfmaq_f32(dnoise, vdupq_lane_f32(vget_low_f32(vec_t4), 0), vec_g3);
    dnoise = vmulq_f32(dnoise, vec_28);
    dnoise_dx = vgetq_lane_f32(dnoise, 1);
    dnoise_dy = vgetq_lane_f32(dnoise, 2);
    dnoise_dz = vgetq_lane_f32(dnoise, 3);
    return noise;
}

#endif

TKFORCEINLINE
float
Noise3dSSE(float x, float y, float z, OUTPARAM(float) dnoise_dx, OUTPARAM(float) dnoise_dy, OUTPARAM(float) dnoise_dz)
{
    /* Skew the input space to determine which simplex cell we're in */
    float s = (x + y + z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int i = (int)IntFloor(xs);
    int j = (int)IntFloor(ys);
    int k = (int)IntFloor(zs);
    float t = (float)(i + j + k)*G3;
    float X0 = i - t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; /* The x,y,z distances from the cell origin */
    float y0 = y - Y0;
    float z0 = z - Z0;
    bool g0 = (x0 >= y0), l0 = !g0;
    bool g1 = (y0 >= z0), l1 = !g1;
    bool g2 = (z0 >= x0), l2 = !g2;
    int i1 = g0 & l2;   /* Offsets for second corner of simplex in (i,j,k) coords */
    int j1 = g1 & l0;
    int k1 = g2 & l1;
    int i2 = g0 | l2;   /* Offsets for third corner of simplex in (i,j,k) coords */
    int j2 = g1 | l0;
    int k2 = g2 | l1;
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    __m128 vec_v0 = _mm_set_ps(z0, y0, x0, 0.0f);
    __m128 vec_v1 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_set_ps((float)k1, (float)j1, (float)i1, 0.0f)), vec_G3);
    __m128 vec_v2 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_set_ps((float)k2, (float)j2, (float)i2, 0.0f)), vec_G3x2);
    __m128 vec_v3 = _mm_add_ps(vec_v0, vec_G3x3);
    /* Calculate the contribution from the four corners */
    __m128 vec_t;
    __m128 vec_v0_2 = _mm_mul_ps(vec_v0, vec_v0);
    __m128 vec_v1_2 = _mm_mul_ps(vec_v1, vec_v1);
    __m128 vec_v2_2 = _mm_mul_ps(vec_v2, vec_v2);
    __m128 vec_v3_2 = _mm_mul_ps(vec_v3, vec_v3);
    vec_t = _mm_sum4(vec_v3_2, vec_v2_2, vec_v1_2, vec_v0_2);
    vec_t = _mm_sub_ps(vec_0point6, vec_t);
    vec_t = _mm_max_ps(_mm_set1_ps(0.0f), vec_t);
    __m128 vec_g0 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii, jj, kk));
    __m128 vec_g1 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + i1, jj + j1, kk + k1));
    __m128 vec_g2 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + i2, jj + j2, kk + k2));
    __m128 vec_g3 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + 1, jj + 1, kk + 1));
    __m128 vec_t2 = _mm_mul_ps(vec_t, vec_t);
    __m128 vec_t4 = _mm_mul_ps(vec_t2, vec_t2);
    __m128 dotn0 = _mm_mul_ps(vec_g0, vec_v0);
    __m128 dotn1 = _mm_mul_ps(vec_g1, vec_v1);
    __m128 dotn2 = _mm_mul_ps(vec_g2, vec_v2);
    __m128 dotn3 = _mm_mul_ps(vec_g3, vec_v3);
    __m128 temp = _mm_sum4(dotn3, dotn2, dotn1, dotn0);
    __m128 tempn = _mm_mul_ps(temp, vec_t4);
    __m128 shuf = _mm_shuffle_ps(tempn, tempn, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(tempn, shuf);
    float noise = _mm_cvtss_f32(_mm_add_ss(sums, _mm_movehl_ps(shuf, sums))) * 28.0f;
    /* Compute derivative, if requested by supplying non-null pointers
    * for the last three arguments */
    {
        __m128 dnoise;
        temp = _mm_mul_ps(temp, vec_t);
        temp = _mm_mul_ps(temp, vec_t2);
        dnoise = _mm_mul_ps(vec_v0, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 3, 3, 3)));
        dnoise = _mm_add_ps(dnoise, _mm_mul_ps(vec_v1, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 2, 2, 2))));
        dnoise = _mm_add_ps(dnoise, _mm_mul_ps(vec_v2, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 1, 1, 1))));
        dnoise = _mm_add_ps(dnoise, _mm_mul_ps(vec_v3, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0))));
        dnoise = _mm_mul_ps(dnoise, vec_minus8);
        vec_g0 = _mm_mul_ps(vec_g0, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(3, 3, 3, 3)));
        vec_g0 = _mm_add_ps(vec_g0, _mm_mul_ps(vec_g1, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(2, 2, 2, 2))));
        vec_g0 = _mm_add_ps(vec_g0, _mm_mul_ps(vec_g2, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(1, 1, 1, 1))));
        vec_g0 = _mm_add_ps(vec_g0, _mm_mul_ps(vec_g3, _mm_shuffle_ps(vec_t4, vec_t4, _MM_SHUFFLE(0, 0, 0, 0))));
        dnoise = _mm_add_ps(dnoise, vec_g0);
        dnoise = _mm_mul_ps(dnoise, vec_28);
#if defined(D_PLATFORM_XBOXONE) || defined(D_PLATFORM_PC)
        dnoise_dx = VecIntrinsics::VExtractF<1>( dnoise );
        dnoise_dy = VecIntrinsics::VExtractF<2>( dnoise );
        dnoise_dz = VecIntrinsics::VExtractF<3>( dnoise );
#elif defined ( D_PLATFORM_NX64 ) || defined ( D_PLATFORM_ARM64 )
        dnoise_dx = dnoise[1];
        dnoise_dy = dnoise[2];
        dnoise_dz = dnoise[3];
#else
        dnoise_dx = dnoise[1];
        dnoise_dy = dnoise[2];
        dnoise_dz = dnoise[3];
#endif
    }

    return noise;
}

//-----------------------------------------------------------------------------
//Main noise 3d function
//-----------------------------------------------------------------------------
TKFORCEINLINE
float
Noise3d(float x, float y, float z)
{
#if defined ( D_PLATFORM_NX64 ) || defined ( D_PLATFORM_ARM64 )
    float s = (x + y + z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int i = (int)IntFloor(xs);
    int j = (int)IntFloor(ys);
    int k = (int)IntFloor(zs);
    float t = (float)(i + j + k)*G3;
    float X0 = i - t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; /* The x,y,z distances from the cell origin */
    float y0 = y - Y0;
    float z0 = z - Z0;
    bool g0 = (x0 >= y0), l0 = !g0;
    bool g1 = (y0 >= z0), l1 = !g1;
    bool g2 = (z0 >= x0), l2 = !g2;
    int i1 = g0 & l2;   /* Offsets for second corner of simplex in (i,j,k) coords */
    int j1 = g1 & l0;
    int k1 = g2 & l1;
    int i2 = g0 | l2;   /* Offsets for third corner of simplex in (i,j,k) coords */
    int j2 = g1 | l0;
    int k2 = g2 | l1;
    __m128 vec_v0 = { 0.0f, x0, y0, z0 };
    
#if defined ( D_PLATFORM_NX64 )
    __m128i vec_ijk1 = { 0, i1, j1, k1 };
    __m128i vec_ijk2 = { 0, i2, j2, k2 };
#else
    __m128i vec_ijk1;
    vec_ijk1 = vsetq_lane_s32( 0, vec_ijk1, 0 );
    vec_ijk1 = vsetq_lane_s32( i1, vec_ijk1, 1 );
    vec_ijk1 = vsetq_lane_s32( j1, vec_ijk1, 2 );
    vec_ijk1 = vsetq_lane_s32( k1, vec_ijk1, 3 );
    
    __m128i vec_ijk2;
    vec_ijk2 = vsetq_lane_s32( 0, vec_ijk2, 0 );
    vec_ijk2 = vsetq_lane_s32( i2, vec_ijk2, 1 );
    vec_ijk2 = vsetq_lane_s32( j2, vec_ijk2, 2 );
    vec_ijk2 = vsetq_lane_s32( k2, vec_ijk2, 3 );
#endif
    
    __m128 vec_v1 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_cvtepi32_ps(vec_ijk1)), vec_G3);
    __m128 vec_v2 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_cvtepi32_ps(vec_ijk2)), vec_G3x2);
    __m128 vec_v3 = _mm_add_ps(vec_v0, vec_G3x3);
    /* Calculate the contribution from the four corners */
    __m128 vec_v0_2 = vmulq_f32(vec_v0, vec_v0);
    __m128 vec_v1_2 = vmulq_f32(vec_v1, vec_v1);
    __m128 vec_v2_2 = vmulq_f32(vec_v2, vec_v2);
    __m128 vec_v3_2 = vmulq_f32(vec_v3, vec_v3);
    __m128 vec_t = { vaddvq_f32(vec_v3_2), vaddvq_f32(vec_v2_2), vaddvq_f32(vec_v1_2), vaddvq_f32(vec_v0_2) };
    vec_t = vsubq_f32(vec_0point6, vec_t);
    vec_t = vmaxq_f32(_mm_set1_ps(0.0f), vec_t);
    __m128 vec_g0 = NoiseGrad3Simplex_SIMD(i, j, k);
    __m128 vec_g1 = NoiseGrad3Simplex_SIMD(i + i1, j + j1, k + k1);
    __m128 vec_g2 = NoiseGrad3Simplex_SIMD(i + i2, j + j2, k + k2);
    __m128 vec_g3 = NoiseGrad3Simplex_SIMD(i + 1, j + 1, k + 1);
    __m128 vec_t2 = vmulq_f32(vec_t, vec_t);
    __m128 vec_t4 = vmulq_f32(vec_t2, vec_t2);
    __m128 dotn0 = vmulq_f32(vec_g0, vec_v0);
    __m128 dotn1 = vmulq_f32(vec_g1, vec_v1);
    __m128 dotn2 = vmulq_f32(vec_g2, vec_v2);
    __m128 dotn3 = vmulq_f32(vec_g3, vec_v3);
    __m128 temp = { vaddvq_f32(dotn3), vaddvq_f32(dotn2), vaddvq_f32(dotn1), vaddvq_f32(dotn0) };
    __m128 tempn = vmulq_f32(temp, vec_t4);
    float noise = vaddvq_f32(tempn);
#else
    float s = (x + y + z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int i = (int)IntFloor(xs);
    int j = (int)IntFloor(ys);
    int k = (int)IntFloor(zs);
    float t = (float)(i + j + k)*G3;
    float X0 = i - t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; /* The x,y,z distances from the cell origin */
    float y0 = y - Y0;
    float z0 = z - Z0;
    bool g0 = (x0 >= y0), l0 = !g0;
    bool g1 = (y0 >= z0), l1 = !g1;
    bool g2 = (z0 >= x0), l2 = !g2;
    int i1 = g0 & l2;   /* Offsets for second corner of simplex in (i,j,k) coords */
    int j1 = g1 & l0;
    int k1 = g2 & l1;
    int i2 = g0 | l2;   /* Offsets for third corner of simplex in (i,j,k) coords */
    int j2 = g1 | l0;
    int k2 = g2 | l1;
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    __m128 vec_v0 = _mm_set_ps(z0, y0, x0, 0.0f);
    __m128 vec_v1 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_set_ps((float)k1, (float)j1, (float)i1, 0.0f)), vec_G3);
    __m128 vec_v2 = _mm_add_ps(_mm_sub_ps(vec_v0, _mm_set_ps((float)k2, (float)j2, (float)i2, 0.0f)), vec_G3x2);
    __m128 vec_v3 = _mm_add_ps(vec_v0, vec_G3x3);
    /* Calculate the contribution from the four corners */
    __m128 vec_t;
    __m128 vec_v0_2 = _mm_mul_ps(vec_v0, vec_v0);
    __m128 vec_v1_2 = _mm_mul_ps(vec_v1, vec_v1);
    __m128 vec_v2_2 = _mm_mul_ps(vec_v2, vec_v2);
    __m128 vec_v3_2 = _mm_mul_ps(vec_v3, vec_v3);
    vec_t = _mm_sum4(vec_v3_2, vec_v2_2, vec_v1_2, vec_v0_2);
    vec_t = _mm_sub_ps(vec_0point6, vec_t);
    vec_t = _mm_max_ps(_mm_set1_ps(0.0f), vec_t);
    __m128 vec_g0 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii, jj, kk));
    __m128 vec_g1 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + i1, jj + j1, kk + k1));
    __m128 vec_g2 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + i2, jj + j2, kk + k2));
    __m128 vec_g3 = NoiseGrad3_SSE(SIMPLEX_NOISE_HASH(ii + 1, jj + 1, kk + 1));
    __m128 vec_t2 = _mm_mul_ps(vec_t, vec_t);
    __m128 vec_t4 = _mm_mul_ps(vec_t2, vec_t2);
    __m128 dotn0 = _mm_mul_ps(vec_g0, vec_v0);
    __m128 dotn1 = _mm_mul_ps(vec_g1, vec_v1);
    __m128 dotn2 = _mm_mul_ps(vec_g2, vec_v2);
    __m128 dotn3 = _mm_mul_ps(vec_g3, vec_v3);
    __m128 temp = _mm_sum4(dotn3, dotn2, dotn1, dotn0);
    __m128 tempn = _mm_mul_ps(temp, vec_t4);
    __m128 shuf = _mm_shuffle_ps(tempn, tempn, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(tempn, shuf);
    float noise = _mm_cvtss_f32(_mm_add_ss(sums, _mm_movehl_ps(shuf, sums)));
#endif
    return noise * 28.0f;;

}

#if !defined(D_PLATFORM_NX64)
#define vornq_s32(a,b)  _mm_or_si128(a, _mm_xor_si128 (b, _mm_set1_epi32(-1) ) )
#define vabsq_f32(a)    _mm_and_ps( _mm_castsi128_ps(_mm_srli_epi32(_mm_set1_epi32(-1), 1)), a ) 

#if defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_SCARLETT )

#else

#define _mm_fmadd_ps(a,b,c) _mm_add_ps(_mm_mul_ps(a,b), c)
#define _mm_fnmadd_ps(a,b,c) _mm_sub_ps(c, _mm_mul_ps(a,b) )

#endif


#endif


TKFORCEINLINE
__m128
Noise3dSOA(__m128 x, __m128 y, __m128 z)
{
    __m128 s = _mm_mul_ps(_mm_set1_ps(F3), _mm_add_ps(x, _mm_add_ps(y, z)));
    __m128 xs = _mm_add_ps(x, s);
    __m128 ys = _mm_add_ps(y, s);
    __m128 zs = _mm_add_ps(z, s);

    __m128 xi = VecIntrinsics::VFloor( xs );
    __m128 yi = VecIntrinsics::VFloor( ys );
    __m128 zi = VecIntrinsics::VFloor( zs );

    __m128 t = _mm_mul_ps(_mm_set1_ps(G3), _mm_add_ps(xi, _mm_add_ps(yi, zi)));
    __m128 X0 = _mm_sub_ps(xi, t);
    __m128 Y0 = _mm_sub_ps(yi, t);
    __m128 Z0 = _mm_sub_ps(zi, t);
    __m128 x0 = _mm_sub_ps(x, X0);
    __m128 y0 = _mm_sub_ps(y, Y0);
    __m128 z0 = _mm_sub_ps(z, Z0);

    __m128i px0 = _mm_cvtps_epi32(xi);
    __m128i py0 = _mm_cvtps_epi32(yi);
    __m128i pz0 = _mm_cvtps_epi32(zi);

    __m128 x_ge_y = _mm_cmpge_ps(x0, y0);
    __m128 y_ge_z = _mm_cmpge_ps(y0, z0);
    __m128 z_ge_x = _mm_cmpge_ps(z0, x0);

    __m128 i1 = _mm_andnot_ps(z_ge_x, x_ge_y);
    __m128 j1 = _mm_andnot_ps(x_ge_y, y_ge_z);
    __m128 k1 = _mm_andnot_ps(y_ge_z, z_ge_x);

    __m128 i2 = _mm_castsi128_ps(vornq_s32(_mm_castps_si128(x_ge_y), _mm_castps_si128(z_ge_x)));
    __m128 j2 = _mm_castsi128_ps(vornq_s32(_mm_castps_si128(y_ge_z), _mm_castps_si128(x_ge_y)));
    __m128 k2 = _mm_castsi128_ps(vornq_s32(_mm_castps_si128(z_ge_x), _mm_castps_si128(y_ge_z)));

    __m128 x1 = _mm_sub_ps(x0, _mm_and_ps(i1, _mm_set1_ps(1)));
    __m128 y1 = _mm_sub_ps(y0, _mm_and_ps(j1, _mm_set1_ps(1)));
    __m128 z1 = _mm_sub_ps(z0, _mm_and_ps(k1, _mm_set1_ps(1)));
    __m128 x2 = _mm_sub_ps(x0, _mm_and_ps(i2, _mm_set1_ps(1)));
    __m128 y2 = _mm_sub_ps(y0, _mm_and_ps(j2, _mm_set1_ps(1)));
    __m128 z2 = _mm_sub_ps(z0, _mm_and_ps(k2, _mm_set1_ps(1)));

    x1 = _mm_add_ps(x1, _mm_set1_ps(G3));
    y1 = _mm_add_ps(y1, _mm_set1_ps(G3));
    z1 = _mm_add_ps(z1, _mm_set1_ps(G3));
    x2 = _mm_add_ps(x2, _mm_set1_ps(G3 * 2));
    y2 = _mm_add_ps(y2, _mm_set1_ps(G3 * 2));
    z2 = _mm_add_ps(z2, _mm_set1_ps(G3 * 2));

    __m128 x3 = _mm_add_ps(x0, _mm_set1_ps(G3 * 3 - 1));
    __m128 y3 = _mm_add_ps(y0, _mm_set1_ps(G3 * 3 - 1));
    __m128 z3 = _mm_add_ps(z0, _mm_set1_ps(G3 * 3 - 1));

    __m128 t0 = _mm_fnmadd_ps(x0, x0, _mm_fnmadd_ps(y0, y0, _mm_fnmadd_ps(z0, z0, _mm_set1_ps(0.6f))));
    __m128 t1 = _mm_fnmadd_ps(x1, x1, _mm_fnmadd_ps(y1, y1, _mm_fnmadd_ps(z1, z1, _mm_set1_ps(0.6f))));
    __m128 t2 = _mm_fnmadd_ps(x2, x2, _mm_fnmadd_ps(y2, y2, _mm_fnmadd_ps(z2, z2, _mm_set1_ps(0.6f))));
    __m128 t3 = _mm_fnmadd_ps(x3, x3, _mm_fnmadd_ps(y3, y3, _mm_fnmadd_ps(z3, z3, _mm_set1_ps(0.6f))));

    t0 = _mm_max_ps(t0, _mm_setzero_ps());
    t1 = _mm_max_ps(t1, _mm_setzero_ps());
    t2 = _mm_max_ps(t2, _mm_setzero_ps());
    t3 = _mm_max_ps(t3, _mm_setzero_ps());

    __m128 t20 = _mm_mul_ps(t0, t0);
    __m128 t21 = _mm_mul_ps(t1, t1);
    __m128 t22 = _mm_mul_ps(t2, t2);
    __m128 t23 = _mm_mul_ps(t3, t3);
    __m128 t40 = _mm_mul_ps(t20, t20);
    __m128 t41 = _mm_mul_ps(t21, t21);
    __m128 t42 = _mm_mul_ps(t22, t22);
    __m128 t43 = _mm_mul_ps(t23, t23);

    __m128i px1 = _mm_sub_epi32(px0, _mm_castps_si128(i1));
    __m128i py1 = _mm_sub_epi32(py0, _mm_castps_si128(j1));
    __m128i pz1 = _mm_sub_epi32(pz0, _mm_castps_si128(k1));
    __m128i px2 = _mm_sub_epi32(px0, _mm_castps_si128(i2));
    __m128i py2 = _mm_sub_epi32(py0, _mm_castps_si128(j2));
    __m128i pz2 = _mm_sub_epi32(pz0, _mm_castps_si128(k2));
    __m128i px3 = _mm_sub_epi32(px0, _mm_set1_epi32(-1));
    __m128i py3 = _mm_sub_epi32(py0, _mm_set1_epi32(-1));
    __m128i pz3 = _mm_sub_epi32(pz0, _mm_set1_epi32(-1));

    __m128 gx0, gy0, gz0, n0;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz0)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz0)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz0)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz0)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx0 = _mm_movehl_ps(tmp2, tmp0);
        gy0 = _mm_movelh_ps(tmp1, tmp3);
        gz0 = _mm_movehl_ps(tmp3, tmp1);
        n0 = _mm_fmadd_ps(x0, gx0, _mm_fmadd_ps(y0, gy0, _mm_mul_ps(z0, gz0)));
    }

    __m128 gx1, gy1, gz1, n1;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz1)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz1)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz1)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz1)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx1 = _mm_movehl_ps(tmp2, tmp0);
        gy1 = _mm_movelh_ps(tmp1, tmp3);
        gz1 = _mm_movehl_ps(tmp3, tmp1);
        n1 = _mm_fmadd_ps(x1, gx1, _mm_fmadd_ps(y1, gy1, _mm_mul_ps(z1, gz1)));
    }

    __m128 gx2, gy2, gz2, n2;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz2)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz2)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz2)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz2)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx2 = _mm_movehl_ps(tmp2, tmp0);
        gy2 = _mm_movelh_ps(tmp1, tmp3);
        gz2 = _mm_movehl_ps(tmp3, tmp1);
        n2 = _mm_fmadd_ps(x2, gx2, _mm_fmadd_ps(y2, gy2, _mm_mul_ps(z2, gz2)));
    }

    __m128 gx3, gy3, gz3, n3;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz3)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz3)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz3)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz3)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx3 = _mm_movehl_ps(tmp2, tmp0);
        gy3 = _mm_movelh_ps(tmp1, tmp3);
        gz3 = _mm_movehl_ps(tmp3, tmp1);
        n3 = _mm_fmadd_ps(x3, gx3, _mm_fmadd_ps(y3, gy3, _mm_mul_ps(z3, gz3)));
    }

    __m128 noiseOutput = _mm_mul_ps(_mm_set1_ps(28), _mm_fmadd_ps(n0, t40, _mm_fmadd_ps(n1, t41, _mm_fmadd_ps(n2, t42, _mm_mul_ps(n3, t43)))));
    return noiseOutput;

}


TKFORCEINLINE
__m128
Noise3dSOA(__m128 x, __m128 y, __m128 z, __m128& dx, __m128& dy, __m128& dz)
{
    __m128 s = _mm_mul_ps( _mm_set1_ps(F3), _mm_add_ps(x, _mm_add_ps(y, z)));
    __m128 xs = _mm_add_ps(x, s);
    __m128 ys = _mm_add_ps(y, s);
    __m128 zs = _mm_add_ps(z, s);

    __m128 xi = VecIntrinsics::VFloor( xs );
    __m128 yi = VecIntrinsics::VFloor( ys );
    __m128 zi = VecIntrinsics::VFloor( zs );

    __m128 t = _mm_mul_ps(_mm_set1_ps(G3), _mm_add_ps(xi, _mm_add_ps(yi, zi)));
    __m128 X0 = _mm_sub_ps(xi, t);
    __m128 Y0 = _mm_sub_ps(yi, t);
    __m128 Z0 = _mm_sub_ps(zi, t);
    __m128 x0 = _mm_sub_ps(x, X0);
    __m128 y0 = _mm_sub_ps(y, Y0);
    __m128 z0 = _mm_sub_ps(z, Z0);

    __m128i px0 = _mm_cvtps_epi32( xi );
    __m128i py0 = _mm_cvtps_epi32( yi );
    __m128i pz0 = _mm_cvtps_epi32( zi );

    __m128 x_ge_y = _mm_cmpge_ps(x0, y0);
    __m128 y_ge_z = _mm_cmpge_ps(y0, z0);
    __m128 z_ge_x = _mm_cmpge_ps(z0, x0);
    
    __m128 i1 = _mm_andnot_ps(z_ge_x, x_ge_y);
    __m128 j1 = _mm_andnot_ps(x_ge_y, y_ge_z);
    __m128 k1 = _mm_andnot_ps(y_ge_z, z_ge_x);
    
    __m128 i2 = _mm_castsi128_ps( vornq_s32( _mm_castps_si128( x_ge_y ), _mm_castps_si128(z_ge_x) ) );
    __m128 j2 = _mm_castsi128_ps( vornq_s32( _mm_castps_si128( y_ge_z ), _mm_castps_si128(x_ge_y) ) );
    __m128 k2 = _mm_castsi128_ps( vornq_s32( _mm_castps_si128( z_ge_x ), _mm_castps_si128(y_ge_z) ) );

    __m128 x1 = _mm_sub_ps( x0, _mm_and_ps( i1, _mm_set1_ps( 1 ) ));
    __m128 y1 = _mm_sub_ps( y0, _mm_and_ps( j1, _mm_set1_ps( 1 ) ));
    __m128 z1 = _mm_sub_ps( z0, _mm_and_ps( k1, _mm_set1_ps( 1 ) ));
    __m128 x2 = _mm_sub_ps( x0, _mm_and_ps( i2, _mm_set1_ps( 1 ) ));
    __m128 y2 = _mm_sub_ps( y0, _mm_and_ps( j2, _mm_set1_ps( 1 ) ));
    __m128 z2 = _mm_sub_ps( z0, _mm_and_ps( k2, _mm_set1_ps( 1 ) ));

    x1 = _mm_add_ps(x1, _mm_set1_ps(G3));
    y1 = _mm_add_ps(y1, _mm_set1_ps(G3));
    z1 = _mm_add_ps(z1, _mm_set1_ps(G3));
    x2 = _mm_add_ps(x2, _mm_set1_ps(G3 * 2));
    y2 = _mm_add_ps(y2, _mm_set1_ps(G3 * 2));
    z2 = _mm_add_ps(z2, _mm_set1_ps(G3 * 2));

    __m128 x3 = _mm_add_ps(x0, _mm_set1_ps(G3 * 3 - 1));
    __m128 y3 = _mm_add_ps(y0, _mm_set1_ps(G3 * 3 - 1));
    __m128 z3 = _mm_add_ps(z0, _mm_set1_ps(G3 * 3 - 1));
    
    __m128 t0 = _mm_fnmadd_ps( x0, x0, _mm_fnmadd_ps( y0, y0, _mm_fnmadd_ps( z0, z0, _mm_set1_ps( 0.6f ) ) ) );
    __m128 t1 = _mm_fnmadd_ps( x1, x1, _mm_fnmadd_ps( y1, y1, _mm_fnmadd_ps( z1, z1, _mm_set1_ps( 0.6f ) ) ) );
    __m128 t2 = _mm_fnmadd_ps( x2, x2, _mm_fnmadd_ps( y2, y2, _mm_fnmadd_ps( z2, z2, _mm_set1_ps( 0.6f ) ) ) );
    __m128 t3 = _mm_fnmadd_ps( x3, x3, _mm_fnmadd_ps( y3, y3, _mm_fnmadd_ps( z3, z3, _mm_set1_ps( 0.6f ) ) ) );

    t0 = _mm_max_ps( t0, _mm_setzero_ps() );
    t1 = _mm_max_ps( t1, _mm_setzero_ps() );
    t2 = _mm_max_ps( t2, _mm_setzero_ps() );
    t3 = _mm_max_ps( t3, _mm_setzero_ps() );

    __m128 t20 = _mm_mul_ps(t0, t0);
    __m128 t21 = _mm_mul_ps(t1, t1);
    __m128 t22 = _mm_mul_ps(t2, t2);
    __m128 t23 = _mm_mul_ps(t3, t3);
    __m128 t40 = _mm_mul_ps(t20, t20);
    __m128 t41 = _mm_mul_ps(t21, t21);
    __m128 t42 = _mm_mul_ps(t22, t22);
    __m128 t43 = _mm_mul_ps(t23, t23);           
        
    __m128i px1 = _mm_sub_epi32(px0, _mm_castps_si128(i1));
    __m128i py1 = _mm_sub_epi32(py0, _mm_castps_si128(j1));
    __m128i pz1 = _mm_sub_epi32(pz0, _mm_castps_si128(k1));
    __m128i px2 = _mm_sub_epi32(px0, _mm_castps_si128(i2));
    __m128i py2 = _mm_sub_epi32(py0, _mm_castps_si128(j2));
    __m128i pz2 = _mm_sub_epi32(pz0, _mm_castps_si128(k2));
    __m128i px3 = _mm_sub_epi32(px0, _mm_set1_epi32(-1));
    __m128i py3 = _mm_sub_epi32(py0, _mm_set1_epi32(-1));
    __m128i pz3 = _mm_sub_epi32(pz0, _mm_set1_epi32(-1));

    __m128 gx0, gy0, gz0, n0;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz0)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz0)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz0)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px0)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py0)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz0)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx0 = _mm_movehl_ps(tmp2, tmp0);
        gy0 = _mm_movelh_ps(tmp1, tmp3);
        gz0 = _mm_movehl_ps(tmp3, tmp1);
        n0 = _mm_fmadd_ps(x0, gx0, _mm_fmadd_ps(y0, gy0, _mm_mul_ps(z0, gz0)));
    }

    __m128 gx1, gy1, gz1, n1;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz1)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz1)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz1)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px1)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py1)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz1)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx1 = _mm_movehl_ps(tmp2, tmp0);
        gy1 = _mm_movelh_ps(tmp1, tmp3);
        gz1 = _mm_movehl_ps(tmp3, tmp1);
        n1 = _mm_fmadd_ps(x1, gx1, _mm_fmadd_ps(y1, gy1, _mm_mul_ps(z1, gz1)));
    }

    __m128 gx2, gy2, gz2, n2;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz2)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz2)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz2)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px2)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py2)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz2)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx2 = _mm_movehl_ps(tmp2, tmp0);
        gy2 = _mm_movelh_ps(tmp1, tmp3);
        gz2 = _mm_movehl_ps(tmp3, tmp1);
        n2 = _mm_fmadd_ps(x2, gx2, _mm_fmadd_ps(y2, gy2, _mm_mul_ps(z2, gz2)));
    }

    __m128 gx3, gy3, gz3, n3;
    {
        __m128 row0 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<0>(_mm_castsi128_ps(pz3)));
        __m128 row1 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<1>(_mm_castsi128_ps(pz3)));
        __m128 row2 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<2>(_mm_castsi128_ps(pz3)));
        __m128 row3 = NoiseGrad3Simplex_SIMD(VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(px3)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(py3)), VecIntrinsics::VExtractI<3>(_mm_castsi128_ps(pz3)));
        __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
        __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
        __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
        __m128 tmp3 = _mm_unpackhi_ps(row2, row3);
        gx3 = _mm_movehl_ps(tmp2, tmp0);
        gy3 = _mm_movelh_ps(tmp1, tmp3);
        gz3 = _mm_movehl_ps(tmp3, tmp1);
        n3 = _mm_fmadd_ps(x3, gx3, _mm_fmadd_ps(y3, gy3, _mm_mul_ps(z3, gz3)));
    }

    __m128 noiseOutput = _mm_mul_ps(_mm_set1_ps(28), _mm_fmadd_ps(n0, t40, _mm_fmadd_ps(n1, t41, _mm_fmadd_ps(n2, t42, _mm_mul_ps(n3, t43)))));

    __m128 temp0 = _mm_mul_ps( t20, _mm_mul_ps( t0, n0 ));
    dx = _mm_mul_ps( temp0, x0 );
    dy = _mm_mul_ps( temp0, y0 );
    dz = _mm_mul_ps( temp0, z0 );
    __m128 temp1 = _mm_mul_ps( t21, _mm_mul_ps( t1, n1 ));
    dx = _mm_fmadd_ps( temp1, x1, dx );
    dy = _mm_fmadd_ps( temp1, y1, dy );
    dz = _mm_fmadd_ps( temp1, z1, dz );
    __m128 temp2 = _mm_mul_ps( t22, _mm_mul_ps( t2, n2 ));
    dx = _mm_fmadd_ps( temp2, x2, dx );
    dy = _mm_fmadd_ps( temp2, y2, dy );
    dz = _mm_fmadd_ps( temp2, z2, dz );
    __m128 temp3 = _mm_mul_ps( t23, _mm_mul_ps( t3, n3 ));
    dx = _mm_fmadd_ps( temp3, x3, dx );
    dy = _mm_fmadd_ps( temp3, y3, dy );
    dz = _mm_fmadd_ps( temp3, z3, dz );
    __m128 tgx = _mm_fmadd_ps( t40, gx0, _mm_fmadd_ps( t41, gx1, _mm_fmadd_ps( t42, gx2, _mm_mul_ps( t43, gx3 ))));
    __m128 tgy = _mm_fmadd_ps( t40, gy0, _mm_fmadd_ps( t41, gy1, _mm_fmadd_ps( t42, gy2, _mm_mul_ps( t43, gy3 ))));
    __m128 tgz = _mm_fmadd_ps( t40, gz0, _mm_fmadd_ps( t41, gz1, _mm_fmadd_ps( t42, gz2, _mm_mul_ps( t43, gz3 ))));
    dx = _mm_fmadd_ps( dx, _mm_set1_ps( -8.0f ), tgx ); // vmlsq_n_f32
    dy = _mm_fmadd_ps( dy, _mm_set1_ps( -8.0f ), tgy );
    dz = _mm_fmadd_ps( dz, _mm_set1_ps( -8.0f ), tgz );
    dx = _mm_mul_ps( dx, _mm_set1_ps( 28.0f )); /* Scale derivative to match the noise scaling */
    dy = _mm_mul_ps( dy, _mm_set1_ps( 28.0f ));
    dz = _mm_mul_ps( dz, _mm_set1_ps( 28.0f ));

    return noiseOutput;
}

#else

float
Noise3d(float x, float y, float z)
{
    float noise;          /* Return value */
    float x1, y1, z1, x2, y2, z2, x3, y3, z3;
    float t0, t1, t2, t3, t20, t40, t21, t41, t22, t42, t23, t43;

    /* Skew the input space to determine which simplex cell we're in */
    float s = (x + y + z)*F3; /* Very nice and simple skew factor for 3D */
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int i = (int)IntFloor(xs);
    int j = (int)IntFloor(ys);
    int k = (int)IntFloor(zs);

    float t = (float)(i + j + k)*G3;
    float X0 = i - t; /* Unskew the cell origin back to (x,y,z) space */
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; /* The x,y,z distances from the cell origin */
    float y0 = y - Y0;
    float z0 = z - Z0;

    /* For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    * Determine which simplex we are in. */
    bool g0 = (x0 >= y0), l0 = !g0;
    bool g1 = (y0 >= z0), l1 = !g1;
    bool g2 = (z0 >= x0), l2 = !g2;
    int i1 = int(g0) & int(l2);   /* Offsets for second corner of simplex in (i,j,k) coords */
    int j1 = int(g1) & int(l0);
    int k1 = int(g2) & int(l1);
    int i2 = int(g0) | int(l2);   /* Offsets for third corner of simplex in (i,j,k) coords */
    int j2 = int(g1) | int(l0);
    int k2 = int(g2) | int(l1);

    /* A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    * a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    * a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    * c = 1/6.   */

    x1 = x0 - i1 + G3; /* Offsets for second corner in (x,y,z) coords */
    y1 = y0 - j1 + G3;
    z1 = z0 - k1 + G3;
    x2 = x0 - i2 + 2.0f * G3; /* Offsets for third corner in (x,y,z) coords */
    y2 = y0 - j2 + 2.0f * G3;
    z2 = z0 - k2 + 2.0f * G3;
    x3 = x0 - 1.0f + 3.0f * G3; /* Offsets for last corner in (x,y,z) coords */
    y3 = y0 - 1.0f + 3.0f * G3;
    z3 = z0 - 1.0f + 3.0f * G3;

    /* Wrap the integer indices at 256, to avoid indexing perm[] out of bounds */
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;

    /* Calculate the contribution from the four corners */
    float n = 0;
    t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 >= 0.0f)
    {
        vec3 g = NoiseGrad3(SIMPLEX_NOISE_HASH(ii, jj, kk));
        t20 = t0 * t0;
        t40 = t20 * t20;
        n += t40 * (g.x * x0 + g.y * y0 + g.z * z0);
    }

    t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 >= 0.0f)
    {
        vec3 g = NoiseGrad3(SIMPLEX_NOISE_HASH(ii + i1, jj + j1, kk + k1));
        t21 = t1 * t1;
        t41 = t21 * t21;
        n += t41 * (g.x * x1 + g.y * y1 + g.z * z1);
    }

    t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 >= 0.0f)
    {
        vec3 g = NoiseGrad3(SIMPLEX_NOISE_HASH(ii + i2, jj + j2, kk + k2));
        t22 = t2 * t2;
        t42 = t22 * t22;
        n += t42 * (g.x * x2 + g.y * y2 + g.z * z2);
    }

    t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 >= 0.0f)
    {
        vec3 g = NoiseGrad3(SIMPLEX_NOISE_HASH(ii + 1, jj + 1, kk + 1));
        t23 = t3 * t3;
        t43 = t23 * t23;
        n += t43 * (g.x * x3 + g.y * y3 + g.z * z3);
    }

    /*  Add contributions from each corner to get the final noise value.
    * The result is scaled to return values in the range [-1,1] */
    noise = 28.0f * n;
    return noise;
}

#endif


#undef SIMPLEX_NOISE_HASH

//-----------------------------------------------------------------------------
//Scales noise somehow - ask somebody!
//-----------------------------------------------------------------------------

TKFORCEINLINE
float
ScaleNoise(
    float lfNoise,
    float lfOneMinusAmount,
    float lfOneOverAmount)
{
    float lfRatioInv   = lfOneMinusAmount;
    float lfRatioRecip = lfOneOverAmount;

    //lfNoise = ( kTkMath.Max( lfNoise - lfRatioInv, 0.0f ) );
    lfNoise = lfNoise - lfRatioInv;
    lfNoise = lfNoise * lfRatioRecip;

    return lfNoise;
}
TKFORCEINLINE
float
ScaleNoise(
    float lfNoise,
    float lfAmount )
{
    return ScaleNoise(lfNoise,1.0f-lfAmount,1.0f / lfAmount);
}

//-----------------------------------------------------------------------------
//Octave noise function
//-----------------------------------------------------------------------------
float
OctaveNoise(
    vec3 lPosition,
    int liNumOctaves)
{
    float lfTotal = 0.0f;
    float lfFrequency = 1.0f;
    float lfAmplitude = 1.0f;

    // We have to keep track of the largest possible amplitude,
    // because each octave adds more, and we need a value in [-1, 1].
#if defined( __cplusplus )
    int liOctave = 0;
    const __m128 lvFrequency = { 1.0f, 2.0f, 4.0f, 8.0f };
    for (; liOctave < liNumOctaves - 3; liOctave += 4)
    {
        vec3 lOctavePosition = lPosition * lfFrequency;
        __m128 lOctavePositionX = _mm_mul_ps(_mm_set1_ps(lOctavePosition.x), lvFrequency);
        __m128 lOctavePositionY = _mm_mul_ps(_mm_set1_ps(lOctavePosition.y), lvFrequency);
        __m128 lOctavePositionZ = _mm_mul_ps(_mm_set1_ps(lOctavePosition.z), lvFrequency);
        __m128 lNoise = Noise3dSOA(lOctavePositionX, lOctavePositionY, lOctavePositionZ);
        lfFrequency *= 16.0f;
#if defined(D_PLATFORM_XBOXONE) || defined(D_PLATFORM_PC)
        lfTotal += VecIntrinsics::VExtractF<0>( lNoise ) * lfAmplitude;
        lfAmplitude *= 0.5f;
        lfTotal += VecIntrinsics::VExtractF<1>( lNoise ) * lfAmplitude;
        lfAmplitude *= 0.5f;                            
        lfTotal += VecIntrinsics::VExtractF<2>( lNoise ) * lfAmplitude;
        lfAmplitude *= 0.5f;                           
        lfTotal += VecIntrinsics::VExtractF<3>( lNoise ) * lfAmplitude;
        lfAmplitude *= 0.5f;
#else        
        lfTotal += lNoise[0] * lfAmplitude;
        lfAmplitude *= 0.5f;
        lfTotal += lNoise[1] * lfAmplitude;
        lfAmplitude *= 0.5f;
        lfTotal += lNoise[2] * lfAmplitude;
        lfAmplitude *= 0.5f;
        lfTotal += lNoise[3] * lfAmplitude;
        lfAmplitude *= 0.5f;
#endif
    }
    for (; liOctave < liNumOctaves; liOctave++)
    {
        lfTotal += Noise3d(lPosition.x * lfFrequency, lPosition.y * lfFrequency, lPosition.z * lfFrequency) * lfAmplitude;
        lfFrequency *= 2.0f;
        lfAmplitude *= 0.5f;
    }
    static const float lfRcpMaxAmp[11] =
    {
        0.0f,
        1.0f / (1.0f),
        1.0f / (1.0f + 0.5f),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f)),
        1.0f / (1.0f + 0.5f + (0.5f * 0.5f) + (0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f) + (0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f * 0.5f))
    };
    TKASSERT(liNumOctaves < 11);
    return lfTotal * lfRcpMaxAmp[liNumOctaves];

#else
    float lfMaxAmplitude = 0;
    for (int liOctave = 0; liOctave < liNumOctaves; liOctave++)
    {
        lfTotal += Noise3d(lPosition.x * lfFrequency, lPosition.y * lfFrequency, lPosition.z * lfFrequency) * lfAmplitude;

        lfFrequency *= 2.0f;
        lfMaxAmplitude += lfAmplitude;
        lfAmplitude *= 0.5f;
    }
    return lfTotal / lfMaxAmplitude;
#endif

}

TKFORCEINLINE
float
Remap(
	float original_value,
	float original_min,
	float original_max,
	float new_min,
	float new_max)
{
	return new_min + (saturate((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}

TKFORCEINLINE
float 
GetBias(
    float lfValue, 
    float lfBias )
{
    return (lfValue / ((((1.0f / lfBias) - 2.0f)*(1.0f - lfValue)) + 1.0f));
}

TKFORCEINLINE
float
GetGain(
    float lfValue,
    float lfGain )
{
    if (lfValue < 0.5f)
    {
        return GetBias( lfValue * 2.0f, lfGain ) / 2.0f;
    }
    else
    {
        return GetBias( lfValue * 2.0f - 1.0f, 1.0f - lfGain ) / 2.0f + 0.5f;
    }
}

#if defined( __cplusplus )
TKFORCEINLINE
__m128
Remap(
    __m128 original_value,
    __m128 original_min,
    __m128 original_max,
    __m128 new_min,
    __m128 new_max)
{
    __m128 tmp = _mm_div_ps(_mm_sub_ps(original_value, original_min), _mm_sub_ps(original_max, original_min));

    return _mm_fmadd_ps(_mm_min_ps(_mm_set1_ps(1.0f), _mm_max_ps(_mm_setzero_ps(), tmp)), _mm_sub_ps(new_max, new_min), new_min);
}

TKFORCEINLINE
__m128
GetBias(
    __m128 lfValue,
    __m128 lfBias)
{
    return _mm_div_ps(lfValue, _mm_fmadd_ps(_mm_sub_ps(_mm_div_ps(_mm_set1_ps(1.0f), lfBias), _mm_set1_ps(2.0f)), _mm_sub_ps(_mm_set1_ps(1.0f), lfValue), _mm_set1_ps(1.0f)));
}

TKFORCEINLINE
__m128
GetGain(
    __m128 lfValue,
    __m128 lfGain )
{
    __m128 ge05 = _mm_cmpge_ps(lfValue, _mm_set1_ps(0.5f));

    __m128 bias = GetBias(
        _mm_fmadd_ps(lfValue, _mm_set1_ps(2.0f), _mm_and_ps(_mm_set1_ps(-1.0f), ge05)),
        VecIntrinsics::VSelect(ge05, lfGain, _mm_sub_ps(_mm_set1_ps(1.0f), lfGain)));

    return _mm_fmadd_ps(bias, _mm_set1_ps(0.5f), _mm_and_ps(_mm_set1_ps(0.5f), ge05));    
}
#endif

//-----------------------------------------------------------------------------
//Uber noise function
//-----------------------------------------------------------------------------
float
UberNoise(
    vec3  lPosition,
    int   liOctaves,
    float lfPerturbFeatures,      
    float lfSharpToRoundFeatures, 
    float lfAmplifyFeatures,
    float lfAltitudeErosion,
    float lfRidgeErosion,         
    float lfSlopeErosion,         
    float lfLacunarity,           
    float lfGain,
    float lfRemapFromMin,
    float lfRemapFromMax,
    float lfRemapToMin,
    float lfRemapToMax,
    float lfSlopeGain,
    float lfSlopeBias )                
{
    const float kfStartAmplitude = 0.9f;

    float lfSum       = 0.0f;
    float lfFrequency = 1.0f;
    float lfAmplitude = liOctaves == 1 ? 1.0f : kfStartAmplitude;
    float lfSharpness = Round(lfSharpToRoundFeatures);

    vec3 lSlopeErosionDerivativeSum      = vec3( 0.0f );
    vec3 lPerturbDerivativeSum           = vec3( 0.0f );
    vec3 lRidgeErosionDerivativeSum      = vec3( 0.0f );

    float lfDampedAmplitude = lfAmplitude;
    float lfCurrentGain     = lfGain + lfAmplifyFeatures;
    //float lfCurrentGain = lfGain + (lfSlopeErosion * 0.75f);

    for ( int liIndex = 0; liIndex < liOctaves; liIndex++ )
    {
        vec3 lOctavePosition;
        vec3 lDerivative;
        lOctavePosition = ( lPosition * lfFrequency ) + lPerturbDerivativeSum;

#if defined( __cplusplus )

#if defined( D_PLATFORM_NX64 ) || defined ( D_PLATFORM_ARM64 )
        float lfNoise = Noise3dNEON(lOctavePosition.x, lOctavePosition.y, lOctavePosition.z, lDerivative.x, lDerivative.y, lDerivative.z);
#else       
        float lfNoise = Noise3dSSE( lOctavePosition.x, lOctavePosition.y, lOctavePosition.z, lDerivative.x, lDerivative.y, lDerivative.z );
#endif

#else
        vec4 dbg;
        float lfNoise = Noise3d( lOctavePosition.x, lOctavePosition.y, lOctavePosition.z, lDerivative.x, lDerivative.y, lDerivative.z, dbg );
#endif
        float lfFeatureNoise = lfNoise;

        lDerivative *= lfFeatureNoise;

        {
            float lfRidgedNoise = ( 1.0f - abs( lfFeatureNoise ) ) * 0.6f;
            float lfBillowNoise = lfFeatureNoise * lfFeatureNoise;
            lfFeatureNoise = lerp( lfFeatureNoise, lfBillowNoise, max( 0.0, lfSharpness ) );
            lfFeatureNoise = lerp( lfFeatureNoise, lfRidgedNoise, abs( min( 0.0, lfSharpness ) ) );
        }

        lSlopeErosionDerivativeSum += lDerivative * lfSlopeErosion;
        lRidgeErosionDerivativeSum += lDerivative * 0.8f;
        lPerturbDerivativeSum      += lDerivative * lfPerturbFeatures;

        lfSum += lfDampedAmplitude * lfFeatureNoise * ( 1.0f / ( 1.0f + dot( lSlopeErosionDerivativeSum, lSlopeErosionDerivativeSum ) ) );

        lfFrequency      *= lfLacunarity;
        lfAmplitude      *= lerp( lfCurrentGain, lfCurrentGain * smoothstep( 0.0f, 1.0f, lfSum ), lfAltitudeErosion );
        lfCurrentGain     = lfGain;
        lfDampedAmplitude = lfAmplitude * ( 1.0f - ( lfRidgeErosion / ( 1.0f + dot( lRidgeErosionDerivativeSum, lRidgeErosionDerivativeSum ) ) ) );
    }

    //clamp at 2^12 precision
    float lfResult = lfSharpness == 0.0f ? ( lfSum + 1.0f ) * 0.5f : lfSum;
    lfResult = Remap(lfResult, lfRemapFromMin, lfRemapFromMax, lfRemapToMin, lfRemapToMax);
    lfResult = GetGain(lfResult, lfSlopeGain);
    lfResult = GetBias( lfResult, lfSlopeBias );

    /*float lfResult2 = ldexp(round(ldexp(lfResult,12)),-12);
    float lfResult1 = ClampPrecision(lfResult,12);
    if (lfResult2 != lfResult1)
    {
           TKPRINT(("different - %f vs %f (%x vs %x. orig = %x)\n", lfResult2, lfResult1, *(sUInt32 *)(&lfResult2), *(sUInt32 *)(&lfResult1) , *(sUInt32 *)(&lfResult)) );
    }*/
    //lfResult = ClampPrecision(lfResult, 12);
    return lfResult;
}

#if defined( __cplusplus )
void
UberNoiseSOA4(
    float* lpStore,
    vec3* lPos,
    int   liOctaves,
    float lfPerturbFeatures,      
    float lfSharpToRoundFeatures, 
    float lfAmplifyFeatures,
    float lfAltitudeErosion,
    float lfRidgeErosion,         
    float lfSlopeErosion,         
    float lfLacunarity,           
    float lfGain,
    float lfRemapFromMin,
    float lfRemapFromMax,
    float lfRemapToMin,
    float lfRemapToMax,
    float lfSlopeGain,
    float lfSlopeBias )                
{
#if 0
    lpStore[0] = GPU::UberNoise(lPos[0], liOctaves, lfPerturbFeatures, lfSharpToRoundFeatures, lfAmplifyFeatures, lfAltitudeErosion, lfRidgeErosion, lfSlopeErosion, lfLacunarity, lfGain, lfRemapFromMin, lfRemapFromMax, lfRemapToMin, lfRemapToMax, lfSlopeGain, lfSlopeBias);
    lpStore[1] = GPU::UberNoise(lPos[1], liOctaves, lfPerturbFeatures, lfSharpToRoundFeatures, lfAmplifyFeatures, lfAltitudeErosion, lfRidgeErosion, lfSlopeErosion, lfLacunarity, lfGain, lfRemapFromMin, lfRemapFromMax, lfRemapToMin, lfRemapToMax, lfSlopeGain, lfSlopeBias);
    lpStore[2] = GPU::UberNoise(lPos[2], liOctaves, lfPerturbFeatures, lfSharpToRoundFeatures, lfAmplifyFeatures, lfAltitudeErosion, lfRidgeErosion, lfSlopeErosion, lfLacunarity, lfGain, lfRemapFromMin, lfRemapFromMax, lfRemapToMin, lfRemapToMax, lfSlopeGain, lfSlopeBias);
    lpStore[3] = GPU::UberNoise(lPos[3], liOctaves, lfPerturbFeatures, lfSharpToRoundFeatures, lfAmplifyFeatures, lfAltitudeErosion, lfRidgeErosion, lfSlopeErosion, lfLacunarity, lfGain, lfRemapFromMin, lfRemapFromMax, lfRemapToMin, lfRemapToMax, lfSlopeGain, lfSlopeBias);
#else
    const float kfStartAmplitude = 0.9f;

    __m128 lSum      = _mm_setzero_ps();
    __m128 lAmplitude = _mm_set1_ps( liOctaves == 1 ? 1.0f : kfStartAmplitude );
    __m128 lFrequency = _mm_set1_ps(1.0f);
    __m128 lLacunarity = _mm_set1_ps(lfLacunarity);
    __m128 lSlopeErosion = _mm_set1_ps(lfSlopeErosion);
    __m128 lRidgeErosion = _mm_set1_ps(lfRidgeErosion);
    __m128 lAltitudeErosion = _mm_set1_ps(lfAltitudeErosion);
    __m128 lPerturbFeatures = _mm_set1_ps(lfPerturbFeatures);

    float lfSharpness = Round(lfSharpToRoundFeatures);
    __m128 lBillowLerp = _mm_set1_ps(max(0.0, lfSharpness));
    __m128 lRidgedLerp = _mm_set1_ps(abs( min( 0.0, lfSharpness )));

    __m128 lDampedAmplitude = lAmplitude;
    __m128 lCurrentGain     = _mm_set1_ps( lfGain + lfAmplifyFeatures );
    //float lfCurrentGain = lfGain + (lfSlopeErosion * 0.75f);

    __m128 lPos0 = _mm_loadu_ps((float*)lPos);
    __m128 lPos1 = _mm_loadu_ps((float*)(lPos+1));
    __m128 lPos2 = _mm_loadu_ps((float*)(lPos+2));
    __m128 lPos3 = _mm_loadu_ps((float*)(lPos+3));
    
    __m128 tmp0 = _mm_unpacklo_ps(lPos0, lPos1);
    __m128 tmp2 = _mm_unpacklo_ps(lPos2, lPos3);
    __m128 tmp1 = _mm_unpackhi_ps(lPos0, lPos1);
    __m128 tmp3 = _mm_unpackhi_ps(lPos2, lPos3);
	__m128 lPositionX = _mm_movelh_ps(tmp0, tmp2);
    __m128 lPositionY = _mm_movehl_ps(tmp2, tmp0);
    __m128 lPositionZ = _mm_movelh_ps(tmp1, tmp3);

    __m128 lSlopeErosionDerivativeSumX = _mm_setzero_ps();
    __m128 lSlopeErosionDerivativeSumY = _mm_setzero_ps();
    __m128 lSlopeErosionDerivativeSumZ = _mm_setzero_ps();
    __m128 lPerturbDerivativeSumX = _mm_setzero_ps();
    __m128 lPerturbDerivativeSumY = _mm_setzero_ps();
    __m128 lPerturbDerivativeSumZ = _mm_setzero_ps();
    __m128 lRidgeErosionDerivativeSumX = _mm_setzero_ps();
    __m128 lRidgeErosionDerivativeSumY = _mm_setzero_ps();
    __m128 lRidgeErosionDerivativeSumZ = _mm_setzero_ps();

    for ( int liIndex = 0; liIndex < liOctaves; liIndex++ )
    {
        __m128 lOctavePositionX = _mm_fmadd_ps( lPositionX, lFrequency, lPerturbDerivativeSumX );
        __m128 lOctavePositionY = _mm_fmadd_ps( lPositionY, lFrequency, lPerturbDerivativeSumY );
        __m128 lOctavePositionZ = _mm_fmadd_ps( lPositionZ, lFrequency, lPerturbDerivativeSumZ );
        __m128 lDerivativeX, lDerivativeY, lDerivativeZ;

#if 1
        __m128 lNoise = Noise3dSOA(lOctavePositionX, lOctavePositionY, lOctavePositionZ, lDerivativeX, lDerivativeY, lDerivativeZ);
#else
        __m128 lNoise;
        vec4 dbg;
        float dx, dy, dz;
        lNoise[0] = Noise3d( lOctavePositionX[0], lOctavePositionY[0], lOctavePositionZ[0], dx, dy, dz, dbg );
        lDerivativeX[0] = dx; lDerivativeY[0] = dy; lDerivativeZ[0] = dz;
        lNoise[1] = Noise3d(lOctavePositionX[1], lOctavePositionY[1], lOctavePositionZ[1], dx, dy, dz, dbg);
        lDerivativeX[1] = dx; lDerivativeY[1] = dy; lDerivativeZ[1] = dz;
        lNoise[2] = Noise3d(lOctavePositionX[2], lOctavePositionY[2], lOctavePositionZ[2], dx, dy, dz, dbg);
        lDerivativeX[2] = dx; lDerivativeY[2] = dy; lDerivativeZ[2] = dz;
        lNoise[3] = Noise3d(lOctavePositionX[3], lOctavePositionY[3], lOctavePositionZ[3], dx, dy, dz, dbg);
        lDerivativeX[3] = dx; lDerivativeY[3] = dy; lDerivativeZ[3] = dz;
#endif
        __m128 lFeatureNoise = lNoise;

        lDerivativeX = _mm_mul_ps( lDerivativeX, lFeatureNoise );
        lDerivativeY = _mm_mul_ps( lDerivativeY, lFeatureNoise );
        lDerivativeZ = _mm_mul_ps( lDerivativeZ, lFeatureNoise );

        {
            __m128 lRidgedNoise = _mm_mul_ps( _mm_sub_ps( _mm_set1_ps( 1.0f ), vabsq_f32( lFeatureNoise ) ), _mm_set1_ps( 0.6f ));
            __m128 lBillowNoise = _mm_mul_ps( lFeatureNoise, lFeatureNoise );
            lFeatureNoise = _mm_fmadd_ps(lBillowLerp, _mm_sub_ps(lBillowNoise, lFeatureNoise), lFeatureNoise);
            lFeatureNoise = _mm_fmadd_ps(lRidgedLerp, _mm_sub_ps(lRidgedNoise, lFeatureNoise), lFeatureNoise);
        }
        
        lSlopeErosionDerivativeSumX = _mm_fmadd_ps(lDerivativeX, lSlopeErosion, lSlopeErosionDerivativeSumX);
        lSlopeErosionDerivativeSumY = _mm_fmadd_ps(lDerivativeY, lSlopeErosion, lSlopeErosionDerivativeSumY);
        lSlopeErosionDerivativeSumZ = _mm_fmadd_ps(lDerivativeZ, lSlopeErosion, lSlopeErosionDerivativeSumZ);

        lRidgeErosionDerivativeSumX = _mm_fmadd_ps(lDerivativeX, _mm_set1_ps( 0.8f ), lRidgeErosionDerivativeSumX);
        lRidgeErosionDerivativeSumY = _mm_fmadd_ps(lDerivativeY, _mm_set1_ps( 0.8f ), lRidgeErosionDerivativeSumY);
        lRidgeErosionDerivativeSumZ = _mm_fmadd_ps(lDerivativeZ, _mm_set1_ps( 0.8f ), lRidgeErosionDerivativeSumZ);

        lPerturbDerivativeSumX = _mm_fmadd_ps(lDerivativeX, lPerturbFeatures, lPerturbDerivativeSumX);
        lPerturbDerivativeSumY = _mm_fmadd_ps(lDerivativeY, lPerturbFeatures, lPerturbDerivativeSumY);
        lPerturbDerivativeSumZ = _mm_fmadd_ps(lDerivativeZ, lPerturbFeatures, lPerturbDerivativeSumZ);

        __m128 lSlopeErosionDerivativeSumDot = _mm_mul_ps(lSlopeErosionDerivativeSumX, lSlopeErosionDerivativeSumX);
        lSlopeErosionDerivativeSumDot = _mm_fmadd_ps(lSlopeErosionDerivativeSumY, lSlopeErosionDerivativeSumY, lSlopeErosionDerivativeSumDot);
        lSlopeErosionDerivativeSumDot = _mm_fmadd_ps(lSlopeErosionDerivativeSumZ, lSlopeErosionDerivativeSumZ, lSlopeErosionDerivativeSumDot);

        lFeatureNoise = _mm_mul_ps(lDampedAmplitude, lFeatureNoise);

        lSum = _mm_fmadd_ps( lFeatureNoise, _mm_div_ps( _mm_set1_ps(1.0f), _mm_add_ps( _mm_set1_ps( 1.0f ), lSlopeErosionDerivativeSumDot ) ), lSum );

        lFrequency = _mm_mul_ps( lFrequency, lLacunarity );

        __m128 lSmoothStepSum = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(_mm_set1_ps(1.0f), lSum));
        lSmoothStepSum = _mm_mul_ps(_mm_mul_ps(lSmoothStepSum, lSmoothStepSum), _mm_fnmadd_ps(_mm_set1_ps(2.0f), lSmoothStepSum, _mm_set1_ps(3.0f)));
        
        lAmplitude = _mm_mul_ps( lAmplitude, _mm_fmadd_ps( lAltitudeErosion, _mm_sub_ps( _mm_mul_ps( lCurrentGain, lSmoothStepSum), lCurrentGain), lCurrentGain ));
        lCurrentGain     = _mm_set1_ps( lfGain );

        __m128 lRidgeErosionDerivativeSumDot = _mm_mul_ps(lRidgeErosionDerivativeSumX, lRidgeErosionDerivativeSumX);
        lRidgeErosionDerivativeSumDot = _mm_fmadd_ps(lRidgeErosionDerivativeSumY, lRidgeErosionDerivativeSumY, lRidgeErosionDerivativeSumDot);
        lRidgeErosionDerivativeSumDot = _mm_fmadd_ps(lRidgeErosionDerivativeSumZ, lRidgeErosionDerivativeSumZ, lRidgeErosionDerivativeSumDot);

        lDampedAmplitude = _mm_mul_ps(lAmplitude, _mm_sub_ps(_mm_set1_ps(1.0f), _mm_div_ps(lRidgeErosion, _mm_add_ps(_mm_set1_ps(1.0f), lRidgeErosionDerivativeSumDot))));
    }

    __m128 lfResult = lfSharpness == 0.0f ? _mm_fmadd_ps(lSum, _mm_set1_ps(0.5f), _mm_set1_ps(0.5f)) : lSum;
    lfResult = Remap(lfResult, _mm_set1_ps(lfRemapFromMin), _mm_set1_ps(lfRemapFromMax), _mm_set1_ps(lfRemapToMin), _mm_set1_ps(lfRemapToMax));
    lfResult = GetGain(lfResult, _mm_set1_ps(lfSlopeGain));
    lfResult = GetBias(lfResult, _mm_set1_ps(lfSlopeBias));

    _mm_storeu_ps( lpStore, lfResult );
#endif
}
#endif

//-----------------------------------------------------------------------------
//Use of octave noise to create the smooth noise function
//-----------------------------------------------------------------------------
STATIC_CONST float kafSmoothNoiseScalars[30] = {
    -0.81f, 0.81f, // 1
    -0.73f, 0.73f, // 2
    -0.66f, 0.67f, // 3
    -0.62f, 0.66f, // 4
    -0.61f, 0.63f, // 5
    -0.61f, 0.62f, // 6
    -0.60f, 0.61f, // 7
    -0.60f, 0.61f, // 8
    -0.60f, 0.61f, // 9
    -0.60f, 0.61f, // 10
    -0.60f, 0.61f, // 11
    -0.60f, 0.61f, // 12
    -0.60f, 0.61f, // 13
    -0.60f, 0.61f, // 14
    -0.60f, 0.61f  // 15
};
float 
SmoothNoise(
    vec3 lPosition, 
    vec3 lSeedOffset, 
    float lfFeatureScale, 
    int liNumOctaves)
{
    //transform / scale position
    lPosition += lSeedOffset;
    lPosition *= lfFeatureScale;

    //get noise
    float lfNoise = OctaveNoise( lPosition, liNumOctaves );

    //scale between 0 and 1
    int liNoiseScalarIdx = (liNumOctaves-1)*2;
    float lfAverageMin = kafSmoothNoiseScalars[liNoiseScalarIdx];
    float lfAverageMax = kafSmoothNoiseScalars[liNoiseScalarIdx+1];
    lfNoise = (lfNoise - lfAverageMin) / ( lfAverageMax - lfAverageMin );

    return lfNoise;
}

//-----------------------------------------------------------------------------
//Utilities for voronoi noise
//-----------------------------------------------------------------------------
STATIC_CONST vec3 kaFaceNormal[ 6 ] =
{
    vec3(  0.0f,  0.0f, -1.0f ),
    vec3(  0.0f,  0.0f,  1.0f ),
    vec3(  1.0f,  0.0f,  0.0f ),
    vec3( -1.0f,  0.0f,  0.0f ),
    vec3(  0.0f, -1.0f,  0.0f ),
    vec3(  0.0f,  1.0f,  0.0f )
};

STATIC_CONST vec3 kaFaceRight[ 6 ] =
{
    vec3(  0.0f,  1.0f,  0.0f ),
    vec3(  0.0f, -1.0f,  0.0f ),
    vec3(  0.0f,  0.0f,  1.0f ),
    vec3(  0.0f,  0.0f, -1.0f ),
    vec3(  1.0f,  0.0f,  0.0f ),
    vec3( -1.0f,  0.0f,  0.0f )
};

STATIC_CONST vec3 kaFaceUp[ 6 ] =
{
    vec3(  1.0f,  0.0f,  0.0f ),
    vec3( -1.0f,  0.0f,  0.0f ),
    vec3(  0.0f,  1.0f,  0.0f ),
    vec3(  0.0f, -1.0f,  0.0f ),
    vec3(  0.0f,  0.0f, -1.0f ),
    vec3(  0.0f,  0.0f,  1.0f )
};
TKINLINE int
GetFaceIndex(
    vec3 lNormal )
{  
    vec3 lDiff;
    lDiff = lNormal - kaFaceNormal[0];
    if (dot(lDiff, lDiff) < 0.000001f)  return 0;
    lDiff = lNormal - kaFaceNormal[1];
    if (dot(lDiff, lDiff) < 0.000001f)  return 1;
    lDiff = lNormal - kaFaceNormal[2];
    if (dot(lDiff, lDiff) < 0.000001f)  return 2;
    lDiff = lNormal - kaFaceNormal[3];   
    if (dot(lDiff, lDiff) < 0.000001f)  return 3;
    lDiff = lNormal - kaFaceNormal[4];
    if (dot(lDiff, lDiff) < 0.000001f)  return 4;
    lDiff = lNormal - kaFaceNormal[5];
    if (dot(lDiff, lDiff) < 0.000001f)  return 5;
    return 0;
}



//-----------------------------------------------------------------------------
//Get voronoi info from sphere position
//-----------------------------------------------------------------------------
void
VoronoiDistanceOnCube(
    vec3                    lSpherePosition,
    int                     liSeed,
    float                   lfRadius,
    float                   lfScaleFactor,
    OUTPARAM(uint64)        liID,
    OUTPARAM(float)         lfCenterDistance,
    OUTPARAM(vec3)          lCenterPosition,
    OUTPARAM(vec3)          lFacePosition,
    OUTPARAM(vec3)          lNormal
    #ifdef TK_TGEN_VORONOI_DEBUG
    ,OUTPARAM(sVoronoiDebugOutput)  lDebug
    #endif
    )
{
    #ifdef TK_TGEN_VORONOI_DEBUG
    lDebug.mSpherePosition = lSpherePosition;
    lDebug.mNormPosition = vec3(0.0f);
    lDebug.mScaledPosition = vec3(0.0f);
    #endif

    //this does exactly the same as the above, but specifically designed to minimize precision loss to avoid gpu vs cpu deviation
    vec3 lCubePosition;
    vec3 lAbsVector = abs(lSpherePosition);
    if (lAbsVector.y >= lAbsVector.x && lAbsVector.y >= lAbsVector.z )
    {
        //y is dominant vector, 
        lNormal = vec3(0,sign(lSpherePosition.y),0);
        lCubePosition = lfScaleFactor * (lSpherePosition / abs(lSpherePosition.y));
        lCubePosition.y = lNormal.y * lfScaleFactor;
    }
    else if( lAbsVector.x >= lAbsVector.y && lAbsVector.x >= lAbsVector.z )
    {
        lNormal = vec3(sign(lSpherePosition.x),0,0);
        lCubePosition = lfScaleFactor * (lSpherePosition / abs(lSpherePosition.x));
        lCubePosition.x = lNormal.x * lfScaleFactor;
    }
    else
    {
        lNormal = vec3(0,0,sign(lSpherePosition.z));
        lCubePosition = lfScaleFactor * (lSpherePosition / abs(lSpherePosition.z));
        lCubePosition.z = lNormal.z * lfScaleFactor;
    }

    //force cube position into fixed precision
    #ifdef TK_TGEN_VORONOI_IDENICAL
    lCubePosition = ldexp(round(ldexp(lCubePosition,8)),-8);
    #else
    //lCubePosition = ldexp(round(ldexp(lCubePosition,16)),-16);
    lCubePosition.x = ClampPrecision(lCubePosition.x,16);
    lCubePosition.y = ClampPrecision(lCubePosition.y,16);
    lCubePosition.z = ClampPrecision(lCubePosition.z,16);
    #endif

    #ifdef TK_TGEN_VORONOI_DEBUG
    lDebug.mToCubePosition = lCubePosition;
    #endif

    int liStartFaceIndex = GetFaceIndex( lNormal );

    //calculate grid position
    float lfx = dot( lCubePosition, kaFaceRight[ liStartFaceIndex ] );
    float lfy = dot( lCubePosition, kaFaceUp[ liStartFaceIndex ] );

    float lffloorx = min( floor( lfx ), ceil( lfx ) );
    float lffloory = min( floor( lfy ), ceil( lfy ) );

#ifdef TK_TGEN_VORONOI_DEBUG
    vec3 lFractPosition = ( lfx - lffloorx ) * kaFaceRight[ liStartFaceIndex ] + ( lfy - lffloory ) * kaFaceUp[ liStartFaceIndex ];
#endif
    vec3 lFloorPosition = lffloorx * kaFaceRight[ liStartFaceIndex ] + lffloory * kaFaceUp[ liStartFaceIndex ] + kaFaceNormal[ liStartFaceIndex ] * lfScaleFactor;

    vec3 lReturnNormal  = lNormal;

    //make an especially random seed from the seed
    uint luRandSeed = HashMixUInt32(liSeed);

    #ifdef TK_TGEN_VORONOI_DEBUG
    lDebug.mFracPosition = lFractPosition;
    lDebug.mFloorPosition = lFloorPosition;
    lDebug.muSeed = luRandSeed;
    #endif

    float   lfMinDistance = 160000.0f;

    // Check surrounding points on cube face
    for( int liX = -1; liX <= 1; liX++ )
    {
        for( int liY = -1; liY <= 1; liY++ )
        {
            vec3 lPositionIndex   = kaFaceRight[ liStartFaceIndex ] * (float)liX + kaFaceUp[ liStartFaceIndex ] * (float)liY;
            int liFaceIndex = liStartFaceIndex;

            // Check whether x and y values are outside this cube face and need to wrap onto neighbouring face
            float lfFaceX = dot( lFloorPosition + kaFaceRight[ liFaceIndex ] * (float)liX, kaFaceRight[ liFaceIndex ] );
            float lfFaceY = dot( lFloorPosition + kaFaceUp[ liFaceIndex ] * (float)liY, kaFaceUp[ liFaceIndex ] );

            #ifdef TK_TGEN_VORONOI_DEBUG
            lDebug.mPrePosIdx[(liX + 1) + (liY+1)*3] = lPositionIndex;
            lDebug.mFaceX[(liX + 1) + (liY+1)*3] = lfFaceX;
            lDebug.mFaceY[(liX + 1) + (liY+1)*3] = lfFaceY;
            #endif

            if( (int)lfFaceX == (int)lfScaleFactor )
            {
                // Right on cube edge, position is valid on next face
                liFaceIndex = GetFaceIndex( kaFaceRight[ liFaceIndex ] );
            }
            else if( (int)lfFaceY == (int)lfScaleFactor )
            {
                // Right on cube edge, position is valid on next face
                liFaceIndex = GetFaceIndex( kaFaceUp[ liFaceIndex ] );
            }
            else if( (int)lfFaceX < (int)-lfScaleFactor || (int)lfFaceX > (int)lfScaleFactor )
            {
                TKASSERT( abs( lfFaceX ) - abs( lfScaleFactor ) <= 1.0f );
                // Position wraps onto next face, one position back along the current face normal
                lPositionIndex   = (float)liY * kaFaceUp[ liFaceIndex ];
                lPositionIndex  -= kaFaceNormal[ liFaceIndex ];
                liFaceIndex      = GetFaceIndex( -(float)liX * kaFaceRight[ liFaceIndex ] );
            }
            else if( (int)lfFaceY < (int)-lfScaleFactor || (int)lfFaceY > (int)lfScaleFactor ) 
            {
                TKASSERT( abs( lfFaceY ) - abs( lfScaleFactor ) <= 1.0f );
                // Position wraps onto next face, one position back along the current face normal
                lPositionIndex   = (float)liX * kaFaceRight[ liFaceIndex ];
                lPositionIndex  -= kaFaceNormal[ liFaceIndex ];
                liFaceIndex      = GetFaceIndex( -(float)liY * kaFaceUp[ liFaceIndex ] );
            }


            // Get an offset so we have a random position within our grid cell
            vec3 lGridPosition = vec3( (float)((int)lFloorPosition.x + (int)lPositionIndex.x), (float)((int)lFloorPosition.y + (int)lPositionIndex.y), (float)((int)lFloorPosition.z + (int)lPositionIndex.z) );
            vec3 lRandomOffset = VoronoiRandom3f( lGridPosition, luRandSeed );

            #ifdef TK_TGEN_VORONOI_DEBUG
            lDebug.mPostPosIdx[(liX + 1) + (liY+1)*3] = lPositionIndex;
            lDebug.mFaceIdx[(liX + 1) + (liY+1)*3] = liFaceIndex;
            lDebug.mRandInputs[(liX + 1) + (liY+1)*3] = lRandInput;
            lDebug.mRandOffsets[(liX + 1) + (liY+1)*3] = lRandomOffset;
            #endif

            // Only shift in 2d across the cube face
            lRandomOffset *= kaFaceRight[ liFaceIndex ] + kaFaceUp[ liFaceIndex ];
            
            // Transform from cube space back onto sphere to check distance
            vec3 lActualCenter = lGridPosition + lRandomOffset;
            lActualCenter /= lfScaleFactor;
            lActualCenter  = ProjectCubeToSphere( lActualCenter, kaFaceNormal[ liFaceIndex ] );

            lActualCenter /= length( lActualCenter );
            lActualCenter *= lfRadius;

            float lfDistanceSquared = lengthSquared( lActualCenter - lSpherePosition );

            // Test for and store nearest position
            if(lfDistanceSquared < (lfMinDistance*lfMinDistance))
            {
                lfMinDistance    = sqrt(lfDistanceSquared);
                lFacePosition    = lGridPosition + lRandomOffset;
                lCenterPosition  = lActualCenter;
                lReturnNormal    = kaFaceNormal[ liFaceIndex ];
            }
            #ifdef TK_TGEN_VORONOI_DEBUG
            lDebug.mfDistances[(liX + 1) + (liY+1)*3] = lfDistance;
            lDebug.mCenterPositions[(liX + 1) + (liY+1)*3] = lFacePosition;
            #endif
        }
    }

    // Get a unique seed for this center position
    liID             = VoronoiID( liSeed, lFacePosition );

    #ifdef TK_TGEN_VORONOI_IDENICAL
    // Force the resulting center position into fixed precision
    lCenterPosition = ldexp(round(ldexp(lCenterPosition,4)),-4);
    #endif

    // Recalculate final distance on the sphere
    lfCenterDistance = length( lCenterPosition - lSpherePosition );

    lNormal = lReturnNormal;

    #ifdef TK_TGEN_VORONOI_IDENICAL
    // Force final distance into fixed precision
    lfCenterDistance = round(lfCenterDistance);
    #endif

    #ifdef TK_TGEN_VORONOI_DEBUG
    lDebug.mOutId = liID;
    lDebug.mOutCentre = lCenterPosition;
    lDebug.mfOutCentreDistance = lfCenterDistance;
    #endif
}

//-----------------------------------------------------------------------------
//Turbulence noise
//-----------------------------------------------------------------------------
STATIC_CONST vec3 kTurbulenceXYZ0Add = vec3(12414.0f / 65536.0f, 65124.0f / 65536.0f, 31337.0f / 65536.0f);
STATIC_CONST vec3 kTurbulenceXYZ1Add = vec3(26519.0f / 65536.0f, 18128.0f / 65536.0f, 60493.0f / 65536.0f);
STATIC_CONST vec3 kTurbulenceXYZ2Add = vec3(53820.0f / 65536.0f, 11213.0f / 65536.0f, 44845.0f / 65536.0f);
vec3 
TurbulencePositions( 
    vec3 lPos,
    vec3 lFrequency, 
    vec3 lPower, 
    int liNumOctaves )
{
    // Get the values from the three Simplex noise directions and
    // add each value to each coordinate of the input coord value.  There are also
    // some offsets added to the coordinates of the input values.  This prevents
    // the distortion modules from returning zero if the (x, y, z) coordinates,
    // when multiplied by the frequency, are near an integer boundary.  This is
    // due to a property of gradient coherent noise, which returns zero at
    // integer boundaries.
    vec3 lX = (lPos + kTurbulenceXYZ0Add) * lFrequency.x;
    vec3 lY = (lPos + kTurbulenceXYZ1Add) * lFrequency.y;
    vec3 lZ = (lPos + kTurbulenceXYZ2Add) * lFrequency.z;

    vec3 lRes = lPos;
    lRes.x += OctaveNoise( lX, liNumOctaves ) * lPower.x;
    lRes.y += OctaveNoise( lY, liNumOctaves ) * lPower.y;
    lRes.z += OctaveNoise( lZ, liNumOctaves ) * lPower.z;

    return lRes;
}

//-----------------------------------------------------------------------------
//Utilities to mimic some of the more advanced noise combinations from basic
//noise helper and basic noise 2D
//-----------------------------------------------------------------------------



TKFORCEINLINE
float
RegionGain(
    float x,
    float k)
{
    x = saturate( x );
    float a = 0.5f*pow(2.0f*((x < 0.5f) ? x : 1.0f - x), k);
    return (x < 0.5f) ? a : 1.0f - a;
}

TKINLINE
float
GenerateRegionNoise(     
    vec3        lPosition,
    float       lfRegionSize,
    float       lfRegionScale,
    float       lfRegionRatio,
    float       lfRegionGain,
    vec3        lSeedOffset,
    int         lbIsLegacy )
{

    //assume noise of 1
    float lfRegionNoise = 1.0f;

    //check valid scale and ratio provided
    if ( lfRegionScale > 1.0f &&
         lfRegionRatio > 0.0f &&
         lfRegionRatio < 1.0f )
    {
        //sample noise
        float lfFeatureScale = 1.0f / (lfRegionSize * lfRegionScale);
        lfRegionNoise = SmoothNoise(lPosition,lSeedOffset,lfFeatureScale,3);

        //scale it
        lfRegionNoise = ScaleNoise(lfRegionNoise,sqrt( lfRegionRatio ) );

        //if legacy, put on smooth curve from 0-0.5
        if ( (bool) lbIsLegacy )
        {
            lfRegionNoise = smoothstep5( 0.0f, 0.5f, lfRegionNoise );
        }
        else
        {
            lfRegionNoise = saturate(lfRegionNoise);
            lfRegionNoise = RegionGain( lfRegionNoise, lfRegionGain );
        }
    }

    //return result
    return lfRegionNoise;
}


float
GenerateNoise2D(
    vec3        lPosition,
    float       lfRegionSize,
    float       lfRegionScale,
    float       lfRegionRatio,
    float       lfRegionGain,
    vec3        lSeedOffset,
    int         lbIsLegacy )
{
    //region scale is typically > 1, so this generates a lower frequency noise based off pos/(size*scale), then scales it by region ratio to get the result
    float lfRegionNoise = GenerateRegionNoise(lPosition,lfRegionSize,lfRegionScale,lfRegionRatio, lfRegionGain, lSeedOffset,lbIsLegacy);

    //introduce higher detail features using just the region size to control frequency, giving pos/size
    float lfFeatureNoise = SmoothNoise( lPosition, lSeedOffset, 1.f/lfRegionSize, 5 );

    //multiply the 2 together to get resulting noise
    return lfFeatureNoise * lfRegionNoise;
}

#include "Custom/SharedEnd.inl"
#endif
