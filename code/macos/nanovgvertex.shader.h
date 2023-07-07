
#ifndef D_VERTEX
#define D_VERTEX
#endif

#include "Common/Defines.shader.h"
#include "Common/CommonNanoVg.shader.h"


DECLARE_INPUT

    INPUT( vec2, mkLocalPositionVec4  , POSITION0  )            // Object space position
    INPUT( vec2, mkTexCoordsVec4      , TEXCOORD0  )

DECLARE_INPUT_END


DECLARE_OUTPUT
        // Projected space position
    OUTPUT( vec4, Position, REG_POSITION )
    OUTPUT( vec2, mTexCoordsVec2       , TEXCOORD0  )            // UVs
    OUTPUT( vec2, mPosVec2             , TEXCOORD1  )

DECLARE_OUTPUT_END

 
//uniform vec2 viewSize;
struct CommonPerMeshUniforms
{
    // mat4 gWorldViewProjectionMat4 MTL_ID(0);
    vec2 viewSize;
    vec4 frag[ UNIFORMARRAY_SIZE ];

BEGIN_SAMPLERBLOCK

	SAMPLER2D( gDiffuseMap );

END_SAMPLERBLOCK
	
DECLARE_UNIFORMS
     DECLARE_PTR( CommonPerMeshUniforms, mpCommonPerMesh )       /*: PER_MESH*/
DECLARE_UNIFORMS_END
	
VERTEX_MAIN_SRT
{
    OUT( mTexCoordsVec2 ) = IN( mkTexCoordsVec4 );
    OUT( mPosVec2 )       = IN( mkLocalPositionVec4 );

    OUT( Position )       = vec4( 
                            2.0 * (  IN( mkLocalPositionVec4 ).x / lUniforms.mpCommonPerMesh.viewSize.x ) - 1.0, 
                            1.0 - 2.0 * ( IN( mkLocalPositionVec4 ).y / lUniforms.mpCommonPerMesh.viewSize.y ), 
                            0, 
                            1 );
}



