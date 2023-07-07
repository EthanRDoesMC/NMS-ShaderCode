//-----------------------------------------------------------------------------
//This is expected to be included from shaders, and basically pulls in all the
//terrain gen functionality into 1 big shared library that shaders can use
//-----------------------------------------------------------------------------
#ifndef TERRAIN_GEN_SHARED_H_
#define TERRAIN_GEN_SHARED_H_

#ifdef __cplusplus
#error "Should not be including this file from c++ - use the individual shared headers"
#endif

#include "Custom/TerrainGenShared_Core.cpp"
#include "Custom/TerrainGenShared_Noise.cpp"
#include "Custom/TerrainGenShared_RegionDecorator.cpp"
#include "Custom/TerrainGenShared_Spawn.cpp"
#include "Custom/TerrainGenShared_NoiseUniforms.h"
#include "Custom/TerrainGenShared_LargeNoise.cpp"

#endif //TERRAIN_GEN_SHARED_H_

