////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonFog.h
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

#ifndef D_COMMONFOG_H
#define D_COMMONFOG_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonUniforms.shader.h"
#include "Common/CommonTriplanarTexturing.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Functions

//-----------------------------------------------------------------------------
///
///     Caustics
///
///     @brief      Caustics
///
///     @param      in vec3   lSkyColourVec3,   
///     @param      in vec3   lFragmentColourVec3
///     @param      in vec4   lFogVec4,
///     @param      in vec3   lCameraPositionVec3
///     @return     vec3
///
//-----------------------------------------------------------------------------

float
Caustics(
    in vec3   lWorldPositionVec3,
    in vec3   lLocalNormalVec3,
	in float  lfWaterHeight,
	in float  lfTime,
    SAMPLER2DARG( lTexture ),
    SAMPLER2DARG( lOffsetTexture ) )
{
	float  lfWaterDepth; 
	float  lfCausticStrength;
	vec3   lOutColVec3;
	vec3   lWaterPositionVec3;

    lfWaterDepth       = ( lfWaterHeight - length( lWorldPositionVec3 ) );
    lfCausticStrength  = 1.0;

    if( lfWaterDepth >= 0.0 )
	{
        lWaterPositionVec3  = normalize( lWorldPositionVec3 ) * lfWaterHeight;

        vec2 lAnimationVec2 = vec2( lfTime * 0.005 * 3.0 + 7.0, -lfTime * 0.005 * 3.0 );
        float lfOffset      = GetTriPlanarColourMM( lLocalNormalVec3, lWorldPositionVec3, lAnimationVec2, 0.01, SAMPLER2DPARAM( lOffsetTexture ) ).g;

        lAnimationVec2      = vec2( lfOffset, 1.0 - lfOffset ) * 0.06 + vec2( lfTime * 0.0025 * 3.0, lfTime * 0.0025 * 2.5 + 7.0 );
        lfCausticStrength   = GetTriPlanarColourMM( lLocalNormalVec3, lWaterPositionVec3, lAnimationVec2, 0.04, SAMPLER2DPARAM( lTexture ) ).g;

        lAnimationVec2      = vec2( lfOffset, 1.0 - lfOffset ) * 0.05 + vec2( lfTime * 0.0025 * 2.0 + 5.0, -lfTime * 0.0025 * 2.5 );
        lfCausticStrength  += GetTriPlanarColourMM( lLocalNormalVec3, lWaterPositionVec3, lAnimationVec2, 0.03, SAMPLER2DPARAM( lTexture ) ).g;
        lfCausticStrength   = lfCausticStrength * 1.3 + 0.6;
    }

    return lfCausticStrength;
}

#endif
