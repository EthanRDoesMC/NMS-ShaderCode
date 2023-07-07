////////////////////////////////////////////////////////////////////////////////
///
///     @file       PostCommon.h
///     @author     User
///     @date       
///
///     @brief      PostCommon
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 
#ifndef SSR_COMMON_H
#define SSR_COMMON_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"

#define D_MAX_PROBES_COUNT  8

//-----------------------------------------------------------------------------
///
///     CustomPerMeshUniforms
///
//-----------------------------------------------------------------------------

struct CustomPerMeshUniforms
{
    vec4 gCustomParamsVec4 MTL_ID(0);

#if defined( D_SSR_USES_PROBES )
    vec4 gaProbePositionsVec4[D_MAX_PROBES_COUNT];
    vec4 gaProbeExtentsVec4  [D_MAX_PROBES_COUNT];
    mat4 gaProbeMat4         [D_MAX_PROBES_COUNT];
#endif

BEGIN_SAMPLERBLOCK

    SAMPLER2D( gBufferMap  );
    SAMPLER2D( gBuffer1Map );
    SAMPLER2D( gBuffer2Map );
    SAMPLER2D( gBuffer3Map );
    SAMPLER2D( gBuffer4Map );
    SAMPLER2D( gBuffer5Map );
    SAMPLER2D( gBuffer6Map );
    SAMPLER2D( gBuffer7Map );
    SAMPLER2D( gBuffer8Map );

    SAMPLER2D(      gSobolMap           );
    SAMPLER2DARRAY( gRankArray0Map      );
    SAMPLER2DARRAY( gRankArray1Map      );
    SAMPLER2DARRAY( gRankArray2Map      );
    SAMPLER2DARRAY( gScrambleArray0Map  );
    SAMPLER2DARRAY( gScrambleArray1Map  );
    SAMPLER2DARRAY( gScrambleArray2Map  );

#if defined( D_SSR_USES_PROBES )
    SAMPLERCUBE( gProbe00Map );
    SAMPLERCUBE( gProbe01Map );
    SAMPLERCUBE( gProbe02Map );
    SAMPLERCUBE( gProbe03Map );
    SAMPLERCUBE( gProbe04Map );
    SAMPLERCUBE( gProbe05Map );
    SAMPLERCUBE( gProbe06Map );
    SAMPLERCUBE( gProbe07Map );
#endif

END_SAMPLERBLOCK

#ifndef D_COMPUTE

#define TEX_COORDS    IN(mTexCoordsVec2)
#define TEX_COORDS_HT IN(mTexCoordsVec2)

#elif defined(D_PLATFORM_METAL)
struct ComputeOutputUniforms
{
    vec4                 gOutTextureOffsetSize MTL_ID(0);
    RWIMAGE2D_INPUT(float4, gOutTexture0, 10);
    RWIMAGE2D_INPUT(float4, gOutTexture1, 11);
    RWIMAGE2D_INPUT(float4, gOutTexture2, 12);
    RWIMAGE2D_INPUT(float4, gOutTexture3, 13);
    RWIMAGE2D_INPUT(float4, gOutTexture4, 14);
    RWIMAGE2D_INPUT(float, gOutTextureDepth, 15);
};

#define TEX_COORDS      ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define TEX_COORDS_HT   ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

#elif defined D_PLATFORM_ORBIS

//-----------------------------------------------------------------------------
///
///     ComputeOutputUniforms
///
///     @brief      ComputeOutputUniforms
///
///     Refs to output textures for compute quad shaders
//-----------------------------------------------------------------------------

struct ComputeOutputUniforms
{
    vec4                 gOutTextureOffsetSize MTL_ID(0);
    RW_Texture2D<float4> gOutTexture0;
    RW_Texture2D<float4> gOutTexture1; 
    RW_Texture2D<float4> gOutTexture2; 
    RW_Texture2D<float4> gOutTexture3; 
    RW_Texture2D<float4> gOutTexture4; 
    RW_Texture2D<float>  gOutTextureDepth;
};

#define TEX_COORDS      ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define TEX_COORDS_HT   ( ( vec2( dispatchThreadID.xy ) + vec2(lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.xy) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )

#elif defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS )

struct ComputeOutputUniforms
{
    vec4               gOutTextureOffsetSize;
    RWIMAGE2D(rgba32f, gOutTexture0);
    RWIMAGE2D(rgba32f, gOutTexture1);
    RWIMAGE2D(rgba32f, gOutTexture2);
    RWIMAGE2D(rgba32f, gOutTexture3);
    RWIMAGE2D(rgba32f, gOutTexture4);
    RWIMAGE2D(r32f, gOutTextureDepth);
};

#define TEX_COORDS ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) * vec2( lUniforms.mpCmpOutPerMesh.gOutTextureOffsetSize.zw ) )
#define TEX_COORDS_HT ( ( vec2( dispatchThreadID.xy ) ) / vec2( GetImgResolution( lUniforms.mpCmpOutPerMesh.gOutTexture0 ) ) )

#elif defined D_PLATFORM_GLSL

    RWIMAGE2D( rgba32f, gOutTexture0 ); 
    RWIMAGE2D( rgba32f, gOutTexture1 ); 
    RWIMAGE2D( rgba32f, gOutTexture2 ); 
    RWIMAGE2D( rgba32f, gOutTexture3 ); 
    RWIMAGE2D( rgba32f, gOutTexture4 ); 
    RWIMAGE2D( r32f,    gOutTextureDepth );

#define TEX_COORDS      ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )
#define TEX_COORDS_HT   ( ( vec2( dispatchThreadID.xy ) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#else

RW_Texture2D<float4> gOutTexture0       : register(u0);
RW_Texture2D<float4> gOutTexture1       : register(u1);
RW_Texture2D<float4> gOutTexture2       : register(u2);
RW_Texture2D<float4> gOutTexture3       : register(u3);
RW_Texture2D<float4> gOutTexture4       : register(u4);
RW_Texture2D<float>  gOutTextureDepth   : register(u5);

#define TEX_COORDS      ( ( vec2( dispatchThreadID.xy ) + vec2(0.5,0.5) ) / vec2( GetImgResolution( gOutTexture0 ) ) )
#define TEX_COORDS_HT   ( ( vec2( dispatchThreadID.xy ) ) / vec2( GetImgResolution( gOutTexture0 ) ) )

#endif

//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )            /*: PER_MESH*/
     DECLARE_PTR( CustomPerMeshUniforms,        mpCustomPerMesh )       /*: PER_MESH*/
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )       /*: PER_MESH*/

#if defined( D_COMPUTE ) && ( defined( D_PLATFORM_ORBIS ) || ( defined( D_PLATFORM_SWITCH ) && !defined( D_SWITCH_NO_BINDLESS_SAMPLERS ) ) || defined( D_PLATFORM_METAL ) )
     DECLARE_PTR( ComputeOutputUniforms,        mpCmpOutPerMesh )   /* hack - marked 'per mesh' so it'll be alloced in cmd buf */
#endif
DECLARE_UNIFORMS_END


#define PROBE_PARAMS_SRT        SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe00Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe01Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe02Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe03Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe04Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe05Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe06Map ),\
                                SAMPLERCUBEPARAM_SRT( lUniforms.mpCustomPerMesh, gProbe07Map )

#define PROBE_PARAMS            SAMPLERCUBEPARAM( lProbe00Map ),\
                                SAMPLERCUBEPARAM( lProbe01Map ),\
                                SAMPLERCUBEPARAM( lProbe02Map ),\
                                SAMPLERCUBEPARAM( lProbe03Map ),\
                                SAMPLERCUBEPARAM( lProbe04Map ),\
                                SAMPLERCUBEPARAM( lProbe05Map ),\
                                SAMPLERCUBEPARAM( lProbe06Map ),\
                                SAMPLERCUBEPARAM( lProbe07Map )

#define PROBE_ARGS              SAMPLERCUBEARG(   lProbe00Map ),\
                                SAMPLERCUBEARG(   lProbe01Map ),\
                                SAMPLERCUBEARG(   lProbe02Map ),\
                                SAMPLERCUBEARG(   lProbe03Map ),\
                                SAMPLERCUBEARG(   lProbe04Map ),\
                                SAMPLERCUBEARG(   lProbe05Map ),\
                                SAMPLERCUBEARG(   lProbe06Map ),\
                                SAMPLERCUBEARG(   lProbe07Map )


#define PROBE_SELECT( P0, P1, P2, P3, P4, P5, P6, P7 )                               \
                                                                                     \
                                if ( liIdx < 4 )                                     \
                                {                                                    \
                                    if ( liIdx < 2 ) lColourVec3 = liIdx == 0 ? P0 : \
                                                                                P1;  \
                                    else             lColourVec3 = liIdx == 2 ? P2 : \
                                                                                P3;  \
                                }                                                    \
                                else                                                 \
                                {                                                    \
                                    if ( liIdx < 6 ) lColourVec3 = liIdx == 4 ? P4 : \
                                                                                P5;  \
                                    else             lColourVec3 = liIdx == 6 ? P6 : \
                                                                                P7;  \
                                }

#endif // SSR_COMMON_H