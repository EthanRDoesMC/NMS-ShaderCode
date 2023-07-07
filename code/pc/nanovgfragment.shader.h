#version 440 core
//
#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif

#extension GL_ARB_gpu_shader5 : enable

// #include "Common/CommonUniforms.shader.h"
in vec2 mTexCoordsVec2;
in vec2 mPosVec2;
// DECLARE_INPUT
//     INPUT( vec2,   mTexCoordsVec2,     TEXCOORD0 )
//     INPUT( vec2,   mPosVec2,            TEXCOORD1 )
// DECLARE_END


#define NANOVG_GL2          1
#define UNIFORMARRAY_SIZE   12
#define EDGE_AA             1


uniform vec4 frag[ UNIFORMARRAY_SIZE ];
uniform sampler2D gDiffuseMap;

#define scissorMat          mat3( frag[ 0 ].xyz, frag[ 1 ].xyz, frag[ 2 ].xyz )
#define paintMat            mat3( frag[ 3 ].xyz, frag[ 4 ].xyz, frag[ 5 ].xyz )
#define innerCol            frag[ 6 ]
#define outerCol            frag[ 7 ]
#define scissorExt          frag[ 8 ].xy
#define scissorScale        frag[ 8 ].zw
#define extent              frag[ 9 ].xy
#define radius              frag[ 9 ].z
#define feather             frag[ 9 ].w
#define strokeMult          frag[ 10 ].x
#define strokeThr           frag[ 10 ].y
#define texType             int( frag[ 10 ].z )
#define type                int( frag[ 10 ].w )
#define flip                frag[ 11 ].x


float 
sdroundrect(
    vec2 pt, 
    vec2 ext, 
    float rad ) 
{
    vec2 ext2 = ext - vec2(rad,rad);
    vec2 d = abs(pt) - ext2;
    return min( max( d.x, d.y ), 0.0 ) + length( max( d, 0.0 ) ) - rad;
}

// Scissoring
float 
scissorMask( 
    vec2 p ) 
{
    vec2 sc = ( abs( ( scissorMat * vec3( p, 1.0 ) ).xy ) - scissorExt );
    sc = vec2( 0.5,0.5 ) - sc * scissorScale;
    return clamp( sc.x, 0.0, 1.0 ) * clamp( sc.y, 0.0, 1.0 );
}

#ifdef EDGE_AA
// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
float strokeMask() 
{
    return min( 1.0, ( 1.0 - abs( mTexCoordsVec2.x * 2.0 - 1.0 ) ) * strokeMult ) * 
           min( 1.0, mTexCoordsVec2.y );
}
#endif


layout(location = 0) out vec4 out_color0;

void main()
//FRAGMENT_MAIN_COLOUR_SRT
{
   vec4 result = vec4( 0.5 );

   float scissor = scissorMask( mPosVec2 );

#ifdef EDGE_AA
    float strokeAlpha = strokeMask();
#else
    float strokeAlpha = 1.0;
#endif

    if ( type == 4 ) // Flat Fill
    {
        vec4 color = innerCol;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    }
    else if ( type == 1 ) // Image
    {
        // Calculate color from texture
        vec2 pt = (paintMat * vec3(mPosVec2,1.0)).xy / extent;
        pt.y  = mix( pt.y, 1.0 - pt.y, flip );
	
        vec4 color = texture( gDiffuseMap, pt );
		
        if ( texType == 1 )
		{
			color = vec4(color.xyz*color.w,color.w);
		}
		if ( texType == 2 ) 
		{
			color = vec4(color.x);        // Apply color tint and alpha.
		}
        color *= innerCol;
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    } 
    else if ( type == 3 ) // Textured tris
    {
        vec2 lTexCoords = mTexCoordsVec2;

        vec4 color = texture( gDiffuseMap, lTexCoords );

        if ( texType == 1 ) 
        {
            color = vec4( color.xyz * color.w,color.w );
        }
        if ( texType == 2 ) 
        {
            color = vec4( color.x );
        }
		
        color *= scissor;
        result = color * innerCol;
    }
    else if ( type == 0 ) // Gradient
    {
        // Calculate gradient color using box gradient
        vec2 pt = (paintMat * vec3(mPosVec2,1.0)).xy;
        float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);
        vec4 color = mix(innerCol,outerCol,d);
        // Combine alpha
        color *= strokeAlpha * scissor;
        result = color;
    }     
    else if ( type == 2 ) // Stencil fill
    {
        result = vec4( 1.0 );
    } 
#ifdef EDGE_AA
    if ( strokeAlpha < strokeThr ) 
    {
        discard;
    }
#endif

    out_color0 = result;

}