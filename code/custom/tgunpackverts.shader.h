////////////////////////////////////////////////////////////////////////////////
///
///     @file       ComputeNoise.h
///     @author     ccummings
///     @date       
///
///     @brief      Basic shader to execute a series of Noise3D samples
///
///     Copyright (c) 2016 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "Common/Defines.shader.h"
#include "Custom/TerrainGenCommon.h"


#ifndef D_PLATFORM_ORBIS	
    ROBUFF(sTerrainVertexPacked,    maPacked, 0);
    RWBUFF(sTerrainVertexUnpacked,  maUnpacked, 1);
#endif

// Entry point for the compute shader with semantics that define the 
// variations of the ID to determine the thread instance invoked.
COMPUTE_MAIN_UNIF( 64, 1, 1, cTkUnpackTerrainVertexComputeUniforms )
{
    //get index and bail if gone to far
	uint idx = dispatchThreadID.x;
	if (idx >= lUniforms.miNumElements)
		return;
     
    UnpackTerrainVertex(GETBUFF(maPacked)[idx],GETBUFF(maUnpacked)[idx]);

    GETBUFF(maUnpacked)[idx].mPosition = MUL(vec4(GETBUFF(maUnpacked)[idx].mPosition,1),lUniforms.mPositionTransform).xyz;
}
  