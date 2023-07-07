
#if !defined( _F50_DISABLE_POSTPROCESS ) && !defined( D_NO_POSTPROCESS ) || defined( D_FORWARD_RENDERER )
#include "OutputPostProcess.shader.h"
#endif
#include "Common/ACES.shader.h"

#if !defined( D_PLATFORM_METAL )
#undef D_BLOOM
#undef D_DOF
#endif

#if defined( D_PLATFORM_METAL )
#if defined( D_FORWARD_RENDERER )
#if defined( D_DEFERRED_DECAL ) && defined( _F03_NORMALMAP ) && !defined( _F51_DECAL_DIFFUSE )
#define D_DEFERRED_NORMAL_DECAL
#endif
#endif

#if defined( D_DEFERRED_NORMAL_DECAL )
#undef D_BLOOM
#undef D_DOF
#endif

//TF_BEGIN
#include "Common/CommonUtils.shader.h"

//-----------------------------------------------------------------------------
//    Functions

#if defined(D_DOF)
float
GetDofPower(
	float lfLinearZ,
	vec4 lDofParamsVec4)
{
	float lfPower = 0.0;

	if (lDofParamsVec4.x != 0.0) // DOF amount
	{
		vec3 lDofBlurCol;
		if (lDofParamsVec4.x >= 0.0) // DOF amount
		{
			vec2 lDistMaskedVec2;

			float lfNearPlane = lDofParamsVec4.z;
			lDistMaskedVec2.x = smoothstep(lfNearPlane, lfNearPlane - min(15.0, lfNearPlane * 0.5), lfLinearZ); // in this case edge0 > edge1 - glsl spec says this gives undefined behaviour... is this line correct?
			lDistMaskedVec2.y = smoothstep(lDofParamsVec4.y, lDofParamsVec4.y + lDofParamsVec4.w, lfLinearZ);

			lfPower = max(lDistMaskedVec2.x, lDistMaskedVec2.y);
		}
		else
		{
			// new physically based DoF
			lDofParamsVec4.x = -lDofParamsVec4.x;

			// y-value is focus distance, z-value is f-stop, w-value is the film plane width
			// for simplicity figure the lens has a focal length of 1
			float lfCoC = 0.0;  // CoC diameter
			if (lDofParamsVec4.y > 100000000.0)
			{
				// infinite focus distance
				lfCoC = 1.0 / (lfLinearZ * lDofParamsVec4.z);
			}
			else
			{
				lfCoC = abs(lfLinearZ - lDofParamsVec4.y) / lfLinearZ;
				lfCoC /= (lDofParamsVec4.z * (lDofParamsVec4.y - 0.5));
			}

			// now get the CoC radius in pixels - just assume 1920-pixel width half-sized
			lfCoC = abs(lfCoC * 480.0 / lDofParamsVec4.w);

			// consider CoC < 1 pix to be full focus
			lfCoC -= 1.0;

			// and normalise to 1.0 == 8 pix radius
			lfCoC = saturate(lfCoC * 0.125);

			// apply strength val
			lfPower = lfCoC * lDofParamsVec4.x;
		}
	}

	return lfPower;
}
#endif // defined(D_DOF)

#endif // defined( D_PLATFORM_METAL )

//TF_END

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
#if !defined( D_ATTRIBUTES ) && !defined( _F07_UNLIT ) || defined( D_FORWARD_RENDERER )
	in  mat3                      lUpMatrix,
	SAMPLER2DSHADOWARG(lShadowMap),
	SAMPLER2DARG(lCloudShadowMap),
	SAMPLER2DARG(lDualPMapBack),
	SAMPLER2DARG(lDualPMapFront),
	//TF_BEGIN
#if defined( D_TILED_LIGHTS )
#if defined( D_PLATFORM_METAL )
	const device atomic_int * gLightCluster,
	in int						  liLightClusterBuffIndex,
#else
	in int						  liLightCountIn,
	in int					      laLightIndices[D_TILE_MAX_LIGHT_COUNT],
#endif
#endif
	in  float					  lfAO,
#endif
	//TF_END
	in  float                     lfPixelDepth,
	in  bool                      lbFrontFacing,
	in  float                     lfPixelSelfShadowing)
{

	// BLOOM requires "encoded" glow value
	const float lfGlowFactor = saturate(sqrt(lfGlow * (1.0 / 4.0)));

	// apply map resulting from encode/decode GBUFFER
	lfGlow = clamp(lfGlow, 0, 4);
	lfRoughness = saturate(lfRoughness);
	lfMetallic = saturate(lfMetallic);
	lfSubsurface = saturate(lfSubsurface);

	vec4 lFragmentColourVec4;
	vec3 lSunColourVec3 = float2vec3(0.0);
	vec3 lAccumulatedLightColourVec3 = float2vec3(0.0);

#if !defined( D_REFLECT_WATER_UP ) && !defined( D_REFLECT_WATER ) && !defined( D_REFLECT_DUALP )
	// colorspace conversion
	lColourVec4.xyz = MUL(lColourVec4.xyz, sRGB_TO_P3D65);
#endif

	//TF_BEGIN
	if (lfPixelSelfShadowing > -1.0)
	{
		// If using parallax and self shadowing we can't be using glow.
		liMaterialID = liMaterialID & ~D_GLOW;
		liMaterialID |= D_PARALLAX;
	}

#if defined( D_DEFERRED_NORMAL_DECAL )
	liMaterialID = liMaterialID & ~D_GLOW;
	liMaterialID = liMaterialID & D_DISABLE_POSTPROCESS;
#endif
	//TF_END

	lFragmentColourVec4 = lColourVec4;

	//-----------------------------------------------------------------------------
	///
	///     Lighting
	///
	//-----------------------------------------------------------------------------
#ifndef _F07_UNLIT
	{
		//TF_BEGIN
		lFragmentColourVec4.xyz = vec3(0.0, 0.0, 0.0);

		//
		// Wet effect for stuff when raining. This all needs a pass to tidy up/move 
		// params into data.
		//
#if defined ( D_FORWARD_RENDERER ) && defined ( D_FIXME ) // gRainParametersVec4 cannot exist on PC
		if (lPerFrameUniforms.gRainParametersVec4.x > 0.0)
		{
			// variety based on roughness
			//lfRoughness = 0.0;
			float lfOriginalRoughness = lfRoughness;
			float lfShininess = 1.0 - lfRoughness;
			//if (lfShininess > 0.22)
			{
				lfShininess = saturate(lfShininess * 3.0);
			}

			// wet sides but not top or bottom
			vec3    lUp = GetWorldUp(lPositionVec3, lMeshUniforms.gPlanetPositionVec4);
			float   lfDot = (dot(lUp, lNormalVec3));
			float   lfClamp = lfDot < 0.0 ? 1.0 : 0.7; // the clamp keeps a minimum shininess, which is less for downward facing normals
			lfDot = abs(lfDot);
			//float   lfUpAmount  =lfDot;
			float   lfUpAmount = clamp(lfDot * lfDot * lfDot, 0.0, lfClamp);
			lfShininess *= (1.0 - lfUpAmount);

			// convert back to roughness
			lfRoughness = 1.0 - lfShininess;

			// Blend between material's roughness and new wet roughness
			float lfMaxWetness = lPerFrameUniforms.gRainParametersVec4.x;// * lUniforms.mpCommonPerMesh.gWetnessParamsVec4.x;
			lfRoughness = mix(lfOriginalRoughness, lfRoughness, lfMaxWetness);
		}
#endif

#ifdef D_TILED_LIGHTS
#ifdef D_PLATFORM_METAL
		const device int * gLightClusterInt = (const device int*)gLightCluster;
		int liLightCount = 0;

#if defined( D_USE_TILE_Z_EXTENTS )
		float minZ = FLT_MAX;
		float maxZ = -FLT_MAX;
		const float distanceInTile = length(lPositionVec3.xyz - lPerFrameUniforms.gViewPositionVec3);
#endif

		if (gLightClusterInt)
		{
			liLightCount = min(gLightClusterInt[liLightClusterBuffIndex], D_TILE_MAX_LIGHT_COUNT);
#ifdef D_PLATFORM_METAL
			liLightCount = min(gLightClusterInt[liLightClusterBuffIndex], MTL_MAX_LIGHT_COUNT);
#endif
#if defined( D_USE_TILE_Z_EXTENTS )
			minZ = as_float(gLightClusterInt[liLightClusterBuffIndex + 1]);
			maxZ = as_float(gLightClusterInt[liLightClusterBuffIndex + 2]);
#endif
		}
#else
		int liLightCount = liLightCountIn;
#endif
		vec3 lViewDirVec3 = normalize(lPerFrameUniforms.gViewPositionVec3 - lPositionVec3.xyz);
		float lfNoV = saturate(dot(lNormalVec3, lViewDirVec3));

#if defined( D_USE_TILE_Z_EXTENTS )
		if (distanceInTile > minZ && distanceInTile < maxZ)
#endif
			for (int i = 0; i < liLightCount; ++i)
			{
#ifdef D_PLATFORM_METAL
#if defined(D_USE_TILE_Z_EXTENTS)
				const int bufferOffset = 3;
#else
				const int bufferOffset = 1;
#endif
				int lLightIndex = gLightClusterInt[liLightClusterBuffIndex + bufferOffset + i];
#else
				int lLightIndex = laLightIndices[i];
#endif
				if (lLightIndex >= D_MAX_LIGHT_COUNT) continue;

				vec3  lLightColourVec3;
				vec3  lLightDirectionVec3;
				float lfAttenuation;

#if defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_PROSPERO )
				float lfCutOff = 0.02;
				float lfOldCutOff = 0.05;
#else
				float lfCutOff = 0.05;
#endif
				{
					vec4 lLightPosIntensity = lPerFrameUniforms.gLocalLightPosMultiVec4[lLightIndex];
					vec4 lLightData = lPerFrameUniforms.gLocalLightDataMultiVec4[lLightIndex];

					vec4 lLightColourAndCookieIdx = Unpack_8_8_8_8(asuint(lLightData.y));
					float lfLightIntensity = lLightPosIntensity.w;
					lLightColourVec3 = lLightColourAndCookieIdx.xyz * lfLightIntensity;

					float lfFalloffType = lLightData.z; // Using fallOffType

					vec3 lLightPositionVec3 = lLightPosIntensity.xyz;

					vec4 lLightDirAndFOV = Unpack_2_10_10_10(asuint(lLightData.x));
					vec3 lSpotlightDirectionVec3 = lLightDirAndFOV.xyz;

					vec3 lPosToLight = lLightPositionVec3 - lPositionVec3;

					const vec3 toLight = lLightPositionVec3 - lPerFrameUniforms.gViewPositionVec3;
					float lfLightRadius = ComputeAttenuationRadius(lfFalloffType, 0.04f, lfLightIntensity);
					const float wDistance = dot(lViewDirVec3, toLight);
#if defined( D_USE_TILE_Z_EXTENTS )
					if (distanceInTile < max(0.0f, wDistance - lfLightRadius))
					{
						break; // since lights are sorted by closest distance, we can safely break here
					}
#endif

					lfAttenuation = ComputeAttenuation(lfFalloffType, lfCutOff, lfLightIntensity, lPosToLight);
					if (lfAttenuation < 0.0) continue;

#if defined( D_PLATFORM_SCARLETT ) || defined( D_PLATFORM_PROSPERO )
					lfAttenuation *= lfAttenuation <= lfOldCutOff / (1.0 - lfOldCutOff) ? lfAttenuation : 1.0;
#endif
					lLightDirectionVec3 = normalize(lPosToLight);

					// Conelight falloff (this can only attenuate down)
					float lfLightFOV = lLightData.w;
					if (lfLightFOV > -2.0)
					{
						float lfCookieStrength = 1.0;
						float lfConeAngle = dot(lSpotlightDirectionVec3, -lLightDirectionVec3);
						float lfConeAttenuation = saturate((lfConeAngle - lfLightFOV) * 5.0);
						lfAttenuation *= lfConeAttenuation;
						if (lfAttenuation <= (lfCutOff / (1.0 - lfCutOff)))
						{
							continue;
						}

					#if defined( D_PLATFORM_METAL ) && defined( D_TILED_LIGHTS ) && !defined(D_PLATFORM_IOS)
						const float lfCookieIdx = lLightColourAndCookieIdx.w * 255 - 1.0f;
						if (USE_SPOTLIGHT_COOKIES &&  lfCookieIdx >= 0.0)
						{
							vec3  lSpotAt = lSpotlightDirectionVec3.xyz;
							vec3  lSpotUp = GetWorldUp(lPositionVec3, lMeshUniforms.gPlanetPositionVec4);
							vec3  lSpotRight = cross(lSpotUp, lSpotAt);
							lSpotUp = cross(lSpotAt, lSpotRight);

							float lfConeRad = abs(sin(acos(lfLightFOV)));
							vec2  lConeUVs = vec2(dot(-lLightDirectionVec3, lSpotRight), -dot(-lLightDirectionVec3, lSpotUp));
							lConeUVs /= lfConeRad * 2.0;
							lConeUVs += 0.5;
							lfCookieStrength = texture2DArray(SAMPLER_GETMAP((&lCustomUniforms), gLightCookiesMap), vec3(lConeUVs, lfCookieIdx)).r;
							lLightColourVec3 *= lfCookieStrength;
						}
#endif
					}

					lfAttenuation = (lfAttenuation - lfCutOff) / (1.0 - lfCutOff);
					lfAttenuation = max(lfAttenuation, 0.0);
				}

				lLightColourVec3 = MUL(lLightColourVec3, sRGB_TO_P3D65);
				if (lfAttenuation > 0.0)
				{
					vec3  lSunColourVec3 = float2vec3(0.0);

#ifndef D_LIGHT_TERRAIN
					if ((liMaterialID & D_UNLIT) != 0)
					{
						lAccumulatedLightColourVec3 += lColourVec4.rgb * lfAttenuation;
					}
					else
#endif
					{
						float lAO = mix(0.61, 1.0, lfAO);

						vec3 lFinalLightColourVec3 = ComputeLocalLightColour(
							lLightDirectionVec3,
							lLightColourVec3,
							lNormalVec3,
							lColourVec4.rgb,
							lfMetallic,
							lfRoughness,
							lViewDirVec3,
							lfNoV
						);


						lFinalLightColourVec3 *= lfAttenuation;

						// this is kind of a hack
						// specular highlights are way too bright compared to other props
						// which causes them to look extremely blown out with the new bloom implementation
						// will need to find a better fix at some point :)
						lFinalLightColourVec3 = min(float2vec3(1.15), lFinalLightColourVec3);

						lAccumulatedLightColourVec3 += lFinalLightColourVec3;
					}

					if ((liMaterialID & D_GLOW) != 0)
					{
						const float lfLocalGlow = mix(lfGlow, lfGlow * sqrt(lfGlow), saturate(lfGlow - 1));
						lAccumulatedLightColourVec3 += lColourVec4.rgb * lfLocalGlow;
					}

				}  // attenuation cutoff
			}
#endif

		vec3 lLightColourVec3;
		vec3 lLightDirectionVec3;
		float lfAttenuation;
		float lfCutOff = 0.05;

		{
			float lLightColourW = lMeshUniforms.gLightColourVec4.w;
#ifdef D_PLATFORM_METAL
			// shift range, as lLightColourW == 0 is used to disable sun
			lLightColourW = max(0.0f, lLightColourW - 1);
#endif
			lLightColourVec3 = lMeshUniforms.gLightColourVec4.xyz * lLightColourW;
			lLightDirectionVec3 = -lMeshUniforms.gLightDirectionVec4.xyz;
			lfAttenuation = 1.0;
		}

		lLightColourVec3 = MUL(lLightColourVec3, sRGB_TO_P3D65);
		const bool sunEnabled = lMeshUniforms.gLightColourVec4.w > 0.0f;

		if ((liMaterialID & D_UNLIT) != 0)
		{
			lFragmentColourVec4.rgb += lColourVec4.rgb * lfAttenuation;
		}
		else if (sunEnabled)
		{
			float lAO = mix(0.25, 1.0, lfAO);
			vec3 lFinalLightColourVec3 = ComputeLightColour(
				lPerFrameUniforms,
				lMeshUniforms,
				SAMPLER2DPARAM(lShadowMap),
				SAMPLER2DPARAM(lCloudShadowMap),
				SAMPLER2DPARAM(lDualPMapBack),
				SAMPLER2DPARAM(lDualPMapFront),
				lLightDirectionVec3,
				lLightColourVec3,
				lCustomUniforms.gLightTopColourVec4,
				lPositionVec3,
				lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w,
				lNormalVec3,
				lColourVec4.rgb,
				lUpMatrix,
				liMaterialID,
				lfMetallic,
				lfRoughness,
				lfSubsurface,
				0.5,
				//TF_BEGIN
				lAO,
				//TF_END
#if defined(D_FORWARD_RENDERER)
				lfPixelSelfShadowing,
#endif
				lSunColourVec3);

			lFinalLightColourVec3 *= lfAttenuation;

			// this is kind of a hack
			// specular highlights are way too bright compared to other props
			// which causes them to look extremely blown out with the new bloom implementation
			// will need to find a better fix at some point :)
			lFinalLightColourVec3 = min(float2vec3(1.15), lFinalLightColourVec3);

			lFragmentColourVec4.rgb += lFinalLightColourVec3;
		}
	}
#endif	

#if !defined ( D_FORWARD_RENDERER )

#ifdef _F34_GLOW
	{
		lfGlow = mix(lfGlow, lfGlow * sqrt(lfGlow), saturate(lfGlow - 1));
		lFragmentColourVec4.rgb += lColourVec4.rgb * lfGlow;
	}
#endif

#if !defined( _F50_DISABLE_POSTPROCESS ) && !defined( D_NO_POSTPROCESS ) 
	{
		lFragmentColourVec4 = PostProcess(
			lPerFrameUniforms,
			lMeshUniforms,
			lCustomUniforms,
			lFragmentColourVec4,
			lPositionVec3,
			lNormalVec3,
			liMaterialID,
			lScreenSpacePositionVec4,
			lSunColourVec3);
	}
#endif

#if defined( D_FORWARD_EFFECTS_PASS )
		// boost glow for bloom effect pass to emulate separate BRIGHT pass
		lFragmentColourVec4.rgb *= 1.5;
#endif

#endif // !defined ( D_FORWARD_RENDERER )

#if defined( D_FORWARD_RENDERER ) && !defined(D_FORWARD_RENDERER_NO_POST)

	if ((liMaterialID & D_GLOW) != 0)
	{
		const float lfLocalGlow = mix(lfGlow, lfGlow * sqrt(lfGlow), saturate(lfGlow - 1));
		lFragmentColourVec4.rgb += lColourVec4.rgb * lfLocalGlow;
	}

#if !defined( _F50_DISABLE_POSTPROCESS ) && !defined( D_NO_POSTPROCESS ) 
	if ((liMaterialID & D_DISABLE_POSTPROCESS) == 0)
	{
		lFragmentColourVec4 = PostProcess(
			lPerFrameUniforms,
			lMeshUniforms,
			lCustomUniforms,
			lFragmentColourVec4,
			lPositionVec3,
			lNormalVec3,
			liMaterialID,
			lScreenSpacePositionVec4,
			lSunColourVec3);
	}
#endif

#ifdef D_FORWARD_EFFECTS_PASS
	{
		float maxChannel = max(lFragmentColourVec4.r, max(lFragmentColourVec4.g, lFragmentColourVec4.b));
		lFragmentColourVec4.rgb = vec3(maxChannel, maxChannel, maxChannel);
	}
#endif

#if !defined(_F07_UNLIT) && defined(D_TILED_LIGHTS)
	lFragmentColourVec4.rgb += lAccumulatedLightColourVec3;
#endif

	// Apply Split Shadow

	float lfParallaxShadow = 1.0;
	if ((liMaterialID & D_PARALLAX) != 0)
	{
		lfParallaxShadow = 1.0 - lfPixelSelfShadowing;
	}

#if defined ( D_NO_SHADOWS )
	float lfShadow = 0.0;
#else
	float lfShadow = ComputeShadowIntensity(
		SAMPLER2DPARAM(lShadowMap),
		lPerFrameUniforms,
		lMeshUniforms,
		lPositionVec3,
		vec3(0.0, 0.0, 0.0),
		lScreenSpacePositionVec4.xy,
		true);
#endif

	lfShadow *= lfParallaxShadow;
	
	vec2 lCloudScreenPositionVec2 = lScreenSpacePositionVec4.xy * 0.5 + 0.5;
	float lfOverlayValue;
	lfOverlayValue  = ComputeCloudOverlay(lCloudScreenPositionVec2, SAMPLER2DPARAM(lCloudShadowMap));

	lfShadow *= lfOverlayValue;

	// blend add
	const bool sunEnabled = lMeshUniforms.gLightColourVec4.w > 0.0f;
	if (sunEnabled)
	{
		lFragmentColourVec4.rgb += lSunColourVec3 * lfShadow;
	}

#endif

	//TF_BEGIN
#if defined(D_BLOOM)
	vec3  lBrightColourVec3 = lFragmentColourVec4.xyz;
	lBrightColourVec3 = clamp(lBrightColourVec3, float2vec3(0.0), float2vec3(1024.0));

	float lfThreshold = min(lCustomUniforms.gHDRParamsVec4.y, 1.0 - lfGlowFactor);
	float lfGain = dot(lBrightColourVec3, lBrightColourVec3);
	lfGain /= lCustomUniforms.gHDRParamsVec4.x * lCustomUniforms.gHDRParamsVec4.x;

	lfGain = clamp(lfGain, 0, 1024 * 1024);

	if (lfGain <= lfThreshold * lfThreshold)
		lBrightColourVec3 = float2vec3(0.0);
	else lBrightColourVec3 = BrightThreshold(lBrightColourVec3, lfThreshold, invsqrt(lfGain));

	float lBloomLum = P3D65_TO_Y(lBrightColourVec3);
#endif
#if defined(D_DOF)
	float linearDepth = ReverseZToLinearDepth(lPerFrameUniforms.gClipPlanesVec4, lScreenSpacePositionVec4.z / lScreenSpacePositionVec4.w);
	float lfPower = GetDofPower(linearDepth, lCustomUniforms.gDoFParamsVec4);
#endif

	//-----------------------------------------------------------------------------
	///
	///     Motion Vectors
	///
	//-----------------------------------------------------------------------------	
#ifdef D_OUTPUT_MOTION_VECTORS
	vec2  lScreenSpaceMotionVec4 = vec2(0.0, 0.0);
	if (HAS_MOTION_VECTORS)
	{
		lScreenSpaceMotionVec4 = lPrevScreenPositionVec4.xy / lPrevScreenPositionVec4.w - lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w;
	}
#else
	vec2 lScreenSpaceMotionVec4 = vec2(0, 0);
#endif


#if defined( D_DEFERRED_NORMAL_DECAL )
	lFragmentColourVec4.rgb = clamp(lFragmentColourVec4.rgb, 0, 1);
#endif


	//TF_END

	//-----------------------------------------------------------------------------
	///
	///     Output
	///
	//-----------------------------------------------------------------------------	
	lOutColours0Vec4 = lFragmentColourVec4;
	//TF_BEGIN
	lOutColours1Vec4 = vec4(
#if defined(D_BLOOM)
		lBloomLum,
#else
		0.0,
#endif
		0.0,
#if defined(D_DOF)
		lfPower,
#else
		0.0,
#endif 
		0.0);
	//TF_END
	lOutColours2Vec4 = vec4(lScreenSpaceMotionVec4, 0.0, 0.0);
	lOutColours3Vec4 = vec4(0.0, 0.0, 0.0, 0.0);
	lOutColours4Vec4 = vec4(0.0, 0.0, 0.0, 0.0);
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
#ifndef _F07_UNLIT
	in  mat3                      lUpMatrix,
	SAMPLER2DSHADOWARG(lShadowMap),
	SAMPLER2DARG(lCloudShadowMap),
	SAMPLER2DARG(lDualPMapBack),
	SAMPLER2DARG(lDualPMapFront),
#endif
	in  half                     lfPixelDepth,
	in  bool                      lbFrontFacing,
	in  half                     lfPixelSelfShadowing)
{
	half4 lFragmentColourVec4;
	half3 lSunColourVec3 = float2half3(0.0);

#if !defined( D_REFLECT_WATER_UP ) && !defined( D_REFLECT_WATER ) && !defined( D_REFLECT_DUALP )
	// colorspace conversion
#if defined ( D_PLATFORM_METAL )
	vec3 lWideColourVec3 = vec3(lColourVec4.xyz);
	lColourVec4.xyz = half3(MUL(lWideColourVec3, sRGB_TO_P3D65));
#else
	lColourVec4.xyz = MUL(lColourVec4.xyz, sRGB_TO_P3D65);
#endif
#endif

	lFragmentColourVec4 = lColourVec4;

	//-----------------------------------------------------------------------------
	///
	///     Lighting
	///
	//-----------------------------------------------------------------------------
#ifndef _F07_UNLIT
	{
		half3 lLightDirectionVec3 = half3(-lMeshUniforms.gLightDirectionVec4.xyz);

		lFragmentColourVec4.rgb = half3(ComputeLightColourHalf(
			lPerFrameUniforms,
			lMeshUniforms,
			SAMPLER2DPARAM(lShadowMap),
			SAMPLER2DPARAM(lCloudShadowMap),
			SAMPLER2DPARAM(lDualPMapBack),
			SAMPLER2DPARAM(lDualPMapFront),
			lLightDirectionVec3,
			half3(lMeshUniforms.gLightColourVec4.xyz * lMeshUniforms.gLightColourVec4.w),
			half4(lCustomUniforms.gLightTopColourVec4),
			lPositionVec3,
			lScreenSpacePositionVec4.xy / lScreenSpacePositionVec4.w,
			lNormalVec3,
			lColourVec4.rgb,
			lUpMatrix,
			liMaterialID,
			lfMetallic,
			lfRoughness,
			lfSubsurface,
			half(0.5),
			half(1.0),
			lSunColourVec3));
	}
#endif

#ifdef _F34_GLOW
	{
		lfGlow = mix(lfGlow, lfGlow * sqrt(lfGlow), saturate(lfGlow - half(1.0)));
		lFragmentColourVec4.rgb += lColourVec4.rgb * lfGlow;
	}
#endif

#if !defined( _F50_DISABLE_POSTPROCESS ) && !defined( D_NO_POSTPROCESS ) 
	{
#if defined ( D_PLATFORM_METAL )
		vec3 lSunColourOutVec3 = vec3(lSunColourVec3);

		lFragmentColourVec4 = half4(PostProcess(
			lPerFrameUniforms,
			lMeshUniforms,
			lCustomUniforms,
			vec4(lFragmentColourVec4),
			lPositionVec3,
			vec3(lNormalVec3),
			liMaterialID,
			lScreenSpacePositionVec4,
			lSunColourOutVec3));

		lSunColourVec3 = half3(lSunColourOutVec3);
#else
		lFragmentColourVec4 = PostProcess(
			lPerFrameUniforms,
			lMeshUniforms,
			lCustomUniforms,
			lFragmentColourVec4,
			lPositionVec3,
			lNormalVec3,
			liMaterialID,
			lScreenSpacePositionVec4,
			lSunColourVec3);
#endif
	}
#endif

	//-----------------------------------------------------------------------------
	///
	///     Output
	///
	//-----------------------------------------------------------------------------	
	lOutColours0Vec4 = lFragmentColourVec4;
	lOutColours1Vec4 = half4(0.0, 0.0, 0.0, 0.0);
	lOutColours2Vec4 = half4(0.0, 0.0, 0.0, 0.0);
	lOutColours3Vec4 = half4(0.0, 0.0, 0.0, 0.0);
	lOutColours4Vec4 = half4(0.0, 0.0, 0.0, 0.0);
}

