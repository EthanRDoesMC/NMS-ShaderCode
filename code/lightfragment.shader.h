////////////////////////////////////////////////////////////////////////////////
///
///     @file       LightFragment.h
///     @author     User
///     @date       
///
///     @brief      LightFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#if defined(D_VERTEX_SUNLIGHT)
#define D_VERTEX
#endif

#if !defined(D_VERTEX) && !defined(D_FRAGMENT)
#define D_FRAGMENT
#endif

#if defined( D_PLATFORM_ORBIS ) && defined( D_FRAGMENT )
#pragma argument (O4; fastmath; scheduler=minpressure)
#pragma argument(unrollallloops)
//#pragma argument (targetoccupancy_atallcosts=10)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma unroll all
//precision mediump float;
#pragma fastmath true
#pragma hoistDiscards true
#endif

// platforms here need to explicit, because this is before Defines.shader.h
#if defined( D_PLATFORM_PROSPERO ) || defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_XBOXGDK ) || defined( D_PLATFORM_DX12 ) || defined( D_PLATFORM_SWITCH )
#define D_USE_CROSS_LANE
#endif

//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"
#include "Common/CommonUniforms.shader.h"
#include "LightCommon.shader.h"
#include "Common/CommonGBuffer.shader.h"
#include "Common/ACES.shader.h"

#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
//precision mediump float;
#endif

#define CustomPerMaterialUniforms  CustomPerMeshUniforms 

#if defined ( D_PLATFORM_ORBIS )

#if defined( D_SPLIT_SHADOW ) && !defined( D_PLATFORM_ORBIS_COMPUTE)
#pragma argument(targetoccupancy_atallcosts=70)
#endif

#if defined( D_SUNLIGHT ) && !defined( D_PLATFORM_ORBIS_COMPUTE )
#pragma argument(targetoccupancy_atallcosts=70)
#endif

#if defined( D_SHADOW_APPLY ) && !defined( D_PLATFORM_ORBIS_COMPUTE )
#pragma argument(targetoccupancy_atallcosts=60)
#endif

#endif

#if defined(D_VERTEX) && !defined(D_VERTEX_SUNLIGHT) && !defined ( D_VERTEX_SSS )

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files


#include "Common/CommonDepth.shader.h"


//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
    INPUT(  vec3, mkLocalPositionVec3, POSITION0 )
DECLARE_INPUT_END

DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE

    #ifdef D_SPHERE
        OUTPUT( vec4, mPositionVec4, TEXCOORD0 )
        OUTPUT( vec4, mScreenSpacePositionVec4, TEXCOORD4 )
    #else
        OUTPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )
    #endif
DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Vertex Main
///
///     @brief      Vertex Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------

VERTEX_MAIN_SRT
{
#ifdef D_SPOTLIGHT
    vec4 lWorldPositionVec4       = MUL( lUniforms.mpCommonPerMesh.gWorldMat4, vec4(IN(mkLocalPositionVec3), 1.0) );
    vec4 lScreenSpacePositionVec4 = MUL( lUniforms.mpPerFrame.gViewProjectionMat4, lWorldPositionVec4 );
#else 
    vec4 lScreenSpacePositionVec4 = MUL( kFSQuadProj, vec4(IN(mkLocalPositionVec3), 1.0) );
#endif

    OUT(mScreenSpacePositionVec4) = lScreenSpacePositionVec4;

    SCREEN_POSITION = lScreenSpacePositionVec4;
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
#ifdef D_SPOTLIGHT   
    OUT(mScreenSpacePositionVec4).xy = vec2(0.5,0.5) * (OUT(mScreenSpacePositionVec4).xy + OUT(mScreenSpacePositionVec4).ww);
#ifndef D_PLATFORM_OPENGL
    OUT(mScreenSpacePositionVec4).y = (1.0 * OUT(mScreenSpacePositionVec4).w) - OUT(mScreenSpacePositionVec4).y;
#endif
#endif

}


#endif // D_VERTEX

#if defined( D_VERTEX ) && defined ( D_VERTEX_SUNLIGHT )

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonPlanet.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
    INPUT(  vec3, mkLocalPositionVec3, POSITION0 )
 	INPUT(  vec2, mkTexCoordsVec4,     TEXCOORD0 )    
DECLARE_INPUT_END

DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE
 	OUTPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
#if defined(D_PLATFORM_METAL)
    flat OUTPUT( vec3, mUpMatrix_col0,     TEXCOORD1 ) 
    flat OUTPUT( vec3, mUpMatrix_col1,     TEXCOORD2 ) 
    flat OUTPUT( vec3, mUpMatrix_col2,     TEXCOORD3 ) 	
#else
    flat OUTPUT( mat3, mUpMatrix,     TEXCOORD1 ) 	
#endif
DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Vertex Main
///
///     @brief      Vertex Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
VERTEX_MAIN_SRT
{   
    SCREEN_POSITION = MUL( kFSQuadProj, vec4( IN( mkLocalPositionVec3 ), 1.0 ) );
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);

    OUT( mTexCoordsVec2 ) = IN( mkTexCoordsVec4 );   
    #ifdef D_PLATFORM_METAL
    mat3 outputUp = GetInverseWorldUpTransform( lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4 );
    OUT(mUpMatrix_col0) = outputUp[0];
    OUT(mUpMatrix_col1) = outputUp[1];
    OUT(mUpMatrix_col2) = outputUp[2];
    #else
	OUT ( mUpMatrix )     = GetInverseWorldUpTransform( lUniforms.mpPerFrame.gViewPositionVec3, lUniforms.mpCommonPerMesh.gLightOriginVec4 );	
    #endif 
}


#endif // D_VERTEX_SUNLIGHT

#if defined ( D_VERTEX ) && defined( D_VERTEX_SSS )

//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#include "Common/CommonPlanet.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

//-----------------------------------------------------------------------------
///
///     Input
///
///     @brief  Input
///
//-----------------------------------------------------------------------------
DECLARE_INPUT
    INPUT(  vec3, mkLocalPositionVec3, POSITION0 )
    INPUT(  vec2, mkTexCoordsVec4,     TEXCOORD0 )    
DECLARE_INPUT_END

DECLARE_OUTPUT
    OUTPUT_SCREEN_POSITION
    OUTPUT_SCREEN_SLICE
    OUTPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )

DECLARE_OUTPUT_END

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR

    OUTPUT_SCREEN_POSITION_REDECLARED

DECLARE_OUTPUT_PER_VERTEX_DESCRIPTOR_END

//-----------------------------------------------------------------------------
///
///     Output
///
///     @brief  Output
///
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Vertex Main
///
///     @brief      Vertex Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
VERTEX_MAIN_SRT
{   
    SCREEN_POSITION = MUL( kFSQuadProj, vec4( IN( mkLocalPositionVec3 ), 1.0 ) );
    WRITE_SCREEN_SLICE(lUniforms.mpPerFrame.gVREyeInfoVec3.x);
    OUT( mTexCoordsVec2 ) = IN( mkTexCoordsVec4 ); 
}

#endif // D_VERTEX_SSS

#ifdef D_NULL

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions


//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main 
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
}

#endif // D_NULL

#if defined( D_LIGHT ) && !defined( D_LIGHT_SHAPE )

//-----------------------------------------------------------------------------
//      Includes

#ifdef D_WATER
    #include "Custom/WaterCommon.h"
#else
    #include "Common/CommonLighting.shader.h"
    #ifndef D_SPOTLIGHT
        #include "OutputPostProcess.shader.h"
    #endif
#endif
//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes  
DECLARE_INPUT
#if defined( D_SPOTLIGHT ) && !defined( D_SPOTLIGHT_MULTI )
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )
#else
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
#if !defined(D_SPOTLIGHT_MULTI)
    #if defined(D_PLATFORM_METAL)
        flat INPUT( vec3, mUpMatrix_col0,     TEXCOORD1 ) 
        flat INPUT( vec3, mUpMatrix_col1,     TEXCOORD2 ) 
        flat INPUT( vec3, mUpMatrix_col2,     TEXCOORD3 ) 		
    #else
        flat INPUT( mat3, mUpMatrix,     TEXCOORD1 ) 	
    #endif
#endif
#endif
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions


//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing. 
///
//-----------------------------------------------------------------------------

#if defined( D_SPOTLIGHT_OUTER )
FRAGMENT_MAIN_COLOUR_SRT
#elif defined( D_SUNLIGHT ) && defined( D_SPLIT_SHADOW )
FRAGMENT_MAIN_COLOUR01_SRT
#else
FRAGMENT_MAIN_COLOUR_SRT
#endif
{
    vec3 lFragmentColourVec3 = float2vec3(0.0);
    vec2 lFragCoordsVec2;
    vec3 lPositionVec3;
    uint liDepthMask;
    uint liMergedDepthMask;

#if defined( D_SPOTLIGHT ) && !defined( D_SPOTLIGHT_MULTI )

#ifndef D_PLATFORM_GLSL
    lFragCoordsVec2.xy = IN(mScreenPositionVec4).xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
#elif defined(D_PLATFORM_SWITCH)
    lFragCoordsVec2.xy = vec2(gl_FragCoord.xy) * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
#else
    lFragCoordsVec2 = IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w;
#endif

#else
    lFragCoordsVec2 = TEX_COORDS;
#endif

    vec4 lBuffer1_Vec4;
    lBuffer1_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer1Map, lFragCoordsVec2 );

    #ifdef D_COMPUTE
        if (lBuffer1_Vec4.x > 0.9999999)
        {
            WRITE_FRAGMENT_COLOUR( vec4( 0.0, 0.0, 0.0, 1.0 ) );
            return;
        }
    #endif

    #if defined( D_SPOTLIGHT_MULTI )
        bool lbNotSky = (lBuffer1_Vec4.x < 0.9999999);
    #if defined( D_PLATFORM_PROSPERO ) && !defined D_COMPUTE_DISABLEWAVE32
        if( wave32::ballot(lbNotSky) == 0)
    #elif defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_XBOXONE ) || defined( D_PLATFORM_SWITCH )
        if ( ballot(lbNotSky) == 0 )
    #endif
        {
            // All Sky, so no lighting!
            WRITE_FRAGMENT_COLOUR(vec4(0.0, 0.0, 0.0, 1.0));
            return;
        }
    #endif

    lPositionVec3 = DecodeGBufferPosition(lFragCoordsVec2,
        lUniforms.mpPerFrame.gClipPlanesVec4,
#if defined( D_SPOTLIGHT ) && !defined( D_SPOTLIGHT_MULTI )
        lUniforms.mpPerFrame.gInverseViewProjectionMat4,
        lUniforms.mpPerFrame.gInverseViewMat4,
        float2vec3( 0.0 ), 
        lBuffer1_Vec4,
        true
#else
        lUniforms.mpPerFrame.gInverseProjectionMat4,
        lUniforms.mpPerFrame.gInverseViewMat4,
        lUniforms.mpPerFrame.gViewPositionVec3,
        lBuffer1_Vec4,

        false
#endif
        );

    #ifdef D_SPOTLIGHT_MULTI

    lBuffer1_Vec4 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBuffer1Map, lFragCoordsVec2);
    {
        float lfDepth = FastDenormaliseDepth(lUniforms.mpPerFrame.gClipPlanesVec4, lBuffer1_Vec4.x);

        lfDepth = min(lfDepth, 427.0); // log max of 8.74
        lfDepth = max(0.0, log2(lfDepth) * 2.0 + 4.0);  // from 0 to 21.5

        liDepthMask = (1 << (uint(lfDepth) + 10));
#ifdef D_USE_CROSS_LANE
        liMergedDepthMask = CrossLaneOr(liDepthMask);
#else
        liMergedDepthMask = liDepthMask;
#endif
    }



    #if defined ( D_USE_CROSS_LANE )

    uvec2 liListBase = uvec2( IN_SCREEN_POSITION.xy ) / 2;
    liListBase /= 8;

#if defined ( D_PLATFORM_METAL )
    liListBase.x = min(liListBase.x, (lUniforms.mpPerFrame.gFrameBufferSizeVec4.z / 8) - 1);
    liListBase.y = min(liListBase.y, (lUniforms.mpPerFrame.gFrameBufferSizeVec4.w / 16) - 1);
#elif defined ( D_PLATFORM_SWITCH )
    liListBase.x = min(liListBase.x, (GetImgResolution(lUniforms.mpCustomPerMesh.gLightCluster).x / 8) - 1);
    liListBase.y = min(liListBase.y, (GetImgResolution(lUniforms.mpCustomPerMesh.gLightCluster).y / 16) - 1);
#else
    liListBase.x = min(liListBase.x, (GetImgResolution(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster)).x / 8) - 1);
    liListBase.y = min(liListBase.y, (GetImgResolution(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster)).y / 16) - 1);
#endif
    liListBase *= uvec2(8,16);

    uint liLaneInd = gl_SubgroupInvocationID;

    uvec2 liLightIn = liListBase;
    liLightIn.x += liLaneInd % 8;
    liLightIn.y += liLaneInd / 8;

    #if defined ( D_PLATFORM_METAL )
    uint numLights = bufferAtomicLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liListBaseIndex );
    #elif defined ( D_PLATFORM_SWITCH )
    ivec4 iLightRead = imageLoad( lUniforms.mpCustomPerMesh.gLightCluster, ivec2(liLightIn) );
    #else
    ivec4 iLightRead = imageLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), ivec2( liLightIn ) );
    #endif

    #if !defined ( D_PLATFORM_METAL )
    ivec4 numLights = ReadFirstLane( iLightRead );
    #endif

    #else

    uvec2 liListBase = uvec2( lFragCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) / 2;
    liListBase /= 8;
    
    //TF_BEGIN
    #ifdef D_PLATFORM_METAL
        liListBase.x = min(liListBase.x, (lUniforms.mpPerFrame.gFrameBufferSizeVec4.z / 8) - 1);
        liListBase.y = min(liListBase.y, (lUniforms.mpPerFrame.gFrameBufferSizeVec4.w / 16) - 1);
    #else
        liListBase.x = min(liListBase.x, (GetImgResolution(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster)).x / 8) - 1);
        liListBase.y = min(liListBase.y, (GetImgResolution(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster)).y / 16) - 1);
    #endif

    //TF_BEGIN converted to uvec2 to avoid implicit cast whic is illegal in metal
    //this not the right index with buffer changes but shader isn't used so at least let it compile.
    uint liListBaseIndex = liListBase.y * 8 + liListBase.x;
    liListBase *= uvec2(8,16);

    #ifdef D_PLATFORM_METAL
        uint numLights = bufferAtomicLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liListBaseIndex );
	#elif defined ( D_PLATFORM_SWITCH )
        uint numLights = imageLoad( lUniforms.mpCustomPerMesh.gOutTexture0, ivec2(liListBase) ).x;
    #else
        uint numLights = imageLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), ivec2( liListBase ) ).x;
    #endif //D_PLATFORM_METAL

    #endif // D_USE_CROSS_LANE

	#if defined ( D_PLATFORM_METAL )
    if( numLights == 0 )
	#else
    if( numLights.x == 0 )
	#endif
    {
        WRITE_FRAGMENT_COLOUR( vec4( lFragmentColourVec3, 1.0 ) ); 
        return;
    }

    vec4 lBuffer0_Vec4;
    vec4 lBuffer2_Vec4;
    vec4 lBuffer3_Vec4;
    lBuffer0_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBufferMap,  lFragCoordsVec2 );
    lBuffer2_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer2Map, lFragCoordsVec2 );
    lBuffer3_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer3Map, lFragCoordsVec2 );

    uint groupStart = 1;
	#if defined ( D_PLATFORM_METAL )
    uint groupEnd = numLights + 1;
	#else
    uint groupEnd = numLights.x + 1;
	#endif

    #ifdef D_USE_CROSS_LANE
    for( uint subGroupStart = 0; subGroupStart < 128; subGroupStart += gl_SubgroupSize)
    {
        if( subGroupStart != 0 )
        {
            liListBase.y += gl_SubgroupSize / 8;
            liLightIn = liListBase;
            liLightIn.x += liLaneInd % 8;
            liLightIn.y += liLaneInd / 8;

            #if defined ( D_PLATFORM_SWITCH )
            iLightRead = imageLoad( lUniforms.mpCustomPerMesh.gLightCluster, ivec2(liLightIn));
            #else
            iLightRead = imageLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), ivec2( liLightIn ) );
            #endif
        }
        groupStart = max( 1, subGroupStart );
        groupEnd = min( uint( numLights.x + 1), subGroupStart + gl_SubgroupSize );
    #endif

        for( uint iTileIndex = groupStart; iTileIndex < groupEnd; ++iTileIndex )
        {
            #ifdef D_USE_CROSS_LANE
                int iSpotIndex = ReadLaneDynamic( iLightRead.x, iTileIndex - subGroupStart );
            #else
                uint liLightIndex = liListBaseIndex;
                uvec2 liLightIn = liListBase;
                liLightIndex += iTileIndex;
                liLightIn.x += iTileIndex % 8;
                liLightIn.y += iTileIndex / 8;
                #ifdef D_PLATFORM_METAL
                    int iSpotIndex =  bufferAtomicLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ),liLightIndex );
                #else
                    int iSpotIndex =  imageLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), ivec2( liLightIn ) ).x;
                #endif
            #endif//D_USE_CROSS_LANE

            if( (iSpotIndex & liMergedDepthMask) == 0 ) continue;

            iSpotIndex = iSpotIndex & 0x3ff;

            // jic, skip over any lights indexed higher than the array can hold
            if( iSpotIndex >= D_MAX_LIGHT_COUNT ) continue;

            //iSpotIndex.x = iTileIndex;

    #endif // D_SPOTLIGHT_MULTI

    vec3  lLightColourVec3;
    vec3  lLightDirectionVec3;
    float lfAttenuation;
    float lfCutOff = 0.05;

    #ifdef D_SPOTLIGHT
    {       
        #ifdef D_SPOTLIGHT_MULTI            
            float lfLightIntensity       = lUniforms.mpCommonPerMesh.gLightColourMultiVec4[iSpotIndex].w;
            //float lfLightRadius        = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.w;
            float lfFalloffType          = lUniforms.mpCommonPerMesh.gSpotlightPositionMultiVec4[iSpotIndex].w;
            vec3 lLightPositionVec3      = lUniforms.mpCommonPerMesh.gSpotlightPositionMultiVec4[iSpotIndex].xyz;

            vec4 lSpotlightDirectionVec4 = lUniforms.mpCommonPerMesh.gSpotlightDirectionMultiVec4[iSpotIndex];
            vec4 lSpotlightUpVec4        = lUniforms.mpCommonPerMesh.gSpotlightUpMultiVec4[iSpotIndex];
        #else           
            float lfLightIntensity       = lUniforms.mpCommonPerMesh.gLightColourVec4.w;
            //float lfLightRadius        = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.w;
            float lfFalloffType          = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.w;
            vec3 lLightPositionVec3      = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.xyz;

            vec4 lSpotlightDirectionVec4 = lUniforms.mpCommonPerMesh.gSpotlightDirectionVec4;
            vec4 lSpotlightUpVec4 = lUniforms.mpCommonPerMesh.gSpotlightUpVec4;
        #endif
                           
        vec3 lPosToLight         = lLightPositionVec3 - lPositionVec3;

#if defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_PROSPERO )
        float lfCutOff      = 0.02;
        float lfOldCutOff   = 0.05;
#else
        float lfCutOff      = 0.05;
#endif

        if (lfFalloffType == 2.0)
        {
            // Quadratic Distance attenuation
            float lfDistanceSquared = dot(lPosToLight, lPosToLight);
            lfAttenuation = lfLightIntensity / max(1.0, lfDistanceSquared);
            if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
            {
#ifndef D_SPOTLIGHT_MULTI                
                discard;
#else
                continue;
#endif
            }
        }
        else if (lfFalloffType == 1.0)
        {
            // Linear Distance attenuation
            //float lfLightDistance = length(lPosToLight);
            //lfAttenuation = 1.0 / max(1.0, lfLightDistance);
            float lfDistanceSquared = dot(lPosToLight, lPosToLight);
            lfAttenuation = invsqrt(lfDistanceSquared);
            lfAttenuation = min( lfAttenuation, 1.0 );
            lfAttenuation *= lfLightIntensity;
            if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
            {
#ifndef D_SPOTLIGHT_MULTI                
                discard;
#else
                continue;
#endif
            }
        }
        else if (lfFalloffType == 0.0)
        {
            // Constant Distance attenuation
            lfAttenuation = lfLightIntensity;
        }
        else if (lfFalloffType == 1.5)
        {
            // Linear Mul Sqrt Distance attenuation
            float lfDistanceSquared = dot(lPosToLight, lPosToLight);
            lfAttenuation = invsqrt(lfDistanceSquared);
            lfAttenuation = lfAttenuation * sqrt(lfAttenuation);
            lfAttenuation = min( lfAttenuation, 1.0 );
            lfAttenuation *= lfLightIntensity;
            if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
            {
#ifndef D_SPOTLIGHT_MULTI                
                discard;
#else
                continue;
#endif
            }
        }
        else if (lfFalloffType == 3.0)
        {
            // Cubic Distance attenuation
            float lfDistanceSquared = dot(lPosToLight, lPosToLight);
            lfAttenuation = invsqrt(lfDistanceSquared) / lfDistanceSquared;
            lfAttenuation = min( lfAttenuation, 1.0 );
            lfAttenuation *= lfLightIntensity;
            if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
            {
#ifndef D_SPOTLIGHT_MULTI                
                discard;
#else
                continue;
#endif
            }
        }        
        else
        {
            // Custom Falloff Distance attenuation
            float lfDistanceSquared = dot(lPosToLight, lPosToLight);
            lfAttenuation = 1.0 / pow( lfDistanceSquared, 0.5 * lfFalloffType );
            lfAttenuation = min(lfAttenuation, 1.0);
            lfAttenuation *= lfLightIntensity;
            if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
            {
#ifndef D_SPOTLIGHT_MULTI                
                discard;
#else
                continue;
#endif
            }
        }

#ifdef D_SPOTLIGHT_MULTI
        lLightColourVec3 = lUniforms.mpCommonPerMesh.gLightColourMultiVec4[iSpotIndex].xyz * lUniforms.mpCommonPerMesh.gLightColourMultiVec4[iSpotIndex].w;
#else
        lLightColourVec3 = lUniforms.mpCommonPerMesh.gLightColourVec4.xyz * lUniforms.mpCommonPerMesh.gLightColourVec4.w;
#endif

#if defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_PROSPERO )
        lfAttenuation *= lfAttenuation <= lfOldCutOff / (1.0 - lfOldCutOff) ? lfAttenuation : 1.0;        
#endif

        lLightDirectionVec3 = normalize(lPosToLight);

        // Conelight falloff (this can only attenuate down)
        float lfLightFOV        = lSpotlightDirectionVec4.w;
        if (lfLightFOV > -2.0)
        {
            float lfCookieStrength = 1.0;
            float lfConeAngle = dot( lSpotlightDirectionVec4.xyz, -lLightDirectionVec3 );
            float lfConeAttenuation   = saturate( (lfConeAngle - lfLightFOV) * 5.0 );
            //lfConeAttenuation  *= lfConeAttenuation;                           
            lfAttenuation      *= lfConeAttenuation;                  
            if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
            {
#ifndef D_SPOTLIGHT_MULTI                
                discard;
#else
                continue;
#endif
            }

            float lfCookieIdx  = lSpotlightUpVec4.w;
            if (  lfCookieIdx >= 0.0 )            
            {
                vec3  lSpotAt       = lSpotlightDirectionVec4.xyz;
                vec3  lSpotUp       = lSpotlightUpVec4.xyz;
                vec3  lSpotRight    = cross( lSpotUp, lSpotAt );

                float lfConeRad     = abs( sin( acos( lfLightFOV ) ) );
                vec2  lConeUVs      = vec2( dot( -lLightDirectionVec3, lSpotRight ), -dot( -lLightDirectionVec3, lSpotUp ) );                
                lConeUVs           /= lfConeRad * 2.0; 
                lConeUVs           += 0.5;
                lfCookieStrength    = texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCookiesMap ), vec3( lConeUVs, lfCookieIdx ) ).r;
                lLightColourVec3 *= lfCookieStrength;
            }
        }

        lfAttenuation = (lfAttenuation - lfCutOff) / (1.0 - lfCutOff);
        lfAttenuation = max(lfAttenuation, 0.0);   
        #ifndef D_SPOTLIGHT_MULTI             
        lPositionVec3 += lUniforms.mpPerFrame.gViewPositionVec3;
        #endif

    } 
    #else
    {
        lLightColourVec3    = lUniforms.mpCommonPerMesh.gLightColourVec4.xyz * lUniforms.mpCommonPerMesh.gLightColourVec4.w;
        lLightDirectionVec3 = -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz;
        lfAttenuation       = 1.0;
    }
    #endif

    lLightColourVec3 = MUL( lLightColourVec3, sRGB_TO_P3D65 );

    #ifdef D_SPOTLIGHT_MULTI
    if (lfAttenuation > 0.0)
    {
    #endif

        vec3  lSunColourVec3 = float2vec3(0.0);
        vec3  lColourVec3;
        vec3  lNormalVec3;
        float lfMetallic;
        float lfRoughness;
        float lfGlow;
        int   liMaterialID;
        float lfSubsurface;

        #if defined( D_SUNLIGHT ) && defined( D_SHADOWED )
        float lfParallaxShadow;
        #endif

        {

        #ifndef D_SPOTLIGHT_MULTI
            vec4 lBuffer0_Vec4;
            vec4 lBuffer2_Vec4;
            vec4 lBuffer3_Vec4;
            lBuffer0_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBufferMap,  lFragCoordsVec2 );
            lBuffer2_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer2Map, lFragCoordsVec2 );
            lBuffer3_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer3Map, lFragCoordsVec2 );
        #endif

            DecodeGBuffer(
                lFragCoordsVec2,
                lUniforms.mpPerFrame.gClipPlanesVec4,
                lUniforms.mpPerFrame.gInverseProjectionMat4,
                lUniforms.mpPerFrame.gInverseViewMat4,
                lUniforms.mpPerFrame.gViewPositionVec3,
                lBuffer0_Vec4,
                lBuffer1_Vec4,
                lBuffer2_Vec4,
                lBuffer3_Vec4,
                false,
                true,
                lColourVec3,
                lPositionVec3,
                lNormalVec3,
                liMaterialID,
                lfRoughness,
                lfMetallic,
                lfSubsurface,
                lfGlow);
                lColourVec3 = MUL( lColourVec3, sRGB_TO_P3D65 );

                #if defined( D_SUNLIGHT ) && defined( D_SHADOWED )
                lfParallaxShadow = 1.0;

                    #if !defined( D_PLATFORM_SWITCH )
                    int liMaterialID_ = int( lBuffer2_Vec4.a * 3.0 ) << 8;
                    if ( ( liMaterialID_ & D_PARALLAX ) != 0 )
                    {
                        lfParallaxShadow -= lBuffer0_Vec4.a;
                    }
                    #endif

                #endif
        }

    #ifndef D_LIGHT_TERRAIN
        if ( ( liMaterialID & D_UNLIT ) != 0 )
        {
            lFragmentColourVec3 += lColourVec3 * lfAttenuation;
        }
        else
    #endif
        {
            mat3 lUpMatrix;
            #ifndef D_SPOTLIGHT
            #ifdef D_COMPUTE
            MAT3_SET_COLUMN(lUpMatrix, 0, lUniforms.mpPerFrame.gInverseWorldUpMatVec4[0].xyz);
            MAT3_SET_COLUMN(lUpMatrix, 1, lUniforms.mpPerFrame.gInverseWorldUpMatVec4[1].xyz);
            MAT3_SET_COLUMN(lUpMatrix, 2, lUniforms.mpPerFrame.gInverseWorldUpMatVec4[2].xyz);
            #else
        	    #ifdef D_PLATFORM_METAL
        	    lUpMatrix = mat3( IN(mUpMatrix_col0), IN(mUpMatrix_col1),IN(mUpMatrix_col2));   // GetInverseWorldUpTransform( lPerFrameUniforms.gViewPositionVec3, lMeshUniforms.gPlanetPositionVec4 ); 
        	    #else
                lUpMatrix = IN( mUpMatrix );   // GetInverseWorldUpTransform( lPerFrameUniforms.gViewPositionVec3, lMeshUniforms.gPlanetPositionVec4 ); 
        	    #endif
            #endif
            #endif

            #if !defined( D_PLATFORM_SWITCH )
            float lAO = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBuffer4Map, lFragCoordsVec2).x;
            #ifdef D_SPOTLIGHT
            lAO = mix(0.61, 1.0, lAO);
            #else
            lAO = mix(0.25, 1.0, lAO);
            #endif
            #endif


            #if !defined( D_PLATFORM_SWITCH )
            //
            // Wet effect for stuff when raining. This all needs a pass to tidy up/move 
            // params into data.
            //
            if ( lUniforms.mpCustomPerMesh.gRainParametersVec4.x > 0.0 )
            {             
                // variety based on roughness
                //lfRoughness = 0.0;
                float lfOriginalRoughness   = lfRoughness;
                float lfShininess           = 1.0 - lfRoughness;
                //if (lfShininess > 0.22)
                {
                   lfShininess = saturate( lfShininess * 3.0 );
                }
                
                // wet sides but not top or bottom
                vec3    lUp         = GetWorldUp( lPositionVec3, lUniforms.mpCommonPerMesh.gPlanetPositionVec4 );
                float   lfDot       = ( dot ( lUp, lNormalVec3 ) );
                float   lfClamp     = lfDot < 0.0 ? 1.0 : 0.7; // the clamp keeps a minimum shininess, which is less for downward facing normals
                lfDot               = abs( lfDot );
                //float   lfUpAmount  =lfDot;
                float   lfUpAmount  = clamp( lfDot * lfDot * lfDot, 0.0, lfClamp  );
                lfShininess        *= ( 1.0 - lfUpAmount );

                // convert back to roughness
                lfRoughness = 1.0 - lfShininess;

                // Blend between material's roughness and new wet roughness
                float lfMaxWetness = lUniforms.mpCustomPerMesh.gRainParametersVec4.x;// * lUniforms.mpCommonPerMesh.gWetnessParamsVec4.x;
                lfRoughness = mix( lfOriginalRoughness, lfRoughness, lfMaxWetness );
            }
            #endif


            vec3 lFinalLightColourVec3 = ComputeLightColour(
                DEREF_PTR( lUniforms.mpPerFrame ),
                DEREF_PTR( lUniforms.mpCommonPerMesh ),
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gShadowMap),
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gCloudShadowMap),
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gDualPMapBack),
                SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gDualPMapFront),
                lLightDirectionVec3,
                lLightColourVec3,
                lUniforms.mpCustomPerMesh.gLightTopColourVec4,
                lPositionVec3,
                lFragCoordsVec2,
                lNormalVec3,
                lColourVec3,
                lUpMatrix,
                liMaterialID,
                lfMetallic,
                lfRoughness,
                lfSubsurface,
                0.5,
                #if !defined( D_PLATFORM_SWITCH )
                lAO,
                #else
                1.0,
                #endif
                lSunColourVec3 );

            lFinalLightColourVec3 *= lfAttenuation;

            // this is kind of a hack
            // specular highlights are way too bright compared to other props
            // which causes them to look extremely blown out with the new bloom implementation
            // will need to find a better fix at some point :)
            lFinalLightColourVec3 = min( float2vec3( 1.15 ), lFinalLightColourVec3 );

            lFragmentColourVec3 += lFinalLightColourVec3;

           // lFragmentColourVec3 = vec3( lfRoughness, lfRoughness, lfRoughness ) * 0.5;
        }

        if ( ( liMaterialID & D_GLOW ) != 0 )
        {
            lfGlow = mix( lfGlow, lfGlow * sqrt( lfGlow ), saturate( lfGlow - 1.0 ) );
            lFragmentColourVec3 += lColourVec3 * lfGlow;
        }

    #ifdef D_SPOTLIGHT_MULTI
    }  // attenuation cutoff
    }  // D_SPOTLIGHT_MULTI loop

    #ifdef D_USE_CROSS_LANE
    }
    #endif
    #endif

#if !defined( D_SPOTLIGHT )
    if ( (liMaterialID & D_DISABLE_POSTPROCESS) == 0 )
    {
        vec4 lScreenSpacePos = vec4(lFragCoordsVec2.x, lFragCoordsVec2.y, 1.0, 1.0);
        lFragmentColourVec3 = PostProcess(
            DEREF_PTR( lUniforms.mpPerFrame ),
            DEREF_PTR( lUniforms.mpCommonPerMesh ),
            DEREF_PTR( lUniforms.mpCustomPerMesh ),
            vec4( lFragmentColourVec3, 1.0 ),
            lPositionVec3,
            lNormalVec3,
            liMaterialID,
            lScreenSpacePos,
            lSunColourVec3
            ).rgb;
    }
#endif

    // directly apply shadowed sunlight colour
#if defined( D_SUNLIGHT ) && defined( D_SHADOWED )

    float lfShadow;
    float lfCloudShadow;

    lfShadow             = ComputeShadowIntensity(
                            SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gShadowMap ),
                            DEREF_PTR( lUniforms.mpPerFrame ),
                            DEREF_PTR( lUniforms.mpCommonPerMesh ),
                            lPositionVec3,
                            float2vec3( 0.0 ),
                            lFragCoordsVec2,
                            true );

    lfCloudShadow        = ComputeCloudOverlay(
                            lFragCoordsVec2,
                            SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gCloudShadowMap));

    lfShadow            *= lfCloudShadow;
    lfShadow            *= lfParallaxShadow;

    lFragmentColourVec3 += lSunColourVec3 * lfShadow;
#endif

#if defined( D_SUNLIGHT ) && defined( D_SPLIT_SHADOW )
    WRITE_FRAGMENT_COLOUR0( vec4( lFragmentColourVec3, 1.0 ) );
    WRITE_FRAGMENT_COLOUR1( vec4( lSunColourVec3, 1.0 ) );
#else
    WRITE_FRAGMENT_COLOUR(  vec4( lFragmentColourVec3, 1.0 ) );
#endif
}

#endif // D_LIGHT && !D_LIGHT_SHAPE

#if defined( D_LIGHT_SHAPE )

//-----------------------------------------------------------------------------
//      Typedefs and Classes

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE

    INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions

FRAGMENT_MAIN_COLOUR_SRT
{
    FRAGMENT_COLOUR = vec4( lUniforms.mpCustomPerMesh.gDebugLightColourVec4 );
}
#endif

#ifdef D_TILE_VIS

//-----------------------------------------------------------------------------
//      Includes

    #include "Common/CommonLighting.shader.h"
//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes  
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
    #if defined(D_PLATFORM_METAL)
        flat INPUT( vec3, mUpMatrix_col0,     TEXCOORD1 ) 	
        flat INPUT( vec3, mUpMatrix_col1,     TEXCOORD2 ) 	
        flat INPUT( vec3, mUpMatrix_col2,     TEXCOORD3 ) 	
    #else
    //flat INPUT( mat3, mUpMatrix     , TEXCOORD1 )
    #endif
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions


//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing. 
///
//-----------------------------------------------------------------------------

FRAGMENT_MAIN_COLOUR_SRT
{
    vec3 lFragmentColourVec3 = float2vec3(0.0);
    vec2 lFragCoordsVec2;
    vec3 lPositionVec3;
    uint liDepthMask;
    uint liMergedDepthMask;

    lFragCoordsVec2 = TEX_COORDS;

    vec4 lBuffer1_Vec4;
    lBuffer1_Vec4 = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer1Map, lFragCoordsVec2 );
    {
        float lfDepth = FastDenormaliseDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lBuffer1_Vec4.x );

        lfDepth = min( lfDepth, 427.0 ); // log max of 8.74
        lfDepth = max( 0.0, log2( lfDepth ) * 2.0 + 4.0 );  // from 0 to 21.5

        liDepthMask = ( 1 << (uint(lfDepth) + 10) );
       // liMergedDepthMask = CrossLaneOr( liDepthMask );
        liMergedDepthMask = liDepthMask;
    }

    uvec2 liListBase = uvec2( lFragCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) / 2;
    uint liLighBaseIndex = liListBase.y * 8 + liListBase.x;

#ifdef D_PLATFORM_METAL
    ivec4 numLightsBase = bufferAtomicLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liLighBaseIndex);
#else
    liListBase /= 8;
    liListBase *= 8;
#if defined ( D_PLATFORM_SWITCH )
    ivec4 numLightsBase = imageLoad( lUniforms.mpCustomPerMesh.gLightCluster, ivec2(liListBase) );
#else
    ivec4 numLightsBase = imageLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), ivec2( liListBase ) );
#endif

#endif // D_PLATFORM_METAL
    int numLights = 0;

    for( int iTileIndex=1; iTileIndex<=numLightsBase.x; ++iTileIndex )
    {
        uvec2 liLightIn = liListBase;
        int liLightIndex = int(liLighBaseIndex) + iTileIndex;
        liLightIn.x += iTileIndex % 8;
        liLightIn.y += iTileIndex / 8;
    #ifdef D_PLATFORM_METAL
        int iSpotIndex = bufferAtomicLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liLightIndex );
    #else
#if defined ( D_PLATFORM_SWITCH )
        int iSpotIndex = imageLoad( lUniforms.mpCustomPerMesh.gLightCluster, ivec2(liLightIn) ).x;
#else
        int iSpotIndex = imageLoad( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), ivec2( liLightIn ) ).x;
#endif
    #endif
        if( (iSpotIndex & liDepthMask) != 0 ) numLights++;
    }

    vec4 colorPoints[6] = {
        vec4( 0, 0, 0, (1.0) ),
        vec4( 0, 0, 1, (2.0) ),
        vec4( 0, 1, 0, (8.0) ),
        vec4( 1, 1, 0, (16.0) ),
        vec4( 1, 0, 1, (32.0) ),
        vec4( 1, 1, 1, (64.0) )
    };

    vec3 outCol = vec3(0,0,0);

    for( int ii = 1; ii<6; ++ii )
    {
        if( float(numLights) < colorPoints[ii].w )
        {
            float lerpVal = saturate( (float(numLights) - colorPoints[ii-1].w) / (colorPoints[ii].w - colorPoints[ii-1].w) );
            lFragmentColourVec3 = mix(colorPoints[ii-1].xyz, colorPoints[ii].xyz, lerpVal);
            break;
        }
    }

    WRITE_FRAGMENT_COLOUR( vec4( lFragmentColourVec3, 0.3 ) ); 

    //FRAGMENT_COLOUR = vec4( lfRoughness * ( 1.0 / 16.0 ), saturate(lfMetallic), saturate(lfSubsurface), 1.0); 
    //FRAGMENT_COLOUR = vec4( 1.0, 0.0, 0.0, 1.0 );
}

#endif // D_TILE_VIS

#ifdef D_POSTPROCESS

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonLighting.shader.h"
#include "OutputPostProcess.shader.h"

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
//    Functions


//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    
    vec2 lFragCoordsVec2 = IN( mTexCoordsVec2 );

    //vec3  lFragmentColourVec3;
    vec3  lFragmentColourVec3;
    vec3  lColourVec3;
    vec3  lPositionVec3;
    vec3  lNormalVec3;
    float lfMetallic;
    float lfRoughness;            
    float lfGlow;            
    int   liMaterialID;            
    float lfSubsurface;

    {
        vec4 lBuffer0_Vec4;
        vec4 lBuffer1_Vec4;
        vec4 lBuffer2_Vec4;
        vec4 lBuffer3_Vec4;

        lBuffer0_Vec4 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBufferMap,  lFragCoordsVec2 );
        lBuffer1_Vec4 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBuffer1Map, lFragCoordsVec2 );
        lBuffer2_Vec4 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBuffer2Map, lFragCoordsVec2 );
        lBuffer3_Vec4 = READ_GBUFFER(lUniforms.mpCustomPerMesh, gBuffer3Map, lFragCoordsVec2 );

        DecodeGBuffer(
        	lFragCoordsVec2,
        	lUniforms.mpPerFrame.gClipPlanesVec4,
            lUniforms.mpPerFrame.gInverseProjectionMat4,
            lUniforms.mpPerFrame.gInverseViewMat4,
            lUniforms.mpPerFrame.gViewPositionVec3,
            lBuffer0_Vec4,
            lBuffer1_Vec4, 
            lBuffer2_Vec4,
            lBuffer3_Vec4,
            true,
            false,
            lColourVec3,
            lPositionVec3,
            lNormalVec3,
            liMaterialID,
            lfRoughness,
            lfMetallic,
            lfSubsurface,
            lfGlow );
            lColourVec3 = MUL( lColourVec3, sRGB_TO_P3D65 );
    }
    
    if ( (liMaterialID & D_DISABLE_POSTPROCESS) == 0 )
    {
        vec4 lColourVec4 = vec4( lColourVec3, 1.0 );
        vec4 lScreenSpacePos = vec4(lFragCoordsVec2.x, lFragCoordsVec2.y, 1.0, 1.0);
        lColourVec3 = PostProcess(
            DEREF_PTR( lUniforms.mpPerFrame ),
            DEREF_PTR( lUniforms.mpCommonPerMesh ),
            DEREF_PTR( lUniforms.mpCustomPerMesh ),
            lColourVec4,
            lPositionVec3,
            lNormalVec3,
            liMaterialID,
            lScreenSpacePos ).rgb;
    }

    //if( IN( mTexCoordsVec2 ).x < 0.5 )
    //{
    //    lColourVec3 = normalize( lPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3 ) * 0.5 + vec3( 0.5 );
    //}

 
    FRAGMENT_COLOUR = vec4( lColourVec3, /* lfGlow */ 1.0 );
}

#endif // D_POSTPROCESS

#ifdef D_SPOTLIGHT_CLEAR_LISTS

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonLighting.shader.h"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes  
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions


//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing. 
///
//-----------------------------------------------------------------------------

#ifdef D_PLATFORM_PC
FRAGMENT_MAIN_COLOUR_EARLYZ_SRT
#else
VOID_MAIN_COLOUR_EARLYZ_SRT
#endif
{    
    vec2 lFragCoordsVec2;

#ifndef D_PLATFORM_GLSL
    lFragCoordsVec2.xy = IN(mScreenPositionVec4).xy;
#else
    lFragCoordsVec2 = IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w;
    lFragCoordsVec2 *= lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
#endif

    ivec2 liFragCoords = ivec2( lFragCoordsVec2 );

    #ifdef D_PLATFORM_METAL
        int liListBaseIndex = liFragCoords.y * 8 + liFragCoords.x;
        bufferAtomicStore( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liListBaseIndex, 0 );
    #else
        ivec2 liListBase = liFragCoords * 8;
#if defined (D_PLATFORM_SWITCH )
    imageStore( lUniforms.mpCustomPerMesh. gOutTexture0, liListBase, 0 );

#else
    imageStore( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liListBase, 0 );
#endif

    #endif
    
    //

    #ifdef D_PLATFORM_PC
    FRAGMENT_COLOUR = vec4( 1.0 );
    #endif
}

#endif

#ifdef D_SPOTLIGHT_BUILD_LISTS

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonLighting.shader.h"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes  
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions


//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing. 
///
//-----------------------------------------------------------------------------

#ifdef D_PLATFORM_PC
FRAGMENT_MAIN_COLOUR_EARLYZ_SRT
#else
VOID_MAIN_COLOUR_EARLYZ_SRT
#endif
{    
    vec2 lFragCoordsVec2;
    float lfDepth;

#ifndef D_PLATFORM_GLSL
    lFragCoordsVec2.xy = IN(mScreenPositionVec4).xy;
    lfDepth = IN(mScreenPositionVec4).z;
#else
    lFragCoordsVec2 = IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w;
    lfDepth = IN(mScreenSpacePositionVec4).z / IN(mScreenSpacePositionVec4).w;
    lFragCoordsVec2 *= lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;
#endif

    ivec2 liFragCoords = ivec2( lFragCoordsVec2 );

    ivec2 liListBase = liFragCoords * ivec2(8,16);
    int luTileGlobalIndex = liFragCoords.y * D_TILE_WIDTH / 2 + liFragCoords.x;

    #ifdef D_PLATFORM_METAL
        int liIndex;
        bufferAtomicAddOut( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), luTileGlobalIndex, 1, liIndex );
    #else
        int liIndex = 0;
        #if defined ( D_PLATFORM_SWITCH )
        imageAtomicAddOut( lUniforms.mpCustomPerMesh.gLightCluster, liListBase, 1, liIndex );
        #else
        imageAtomicAddOut( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liListBase, 1, liIndex );
        #endif
    #endif

    if( liIndex < 126 )
    {
        liIndex += 1;
        ivec2 liLightOut = liListBase;
        liLightOut.x += liIndex % 8;
        liLightOut.y += liIndex / 8;
        luTileGlobalIndex += liIndex;

        uint liLightIndex = uint(lUniforms.mpCommonPerMesh.gUserDataVec4.x);

        // jic, skip over any lights indexed higher than the array can hold
        if( liLightIndex < D_MAX_LIGHT_COUNT ) 
        {

            // depth encoding
            lfDepth = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepth );

            #ifdef D_SPOTLIGHT_INNER            
            lfDepth = min( lfDepth, 427.0 ); // log max of 8.74
            lfDepth = max( 0.0, log2( lfDepth ) * 2.0 + 4.0 );  // from 0 to 21.49
            uint liDepthMask = ( 1 << (uint(lfDepth+1.0)) ) - 1;

            // merge in near depth from user data
            lfDepth = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lUniforms.mpCommonPerMesh.gUserDataVec4.z );
            lfDepth = min( lfDepth, 427.0 ); // log max of 8.74
            lfDepth = max( 0.0, log2( lfDepth ) * 2.0 + 4.0 );  // from 0 to 21.49
            uint liNearDepthMask = ( 1 << (uint(lfDepth-0.5)) ) - 1;
            liDepthMask &= ~liNearDepthMask;
            #else
            lfDepth = min( lfDepth, 427.0 ); // log max of 8.74
            lfDepth = max( 0.0, log2( lfDepth ) * 2.0 + 4.0 );  // from 0 to 21.49
            uint liDepthMask = ( 1 << (uint(lfDepth-0.5)) ) - 1;
            liDepthMask = ~liDepthMask;

            // merge in far depth from user data
            lfDepth = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lUniforms.mpCommonPerMesh.gUserDataVec4.y );
            lfDepth = min( lfDepth, 427.0 ); // log max of 8.74
            lfDepth = max( 0.0, log2( lfDepth ) * 2.0 + 4.0 );  // from 0 to 21.49
            uint liFarDepthMask = ( 1 << (uint(lfDepth+1.2)) ) - 1;
            liDepthMask &= liFarDepthMask;
            #endif


            // liDepthMask &= uint(lUniforms.mpCommonPerMesh.gUserDataVec4.y);
            liLightIndex |= liDepthMask << 10;
            #if defined ( D_PLATFORM_SWITCH )
            imageStore( lUniforms.mpCustomPerMesh.gLightCluster, liLightOut, ivec4(liLightIndex, 0, 0, 0));
            #elif defined( D_PLATFORM_GLSL )
                imageStore( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liLightOut, ivec4(liLightIndex,0,0,0) );
            #elif defined(D_PLATFORM_METAL)
                bufferAtomicStore( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), luTileGlobalIndex, int(liLightIndex) );
            #else //TF_END
                imageStore( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gLightCluster ), liLightOut, int(liLightIndex) );
            #endif
        }
    }

    #ifdef D_PLATFORM_PC
    FRAGMENT_COLOUR = vec4( 1.0 );
    #endif
}

#endif

#ifdef D_SSS_MARCH

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonLighting.shader.h"
#include "Common/CommonDepth.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

#if defined( D_SSS_MARCH_LOW )
STATIC_CONST float   kfRayRadius            = 0.5f;
STATIC_CONST float   kfFixedStepSizeUV      = 0.1f;
STATIC_CONST uint    kuNumberMinSteps       = 3u;
STATIC_CONST uint    kuNumberMaxSteps       = 8u;
#elif defined( D_SSS_MARCH_MID )
STATIC_CONST float   kfRayRadius            = 0.5f;
STATIC_CONST float   kfFixedStepSizeUV      = 0.025f;
STATIC_CONST uint    kuNumberMinSteps       = 4u;
STATIC_CONST uint    kuNumberMaxSteps       = 12u;
#elif defined( D_SSS_MARCH_HIGH )
STATIC_CONST float   kfRayRadius            = 1.0f;
STATIC_CONST float   kfFixedStepSizeUV      = 0.01f;
STATIC_CONST uint    kuNumberMinSteps       = 4u;
STATIC_CONST uint    kuNumberMaxSteps       = 16u;
#else
#error Unexpected SSS march setting
#endif

STATIC_CONST float   kfDistanceThreshold    = 1.0e-3;
STATIC_CONST float   kfDistanceFadeScale    = 1024.0 * 8.0;

STATIC_CONST float   kfThicknessMin         = 0.15;
STATIC_CONST float   kfThicknessMax         = 0.2;
STATIC_CONST float   kfThicknessScale       = 8.0;

STATIC_CONST float   kfDepthDiffMax         = 0.25;
STATIC_CONST float   kfDepthDiffScale       = 0.001;


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE
    INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions

float
InterleavedGradientNoise(
    uvec2   lvPos,
    int     liFrame)
{
    liFrame = liFrame & 0xff;
    float x = float( lvPos.x ) + 5.588238 * float( liFrame );
    float y = float( lvPos.y ) + 5.588238 * float( liFrame );
    return fract( 52.9829189 * fract( 0.06711056 * x + 0.00583715 * y ) );
}

vec2
GetUvCoordsFromView(
    in PerFrameUniforms lPerFrameUniforms,
    in vec3             lViewPosVec3 )
{
    vec4    lFragVec4   = vec4( lViewPosVec3, 1.0 );
    lFragVec4           = MUL( lPerFrameUniforms.gProjectionMat4, lFragVec4 );
    lFragVec4.xy       /= lFragVec4.w;
    lFragVec4.xy        = lFragVec4.xy * 0.5 + 0.5;
    lFragVec4.xy        = SCREENSPACE_AS_RENDERTARGET_UVS( lFragVec4.xy );

    return lFragVec4.xy;
}

mat3
asmat3(
    mat4 mat )
{
    #if defined( D_PLATFORM_PC )
    return mat3( mat );
	#elif defined ( D_PLATFORM_METAL )
	mat3 lOutput = { mat[0].xyz, mat[1].xyz, mat[2].xyz };
	return lOutput;
    #else
    return (mat3)mat;
    #endif
}

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lvUvCoords          = TEX_COORDS;
    float   lfDepthRevZ         = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvUvCoords, 0.0 ) );

    // return if sky or far away surfaces
    if ( lfDepthRevZ <= kfDistanceThreshold )
    {
        #if !defined( D_COMPUTE )
        discard;
        #else
        return;
        #endif
    }

    float   lfDepthLinear       = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepthRevZ );
    vec3    lViewPosVec3        = RecreateViewPositionFromRevZDepthSC( lfDepthRevZ, lvUvCoords, lUniforms.mpPerFrame.gInverseProjectionSCMat4 );
    vec3    lViewDirVec3        = MUL( asmat3( lUniforms.mpPerFrame.gViewMat4 ), normalize( -lUniforms.mpCommonPerMesh.gLightDirectionVec4.xyz ) );

    // Clamp the radius to avoid ending up with an end position that's behind the near plane
    float   lfRayRadius         = min( kfRayRadius, abs( lViewPosVec3.z / lViewDirVec3.z ) );
    vec3    lViewdRayEndPosVec3 = lViewPosVec3 + lViewDirVec3 * lfRayRadius;

    vec2    lTexSizeVec2        = vec2( GetTexResolution( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ) ) );
    float   lfAspectRatio       = lTexSizeVec2.x / lTexSizeVec2.y;

    // Transform start and end point to screen space.
    vec2    lvUvStart           = lvUvCoords;
    vec2    lvUvEnd             = GetUvCoordsFromView( DEREF_PTR( lUniforms.mpPerFrame ), lViewdRayEndPosVec3 );
    vec2    lvDelta             = lvUvEnd - lvUvStart;
    int     liDeltaIdx          = abs( lvDelta.x * lfAspectRatio ) >= abs( lvDelta.y ) ? 0 : 1;

    //Make ray step pixel size fixed
    vec2    lvFixedStepSizeUV   = float2vec2( kfFixedStepSizeUV ) * vec2( 1.0, lfAspectRatio );
    uint    liCount             = clamp( uint( abs( lvDelta[ liDeltaIdx ] ) / lvFixedStepSizeUV[ liDeltaIdx ] ), kuNumberMinSteps, kuNumberMaxSteps );
    vec2    lvIncrement         = lvDelta / liCount;
    float   lfDepthEndLinear    = -lViewdRayEndPosVec3.z;

    //Add dithered shift to prevent banding
    uvec2 lvPixCoords           = uvec2( lvUvCoords * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy );
    float lfDither              = InterleavedGradientNoise( lvPixCoords, lUniforms.mpPerFrame.giFrameIndex );
    vec2  lvMarchShift          = lvIncrement * lfDither;

    float lfOcclusion           = 0;
    vec2  lvUvCurr              = lvUvStart + lvMarchShift;
    float lfSearch              = lvMarchShift[ liDeltaIdx ] / lvDelta[ liDeltaIdx ];

    do
    {
        #ifdef D_PLATFORM_ORBIS
        float   lfDepthCurZ = DecodeDepthFromColour( texture2DArray( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), vec3( lvUvCurr, lUniforms.mpPerFrame.gVREyeInfoVec3.x ) ) );
        #else
        float   lfDepthCurZ = DecodeDepthFromColour( texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lvUvCurr, 0.0 ) );
        #endif
        float   lfDepthCurL = ReverseZToLinearDepth( lUniforms.mpPerFrame.gClipPlanesVec4, lfDepthCurZ );

        // Perspective correct mixing of linear depth from uv space interpolation parameter
        float   lfRayDepth  = ( lfDepthLinear * lfDepthEndLinear ) / mix( lfDepthEndLinear, lfDepthLinear, lfSearch );
        float   lfDepthDiff = lfRayDepth - lfDepthCurL;
        float   lfThickness = mix( kfThicknessMin, kfThicknessMax, saturate( lfRayDepth / kfThicknessScale ) );
        float   lfDepthLow  = min( kfDepthDiffMax, lfRayDepth * kfDepthDiffScale );
        bool    lbHit       = lfDepthDiff > lfDepthLow && lfDepthDiff < lfThickness;

        if ( lbHit )
        {
            lfOcclusion = 1.0 - lfSearch * lfSearch;
            break;
        }

        lvUvCurr += lvIncrement;
        lfSearch += 1.0 / liCount;
    }
    while( lfSearch <= 1.0 && lvUvCurr.x > 0.0 && lvUvCurr.y > 0.0 && lvUvCurr.x < 1.0 && lvUvCurr.y < 1.0 );

    // fade occlusion on far away surfaces
    lfOcclusion *= saturate( ( lfDepthRevZ - kfDistanceThreshold ) * kfDistanceFadeScale );

    WRITE_FRAGMENT_COLOUR( vec4( lfOcclusion, 0.0, 0.0, 1.0 ) );
}
#endif // D_SSS_MARCH

#ifdef D_SSS_TEMPORAL

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonLighting.shader.h"
#include "Common/CommonPostProcess.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE
    INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions
float sqr( float v ) { return v * v; }

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec2    lvUvCoords  = TEX_COORDS;
    
    // Get Motion
    vec3    lvEcdMotion = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lvUvCoords, 0.0 ).xyz;
    vec2    lvDcdMotion = DecodeMotion( lvEcdMotion.xy );
    vec2    lvRpjCoords = lvUvCoords + lvDcdMotion;
    vec2    lvStepSize  = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;

    // Sample current colour
    float   lfOcclCurr  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvUvCoords, 0.0 ).r;

    // Sample current 4 neighbours in a cross pattern
    float   lfOcclCurrT = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvUvCoords - vec2( 0.0, lvStepSize.y ), 0.0 ).r;
    float   lfOcclCurrB = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvUvCoords + vec2( 0.0, lvStepSize.y ), 0.0 ).r;
    float   lfOcclCurrL = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvUvCoords - vec2( lvStepSize.x, 0.0 ), 0.0 ).r;
    float   lfOcclCurrR = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap  ), lvUvCoords + vec2( lvStepSize.x, 0.0 ), 0.0 ).r;

    // Sample history colour
    float   lfOcclHist  = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lvRpjCoords, 0.0 ).r;

    // Sample speed weight
    float   lfSpeedW    = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lvUvCoords,  0.0 ).r;

    // Compute clipping box using Salvi's variance based method
    float   lfMoment0   = lfOcclCurrT + lfOcclCurrB + lfOcclCurrL + lfOcclCurrR + lfOcclCurr;
    float   lfMoment1   = sqr( lfOcclCurrT ) + sqr( lfOcclCurrB ) + sqr( lfOcclCurrL ) + sqr( lfOcclCurrR ) + sqr( lfOcclCurr );
    float   lfAvg       = lfMoment0 / 5.0;
    float   lfStd       = sqrt( max( 0.0, lfMoment1 / 5.0 - lfAvg * lfAvg ) );

    // Clip History
    float   lfOcclHClip = max( 0.0, clamp( lfOcclHist, lfAvg - 4.0 * lfStd, lfAvg + 4.0 * lfStd ) );

    // Accumulate
    float   lfOcclFinal;

    // Use the average as it's more stable and we don't care about detail/sharpness
    // Let a bit of unclipped history in; trades minor amount of ghosting for more stability
    lfOcclFinal = mix( lfOcclHist,  lfAvg,       0.125 + lfSpeedW * 0.875 );

    // Blend with the clipped history
    lfOcclFinal = mix( lfOcclHClip, lfOcclFinal, 0.250 );

    WRITE_FRAGMENT_COLOUR( vec4( lfOcclFinal, 0.0, 0.0, 1.0 ) );
}
#endif // D_SSS_TEMPORAL

#ifdef D_SHADOW_APPLY

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonLighting.shader.h"
#include "OutputPostProcess.shader.h"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec2, mTexCoordsVec2, TEXCOORD0 )
    #if defined(D_PLATFORM_METAL)
        flat INPUT( vec3, mUpMatrix_col0,     TEXCOORD1 ) 	
        flat INPUT( vec3, mUpMatrix_col1,     TEXCOORD2 ) 	
        flat INPUT( vec3, mUpMatrix_col2,     TEXCOORD3 ) 		
    #else
        flat INPUT( mat3, mUpMatrix,     TEXCOORD1 ) 	
    #endif 
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 lFragCoordsVec2 = TEX_COORDS;

    vec4 lBuffer1_Vec4   = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer1Map, lFragCoordsVec2 );

#if !defined( D_PLATFORM_SWITCH )
    // If material is parallax, then the value where glow would be is parallax self shadow value or nothing.
    int liMaterialID     = int( READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer2Map, lFragCoordsVec2 ).a * 3.0 ) << 8;

    float lfParallaxShadow = 1.0; // no shadow    
    if ( ( liMaterialID & D_PARALLAX ) != 0) 
    {
        lfParallaxShadow   = 1.0 - READ_GBUFFER( lUniforms.mpCustomPerMesh, gBuffer3Map, lFragCoordsVec2 ).a;
    }
#endif

    vec3 lPositionVec3 = DecodeGBufferPosition(
        lFragCoordsVec2,
        lUniforms.mpPerFrame.gClipPlanesVec4,
        lUniforms.mpPerFrame.gInverseProjectionMat4,
        lUniforms.mpPerFrame.gInverseViewMat4,
        lUniforms.mpPerFrame.gViewPositionVec3,
        lBuffer1_Vec4,
        false );

    float lfShadow = ComputeShadowIntensity( 
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gShadowMap), 
        DEREF_PTR( lUniforms.mpPerFrame ), 
        DEREF_PTR( lUniforms.mpCommonPerMesh ), 
        lPositionVec3, 
        vec3( 0.0, 0.0, 0.0 ), 
        lFragCoordsVec2, 
        true );

#if 0
    float lfOcclusion   = texture2DLod( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer4Map ), lFragCoordsVec2, 0.0 ).r;
    lfShadow            = min( 1.0f - lfOcclusion, lfShadow );
#endif

#if !defined( D_PLATFORM_SWITCH )
    // 1.0 = NOT IN SHADOW, 0.0 = IN SHADOW!
    lfShadow *= lfParallaxShadow;
#endif

    float lfCloudShadow = ComputeCloudOverlay(
        lFragCoordsVec2,
        SAMPLER2DPARAM_SRT(lUniforms.mpCustomPerMesh, gCloudShadowMap));

    lfShadow *= lfCloudShadow;


    //if( IN( mTexCoordsVec2 ).x < 0.5 )
    //{
    //    lColourVec3 = normalize( lPositionVec3 - lUniforms.mpPerFrame.gViewPositionVec3 ) * 0.5 + vec3( 0.5 );
    //}

    vec3 lSunColour = READ_GBUFFER( lUniforms.mpCustomPerMesh, gBufferMap, lFragCoordsVec2 ).rgb;
 
    WRITE_FRAGMENT_COLOUR( vec4( lSunColour * lfShadow, 1.0 ) );

#if 0
    vec4 lColour = texture2DLod(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gShadowMap), lFragCoordsVec2, 0);
    lColour.w = 1.0;
    WRITE_FRAGMENT_COLOUR(lColour);
#endif
}

#endif // D_POSTPROCESS

#if defined( D_SPOTLIGHT_VOLUME ) || defined( D_POINTLIGHT_VOLUME )

//-----------------------------------------------------------------------------
//      Includes

#include "Common/CommonUtils.shader.h"
#include "Common/CommonLighting.shader.h"
#include "OutputPostProcess.shader.h"

//-----------------------------------------------------------------------------
//      Global Data

#define G_SCATTERING    0.4

#if defined( D_SPOTLIGHT_VOLUME )
#define LENGTH_EPS_HIGH 1.01
#define LENGTH_EPS_LOW  0.99
#define ANGLE_EPS_HIGH  1.05
#define ANGLE_EPS_LOW   0.95
#define INF_POS         asfloat(0x7F800000)

#define STRENGTH_FACTOR 4.0
#define NOISE_P_FACTOR  4.0
#endif

#if defined( D_POINTLIGHT_VOLUME )
#define LENGTH_EPS_HIGH 1.03
#define LENGTH_EPS_LOW  0.95

#define STRENGTH_FACTOR 8.0
#define NOISE_P_FACTOR  8.0
#endif

#define DITHER_SCALE    4
#define STEPS_FACTOR    8
#define STEPS_ARR_SIZE  7
#define sqrt_fast       sqrt_fast_0
#define invsqrt_fast    invsqrt_fast_0
#define rcp_fast        rcp_fast_0

//#define D_DEBUG_COLOUR

STATIC_CONST float STEPS_SIZE[ STEPS_ARR_SIZE ] = {   1.25,  1.65,  1.75,  2.5, 2.75, 2.75, 5.0 };
STATIC_CONST int   STEPS_MAX [ STEPS_ARR_SIZE ] = {     16,    12,    10,    8,    6,    6,   4 };
STATIC_CONST int   STEPS_MIN [ STEPS_ARR_SIZE ] = {      4,     4,     2,    2,    2,    2,   1 };

//-----------------------------------------------------------------------------
//      Typedefs and Classes  
DECLARE_INPUT
    INPUT_SCREEN_POSITION
    INPUT_SCREEN_SLICE
    INPUT( vec4, mScreenSpacePositionVec4, TEXCOORD0 )
DECLARE_INPUT_END

#if defined( D_PLATFORM_ORBIS )
#pragma argument(unrollallloops)
#endif
#if defined( D_PLATFORM_SWITCH )
#pragma optionNV(unroll all)
//precision mediump float;
#endif

#if defined( D_PLATFORM_XBOXONE )
#define fma __XB_FMA_F32
#elif defined( D_PLATFORM_VULKAN )
#elif defined( D_PLATFORM_DX12 )
float fma(float a, float b, float c)
{
    return a * b + c;
}
#endif

//-----------------------------------------------------------------------------
//    Functions

//-----------------------------------------------------------------------------
///
///     ComputeScatteringMie
///     Mie-scaterring approximated with Henyey-Greenstein phase function.
///
//-----------------------------------------------------------------------------
float
ComputeScatteringMie(
    float costh )
{    
    float g = G_SCATTERING;
    //return (1.0 - g * g) * rcp_fast(4.0 * M_PI * pow(1.0 + g * g - 2.0 * g * costh, 3.0 / 2.0));

    float a = 1.0 + g * g - 2.0 * g * costh;
    float b = invsqrt_fast( a );
    return 0.25 * M_1_PI * (1.0 - g * g) * b * b * b;
}

//-----------------------------------------------------------------------------
///
///     ComputeScatteringRayleigh
///
//-----------------------------------------------------------------------------
float
ComputeScatteringRayleigh(
    float costh )
{    
    return 0.75 * fma( costh, costh, 1.0 );
}

//-----------------------------------------------------------------------------
///
///     ComputeScattering
///
//-----------------------------------------------------------------------------
float
ComputeScattering(
    float costh)
{
    return ComputeScatteringMie( costh ) + 0.125 * ComputeScatteringRayleigh( costh );
}

//-----------------------------------------------------------------------------
///
///     ComputeLightAttenuation
///
//-----------------------------------------------------------------------------
float 
ComputeLightAttenuation(
#if !defined( D_LIGHT_CUSTOM )
    float lfDistSqr )
#else
    float lfDistSqr,
    float lfFalloff )
#endif
{        
    float lfAttenuation;
#if   defined( D_LIGHT_CONSTANT )
    lfAttenuation = 1.0;
#elif defined( D_LIGHT_LINEAR )
    lfAttenuation = invsqrt_fast( lfDistSqr );
    lfAttenuation = min( lfAttenuation, 1.0 );
#elif defined( D_LIGHT_LINEAR_SQRT )
    lfAttenuation = invsqrt_fast( lfDistSqr );
    lfAttenuation = lfAttenuation * sqrt_fast( lfAttenuation );
    lfAttenuation = min( lfAttenuation, 1.0 );
#elif defined( D_LIGHT_QUADRATIC )
    lfAttenuation = rcp_fast( lfDistSqr );
    lfAttenuation = min( lfAttenuation, 1.0 );
#elif defined( D_LIGHT_CUBIC )
    lfAttenuation = rcp_fast( lfDistSqr ) * invsqrt_fast( lfDistSqr );
    lfAttenuation = min( lfAttenuation, 1.0 );
#elif defined( D_LIGHT_CUSTOM )
    lfAttenuation = rcp_fast( pow( lfDistSqr, 0.5 * lfFalloff ) );
    lfAttenuation = min( lfAttenuation, 1.0 );
#endif
    return lfAttenuation;
}

float
GetStepSize(
    in  vec2  lTVec2,
    in  float lfDither,
    out int   liNumSteps )
{
    int   liStepIdx;
    float lfDist;
    float lfExtent;
    float lfStepSize;

    lfDist     = lTVec2.x + lfDither * float(DITHER_SCALE);
    lfExtent   = max( 0.0, lTVec2.y - lfDist );
    liStepIdx  = clamp( int(lfDist / float(STEPS_FACTOR)), 0, STEPS_ARR_SIZE - 1 );
    lfStepSize = STEPS_SIZE[ liStepIdx ];
    liNumSteps = min( STEPS_MAX[ liStepIdx ], max( STEPS_MIN[ liStepIdx ], int( lfExtent / lfStepSize ) ) );
    lfStepSize = ( lTVec2.y - lTVec2.x ) / ( float( liNumSteps ) );

    return lfStepSize;
}

#if defined( D_DEBUG_COLOUR )
vec3
GetStepsDebugColour(
    int liNumSteps )
{
    vec3 lDebugColourVec3;

    if      ( liNumSteps >= 12 ) lDebugColourVec3 = vec3( 1.0, 0.0, 0.0 );
    else if ( liNumSteps >= 10 ) lDebugColourVec3 = vec3( 0.0, 0.0, 1.0 );
    else if ( liNumSteps >= 8  ) lDebugColourVec3 = vec3( 0.0, 1.0, 0.0 );
    else if ( liNumSteps >= 4  ) lDebugColourVec3 = vec3( 0.5, 0.5, 0.5 );
    else if ( liNumSteps >= 2  ) lDebugColourVec3 = vec3( 1.0, 1.0, 0.0 );
    else                         lDebugColourVec3 = vec3( 1.0, 0.0, 1.0 );

    return lDebugColourVec3 * 0.25;
}
#endif

bool
ValidateRayLightIntersection(
    in    float depth,
    inout vec2  t_vec )
{
    bool valid;
    valid = depth >= t_vec.x;
    t_vec = vec2( max( t_vec.x, 0.0 ), min( t_vec.y, depth ) );
    return valid;
}

#if defined( D_SPOTLIGHT_VOLUME )

//-----------------------------------------------------------------------------
///
///     RayConeIntersect
///     http://lousodrome.net/blog/light/2017/01/03/intersection-of-a-ray-and-a-cone/
///
//-----------------------------------------------------------------------------
bool 
RayConeIntersect(    
    in  vec3    lRayOriginVec3,
    in  vec3    lRayDirVec3,
    in  vec3    lSpotPosVec3,
    in  vec3    lSpotAtVec3,
    in  float   lfSpotLength,
    in  float   lfSpotRad,
    in  float   lfSpotFOV,
    out vec2    lTVec2 )
{    
    // cone properties
    vec3    c_o         = lSpotPosVec3;
    vec3    c_d         = lSpotAtVec3;
    float   c_h         = lfSpotLength;
    float   c_r         = lfSpotRad;
    float   c_cos_sqr   = lfSpotFOV * lfSpotFOV;

    // ray properties
    vec3    r_o         = lRayOriginVec3;
    vec3    r_d         = lRayDirVec3;

    // others
    vec3    c_to_r      = r_o - c_o;
    float   eps         = 1.0e-64;

    // no disk intersection; look for cone side intersection
    // quadratic equation
    float a     = dot(r_d,c_d)*dot(r_d,c_d) - c_cos_sqr;    
    float a_rcp = 1.0 / a;
    float b     = a_rcp * ( dot(r_d,c_d)   *dot(c_to_r,c_d) - dot(r_d, c_to_r)   *c_cos_sqr );
    float c     = a_rcp * ( dot(c_to_r,c_d)*dot(c_to_r,c_d) - dot(c_to_r, c_to_r)*c_cos_sqr );
    float t     = b*b - c;
    if ( abs(a) < eps || t < 0.0 ) return false;
    
    t = sqrt( t );

    // cone intersections
    vec2  t_vec;
    t_vec       = vec2( -b - t, -b + t );

    // validate intersections
    vec2  t_vec_h;
    t_vec_h.x   = dot( r_o + r_d * t_vec.x - c_o, c_d );
    t_vec_h.y   = dot( r_o + r_d * t_vec.y - c_o, c_d );
    
    t_vec.x     = t_vec_h.x < 0.0 || t_vec_h.x > c_h ? INF_POS : t_vec.x;
    t_vec.y     = t_vec_h.y < 0.0 || t_vec_h.y > c_h ? INF_POS : t_vec.y;
    t_vec       = t_vec.x > t_vec.y ? t_vec.yx : t_vec;    
    lTVec2      = t_vec;

    return !isinf( t_vec.x );
}

//-----------------------------------------------------------------------------
///
///     RayPlaneIntersect
///
//-----------------------------------------------------------------------------
bool 
RayPlaneIntersect(    
    in  vec3    lRayOriginVec3,
    in  vec3    lRayDirVec3,
    in  vec3    lSpotPosVec3,
    in  vec3    lSpotAtVec3,
    in  float   lfSpotLength,
    in  float   lfSpotRad,    
    out float   lfT )
{    
    // cone properties
    vec3  c_o   = lSpotPosVec3;
    vec3  c_d   = lSpotAtVec3;
    float c_h   = lfSpotLength;
    float c_r   = lfSpotRad;
    
    // ray properties
    vec3  r_o   = lRayOriginVec3;
    vec3  r_d   = lRayDirVec3;

    // plane properties
    vec3  p_o   = c_o + c_h * c_d;
    vec3  p_d   = c_d;      
    
    // ray-disk intersection
    float t     = dot( p_o - r_o, p_d ) / dot( r_d, p_d );
    lfT         = t;

    return !isinf( t );
}

//-----------------------------------------------------------------------------
///
///     IsFragmentInsideCone
///
//-----------------------------------------------------------------------------
bool
IsFragmentInsideCone(
    vec3  lSpotPosVec3,
    vec3  lSpotAtVec3,
    vec3  lWorldPosVec3,
    float lfConeDepth,
    float lfWorldDepth,
    float lfSpotFOV,
    float lfSpotLength )
{    
    if ( lfConeDepth < lfWorldDepth ) return true;

    bool lbOutside = dot( lWorldPosVec3 - lSpotPosVec3, lSpotAtVec3 ) > lfSpotLength * LENGTH_EPS_HIGH;
    if ( lbOutside ) return false;
    lbOutside      = dot( normalize( lWorldPosVec3 - lSpotPosVec3 ), lSpotAtVec3 ) <= lfSpotFOV * ANGLE_EPS_LOW;
    if ( lbOutside ) return false;
    return true;
}

bool
IsInsideCone(
    vec3  lSpotPosVec3,
    vec3  lSpotAtVec3,
    vec3  lWorldPosVec3,
    float lfSpotFOV,
    float lfSpotLength )
{   
    bool lbOutside = dot( lWorldPosVec3 - lSpotPosVec3, lSpotAtVec3 ) > lfSpotLength * LENGTH_EPS_HIGH;
    if ( lbOutside ) return false;
    lbOutside      = dot( normalize( lWorldPosVec3 - lSpotPosVec3 ), lSpotAtVec3 ) <= lfSpotFOV * ANGLE_EPS_LOW;
    if ( lbOutside ) return false;
    return true;
}



//-----------------------------------------------------------------------------
///
///     GetRayConeIntersection
///
//-----------------------------------------------------------------------------
bool
GetRayConeIntersection(
    in  vec3    lWorldPosVec3,
    in  vec3    lConePosVec3,
    in  vec3    lSpotPosVec3,
    in  vec3    lSpotAtVec3,
    in  float   lfSpotLength,
    in  float   lfSpotRad,
    in  float   lfSpotFOV,
    in  vec3    lRayOrigVec3,
    in  vec3    lRayDirVec3,
    out vec2    lTVec2 )
{     
    bool  lbValid;
    float lfT;
    float lfWorldDepth;
    float lfConeDepth;

    lfWorldDepth    = dot( lWorldPosVec3,  lRayDirVec3 );
    lfConeDepth     = dot( lConePosVec3,   lRayDirVec3 );

#if defined( D_LIGHT_IN )
    lbValid         = IsFragmentInsideCone( lSpotPosVec3, lSpotAtVec3, lWorldPosVec3, lfConeDepth, lfWorldDepth, lfSpotFOV, lfSpotLength );
    if ( !lbValid ) return false;
#endif

    lbValid         = RayConeIntersect( lRayOrigVec3, lRayDirVec3, lSpotPosVec3, lSpotAtVec3, lfSpotLength, lfSpotRad, lfSpotFOV, lTVec2 );
    if ( !lbValid ) return false;
    //
    //// if both      cone hits are invalid, we already earlied out;
    //// if both      cone hits are valid,   no need to check for a plane hit
    //// if only one  cone hit  is  invalid, the plane hit will also be a disk hit
    if ( isinf( lTVec2.y ) )
    {
        float tempY = lTVec2.y;
        lbValid     = RayPlaneIntersect( lRayOrigVec3, lRayDirVec3, lSpotPosVec3, lSpotAtVec3, lfSpotLength, lfSpotRad, tempY );
        lTVec2.y = tempY;
        if ( !lbValid ) return false;
    
        lTVec2      = lTVec2.x > lTVec2.y ? lTVec2.yx : lTVec2;
    }
    //if ( any( isinf( lTVec2 ) ) ) return false;

    lbValid         = ValidateRayLightIntersection( lfWorldDepth, lTVec2 );
    //lbValid         = lbValid && IsInsideCone( lSpotPosVec3, lSpotAtVec3, lRayDirVec3 * lTVec2.x, lfSpotFOV, lfSpotLength );
    //lbValid         = lbValid && IsInsideCone( lSpotPosVec3, lSpotAtVec3, lRayDirVec3 * lTVec2.y, lfSpotFOV, lfSpotLength );
    return lbValid;
}


//-----------------------------------------------------------------------------
///
///     ComputeSpotStrength
///
//-----------------------------------------------------------------------------
float 
ComputeTotalSpotStrength(
    SAMPLER3DARG(lNoiseMap),
    vec3  lSpotPosVec3,
    vec3  lSpotAtVec3,  
    vec3  lSpotUpVec3,
    vec3  lSpotRightVec3,  
    float lfSpotStrength, 
    float lfSpotVolumetric, 
    float lfSpotLength,
    float lfSpotRad,
    #if defined( D_LIGHT_CUSTOM )
    float lfSpotFalloff,
    #endif
    vec3  lRayOrigVec3,
    vec3  lRayDirVec3,
    vec2  lTVec2,
    int   liNumSteps,
    float lfStep,
    float lfDither,
    float lfTime )
{    
    float   lfNoise;
    float   lfScattering;
    float   lfTotalSpotStrength;
    float   lfTotalStrengthFactor;
    float   lfSqrDist;
    vec3    lStepVec3;
    vec3    lConePointVec3;
    vec3    lNoiseUVs;

    lfTotalSpotStrength     = 0.0;
    lfDither               *= lfStep;
    lConePointVec3          = lRayOrigVec3 + lRayDirVec3 * ( lTVec2.x + lfDither );
    lStepVec3               = lRayDirVec3 * lfStep;

    // place coordinate system origin at cone vertex
    lConePointVec3         -= lSpotPosVec3;

    // rough approximation, 
    // the correct calculation should take the scattering at 
    // the normalized cone point direction for each sample
    lfScattering            = ComputeScattering( dot( lRayDirVec3, lSpotAtVec3 ) );
    lfTotalStrengthFactor   = lfStep * lfSpotStrength * lfSpotVolumetric * lfScattering * STRENGTH_FACTOR;

    for ( int ii = 0; ii < liNumSteps; ++ii )
    {               
        lfSqrDist               = dot( lConePointVec3, lConePointVec3 );
        lNoiseUVs.x             = dot( lConePointVec3, lSpotRightVec3 ) * rcp_fast( lfSpotRad );
        lNoiseUVs.y             = dot( lConePointVec3, lSpotUpVec3 )    * rcp_fast( lfSpotRad );
        lNoiseUVs.z             = dot( lConePointVec3, lSpotAtVec3 )    * rcp_fast( lfSpotLength ); 
        lNoiseUVs.z             = mod( lNoiseUVs.z - lfTime / 64.0, 1.0 );
        lNoiseUVs              *= 2.0;
        lfNoise                 = texture3DLod( lNoiseMap, lNoiseUVs, 0 ).r;
        
        // TODO(sal): re-export the texture with scaled up noise
        lfNoise                *= pow( lfNoise, NOISE_P_FACTOR );
        
        #if !defined( D_LIGHT_CUSTOM )
        lfTotalSpotStrength    += ComputeLightAttenuation( lfSqrDist ) * lfNoise;
        #else
        lfTotalSpotStrength    += ComputeLightAttenuation( lfSqrDist, lfSpotFalloff ) * lfNoise;
        #endif
        lConePointVec3         += lStepVec3;
    }
    lfTotalSpotStrength *= lfTotalStrengthFactor;
    return lfTotalSpotStrength;
}


#if     defined( D_LIGHT_IN )
FRAGMENT_MAIN_COLOUR_SRT
#elif   defined( D_LIGHT_OUT )
FRAGMENT_MAIN_COLOUR_EARLYZ_SRT
#endif
{
#ifndef D_PLATFORM_GLSL
    vec2  lFragCoordsVec2   = IN(mScreenPositionVec4).xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
#else
    vec2  lFragCoordsVec2   = IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w;
#endif
    float lfWorldDepthRevZ  = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lFragCoordsVec2 ) );

    vec3  lWorldPosVec3     = RecreatePositionFromRevZDepth(
                                lfWorldDepthRevZ, lFragCoordsVec2, float2vec3(0.0),
                                lUniforms.mpPerFrame.gInverseViewProjectionMat4 );

    float lfConeDepthRevZ   = IN_SCREEN_POSITION.z;

    vec3  lConePosVec3      = RecreatePositionFromRevZDepth(
                                lfConeDepthRevZ, lFragCoordsVec2, float2vec3(0.0),
                                lUniforms.mpPerFrame.gInverseViewProjectionMat4 ); 
        
    vec3  lSpotPosVec3      = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.xyz;
    vec3  lSpotAtVec3       = lUniforms.mpCommonPerMesh.gSpotlightDirectionVec4.xyz;    
    vec3  lSpotUpVe3        = lUniforms.mpCommonPerMesh.gSpotlightUpVec4.xyz;    
    vec3  lRayOrigVec3      = float2vec3( 0.0 );
    vec3  lRayDirVec3       = normalize( lConePosVec3 );
    
    float lfSpotFOV         = lUniforms.mpCommonPerMesh.gSpotlightDirectionVec4.w * ANGLE_EPS_HIGH;
    float lfSpotLength      = lUniforms.mpCommonPerMesh.gLightPositionVec4.w      * LENGTH_EPS_LOW;
    // TODO(sal): take care of this awful transcendental functions galore
    float lfSpotRad         = abs( sin( acos( lfSpotFOV ) ) ) / lfSpotFOV * lfSpotLength;
#if defined( D_LIGHT_CUSTOM )
    float lfSpotFalloff     = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.w;
#endif
    vec2  lTVec2;
    bool  lbIntersect       = GetRayConeIntersection(   lWorldPosVec3, lConePosVec3, 
                                                        lSpotPosVec3,  lSpotAtVec3, 
                                                        lfSpotLength,  lfSpotRad,   lfSpotFOV,
                                                        lRayOrigVec3,  lRayDirVec3, lTVec2 );

    // we explicitly requested early-z test for the out context
    // so discarding shouldn't be a problem,
    // still, avoiding it might help on some platforms
    if ( !lbIntersect )
    {
#if   defined( D_LIGHT_IN )
        discard;
#elif defined( D_LIGHT_OUT )
        WRITE_FRAGMENT_COLOUR( vec4(  0.0, 0.0, 0.0, 1.0 ) );
        return;
#endif
    }
        
    vec3  lSpotRightVec3    = cross( lSpotUpVe3, lSpotAtVec3 );

    float lfSpotStrength    = lUniforms.mpCommonPerMesh.gLightColourVec4.w;
    float lfSpotVolumetric  = lUniforms.mpCommonPerMesh.gLightCustomParamsVec4.x;
    float lfTime            = lUniforms.mpPerFrame.gfTime;
    float lfDither          = BayerFract( uvec2( lFragCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) );
    
    int   liNumSteps;
    float lfStep            = GetStepSize( lTVec2, lfDither, liNumSteps );

    lfSpotStrength          = ComputeTotalSpotStrength( SAMPLER3DPARAM_SRT( lUniforms.mpCustomPerMesh, gLightVolNoise3D ),
                                                        lSpotPosVec3,   lSpotAtVec3,      lSpotUpVe3,   lSpotRightVec3, 
                                                        lfSpotStrength, lfSpotVolumetric, lfSpotLength, lfSpotRad,
                                                        #if defined( D_LIGHT_CUSTOM )
                                                        lfSpotFalloff,
                                                        #endif
                                                        lRayOrigVec3,   lRayDirVec3,      lTVec2,
                                                        liNumSteps,     lfStep,           lfDither,     lfTime );

    vec3  lSpotColourVec3;
    lSpotColourVec3         = lUniforms.mpCommonPerMesh.gLightColourVec4.xyz;
    lSpotColourVec3        *= lfSpotStrength;

#if defined( D_DEBUG_COLOUR )
    lSpotColourVec3         = GetStepsDebugColour( liNumSteps );
#endif

    WRITE_FRAGMENT_COLOUR( vec4(  lSpotColourVec3, 1.0 ) );
}

#endif // D_SPOTLIGHT_VOLUME

#ifdef D_POINTLIGHT_VOLUME

bool
RaySphereIntersect(
    in  vec3  ro, 
    in  vec3  rd, 
    in  vec3  sph, 
    in  float rad,
    out vec2  t_vec )
{
    vec3  oc     = ro - sph;
    float b      = dot(oc, rd);
    float c      = dot(oc, oc) - rad * rad;
    float t      = b * b - c;
    
    if (t < 0.0) return false;

    t     = sqrt(t);
    t_vec = vec2( -b - t, -b + t );

    return t_vec.y >= 0.0;
}

//-----------------------------------------------------------------------------
///
///     IsFragmentInsideSphere
///
//-----------------------------------------------------------------------------
bool
IsFragmentInsideSphere(
    vec3    lPointPosVec3,
    vec3    lWorldPosVec3,
    float   lfSphereDepth,
    float   lfWorldDepth,
    float   lfPointRad )
{
    if ( lfSphereDepth < lfWorldDepth ) return true;

    float lfActualRad = lfPointRad * LENGTH_EPS_HIGH;
    bool  lbInside    = dot( lWorldPosVec3 - lPointPosVec3, lWorldPosVec3 - lPointPosVec3 ) < lfActualRad * lfActualRad;
    
    return lbInside;
}


bool 
GetRaySphereIntersection(
    in  vec3    lWorldPosVec3,
    in  vec3    lSpherePosVec3,
    in  vec3    lPointPosVec3,
    in  float   lfPointRad,
    in  vec3    lRayOrigVec3,
    in  vec3    lRayDirVec3,
    out vec2    lTVec2 )
{
    bool  lbValid;
    float lfWorldDepth;
    float lfSphereDepth;

    lfWorldDepth    = dot( lWorldPosVec3,  lRayDirVec3 );
    lfSphereDepth   = dot( lSpherePosVec3, lRayDirVec3 );

#if defined( D_LIGHT_IN )
    lbValid         = IsFragmentInsideSphere( lPointPosVec3, lWorldPosVec3, lfSphereDepth, lfWorldDepth, lfPointRad );
    if ( !lbValid ) return false;
#endif

    lbValid         = RaySphereIntersect( lRayOrigVec3, lRayDirVec3, lPointPosVec3, lfPointRad, lTVec2 );        
    lbValid         = lbValid && ValidateRayLightIntersection( lfWorldDepth, lTVec2 );
    
    return lbValid;
}


vec2 OctWrap( vec2 v )
{
    return ( 1.0 - abs( v.yx ) ) * ( vec2( v.x >= 0.0, v.y >= 0.0 ) * 2.0 - 1.0 );
}
 
vec2 Oct3dTo2dMapping( vec3 n )
{
    n    = n * rcp_fast( abs( n.x ) + abs( n.y ) + abs( n.z ) );
    n.xy = n.z >= 0.0 ? n.xy : OctWrap( n.xy );
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
}


//-----------------------------------------------------------------------------
///
///     ComputeTotalPointStrength
///
//-----------------------------------------------------------------------------
float 
ComputeTotalPointStrength(
    SAMPLER3DARG( lNoiseMap ),
    vec3  lPointPosVec3,
    float lfPointStrength,
    float lfPointVolumetric,
    float lfPointRad,
    #if defined( D_LIGHT_CUSTOM )
    float lfPointFalloff,
    #endif
    vec3  lRayOrigVec3,
    vec3  lRayDirVec3,
    vec2  lTVec2,
    int   liNumSteps,
    float lfStep,
    float lfDither,
    float lfTime )  
{    
    float   lfNoise;
    float   lfScattering;
    float   lfTotalPointStrength;
    float   lfTotalStrengthFactor;
    float   lfDist;
    float   lfDistSqr;
    float   lfDistRcp;
    vec3    lStepVec3;
    vec3    lSpherePointVec3;
    vec3    lNoiseUVs;

    lfTotalPointStrength    = 0.0;
    lfDither               *= lfStep;
    lfTotalStrengthFactor   = lfStep * lfPointStrength * lfPointVolumetric * STRENGTH_FACTOR;
    lSpherePointVec3        = lRayOrigVec3 + lRayDirVec3 * ( lTVec2.x + lfDither );
    lStepVec3               = lRayDirVec3 * lfStep;

    // place coordinate system origin at sphere center
    lSpherePointVec3       -= lPointPosVec3;

    for ( int ii = 0; ii < liNumSteps; ++ii )
    {
        lfDistSqr               = dot( lSpherePointVec3, lSpherePointVec3 );
        lfDist                  = sqrt_fast( lfDistSqr );
        lfDistRcp               = invsqrt_fast( lfDistSqr );        
        lNoiseUVs.xy            = Oct3dTo2dMapping( lSpherePointVec3 );
        lNoiseUVs.z             = sqrt_fast( sqrt_fast( lfDist ) );
        lNoiseUVs.z             = fract( lNoiseUVs.z - lfTime / 32.0 );
        lfNoise                 = texture3DLod( lNoiseMap, lNoiseUVs, 0 ).r;

        // TODO(sal): re-export the texture with scaled up noise
        lfNoise                *= pow( lfNoise, NOISE_P_FACTOR );

        lfScattering            = ComputeScattering( dot( lRayDirVec3, lSpherePointVec3 * lfDistRcp ) );
        #if !defined( D_LIGHT_CUSTOM )
        lfTotalPointStrength   += ComputeLightAttenuation( lfDistSqr ) * lfScattering * lfNoise;
        #else
        lfTotalPointStrength   += ComputeLightAttenuation( lfDistSqr, lfPointFalloff ) * lfScattering * lfNoise;
        #endif
        lSpherePointVec3       += lStepVec3;
    }
    lfTotalPointStrength *= lfTotalStrengthFactor;
    return lfTotalPointStrength;
}


#if     defined( D_LIGHT_IN )
FRAGMENT_MAIN_COLOUR_SRT
#elif   defined( D_LIGHT_OUT )
FRAGMENT_MAIN_COLOUR_EARLYZ_SRT
#endif
{
#ifndef D_PLATFORM_GLSL
    vec2  lFragCoordsVec2   = IN(mScreenPositionVec4).xy * lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
#else
    vec2  lFragCoordsVec2   = IN(mScreenSpacePositionVec4).xy / IN(mScreenSpacePositionVec4).w;
#endif
    float lfWorldDepthRevZ  = DecodeDepthFromColour( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBufferMap ), lFragCoordsVec2 ) );

    vec3  lWorldPosVec3     = RecreatePositionFromRevZDepth(
                                lfWorldDepthRevZ, lFragCoordsVec2, float2vec3(0.0),
                                lUniforms.mpPerFrame.gInverseViewProjectionMat4 );

    float lfSphereDepthRevZ = IN_SCREEN_POSITION.z;

    vec3  lSpherePosVec3    = RecreatePositionFromRevZDepth(
                                lfSphereDepthRevZ, lFragCoordsVec2, float2vec3(0.0),
                                lUniforms.mpPerFrame.gInverseViewProjectionMat4 );

    vec3  lPointPosVec3     = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.xyz;
    vec3  lRayOrigVec3      = float2vec3( 0.0 );
    vec3  lRayDirVec3       = normalize( lSpherePosVec3 );
    float lfPointRad        = lUniforms.mpCommonPerMesh.gLightPositionVec4.w * LENGTH_EPS_LOW;

#if defined( D_LIGHT_CUSTOM )
    float lfPointFalloff    = lUniforms.mpCommonPerMesh.gSpotlightPositionVec4.w;
#endif

    vec2  lTVec2;
    bool  lbIntersect       = GetRaySphereIntersection( lWorldPosVec3, lSpherePosVec3, 
                                                        lPointPosVec3, lfPointRad,
                                                        lRayOrigVec3,  lRayDirVec3, lTVec2 );
    
    // we explicitly requested early-z test for the out context
    // so discarding shouldn't be a problem,
    // still, avoiding it might help on some platforms
    if ( !lbIntersect )
    {
#if   defined( D_LIGHT_IN )
        discard;
#elif defined( D_LIGHT_OUT )
        WRITE_FRAGMENT_COLOUR( vec4(  0.0, 0.0, 0.0, 1.0 ) );
        return;
#endif
    }
    
    float lfPointStrength   = lUniforms.mpCommonPerMesh.gLightColourVec4.w;
    float lfPointVolumetric = lUniforms.mpCommonPerMesh.gLightCustomParamsVec4.x;
    float lfTime            = lUniforms.mpPerFrame.gfTime;
    float lfDither          = BayerFract( uvec2( lFragCoordsVec2 * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy ) );
    
    int   liNumSteps;
    float lfStep            = GetStepSize( lTVec2, lfDither, liNumSteps );

    lfPointStrength         = ComputeTotalPointStrength( SAMPLER3DPARAM_SRT( lUniforms.mpCustomPerMesh, gLightVolNoise3D ),
                                                         lPointPosVec3, 
                                                         lfPointStrength, lfPointVolumetric,
                                                         lfPointRad,
                                                         #if defined( D_LIGHT_CUSTOM )
                                                         lfPointFalloff,
                                                         #endif
                                                         lRayOrigVec3,    lRayDirVec3,  lTVec2,
                                                         liNumSteps,      lfStep,       lfDither,   lfTime );

    vec3  lPointColourVec3;
    lPointColourVec3        = lUniforms.mpCommonPerMesh.gLightColourVec4.xyz;
    lPointColourVec3       *= lfPointStrength;

#if defined( D_DEBUG_COLOUR )
    lPointColourVec3        = GetStepsDebugColour( liNumSteps );
#endif

    WRITE_FRAGMENT_COLOUR( vec4( lPointColourVec3, 1.0 ) );
}

#endif // D_POINTLIGHT_VOLUME

#endif // D_SPOTLIGHT_VOLUME || D_POINTLIGHT_VOLUME

//TF_BEGIN

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

// Dummy main
#if defined(D_TILED_LIGHTSx)
DECLARE_INPUT

INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE
INPUT(vec2, mTexCoordsVec2, TEXCOORD0)

DECLARE_INPUT_END

FRAGMENT_MAIN_COLOUR_SRT
{
}
#endif
#if defined(D_TILED_LIGHTS)
//-----------------------------------------------------------------------------
//      Includes
#include "Common/CommonLighting.shader.h"

//-----------------------------------------------------------------------------
//      Global Data


//-----------------------------------------------------------------------------
//      Typedefs and Classes  
DECLARE_INPUT

INPUT_SCREEN_POSITION
INPUT_SCREEN_SLICE
INPUT(vec2, mTexCoordsVec2, TEXCOORD0)

DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//    Functions

bool sphereCone(
    const vec3 sphereCenter, const float sphereRadius,
    const vec3 coneOrigin, const vec3 coneNormal,
    const float sinAngle, const float tanAngleSqPlusOne)
{
    const vec3 diff = sphereCenter - coneOrigin;

    /* Point - cone test */
    if(dot(diff - sphereRadius*sinAngle*coneNormal, coneNormal) > 0.0f) {
        const vec3 c = sinAngle*diff + coneNormal*sphereRadius;
        const float lenA = dot(c, coneNormal);

        return dot(c, c) <= lenA*lenA*tanAngleSqPlusOne;

    /* Simple sphere point check */
    } else return dot(diff, diff) <= sphereRadius*sphereRadius;
}
bool sphereCone(
    const vec3 sphereCenter, const float sphereRadius,
    const vec3 coneOrigin, const vec3 coneNormal, const float coneAngle)
{
    const float halfAngle = coneAngle*0.5f;
    const float sinAngle = sin(halfAngle);
    const float tanAngleSqPlusOne = 1.0f + pow(tan(halfAngle), 2.0f);

    return sphereCone(sphereCenter, sphereRadius, coneOrigin, coneNormal, sinAngle, tanAngleSqPlusOne);
}
//-----------------------------------------------------------------------------
///
///     Main
///
///     @brief      Fragment Main
///
///     @param      void
///     @return     Nothing. 
///
//-----------------------------------------------------------------------------
FRAGMENT_MAIN_COLOUR_SRT
{
	const int liLightsVisible = int(lUniforms.mpCommonPerMesh.gUserDataVec4.x);
#ifdef D_PLATFORM_METAL
    //This include tile width. only to convert from pix coords.
    const int tilesPerWidth = int(mpCommonPerMesh.gUserDataVec4.y);
    const int tilesPerHeight = int(mpCommonPerMesh.gUserDataVec4.w);
	//This include tile width + 1 to consider numOfLights
	const int sizeOftile = int(mpCommonPerMesh.gUserDataVec4.z);
#else
    const int tilesPerWidth = D_TILE_WIDTH;
    const int tilesPerHeight = D_TILE_HEIGHT;
#endif

    //pixel coords
	const ivec2 liThreadGlobalCoord = ivec2(TEX_COORDS * lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy);
	//which pixel in current tile (0 to tile_width - 1, 0 to tile_height -1 )
	const ivec2 liThreadLocalCoord = ivec2(liThreadGlobalCoord.x % D_TILE_WIDTH, liThreadGlobalCoord.y % D_TILE_HEIGHT);
	const int luThreadLocalIndex = liThreadLocalCoord.x + liThreadLocalCoord.y * D_TILE_WIDTH;
	//current 2d tile
	const ivec2 liTileCoord = ivec2(liThreadGlobalCoord.x / D_TILE_WIDTH, liThreadGlobalCoord.y / D_TILE_HEIGHT);

#ifdef D_PLATFORM_METAL
    //tile index
    const int luTileImageIndex =  liTileCoord.x + liTileCoord.y *  tilesPerWidth;
	//tile buffer start
	const int luTileBufferIndex = luTileImageIndex * sizeOftile;
#endif

	ivec2 liTileBase = liTileCoord * ivec2(D_TILE_WIDTH, D_TILE_HEIGHT);

#ifdef D_TILED_LIGHTS_CLUSTER_DEBUG
#ifndef D_COMPUTE
        // Visualize tiles (works)
		const ivec2 liTileDim = ivec2(lUniforms.mpPerFrame.gFrameBufferSizeVec4.x / D_TILE_WIDTH, lUniforms.mpPerFrame.gFrameBufferSizeVec4.y / D_TILE_HEIGHT);
    	WRITE_FRAGMENT_COLOUR(vec4(float(liTileCoord.x) / float(liTileDim.x), float(liTileCoord.y) / float(liTileDim.y), 0.0f, 0.3f));
#endif
#endif

#ifndef D_TILED_LIGHTS_DEBUG


	const  int liThreadLocalIndex =  liThreadLocalCoord.y * D_TILE_WIDTH + liThreadLocalCoord.x;
#ifdef D_PLATFORM_METAL
    if(liThreadLocalIndex != 0)
        return Out;
    else
    {
        atomic_store_explicit(& gLightCluster [luTileBufferIndex], 0, memory_order::memory_order_relaxed);
    }
#endif

    vec2 lfTileStepSize = (vec2(D_TILE_WIDTH, D_TILE_HEIGHT) / lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy);
    vec2 liTileProject = vec2(liTileBase) / lUniforms.mpPerFrame.gFrameBufferSizeVec4.xy;

    // compute frustum plane normals and tile Cone normal and angles
    const vec2 tilesHalfScale = vec2( tilesPerWidth, tilesPerHeight ) / 2.0f;
    const vec2 tileBias = tilesHalfScale - vec2(liTileCoord) + vec2(-1.5,0.5);
    mat4 proj = lUniforms.mpPerFrame.gProjectionMat4;
    vec4 col4 = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    vec4 col1 = vec4(proj[0][0] * -tilesHalfScale.x, 0.0f, tileBias.x, 0.0f);
    vec4 col2 = vec4(0.0f, proj[1][1] * tilesHalfScale.y, tileBias.y, 0.0f);

    vec4 frustumPlanes[6];
    frustumPlanes[0] = col4 + col1; // left
    // frustumPlanes[1] = col4 - col1; // right
    frustumPlanes[2] = col4 - col2; // top
    // frustumPlanes[3] = col4 + col2; // bottom
    // frustumPlanes[4] = vec4(0.0f, 0.0f, -1.0f,  -1);
    // frustumPlanes[5] = vec4(0.0f, 0.0f, -1.0f,  1000);
    const vec3 coneNormal = normalize(cross(frustumPlanes[2].xyz, frustumPlanes[0].xyz));
    const float fOvOverTiles = lUniforms.mpPerFrame.gFrameBufferSizeVec4.x / D_TILE_WIDTH;
    const float tileAngle = 2.0*atan( 1.0/proj[1][1] ) / fOvOverTiles;

#if defined( D_USE_TILE_Z_EXTENTS )
	float minZ = FLT_MAX;
	float maxZ = -FLT_MAX;
#endif

#ifdef D_PLATFORM_METAL	
	for (int i = 0; i < liLightsVisible; i += 1)
    {
		int luLightIndex = i;
#else
    for (int i = 0; i < liLightsVisible; i += D_TILE_SIZE)
	{
		int luLightIndex = luThreadLocalIndex + i;
#endif

		if (luLightIndex >= liLightsVisible)
		{
			break;
		}

		vec4 lLightPosIntensity = lUniforms.mpPerFrame.gLocalLightPosMultiVec4[luLightIndex];
		vec4 lLightData= lUniforms.mpPerFrame.gLocalLightDataMultiVec4[luLightIndex];
		float lfCutOff = 0.04;
		float lfLightRadius = ComputeAttenuationRadius(lLightData.z, lfCutOff, lLightPosIntensity.w);

		float lfLightFOV = lLightData.w;
	    if (lfLightFOV > -2.0)
	    {
              //Adjust position of spotLight
              vec4 lLightDir = Unpack_2_10_10_10(asuint(lLightData.x));
              lLightPosIntensity.xyz += lLightDir.xyz * lfLightRadius * 0.5;
	    }

		vec4 lViewSpacePositionVec4 = MUL(lUniforms.mpPerFrame.gViewMat4, vec4(lLightPosIntensity.xyz, 1.0));

		const bool inFrustum = sphereCone(lViewSpacePositionVec4.xyz, lfLightRadius, vec3(0,0,0), coneNormal, tileAngle);

		if (inFrustum)
		{

#if defined( D_USE_TILE_Z_EXTENTS )
            // update min/max Z
            const float wDistance = dot(coneNormal, lViewSpacePositionVec4.xyz);
            minZ = min(minZ, wDistance - lfLightRadius);
            maxZ = max(maxZ, wDistance + lfLightRadius);
#endif

			int liStoreLocation;
            #ifdef D_PLATFORM_METAL
			    bufferAtomicAddOut(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster), luTileBufferIndex, 1, liStoreLocation);
			#else
                imageAtomicAddOut(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster), liTileBase, 1, liStoreLocation);
            #endif
            liStoreLocation = liStoreLocation + 1; // Location '0' is reserved for light count in tile
#if defined( D_USE_TILE_Z_EXTENTS )
            liStoreLocation = liStoreLocation + 2; // Location '1/2' is reserved for tile depth min max
#endif

			// Insert the light index
			if (liStoreLocation < D_TILE_MAX_LIGHT_COUNT)
			{
                #ifdef D_PLATFORM_METAL
                    bufferAtomicStore(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster),
                    luTileBufferIndex + liStoreLocation,
                    luLightIndex);
                #else
                    imageStore(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster),
                    liTileBase + ivec2(liStoreLocation % D_TILE_WIDTH, liStoreLocation / D_TILE_WIDTH),
                    ivec4(luLightIndex,0,0,0));
                #endif
                
			}
		}
	}

#if defined( D_USE_TILE_Z_EXTENTS )
    bufferAtomicStore(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster), luTileBufferIndex + 1, as_int( minZ ));
    bufferAtomicStore(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster), luTileBufferIndex + 2, as_int( maxZ ));
#endif

#endif // !D_TILED_LIGHTS_DEBUG

#ifdef D_TILED_LIGHTS_COUNT_DEBUG
#ifdef D_PLATFORM_METAL
	int lightCount = bufferAtomicLoad(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster), luTileBufferIndex);
#else
    int lightCount = imageLoad(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gLightCluster), liTileBase).x;
#endif

    vec4 blue = vec4(0.0, 0.0, 1.0, 0.3);
    vec4 green = vec4(0.0, 1.0, 0.0, 0.3);
    vec4 orange = vec4(1.0, 1.0, 0.0, 0.3);
    vec4 red = vec4(1.0, 0.0, 0.0, 0.3);
    vec4 outColor;
    if(lightCount < 1) // 0 lights
        outColor = blue;
    else if(lightCount < 3) // 1-2 lights
        outColor = green;
    else if(lightCount < 9) // 3 - 8 lights
        outColor = orange;
    else // all above 8
        outColor = red;

	float lightNormalized = float(lightCount) / D_TILE_MAX_LIGHT_COUNT;

	WRITE_FRAGMENT_COLOUR(outColor);
#endif
}
#endif //D_TILED_LIGHTS
//TF_END