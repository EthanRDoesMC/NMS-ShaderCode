#define D_PARTICLE_UNIFORMS


#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonVertex.shader.h"
#include "Common/CommonDepth.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "Custom/TrailCommon.h"

STATIC_CONST float radius      = 1.0;
STATIC_CONST float invScrRatio = 1280.0 / 720.0;

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------

//ALEXCHECK - number of components in attributes
DECLARE_INPUT

	INPUT(  vec4, mkLocalPositionVec4, POSITION0 )

	INPUT( vec4, mkLocalNormalVec4,     TEXCOORD0     )
	INPUT( vec4, mkCustom1Vec4,			TEXCOORD1     )
	INPUT( vec4, mkCustom2Vec4,			TEXCOORD2     )

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------
DECLARE_OUTPUT

    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE
    OUTPUT( vec4, mfTexCoord_Diffuse_Distortion, TEXCOORD1 )
    OUTPUT( vec4, mfTexCoord_Alpha,              TEXCOORD2 )
    OUTPUT( float, mfFade,                       TEXCOORD3 )

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

vec4
GetLinePosition(
    vec4 lScreenPositionA,
    vec4 lScreenPositionB,
    vec2 lOffset )
{
    vec2 lineDirProj;

    //  line direction in screen space (perspective division required)
    lineDirProj = radius * normalize( lScreenPositionA.xy/lScreenPositionA.ww - lScreenPositionB.xy/lScreenPositionB.ww );

    // small trick to avoid inversed line condition when points are not on the same side of Z plane
    if( sign(lScreenPositionA.w) != sign(lScreenPositionB.w) )
        lineDirProj = -lineDirProj;
        
    vec4 vMVP     = lScreenPositionA;

    // offset position in screen space along line direction and orthogonal direction
    vMVP.xy += lineDirProj.xy							* lOffset.xx * vec2(1.0,invScrRatio);
    vMVP.xy += lineDirProj.yx	* vec2( 1.0, -1.0 )		* lOffset.yy * vec2(1.0,invScrRatio);

    return vMVP;
}

VERTEX_MAIN_SRT
{
    
    vec4 OffsetUV;
    vec2 lDistortionUVvec2;
    vec4 laPosition[3];
	vec4 laViewPosition[3];
    vec4 laScreenPosition[3];
    vec3 lTemp = vec3(0.0, 0.0, 0.0);
    
    laPosition[0] = IN( mkLocalPositionVec4 );
    laPosition[1] = IN(mkLocalNormalVec4);
    laPosition[2] = IN(mkCustom1Vec4);

	laScreenPosition[0] = MUL(lUniforms.mpPerFrame.gViewProjectionMat4, laPosition[0]);
	laScreenPosition[1] = MUL(lUniforms.mpPerFrame.gViewProjectionMat4, laPosition[1]);
	laScreenPosition[2] = MUL(lUniforms.mpPerFrame.gViewProjectionMat4, laPosition[2]);

    OffsetUV = IN( mkCustom2Vec4 );

    // scale
    OffsetUV.y *= ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, 0 ).x;

    vec4 lFinalScreenPosition;

    vec4 vMVP_A  = GetLinePosition( laScreenPosition[0], laScreenPosition[1], vec2( 0.0,  OffsetUV.y ) );
    vec4 vMVP_B  = GetLinePosition( laScreenPosition[0], laScreenPosition[2], vec2( 0.0, -OffsetUV.y ) );

    lFinalScreenPosition    = (vMVP_A + vMVP_B) * 0.5;

    vec2  lNormal        = ((vMVP_A.xy + vMVP_B.xy) * 0.5)  - laScreenPosition[0].xy;
    float lfNormalLength = length( lNormal );

    if( lfNormalLength > 0.0 )
    {
        lNormal *= (1.0f / lfNormalLength);
    }

    // Clamp size to minimum number of screenspace pixels.
    
    float lfMinSizeinScreenSpace = ( (( lUniforms.mpCustomPerMaterial.gMinPixelSize_Glow.x / lUniforms.mpPerFrame.gFrameBufferSizeVec4.y ) * lFinalScreenPosition.w) );
    float lfNormalInScreenSpace  = abs( OffsetUV.y );

    float lfScale = lfNormalInScreenSpace < lfMinSizeinScreenSpace  ? lfMinSizeinScreenSpace * 0.5 : lfNormalInScreenSpace * 0.5; // times half because it gets offset in both directions.
    lFinalScreenPosition.xy = laScreenPosition[0].xy + (lNormal * vec2(1.0,invScrRatio) * lfScale );

#if defined( _F14_UVSCROLL )
    // Alpha map do not scroll uvs. (This is in addition to whatever alpha is in the diffuse texture)
    OUT( mfTexCoord_Alpha ).xy = OffsetUV.zw;

    // Diffuse map scroll uvs.
    OffsetUV.zw += ( lUniforms.mpPerFrame.gfTime * lUniforms.mpCustomPerMaterial.gUvScrollStep_DistortionStep.xy );   
#endif    

#if defined( _F31_DISPLACEMENT )
    // Scroll distortion UVs independently of diffuse map ones.
    lDistortionUVvec2 = OffsetUV.zw;
    lDistortionUVvec2 += lUniforms.mpPerFrame.gfTime * lUniforms.mpCustomPerMaterial.gUvScrollStep_DistortionStep.zw;
    OUT( mfTexCoord_Diffuse_Distortion ).zw = lDistortionUVvec2;
#endif
    
    OUT( mfTexCoord_Diffuse_Distortion ).xy = OffsetUV.zw;

    float lfFade = ARRAY_LOOKUP_FP( lUniforms.mpCommonPerMesh, mpCommonPerMesh, gaParticleSizeAndRotationsVec4, 0 ).y;

    // Fade out the trail if you are near it, and if it is facing down the camera at
    lfFade *= saturate( (laScreenPosition[ 0 ].z * laScreenPosition[ 0 ].w) / 0.2 ); // these are tweaked values    	

    OUT( mfFade ) = lfFade;

    SCREEN_POSITION = lFinalScreenPosition;    
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
}
