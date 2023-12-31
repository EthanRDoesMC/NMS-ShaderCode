#version 440 core
#extension GL_NV_gpu_shader5 : require
#extension GL_ARB_separate_shader_objects : require

#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonNanoVg.shader.h"

DECLARE_INPUT
    INPUT( vec2, mkLocalPositionVec4  , 0  )            // Object space position
    INPUT( vec2, mkTexCoordsVec4      , 1  )
DECLARE_INPUT_END

DECLARE_OUTPUT
        // Projected space position
    OUTPUT( vec2, mTexCoordsVec2       , TEXCOORD0  )            // UVs
    OUTPUT( vec2, mPosVec2             , TEXCOORD1  )
DECLARE_OUTPUT_END

//uniform vec2 viewSize;
struct CommonPerMeshUniforms
{
    // mat4 gWorldViewProjectionMat4;
    vec2 viewSize;
    vec4 frag[UNIFORMARRAY_SIZE];

BEGIN_SAMPLERBLOCK
	SAMPLER2D( gDiffuseMap );
END_SAMPLERBLOCK
	
DECLARE_UNIFORMS
     DECLARE_PTR( CommonPerMeshUniforms, mpCommonPerMesh )       /*: PER_MESH*/
DECLARE_UNIFORMS_END
	
DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR
    OUTPUT_SCREEN_POSITION_REDECLARED
DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

VERTEX_MAIN_SRT
{
    OUT( mTexCoordsVec2 ) = IN( mkTexCoordsVec4 );
    OUT( mPosVec2 )       = IN( mkLocalPositionVec4 );

    SCREEN_POSITION       = vec4( 
                            2.0 * (  IN( mkLocalPositionVec4 ).x / lUniforms.mpCommonPerMesh.viewSize.x ) - 1.0, 
                            1.0 - 2.0 * ( IN( mkLocalPositionVec4 ).y / lUniforms.mpCommonPerMesh.viewSize.y ), 
                            0, 
                            1 );

}
