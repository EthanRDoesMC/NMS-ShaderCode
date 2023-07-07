////////////////////////////////////////////////////////////////////////////////
///
///     @file       SMAAFragment.h
///     @author     strgiu
///     @date       
///
///     @brief      SMAAFragment
///
///     Copyright (c) 2020 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files
#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Fullscreen/SMAACommon.h"

#if defined( D_SMAA_MORPHOLOGICAL )
#include "Fullscreen/SMAAMorphological.h"
#endif

#if defined( D_SMAA_TEMPORAL )
#include "Fullscreen/SMAATemporal.h"
#endif

// =================================================================================================
//
// SMAA EDGE
//
// =================================================================================================

#ifdef D_SMAA_EDGE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

void 
GetSMAAEdgeOffsets(
    in    vec2 lFragCoords,
    in    vec2 lTexelSize,
    inout vec4 laOffsets[3])
{
    laOffsets[0] = lFragCoords.xyxy + lTexelSize.xyxy * vec4(-1.0, 0.0, 0.0, -1.0);
    laOffsets[1] = lFragCoords.xyxy + lTexelSize.xyxy * vec4( 1.0, 0.0, 0.0,  1.0);
    laOffsets[2] = lFragCoords.xyxy + lTexelSize.xyxy * vec4(-2.0, 0.0, 0.0, -2.0);
}



FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lFragCoords    = TEX_COORDS.xy;
    vec2 lTexelSize     = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    
    vec4 lFragColour;
    vec4 laOffsets[3];

    GetSMAAEdgeOffsets( lFragCoords, lTexelSize, laOffsets );

    lFragColour =
#ifdef D_SMAA_EDGE_LUMA
        SMAALumaEdgeDetectionPS(    
#elif defined(D_SMAA_EDGE_COLOUR)
        SMAAColorEdgeDetectionPS(
#elif defined(D_SMAA_EDGE_DEPTH)
        SMAADepthEdgeDetectionPS(
#else
        #error invalid smaa edge context        
#endif
            lFragCoords,
            lTexelSize,
            laOffsets,
            SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBufferMap ) ); // Colour Map in linear space

    FRAGMENT_COLOUR  = lFragColour;
}

#endif

// =================================================================================================
//
// SMAA BLEND COMPUTE
//
// =================================================================================================

#ifdef D_SMAA_BLEND_COMPUTE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

void 
GetSMAABlendComputeOffsets(
    in  vec2 lFragCoords,
    in  vec2 lTexelSize,
    out vec4 laOffsets[3])
{
    laOffsets[0] = lFragCoords.xyxy + lTexelSize.xyxy * vec4(-0.250, -0.125,  1.250, -0.125);
    laOffsets[1] = lFragCoords.xyxy + lTexelSize.xyxy * vec4(-0.125, -0.250, -0.125,  1.250);

    // And these for the searches, they indicate the ends of the loops:
    laOffsets[2] = vec4(laOffsets[0].xz, laOffsets[1].yw) +
                   vec4(-2.0, 2.0, -2.0, 2.0) *
                   lTexelSize.xxyy * float(SMAA_MAX_SEARCH_STEPS);
}

void
GetSMAASubsampleIndices(
    in  vec2  lDejittVec2,
    out ivec4 liSubsampleIndices )
{
    /*
    |  Camera Jitter   |  subsampleIndices  |
    +------------------+--------------------+    
    SMAA1
    | ( 0.000,  0.000) |  ivec4(0, 0, 0, 0) |
    +------------------+--------------------+
    SMAAT2x
    | ( 0.250, -0.250) |  ivec4(1, 1, 1, 0) |
    | (-0.250,  0.250) |  ivec4(2, 2, 2, 0) |
    +------------------+--------------------+
    SMAAT4x
    | ( 0.375, -0.125) |  ivec4(5, 3, 1, 3) |
    | (-0.125,  0.375) |  ivec4(4, 6, 2, 3) |    
    | ( 0.125, -0.375) |  ivec4(3, 5, 1, 4) |
    | (-0.375,  0.125) |  ivec4(6, 4, 2, 4) |
    */
    if ( abs( lDejittVec2.x ) == 0 )
    {
        liSubsampleIndices = ivec4( 0, 0, 0, 0 );        
    }
    else 
    if ( abs( lDejittVec2.x ) == 0.25 )
    {
        liSubsampleIndices = -lDejittVec2.x > 0.0 ?
                                ivec4( 1, 1, 1, 0 ) :
                                ivec4( 2, 2, 2, 0 );        
    }
    else
    if ( abs( lDejittVec2.x ) == 0.125 )
    {
        liSubsampleIndices = -lDejittVec2.x > 0.0 ?
                                ivec4( 3, 5, 1, 4 ) :
                                ivec4( 4, 6, 2, 3 );
    }
    else
    //if ( abs( lDejittVec2.x ) == 0.375 )
    {
        liSubsampleIndices = -lDejittVec2.x > 0.0 ?
                                ivec4( 5, 3, 1, 3 ) :
                                ivec4( 6, 4, 2, 4 );
    }
}


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoords    = TEX_COORDS.xy;
    vec2  lTextureSize   = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    vec2  lTexelSize     = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    
    vec4  lFragColour;
    vec4  laOffsets[3];
    ivec4 liSubsampleIndices;

    GetSMAABlendComputeOffsets( lFragCoords, lTexelSize, laOffsets );
    GetSMAASubsampleIndices( lUniforms.mpPerFrame.gDeJitterVec3.xy, liSubsampleIndices );

    lFragColour = SMAABlendingWeightCalculationPS(
                    lFragCoords,
                    lFragCoords * lTextureSize,
                    lTexelSize,
                    laOffsets,
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBufferMap ), // Edge   Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gAreaMap   ), // Area   Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gSearchMap ), // Search Map
                    liSubsampleIndices );

    FRAGMENT_COLOUR = lFragColour;
}

#endif



// =================================================================================================
//
// SMAA BLEND RESOLVE
//
// =================================================================================================

#ifdef D_SMAA_BLEND_RESOLVE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions 

void 
GetSMAABlendResolveOffsets(
    in  vec2 lFragCoords,
    in  vec2 lTexelSize,
    out vec4 laOffsets[2])
{
    laOffsets[0] = lFragCoords.xyxy + lTexelSize.xyxy * vec4(-1.0, 0.0, 0.0, -1.0);
    laOffsets[1] = lFragCoords.xyxy + lTexelSize.xyxy * vec4( 1.0, 0.0, 0.0,  1.0);
}

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoords    = TEX_COORDS.xy;
    vec2  lTextureSize   = lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
    vec2  lTexelSize     = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    
    vec4  lFragColour;
    vec4  laOffsets[2];
    ivec4 liSubsampleIndices;

    GetSMAABlendResolveOffsets( lFragCoords, lTexelSize, laOffsets );

    lFragColour = SMAANeighborhoodBlendingPS(
                    lFragCoords,                   
                    lTexelSize,
                    laOffsets,
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBufferMap  ),   // Colour Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer1Map ) ); // Blend  Map                    

    FRAGMENT_COLOUR  = lFragColour;
}

#endif


// =================================================================================================
//
// SMAA TEMPORAL SUPERSAMPLE
//
// =================================================================================================

#ifdef D_SMAA_TEMPORAL_SUPERSAMPLE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoords    = TEX_COORDS.xy;
    
    vec4  lFragColour;
    
    lFragColour = SMAATemporalSupersamplingPS(
                    lFragCoords,                                       
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBufferMap   ),   // ColourFrame_0 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer1Map  ),   // ColourFrame_1 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer2Map  ),   // ColourFrame_2 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer3Map  ),   // ColourFrame_3 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer4Map  ),   // ColourFrame_4 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer5Map  ),   // MotionFrame_0 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer6Map  ),   // MotionFrame_1 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer7Map  ),   // MotionFrame_2 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer8Map  ),   // MotionFrame_3 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer9Map  ),   // DepthLFrama_0 Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer10Map ) ); // DepthLFrama_4 Map

    FRAGMENT_COLOUR  = lFragColour;
}

#endif

// =================================================================================================
//
// SMAA TEMPORAL FILTER
//
// =================================================================================================

#ifdef D_SMAA_TEMPORAL_FILTER

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
DECLARE_INPUT_END


FRAGMENT_MAIN_COLOUR_SRT
{
    vec2  lFragCoords    = TEX_COORDS.xy;
    
    vec4  lFragColour;
    
    lFragColour = SMAATemporalFilteringPS(
                    lFragCoords,
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBufferMap  ),   // Colour Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBufferMap  ),   // Colour Map
                    SMAATexture2DParamStr( lUniforms.mpCustomPerMesh, gBuffer1Map ) ); // Blend  Map                    

    FRAGMENT_COLOUR  = lFragColour;
}

#endif