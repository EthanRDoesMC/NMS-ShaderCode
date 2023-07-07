////////////////////////////////////////////////////////////////////////////////
///
///     @file       UberFragmentShader.h
///     @author     User
///     @date       
///
///     @brief      UberFragmentShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 




//-----------------------------------------------------------------------------
//    Functions



//-----------------------------------------------------------------------------
///
///     WriteOutput
///
///     @brief      WriteOutput
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
void
WriteOutput(
    out vec4                      lOutColours0Vec4,
    out vec4                      lOutColours1Vec4,
    out vec4                      lOutColours2Vec4,
    out vec4                      lOutColours3Vec4,
    out vec4                      lOutColours4Vec4,
    in  PerFrameUniforms          lPerFrameUniforms,
    in  CommonPerMeshUniforms     lMeshUniforms,
    in  CustomPerMaterialUniforms lCustomUniforms,
    in  vec4                      lColourVec4,
    in  vec3                      lPositionVec3,
    in  vec3                      lNormalVec3,
    in  int                       liMaterialID,
    in  float                     lfMetallic,
    in  float                     lfRoughness,
    in  float                     lfSubsurface,
    in  float                     lfGlow,
    in  vec4                      lScreenSpacePositionVec4,
    in  vec4                      lPrevScreenPositionVec4,
    in  float                     lfPixelDepth,
    in  bool                      lbFrontFacing,
    in  float                     lfPixelSelfShadowing 
#ifdef D_OUTPUT_LINEARDEPTH
    ,in  float                    lfLinearDepth
#endif
)
{
    //-----------------------------------------------------------------------------
    ///
    ///     Output
    ///
    //-----------------------------------------------------------------------------	
    #if defined( D_IMPOSTER )
    if (lMeshUniforms.giShaderContextVariant == 0)
    {
        lOutColours0Vec4 = vec4(lColourVec4.x, lColourVec4.y, lColourVec4.z ,1.0);
        lOutColours1Vec4 = float2vec4(0.0);
        lOutColours2Vec4 = float2vec4(0.0);
        lOutColours3Vec4 = float2vec4(0.0);
        lOutColours4Vec4 = float2vec4(0.0);
    }
    else if (lMeshUniforms.giShaderContextVariant == 1 || lMeshUniforms.giShaderContextVariant == 2)
    {
        if(lMeshUniforms.giShaderContextVariant == 2)
        {
            mat3 lWorldNormalMat3 =
                mat3(normalize(vec3(lMeshUniforms.gWorldMat4[0][0], lMeshUniforms.gWorldMat4[0][1], lMeshUniforms.gWorldMat4[0][2])),
                    normalize(vec3(lMeshUniforms.gWorldMat4[1][0], lMeshUniforms.gWorldMat4[1][1], lMeshUniforms.gWorldMat4[1][2])),
                    normalize(vec3(lMeshUniforms.gWorldMat4[2][0], lMeshUniforms.gWorldMat4[2][1], lMeshUniforms.gWorldMat4[2][2])));

            lWorldNormalMat3 = transpose(lWorldNormalMat3);

            lNormalVec3 = MUL(lWorldNormalMat3, lNormalVec3);
        }
        else
        {
            //lNormalVec3.y *= -1.0;
        }

        vec3 lEncodedNormal = lNormalVec3;
#if !ENABLE_OCTAHEDRAL_IMPOSTERS
        lfPixelDepth = 1.0;
#endif
        lEncodedNormal   = ( lEncodedNormal + 1.0 ) * 0.5;
        lOutColours0Vec4 = vec4( lEncodedNormal.x, lEncodedNormal.y, lEncodedNormal.z, lfPixelDepth );
        lOutColours1Vec4 = float2vec4(0.0);
        lOutColours2Vec4 = float2vec4(0.0);
        lOutColours3Vec4 = float2vec4(0.0);
        lOutColours4Vec4 = float2vec4(0.0);
    }
    else  //if (lMeshUniforms.giShaderContextVariant == 2)
    {
#ifdef _F61_CLAMP_AMBIENT
        float lfClampAmbient = 1.0;
#else
        float lfClampAmbient = 0.1;
#endif
        lOutColours0Vec4 = vec4( lfSubsurface, lfRoughness, lfMetallic, lfClampAmbient );
        lOutColours1Vec4 = float2vec4(0.0);
        lOutColours2Vec4 = float2vec4(0.0);
        lOutColours3Vec4 = float2vec4(0.0);
        lOutColours4Vec4 = float2vec4(0.0);
    }
    #else
    {
        #if !defined( D_ATTRIBUTES )
            lOutColours0Vec4     = vec4( lColourVec4.xyz, lfGlow );
            lOutColours1Vec4     = float2vec4(0.0);
            lOutColours2Vec4     = float2vec4(0.0);
            lOutColours3Vec4     = float2vec4(0.0);
            lOutColours4Vec4     = float2vec4(0.0);
        #else

        #if defined( D_OUTPUT_MOTION_VECTORS )
            vec2  lScreenSpaceMotionVec2 = vec2( 1.0, 0.0 );
            if (HAS_MOTION_VECTORS)
            {
                lScreenSpaceMotionVec2 = lPrevScreenPositionVec4.xy / lPrevScreenPositionVec4.w - lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w;
            }
        #else   
            vec2  lScreenSpaceMotionVec2 = vec2( 1.0, 0.0 );
        #endif 

            vec4  lBuffer0_Vec4;
            vec4  lBuffer1_Vec4;
            vec4  lBuffer2_Vec4;
            vec4  lBuffer3_Vec4;
            vec4  lBuffer4_Vec4;

            EncodeGBuffer(
                lBuffer0_Vec4,
                lBuffer1_Vec4,
                lBuffer2_Vec4,
                lBuffer3_Vec4,
                lBuffer4_Vec4,
                lColourVec4.xyz,
                lPositionVec3,
                lNormalVec3,
                liMaterialID,
                lfRoughness,
                lfMetallic,
                lfSubsurface,
                lfGlow,
                lScreenSpaceMotionVec2,
                lfPixelSelfShadowing 
#ifdef D_OUTPUT_LINEARDEPTH
                ,lfLinearDepth
#endif
            );

            lOutColours0Vec4 = lBuffer0_Vec4;
            lOutColours1Vec4 = lBuffer1_Vec4;
            lOutColours2Vec4 = lBuffer2_Vec4;
            lOutColours3Vec4 = lBuffer3_Vec4;
            lOutColours4Vec4 = lBuffer4_Vec4;

        #endif
    }
    #endif
}

void
WriteOutputHalf(
    out half4                     lOutColours0Vec4,
    out half4                     lOutColours1Vec4,
    out half4                     lOutColours2Vec4,
    out half4                     lOutColours3Vec4,
    out half4                     lOutColours4Vec4,
    in  PerFrameUniforms          lPerFrameUniforms,
    in  CommonPerMeshUniforms     lMeshUniforms,
    in  CustomPerMaterialUniforms lCustomUniforms,
    in  half4                     lColourVec4,
    in  vec3                      lPositionVec3,
    in  half3                     lNormalVec3,
    in  int                       liMaterialID,
    in  half                      lfMetallic,
    in  half                      lfRoughness,
    in  half                      lfSubsurface,
    in  half                      lfGlow,
    in  vec4                      lScreenSpacePositionVec4,
    in  vec4                      lPrevScreenPositionVec4,
    in  half                      lfPixelDepth,
    in  bool                      lbFrontFacing,
    in  half                      lfPixelSelfShadowing )
{
    //-----------------------------------------------------------------------------
    ///
    ///     Output
    ///
    //-----------------------------------------------------------------------------	
    #if defined( D_IMPOSTER ) 
    if (lMeshUniforms.giShaderContextVariant == 0)
    {
        lOutColours0Vec4 = half4(lColourVec4.x, lColourVec4.y, lColourVec4.z ,1.0);
        lOutColours1Vec4 = float2half4(0.0);
        lOutColours2Vec4 = float2half4(0.0);
        lOutColours3Vec4 = float2half4(0.0);
        lOutColours4Vec4 = float2half4(0.0);
    }
    else if (lMeshUniforms.giShaderContextVariant == 1)
    {
        half3 lEncodedNormal;

        lEncodedNormal = lNormalVec3;

        lEncodedNormal    = ( lEncodedNormal + 1.0 ) * 0.5;
        lOutColours0Vec4 = half4( lEncodedNormal.x, lEncodedNormal.y, lEncodedNormal.z, 1.0 );
        lOutColours1Vec4 = float2half4(0.0);
        lOutColours2Vec4 = float2half4(0.0);
        lOutColours3Vec4 = float2half4(0.0);
        lOutColours4Vec4 = float2half4(0.0);
    }
    else if (lMeshUniforms.giShaderContextVariant == 2)
    {
#ifdef _F61_CLAMP_AMBIENT
        float lfClampAmbient = 1.0;
#else
        float lfClampAmbient = 0.1;
#endif

        lOutColours0Vec4 = half4( lfSubsurface, lfRoughness, lfMetallic, lfClampAmbient );
        lOutColours1Vec4 = float2half4(0.0);
        lOutColours2Vec4 = float2half4(0.0);
        lOutColours3Vec4 = float2half4(0.0);
        lOutColours4Vec4 = float2half4(0.0);
    }
    #else
    {
        #if !defined( D_ATTRIBUTES )
            lOutColours0Vec4     = half4( lColourVec4.xyz, lfGlow );
            lOutColours1Vec4     = float2half4(0.0);
            lOutColours2Vec4     = float2half4(0.0);
            lOutColours3Vec4     = float2half4(0.0);
            lOutColours4Vec4     = float2half4(0.0);
        #else

        #if defined( D_OUTPUT_MOTION_VECTORS )
            vec2  lScreenSpaceMotionVec2 = lPrevScreenPositionVec4.xy / lPrevScreenPositionVec4.w - lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w;
        #else   
            vec2  lScreenSpaceMotionVec2 = vec2( 1.0, 0.0 );
        #endif 

            half4  lBuffer0_Vec4;
            half4  lBuffer1_Vec4;
            half4  lBuffer2_Vec4;
            half4  lBuffer3_Vec4;
            half4  lBuffer4_Vec4;

            EncodeGBufferHalf(
                lBuffer0_Vec4,
                lBuffer1_Vec4,
                lBuffer2_Vec4,
                lBuffer3_Vec4,
                lBuffer4_Vec4,
                lColourVec4.xyz,
                lPositionVec3,
                lNormalVec3,
                liMaterialID,
                lfRoughness,
                lfMetallic,
                lfSubsurface,
                lfGlow,
                lScreenSpaceMotionVec2,
                lfPixelSelfShadowing );

            lOutColours0Vec4 = lBuffer0_Vec4;
            lOutColours1Vec4 = lBuffer1_Vec4;
            lOutColours2Vec4 = lBuffer2_Vec4;
            lOutColours3Vec4 = lBuffer3_Vec4;
            lOutColours4Vec4 = lBuffer4_Vec4;

        #endif
    }
    #endif
}
     