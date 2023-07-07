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
#ifdef __hlsl_dx_compiler
#pragma dxc diagnostic push
#pragma dxc diagnostic ignored "-Wambig-lit-shift"
#endif //__hlsl_dx_compiler
#include "Fullscreen/FidelityFX/ffx_core.h"
#ifdef __hlsl_dx_compiler
#pragma dxc diagnostic pop
#endif //__hlsl_dx_compiler
#endif // #if defined(FFX_GPU)

#if defined(FFX_GPU)
#ifndef FFX_FSR2_PREFER_WAVE64
#define FFX_FSR2_PREFER_WAVE64
#endif // #if defined(FFX_GPU)

#if defined(FFX_GPU)
#pragma warning(disable: 3205)  // conversion from larger type to smaller
#endif // #if defined(FFX_GPU)

#define DECLARE_SRV_REGISTER(regIndex)  t##regIndex
#define DECLARE_UAV_REGISTER(regIndex)  u##regIndex
#define DECLARE_CB_REGISTER(regIndex)   b##regIndex
#define FFX_FSR2_DECLARE_SRV(regIndex)  register(DECLARE_SRV_REGISTER(regIndex))
#define FFX_FSR2_DECLARE_UAV(regIndex)  register(DECLARE_UAV_REGISTER(regIndex))
#define FFX_FSR2_DECLARE_CB(regIndex)   register(DECLARE_CB_REGISTER(regIndex))

#if defined(FSR2_BIND_CB_FSR2)
    cbuffer cbFSR2 : FFX_FSR2_DECLARE_CB(FSR2_BIND_CB_FSR2)
    {
	FfxFloat32x2 fRenderOffsetUV;
	FfxInt32x2 iRenderOffset;
	FfxInt32x2 iRenderSize;
	FfxFloat32x2 fDisplayOffsetUV;
	FfxInt32x2 iDisplayOffset;
	FfxInt32x2 iDisplaySize;
        FfxUInt32x2  uLumaMipDimensions;
        FfxUInt32   uLumaMipLevelToUse;
        FfxUInt32   uFrameIndex;
        FfxFloat32x2  fDisplaySizeRcp;
        FfxFloat32x2  fJitter;
        FfxFloat32x4  fDeviceToViewDepth;
        FfxFloat32x2  depthclip_uv_scale;
        FfxFloat32x2  postprocessed_lockstatus_uv_scale;
        FfxFloat32x2  reactive_mask_dim_rcp;
        FfxFloat32x2  MotionVectorScale;
        FfxFloat32x2  fDownscaleFactor;
        FfxFloat32   fPreExposure;
        FfxFloat32   fTanHalfFOV;
        FfxFloat32x2  fMotionVectorJitterCancellation;
        FfxFloat32   fJitterSequenceLength;
        FfxFloat32   fLockInitialLifetime;
        FfxFloat32   fLockTickDelta;
        FfxFloat32   fDeltaTime;
        FfxFloat32   fDynamicResChangeFactor;
        FfxFloat32   fLumaMipRcp;
#if defined(FFX_FSR2_CUSTOM_IMPLEMENTATION_SINGLECB)
        uint    mips;
        uint    numWorkGroups;
        uint2   workGroupOffset;
        uint2   renderSize;

        uint4   rcasConfig;
        #define FFX_FSR2_CONSTANT_BUFFER_1_SIZE 54  // Number of 32-bit values. This must be kept in sync with the cbFSR2 size.
#else
        #define FFX_FSR2_CONSTANT_BUFFER_1_SIZE 44  // Number of 32-bit values. This must be kept in sync with the cbFSR2 size.
#endif

    };
#else
    #define iRenderSize                         0
    #define iDisplaySize                        0
    #define uLumaMipDimensions                  0
    #define uLumaMipLevelToUse                  0
    #define uFrameIndex                         0
    #define fDisplaySizeRcp                     0
    #define fJitter                             0
    #define fDeviceToViewDepth                  FfxFloat32x4(0,0,0,0)
    #define depthclip_uv_scale                  0
    #define postprocessed_lockstatus_uv_scale   0
    #define reactive_mask_dim_rcp               0
    #define MotionVectorScale                   0
    #define fDownscaleFactor                    0
    #define fPreExposure                        0
    #define fTanHalfFOV                         0
    #define fMotionVectorJitterCancellation     0
    #define fJitterSequenceLength               0
    #define fLockInitialLifetime                0
    #define fLockTickDelta                      0
    #define fDeltaTime                          0
    #define fDynamicResChangeFactor             0
    #define fLumaMipRcp                         0
#endif

#if defined(FFX_GPU)
#define FFX_FSR2_ROOTSIG_STRINGIFY(p) FFX_FSR2_ROOTSIG_STR(p)
#define FFX_FSR2_ROOTSIG_STR(p) #p
#define FFX_FSR2_ROOTSIG [RootSignature("CBV(b0, space = 0,visibility=SHADER_VISIBILITY_ALL),"\
                                    "DescriptorTable( SRV(t0, numDescriptors=25, space = 0),  UAV(u0, numDescriptors=8),  visibility=SHADER_VISIBILITY_ALL)," \
                                    "DescriptorTable( Sampler(s0, numDescriptors=25),  visibility=SHADER_VISIBILITY_ALL), " \
                                    "StaticSampler(s0,space = 1, filter = FILTER_MIN_MAG_MIP_POINT, " \
                                                        "addressU = TEXTURE_ADDRESS_CLAMP, " \
                                                        "addressV = TEXTURE_ADDRESS_CLAMP, " \
                                                        "addressW = TEXTURE_ADDRESS_CLAMP, " \
                                                        "comparisonFunc = COMPARISON_NEVER, " \
                                                        "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK), " \
                                         "StaticSampler(s1, space = 1, filter = FILTER_MIN_MAG_MIP_LINEAR, " \
                                                        "addressU = TEXTURE_ADDRESS_CLAMP, " \
                                                        "addressV = TEXTURE_ADDRESS_CLAMP, " \
                                                        "addressW = TEXTURE_ADDRESS_CLAMP, " \
                                                        "comparisonFunc = COMPARISON_NEVER, " \
                                                        "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK)" )]

#define FFX_FSR2_CONSTANT_BUFFER_2_SIZE 6  // Number of 32-bit values. This must be kept in sync with max( cbRCAS , cbSPD) size.

#define FFX_FSR2_CB2_ROOTSIG [RootSignature("CBV(b0, space = 0,visibility=SHADER_VISIBILITY_ALL),"\
                                        "DescriptorTable( SRV(t0, numDescriptors=25, space = 0),  UAV(u0, numDescriptors=8),  visibility=SHADER_VISIBILITY_ALL)," \
                                        "DescriptorTable( Sampler(s0, numDescriptors=25),  visibility=SHADER_VISIBILITY_ALL), " \
                                        "StaticSampler(s0,space = 1, filter = FILTER_MIN_MAG_MIP_POINT, " \
                                         "addressU = TEXTURE_ADDRESS_CLAMP, " \
                                         "addressV = TEXTURE_ADDRESS_CLAMP, " \
                                         "addressW = TEXTURE_ADDRESS_CLAMP, " \
                                         "comparisonFunc = COMPARISON_NEVER, " \
                                         "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK), " \
                                         "StaticSampler(s1, space = 1, filter = FILTER_MIN_MAG_MIP_LINEAR, " \
                                         "addressU = TEXTURE_ADDRESS_CLAMP, " \
                                         "addressV = TEXTURE_ADDRESS_CLAMP, " \
                                         "addressW = TEXTURE_ADDRESS_CLAMP, " \
                                         "comparisonFunc = COMPARISON_NEVER, " \
                                         "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK)" )]

#if defined(FFX_FSR2_EMBED_ROOTSIG)
#define FFX_FSR2_EMBED_ROOTSIG_CONTENT FFX_FSR2_ROOTSIG
#define FFX_FSR2_EMBED_CB2_ROOTSIG_CONTENT FFX_FSR2_CB2_ROOTSIG
#else
#define FFX_FSR2_EMBED_ROOTSIG_CONTENT
#define FFX_FSR2_EMBED_CB2_ROOTSIG_CONTENT
#endif // #if FFX_FSR2_EMBED_ROOTSIG
#endif // #if defined(FFX_GPU)


FfxFloat32 LumaMipRcp()
{
    return fLumaMipRcp;
}

uint2 LumaMipDimensions()
{
    return uLumaMipDimensions;
}

FfxUInt32  LumaMipLevelToUse()
{
    return uLumaMipLevelToUse;
}

FfxFloat32x2 DownscaleFactor()
{
    return fDownscaleFactor;
}

FfxFloat32x2 Jitter()
{
    return fJitter;
}

FfxFloat32x2 MotionVectorJitterCancellation()
{
    return fMotionVectorJitterCancellation;
}

int2 RenderSize()
{
    return iRenderSize;
}

int2 RenderOffset()
{
	return iRenderOffset;
}

FfxFloat32x2 RenderOffsetUV()
{
	return fRenderOffsetUV;
}

int2 DisplaySize()
{
    return iDisplaySize;
}

FfxFloat32x2 DisplaySizeRcp()
{
    return fDisplaySizeRcp;
}

FfxFloat32 JitterSequenceLength()
{
    return fJitterSequenceLength;
}

FfxFloat32 LockInitialLifetime()
{
    return fLockInitialLifetime;
}

FfxFloat32 LockTickDelta()
{
    return fLockTickDelta;
}

FfxFloat32 DeltaTime()
{
    return fDeltaTime;
}

FfxFloat32 DynamicResChangeFactor()
{
    return fDynamicResChangeFactor;
}

FfxUInt32 FrameIndex()
{
    return uFrameIndex;
}

#if defined( D_PLATFORM_PROSPERO )

constexpr SamplerState
GetSamplerStatePoint()
{
    const sce::Agc::Core::Sampler s =
        sce::Agc::Core::Sampler()
        .init()
        .setWrapMode(sce::Agc::Core::Sampler::WrapMode::kClampLastTexel)
        .setXyFilterMode(sce::Agc::Core::Sampler::FilterMode::kPoint, sce::Agc::Core::Sampler::FilterMode::kPoint)
        .setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);

    return SamplerState(s);
}

constexpr SamplerState
GetSamplerStateLinear()
{
    const sce::Agc::Core::Sampler s =
        sce::Agc::Core::Sampler()
        .init()
        .setWrapMode(sce::Agc::Core::Sampler::WrapMode::kClampLastTexel)
        .setXyFilterMode(sce::Agc::Core::Sampler::FilterMode::kBilinear, sce::Agc::Core::Sampler::FilterMode::kBilinear)
        .setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);

    return SamplerState(s);
}

static SamplerState s_PointClamp  = GetSamplerStatePoint();
static SamplerState s_LinearClamp = GetSamplerStateLinear();

#elif defined( D_PLATFORM_ORBIS )

SamplerState
GetSamplerStatePoint()
{
    sce::Gnm::Sampler s;
    s.init();
    s.setWrapMode(sce::Gnm::kWrapModeClampLastTexel, sce::Gnm::kWrapModeClampLastTexel, sce::Gnm::kWrapModeClampLastTexel);
    s.setXyFilterMode(sce::Gnm::kFilterModePoint, sce::Gnm::kFilterModePoint);
    s.setMipFilterMode(sce::Gnm::kMipFilterModePoint);
    return SamplerState(s);
}

SamplerState
GetSamplerStateLinear()
{
    sce::Gnm::Sampler s;
    s.init();
    s.setWrapMode(sce::Gnm::kWrapModeClampLastTexel, sce::Gnm::kWrapModeClampLastTexel, sce::Gnm::kWrapModeClampLastTexel);
    s.setXyFilterMode(sce::Gnm::kFilterModeBilinear, sce::Gnm::kFilterModeBilinear);
    s.setMipFilterMode(sce::Gnm::kMipFilterModePoint);
    return SamplerState(s);
}

static SamplerState s_PointClamp  = GetSamplerStatePoint();
static SamplerState s_LinearClamp = GetSamplerStateLinear();

#else

SamplerState s_PointClamp : register(s0, space1);
SamplerState s_LinearClamp : register(s1, space1);

#endif

typedef FFX_MIN16_F4 PREPARED_INPUT_COLOR_T;
typedef FFX_MIN16_F3 PREPARED_INPUT_COLOR_F3;
typedef FFX_MIN16_F  PREPARED_INPUT_COLOR_F1;

typedef FfxFloat32x3 UPSAMPLED_COLOR_T;

#define RW_UPSAMPLED_WEIGHT_T FfxFloat32

typedef FFX_MIN16_F3    LOCK_STATUS_T;
typedef FFX_MIN16_F     LOCK_STATUS_F1;

// SRVs
#if defined(FFX_INTERNAL)
    Texture2D<FfxFloat32x4>                       r_input_color_jittered              : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INPUT_COLOR);
    Texture2D<FfxFloat32x4>                       r_motion_vectors                    : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INPUT_MOTION_VECTORS);
    Texture2D<FfxFloat32>                         r_depth                             : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH);
    Texture2D<FfxFloat32x2>                       r_exposure                          : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INPUT_EXPOSURE);
    Texture2D<FFX_MIN16_F>                        r_reactive_mask                     : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INPUT_REACTIVE_MASK);
    Texture2D<FFX_MIN16_F>                        r_transparency_and_composition_mask : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INPUT_TRANSPARENCY_AND_COMPOSITION_MASK);
    Texture2D<FfxUInt32>                          r_ReconstructedPrevNearestDepth     : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_RECONSTRUCTED_PREVIOUS_NEAREST_DEPTH);
    Texture2D<FfxFloat32x2>                       r_dilated_motion_vectors            : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_DILATED_MOTION_VECTORS);
    Texture2D<FfxFloat32>                         r_dilatedDepth                      : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_DILATED_DEPTH);
    Texture2D<FFX_MIN16_F4>                       r_internal_upscaled_color           : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_INTERNAL_UPSCALED_COLOR);
    Texture2D<LOCK_STATUS_T>                      r_lock_status                       : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_LOCK_STATUS);
    Texture2D<FfxFloat32>                         r_depth_clip                        : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_DEPTH_CLIP);
    Texture2D<PREPARED_INPUT_COLOR_T>             r_prepared_input_color              : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_PREPARED_INPUT_COLOR);
    Texture2D<PREPARED_INPUT_COLOR_F1>            r_prepared_input_color_luma         : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_PREPARED_INPUT_COLOR_LUMA);
    Texture2D<PREPARED_INPUT_COLOR_F1>            r_prepared_input_log_luminance      : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_PREPARED_INPUT_LOG_LUMINANCE);
    Texture2D<PREPARED_INPUT_COLOR_T>             r_pre_upsampled_color               : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_PRE_UPSAMPLED_COLOR);
    Texture2D<unorm FfxFloat32x4>                 r_luma_history                      : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_LUMA_HISTORY);
    Texture2D<FfxFloat32x4>                       r_rcas_input                        : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_RCAS_INPUT);
    Texture2D<FFX_MIN16_F>                        r_lanczos_lut                       : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_LANCZOS_LUT);
    Texture2D<FFX_MIN16_F>                        r_imgMips                           : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_AUTO_EXPOSURE);
    Texture2D<FfxFloat32>                         r_upsample_maximum_bias_lut         : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTITIER_UPSAMPLE_MAXIMUM_BIAS_LUT);
    Texture2D<FfxFloat32x2>                       r_dilated_reactive_masks            : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_DILATED_REACTIVE_MASKS);

    // declarations not current form, no accessor functions
    Texture2D<FfxFloat32x4>                       r_transparency_mask                 : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_TRANSPARENCY_MASK);
    Texture2D<FfxFloat32x4>                       r_bias_current_color_mask           : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_BIAS_CURRENT_COLOR_MASK);
    Texture2D<FfxFloat32x4>                       r_gbuffer_albedo                    : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_ALBEDO);
    Texture2D<FfxFloat32x4>                       r_gbuffer_roughness                 : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_ROUGHNESS);
    Texture2D<FfxFloat32x4>                       r_gbuffer_metallic                  : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_METALLIC);
    Texture2D<FfxFloat32x4>                       r_gbuffer_specular                  : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_SPECULAR);
    Texture2D<FfxFloat32x4>                       r_gbuffer_subsurface                : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_SUBSURFACE);
    Texture2D<FfxFloat32x4>                       r_gbuffer_normals                   : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_NORMALS);
    Texture2D<FfxFloat32x4>                       r_gbuffer_shading_mode_id           : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_SHADING_MODE_ID);
    Texture2D<FfxFloat32x4>                       r_gbuffer_material_id               : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_GBUFFER_MATERIAL_ID);
    Texture2D<FfxFloat32x4>                       r_motion_vectors_3d                 : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_VELOCITY_3D);
    Texture2D<FfxFloat32x4>                       r_is_particle_mask                  : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_IS_PARTICLE_MASK);
    Texture2D<FfxFloat32x4>                       r_animated_texture_mask             : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_ANIMATED_TEXTURE_MASK);
    Texture2D<FfxFloat32>                         r_depth_high_res                    : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_DEPTH_HIGH_RES);
    Texture2D<FfxFloat32x4>                       r_position_view_space               : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_POSITION_VIEW_SPACE);
    Texture2D<FfxFloat32x4>                       r_ray_tracing_hit_distance          : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_RAY_TRACING_HIT_DISTANCE);
    Texture2D<FfxFloat32x4>                       r_motion_vectors_reflection         : FFX_FSR2_DECLARE_SRV(FFX_FSR2_RESOURCE_IDENTIFIER_VELOCITY_REFLECTION);

    // UAV declarations
    RWTexture2D<FfxUInt32>                        rw_ReconstructedPrevNearestDepth    : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_RECONSTRUCTED_PREVIOUS_NEAREST_DEPTH);
    RWTexture2D<FfxFloat32x2>                     rw_dilated_motion_vectors           : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_DILATED_MOTION_VECTORS);
    RWTexture2D<FfxFloat32>                       rw_dilatedDepth                     : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_DILATED_DEPTH);
    RWTexture2D<FfxFloat32x4>                     rw_internal_upscaled_color          : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_INTERNAL_UPSCALED_COLOR);        
    RWTexture2D<LOCK_STATUS_T>                    rw_lock_status                      : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_LOCK_STATUS);
    RWTexture2D<FfxFloat32>                       rw_depth_clip                       : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_DEPTH_CLIP);
    RWTexture2D<PREPARED_INPUT_COLOR_T>           rw_prepared_input_color             : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_PREPARED_INPUT_COLOR);
    RWTexture2D<unorm FfxFloat32x4>               rw_luma_history                     : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_LUMA_HISTORY);
    RWTexture2D<FfxFloat32x4>                     rw_upscaled_output                  : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_UPSCALED_OUTPUT);
    //globallycoherent RWTexture2D<FfxFloat32>     rw_imgMipmap[13]                    : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_AUTO_EXPOSURE);
    globallycoherent RWTexture2D<FfxFloat32>      rw_img_mip_shading_change           : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_AUTO_EXPOSURE_MIPMAP_SHADING_CHANGE);
    globallycoherent RWTexture2D<FfxFloat32>      rw_img_mip_5                        : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_AUTO_EXPOSURE_MIPMAP_5);
    RWTexture2D<unorm FfxFloat32x2>               rw_dilated_reactive_masks           : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_DILATED_REACTIVE_MASKS);
    RWTexture2D<FfxFloat32x2>                     rw_exposure                         : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_EXPOSURE);
    globallycoherent RWTexture2D<FfxUInt32>       rw_spd_global_atomic                : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_SPD_ATOMIC_COUNT);
    RWTexture2D<FfxFloat32x4>                     rw_debug_out                        : FFX_FSR2_DECLARE_UAV(FFX_FSR2_RESOURCE_IDENTIFIER_DEBUG_OUTPUT);
    
#else // #if defined(FFX_INTERNAL)
    #if defined FSR2_BIND_SRV_INPUT_COLOR
        Texture2D<FfxFloat32x4>                   r_input_color_jittered                    : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_INPUT_COLOR);
    #endif
    #if defined FSR2_BIND_SRV_MOTION_VECTORS
        Texture2D<FfxFloat32x4>                   r_motion_vectors                          : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_MOTION_VECTORS);
    #endif
    #if defined FSR2_BIND_SRV_DEPTH
        Texture2D<FfxFloat32>                     r_depth                                   : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_DEPTH);
    #endif 
    #if defined FSR2_BIND_SRV_EXPOSURE
        Texture2D<FfxFloat32x2>                   r_exposure                                : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_EXPOSURE);
    #endif
    #if defined FSR2_BIND_SRV_REACTIVE_MASK
        Texture2D<FFX_MIN16_F>                    r_reactive_mask                           : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_REACTIVE_MASK);
    #endif 
    #if defined FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK
        Texture2D<FFX_MIN16_F>                    r_transparency_and_composition_mask       : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK);
    #endif
    #if defined FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH
        Texture2D<FfxUInt32>                      r_reconstructed_previous_nearest_depth    : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH);
    #endif 
    #if defined FSR2_BIND_SRV_DILATED_MOTION_VECTORS
       Texture2D<FfxFloat32x2>                    r_dilated_motion_vectors                  : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_DILATED_MOTION_VECTORS);
    #endif
    #if defined FSR2_BIND_SRV_DILATED_DEPTH
        Texture2D<FfxFloat32>                     r_dilatedDepth                            : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_DILATED_DEPTH);
    #endif
    #if defined FSR2_BIND_SRV_INTERNAL_UPSCALED
        Texture2D<FFX_MIN16_F4>                   r_internal_upscaled_color                 : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_INTERNAL_UPSCALED);
    #endif
    #if defined FSR2_BIND_SRV_LOCK_STATUS
        #if FFX_COMPILE_FOR_SPIRV
        Texture2D<FfxFloat32x3>                   r_lock_status                             : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_LOCK_STATUS);
        #else
        Texture2D<LOCK_STATUS_T>                  r_lock_status                             : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_LOCK_STATUS);
        #endif
    #endif
    #if defined FSR2_BIND_SRV_DEPTH_CLIP
        Texture2D<FfxFloat32>                     r_depth_clip                              : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_DEPTH_CLIP);
    #endif
    #if defined FSR2_BIND_SRV_PREPARED_INPUT_COLOR
        #if FFX_COMPILE_FOR_SPIRV
        Texture2D<FfxFloat32x4>                   r_prepared_input_color                    : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_PREPARED_INPUT_COLOR);
        #else
        Texture2D<PREPARED_INPUT_COLOR_T>         r_prepared_input_color                    : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_PREPARED_INPUT_COLOR);
        #endif
    #endif
    #if defined FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA
        #if FFX_COMPILE_FOR_SPIRV
        Texture2D<FfxFloat32>                     r_prepared_input_color_luma               : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA);
        #else
        Texture2D<PREPARED_INPUT_COLOR_F1>        r_prepared_input_color_luma               : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA);
        #endif
    #endif
    #if defined FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE
        #if FFX_COMPILE_FOR_SPIRV
        Texture2D<FfxFloat32>                     r_prepared_input_log_luminance            : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE);
        #else
        Texture2D<PREPARED_INPUT_COLOR_F1>        r_prepared_input_log_luminance            : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE);
        #endif
    #endif
    #if defined FSR2_BIND_SRV_LUMA_HISTORY
        Texture2D<unorm FfxFloat32x4>             r_luma_history                            : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_LUMA_HISTORY);
    #endif
    #if defined FSR2_BIND_SRV_RCAS_INPUT
        Texture2D<FfxFloat32x4>                   r_rcas_input                              : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_RCAS_INPUT);
    #endif
    #if defined FSR2_BIND_SRV_LANCZOS_LUT
        Texture2D<FFX_MIN16_F>                    r_lanczos_lut                             : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_LANCZOS_LUT);
    #endif
    #if defined FSR2_BIND_SRV_EXPOSURE_MIPS
        Texture2D<FFX_MIN16_F>                    r_imgMips                                 : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_EXPOSURE_MIPS);
    #endif
    #if defined FSR2_BIND_SRV_DILATED_REACTIVE_MASKS
        Texture2D<FfxFloat32x2>                   r_dilated_reactive_masks                  : FFX_FSR2_DECLARE_SRV(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS);
    #endif

    // UAV declarations
    #if defined FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH
        RWTexture2D<FfxUInt32>                    rw_reconstructed_previous_nearest_depth   : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH);
    #endif
    #if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS
        RWTexture2D<FfxFloat32x2>                 rw_dilated_motion_vectors                 : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_DILATED_MOTION_VECTORS);
    #endif
    #if defined FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM
        RWTexture2D<unorm FfxFloat32x2>           rw_dilated_motion_vectors                 : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM);
    #endif
    #if defined FSR2_BIND_UAV_DILATED_DEPTH
        RWTexture2D<FfxFloat32>                   rw_dilatedDepth                           : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_DILATED_DEPTH);
    #endif
    #if defined FSR2_BIND_UAV_INTERNAL_UPSCALED
        RWTexture2D<FfxFloat32x4>                 rw_internal_upscaled_color                : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_INTERNAL_UPSCALED);
    #endif
    #if defined FSR2_BIND_UAV_LOCK_STATUS
        #if FFX_COMPILE_FOR_SPIRV
        RWTexture2D<FfxFloat32x3>                 rw_lock_status                            : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_LOCK_STATUS);
        #else
        RWTexture2D<LOCK_STATUS_T>                rw_lock_status                            : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_LOCK_STATUS);
        #endif
    #endif
    #if defined FSR2_BIND_UAV_LOCK_STATUS_UNORM
        #if FFX_COMPILE_FOR_SPIRV
        RWTexture2D<unorm FfxFloat32x3>           rw_lock_status                            : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_LOCK_STATUS_UNORM);
        #else
        RWTexture2D<unorm LOCK_STATUS_T>          rw_lock_status                            : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_LOCK_STATUS_UNORM);
        #endif
    #endif
    #if defined FSR2_BIND_UAV_DEPTH_CLIP
        RWTexture2D<FfxFloat32>                   rw_depth_clip                             : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_DEPTH_CLIP);
    #endif
    #if defined FSR2_BIND_UAV_PREPARED_INPUT_COLOR
        #if FFX_COMPILE_FOR_SPIRV
        RWTexture2D<FfxFloat32x4>                 rw_prepared_input_color                   : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_PREPARED_INPUT_COLOR);
        #else
        RWTexture2D<PREPARED_INPUT_COLOR_T>       rw_prepared_input_color                   : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_PREPARED_INPUT_COLOR);
        #endif
    #endif
    #if defined FSR2_BIND_UAV_LUMA_HISTORY
        RWTexture2D<unorm FfxFloat32x4>           rw_luma_history                           : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_LUMA_HISTORY);
    #endif
    #if defined FSR2_BIND_UAV_UPSCALED_OUTPUT
        RWTexture2D<FfxFloat32x4>                 rw_upscaled_output                        : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_UPSCALED_OUTPUT);
    #endif
    #if defined FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE
        globallycoherent RWTexture2D<FfxFloat32>  rw_img_mip_shading_change                 : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE);
    #endif
    #if defined FSR2_BIND_UAV_EXPOSURE_MIP_5
        globallycoherent RWTexture2D<FfxFloat32>  rw_img_mip_5                              : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_EXPOSURE_MIP_5);
    #endif
    #if defined FSR2_BIND_UAV_DILATED_REACTIVE_MASKS
        RWTexture2D<unorm FfxFloat32x2>           rw_dilated_reactive_masks                 : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_DILATED_REACTIVE_MASKS);
    #endif
    #if defined FSR2_BIND_UAV_EXPOSURE
        RWTexture2D<FfxFloat32x2>                 rw_exposure                               : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_EXPOSURE);
    #endif
    #if defined FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC
        globallycoherent RWTexture2D<FfxUInt32>   rw_spd_global_atomic                      : FFX_FSR2_DECLARE_UAV(FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC);
    #endif
#endif // #if defined(FFX_INTERNAL)

FfxFloat32 LoadMipLuma(FFX_MIN16_I2 iPxPos, FfxUInt32 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS) || defined(FFX_INTERNAL)
#if defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_PROSPERO )
    return r_imgMips.MipMaps[mipLevel][iPxPos];
#else
    return r_imgMips.mips[mipLevel][iPxPos];
#endif
#else
    return 0.f;
#endif
}
#if FFX_HALF
FfxFloat16 LoadMipLuma(FfxInt16x2 iPxPos, FfxUInt16 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS) || defined(FFX_INTERNAL)
#if defined( D_PLATFORM_ORBIS ) || defined( D_PLATFORM_PROSPERO )
    return r_imgMips.MipMaps[mipLevel][iPxPos];
#else
    return r_imgMips.mips[mipLevel][iPxPos];
#endif
#else
    return 0.f;
#endif
}
#endif
FfxFloat32 SampleMipLuma(FfxFloat32x2 fUV, FfxUInt32 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS) || defined(FFX_INTERNAL)
    fUV *= depthclip_uv_scale;
    return r_imgMips.SampleLevel(s_LinearClamp, fUV, mipLevel);
#else
    return 0.f;
#endif

}
#if FFX_HALF
FfxFloat16 SampleMipLuma(FfxFloat16x2 fUV, FfxUInt32 mipLevel)
{
#if defined(FSR2_BIND_SRV_EXPOSURE_MIPS) || defined(FFX_INTERNAL)
    fUV *= FfxFloat16x2(depthclip_uv_scale);
    return r_imgMips.SampleLevel(s_LinearClamp, fUV, mipLevel);
#else
    return 0.f;
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
    return -fDeviceToViewDepth[2] / (fDeviceDepth * fDeviceToViewDepth[1] - fDeviceToViewDepth[0]);
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
    return -fLogLuminance * 3.0f;
}

FfxFloat32 LoadInputDepth(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_DEPTH) || defined(FFX_INTERNAL)
    return r_depth[iPxPos];
#else
    return 0.f;
#endif
}

FFX_MIN16_F LoadReactiveMask(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_REACTIVE_MASK) || defined(FFX_INTERNAL)
    return r_reactive_mask[iPxPos];
#else 
    return 0.f;
#endif
}

FfxFloat32x4 GatherReactiveMask(int2 iPxPos)
{
#if defined(FSR2_BIND_SRV_REACTIVE_MASK) || defined(FFX_INTERNAL)
    return r_reactive_mask.GatherRed(s_LinearClamp, FfxFloat32x2(iPxPos) * reactive_mask_dim_rcp);
#else
    return 0.f;
#endif
}

FFX_MIN16_F LoadTransparencyAndCompositionMask(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK) || defined(FFX_INTERNAL)
    return r_transparency_and_composition_mask[iPxPos];
#else
    return 0.f;
#endif
}

FfxFloat32 PreExposure()
{
#if FFX_FSR2_CUSTOM_OPTION_NO_PRE_EXPOSURE == 0
    return fPreExposure;
#else
    return FfxFloat32(1.0f);
#endif
}

FfxFloat32x3 LoadInputColor(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_input_color_jittered[iPxPos].rgb / PreExposure();
#else
    return 0;
#endif
}

FfxFloat32x3 LoadInputColorWithoutPreExposure(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_input_color_jittered[iPxPos].rgb;
#else
    return 0;
#endif
}

#if FFX_HALF
FfxFloat16x3 LoadPreparedInputColor(FfxInt16x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_prepared_input_color[iPxPos].rgb;
#else
    return 0.f;
#endif
}

FfxFloat16x3 SamplePreparedInputColor(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_prepared_input_color.SampleLevel(s_LinearClamp, fUV, 0).rgb;
#else
	return 0.f;
#endif
}

FfxFloat16x3 SamplePreparedInputColorLuma(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
    return r_prepared_input_color_luma.SampleLevel(s_LinearClamp, fUV, 0).r;
    #elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    return r_prepared_input_color.SampleLevel(s_LinearClamp, fUV, 0).a;
    #else
    return 0.f;
    #endif
}

#else

FFX_MIN16_F3 LoadPreparedInputColor(int2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_prepared_input_color[iPxPos].rgb;
#else
    return 0.f;
#endif
}

FFX_MIN16_F3 SamplePreparedInputColor(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_prepared_input_color.SampleLevel(s_LinearClamp, fUV, 0).rgb;
#else
	return 0.f;
#endif
}

FFX_MIN16_F SamplePreparedInputColorLuma(FfxFloat32x2 fUV)
{
    #if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
    return r_prepared_input_color_luma.SampleLevel(s_LinearClamp, fUV, 0).r;
    #elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    return r_prepared_input_color.SampleLevel(s_LinearClamp, fUV, 0).a;
    #else
    return 0.f;
    #endif
}

#endif

FFX_MIN16_F LoadPreparedInputColorLuma(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR) || defined(FFX_INTERNAL)
    return r_prepared_input_color[iPxPos].a;
#else
    return 0.f;
#endif
}

FFX_MIN16_F LoadPreparedInputLogLuminance(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE) || defined(FFX_INTERNAL)
	return r_prepared_input_log_luminance[iPxPos].r;
#else
	return 0.f;
#endif
}

FFX_MIN16_F4 GatherPreparedInputColorLuma(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
	return r_prepared_input_color_luma.GatherRed(s_LinearClamp, fUV);
#elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
	return r_prepared_input_color.GatherAlpha(s_LinearClamp, fUV);
#else
    return 0.f;
#endif
}

FFX_MIN16_F4 GatherOffsetPreparedInputColorLuma(FfxFloat32x2 fUV, FFX_MIN16_I2 iOffset)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR_LUMA)
    return r_prepared_input_color_luma.GatherRed(s_LinearClamp, fUV, iOffset);
#elif defined(FSR2_BIND_SRV_PREPARED_INPUT_COLOR)
    return r_prepared_input_color.GatherAlpha(s_LinearClamp, fUV, iOffset);
#else
    return FFX_MIN16_F4(0.f,0.f,0.f,0.f);
#endif
}

FFX_MIN16_F4 GatherPreparedInputLogLuminance(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE) || defined(FFX_INTERNAL)
	return r_prepared_input_log_luminance.GatherRed(s_LinearClamp, fUV);
#else
    return 0.f;
#endif
}

FFX_MIN16_F SamplePreparedInputLogLuminance(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_PREPARED_INPUT_LOG_LUMINANCE) || defined(FFX_INTERNAL)
	return DecodeLogLuminance(r_prepared_input_log_luminance.SampleLevel(s_LinearClamp, fUV, 0).r);
#else
    return 0.f;
#endif
}

FfxFloat32x2 LoadInputMotionVector(FFX_MIN16_I2 iPxDilatedMotionVectorPos)
{
#if defined(FSR2_BIND_SRV_MOTION_VECTORS) || defined(FFX_INTERNAL)
    FfxFloat32x2 fSrcMotionVector = r_motion_vectors[iPxDilatedMotionVectorPos].xy;
#else
    FfxFloat32x2 fSrcMotionVector = 0.f;
#endif

#if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
    FfxFloat32x2 fUvMotionVector = DecodeMotionVector(fSrcMotionVector);
#else
    FfxFloat32x2 fUvMotionVector = fSrcMotionVector * MotionVectorScale;
#endif

#if FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
    fUvMotionVector -= fMotionVectorJitterCancellation;
#endif

    return fUvMotionVector;
}

FFX_MIN16_F4 LoadHistory(int2 iPxHistory)
{
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED) || defined(FFX_INTERNAL)
    return r_internal_upscaled_color[iPxHistory];
#else
    return 0.f;
#endif
}

FFX_MIN16_F4 SampleHistory(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_INTERNAL_UPSCALED) || defined(FFX_INTERNAL)
    return r_internal_upscaled_color.SampleLevel(s_LinearClamp, fUV, 0);
#else
    return 0.f;
#endif
}

FfxFloat32x4 LoadRwInternalUpscaledColorAndWeight(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_UAV_INTERNAL_UPSCALED) || defined(FFX_INTERNAL)
    return rw_internal_upscaled_color[iPxPos];
#else
    return 0.f;
#endif
}

void StoreLumaHistory(FFX_MIN16_I2 iPxPos, FfxFloat32x4 fLumaHistory)
{
#if defined(FSR2_BIND_UAV_LUMA_HISTORY) || defined(FFX_INTERNAL)
    rw_luma_history[iPxPos] = fLumaHistory;
#endif
}

FfxFloat32x4 LoadRwLumaHistory(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_UAV_LUMA_HISTORY) || defined(FFX_INTERNAL)
    return rw_luma_history[iPxPos];
#else
    return 1.f;
#endif
}

FfxFloat32 LoadLumaStabilityFactor(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY) || defined(FFX_INTERNAL)
    return r_luma_history[iPxPos].w;
#else
    return 0.f;
#endif
}

FfxFloat32x4 SampleLumaHistory(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY)
    fUV *= depthclip_uv_scale;
    return r_luma_history.SampleLevel(s_LinearClamp, fUV, 0);
#else
    return 1.f;
#endif
}


FfxFloat32 SampleLumaStabilityFactor(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_LUMA_HISTORY) || defined(FFX_INTERNAL)
    fUV *= depthclip_uv_scale;
    return r_luma_history.SampleLevel(s_LinearClamp, fUV, 0).w;
#else
    return 0.f;
#endif
}

void StoreReprojectedHistory(FFX_MIN16_I2 iPxHistory, FFX_MIN16_F4 fHistory)
{
#if defined(FSR2_BIND_UAV_INTERNAL_UPSCALED) || defined(FFX_INTERNAL)
    rw_internal_upscaled_color[iPxHistory] = fHistory;
#endif
}

void StoreInternalColorAndWeight(FFX_MIN16_I2 iPxPos, FfxFloat32x4 fColorAndWeight)
{
#if defined(FSR2_BIND_UAV_INTERNAL_UPSCALED) || defined(FFX_INTERNAL)
    rw_internal_upscaled_color[iPxPos] = fColorAndWeight;
#endif
}

void StoreUpscaledOutput(FFX_MIN16_I2 iPxPos, FfxFloat32x3 fColor)
{
#if defined(FSR2_BIND_UAV_UPSCALED_OUTPUT) || defined(FFX_INTERNAL)
    FfxFloat32x3 fOutputColor = fColor * PreExposure();
    rw_upscaled_output[iPxPos] = FfxFloat32x4(fOutputColor, 1.f);
#endif
}

//LOCK_LIFETIME_REMAINING == 0
//Should make LockInitialLifetime() return a const 1.0f later
LOCK_STATUS_T LoadLockStatus(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_LOCK_STATUS) || defined(FFX_INTERNAL)
    LOCK_STATUS_T fLockStatus = r_lock_status[iPxPos];

    fLockStatus[0] = DecodeLockLifetime(fLockStatus[0]);
    return fLockStatus;
#else
    return 0.f;
#endif

    
}

LOCK_STATUS_T LoadRwLockStatus(int2 iPxPos)
{
#if defined(FSR2_BIND_UAV_LOCK_STATUS) || defined(FSR2_BIND_UAV_LOCK_STATUS_UNORM) || defined(FFX_INTERNAL)
    LOCK_STATUS_T fLockStatus = rw_lock_status[iPxPos];

    fLockStatus[0] = DecodeLockLifetime(fLockStatus[0]);

    return fLockStatus;
#else
    return 0.f;
#endif
}

void StoreLockStatus(FFX_MIN16_I2 iPxPos, LOCK_STATUS_T fLockStatus)
{
#if defined(FSR2_BIND_UAV_LOCK_STATUS) || defined(FSR2_BIND_UAV_LOCK_STATUS_UNORM) || defined(FFX_INTERNAL)
    fLockStatus[0] = EncodeLockLifetime(fLockStatus[0]);

    rw_lock_status[iPxPos] = fLockStatus;
#endif
}

void StoreLockStatus(FFX_MIN16_I2 iPxPos, LOCK_STATUS_T fLockStatus, FfxFloat32 fEdgeStatus)
{
    // TODO(sal)
}

void StorePreparedInputColor(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN PREPARED_INPUT_COLOR_T fTonemapped)
{
#if defined(FSR2_BIND_UAV_PREPARED_INPUT_COLOR) || defined(FFX_INTERNAL)
    rw_prepared_input_color[iPxPos] = fTonemapped;
#endif
}

FfxBoolean IsResponsivePixel(FFX_MIN16_I2 iPxPos)
{
    return FFX_FALSE; //not supported in prototype
}

FfxFloat32 LoadDepthClip(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_DEPTH_CLIP) || defined(FFX_INTERNAL)
    return r_depth_clip[iPxPos];
#else
    return 0.f;
#endif
}

FfxFloat32 SampleDepthClip(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_DEPTH_CLIP) || defined(FFX_INTERNAL)
    fUV *= depthclip_uv_scale;
    return r_depth_clip.SampleLevel(s_LinearClamp, fUV, 0);
#else
    return 0.f;
#endif
}

LOCK_STATUS_T SampleLockStatus(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_LOCK_STATUS) || defined(FFX_INTERNAL)
    fUV *= postprocessed_lockstatus_uv_scale;
    LOCK_STATUS_T fLockStatus = r_lock_status.SampleLevel(s_LinearClamp, fUV, 0);
    fLockStatus[0] = DecodeLockLifetime(fLockStatus[0]);
    return fLockStatus;
#else
    return 0.f;
#endif
}

void StoreDepthClip(FFX_MIN16_I2 iPxPos, FfxFloat32 fClip)
{
#if defined(FSR2_BIND_UAV_DEPTH_CLIP) || defined(FFX_INTERNAL)
    rw_depth_clip[iPxPos] = fClip;
#endif
}

FfxFloat32 TanHalfFoV()
{
    return fTanHalfFOV;
}

FfxFloat32 LoadSceneDepth(FFX_MIN16_I2 iPxInput)
{
#if defined(FSR2_BIND_SRV_DEPTH) || defined(FFX_INTERNAL)
    return r_depth[iPxInput];
#else
    return 0.f;
#endif
}

FfxFloat32 LoadReconstructedPrevDepth(FFX_MIN16_I2 iPxPos)
{
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH) || defined(FFX_INTERNAL)
    return asfloat(r_reconstructed_previous_nearest_depth[iPxPos]);
#else 
    return 0;
#endif
}

FfxFloat32x4 GatherReconstructedPrevDepth(FfxFloat32x2 fUv)
{
#if defined(FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH)
    return r_reconstructed_previous_nearest_depth.GatherRed(s_LinearClamp, fUv);
#else 
	return FfxFloat32x4(0.f,0.f,0.f,0.f);
#endif
}

void StoreReconstructedDepth(FFX_MIN16_I2 iPxSample, FfxFloat32 fDepth)
{
    FfxUInt32 uDepth = asuint(fDepth);
#if defined(FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH) || defined(FFX_INTERNAL)
    #if FFX_FSR2_OPTION_INVERTED_DEPTH
        InterlockedMax(rw_reconstructed_previous_nearest_depth[iPxSample], uDepth);
    #else
        InterlockedMin(rw_reconstructed_previous_nearest_depth[iPxSample], uDepth); // min for standard, max for inverted depth
    #endif
#endif
}

void SetReconstructedDepth(FFX_MIN16_I2 iPxSample, const FfxUInt32 uValue)
{
#if defined(FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH) || defined(FFX_INTERNAL)
    rw_reconstructed_previous_nearest_depth[iPxSample] = uValue;
#endif
}

void StoreDilatedDepth(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN FfxFloat32 fDepth)
{
#if defined(FSR2_BIND_UAV_DILATED_DEPTH) || defined(FFX_INTERNAL)
    //FfxUInt32 uDepth = f32tof16(fDepth);
    rw_dilatedDepth[iPxPos] = fDepth;
#endif
}

void StoreDilatedMotionVector(FFX_PARAMETER_IN FFX_MIN16_I2 iPxPos, FFX_PARAMETER_IN FfxFloat32x2 fMotionVector)
{
#if defined(FSR2_BIND_UAV_DILATED_MOTION_VECTORS) || defined(FSR2_BIND_UAV_DILATED_MOTION_VECTORS_UNORM) || defined(FFX_INTERNAL)
#if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
    rw_dilated_motion_vectors[iPxPos] = EncodeMotionVector(fMotionVector);
#else
    rw_dilated_motion_vectors[iPxPos] = fMotionVector;
#endif
#endif
}

FfxFloat32x2 LoadDilatedMotionVector(FFX_MIN16_I2 iPxInput)
{
#if defined(FSR2_BIND_SRV_DILATED_MOTION_VECTORS) || defined(FFX_INTERNAL)
#if FFX_FSR2_CUSTOM_OPTION_UV_SPACE_MOTION_VECTORS
    return DecodeMotionVector(r_dilated_motion_vectors[iPxInput].xy);
#else
    return r_dilated_motion_vectors[iPxInput].xy;
#endif
#else 
    return 0.f;
#endif
}


FfxFloat32 LoadDilatedDepth(FFX_MIN16_I2 iPxInput)
{
#if defined(FSR2_BIND_SRV_DILATED_DEPTH) || defined(FFX_INTERNAL)
    return r_dilatedDepth[iPxInput];
#else
    return 0.f;
#endif
}

FfxFloat32 Exposure()
{
    // return 1.0f;
    #if defined(FSR2_BIND_SRV_EXPOSURE) || defined(FFX_INTERNAL)
        FfxFloat32 exposure = r_exposure[FFX_MIN16_I2(0, 0)].x;
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
#if defined(FSR2_BIND_SRV_LANCZOS_LUT) || defined(FFX_INTERNAL)
    return r_lanczos_lut.SampleLevel(s_LinearClamp, FfxFloat32x2(x / 2, 0.5f), 0);
#else
    return 0.f;
#endif
}

#if FFX_HALF
FfxFloat16 SampleLanczos2Weight(FfxFloat16 x)
{
#if defined(FSR2_BIND_SRV_LANCZOS_LUT) || defined(FFX_INTERNAL)
    return r_lanczos_lut.SampleLevel(s_LinearClamp, FfxFloat16x2(x / 2, 0.5f), 0);
#else
    return 0.f;
#endif
}
#endif


FfxFloat32 SampleLanczos2SumWeight(FfxFloat32x3 x)
{
#if defined(FSR2_BIND_SRV_LANCZOS_SUM_LUT)
#if !defined(FFX_FSR2_CUSTOM_IMPLEMENTATION)
    return r_lanczos_sum_lut.SampleLevel(s_LinearClamp, x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x;
#else
    return r_lanczos_sum_lut.SampleLevel(s_LinearClamp, x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x;
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
    return FfxFloat16(r_lanczos_sum_lut.SampleLevel(s_LinearClamp, x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x);
#else
    return FfxFloat16(r_lanczos_sum_lut.SampleLevel(s_LinearClamp, x * FfxFloat32x3(1.0f, 1.0f, 0.5f), 0.0f).x);
#endif
#else
    return FfxFloat16(0.f);
#endif
}
#endif


FfxFloat32x2 SampleDilatedReactiveMasks(FfxFloat32x2 fUV)
{
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS) || defined(FFX_INTERNAL)
    fUV *= depthclip_uv_scale;
	return r_dilated_reactive_masks.SampleLevel(s_LinearClamp, fUV, 0);
#else
	return 0.f;
#endif
}

FfxFloat32x2 LoadDilatedReactiveMasks(FFX_PARAMETER_IN FfxUInt32x2 iPxPos)
{
#if defined(FSR2_BIND_SRV_DILATED_REACTIVE_MASKS) || defined(FFX_INTERNAL)
    return r_dilated_reactive_masks[iPxPos];
#else
    return 0.f;
#endif
}

void StoreDilatedReactiveMasks(FFX_PARAMETER_IN FfxUInt32x2 iPxPos, FFX_PARAMETER_IN FfxFloat32x2 fDilatedReactiveMasks)
{
#if defined(FSR2_BIND_UAV_DILATED_REACTIVE_MASKS) || defined(FFX_INTERNAL)
    rw_dilated_reactive_masks[iPxPos] = fDilatedReactiveMasks;
#endif
}

#endif // #if defined(FFX_GPU)
