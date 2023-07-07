////////////////////////////////////////////////////////////////////////////////
///
///     @file       WaterFragment.h
///     @author     User
///     @date       
///
///     @brief      WaterFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

// define this before including commonuniforms so we have the correct samplers declared.

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "Common/CommonFragment.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/ACES.shader.h"

#if defined ( D_PLATFORM_ORBIS )
#pragma argument (O4; fastmath; scheduler=minpressure)
#if defined ( D_FAST_NORMALS )
#pragma argument (targetoccupancy_atallcosts=7)
#else
#pragma argument (targetoccupancy_atallcosts=6)
#endif
#endif

#if defined ( D_PLATFORM_SWITCH )
//#undef D_FAST_NORMALS
#pragma optionNV(unroll all)
#pragma optionNV(fastmath on)
//#define  pow( _X, _Y )         pow( max( _X, 0.0000001 ) , _Y )
#endif

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 


#include "Custom/WaterCommon.h"
#include "Common/CommonLighting.shader.h"
#include "Common/CommonFade.shader.h"
#include "Common/CommonPlanet.shader.h"
#include "Common/CommonScattering.shader.h"
#include "Common/CommonFog.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonTriplanarTexturing.shader.h"
#include "Common/CommonVertex.shader.h"
#include "Common/CommonGBuffer.shader.h"
#include "Common/CommonNoise.shader.h"
#include "Common/CommonPostProcess.shader.h"
#include "Common/ACES.shader.h"
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
    INPUT_NOPERSP( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


STATIC_CONST int NUM_STEPS = 8; 

STATIC_CONST float MAX_DISTANCE = 800.0;

STATIC_CONST mat3 octave_m = mat3(  0.00,  1.60,  1.20,
                                   -1.60,  0.72, -0.96,
                                   -1.20, -0.96,  1.28 );

float 
SeaNoise( 
    vec3  lPositionVec3, 
    float lChoppyness, 
    SAMPLER3DARG( lNoiseMap ) )
{
    return  texture3DLod( lNoiseMap, lPositionVec3, 0.0 ).g;
}

float 
Map( 
    vec3  lOriginVec3, 
    vec3  lPositionVec3, 
    float lfSeaLevel, 
    float lfTime,
    float lfWaveHeight,
    float lfWaveFreq,
    float lfWaveChoppiness,
    SAMPLER3DARG( lNoiseMap ),
    int liOctaves )
{
    vec3 lCurrentPositionVec3 = lPositionVec3;

    float lfDepth = length( lOriginVec3 - lPositionVec3 );

    float lfDepthFade = 1.0 - saturate( ( lfDepth - 30.0 ) / 50.0 );
 
    float lfNoise, lfHeight = 0.0;
    for( int i = 0; i < liOctaves; i++ )
    {
        //#ifdef D_FROM_BELOW
        //if( lfDepth > (200.0*200.0) && i == 1 )
        //{
        //    // stop after 1 octave
        //    break;
        //}
        //#else
        //if ( lfDepth > (100.0*100.0) && i == 1 )
        //{
        //    // stop after 1 octave
        //    break;
        //}            
        //#endif
        lfNoise   = SeaNoise( ( lCurrentPositionVec3 + float2vec3( lfTime ) )* lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
        lfNoise  += SeaNoise( ( lCurrentPositionVec3 - float2vec3( lfTime ) )* lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
        lfNoise *= 0.5;
        lfHeight += lfNoise * lfWaveHeight;

        lCurrentPositionVec3 = MUL( octave_m, lCurrentPositionVec3 ); 
        lfWaveFreq   *= lfWaveChoppiness;
        lfWaveHeight *= 0.22 * lfDepthFade;
        //lfWaveChoppiness = mix( lfWaveChoppiness, 1.0, 0.2 );
    }
    return (length( lPositionVec3 ) - lfSeaLevel) - lfHeight;
}

float
Map2(
    vec3  lOriginVec3,
    vec3  lPositionVec3,
    float lfSeaLevel,
    float lfTime,
    float lfWaveHeight,
    float lfWaveFreq,
    float lfWaveChoppiness,
    SAMPLER3DARG( lNoiseMap ) )
{
    // The Octave noise is an expansion of the following terms.  lfDepthFade is scaled externally to 
    //  this function.
    // 0.5 * lfWaveHeight * 
    // [ ( a0 + b0 ) * ( 0.22 * lfDepthFade ) ^ 0 +
    //    ( a1 + b1 ) * ( 0.22 * lfDepthFade ) ^ 1 +
    //    ( a2 + b2 ) * ( 0.22 * lfDepthFade ) ^ 2 +
    //    ( a3 + b3 ) * ( 0.22 * lfDepthFade ) ^ 3 +
    //]

    float lfDepth = length( lOriginVec3 - lPositionVec3 );
    float lfDepthFade = 1.0 - saturate( ( lfDepth - 30.0 ) / 50.0 );
    float lfDistance = length( lPositionVec3 ) - lfSeaLevel;

#if 0
    // Loop term expansion...
    float lfHeight = 0.0;

    for ( uint i = 0; i < 4; ++i )
    {
        float a = texture3DLod( lNoiseMap, ( lPositionVec3 + lfTime ) * lfWaveFreq, 0.0 ).g;
        float b = texture3DLod( lNoiseMap, ( lPositionVec3 - lfTime ) * lfWaveFreq, 0.0 ).g;
        lfHeight += ( a + b ) * pow( lfDepthFade, i );

        lPositionVec3 = MUL( octave_m, lPositionVec3 );

        lfWaveFreq *= lfWaveChoppiness;
    }

#else
    // Two term expansion...

    float lfHeight = 0.0;
    float lfTimeFreq = lfTime * lfWaveFreq;

    float a = texture3DLod( lNoiseMap, lPositionVec3 * lfWaveFreq + lfTimeFreq, 0.0 ).g;
    float b = texture3DLod( lNoiseMap, lPositionVec3 * lfWaveFreq - lfTimeFreq, 0.0 ).g;

    lfHeight += ( a + b );

    lPositionVec3 = MUL( octave_m, lPositionVec3 );

    a = texture3DLod( lNoiseMap, ( lPositionVec3 * lfWaveFreq + lfTimeFreq ) * lfWaveChoppiness, 0.0 ).g;
    b = texture3DLod( lNoiseMap, ( lPositionVec3 * lfWaveFreq - lfTimeFreq ) * lfWaveChoppiness, 0.0 ).g;

    lfHeight += ( a + b ) * lfDepthFade * 0.22;

#endif

    // WaveHeight is multiplied through elsewhere.
    //lfHeight *= 0.5 * lfWaveHeight;
    lfHeight *= lfWaveHeight;

    return lfDistance - lfHeight;
}

float
MapFastSingleFactor2(
    vec3  lOriginVec3,
    vec3  lPositionVec3,
    float lfSeaLevel,
    float lfTime,
    float lfWaveHeight,
    float lfWaveFreq,
    float lfWaveChoppiness,
    SAMPLER3DARG( lNoiseMap ) )
{
    // The Octave noise is an expansion of the following terms.  lfDepthFade is scaled externally to 
    //  this function.
    // 0.5 * lfWaveHeight * 
    // [ ( a0 + b0 ) * ( 0.22 * lfDepthFade ) ^ 0 +
    //    ( a1 + b1 ) * ( 0.22 * lfDepthFade ) ^ 1 +
    //    ( a2 + b2 ) * ( 0.22 * lfDepthFade ) ^ 2 +
    //    ( a3 + b3 ) * ( 0.22 * lfDepthFade ) ^ 3 +
    //]

    float lfDepth = length( lOriginVec3 - lPositionVec3 );
    float lfDepthFade = 1.0 - saturate( ( lfDepth - 30.0 ) / 50.0 );
    float lfDistance = length( lPositionVec3 ) - lfSeaLevel;

    // Two term expansion...

    float lfHeight = 0.0;
    float lfTimeFreq = lfTime * lfWaveFreq;

    float a = texture3DLod( lNoiseMap, lPositionVec3 * lfWaveFreq + lfTimeFreq, 0.0 ).g;
    lfHeight += a;

    lPositionVec3 = MUL( octave_m, lPositionVec3 );

    a = texture3DLod( lNoiseMap, ( lPositionVec3 * lfWaveFreq - lfTimeFreq ) * lfWaveChoppiness, 0.0 ).g;
    lfHeight += a * lfDepthFade * 0.22;

    lfHeight *= lfWaveHeight;

    return lfDistance - lfHeight;
}

vec3
GetWaveNormal(
    vec3  lOriginVec3,
    vec3  lPositionVec3,
    float lfOffset,
    float lfSeaLevel,
    float lfTime,
    float lfWaveHeight,
    float lfWaveFreq,
    float lfWaveChoppiness,
    SAMPLER3DARG( lNoiseMap ) )
{
    //int liOctaves = 2; // using the 2 term versions

#if !defined ( D_FAST_NORMALS )

    float lfHeight = Map2( lOriginVec3, lPositionVec3, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );

    vec3 lNormalVec3;

    lNormalVec3.x = Map2( lOriginVec3, vec3( lPositionVec3.x + lfOffset, lPositionVec3.y, lPositionVec3.z ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) ) - lfHeight;
    lNormalVec3.y = Map2( lOriginVec3, vec3( lPositionVec3.x, lPositionVec3.y + lfOffset, lPositionVec3.z ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) ) - lfHeight;
    lNormalVec3.z = Map2( lOriginVec3, vec3( lPositionVec3.x, lPositionVec3.y, lPositionVec3.z + lfOffset ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) ) - lfHeight;

#else

    float lfHeight = MapFastSingleFactor2( lOriginVec3, lPositionVec3, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );

    vec3 lNormalVec3;

    lNormalVec3.x = MapFastSingleFactor2( lOriginVec3, vec3( lPositionVec3.x + lfOffset, lPositionVec3.y, lPositionVec3.z ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) ) - lfHeight;
    lNormalVec3.y = MapFastSingleFactor2( lOriginVec3, vec3( lPositionVec3.x, lPositionVec3.y + lfOffset, lPositionVec3.z ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) ) - lfHeight;
    lNormalVec3.z = MapFastSingleFactor2( lOriginVec3, vec3( lPositionVec3.x, lPositionVec3.y, lPositionVec3.z + lfOffset ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) ) - lfHeight;

#endif
    return normalize( lNormalVec3 );
}

float
GetWavePosition(
    in vec3  lOriginVec3,
    in vec3  lDirectionVec3,
    in float lfSeaLevel,
    in float lfTime,
    in int liStepLod,
    float lfWaveHeight,
    float lfWaveFreq,
    float lfWaveChoppiness,
    SAMPLER3DARG( lNoiseMap ),
    out vec3 lPosition )
{
    float lfNear = 0.0;
    float lfFar = MAX_DISTANCE;
    int liOctaves = 2; // We're only using 2 octaves

#if !defined ( D_FAST_NORMALS )
    float hx = Map( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfFar, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ), liOctaves );
#else
    float hx = Map2( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfFar, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
#endif
    if ( hx > 0.0 )
    {
        return lfFar;
    }

#if !defined ( D_FAST_NORMALS )
    float hm = Map( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfNear, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ), liOctaves );
#else
    float hm = Map2( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfNear, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
#endif
    float lfMid = 0.0;

    for ( int i = 0; i < liStepLod; i++ )
    {
        lfMid = mix( lfNear, lfFar, hm / ( hm - hx ) );
        lPosition = lOriginVec3 + lDirectionVec3 * lfMid;

#if !defined ( D_FAST_NORMALS )
        float hmid = Map( lOriginVec3, lPosition, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ), liOctaves );
#else
        float hmid = Map2( lOriginVec3, lPosition, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
#endif
        if ( hmid < 0.0 )
        {
            lfFar = lfMid;
            hx = hmid;
        }
        else
        {
            lfNear = lfMid;
            hm = hmid;
        }
    }
    lfMid = mix( lfNear, lfFar, hm / ( hm - hx ) );
    return lfMid;
}

float
GetWavePositionFromBelow(
    vec3  lOriginVec3,
    vec3  lDirectionVec3,
    float lfSeaLevel,
    float lfTime,
    float lfWaveHeight,
    float lfWaveFreq,
    float lfWaveChoppiness,
    SAMPLER3DARG( lNoiseMap ),
    out vec3 lPosition )
{
    float lfNear = 0.0;
    float lfFar  = MAX_DISTANCE;
    int liOctaves = 3;
    
#if !defined ( D_FAST_NORMALS )
    float hx = Map( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfFar, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ), liOctaves );
#else
    lfWaveHeight *= 0.5;

    float hx = Map2( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfFar, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
#endif
    if( hx < 0.0 ) // reverse - we're moving from high density to low
    {
        return lfFar;
    }

#if !defined ( D_FAST_NORMALS )
    float hm = Map( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfNear, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ), liOctaves );
#else
    float hm = Map2( lOriginVec3, lOriginVec3 + lDirectionVec3 * lfNear, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
#endif

    float lfMid = 0.0;

    for( int i = 0; i < NUM_STEPS; i++ )
    {
        lfMid = mix( lfNear, lfFar, hm/( hm-hx ) );
        lPosition = lOriginVec3 + lDirectionVec3 * lfMid;

#if !defined ( D_FAST_NORMALS )        
        float hmid = Map( lOriginVec3, lPosition, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ), liOctaves );
#else
        float hmid = Map2( lOriginVec3, lPosition, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM( lNoiseMap ) );
#endif

        if( hmid >= 0.0 ) // reverse - we're moving from high density to low
        {
            lfFar = lfMid;
            hx = hmid;
        }
        else
        {
            lfNear = lfMid;
            hm = hmid;
        }
    }
    return lfMid;
}

vec4
GetSeaColour(
    vec3  lPositionVec3,
    vec3  lNormalVec3,
    vec3  lLightVec3,
    vec3  lToCameraVec3,
    vec3  lToOriginVec3,
    vec3  lNearColourVec3,
    vec3  lFarColourVec3,
    vec3  lSkyColourVec3,
    float lfWaveHeight,
    float lfFresnelPow,
    float lfFresnelMultiply,
    float lfFresnelAlpha,
    float lfSeaLevel,
    float lfSmoothness  )
{
    float lfFresnel = clamp( 1.0 - dot( lNormalVec3, -lToCameraVec3 ), 0.0, 1.0 );
    lfFresnel = pow( lfFresnel, lfFresnelPow ) * lfFresnelMultiply;

    float lfDiffuse  = smoothstep( 0.999, 1.0, dot( lNormalVec3, lLightVec3 ) );
    float lfSpecular = pow( max( dot( reflect( lToCameraVec3, lNormalVec3 ), lLightVec3 ), 0.0 ), 120.0 ) * 10.0 * lfSmoothness;

    vec3 lReflectedVec3 = lSkyColourVec3;
    vec3 lRefractedVec3 = lFarColourVec3 + lfDiffuse * lNearColourVec3 * 0.12;

    vec4 lSeaColourVec4;
    lSeaColourVec4.rgb = mix( lRefractedVec3, lReflectedVec3, lfFresnel );
    lSeaColourVec4.a   = saturate( lfFresnel + lfSpecular + lfFresnelAlpha );

    float lfAttenuation = max( 1.0 - dot( lToOriginVec3, lToOriginVec3 ) * 0.001, 0.0 );
    lSeaColourVec4.rgb  += lNearColourVec3 * ( length( lPositionVec3 ) - ( lfWaveHeight + lfSeaLevel ) ) * 0.18 * lfAttenuation;

    lSeaColourVec4.rgb  += float2vec3( lfSpecular );
    //lSeaColourVec4 = vec4( 1.0, 0.0, 0.0, 1.0 );
    //lSeaColourVec4.rgb = lNormalVec3 * 0.5 + vec3( 0.5 );
    //lSeaColourVec4.a = 1.0;

    return lSeaColourVec4;
}


vec4
GetFoamColour(
    in CustomPerMaterialUniforms lPerMaterialUniforms,
    in vec3 lPlanetRelPositionVec3,
    in vec3 lLocalNormalVec3,
    in float lfTime,
    in float lfFoamArea,
    in float lfDepth )
{
    vec2  lWindDirection1  = lPerMaterialUniforms.gWindDirectionVec4.xy;
    vec2  lWindDirection2  = lPerMaterialUniforms.gWindDirectionVec4.zw;

    float lfFoamScale1   = lPerMaterialUniforms.gFoamParamsVec4.x;
    float lfFoamScale2   = lPerMaterialUniforms.gFoamParamsVec4.y;
    vec2  lFoamTime1Vec2 = lWindDirection1 * lfTime * lPerMaterialUniforms.gFoamParamsVec4.z;
    vec2  lFoamTime2Vec2 = lWindDirection2 * lfTime * lPerMaterialUniforms.gFoamParamsVec4.w;
    vec2  lFoamTime3Vec2 = lWindDirection2 * lfTime * lPerMaterialUniforms.gWaterSurfaceParamsVec4.z * 2.0;
    vec2  lFoamTime4Vec2 = lWindDirection1 * lfTime * lPerMaterialUniforms.gWaterSurfaceParamsVec4.w * 2.0;

    float lfFoamHeightTex  = 1.0 - GetTriPlanarColourMM( lLocalNormalVec3, lPlanetRelPositionVec3, lFoamTime2Vec2, lfFoamScale2, SAMPLER2DPARAM_LOCAL( lPerMaterialUniforms, gFoamHeightMap ) ).r;

    //lPlanetRelPositionVec3 = vec3( lPlanetRelPositionVec3.y, lPlanetRelPositionVec3.z, lPlanetRelPositionVec3.x );
    float lfFoamHeightTex2 = 1.0 - GetTriPlanarColourMM( lLocalNormalVec3, lPlanetRelPositionVec3, lFoamTime1Vec2, lfFoamScale1, SAMPLER2DPARAM_LOCAL( lPerMaterialUniforms, gFoamHeightMap ) ).r;

    float lfFoam = 0.0;

    lfFoamHeightTex = ( lfFoamHeightTex + lfFoamHeightTex2 ) / 2.0;

    lfFoam = smoothstep( clamp( lfFoamHeightTex - 0.15, 0.0, 1.0 ), lfFoamHeightTex + 0.15, lfFoamArea );
    lfFoam += pow( lfFoamArea, 10.0 );
    lfFoam *= lfFoamHeightTex2;
    lfFoam = min( lfFoam, 1.0 );

   // lfFoam *= lPerMaterialUniforms.gFoamColourVec4.a;

    vec4 lFoamColour;
    
    lFoamColour = ( lPerMaterialUniforms.gFoamColourVec4 ) * lfFoam;

    lFoamColour.a = min( pow( lfDepth, 10.0 ), 1.0 ); 

    return lFoamColour;
}

vec4
GetFogColour(
    vec4 lInputColourVec4,
    vec3 lNearColour,
    vec3 lFarColour,
    vec3 lLightColour,
    vec3 lLightDirection,
    vec3 lPlanetRelPositionVec3,
    vec3 lCameraPositionVec3,
    float lfWaterStrength,
    float lfWaterColourStrength,
    float lfWaterMultiplyStrength,
    float lfSeaLevel )
{
    vec4  lFragmentColourVec4 = lInputColourVec4;
    vec3  lNearPosition       = lCameraPositionVec3;
    vec3  lFarPosition        = lPlanetRelPositionVec3;

    float lfDarken          = clamp( dot( lLightDirection, normalize( lFarPosition ) ), 0.0, 1.0 );

    float lfFogDistance = length( lFarPosition - lNearPosition );

    float lfFogValue        = lfFogDistance * lfWaterStrength;
    lfFogValue              = 1.0 / exp( lfFogValue * lfFogValue );
    lfFogValue              = 1.0 - clamp( lfFogValue, 0.0, 1.0 );

    float lfWaterFade       = lfFogDistance * lfWaterColourStrength;
    lfWaterFade             = 1.0 / exp( lfWaterFade * lfWaterFade );
    lfWaterFade             = 1.0 - clamp( lfWaterFade, 0.0, 1.0 );

    vec3 lWaterColour        = mix( lNearColour, lFarColour, clamp( lfWaterFade, 0.0, 1.0 ) );// * /*lLightColour */ lfDarken;
    lFragmentColourVec4.rgb  = mix( lFragmentColourVec4.rgb, lWaterColour, clamp( lfFogValue, 0.0, 1.0 ) );
    lFragmentColourVec4.a    = max( lFragmentColourVec4.a, clamp( lfFogValue * 2.0, 0.0, 1.0 ) );

    return lFragmentColourVec4;
}

float ReadDepth(
    in CustomPerMaterialUniforms lPerMaterialUniforms,
    in vec4 lClipPlanes,
    in vec3 lVREyeInfo,
#if defined ( D_PLATFORM_METAL ) && defined ( D_WRITE_LINEARDEPTH )
    in vec2 lFragCoordsVec2,
    RWIMAGE2DACCESSARG( rgba32f, lRWDepthLinear, access::read_write )
#else
    in vec2 lFragCoordsVec2
#endif
)
{
#ifdef   D_WRITE_LINEARDEPTH

#if defined ( D_PLATFORM_SWITCH )
    vec4 lLinearDepthInVec4 = imageLoad(lPerMaterialUniforms.gRWDepthLinear, ivec2(lFragCoordsVec2 * GetImgResolution(lPerMaterialUniforms.gRWDepthLinear)));
#elif defined  ( D_PLATFORM_METAL )
    vec4 lLinearDepthInVec4 = imageLoad(IMAGE_GETMAP(lPerMaterialUniforms, lRWDepthLinear), ivec2(lFragCoordsVec2 * GetImgResolution(IMAGE_GETMAP(lPerMaterialUniforms, lRWDepthLinear))));
#else
    vec4 lLinearDepthInVec4 = imageLoad(SAMPLER_GETLOCAL(lPerMaterialUniforms, gRWDepthLinear), ivec2(lFragCoordsVec2 * GetImgResolution(SAMPLER_GETLOCAL(lPerMaterialUniforms, gRWDepthLinear))));
#endif
    float lfOriginalDepth = FastDenormaliseDepth(lClipPlanes, DecodeDepthFromColour(lLinearDepthInVec4));

#else

#ifdef D_PLATFORM_ORBIS
    float lfOriginalDepth = FastDenormaliseDepth(lClipPlanes, DecodeDepthFromColour(texture2DArray(SAMPLER_GETLOCAL(lPerMaterialUniforms, gBuffer1Map), vec3(lFragCoordsVec2, lVREyeInfo.x))));
#else
    float lfOriginalDepth = FastDenormaliseDepth(lClipPlanes, DecodeDepthFromColour(texture2D(SAMPLER_GETLOCAL(lPerMaterialUniforms, gBuffer1Map), lFragCoordsVec2)));
#endif

#endif

    return lfOriginalDepth;
}

#ifdef D_FORWARD
FRAGMENT_MAIN_COLOUR_SRT
#else
FRAGMENT_MAIN_COLOUR_DEPTH_SRT
#endif
#ifdef D_FULLSCREEN
{
    vec3 lViewPos = lUniforms.mpPerFrame.gViewPositionVec3;

    float lfHeightFade = length( lViewPos - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;
    lfHeightFade       = clamp( ( lfHeightFade - 1000.0 ) / 500.0, 0.0, 1.0 );
    if( lfHeightFade >= 1.0 )
    {
#if defined ( D_COMPUTE )
        return;
#else
        discard;
#endif
    }

    //vec2  lFragCoordsVec2 = IN(mTexCoordsVec2);
    vec2  lFragCoordsVec2 = TEX_COORDS.xy; 

    vec2 lFragCoordsNVec2 = lFragCoordsVec2;
    lFragCoordsNVec2.x = (lFragCoordsNVec2.x - lUniforms.mpPerFrame.gVREyeInfoVec3.y) * lUniforms.mpPerFrame.gVREyeInfoVec3.z;
    //lFragCoordsNVec2 = GetDejitteredTexCoord( lFragCoordsVec2, lUniforms.mpPerFrame.gDeJitterVec4 );


    float lfOriginalDepth = ReadDepth(
        DEREF_PTR(lUniforms.mpCustomPerMaterial),
        lUniforms.mpPerFrame.gClipPlanesVec4,
        lUniforms.mpPerFrame.gVREyeInfoVec3,
#if defined ( D_PLATFORM_METAL ) && defined ( D_WRITE_LINEARDEPTH )
        lFragCoordsVec2,
        IMAGE_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear));
#else  
        lFragCoordsVec2);
#endif

    float lfSceneDepth         = lfOriginalDepth;
    if( lfOriginalDepth >= lUniforms.mpPerFrame.gClipPlanesVec4.y - 100.0 )
    {
        lfSceneDepth = 50000.0;
    }

    vec3  lPositionVec3;
    vec3  lLightDirectionVec3;
    float lfAttenuation;

    lPositionVec3 = RecreatePositionFromDepthWithIVP(lfSceneDepth, lFragCoordsVec2, lViewPos, lUniforms.mpPerFrame.gInverseViewProjectionMat4, lUniforms.mpPerFrame.gClipPlanesVec4);


    float lfWaterHeight = lUniforms.mpCustomPerMaterial.gWaterFogVec4.r;

    float lfAddedHeight = length( lViewPos - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;
    lfAddedHeight       = clamp( ( lfAddedHeight - lfWaterHeight ) / 3.0, 0.0, 1.0 );
    lfAddedHeight       = lUniforms.mpCustomPerMaterial.gFresnelParamsVec4.r;//mix( 0.0, 1.0, lfAddedHeight );

    vec3  lPlanetPosition      = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz;
    float lfRadius             = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w;

    float lfSeaLevel           = lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w + lfWaterHeight;
    float lfTime               = lUniforms.mpPerFrame.gfTime;

    vec3  lNearPosition        = lViewPos - lPlanetPosition;
    vec3  lFarPosition         = lPositionVec3 - lPlanetPosition;

    vec3  lLightColourVec3        = ( lUniforms.mpCommonPerMesh.gLightColourVec4.rgb );
    vec3  lWaterColourBaseVec3    = ( lUniforms.mpCustomPerMaterial.gWaterColourBaseVec4.rgb )    * lLightColourVec3;
    vec3  lWaterColourAddVec3     = ( lUniforms.mpCustomPerMaterial.gWaterColourAddVec4.rgb )     * lLightColourVec3;
    vec3  lWaterFogColourFarVec3  = ( lUniforms.mpCustomPerMaterial.gWaterFogColourFarVec4.rgb )  * lLightColourVec3;
    vec3  lWaterFogColourNearVec3 = ( lUniforms.mpCustomPerMaterial.gWaterFogColourNearVec4.rgb ) * lLightColourVec3;


    vec3 lSurfacePositionVec3, lDummyPos2;

    float lfRayTest = GetRayIntersectionPoint( lNearPosition, lFarPosition, ( lfRadius + lfWaterHeight ), lSurfacePositionVec3, lDummyPos2 );

#ifdef D_FROM_BELOW

    lSurfacePositionVec3 = lDummyPos2;

    if( lfRayTest == 0.0 )
    {
#if defined ( D_COMPUTE )
        return;
#else
        discard;
#endif
    }
    if( lfRayTest == 1.0 )
    {
#if defined ( D_COMPUTE )
        return;
#else
        discard;
#endif
    }
    if( lfRayTest == 3.0 )
    {
#if defined ( D_COMPUTE )
        return;
#else
        discard;
#endif
    }
#else
    if( lfRayTest == 0.0 )
    {
#if defined ( D_COMPUTE )
        return;
#else
        discard;
#endif
    }
    if( lfRayTest == 2.0 )
    {
#if defined ( D_COMPUTE )
        return;
#else
        discard;
#endif
    }
#endif // D_FROM_BELOW

    float lfHeightTrace;
    vec3  lUp               = normalize( lSurfacePositionVec3 );
    lSurfacePositionVec3 -= lUp;
    lfSeaLevel           -= lfAddedHeight * 1.0;

    vec3  lRayOriginVec3    = lNearPosition;
    vec3  lRayDirectionVec3 = normalize( lFarPosition - lRayOriginVec3 );
    vec3  lWavePositionVec3 = lSurfacePositionVec3;
    vec3  lNormal = lUp;

    float lfWaveHeight      = lUniforms.mpCustomPerMaterial.gFresnelParamsVec4.r;
    float lfWaveFreq        = lUniforms.mpCustomPerMaterial.gFresnelParamsVec4.g;
    float lfWaveChoppiness  = lUniforms.mpCustomPerMaterial.gFresnelParamsVec4.b;
    float lfWaveCutoff      = lUniforms.mpCustomPerMaterial.gFresnelParamsVec4.a;
    float lfEpsilonNormal   = lUniforms.mpCustomPerMaterial.gWaterSurfaceParamsVec4.r;

    // If we're up very high, don't run the full get wave position at all!

    float lfBlend = 1.0;
    //vec3 lToOriginVec3;
	
    lWavePositionVec3 = lSurfacePositionVec3;
    
//#if defined ( D_COMPUTE )
    //float lfNewDepth = dot( ( lWavePositionVec3 + lPlanetPosition ) - lViewPos, -normalize( MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 ).xyz ) );
    //WRITE_FRAGMENT_COLOUR( vec4( lfHeightTrace / 10000.0, 0.0, 1.0, 1.0 ) );
    //WRITE_FRAGMENT_DEPTH( LinearToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfNewDepth ) );
    //return;
//    float lfNewDepth;
//#else
    float lfNewDepth;
//#endif

    vec3 lToOriginVec3 = lWavePositionVec3 - lRayOriginVec3;
    vec3 lFarDistance = lRayOriginVec3 + lRayDirectionVec3 * MAX_DISTANCE;

    float lfLodDistance = length( lFarDistance ) - lfSeaLevel - lfWaveFreq * lfWaveHeight;

    if ( lfLodDistance > 0.0 )
    {
        lfHeightTrace = MAX_DISTANCE;
    }
    else
    {
        const float lfLodScale = 256.0 / 800.0;
        lfLodDistance += MAX_DISTANCE;
        float lfContinuousLod = log( max( 0.0, lfLodDistance ) * lfLodScale );
        int liIterationCount = int( min( int( max( NUM_STEPS - lfContinuousLod, 1.0 ) ), NUM_STEPS ) );
        
        //float lfNewDepth = dot( ( lWavePositionVec3 + lPlanetPosition ) - lViewPos, -normalize( MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 ).xyz ) );
        //WRITE_FRAGMENT_COLOUR( vec4( lfContinuousLod / 8.0, 0.0, 1.0, 1.0 ) );
        //WRITE_FRAGMENT_DEPTH( LinearToReverseZDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfNewDepth ) );
        //return;

#ifdef D_FROM_BELOW
        {
            lfHeightTrace = GetWavePositionFromBelow( lRayOriginVec3, lRayDirectionVec3, lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM_SRT( lUniforms.mpCustomPerMaterial, gNoiseMap ), lWavePositionVec3 );
        }
#else
        {
            lfHeightTrace = GetWavePosition( lRayOriginVec3, lRayDirectionVec3, lfSeaLevel, lfTime, liIterationCount, lfWaveHeight * 0.5, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM_SRT( lUniforms.mpCustomPerMaterial, gNoiseMap ), lWavePositionVec3 );
        }
#endif // D_FROM_BELOW
    }

    lToOriginVec3 = lWavePositionVec3 - lRayOriginVec3;

    lfBlend = saturate( ( lfHeightTrace - ( MAX_DISTANCE - 500.0 ) ) / 500.0 );

    vec3 lAt = -normalize( lToOriginVec3 );
    float lfDot = dot( lUp, lAt );
    lfDot = abs( lfDot );
    lfDot = smoothstep( 0.02, 0.12, lfDot );
    lfBlend = 1.0 - ( ( 1.0 - lfBlend ) * lfDot );

    if ( lfHeightTrace < MAX_DISTANCE && lfBlend < 1.0 )
    {
        float lfEpsilon = length( lViewPos - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz ) - ( lUniforms.mpCommonPerMesh.gPlanetPositionVec4.w + lfWaterHeight );
        lfEpsilon = clamp( ( lfEpsilon - 20.0 ) / 100.0, 0.0, 1.0 );
        lfEpsilon = mix( 0.1, lfEpsilonNormal, lfEpsilon );
        lfEpsilon /= ( 1920.0f * 0.5f );

#if !defined ( D_FAST_NORMALS )
        lNormal = GetWaveNormal( lRayOriginVec3, lWavePositionVec3, max( dot( lToOriginVec3, lToOriginVec3 ) * lfEpsilon, lfWaveCutoff ), lfSeaLevel, lfTime, lfWaveHeight * 0.5, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM_SRT( lUniforms.mpCustomPerMaterial, gNoiseMap ) );
#else
        lNormal = GetWaveNormal( lRayOriginVec3, lWavePositionVec3, max( dot( lToOriginVec3, lToOriginVec3 ) * lfEpsilon, lfWaveCutoff ), lfSeaLevel, lfTime, lfWaveHeight, lfWaveFreq, lfWaveChoppiness, SAMPLER3DPARAM_SRT( lUniforms.mpCustomPerMaterial, gNoiseMap ) );
#endif
    }
    else
    {
        lWavePositionVec3 = lSurfacePositionVec3;
    }
    
    vec3 lLight  = normalize( -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz );

    lNormal           = normalize( mix( lNormal, lUp, lfBlend ) );
    lWavePositionVec3 = mix( lWavePositionVec3, lSurfacePositionVec3, lfBlend );
    //float lfNewDepth  = dot( ( lWavePositionVec3 + lPlanetPosition ) - lViewPos, -normalize( MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 ).xyz ) );
    lfNewDepth = dot( ( lWavePositionVec3 + lPlanetPosition ) - lViewPos, -normalize( MAT4_GET_COLUMN( lUniforms.mpPerFrame.gCameraMat4, 2 ).xyz ) );

    // Reflections
    vec4 lFinalColour;

    #ifdef D_FROM_BELOW
    {
#if defined( D_FORWARD )
		//TF_BEGIN
		if (lfNewDepth >= lfSceneDepth)
		{
#if defined ( D_COMPUTE )
			return;
#else
			discard;
#endif
		}
		//TF_END
#endif
        float lfFresnelPow      = lUniforms.mpCustomPerMaterial.gWaveSpeedVec4.r;
        float lfFresnelMultiply = lUniforms.mpCustomPerMaterial.gWaveSpeedVec4.g;
        float lfFresnelAlpha    = lUniforms.mpCustomPerMaterial.gWaveSpeedVec4.b;

        vec3 lReflectionVec3;

#if defined ( D_USE_REFLECTION_MAP )
        {
            float lfReflectionScale1 = lUniforms.mpCustomPerMaterial.gFoamParamsVec4.x * 0.01;
            vec2  lReflectionTime1Vec2 = lUniforms.mpCustomPerMaterial.gWindDirectionVec4.xy * lfTime * lUniforms.mpCustomPerMaterial.gFoamParamsVec4.z * 0.1;
            vec3  lReflectionTextureAVec3 = GetTriPlanarColourMM(lNormal, lSurfacePositionVec3, lReflectionTime1Vec2, lfReflectionScale1, SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMaterial, gFoamMap));

            // Offset based on normal map
            vec2 lProjectedCoordsVec2 = lFragCoordsNVec2;

            lProjectedCoordsVec2 += vec2(lReflectionTextureAVec3.r, lReflectionTextureAVec3.r) * 0.2 - float2vec2(0.1);
            lProjectedCoordsVec2 -= float2vec2(saturate((length(lWavePositionVec3 - lSurfacePositionVec3) - 1.0) * 0.05)) * 0.05;
            lReflectionVec3 = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gReflectionMap), lProjectedCoordsVec2).rgb;
        }
#else
        lReflectionVec3 = lUniforms.mpCustomPerMaterial.gSkyColourVec4.rgb;
#endif

        vec3 lWaterColourVec3 = lWaterFogColourNearVec3;

        float lfCamHeight = lfSeaLevel - length( lNearPosition );
        vec3 lCamPos      = normalize( lNearPosition ) * ( lfSeaLevel + lfCamHeight );
        lCamPos           = normalize( lCamPos - lWavePositionVec3 );

        lLight = reflect( lLight, lUp );

        lFinalColour = GetSeaColour(
            lWavePositionVec3,
            -lNormal,
            lLight,
            lRayDirectionVec3,
            lToOriginVec3,
            lWaterColourAddVec3,
            lWaterColourBaseVec3,
            lReflectionVec3,
            0.6,
            lfFresnelPow,
            lfFresnelMultiply,
            lfFresnelAlpha,
            lfSeaLevel,
            1.0 );

        lFinalColour = GetFogColour(
                    lFinalColour,
                    lWaterFogColourNearVec3,
                    lWaterFogColourFarVec3,
                    lUniforms.mpCommonPerMesh.gLightColourVec4.rgb,
                    lLight,
                    lWavePositionVec3,
                    lViewPos - lUniforms.mpCommonPerMesh.gPlanetPositionVec4.xyz,
                    lUniforms.mpCustomPerMaterial.gWaterFogVec4.g,
                    lUniforms.mpCustomPerMaterial.gWaterFogVec4.b,
                    lUniforms.mpCustomPerMaterial.gWaterFogVec4.a,
                    lfSeaLevel );

    }
    #else
    {
        if( lfNewDepth >= lfSceneDepth )
        {
#if defined ( D_COMPUTE )
            return;
#else
            discard;
#endif
        }

        // Calculate the general sea colour first...

        vec2 lProjectedCoordsVec2 = lFragCoordsNVec2;

        float lfFresnelPow      = lUniforms.mpCustomPerMaterial.gWaveScaleVec4.r;
        float lfFresnelMultiply = lUniforms.mpCustomPerMaterial.gWaveScaleVec4.g;
        float lfFresnelAlpha    = lUniforms.mpCustomPerMaterial.gWaveScaleVec4.b;


#if defined ( D_USE_REFLECTION_MAP )
        float lfReflectionScale1      = lUniforms.mpCustomPerMaterial.gFoamParamsVec4.x * 0.01;
        vec2  lReflectionTime1Vec2    = lUniforms.mpCustomPerMaterial.gWindDirectionVec4.xy * lfTime * lUniforms.mpCustomPerMaterial.gFoamParamsVec4.z * 0.1;
        vec3  lReflectionTextureAVec3 = GetTriPlanarColourMM( lNormal, lSurfacePositionVec3, lReflectionTime1Vec2, lfReflectionScale1, SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFoamMap ) );

        // Offset based on normal map
        lProjectedCoordsVec2 += vec2( lReflectionTextureAVec3.r, lReflectionTextureAVec3.r ) * 0.06 - float2vec2( 0.03 );
        lProjectedCoordsVec2 -= float2vec2( saturate( ( length( lWavePositionVec3 - lSurfacePositionVec3 ) - 1.0 ) * 0.05 ) ) * 0.05;
#ifdef D_PLATFORM_ORBIS
        //lProjectedCoordsVec2.y -= 0.5;
        //lProjectedCoordsVec2.y /= 1.05733836;
        //lProjectedCoordsVec2.y += 0.5;
#endif
        vec3 lReflectionVec3  = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMaterial, gReflectionMap ), lProjectedCoordsVec2 ).rgb;
#else
        vec3 lReflectionVec3 = lUniforms.mpCustomPerMaterial.gSkyColourVec4.rgb;
#endif

        lFinalColour = GetSeaColour( 
            lWavePositionVec3,
            lNormal,
            lLight,
            lRayDirectionVec3,
            lToOriginVec3,
            lWaterColourAddVec3,
            lWaterColourBaseVec3,
            lReflectionVec3,
            0.6,
            lfFresnelPow,
            lfFresnelMultiply,
            lfFresnelAlpha,
            lfSeaLevel,
            1.0 );

        if ( lfDot > 0.0 )
        {
            // Calculate the foam scale.  If we order the sea colour, followed by the foam scale, we can reduce the memory and register pressure for the shader.

            float lfFoamScale1 = lUniforms.mpCustomPerMaterial.gFoamParamsVec4.x;
            vec2  lFoamTime1Vec2 = lUniforms.mpCustomPerMaterial.gWindDirectionVec4.xy * lfTime * lUniforms.mpCustomPerMaterial.gFoamParamsVec4.z;
            vec3  lFoamTextureAVec3 = GetTriPlanarColourMM( lUp, lSurfacePositionVec3, lFoamTime1Vec2, lfFoamScale1, SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMaterial, gFoamMap ) );

            vec2  lFoamCoords1 = lFragCoordsNVec2 + ( vec2( lFoamTextureAVec3.r, lFoamTextureAVec3.r ) * 0.08 - float2vec2( 0.04 ) );
#ifndef D_PLATFORM_SWITCH
            lFoamCoords1.x = ( lFoamCoords1.x / lUniforms.mpPerFrame.gVREyeInfoVec3.z ) + lUniforms.mpPerFrame.gVREyeInfoVec3.y;
#endif

            float lfFoamDepth1 = ReadDepth(
                DEREF_PTR(lUniforms.mpCustomPerMaterial),
                lUniforms.mpPerFrame.gClipPlanesVec4,
                lUniforms.mpPerFrame.gVREyeInfoVec3,
#if defined ( D_PLATFORM_METAL ) && defined ( D_WRITE_LINEARDEPTH )
                lFoamCoords1,
                IMAGE_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear));
#else  
                lFoamCoords1);
#endif

            vec2  lFoamCoords3 = lFragCoordsNVec2 - ( vec2( lFoamTextureAVec3.r, lFoamTextureAVec3.r ) * 0.08 - float2vec2( 0.04 ) );
#ifndef D_PLATFORM_SWITCH
            lFoamCoords3.x = ( lFoamCoords3.x / lUniforms.mpPerFrame.gVREyeInfoVec3.z ) + lUniforms.mpPerFrame.gVREyeInfoVec3.y;
#endif
            float lfFoamDepth3 = ReadDepth(
                DEREF_PTR(lUniforms.mpCustomPerMaterial),
                lUniforms.mpPerFrame.gClipPlanesVec4,
                lUniforms.mpPerFrame.gVREyeInfoVec3,
#if defined ( D_PLATFORM_METAL ) && defined ( D_WRITE_LINEARDEPTH )
                lFoamCoords3,
                IMAGE_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear));
#else  
                lFoamCoords3);
#endif                
            //lfFoamDepth1 = min( lfFoamDepth1, min( lfSceneDepth, lfFoamDepth3 ) );
            lfFoamDepth1 = min( lfFoamDepth1, lfFoamDepth3 );

            float lfFoamBlend = ( saturate( ( max( lfFoamDepth1, lfNewDepth ) - 5.0 ) / 2.0 ) * 0.8 + 0.2 ) * lfDot;

            if ( lfFoamBlend > 0.0 )
            {
                float lfFoamHeight1 = 1.0 - saturate( ( lfFoamDepth1 - lfNewDepth ) / 8.0 );
                float lfFoamHeight2 = saturate( ( lfSceneDepth - lfNewDepth ) );
                if ( lfFoamDepth1 < lfNewDepth )
                {
                    lfFoamHeight1 = 1.0 - saturate( ( lfSceneDepth - lfNewDepth ) / 8.0 );
                }
                vec4 lFoamColour = GetFoamColour( DEREF_PTR( lUniforms.mpCustomPerMaterial ), lSurfacePositionVec3, lUp, lfTime, lfFoamHeight1, lfFoamHeight2 );

                lFoamColour.rgb *= lfFoamBlend;

                // Calculate colour
                lFinalColour.rgb += lFoamColour.rgb;
                lFinalColour.a = min( lFinalColour.a, lFoamColour.a );
            }
        }

        lFinalColour = saturate( lFinalColour );
    }
    #endif // D_FROM_BELOW

    if( lfSceneDepth >= 50000.0 )
    {
        lFinalColour.rgb = mix( lWaterFogColourFarVec3, lFinalColour.rgb, lFinalColour.a );
        lFinalColour.a = 1.0;
        //lfNewDepth = lfOriginalDepth;
    }

    //#ifdef D_FROM_BELOW
    //    lFinalColour = vec4( 0.0, 1.0, 0.0, 1.0 );//mix( lFinalColour.a, 0.0, lfHeightFade );
    //#else
    //    lFinalColour = vec4( 0.0, 0.0, 1.0, 1.0 );//mix( lFinalColour.a, 0.0, lfHeightFade );
    //#endif

    //if( lFragCoordsVec2.x > 0.5 )
    //{
    //    lFinalColour = GetDepthColour( min( lfNewDepth, lfOriginalDepth ) );
    //}
    //else
    //{
    //    lFinalColour = GetDepthColour( lfOriginalDepth );
    //}

	float lfNewDepthRevZ = LinearToReverseZDepth(lUniforms.mpPerFrame.gClipPlanesVec4, lfNewDepth);

#if defined( D_CONVERT_SRGB_P3 )
    lFinalColour.rgb = MUL(lFinalColour.rgb, sRGB_TO_P3D65);
#endif

#if defined ( D_COMPUTE )
    WRITE_FRAGMENT_DEPTH( lfNewDepthRevZ );
    WRITE_FRAGMENT_COLOUR( lFinalColour );
#else
#if defined( D_FORWARD )
	//TF_BEGIN
	if (lfNewDepthRevZ <= 0.0)
	{
		discard;
	}

	lFinalColour.rgb = MUL(lFinalColour.rgb, sRGB_TO_P3D65);
	FRAGMENT_COLOUR = lFinalColour;
	//TF_END
#else
    FRAGMENT_DEPTH = lfNewDepthRevZ;
    FRAGMENT_COLOUR = lFinalColour;
#endif // defined( D_FORWARD )
#endif // defined ( D_COMPUTE )
#ifdef    D_WRITE_LINEARDEPTH
#if defined ( D_PLATFORM_SWITCH )
    imageStore( lUniforms.mpCustomPerMaterial.gRWDepthLinear, ivec2( lFragCoordsVec2 * GetImgResolution( lUniforms.mpCustomPerMaterial.gRWDepthLinear ) ), vec4(lfNewDepth, lfNewDepth, lfNewDepth, lfNewDepth) );
#elif defined ( D_PLATFORM_METAL )
    //imageStore( IMAGE_GETMAP( lUniforms.mpCustomPerMaterial, gRWDepthLinear ), ivec2(lFragCoordsVec2 * GetImgResolution(IMAGE_GETMAP( lUniforms.mpCustomPerMaterial, gRWDepthLinear ))), vec4(lfNewDepth, lfNewDepth, lfNewDepth, lfNewDepth));
    imageStore(IMAGE_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear), ivec2(lFragCoordsVec2* GetImgResolution(IMAGE_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear))), vec4(lfNewDepth, lfNewDepth, lfNewDepth, lfNewDepth));
#else
    imageStore( SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear), ivec2( lFragCoordsVec2 * GetImgResolution( SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, gRWDepthLinear) ) ), vec4(lfNewDepth, lfNewDepth, lfNewDepth, lfNewDepth) );
#endif
#endif

}

#endif