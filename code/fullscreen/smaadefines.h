////////////////////////////////////////////////////////////////////////////////
///
///     @file       SMAACommon.h
///     @author     strgiu
///     @date       
///
///     @brief      SMAADefines
///
///     Copyright (c) 2020 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Includes
#include "Common/Defines.shader.h"

//-----------------------------------------------------------------------------
// Porting Functions


// SamplerState LinearSampler{ Filter = MIN_MAG_LINEAR_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };
// SamplerState PointSampler{  Filter = MIN_MAG_MIP_POINT; AddressU = Clamp; AddressV = Clamp; };

#define SMAATexture2DArg(NAME)                          SAMPLER2DARG(NAME)
#define SMAATexture2DParam(NAME)                        SAMPLER2DPARAM(NAME)
#define SMAATexture2DParamStr(STRUCTURE, NAME)          SAMPLER2DPARAM_SRT(STRUCTURE, NAME)
#define SMAASampleLevelZero(tex, coord)                 texture2DLod(tex, coord, 0.0)
#define SMAASampleLevelZeroPoint(tex, coord)            texture2DLod(tex, coord, 0.0)
#define SMAASample(tex, coord)                          texture2D(tex, coord)
#define SMAASamplePoint(tex, coord)                     texture2D(tex, coord)
#define SMAASampleOffset(tex, coord, offset)            texture2DOffset(tex, coord, offset)
#define SMAASampleLevelZeroOffset(tex, coord, offset)   texture2DLodOffset(tex, coord, 0.0, offset)
#define SMAALerp(a, b, t)                               mix(a, b, t)
#define SMAASaturate(a)                                 saturate(a)
#define SMAAGather(tex, coord)                          textureGatherRed(tex, coord)
#define SMAAMad(a, b, c)                                (a * b + c)
//#define SMAAMad(a, b, c) mad(a, b, c) // HLSL
//#define SMAAMad(a, b, c) fma(a, b, c) // GLSL
#define SMAA_FLATTEN //[flatten]
#define SMAA_BRANCH  //[branch]


//-----------------------------------------------------------------------------
  // SMAA Presets

  /**
   * Note that if you use one of these presets, the corresponding macros below
   * won't be used.
   */

#if defined( D_SMAA_MORPHOLOGICAL )

#define SMAA_PRESET_ULTRA

#if defined( SMAA_PRESET_LOW )
#define SMAA_THRESHOLD 0.15
#define SMAA_MAX_SEARCH_STEPS 4
#define SMAA_MAX_SEARCH_STEPS_DIAG 0
#define SMAA_CORNER_ROUNDING 100
#elif defined( SMAA_PRESET_MEDIUM )
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 8
#define SMAA_MAX_SEARCH_STEPS_DIAG 0
#define SMAA_CORNER_ROUNDING 100
#elif defined( SMAA_PRESET_HIGH )
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 16
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#define SMAA_CORNER_ROUNDING 25
#elif defined( SMAA_PRESET_ULTRA )
#define SMAA_THRESHOLD 0.05
#define SMAA_MAX_SEARCH_STEPS 32
#define SMAA_MAX_SEARCH_STEPS_DIAG 16
#define SMAA_CORNER_ROUNDING 25
#elif defined( SMAA_PRESET_CUSTOM )
#define SMAA_THRESHOLD 0.01
#define SMAA_MAX_SEARCH_STEPS 48
#define SMAA_MAX_SEARCH_STEPS_DIAG 24
#define SMAA_CORNER_ROUNDING 99
#endif

//-----------------------------------------------------------------------------
// Configurable Defines

/**
 * SMAA_THRESHOLD specifies the threshold or sensitivity to edges.
 * Lowering this value you will be able to detect more edges at the expense of
 * performance.
 *
 * Range: [0, 0.5]
 *   0.1 is a reasonable value, and allows to catch most visible edges.
 *   0.05 is a rather overkill value, that allows to catch 'em all.
 *
 *   If temporal supersampling is used, 0.2 could be a reasonable value, as low
 *   contrast edges are properly filtered by just 2x.
 */
#ifndef SMAA_THRESHOLD
#define SMAA_THRESHOLD 0.1
#endif

/**
 * SMAA_DEPTH_THRESHOLD specifies the threshold for depth edge detection.
 *
 * Range: depends on the depth range of the scene.
 */
#ifndef SMAA_DEPTH_THRESHOLD
#define SMAA_DEPTH_THRESHOLD (0.1 * SMAA_THRESHOLD)
#endif

/**
 * SMAA_MAX_SEARCH_STEPS specifies the maximum steps performed in the
 * horizontal/vertical pattern searches, at each side of the pixel.
 *
 * In number of pixels, it's actually the double. So the maximum line length
 * perfectly handled by, for example 16, is 64 (by perfectly, we meant that
 * longer lines won't look as good, but still antialiased).
 *
 * Range: [0, 98]
 */
#ifndef SMAA_MAX_SEARCH_STEPS
#define SMAA_MAX_SEARCH_STEPS 16
#endif

/**
 * SMAA_MAX_SEARCH_STEPS_DIAG specifies the maximum steps performed in the
 * diagonal pattern searches, at each side of the pixel. In this case we jump
 * one pixel at time, instead of two.
 *
 * Range: [0, 20]; set it to 0 to disable diagonal processing.
 *
 * On high-end machines it is cheap (between a 0.8x and 0.9x slower for 16
 * steps), but it can have a significant impact on older machines.
 */
#ifndef SMAA_MAX_SEARCH_STEPS_DIAG
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#endif

/**
 * SMAA_CORNER_ROUNDING specifies how much sharp corners will be rounded.
 *
 * Range: [0, 100]; set it to 100 to disable corner detection.
 */
#ifndef SMAA_CORNER_ROUNDING
#define SMAA_CORNER_ROUNDING 25
#endif

/**
 * Predicated thresholding allows to better preserve texture details and to
 * improve performance, by decreasing the number of detected edges using an
 * additional buffer like the light accumulation buffer, object ids or even the
 * depth buffer (the depth buffer usage may be limited to indoor or short range
 * scenes).
 *
 * It locally decreases the luma or color threshold if an edge is found in an
 * additional buffer (so the global threshold can be higher).
 *
 * This method was developed by Playstation EDGE MLAA team, and used in
 * Killzone 3, by using the light accumulation buffer. More information here:
 *     http://iryoku.com/aacourse/downloads/06-MLAA-on-PS3.pptx
 */
#ifndef SMAA_PREDICATION
#define SMAA_PREDICATION 0
#endif

/**
 * Threshold to be used in the additional predication buffer.
 *
 * Range: depends on the input, so you'll have to find the magic number that
 * works for you.
 */
#ifndef SMAA_PREDICATION_THRESHOLD
#define SMAA_PREDICATION_THRESHOLD 0.01
#endif

/**
 * How much to scale the global threshold used for luma or color edge
 * detection when using predication.
 *
 * Range: [1, 5]
 */
#ifndef SMAA_PREDICATION_SCALE
#define SMAA_PREDICATION_SCALE 2.0
#endif

/**
 * How much to locally decrease the threshold.
 *
 * Range: [0, 1]
 */
#ifndef SMAA_PREDICATION_STRENGTH
#define SMAA_PREDICATION_STRENGTH 0.4
#endif

/**
 * Temporal reprojection allows to remove ghosting artifacts when using
 * temporal supersampling. We use the CryEngine 3 method which also introduces
 * velocity weighting. This feature is of extreme importance for totally
 * removing ghosting. More information here:
 *    http://iryoku.com/aacourse/downloads/13-Anti-Aliasing-Methods-in-CryENGINE-3.pdf
 *
 * Note that you'll need to setup a velocity buffer for enabling reprojection.
 * For static geometry, saving the previous depth buffer is a viable
 * alternative.
 */
#ifndef SMAA_REPROJECTION
#define SMAA_REPROJECTION 0
#endif

/**
 * SMAA_REPROJECTION_WEIGHT_SCALE controls the velocity weighting. It allows to
 * remove ghosting trails behind the moving object, which are not removed by
 * just using reprojection. Using low values will exhibit ghosting, while using
 * high values will disable temporal supersampling under motion.
 *
 * Behind the scenes, velocity weighting removes temporal supersampling when
 * the velocity of the subsamples differs (meaning they are different objects).
 *
 * Range: [0, 80]
 */
#define SMAA_REPROJECTION_WEIGHT_SCALE 30.0

              /**
               * In the last pass we leverage bilinear filtering to avoid some lerps.
               * However, bilinear filtering is done in gamma space in DX9, under DX9
               * hardware (but not in DX9 code running on DX10 hardware), which gives
               * inaccurate results.
               *
               * So, if you are in DX9, under DX9 hardware, and do you want accurate linear
               * blending, you must set this flag to 1.
               *
               * It's ignored when using SMAA_HLSL_4, and of course, only has sense when
               * using sRGB read and writes on the last pass.
               */
#ifndef SMAA_DIRECTX9_LINEAR_BLEND
#define SMAA_DIRECTX9_LINEAR_BLEND 0
#endif

               /**
                * On ATI compilers, discard cannot be used in vertex shaders. Thus, they need
                * to be compiled separately. These macros allow to easily accomplish it.
                */
#ifndef SMAA_ONLY_COMPILE_VS
#define SMAA_ONLY_COMPILE_VS 0
#endif
#ifndef SMAA_ONLY_COMPILE_PS
#define SMAA_ONLY_COMPILE_PS 1
#endif

//-----------------------------------------------------------------------------
// Non-Configurable Defines

#ifndef SMAA_AREATEX_MAX_DISTANCE
#define SMAA_AREATEX_MAX_DISTANCE 16
#endif
#ifndef SMAA_AREATEX_MAX_DISTANCE_DIAG
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#endif
#define SMAA_AREATEX_PIXEL_SIZE  (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)

#endif // D_SMAA_MORPHOLOGICAL == 0

#if defined( D_SMAA_TEMPORAL )

#define SMAA_COLOUR_COHERENCE_FACTOR   0.4
#define SMAA_MOTION_COHERENCE_FACTOR   0.1
#define SMAA_DEPTHL_COHERENCE_FACTOR   1.0e-2

#endif