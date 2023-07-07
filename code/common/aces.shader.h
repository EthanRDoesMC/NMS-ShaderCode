////////////////////////////////////////////////////////////////////////////////
///
///     @file       ACES.shader.h
///     @author     User
///     @date       
///
///     @brief      ACES color transform utilities
///
////////////////////////////////////////////////////////////////////////////////

// # License Terms for Academy Color Encoding System Components #
// 
// Academy Color Encoding System (ACES) software and tools are provided by the
// Academy under the following terms and conditions: A worldwide, royalty-free,
// non-exclusive right to copy, modify, create derivatives, and use, in source and
// binary forms, is hereby granted, subject to acceptance of this license.
// 
// Copyright © 2015 Academy of Motion Picture Arts and Sciences (A.M.P.A.S.).
// Portions contributed by others as indicated. All rights reserved.
// 
// Performance of any of the aforementioned acts indicates acceptance to be bound
// by the following terms and conditions:
// 
// * Copies of source code, in whole or in part, must retain the above copyright
// notice, this list of conditions and the Disclaimer of Warranty.
// 
// * Use in binary form must retain the above copyright notice, this list of
// conditions and the Disclaimer of Warranty in the documentation and/or other
// materials provided with the distribution.
// 
// * Nothing in this license shall be deemed to grant any rights to trademarks,
// copyrights, patents, trade secrets or any other intellectual property of
// A.M.P.A.S. or any contributors, except as expressly stated herein.
// 
// * Neither the name "A.M.P.A.S." nor the name of any other contributors to this
// software may be used to endorse or promote products derivative of or based on
// this software without express prior written permission of A.M.P.A.S. or the
// contributors, as appropriate.
// 
// This license shall be construed pursuant to the laws of the State of
// California, and any disputes related thereto shall be subject to the
// jurisdiction of the courts therein.
// 
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
// NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT SHALL A.M.P.A.S., OR ANY
// CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, RESITUTIONARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY SPECIFICALLY
// DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER RELATED TO PATENT OR
// OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY COLOR ENCODING SYSTEM, OR
// APPLICATIONS THEREOF, HELD BY PARTIES OTHER THAN A.M.P.A.S.,WHETHER DISCLOSED OR
// UNDISCLOSED.

#ifndef D_ACES_H
#define D_ACES_H


STATIC_CONST vec3 D60_White_xyY = vec3( 0.32168,  0.33767, 1.0 );
STATIC_CONST vec3 D65_White_xyY = vec3( 0.31270,  0.32900, 1.0 );

#define xyY_TO_XYZ( IN ) vec3( IN.x / IN.y, 1.0, ( 1.0 - IN.x - IN.y ) / IN.y ) * IN.z

// these are matrices using the AP0 and AP1 primaries but the D65 white point
// ACES usually uses the D60 white point but I don't like the added complexity of
// trying to do white point transforms

STATIC_CONST mat3 AP0_D65_TO_XYZ = mat3( 
    0.9503548,  0.0000000,  0.0001011,
    0.3431729,  0.7346964, -0.0778693,
    0.0000000,  0.0000000,  1.0890578
);

STATIC_CONST mat3 XYZ_TO_AP0_D65 = mat3( 
    1.0522386,  0.0000000, -0.0000977,
   -0.4914952,  1.3611064,  0.0973668,
    0.0000000,  0.0000000,  0.9182250
);

STATIC_CONST mat3 AP1_D65_TO_XYZ = mat3( 
     0.6475072,  0.1343791,  0.1685696,
     0.2660864,  0.6759678,  0.0579458,
    -0.0054489,  0.0040721,  1.0904345
);

STATIC_CONST mat3 XYZ_TO_AP1_D65 = mat3( 
     1.6789046, -0.3323010, -0.2418823,
    -0.6618112,  1.6108246,  0.0167096,
     0.0108609, -0.0076759,  0.9157945
);

STATIC_CONST mat3 sRGB_TO_XYZ = mat3( 
     0.4124564,  0.3575761,  0.1804375,
     0.2126729,  0.7151522,  0.0721750,
     0.0193339,  0.1191920,  0.9503041
);

STATIC_CONST mat3 XYZ_TO_sRGB = mat3( 
     3.2404542, -1.5371385, -0.4985314,
    -0.9692660,  1.8760108,  0.0415560,
     0.0556434, -0.2040259,  1.0572252
);

STATIC_CONST mat3 P3D65_TO_XYZ = mat3( 
     0.5159528, 0.2684087, 0.1660945,
     0.2428013, 0.6988754, 0.0583232,
     0.0000000, 0.0455788, 1.0434789
);

STATIC_CONST mat3 XYZ_TO_P3D65 = mat3( 
     2.3535417, -0.8826821, -0.3252862,
    -0.8206522,  1.7438858,  0.0331551,
     0.0358458, -0.0761724,  0.9568845
);

STATIC_CONST mat3 AP0_TO_AP1 = MUL( AP0_D65_TO_XYZ, XYZ_TO_AP1_D65 );
STATIC_CONST mat3 AP1_TO_AP0 = MUL( AP1_D65_TO_XYZ, XYZ_TO_AP0_D65 );

STATIC_CONST mat3 sRGB_TO_P3D65 = MUL( sRGB_TO_XYZ, XYZ_TO_P3D65 );
STATIC_CONST mat3 P3D65_TO_sRGB = MUL( P3D65_TO_XYZ, XYZ_TO_sRGB );

STATIC_CONST mat3 P3D65_TO_AP1 = MUL( P3D65_TO_XYZ, XYZ_TO_AP1_D65 );
STATIC_CONST mat3 AP1_TO_sRGB  = MUL( AP1_D65_TO_XYZ, XYZ_TO_sRGB );


#define HALF_MIN 6.10e-5

//TF_BEGIN
#define P3D65_TO_Y( in ) in.x * 0.2428013 + 0.6988754 * in.y + 0.0583232 * in.z
//TF_END

//-----------------------------------------------------------------------------
///
///     HSVToRGB
///
///     @brief      http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
///
//-----------------------------------------------------------------------------

/*
vec3 rrt_sweeteners( vec3 in )
{
    float aces[3] = in;
    
    // --- Glow module --- //
    float saturation = rgb_2_saturation( aces);
    float ycIn = rgb_2_yc( aces);
    float s = sigmoid_shaper( (saturation - 0.4) / 0.2);
    float addedGlow = 1. + glow_fwd( ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);

    aces = mult_f_f3( addedGlow, aces);

    // --- Red modifier --- //
    float hue = rgb_2_hue( aces);
    float centeredHue = center_hue( hue, RRT_RED_HUE);
    float hueWeight = cubic_basis_shaper( centeredHue, RRT_RED_WIDTH);

    aces[0] = aces[0] + hueWeight * saturation * (RRT_RED_PIVOT - aces[0]) * (1. - RRT_RED_SCALE);

    // --- ACES to RGB rendering space --- //
    aces = clamp_f3( aces, 0., HALF_POS_INF);
    float rgbPre[3] = mult_f3_f44( aces, AP0_2_AP1_MAT);
    rgbPre = clamp_f3( rgbPre, 0., HALF_MAX);
    
    // --- Global desaturation --- //
    rgbPre = mult_f3_f33( rgbPre, RRT_SAT_MAT);
    return rgbPre;
}
*/

struct TsPoint
{
    //float x;        // ACES
    //float y;        // luminance
    float slope;    // 
    float logx;     // log10 of aces
    float logy;     // log10 of luminance
};

struct TsParams
{
    TsPoint Min;
    TsPoint Mid;
    TsPoint Max;
    float coefsLow[6];
    float coefsHigh[6];    
};

#if! defined(D_PLATFORM_ORBIS) && !defined(D_PLATFORM_METAL)
float log10( float x ) { return log2(x) / log2(10.0); }
#endif
float pow10( float x ) { return pow(10, x); }

vec3 mult_f3_f33( vec3 f, mat3 M ) { return MUL(M, f); }

STATIC_CONST float MIN_STOP_SDR = -6.5;
STATIC_CONST float MAX_STOP_SDR = 6.5;

STATIC_CONST float MIN_STOP_RRT = -15.;
STATIC_CONST float MAX_STOP_RRT = 18.;

STATIC_CONST float MIN_LUM_SDR = 0.02;
STATIC_CONST float MAX_LUM_SDR = 48.0;

STATIC_CONST float MIN_LUM_RRT = 0.0001;
STATIC_CONST float MAX_LUM_RRT = 10000.0;

STATIC_CONST float TINY = 1e-10;

#ifdef D_PLATFORM_METAL
float interpolate1D( thread const float table[2][2], float p )
#else
float interpolate1D( float table[2][2], float p )
#endif
{
    if( p <  table[0][0] ) return table[0][1];
    if( p >= table[1][0] ) return table[1][1];

    p = ( p - table[0][0] ) / ( table[1][0] - table[0][0] );

    return mix(table[0][1], table[1][1], p);
}

float lookup_ACESmin( float minLum )
{
    const float minTable[2][2] = { { log10(MIN_LUM_RRT), MIN_STOP_RRT }, 
                                   { log10(MIN_LUM_SDR), MIN_STOP_SDR } };

    return 0.18*exp2(interpolate1D( minTable, log10( minLum)));
}

float lookup_ACESmax( float maxLum )
{
    const float maxTable[2][2] = { { log10(MAX_LUM_SDR), MAX_STOP_SDR }, 
                                   { log10(MAX_LUM_RRT), MAX_STOP_RRT } };

    return 0.18*exp2(interpolate1D( maxTable, log10( maxLum)));
}

void init_coefsLow(
    TsPoint TsPointLow,
    TsPoint TsPointMid,
    float pctLow,
    out float coefsLow[6]
)
{
    float knotIncLow = ((TsPointMid.logx) - (TsPointLow.logx)) / 3.;
    // float halfKnotInc = ((TsPointMid.logx) - (TsPointLow.logx)) / 6.;

    // Determine two lowest coefficients (straddling minPt)
    coefsLow[0] = (TsPointLow.slope * ((TsPointLow.logx)-0.5*knotIncLow)) + ( (TsPointLow.logy) - TsPointLow.slope * (TsPointLow.logx));
    coefsLow[1] = (TsPointLow.slope * ((TsPointLow.logx)+0.5*knotIncLow)) + ( (TsPointLow.logy) - TsPointLow.slope * (TsPointLow.logx));
    // NOTE: if slope=0, then the above becomes just 
        // coefsLow[0] = (TsPointLow.logy);
        // coefsLow[1] = (TsPointLow.logy);
    // leaving it as a variable for now in case we decide we need non-zero slope extensions

    // Determine two highest coefficients (straddling midPt)
    coefsLow[3] = (TsPointMid.slope * ((TsPointMid.logx)-0.5*knotIncLow)) + ( (TsPointMid.logy) - TsPointMid.slope * (TsPointMid.logx));
    coefsLow[4] = (TsPointMid.slope * ((TsPointMid.logx)+0.5*knotIncLow)) + ( (TsPointMid.logy) - TsPointMid.slope * (TsPointMid.logx));
    
    // Middle coefficient (which defines the "sharpness of the bend") is linearly interpolated
    // float bendsLow[2][2] = { {MIN_STOP_RRT, 0.18}, 
    //                          {MIN_STOP_SDR, 0.35} };
    // float pctLow = interpolate1D( bendsLow, log2(pow10(TsPointLow.logx)/0.18));
    coefsLow[2] = (TsPointLow.logy) + pctLow*((TsPointMid.logy)-(TsPointLow.logy));

    coefsLow[5] = coefsLow[4];
} 

void init_coefsHigh( 
    TsPoint TsPointMid, 
    TsPoint TsPointMax,
    out float coefsHigh[6]
)
{
    float knotIncHigh = ((TsPointMax.logx) - (TsPointMid.logx)) / 3.;
    // float halfKnotInc = ((TsPointMax.logx) - (TsPointMid.logx)) / 6.;

    // Determine two lowest coefficients (straddling midPt)
    coefsHigh[0] = (TsPointMid.slope * ((TsPointMid.logx)-0.5*knotIncHigh)) + ( (TsPointMid.logy) - TsPointMid.slope * (TsPointMid.logx));
    coefsHigh[1] = (TsPointMid.slope * ((TsPointMid.logx)+0.5*knotIncHigh)) + ( (TsPointMid.logy) - TsPointMid.slope * (TsPointMid.logx));

    // Determine two highest coefficients (straddling maxPt)
    coefsHigh[3] = (TsPointMax.slope * ((TsPointMax.logx)-0.5*knotIncHigh)) + ( (TsPointMax.logy) - TsPointMax.slope * (TsPointMax.logx));
    coefsHigh[4] = (TsPointMax.slope * ((TsPointMax.logx)+0.5*knotIncHigh)) + ( (TsPointMax.logy) - TsPointMax.slope * (TsPointMax.logx));
    // NOTE: if slope=0, then the above becomes just
        // coefsHigh[0] = (TsPointHigh.logy);
        // coefsHigh[1] = (TsPointHigh.logy);
    // leaving it as a variable for now in case we decide we need non-zero slope extensions
    
    // Middle coefficient (which defines the "sharpness of the bend") is linearly interpolated
    float bendsHigh[2][2] = { {MAX_STOP_SDR, 0.89}, 
                              {MAX_STOP_RRT, 0.90} };
    float pctHigh = interpolate1D( bendsHigh, log2(pow10(TsPointMax.logx)/0.18));
    coefsHigh[2] = (TsPointMid.logy) + pctHigh*((TsPointMax.logy)-(TsPointMid.logy));

    coefsHigh[5] = coefsHigh[4];
}

float shift( float inVal, float expShift)
{
    return exp2((log2(inVal)-expShift));
}

TsParams init_TsParams(
    float minLum,
    float maxLum,
    float expShift,
    float midSlope,
    float maxSlope,
    float blackBend
)
{
    TsParams P;

    TsPoint MIN_PT;
    MIN_PT.logx = log10( lookup_ACESmin(minLum) );
    MIN_PT.logy = log10( minLum );
    MIN_PT.slope = 0.0;

    TsPoint MID_PT;
    MID_PT.logx = log10( 0.18 );
    MID_PT.logy = log10( 4.8 );
    MID_PT.slope = midSlope;

    TsPoint MAX_PT;
    MAX_PT.logx = log10( lookup_ACESmax(maxLum) );
    MAX_PT.logy = log10( maxLum );
    MAX_PT.slope = maxSlope;

    init_coefsLow( MIN_PT, MID_PT, blackBend, P.coefsLow );
    init_coefsHigh( MID_PT, MAX_PT, P.coefsHigh );

    MIN_PT.logx = log10( shift(lookup_ACESmin(minLum),expShift) );
    MID_PT.logx = log10( shift(0.18,expShift) );
    MAX_PT.logx = log10( shift(lookup_ACESmax(maxLum),expShift) );

    P.Min = MIN_PT;
    P.Mid = MID_PT;
    P.Max = MAX_PT;
         
    return P;
}

#if defined ( D_ACES_PARAMS )

float ssts
( 
    float x,
    CustomPerMeshUniforms lMeshUniforms
)
{
    const int N_KNOTS_LOW = 4;
    const int N_KNOTS_HIGH = 4;

    const mat3 M1 = mat3(
        0.5, -1.0, 0.5 ,
       -1.0,  1.0, 0.5 ,
        0.5,  0.0, 0.0 
    );

    // Check for negatives or zero before taking the log. If negative or zero,
    // set to HALF_MIN.
    float logx = log10( max(x, HALF_MIN )); 

    float logy;

    vec4 cMin = ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, 0 );
    vec4 cMid = ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, 1 );
    vec4 cMax = ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, 2 );

    if ( logx <= (cMin.y) ) { 

        logy = logx * cMin.x + ( (cMin.z) - cMin.x * (cMin.y) );

    } else if (( logx > (cMin.y) ) && ( logx < (cMid.y) )) {

        float knot_coord = (N_KNOTS_LOW-1) * (logx-(cMin.y))/((cMid.y)-(cMin.y));
        int j = int( knot_coord );
        float t = knot_coord - j;

        vec3 cf = vec3( 
            ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, j + 3 ).x, 
            ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, j + 4 ).x, 
            ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, j + 5 ).x );

        vec3 monomials = vec3( t * t, t, 1.0 );
        logy = dot( monomials, mult_f3_f33( cf, M1));

    } else if (( logx >= (cMid.y) ) && ( logx < (cMax.y) )) {

        float knot_coord = (N_KNOTS_HIGH-1) * (logx-(cMid.y))/((cMax.y)-(cMid.y));
        int j = int( knot_coord );
        float t = knot_coord - j;

        vec3 cf = vec3( 
            ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, j + 3 ).y, 
            ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, j + 4 ).y, 
            ARRAY_LOOKUP_FS( lMeshUniforms, mpCustomPerMesh, gAcesCurveParams, j + 5 ).y ); 

        vec3 monomials = vec3( t * t, t, 1.0 );
        logy = dot( monomials, mult_f3_f33( cf, M1));

    } else { //if ( logIn >= (cMax.y) ) { 

        logy = logx * cMax.x + ( (cMax.z) - cMax.x * (cMax.y) );

    }

    return pow10(logy);

}


vec3 ssts_f3
( 
    vec3 x,
    CustomPerMeshUniforms lMeshUniforms
)
{
    vec3 outVec;
    outVec.x = ssts( x.x, lMeshUniforms );
    outVec.y = ssts( x.y, lMeshUniforms );
    outVec.z = ssts( x.z, lMeshUniforms );

    return outVec;
}

#endif

#endif
