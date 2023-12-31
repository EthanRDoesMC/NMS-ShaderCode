// This file is part of the FidelityFX SDK.
//
// Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "Fullscreen/FidelityFX/ffx_fsr2_custom.h"
#include "Fullscreen/FidelityFX/ffx_fsr2_resources.h"

#if defined(FFX_GPU)
#include "Fullscreen/FidelityFX/ffx_core.h"
#endif // #if defined(FFX_GPU)

#if defined(FFX_GPU)
#ifndef FFX_FSR2_PREFER_WAVE64
#define FFX_FSR2_PREFER_WAVE64
#endif // #if defined(FFX_GPU)


#if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
#if  defined(FSR2_BIND_CB_FSR2)
	layout (set = 1, binding = FSR2_BIND_CB_FSR2, std140) uniform cbFSR2_t
	{
		FfxFloat32x2 fRenderOffsetUV;
		FfxInt32x2 iRenderOffset;
		FfxInt32x2 iRenderSize;
		FfxFloat32x2 fDisplayOffsetUV;
		FfxInt32x2 iDisplayOffset;
		FfxInt32x2 iDisplaySize;
		FfxUInt32x2 uLumaMipDimensions;
		FfxUInt32  uLumaMipLevelToUse;
		FfxUInt32  uFrameIndex;
		FfxFloat32x2 fDisplaySizeRcp;
		FfxFloat32x2 fJitter;
		FfxFloat32x4 fDeviceToViewDepth;
		FfxFloat32x2 depthclip_uv_scale;
		FfxFloat32x2 postprocessed_lockstatus_uv_scale;
		FfxFloat32x2 reactive_mask_dim_rcp;
		FfxFloat32x2 MotionVectorScale;
		FfxFloat32x2 fDownscaleFactor;
		FfxFloat32  fPreExposure;
		FfxFloat32  fTanHalfFOV;
		FfxFloat32x2 fMotionVectorJitterCancellation;
		FfxFloat32  fJitterSequenceLength;
		FfxFloat32  fLockInitialLifetime;
		FfxFloat32  fLockTickDelta;
		FfxFloat32  fDeltaTime;
		FfxFloat32  fDynamicResChangeFactor;
		FfxFloat32  fLumaMipRcp;
	} cbFSR2;
#endif // #if  defined(FSR2_BIND_CB_FSR2)
#endif // #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)

FfxFloat32 LumaMipRcp()
{
	return cbFSR2.fLumaMipRcp;
}

FfxUInt32x2 LumaMipDimensions()
{
	return cbFSR2.uLumaMipDimensions;
}

FfxUInt32  LumaMipLevelToUse()
{
	return cbFSR2.uLumaMipLevelToUse;
}

FfxFloat32x2 DownscaleFactor()
{
	return cbFSR2.fDownscaleFactor;
}

FfxFloat32x2 Jitter()
{
	return cbFSR2.fJitter;
}

FfxFloat32x2 MotionVectorJitterCancellation()
{
	return cbFSR2.fMotionVectorJitterCancellation;
}

FfxInt32x2 RenderSize()
{
	return cbFSR2.iRenderSize;
}

FfxInt32x2 RenderOffset()
{
	return cbFSR2.iRenderOffset;
}

FfxFloat32x2 RenderOffsetUV()
{
	return cbFSR2.fRenderOffsetUV;
}

FfxInt32x2 DisplaySize()
{
	return cbFSR2.iDisplaySize;
}

FfxInt32x2 DisplayOffset()
{
	return cbFSR2.iDisplayOffset;
}

FfxFloat32x2 DisplaySizeRcp()
{
	return cbFSR2.fDisplaySizeRcp;
}

FfxFloat32 JitterSequenceLength()
{
	return cbFSR2.fJitterSequenceLength;
}

FfxFloat32 LockInitialLifetime()
{
	return cbFSR2.fLockInitialLifetime;
}

FfxFloat32 LockTickDelta()
{
	return cbFSR2.fLockTickDelta;
}

FfxFloat32 DeltaTime()
{
	return cbFSR2.fDeltaTime;
}

FfxFloat32 DynamicResChangeFactor()
{
	return cbFSR2.fDynamicResChangeFactor;
}

FfxUInt32 FrameIndex()
{
	return cbFSR2.uFrameIndex;
}

#if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
layout (set = 0, binding = 0) uniform sampler s_PointClamp;
layout (set = 0, binding = 1) uniform sampler s_LinearClamp;
#endif

#define PREPARED_INPUT_COLOR_T FFX_MIN16_F4
#define PREPARED_INPUT_COLOR_F3 FFX_MIN16_F3
#define PREPARED_INPUT_COLOR_F1 FFX_MIN16_F

#define UPSAMPLED_COLOR_T FfxFloat32x3

#define RW_UPSAMPLED_WEIGHT_T FfxFloat32

#define LOCK_STATUS_T FFX_MIN16_F3
#define LOCK_STATUS_F1 FFX_MIN16_F

#if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
// SRVs
#if defined(FSR2_BIND_SRV_INPUT_COLOR)
	layout (set = 1, binding = FSR2_BIND_SRV_INPUT_COLOR) 						 uniform texture2D  r_input_color_jittered;
#endif
#if defined(FSR2_BIND_SRV_MOTION_VECTORS)
	layout (set = 1, binding = FSR2_BIND_SRV_MOTION_VECTORS) 				 	 uniform texture2D  r_motion_vectors;
#endif
#if defined(FSR2_BIND_SRV_DEPTH)
	layout (set = 1, binding = FSR2_BIND_SRV_DEPTH) 						 	 uniform texture2D  r_depth;                          
#endif
#if defined(FSR2_BIND_SRV_EXPOSURE)
	layout (set = 1, binding = FSR2_BIND_SRV_EXPOSURE) 							 uniform texture2D  r_exposure;
#endif
#if defined(FSR2_BIND_SRV_REACTIVE_MASK)
	layout (set = 1, binding = FSR2_BIND_SRV_REACTIVE_MASK) 				 	 uniform texture2D  r_reactive_mask;
#endif
#if defined(FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK)
	layout (set = 1, binding = FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK)  uniform texture2D  r_transparency_and_composition_mask;                  
#endif
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
	layout (set = 1, binding = FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH) 	 uniform utexture2D r_reconstructed_previous_nearest_depth;
#endif
#if defined(FSR2_BIND_SRV_DILATED_MOTION_VECTORS)
	layout (set = 1, binding = FSR2_BIND_SRV_DILATED_MOTION_VECTORS) 			 uniform texture2D  r_dilated_motion_vectors;         
#endif
#if defined(FSR2_BIND_SRV_DILATED_DEPTH)
	layout (set = 1, binding = FSR2_BIND_SRV_DILATED_DEPTH) 					 uniform texture2D  r_dilatedDepth;                   
#endif
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
	layout (set = 1, binding = FSR2_BIND_SRV_INTERNAL_UPSCALED) 			 	 uniform texture2D  r_internal_upscaled_color;        
#endif
#if defined(FSR2_BIND_SRV_LOCK_STATUS)
	layout (set = 1, binding = FSR2_BIND_SRV_LOCK_STATUS) 		 				 uniform texture2D  r_lock_status;                                
#endif
#if defined(FSR2_BIND_SRV_DEPTH_CLIP)
	layout (set = 1, binding = FSR2_BIND_SRV_DEPTH_CLIP) 						 uniform texture2D  r_depth_clip;                     
#endif
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
	layout (set = 1, binding = FSR2_BIND_SRV_PREPARED_INPUT_COLOR) 				 uniform texture2D  r_prepared_input_color;           
#endif
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
	layout (set = 1, binding = FSR2_BIND_SRV_LUMA_HISTORY) 						 uniform texture2D  r_luma_history;                   
#endif
#if defined(FSR2_BIND_SRV_RCAS_INPUT)
	layout (set = 1, binding = FSR2_BIND_SRV_RCAS_INPUT) 						 uniform texture2D  r_rcas_input;                     
#endif
#if defined(FSR2_BIND_SRV_LANCZOS_LUT)
	layout (set = 1, binding = FSR2_BIND_SRV_LANCZOS_LUT) 						 uniform texture2D  r_lanczos_lut;
#endif
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
	layout (set = 1, binding = FSR2_BIND_SRV_EXPOSURE_MIPS) 				 	 uniform texture2D  r_imgMips;
#endif
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS)
	layout(set = 1, binding = FSR2_BIND_SRV_DILATED_REACTIVE_MASKS) 		 	 uniform texture2D  r_dilated_reactive_masks;
#endif			 

// UAV
#if defined FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH
	layout (set = 1, binding = FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH, r32ui) uniform uimage2D   	    rw_reconstructed_previous_nearest_depth; 
#endif
#if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS
	layout (set = 1, binding = FSR2_BIND_UAV_DILATED_MOTION_VECTORS, rg32f) 		  uniform image2D   	    rw_dilated_motion_vectors;        
#endif
#if defined FSR2_BIND_UAV_DILATED_DEPTH
	layout (set = 1, binding = FSR2_BIND_UAV_DILATED_DEPTH, r32f) 					  uniform image2D   	    rw_dilatedDepth;                  
#endif
#if defined FSR2_BIND_UAV_INTERNAL_UPSCALED
	layout (set = 1, binding = FSR2_BIND_UAV_INTERNAL_UPSCALED, rgba32f) 		  	  uniform image2D   	    rw_internal_upscaled_color;       
#endif
#if defined FSR2_BIND_UAV_LOCK_STATUS
	layout (set = 1, binding = FSR2_BIND_UAV_LOCK_STATUS, r11f_g11f_b10f) 			  uniform image2D   	    rw_lock_status;              
#endif
#if defined FSR2_BIND_UAV_DEPTH_CLIP
	layout (set = 1, binding = FSR2_BIND_UAV_DEPTH_CLIP, r32f) 						  uniform image2D   	    rw_depth_clip;                    
#endif
#if defined FSR2_BIND_UAV_PREPARED_INPUT_COLOR
	layout (set = 1, binding = FSR2_BIND_UAV_PREPARED_INPUT_COLOR, rgba32f) 		  uniform image2D   	    rw_prepared_input_color;          
#endif
#if defined FSR2_BIND_UAV_LUMA_HISTORY
	layout (set = 1, binding = FSR2_BIND_UAV_LUMA_HISTORY, rgba32f) 				  uniform image2D   	    rw_luma_history;                  
#endif
#if defined FSR2_BIND_UAV_UPSCALED_OUTPUT
	layout (set = 1, binding = FSR2_BIND_UAV_UPSCALED_OUTPUT, rgba32f) 				  uniform image2D   	    rw_upscaled_output;               
#endif
#if defined FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE
	layout (set = 1, binding = FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE, r32f) 	  	  coherent uniform image2D  rw_img_mip_shading_change;
#endif
#if defined FSR2_BIND_UAV_EXPOSURE_MIP_5
	layout (set = 1, binding = FSR2_BIND_UAV_EXPOSURE_MIP_5, r32f) 					  coherent uniform image2D  rw_img_mip_5;
#endif
#if defined FSR2_BIND_UAV_DILATED_REACTIVE_MASKS
	layout (set = 1, binding = FSR2_BIND_UAV_DILATED_REACTIVE_MASKS, rg32f) 	 	  uniform image2D		    rw_dilated_reactive_masks;
#endif 
#if defined FSR2_BIND_UAV_EXPOSURE 
	layout (set = 1, binding = FSR2_BIND_UAV_EXPOSURE, rg32f) 					 	  uniform image2D 		    rw_exposure;
#endif 
#if defined FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC 
	layout (set = 1, binding = FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC, r32ui) 		 	 	  coherent uniform uimage2D rw_spd_global_atomic;
#endif

#endif // if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)

FfxFloat32 LoadMipLuma(FfxInt32x2 iPxPos, FfxUInt32 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
	return texelFetch(r_imgMips, iPxPos, FfxInt32(mipLevel)).r;
#else
    return 0.f;
#endif
}
#if FFX_HALF
FfxFloat16 LoadMipLuma(FfxInt16x2 iPxPos, FfxUInt16 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
	return FfxFloat16(texelFetch(r_imgMips, iPxPos, FfxInt32(mipLevel)).r);
#else
	return FfxFloat16(0.f);
#endif
}
#endif
FfxFloat32 SampleMipLuma(FfxFloat32x2 fUV, FfxUInt32 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
	fUV *= cbFSR2.depthclip_uv_scale;
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return textureLod(sampler2D(r_imgMips, s_LinearClamp), fUV, FfxFloat32(mipLevel)).r;
    #else
    return textureLod(r_imgMips, fUV, FfxFloat32(mipLevel)).r;
    #endif
#else
    return 0.f;
#endif
}
#if FFX_HALF
FfxFloat16 SampleMipLuma(FfxFloat16x2 fUV, FfxUInt32 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS)
	fUV *= FfxFloat16x2(cbFSR2.depthclip_uv_scale);
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return FfxFloat16(textureLod(sampler2D(r_imgMips, s_LinearClamp), fUV, FfxFloat32(mipLevel)).r);
    #else
    return FfxFloat16(textureLod(r_imgMips, fUV, FfxFloat32(mipLevel)).r);
    #endif
#else
    return FfxFloat16(0.f);
#endif
}
#endif


//
// a 0 0 0   x
// 0 b 0 0   y
// 0 0 c d   z
// 0 0 e 0   1
// 
// z' = (z*c+d)/(z*e)
// z' = (c/e) + d/(z*e)
// z' - (c/e) = d/(z*e)
// (z'e - c)/e = d/(z*e)
// e / (z'e - c) = (z*e)/d
// (e * d) / (z'e - c) = z*e
// z = d / (z'e - c)
FfxFloat32 ConvertFromDeviceDepthToViewSpace(FfxFloat32 fDeviceDepth)
{
	return -cbFSR2.fDeviceToViewDepth[2] / (fDeviceDepth * cbFSR2.fDeviceToViewDepth[1] - cbFSR2.fDeviceToViewDepth[0]);
}

FfxFloat32x2 EncodeMotionVector(FfxFloat32x2 fMotion)
{
	return fMotion * FFX_BROADCAST_FLOAT32X2(0.5f) + FFX_BROADCAST_FLOAT32X2(0.5f);
}

FfxFloat32x2 DecodeMotionVector(FfxFloat32x2 fMotion)
{
	return fMotion * FFX_BROADCAST_FLOAT32X2(2.0f) - FFX_BROADCAST_FLOAT32X2(1.0f);
}

LOCK_STATUS_F1 EncodeLockLifetime(LOCK_STATUS_F1 fLockLifetime)
{
#if FFX_FSR2_CUSTOM_OPTION_COMPACT_LOCKS
	return fLockLifetime / LOCK_STATUS_F1(LockInitialLifetime() * 2.0f);
#else
	return fLockLifetime + LOCK_STATUS_F1(LockInitialLifetime() * 2.0f);
#endif
}

LOCK_STATUS_F1 DecodeLockLifetime(LOCK_STATUS_F1 fLockLifetime)
{
#if FFX_FSR2_CUSTOM_OPTION_COMPACT_LOCKS
	return fLockLifetime * LOCK_STATUS_F1(LockInitialLifetime() * 2.0f);
#else
	return fLockLifetime - LOCK_STATUS_F1(LockInitialLifetime() * 2.0f);
#endif
}

FFX_MIN16_F DecodeLogLuminance(FFX_MIN16_F fLogLuminance)
{
    return -fLogLuminance * FFX_MIN16_F(3.0f);
}

FfxFloat32 LoadInputDepth(FfxInt32x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_DEPTH)
	return texelFetch(r_depth, iPxPos + RenderOffset(), 0).r;
#else
	return 0.f;
#endif
}

FfxFloat32 LoadReactiveMask(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_REACTIVE_MASK) 
	return texelFetch(r_reactive_mask, FfxInt32x2(iPxPos) + RenderOffset(), 0).r;
#else
	return 0.f;
#endif
}

FfxFloat32 SampleReactiveMask(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_REACTIVE_MASK)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return textureLod(sampler2D(r_reactive_mask, s_LinearClamp), fUV, 0.0f).r;
    #else
    return textureLod(r_reactive_mask, fUV, 0.0f).r;
    #endif
    #else
    return 0.f;
    #endif
}

FfxFloat32x4 GatherReactiveMask(FfxInt32x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_REACTIVE_MASK)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return textureGather(sampler2D(r_reactive_mask, s_LinearClamp), (FfxFloat32x2(iPxPos)  * cbFSR2.reactive_mask_dim_rcp) + RenderOffsetUV() , 0);
    #else
    return textureGather(r_reactive_mask, (FfxFloat32x2(iPxPos)  * cbFSR2.reactive_mask_dim_rcp) + RenderOffsetUV() , 0);
    #endif
#else
	return FfxFloat32x4(0.f);
#endif	
}

FFX_MIN16_F LoadTransparencyAndCompositionMask(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK)
	return FFX_MIN16_F(texelFetch(r_transparency_and_composition_mask, FfxInt32x2(iPxPos) + RenderOffset(), 0).r);
#else 
	return FFX_MIN16_F(0.f);
#endif
}

FfxFloat32 PreExposure()
{
#if FFX_FSR2_CUSTOM_OPTION_NO_PRE_EXPOSURE == 0
	return cbFSR2.fPreExposure;
#else
	return FfxFloat32(1.0f);
#endif
}

FfxFloat32x3 LoadInputColor(FfxInt32x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_INPUT_COLOR)
	return texelFetch(r_input_color_jittered, iPxPos + RenderOffset(), 0).rgb / PreExposure();
#else
	return FfxFloat32x3(0.f);
#endif
}

FfxFloat32x3 LoadInputColorWithoutPreExposure(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_INPUT_COLOR)
	return texelFetch(r_input_color_jittered, FfxInt32x2(iPxPos) + RenderOffset(), 0).rgb;
#else
    return FfxFloat32x3(0.f);
#endif
}

#if FFX_HALF
FfxFloat16x3 LoadPreparedInputColor(FfxInt16x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
	return FfxFloat16x3(texelFetch(r_prepared_input_color, FfxInt32x2(iPxPos), 0).rgb);
#else
	return FfxFloat16x3(0.f);
#endif
}
#endif // #if FFX_HALF

FFX_MIN16_F3 LoadPreparedInputColor(FfxInt32x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
	return FFX_MIN16_F3(texelFetch(r_prepared_input_color, iPxPos, 0).rgb);
#else
	return FFX_MIN16_F3(0.f);
#endif
}

FFX_MIN16_F3 SampleInputColor(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_INPUT_COLOR)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F3(textureLod(sampler2D(r_input_color_jittered, s_LinearClamp), fUV, 0.0f).rgb);
    #else
    return FFX_MIN16_F3(textureLod(r_input_color_jittered, fUV, 0.0f).rgb);
    #endif
    #else
    return FFX_MIN16_F3(0.f);
    #endif
}

FFX_MIN16_F3 SamplePreparedInputColor(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return FFX_MIN16_F3(textureLod(sampler2D(r_prepared_input_color, s_LinearClamp), fUV, 0.0f).rgb);
    #else
	return FFX_MIN16_F3(textureLod(r_prepared_input_color, fUV, 0.0f).rgb);
    #endif
#else
	return FFX_MIN16_F3(0.f);
#endif
}

FFX_MIN16_F SamplePreparedInputColorLuma(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F(textureLod(sampler2D(r_prepared_input_color_luma, s_LinearClamp), fUV, 0.0f).r);
    #else
    return FFX_MIN16_F(textureLod(r_prepared_input_color_luma, fUV, 0.0f).r);
    #endif
    #elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F(textureLod(sampler2D(r_prepared_input_color, s_LinearClamp), fUV, 0.0f).a);
    #else
    return FFX_MIN16_F(textureLod(r_prepared_input_color, fUV, 0.0f).a);
    #endif
    #else
    return FFX_MIN16_F(0.f);
    #endif
}

FFX_MIN16_F LoadPreparedInputColorLuma(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
	return FFX_MIN16_F(texelFetch(r_prepared_input_color_luma, iPxPos, 0).r);
#elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    return FFX_MIN16_F(texelFetch(r_prepared_input_color, iPxPos, 0).a);
#else
	return FFX_MIN16_F(0.f);
#endif
}

FFX_MIN16_F LoadPreparedInputLogLuminance(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE)
	return FFX_MIN16_F(texelFetch(r_prepared_input_log_luminance, iPxPos, 0).r);
#else
	return FFX_MIN16_F(0.f);
#endif
}

FFX_MIN16_F4 GatherPreparedInputColorLuma(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F4(textureGather(sampler2D(r_prepared_input_color_luma, s_LinearClamp), fUV, 0));
    #else
    return FFX_MIN16_F4(textureGather(r_prepared_input_color_luma, fUV, 0));
    #endif
#elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F4(textureGather(sampler2D(r_prepared_input_color, s_LinearClamp), fUV, 3));
    #else
    return FFX_MIN16_F4(textureGather(r_prepared_input_color, fUV, 3));
    #endif
#else
    return FFX_MIN16_F4(0.f);
#endif
}

FFX_MIN16_F4 GatherOffsetPreparedInputColorLuma(FfxFloat32x2 fUV, FFX_MIN16_I2 iOffset)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F4(textureGatherOffset(sampler2D(r_prepared_input_color_luma, s_LinearClamp), fUV, iOffset, 0));
    #else
    return FFX_MIN16_F4(textureGatherOffset(r_prepared_input_color_luma, fUV, iOffset, 0));
    #endif
#elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F4(textureGatherOffset(sampler2D(r_prepared_input_color, s_LinearClamp), fUV, iOffset, 3));
    #else
    return FFX_MIN16_F4(textureGatherOffset(r_prepared_input_color, fUV, iOffset, 3));
    #endif
#else
    return FFX_MIN16_F4(0.f);
#endif
}

FFX_MIN16_F4 GatherPreparedInputLogLuminance(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return textureGather(sampler2D(r_prepared_input_log_luminance, s_LinearClamp), fUV, 0);
    #else
	return textureGather(r_prepared_input_log_luminance, fUV, 0);
    #endif
#else
    return FFX_MIN16_F4(0.f);
#endif
}

FFX_MIN16_F SamplePreparedInputLogLuminance(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return DecodeLogLuminance(textureLod(sampler2D(r_prepared_input_log_luminance, s_LinearClamp), fUV, 0.0f).r);
    #else
	return DecodeLogLuminance(textureLod(r_prepared_input_log_luminance, fUV, 0.0f).r);
    #endif
#else
    return FFX_MIN16_F(0.f);
#endif
}

FfxFloat32x2 LoadInputMotionVector(FFX_MIN16_I2 iPxDilatedMotionVectorPos)
{
#if defined(FSR2_BIND_SRV_MOTION_VECTORS)
	FfxFloat32x2 fSrcMotionVector = texelFetch(r_motion_vectors, FfxInt32x2(iPxDilatedMotionVectorPos) + RenderOffset(), 0).xy;
#else
	FfxFloat32x2 fSrcMotionVector = FfxFloat32x2(0.f);
#endif

#if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
	FfxFloat32x2 fUvMotionVector = DecodeMotionVector(fSrcMotionVector);
#else
	FfxFloat32x2 fUvMotionVector = fSrcMotionVector * cbFSR2.MotionVectorScale;
#endif

#if FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
	fUvMotionVector -= cbFSR2.fMotionVectorJitterCancellation;
#endif

	return fUvMotionVector;
}

FFX_MIN16_F4 LoadHistory(FfxInt32x2 iPxHistory)
{
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
	return FFX_MIN16_F4(texelFetch(r_internal_upscaled_color, iPxHistory, 0));
#else
	return FFX_MIN16_F4(0.f);
#endif
}

FFX_MIN16_F3 LoadHistoryColor(FfxInt32x2 iPxHistory)
{
    #if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
    return FFX_MIN16_F3(texelFetch(r_internal_upscaled_color, iPxHistory, 0).rgb);
    #else
    return FFX_MIN16_F3(0.f);
    #endif
}

FFX_MIN16_F LoadHistoryWeight(FfxInt32x2 iPxHistory)
{
    #if   defined(FSR2_BIND_SRV_INTERNAL_UPSCALED_WEIGHT)
    return FFX_MIN16_F(texelFetch(r_internal_upscaled_color_weight, iPxHistory, 0).r);
    #elif defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
    return FFX_MIN16_F(texelFetch(r_internal_upscaled_color, iPxHistory, 0).w);
    #else
    return FFX_MIN16_F(0.f);
    #endif
}

FFX_MIN16_F4 SampleHistory(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return FFX_MIN16_F4(textureLod(sampler2D(r_internal_upscaled_color, s_LinearClamp), fUV, 0.0f));
    #else
	return FFX_MIN16_F4(textureLod(r_internal_upscaled_color, fUV, 0.0f));
    #endif
#else
    return FFX_MIN16_F4(0.f);
#endif
}

FFX_MIN16_F3 SampleHistoryColor(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F3(textureLod(sampler2D(r_internal_upscaled_color, s_LinearClamp), fUV, 0.0f).rgb);
    #else
    return FFX_MIN16_F3(textureLod(r_internal_upscaled_color, fUV, 0.0f).rgb);
    #endif
    #else
    return FFX_MIN16_F3(0.f);
    #endif
}

FFX_MIN16_F SampleHistoryWeight(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED_WEIGHT)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F(textureLod(sampler2D(r_internal_upscaled_color_weight, s_LinearClamp), fUV, 0.0f).r);
    #else
    return FFX_MIN16_F(textureLod(r_internal_upscaled_color_weight, fUV, 0.0f).r);
    #endif
    #elif defined(FSR2_BIND_SRV_INTERNAL_UPSCALED)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FFX_MIN16_F(textureLod(sampler2D(r_internal_upscaled_color, s_LinearClamp), fUV, 0.0f).w);
    #else
    return FFX_MIN16_F(textureLod(r_internal_upscaled_color, fUV, 0.0f).w);
    #endif
    #else
    return FFX_MIN16_F(0.f);
    #endif
}

FfxFloat32x4 LoadRwInternalUpscaledColorAndWeight(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_UAV_INTERNAL_UPSCALED)
	return imageLoad(rw_internal_upscaled_color, FfxInt32x2(iPxPos));
#else
	return FfxFloat32x4(0.f);
#endif
}

void StoreReducedLuminance(FFX_MIN16_I2 iPxPos, FfxFloat32 fReducedLuminance)
{
#if defined(FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE)
	imageStore(rw_img_mip_shading_change, iPxPos, vec4(fReducedLuminance, 0.0f, 0.0f, 0.0f));
#endif
}

void StoreLumaHistory(FFX_MIN16_I2 iPxPos, FfxFloat32x4 fLumaHistory)
{
#if defined(FSR2_BIND_UAV_LUMA_HISTORY)
	imageStore(rw_luma_history, FfxInt32x2(iPxPos), fLumaHistory);
#endif
}

FfxFloat32x4 LoadRwLumaHistory(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_UAV_LUMA_HISTORY)
	return imageLoad(rw_luma_history, FfxInt32x2(iPxPos) );
#else
	return FfxFloat32x4(1.f);
#endif
}

FfxFloat32x4 LoadLumaHistory(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
    return texelFetch(r_luma_history, FfxInt32x2(iPxPos), 0);
#else
    return FfxFloat32x4(1.f);
#endif
}

void StoreLumaStabilityFactor(FFX_MIN16_I2 iPxPos, FfxFloat32 fLumaStability)
{
#if defined(FSR2_BIND_UAV_LUMA_STABILITY)
	imageStore(rw_luma_stability, FfxInt32x2(iPxPos), FfxFloat32x4(fLumaStability, 0.0f, 0.0f, 0.0f));
#endif
}

FfxFloat32 LoadLumaStabilityFactor(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
	return texelFetch(r_luma_history, FfxInt32x2(iPxPos) , 0).w;
#else
    return 0.f;
#endif
}

FfxFloat32x4 SampleLumaHistory(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
    fUV *= cbFSR2.depthclip_uv_scale;
#if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return textureLod(sampler2D(r_luma_history, s_LinearClamp), fUV, 0.0f);
#else
    return textureLod(r_luma_history, fUV, 0.0f);
#endif
#else
    return FfxFloat32x4(1.f);
#endif
}

FfxFloat32 SampleLumaStabilityFactor(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
	fUV *= cbFSR2.depthclip_uv_scale;
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return textureLod(sampler2D(r_luma_history, s_LinearClamp), fUV, 0.0f).w;
    #else
	return textureLod(r_luma_history, fUV, 0.0f).w;
    #endif
#else
    return 0.f;
#endif
}

void StoreReprojectedHistory(FFX_MIN16_I2 iPxHistory, FFX_MIN16_F4 fHistory)
{
#if defined(FSR2_BIND_UAV_INTERNAL_UPSCALED)
	imageStore(rw_internal_upscaled_color, FfxInt32x2(iPxHistory) , fHistory);
#endif
}

void StoreInternalColorAndWeight(FFX_MIN16_I2 iPxPos, FfxFloat32x4 fColorAndWeight)
{
#if defined(FSR2_BIND_UAV_INTERNAL_UPSCALED)
	imageStore(rw_internal_upscaled_color, FfxInt32x2(iPxPos), fColorAndWeight);
#endif
}

void StoreUpscaledOutput(FFX_MIN16_I2 iPxPos, FfxFloat32x3 fColor)
{
#if defined(FSR2_BIND_UAV_UPSCALED_OUTPUT)
	FfxFloat32x3 fOutputColor = fColor * PreExposure();
    imageStore(rw_upscaled_output, FfxInt32x2(iPxPos) + DisplayOffset(), FfxFloat32x4(fOutputColor, 1.f));
#endif
}

LOCK_STATUS_T LoadLockStatus(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_LOCK_STATUS)
	LOCK_STATUS_T fLockStatus = LOCK_STATUS_T(texelFetch(r_lock_status, iPxPos, 0).rgb);

    fLockStatus[0] = DecodeLockLifetime(fLockStatus[0]);

    return fLockStatus;
#else
	return LOCK_STATUS_T(0.f);
#endif
}

LOCK_STATUS_T LoadRwLockStatus(FfxInt32x2 iPxPos)
{
#if defined(FSR2_BIND_UAV_LOCK_STATUS) || defined(FSR2_BIND_UAV_LOCK_STATUS_UNORM)
	LOCK_STATUS_T fLockStatus = LOCK_STATUS_T(imageLoad(rw_lock_status, iPxPos ).rgb);

    fLockStatus[0] = DecodeLockLifetime(fLockStatus[0]);

    return fLockStatus;
#else
    return LOCK_STATUS_T(0.f);
#endif
}

void StoreLockStatus(FFX_MIN16_I2 iPxPos, LOCK_STATUS_T fLockStatus)
{
#if defined(FSR2_BIND_UAV_LOCK_STATUS) || defined(FSR2_BIND_UAV_LOCK_STATUS_UNORM)
	fLockStatus[0] = EncodeLockLifetime(fLockStatus[0]);

	imageStore(rw_lock_status, iPxPos, vec4(fLockStatus, 0.0f));
#endif
}

void StoreLockStatus(FFX_MIN16_I2 iPxPos, LOCK_STATUS_T fLockStatus, FfxFloat32 fEdgeStatus)
{
    #if defined(FSR2_BIND_UAV_LOCK_STATUS) || defined(FSR2_BIND_UAV_LOCK_STATUS_UNORM)
    fLockStatus[0] = EncodeLockLifetime(fLockStatus[0]);

    imageStore(rw_lock_status, iPxPos, vec4(fLockStatus, fEdgeStatus));
    #endif
}

FFX_MIN16_F LoadEdgeStatus(FFX_MIN16_I2 iPxPos)
{
    #if defined(FSR2_BIND_SRV_LOCK_STATUS)
    return FFX_MIN16_F(texelFetch(r_lock_status, iPxPos, 0).a);
#else
    return FFX_MIN16_F(0);
#endif
}

void StorePreparedInputColor(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN PREPARED_INPUT_COLOR_T fTonemapped)
{
#if   defined(FSR2_BIND_UAV_PREPARED_INPUT_COLOR)
	imageStore(rw_prepared_input_color, iPxPos, fTonemapped);
#endif
}

void StorePreparedInputColorLuma(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN PREPARED_INPUT_COLOR_F1 fLuma)
{
#if defined(FSR2_BIND_UAV_PREPARED_INPUT_COLOR_LUMA)
	imageStore(rw_prepared_input_color_luma, iPxPos, vec4(fLuma, 0.0f, 0.0f, 0.0f));
#endif
}

FfxBoolean IsResponsivePixel(FfxInt32x2 iPxPos)
{
	return FFX_FALSE; //not supported in prototype
}

FfxFloat32 LoadDepthClip(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_DEPTH_CLIP)
	return texelFetch(r_depth_clip, iPxPos, 0).r;
#else
    return 0.f;
#endif
}

FfxFloat32 SampleDepthClip(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_DEPTH_CLIP)
	fUV *= cbFSR2.depthclip_uv_scale;
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return textureLod(sampler2D(r_depth_clip, s_LinearClamp), fUV, 0.0f).r;
    #else
	return textureLod(r_depth_clip, fUV, 0.0f).r;
    #endif
#else
    return 0.f;
#endif
}

LOCK_STATUS_T SampleLockStatus(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_LOCK_STATUS)
	fUV *= cbFSR2.postprocessed_lockstatus_uv_scale;
	//fUV += FfxFloat32x2(DisplayOffset()) * DisplaySizeRcp();
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	LOCK_STATUS_T fLockStatus = LOCK_STATUS_T(textureLod(sampler2D(r_lock_status, s_LinearClamp), fUV, 0.0f).rgb);
    #else
	LOCK_STATUS_T fLockStatus = LOCK_STATUS_T(textureLod(r_lock_status, fUV, 0.0f).rgb);
    #endif
    fLockStatus[0] = DecodeLockLifetime(fLockStatus[0]);
	return fLockStatus;
#else
    return LOCK_STATUS_T(0.f);
#endif
}

void StoreDepthClip(FFX_MIN16_I2 iPxPos, FfxFloat32 fClip)
{
#if defined(FSR2_BIND_UAV_DEPTH_CLIP)
	imageStore(rw_depth_clip, iPxPos, vec4(fClip, 0.0f, 0.0f, 0.0f));
#endif
}

FfxFloat32 TanHalfFoV()
{
	return cbFSR2.fTanHalfFOV;
}

FfxFloat32 LoadSceneDepth(FFX_MIN16_I2 iPxInput)
{
#if defined(FSR2_BIND_SRV_DEPTH)
	return texelFetch(r_depth, iPxInput + RenderOffset(), 0).r;
#else
    return 0.f;
#endif
}

FfxFloat32 LoadReconstructedPrevDepth(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
	return uintBitsToFloat(FfxUInt32(texelFetch(r_reconstructed_previous_nearest_depth, iPxPos, 0).r));
#else 
    return 0.f;
#endif
}

FfxFloat32x4 GatherReconstructedPrevDepth(FfxFloat32x2 fUv)
{
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    FfxUInt32x4 uDepths = textureGather(sampler2D(r_reconstructed_previous_nearest_depth, s_LinearClamp), fUv , 0);
    #else
    FfxUInt32x4 uDepths = textureGather(r_reconstructed_previous_nearest_depth, fUv , 0);
    #endif
    return FfxFloat32x4(uintBitsToFloat(uDepths.x), uintBitsToFloat(uDepths.y), uintBitsToFloat(uDepths.z), uintBitsToFloat(uDepths.w));
#else 
	return FfxFloat32x4(0.f);
#endif
}

void StoreReconstructedDepth(FFX_MIN16_I2 iPxSample, FfxFloat32 fDepth)
{
	FfxUInt32 uDepth = floatBitsToUint(fDepth);
#if defined(FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
	#if FFX_FSR2_OPTION_INVERTED_DEPTH
		imageAtomicMax(rw_reconstructed_previous_nearest_depth, iPxSample, uDepth);
	#else
		imageAtomicMin(rw_reconstructed_previous_nearest_depth, iPxSample, uDepth); // min for standard, max for inverted depth
	#endif
#endif
}

void SetReconstructedDepth(FFX_MIN16_I2 iPxSample, FfxUInt32 uValue)
{
#if defined(FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
	imageStore(rw_reconstructed_previous_nearest_depth, iPxSample, uvec4(uValue, 0, 0, 0));
#endif
}

void StoreDilatedDepth(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN FfxFloat32 fDepth)
{
#if defined(FSR2_BIND_UAV_DILATED_DEPTH)
	//FfxUInt32 uDepth = f32tof16(fDepth);
	imageStore(rw_dilatedDepth, iPxPos, vec4(fDepth, 0.0f, 0.0f, 0.0f));
#endif
}

void StoreDilatedMotionVector(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN FfxFloat32x2 fMotionVector)
{
#if defined(FSR2_BIND_UAV_DILATED_MOTION_VECTORS) || defined(FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM) 
    #if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
	imageStore(rw_dilated_motion_vectors, iPxPos, vec4(EncodeMotionVector(fMotionVector), 0.0f, 0.0f));
    #else
	imageStore(rw_dilated_motion_vectors, iPxPos, vec4(fMotionVector, 0.0f, 0.0f));
    #endif
#endif
}

FfxFloat32x2 LoadDilatedMotionVector(FFX_MIN16_I2 iPxInput)
{
#if defined(FSR2_BIND_SRV_DILATED_MOTION_VECTORS)
	#if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
	return DecodeMotionVector(texelFetch(r_dilated_motion_vectors, iPxInput, 0).rg);
	#else
	return texelFetch(r_dilated_motion_vectors, iPxInput, 0).rg;
    #endif
#else 
    return FfxFloat32x2(0.f);
#endif
}


FfxFloat32 LoadDilatedDepth(FFX_MIN16_I2 iPxInput)
{
#if defined(FSR2_BIND_SRV_DILATED_DEPTH)
	return texelFetch(r_dilatedDepth, iPxInput, 0).r;
#else
    return 0.f;
#endif
}

FfxFloat32 Exposure()
{
	#if defined(FSR2_BIND_SRV_EXPOSURE)
		FfxFloat32 exposure = texelFetch(r_exposure, FfxInt32x2(0,0), 0).x;
	#else
		FfxFloat32 exposure = 1.f;
	#endif

	if (exposure == 0.0f) {
		exposure = 1.0f;
	}

	return exposure;
}

FfxFloat32 SampleLanczos2Weight(FfxFloat32 x)
{
#if defined(FSR2_BIND_SRV_LANCZOS_LUT)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return textureLod(sampler2D(r_lanczos_lut, s_LinearClamp), FfxFloat32x2(x / 2.0f, 0.5f), 0.0f).x; 
    #else
	return textureLod(r_lanczos_lut, FfxFloat32x2(x / 2.0f, 0.5f), 0.0f).x; 
    #endif
#else
    return 0.f;
#endif
}

#if FFX_HALF
FfxFloat16 SampleLanczos2Weight(FfxFloat16 x)
{
#if defined(FSR2_BIND_SRV_LANCZOS_LUT)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return FfxFloat16(textureLod(sampler2D(r_lanczos_lut, s_LinearClamp), FfxFloat32x2(x / 2.0f, 0.5f), 0.0f).x);
    #else
	return FfxFloat16(textureLod(r_lanczos_lut, FfxFloat32x2(x / 2.0f, 0.5f), 0.0f).x);
    #endif
#else
    return FfxFloat16(0.f);
#endif
}
#endif

FfxFloat32 SampleLanczos2SumWeight(FfxFloat32x3 x)
{
#if defined(FSR2_BIND_SRV_LANCZOS_SUM_LUT)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return textureLod(sampler3D(r_lanczos_sum_lut, s_LinearClamp), x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x;
    #else
    return textureLod(r_lanczos_sum_lut, x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x;
    #endif
#else
    return 0.f;
#endif
}

#if FFX_HALF
FfxFloat16 SampleLanczos2SumWeight(FfxFloat16 x)
{
#if defined(FSR2_BIND_SRV_LANCZOS_SUM_LUT)
    #if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return FfxFloat16(textureLod(sampler3D(r_lanczos_sum_lut, s_LinearClamp), x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x);
    #else
    return FfxFloat16(textureLod(r_lanczos_sum_lut, x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x);
    #endif
#else
    return FfxFloat16(0.f);
#endif
}
#endif

FfxFloat32x2 SampleDilatedReactiveMasks(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS)
	fUV *= cbFSR2.depthclip_uv_scale; // TODO: assuming these are (RenderSize() / MaxRenderSize())
	#if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
	return textureLod(sampler2D(r_dilated_reactive_masks, s_LinearClamp), fUV, 0.0f).rg;
	#else
	return textureLod(r_dilated_reactive_masks, fUV, 0.0f).rg;
	#endif
#else
	return FfxFloat32x2(0.f);
#endif
}

FfxFloat32x2 LoadDilatedReactiveMasks(FFX_PARAMETER_IN FfxInt32x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS)
    return texelFetch(r_dilated_reactive_masks, iPxPos, 0).rg;
#else
    return FfxFloat32x2(0.f);
#endif
}

void StoreDilatedReactiveMasks(FFX_PARAMETER_IN FfxInt32x2 iPxPos, FFX_PARAMETER_IN FfxFloat32x2 fDilatedReactiveMasks)
{
#if defined(FSR2_BIND_UAV_DILATED_REACTIVE_MASKS)
    imageStore(rw_dilated_reactive_masks, iPxPos, vec4(fDilatedReactiveMasks, 0.0f, 0.0f));
#endif
}

#endif // #if defined(FFX_GPU)
