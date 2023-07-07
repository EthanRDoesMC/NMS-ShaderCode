////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonFade.h
///     @author     User
///     @date       
///
///     @brief      CommonFogShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_COMMONFADE_H
#define D_COMMONFADE_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonUniforms.shader.h"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Functions


//-----------------------------------------------------------------------------
///
///     Hash
///
///     @brief      Hash
///
///     @param      vec2 lPositionVec2 
///     @return     float
///
//-----------------------------------------------------------------------------
float 
Hash( 
    vec3 lPositionVec3 )
{
    return fract(sin(dot(lPositionVec3,vec3(1.0,113.0,337.0)))*43758.5453123);
}

//-----------------------------------------------------------------------------
///
///     CalculateFadeValue
///
///     @brief      Hash
///
///     @param      vec2 lPositionVec2 
///     @return     float
///
//-----------------------------------------------------------------------------
void
CheckFade(
    SAMPLER2DARG( lFadeNoiseMap ),
    vec4          lScreenSpacePositionVec4,
    float         lfCommonFadeValue,
    vec2          lScreenSizeVec2,
    vec3          lVREyeInfoVec3 )
{
	if (lfCommonFadeValue < 1.0)
	{
        vec2  lScreenVal  = ( ( lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w ) + 1.0 ) * float2vec2( 0.5 );

        lScreenVal.x = (lScreenVal.x - lVREyeInfoVec3.y) * lVREyeInfoVec3.z;

        lScreenVal *= vec2( lScreenSizeVec2.x / 512.0, lScreenSizeVec2.y / 512.0 );

	    float lfScreenVal = texture2DLod( lFadeNoiseMap, lScreenVal, 0.0 ).x;
		
	    float lfFadeValue = lfCommonFadeValue;
	
	    if( lfFadeValue < 0.0 )
	    {
	        // Fade out
	        if( ( 1.0-lfScreenVal ) > ( lfFadeValue+2.0 ) )
	        {
#if defined ( D_COMPUTE )
                return;
#else
	            discard;
#endif
	        }
	    }
	    else
	    {
	        // Fade in
	        if( lfScreenVal > lfFadeValue )
	        {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
	        }
	    }
	}
}

//-----------------------------------------------------------------------------
///
///     CheckFadeFragCoord
///
///     @brief      Runs fade with gl_fragCoord or SV_POSITION as input
///
///     @param      vec2 lPositionVec2 
///     @return     float
///
//-----------------------------------------------------------------------------
void
CheckFadeFragCoord(
    SAMPLER2DARG( lFadeNoiseMap ),
    vec4          lFragCoordVec4,
    float         lfCommonFadeValue,
    vec3          lVREyeInfoVec3 )
{
    if ( lfCommonFadeValue < 1.0 )
    {
        vec2  lScreenVal = lFragCoordVec4.xy;

        lScreenVal.x = ( lScreenVal.x - lVREyeInfoVec3.y ) * lVREyeInfoVec3.z;

        lScreenVal *= vec2( 1.0 / 512.0, 1.0 / 512.0 );

        float lfScreenVal = texture2DLod( lFadeNoiseMap, lScreenVal, 0.0 ).x;

        float lfFadeValue = lfCommonFadeValue;

        if ( lfFadeValue < 0.0 )
        {
            // Fade out
            if ( ( 1.0 - lfScreenVal ) > ( lfFadeValue + 2.0 ) )
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
        else
        {
            // Fade in
            if ( lfScreenVal > lfFadeValue )
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
    }
}

//-----------------------------------------------------------------------------
///
///     CheckFadeFragCoordNoVR
///
///     @brief      Runs fade with gl_fragCoord or SV_POSITION as input
///
///     @param      vec2 lPositionVec2 
///     @return     float
///
//-----------------------------------------------------------------------------
void
CheckFadeFragCoordNoVR(
    SAMPLER2DARG( lFadeNoiseMap ),
    vec4          lFragCoordVec4,    
    float         lfCommonFadeValue,
    float         lfTextureWidth )
{
    if ( lfCommonFadeValue < 1.0 )
    {
        vec2  lScreenVal = lFragCoordVec4.xy;
        lScreenVal *= vec2( 1.0 / lfTextureWidth, 1.0 / lfTextureWidth );

        float lfScreenVal = texture2DLod( lFadeNoiseMap, lScreenVal, 0.0 ).x;

        float lfFadeValue = lfCommonFadeValue;

        if ( lfFadeValue < 0.0 )
        {
            // Fade out
            if ( ( 1.0 - lfScreenVal ) > ( lfFadeValue + 2.0 ) )
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
        else
        {
            // Fade in
            if ( lfScreenVal > lfFadeValue )
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
    }
}

//-----------------------------------------------------------------------------
///
///     CalculateFadeValue
///
///     @brief      Hash
///
///     @param      vec2 lPositionVec2 
///     @return     float
///
//-----------------------------------------------------------------------------
void 
CheckHeightFade( 
    SAMPLER2DARG( lNoiseMap ),
    vec4    lScreenSpacePositionVec4,
    float   lfHeight,
    float   lfCommonFadeValue )
{
    vec2  lScreenVal  = ((lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w) * 0.5) + 0.5;
    float lfScreenVal = texture2D( lNoiseMap, lScreenVal*vec2(5.0,3.0) ).x;

    float lfFadeValue = lfCommonFadeValue * lfHeight;

    if( lfFadeValue < 0.0 )
    {
        // Fade out
        if( (1.0-lfScreenVal) > (lfFadeValue+2.0) )
        {
#if defined ( D_COMPUTE )
            return;
#else
            discard;
#endif
        }
    }
    else
    {
        // Fade in
        if( lfScreenVal > lfFadeValue )
        {
#if defined ( D_COMPUTE )
            return;
#else
            discard;
#endif
        }
    }
}


//-----------------------------------------------------------------------------
///
///     CheckFadePlanetSpaceCoord
///
///     @brief      Transform world space coordinate into planet relative position
///                 to create a pixellated fade between objects.
///
///     @param      
///     @return     void
///
//-----------------------------------------------------------------------------
void
CheckFadePlanetSpaceCoord(
    SAMPLER2DARG( lFadeNoiseMap ),
    vec3          lWorldPositionVec3,
    vec3          lPlanetCentreVec3,
    float         lfCommonFadeValue )
{
    if ( lfCommonFadeValue < 1.0 )
    {
        const float kPI = 3.141592653589793;

        // Translating to spherical coordinates...
        vec3 lPlanetRelativePositionVec3 = lWorldPositionVec3 - lPlanetCentreVec3;
        float lPlanetRadius = length( lPlanetRelativePositionVec3 );
        vec3 lPlanetNormalVec3 = lPlanetRelativePositionVec3 / lPlanetRadius;

        float lfTheta = cosh( lPlanetNormalVec3.z );
        float lfScale = lPlanetNormalVec3.x == 0.0 ? lPlanetNormalVec3.y : lPlanetNormalVec3.y / lPlanetNormalVec3.x;
        float lfPhi = tanh( lfScale );

        if ( lPlanetNormalVec3.x < 0.0 )
        {
            lfPhi += kPI;
        }

        vec2 lSphericalCoordsVec2 = vec2( lfTheta, lfPhi );

        vec2  lTextureCoordsVec2 = vec2( fract( lSphericalCoordsVec2 * lPlanetRadius ) );
        float lfNoiseValue = texture2DLod( lFadeNoiseMap, lTextureCoordsVec2, 0.0 ).x;

        if ( lfCommonFadeValue < 0.0 )
        {
            // Fade out
            if ( ( 1.0 - lfNoiseValue ) > lfCommonFadeValue + 2.0 )
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
        else
        {
            // Fade in
            if ( lfNoiseValue > lfCommonFadeValue )
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
    }
}



//-----------------------------------------------------------------------------
///
///     CheckFadePlanetSpaceCoord
///
///     @brief      Transform world space coordinate into planet relative position
///                 to create a pixellated fade between objects.
///
///     @param      
///     @return     void
///
//-----------------------------------------------------------------------------
void
CheckFadeWorldSpaceCoord(
    SAMPLER2DARG(lFadeNoiseMap),
    vec3          lWorldPositionVec3,
    float         lfCommonFadeValue)
{
    if (lfCommonFadeValue < 1.0)
    {
        vec3 p = floor(lWorldPositionVec3);
        vec3 f = fract(lWorldPositionVec3);
        f = f * f*(3.0 - 2.0*f);
        vec2 lTextureCoordsVec2 = (p.xy + vec2(37.0, 17.0)*p.z) + f.xy;
        //lTextureCoordsVec2 = (lTextureCoordsVec2 + vec2(0.5,0.5)) * vec2(1.0 / 256.0, 1.0 / 256.0);

        float lfNoiseValue = texture2DLod(lFadeNoiseMap, lTextureCoordsVec2, 0.0).x;

        if (lfCommonFadeValue < 0.0)
        {
            // Fade out
            if ((1.0 - lfNoiseValue) > lfCommonFadeValue + 2.0)
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
        else
        {
            // Fade in
            if (lfNoiseValue > lfCommonFadeValue)
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
    }
}

//-----------------------------------------------------------------------------
///
///     CheckFadeLocalSpaceWithOffsetCoord
///
//-----------------------------------------------------------------------------
void
CheckFadeLocalSpaceWithOffsetCoord(
    SAMPLER2DARG(lFadeNoiseMap),
    vec3          lLocalPositionVec3,
    vec3          lOffsetVec3,
    float         lfCommonFadeValue)
{
    if (lfCommonFadeValue < 1.0)
    {
        vec3 f = fract( lLocalPositionVec3 + fract( lOffsetVec3 ) );
        f = f * f*(3.0 - 2.0*f);
        vec2 lTextureCoordsVec2 = f.xy;

        float lfNoiseValue = texture2DLod(lFadeNoiseMap, lTextureCoordsVec2, 0.0).x;

        if (lfCommonFadeValue < 0.0)
        {
            // Fade out
            if ((1.0 - lfNoiseValue) > lfCommonFadeValue + 2.0)
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
        else
        {
            // Fade in
            if (lfNoiseValue > lfCommonFadeValue)
            {
#if defined ( D_COMPUTE )
                return;
#else
                discard;
#endif
            }
        }
    }
}


#endif
