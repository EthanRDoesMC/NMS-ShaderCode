////////////////////////////////////////////////////////////////////////////////
///
///     @file       SSRFragment.shader.h
///     @author     strgiu
///     @date       
///
///     @brief      SSRFragmement
///
///     Copyright (c) 2021 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 
#ifndef SSR_SETTINGS_H
#define SSR_SETTINGS_H

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"

#if defined( D_SSR_SHADER )

#define D_OVERRIDE_ROUGHNESS_SCALE
#define D_LOCAL_REFLECTIONS
//#define D_DEBUG_PROBE_COVERAGE

// Debug Settings
STATIC_CONST float ROUGHNESS_SCALE_OVERRIDE         = 1.0;

// General Settings
STATIC_CONST float MIRROR_THRESHOLD                 = 0.2;

// Sampling settings
STATIC_CONST float GGX_BIAS_MIN                     = 0.5;
STATIC_CONST float GGX_BIAS_MAX                     = 1.0 - 0.5e-3;
STATIC_CONST float GGX_PDF_MAX                      = 1.0e4;
STATIC_CONST int   GGX_RUNS_MAX                     = 8;

// March Settings
STATIC_CONST float MAX_MARCH_DISTANCE               = 1.0e16;

// Hi-Z  Settings
STATIC_CONST int   HI_Z_MAX_STEPS                   = 400;
STATIC_CONST int   HI_Z_START_LEVEL                 = 0;
STATIC_CONST int   HI_Z_STOP_LEVEL                  = 0;
STATIC_CONST int   HI_Z_MAX_LEVEL                   = 6;
STATIC_CONST float HI_Z_THICKNESS_MIN               = 0.000025;
STATIC_CONST float HI_Z_THICKNESS_MAX               = 0.00025;
STATIC_CONST float HI_Z_THICKNESS_SCALE             = 2048.0;
STATIC_CONST float HI_Z_THICKNESS_ROUGH_BIAS        = 64.0;
#if defined( D_PLATFORM_PC )
STATIC_CONST float HI_Z_EPS                         = 0.0025;
#else
STATIC_CONST float HI_Z_EPS                         = 0.25;
#endif

// Radiance Settings
STATIC_CONST float SSR_STRENGTH_BIAS                = 0.125;
STATIC_CONST float LOCAL_REFLECTION_STRENGTH_BIAS   = 0.125;
STATIC_CONST float VISIBILITY_THRESHOLD             = 1.0 / 255.0;

// Mip Settings
STATIC_CONST float MIP_BIAS_FACTOR                  = 1.5;
STATIC_CONST float MIP_BIAS_FACTOR_LOCAL            = 1.5;
STATIC_CONST float MIP_BIAS_MAX                     = 8;
STATIC_CONST float MIP_BIAS_LOCAL_MAX               = 7;
STATIC_CONST float MIP_STRENGHT_BIAS                = 0.0;
STATIC_CONST float MIP_STRENGHT_BIAS_LOCAL          = 0.0;

// AABB Settings
STATIC_CONST float AABB_MIN_EXTENT                  = 1.0e-4;
STATIC_CONST float AABB_FADE_DIST                   = 8.0;

// Resolve Settings
STATIC_CONST int   RAY_REUSE_NUM_MAX                = 9;
STATIC_CONST float RAY_REUSE_RAD_MIN                = 2.5;
STATIC_CONST float RAY_REUSE_RAD_MAX                = 250.0;

// Temporal Settings
STATIC_CONST float VARIANCE_TOLERANCE               = 0.75;
STATIC_CONST float VARIANCE_HYSTERIS                = 0.10;
STATIC_CONST float VARIANCE_FACTOR                  = 4096.0 * 128.0;
STATIC_CONST float MOTION_HYSTERIS                  = 0.5;
STATIC_CONST float MOTION_TOLERANCE                 = 0.25;
STATIC_CONST float MOTION_THRESHOLD                 = 0.0015;
STATIC_CONST float SPEED_FACTOR                     = 256.0;

#endif // SSR_SHADER

#endif // SSR_SETTINGS_H