////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonTonemapPowerCurves.h
///     @author     strgiu
///     @date       29/01/2023
///
///     @brief      Implementation of John Hable's power curves based tonemapper
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONTONEMAPPOWERCURVES_H
#define D_COMMONTONEMAPPOWERCURVES_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"

//----------------------------------------------------------------------------- 
//      Global Data

// NOTE(sal): Implements the "Power Curves Filmic Tonemapper" described by
// John Hable in http://filmicworlds.com/blog/filmic-tonemapping-with-piecewise-power-curves/

STATIC_CONST float kfTonemapPowerCurvers_X0     = 0.150;
STATIC_CONST float kfTonemapPowerCurvers_Y0     = 0.075;
STATIC_CONST float kfTonemapPowerCurvers_X1     = 0.400;
STATIC_CONST float kfTonemapPowerCurvers_Y1     = 0.150;
STATIC_CONST float kfTonemapPowerCurvers_OVS_X  = 0.500;
STATIC_CONST float kfTonemapPowerCurvers_OVS_Y  = 0.100;

//-----------------------------------------------------------------------------
//    Functions
float
toe_segment(
    float x,
    float ln_a,
    float b )
{
    return  exp( ln_a + b * log( x ) );
}

float
toe_segment_inv(
    float y,
    float ln_a,
    float b )
{
    return exp( ( log( y ) - ln_a ) / b );
}

float
shoulder_segment(
    float x,
    float ln_a,
    float b,
    float x_n,
    float y_n )
{
    return -exp( ln_a + b * log( x_n - x ) ) + y_n;
}

float
shoulder_segment_inv(
    float y,
    float ln_a,
    float b,
    float x_n,
    float y_n )
{
    return -exp( ( log( y_n - y ) - ln_a ) / b ) + x_n;
}

float
line_segment(
    float x,
    float m,
    float q )
{
    return m * x + q;
}

float
line_segment_inv(
    float y,
    float m,
    float b )
{
    return ( y - b ) / m;
}

vec2
solve_A_B(
    float x0,
    float y0,
    float m )
{
    float b     = ( m * x0 ) / y0;
    float ln_a  = log( y0 ) - b * log( x0 );
    return vec2( ln_a, b );
}


vec2
as_slope_intercept(
    float x0,
    float x1,
    float y0,
    float y1 )
{
    float dx = ( x1 - x0 );
    float dy = ( y1 - y0 );

    float m  = dx == 0 ? 1.0 : dy / dx;
    float q  = y0 - x0 * m;

    return vec2( m, q );
}

float
curve(
    float x,
    float l_W,
    float l_x0,
    float l_y0,
    float l_x1,
    float l_y1,
    float l_overshoot_x,
    float l_overshoot_y )
{
    float l_x2 = l_W + l_overshoot_x;
    float l_y2 = 1.0 + l_overshoot_y;

    vec2  mq = as_slope_intercept( l_x0, l_x1, l_y0, l_y1 );
    float m  = mq.x;
    float q  = mq.y;

    if ( x < l_x0 )
    {
        vec2   AB_t = solve_A_B( l_x0, l_y0, m );
        float  toe  = toe_segment( x, AB_t.x, AB_t.y );
        return toe;
    }
    else
    if ( x > l_x1 )
    {
        vec2   AB_s     = solve_A_B( l_x2 - l_x1, l_y2 - l_y1, m );
        float  shoulder = shoulder_segment( x, AB_s.x, AB_s.y, l_x2, l_y2 );
        return shoulder;
    }
    else
    {
        float  linear_segment = line_segment( x, m, q );
        return linear_segment;
    }
}

vec3
curve(
    vec3  x,
    float l_W,
    float l_x0,
    float l_y0,
    float l_x1,
    float l_y1,
    float l_overshoot_x,
    float l_overshoot_y )
{
    return vec3( curve( x.x, l_W, l_x0, l_y0, l_x1, l_y1, l_overshoot_x, l_overshoot_y ),
                 curve( x.y, l_W, l_x0, l_y0, l_x1, l_y1, l_overshoot_x, l_overshoot_y ),
                 curve( x.z, l_W, l_x0, l_y0, l_x1, l_y1, l_overshoot_x, l_overshoot_y ) );
}

float
curve_inv(
    float y,
    float l_W,
    float l_x0,
    float l_y0,
    float l_x1,
    float l_y1,
    float l_overshoot_x,
    float l_overshoot_y )
{
    float l_x2 = l_W + l_overshoot_x;
    float l_y2 = 1.0 + l_overshoot_y;

    vec2  mq = as_slope_intercept( l_x0, l_x1, l_y0, l_y1 );
    float m  = mq.x;
    float q  = mq.y;

    if ( y < l_y0 )
    {
        vec2   AB_t = solve_A_B( l_x0, l_y0, m );
        float  toe  = toe_segment_inv( y, AB_t.x, AB_t.y );
        return toe;
    }
    else
    if ( y > l_y1 )
    {
        vec2   AB_s     = solve_A_B( l_x2 - l_x1, l_y2 - l_y1, m );
        float  shoulder = shoulder_segment_inv( y, AB_s.x, AB_s.y, l_x2, l_y2 );
        return shoulder;
    }
    else
    {
        float  linear_segment = line_segment_inv( y, m, q );
        return linear_segment;
    }
}

vec3
curve_inv(
    vec3  y,
    float l_W,
    float l_x0,
    float l_y0,
    float l_x1,
    float l_y1,
    float l_overshoot_x,
    float l_overshoot_y )
{
    return vec3( curve_inv( y.x, l_W, l_x0, l_y0, l_x1, l_y1, l_overshoot_x, l_overshoot_y ),
                 curve_inv( y.y, l_W, l_x0, l_y0, l_x1, l_y1, l_overshoot_x, l_overshoot_y ),
                 curve_inv( y.z, l_W, l_x0, l_y0, l_x1, l_y1, l_overshoot_x, l_overshoot_y ) );
}

float
TonemapPowerCurvesUnscaled(
    float lfX,
    float lfW,
    float lfX0,
    float lfY0,
    float lfX1,
    float lfY1,
    float lfOvershootX,
    float lfOvershootY )
{
    return curve( lfX, lfW, lfX0, lfY0, lfX1, lfY1, lfOvershootX, lfOvershootY );
}

vec3
TonemapPowerCurvesUnscaled(
    vec3  lvX,
    float lfW,
    float lfX0,
    float lfY0,
    float lfX1,
    float lfY1,
    float lfOvershootX,
    float lfOvershootY )
{
    return curve( lvX, lfW, lfX0, lfY0, lfX1, lfY1, lfOvershootX, lfOvershootY );
}

float
TonemapPowerCurves(
    float lfX,
    float lfW )
{
    float lfScale;
    lfScale = TonemapPowerCurvesUnscaled(
                lfW, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y );

    return  TonemapPowerCurvesUnscaled(
                lfX, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y ) / lfScale;
}

vec3
TonemapPowerCurves(
    vec3  lvX,
    float lfW )
{
    float lfScale;
    lfScale = TonemapPowerCurvesUnscaled(
                lfW, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y );

    return  TonemapPowerCurvesUnscaled(
                lvX, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y ) / float2vec3( lfScale );
}

float
TonemapPowerCurvesUnscaledInverse(
    float lfY,
    float lfW,
    float lfX0,
    float lfY0,
    float lfX1,
    float lfY1,
    float lfOvershootX,
    float lfOvershootY )
{
    return curve_inv( lfY, lfX0, lfY0, lfX1, lfY1, lfW, lfOvershootX, lfOvershootY );
}

vec3
TonemapPowerCurvesUnscaledInverse(
    vec3  lvY,
    float lfW,
    float lfX0,
    float lfY0,
    float lfX1,
    float lfY1,
    float lfOvershootX,
    float lfOvershootY )
{
    return curve_inv( lvY, lfX0, lfY0, lfX1, lfY1, lfW, lfOvershootX, lfOvershootY );
}

float
TonemapPowerCurvesInverse(
    float lfY,
    float lfW )
{
    float lfScale;
    lfScale = TonemapPowerCurvesUnscaled(
                lfW, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y );

    return TonemapPowerCurvesUnscaledInverse(
                lfY * lfScale, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y );
}

vec3
TonemapPowerCurvesInverse(
    vec3  lvY,
    float lfW )
{
    float lfScale;
    lfScale = TonemapPowerCurvesUnscaled(
                lfW, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y );

    return TonemapPowerCurvesUnscaledInverse(
                lvY * lfScale, lfW,
                kfTonemapPowerCurvers_X0,    kfTonemapPowerCurvers_Y0,
                kfTonemapPowerCurvers_X1,    kfTonemapPowerCurvers_Y1,
                kfTonemapPowerCurvers_OVS_X, kfTonemapPowerCurvers_OVS_Y );
}

#endif

