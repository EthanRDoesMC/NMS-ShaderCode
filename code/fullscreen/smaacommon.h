////////////////////////////////////////////////////////////////////////////////
///
///     @file       SMAACommon.h
///     @author     strgiu
///     @date       
///
///     @brief      SMAACommon
///
///     Copyright (c) 2020 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"

struct CustomPerMeshUniforms
{
    vec4 gDummyVec4 MTL_ID(0);

BEGIN_SAMPLERBLOCK

    SAMPLER2D( gBufferMap   );
    SAMPLER2D( gBuffer1Map  );
    SAMPLER2D( gBuffer2Map  );
    SAMPLER2D( gBuffer3Map  );
    SAMPLER2D( gBuffer4Map  );
    SAMPLER2D( gBuffer5Map  );
    SAMPLER2D( gBuffer6Map  );
    SAMPLER2D( gBuffer7Map  );
    SAMPLER2D( gBuffer8Map  );
    SAMPLER2D( gBuffer9Map  );
    SAMPLER2D( gBuffer10Map );
    SAMPLER2D( gAreaMap     );
    SAMPLER2D( gSearchMap   );
    SAMPLER2D( gTest0Map    );
    SAMPLER2D( gTest1Map    );
    
END_SAMPLERBLOCK

#define TEX_COORDS    IN(mTexCoordsVec2)
#define TEX_COORDS_HT IN(mTexCoordsVec2)


//
// This is the SRT buffer that everything gets uploaded to (on PS4). PC just functions as normal.
//
DECLARE_UNIFORMS
     DECLARE_PTR( PerFrameUniforms,             mpPerFrame )            /*: PER_MESH*/
     DECLARE_PTR( CustomPerMeshUniforms,        mpCustomPerMesh )       /*: PER_MESH*/
     DECLARE_PTR( CommonPerMeshUniforms,        mpCommonPerMesh )       /*: PER_MESH*/

DECLARE_UNIFORMS_END