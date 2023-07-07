////////////////////////////////////////////////////////////////////////////////
///
///     @file       UberVertexShader.h
///     @author     User
///     @date       
///
///     @brief      UberVertexShader
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Include files

#include "Common/Defines.shader.h"

//-----------------------------------------------------------------------------
//      Compilation defines 

#if defined(D_PLATFORM_SWITCH) || defined (D_PLATFORM_METAL)
// Just to help with occupancy as we aren't using octahedral importers for now, disable the runtime
// code-path for them completely on Switch. The perf impact of enabling this isn't that high, but
// might as well be safe rather than sorry. (Previous reasons why having this enabled was high was
// that the fragment depth writing disabled early-z culling).
#define ENABLE_OCTAHEDRAL_IMPOSTERS 0
#else
#define ENABLE_OCTAHEDRAL_IMPOSTERS 1
#endif

// A define if you want to disable the ability to runtime switch between octahedral and "simple" (old) imposters
#define FORCE_ONLY_OCTAHEDRAL_IMPOSTERS 0

// Just make sure forcing octahedral imposters *does* force them on
#if FORCE_ONLY_OCTAHEDRAL_IMPOSTERS
#undef ENABLE_OCTAHEDRAL_IMPOSTERS
#define ENABLE_OCTAHEDRAL_IMPOSTERS 1
#endif

#define FORCE_ENABLE_FRAME_BLEND 1
#define FORCE_DISABLE_FRAME_BLEND 0
#define FORCE_ENABLE_DEPTH_REPROJECTION 0
#define FORCE_DISABLE_DEPTH_REPROJECTION 0
    
STATIC_CONST bool SELECT_A_FRAME = false;
// Setting these to non-zero threshold seems to have a negligable impact on perf at the expense of quality (more popping), so leaving these
// here as-is.
STATIC_CONST float kfImposterAlphaThreshold = 0.0;
#if defined(D_PLATFORM_SWITCH)
STATIC_CONST float kfBlendweightClampThreshold = 0.1;
#else
STATIC_CONST float kfBlendweightClampThreshold = 0.01;
#endif


#if FORCE_ONLY_OCTAHEDRAL_IMPOSTERS
#define OCTAHEDRAL_IMPOSTERS_ENABLED(lImposterQualitySettingsVec4) (true)
#elif !ENABLE_OCTAHEDRAL_IMPOSTERS
#define OCTAHEDRAL_IMPOSTERS_ENABLED(lImposterQualitySettingsVec4) (false)
#else
#define OCTAHEDRAL_IMPOSTERS_ENABLED(lImposterQualitySettingsVec4) (lImposterQualitySettingsVec4.x != 0.0)
#endif

#if ENABLE_OCTAHEDRAL_IMPOSTERS && defined(_F44_IMPOSTER) && !defined(D_PLATFORM_SWITCH)
// On Switch the lack of early-z has a massive negative perf impact
#define IMPOSTER_ENABLE_FRAGMENT_DEPTH_WRITING 1
#else
#define IMPOSTER_ENABLE_FRAGMENT_DEPTH_WRITING 0
#endif

#if defined( D_PLATFORM_METAL )
#undef OCTAHEDRAL_IMPOSTERS_ENABLED
#define OCTAHEDRAL_IMPOSTERS_ENABLED(lImposterQualitySettingsVec4) (false)
#endif

//-----------------------------------------------------------------------------
//      Functions

float
CalculateImposterTexCoordsU( 
    vec3 lWorldPositionVec3, 
    vec3 lViewPositionVec3, 
    vec3 lImposterAt, 
    vec3 lWindingAxis, 
    vec2 lTexCoordsVec2,
    vec4 lImposterDataVec4 )
{
    
    vec3 lCameraAtVec3  = normalize( lViewPositionVec3 - lWorldPositionVec3 );
    //vec3 lCameraAtVec3  = normalize( lCameraAt );
    lImposterAt         = normalize( lImposterAt );
    
    /*
    vec3 lCameraAtVec3   = gImposterDebug.xyz;
    lCameraAtVec3 = normalize( lCameraAtVec3 );
    lImposterAt = vec3( 0.0, 0.0, -1.0 );
    */
    float lfAngle;
    vec3   lAxis;

    // Dot them together to get cosine of angle between them
    lfAngle = dot( -lCameraAtVec3, lImposterAt );

    // Ensure it's in the appropriate range (it might be just outside this range due to rounding errors)
    lfAngle = clamp( lfAngle, -1.0, 1.0 );
    lfAngle = acos(lfAngle) * 57.29577951;

    // Cross products wind counter-clockwise, so everything will be negated.
    lAxis = normalize( cross(lImposterAt, lCameraAtVec3) );
    if ( dot( lAxis, lWindingAxis ) >= 0.0 )
    {
        lfAngle = 360.0 - lfAngle;
    }

    float lfFakeDot = lfAngle * 0.002778; // 1.0 / 360.0 = 0.00277777777..
    
    // n discrete views - must match the settings in GcFadeNode.cpp
    float lfU = ( 1.0 - lTexCoordsVec2.x ) * lImposterDataVec4.y; //lImposterDataVec4.x = kiNumImposterViews. lIposterData.y = 1.0f / kiNumImposterViews.
    lfU = ( lfU + ( floor( lfFakeDot * lImposterDataVec4.x ) * lImposterDataVec4.y ) );
    //lfU = lfU + floor( lfFakeDot );

    // all imposters (debug)
    //float lfU = lTexCoordsVec2.x;

    return lfU;

}



float
EncodeBitInNormalFloat(float lfNormalFloat, bool lbBitToEncode )
{
    int liFloatValue = asint( lfNormalFloat );
    if (lbBitToEncode)
    {
        // As normal ranges from -1.0 to 1.0, first bit of exponent will never be set - so we can encode a bit there
        liFloatValue |= 1 << 30;
    }
    lfNormalFloat = asfloat( liFloatValue );
    return lfNormalFloat;
}

void
DecodeBitFromNormalFloat(
    float lfNormalFloat,
    out float lfDecodedFloat, out bool lbDecodedBit )
{
    int liFloatValue = asint( lfNormalFloat );
    lbDecodedBit = (liFloatValue & (1 << 30)) != 0;
    liFloatValue &= ~(1 << 30);
    lfDecodedFloat = asfloat( liFloatValue );
}

vec3
GridToHemiSphere(vec2 lNormalisedGridCoordinate)
{
    float lXPos = lNormalisedGridCoordinate.x - lNormalisedGridCoordinate.y; // Encode one diagonal across the grid as the xAxis, x == y
    float lZPos = lNormalisedGridCoordinate.x - (1.0f - lNormalisedGridCoordinate.y); // Encode other diagonal across the grid as zAxis, x == (1 - y)

    float lYPos = 1.0f - abs(lXPos) - abs(lZPos); // Encode the height as the closest point we are to the centre!
    vec3 lResult = vec3(lXPos, lYPos, lZPos);
    lResult = normalize(lResult);
    return lResult;
}


vec2
HemiSphereToGrid(vec3 lHemiSpherePos)
{
    lHemiSpherePos.y = abs(lHemiSpherePos.y);
    vec3 lOctant = vec3(sign(lHemiSpherePos));

    //  |x| + |y| + |z| = 1
    float lSum = dot(lHemiSpherePos, lOctant);
    vec3 lOctahedron = lHemiSpherePos / lSum;

    vec2 grid = vec2(
        lOctahedron.x + lOctahedron.z,
        lOctahedron.z - lOctahedron.x);

    grid = clamp((grid + vec2(1.0, 1.0)) * 0.5, vec2(0.0, 0.0), vec2(1.0, 1.0));

    return grid;
}

vec3
CalculateBlendWeights(vec2 lGridOffset)
{
    vec3 lBlendWeights;
    // Calculate the blend weights
    lBlendWeights.x = min(1.0 - lGridOffset.x, 1.0 - lGridOffset.y);
    lBlendWeights.y = abs(lGridOffset.x - lGridOffset.y);
    lBlendWeights.z = min(lGridOffset.x, lGridOffset.y);
    lBlendWeights.xyz /= (lBlendWeights.x + lBlendWeights.y + lBlendWeights.z);
    lBlendWeights.xyz *= lBlendWeights.xyz;
    lBlendWeights.xyz *= lBlendWeights.xyz;
    lBlendWeights.xyz -= vec3(kfBlendweightClampThreshold, kfBlendweightClampThreshold, kfBlendweightClampThreshold);
    lBlendWeights = saturate(lBlendWeights);
    lBlendWeights.xyz /= (lBlendWeights.x + lBlendWeights.y + lBlendWeights.z);
    return lBlendWeights;
}

vec2
ReprojectUVs(
    vec2 lTexCoordsProjected0,
    vec3 frameRayVec3,
    vec3 lVertexToCamera,
    vec3 lVertexToCameraNormalised,
    vec3 lLocalCameraPos,
    vec2 lInvImposterDimensions,
    out vec2 outFrameProjection
)
{
    vec3 framePlaneX = normalize( cross( frameRayVec3, vec3( 0, 1, 0 ) ) ) * lInvImposterDimensions.x;
    vec3 framePlaneY = normalize( cross( framePlaneX, frameRayVec3 ) ) * lInvImposterDimensions.y;

    outFrameProjection = vec2( dot( lVertexToCameraNormalised, framePlaneX ), dot( lVertexToCameraNormalised, framePlaneY ) );

    vec3 lOffsetVector = lVertexToCamera * (dot( frameRayVec3, lLocalCameraPos ) / dot( frameRayVec3, lVertexToCamera )) - lLocalCameraPos;

    lTexCoordsProjected0 = vec2(
        dot( framePlaneX, lOffsetVector),
        dot( framePlaneY, lOffsetVector)
    );

    lTexCoordsProjected0 = (lTexCoordsProjected0) + vec2(0.5, 0.5);

    return lTexCoordsProjected0;
}

void
ImposterVertexShadow(
    vec4 lWorldPositionVec4,
    vec3 lProperLocalPos,
    vec3 lViewPositionVec3,
    vec3 lLocalCameraPos,
    vec3 lImposterAt,
    vec2 lTexCoordsVec2,
    vec4 lImposterDataVec4,
    vec4 lOctahedralImposterDataVec4,
    out vec2 outTexCoords,
    out vec2 outImposterFrame 
)
{
    const float kfNumImposterViews = lImposterDataVec4.x;
    const float kfNumImposterViewsMinusOne = lImposterDataVec4.x - 1.0;  
    const float kfInvNumImposterViews = lImposterDataVec4.y;
    const float kfInvNumImposterViewsMinusOne = lImposterDataVec4.z;
    const vec2  lInvImposterDimensions = lOctahedralImposterDataVec4.xy;
    const float lfImposterHeight = lOctahedralImposterDataVec4.w;
    const float kfClipDistance = lImposterDataVec4.w;
     
    vec3 lCameraAtVec3 = normalize( lLocalCameraPos );

    // We need to offset the camera relative to the height of the target - for some reason, not entirely sure why
    lProperLocalPos -= vec3( 0.0, lfImposterHeight * 0.5, 0.0 );
    lLocalCameraPos -= vec3( 0.0, lfImposterHeight * 0.5, 0.0 );

    vec2 lNormalisedGridCoordinate = HemiSphereToGrid( lCameraAtVec3 );

    vec2 lGridCoordinate = lNormalisedGridCoordinate * kfNumImposterViewsMinusOne;

    // Map back to 3D octahedron locations from nearest valid grid coordinates
    vec2 lImposterFrame = floor( lGridCoordinate );

    vec3 lVertexToCamera = lLocalCameraPos - lProperLocalPos;
    vec3 lVertexToCameraNormalised = normalize( lVertexToCamera );


    vec2 lIgnoredFrameProjection;
    vec2 lTexCoords = ReprojectUVs(
        lTexCoordsVec2,
        GridToHemiSphere( lImposterFrame * kfInvNumImposterViewsMinusOne ),
        lVertexToCamera,
        lVertexToCameraNormalised,
        lLocalCameraPos,
        lInvImposterDimensions,
        lIgnoredFrameProjection
    );

    outImposterFrame = lImposterFrame;
    outTexCoords = lTexCoords;
}

void
ImposterVertex(
    vec4 lWorldPositionVec4,
    vec3 lProperLocalPos,
    vec3 lViewPositionVec3,
    vec3 lLocalCameraPos,
    vec3 lImposterAt,
    vec2 lTexCoordsVec2,
    vec4 lImposterDataVec4,
    vec4 lOctahedralImposterDataVec4,
    vec4 lImposterQualitySettingsVec4,
    out vec2 outGridCoordinatePos,
    //out vec3 outVertexPos,
    out vec3 outNormal,
    out vec4 outBlendWeights,
    out bool outFrameBlendEnabled,
    out bool outDepthReprojectionEnabled,
    out vec2 outTexCoords0,
    out vec2 outTexCoords1,
    out vec2 outTexCoords2,
    out vec2 outImposterFrame0,
    out vec2 outImposterFrame1,
    out vec2 outImposterFrame2,
    out vec2 outFrameProjection0,
    out vec2 outFrameProjection1,
    out vec2 outFrameProjection2
)
{
    const float kfNumImposterViews = lImposterDataVec4.x;
    const float kfNumImposterViewsMinusOne = lImposterDataVec4.x - 1.0;
    const float kfInvNumImposterViews = lImposterDataVec4.y;
    const float kfInvNumImposterViewsMinusOne = lImposterDataVec4.z;
    const vec2  lInvImposterDimensions = lOctahedralImposterDataVec4.xy;
    const float lfImposterHeight = lOctahedralImposterDataVec4.w;
    const float kfClipDistance = lImposterDataVec4.w;

    float lfCameraDistance = dot( lLocalCameraPos, lLocalCameraPos );
    bool lbFrameBlendEnabled         = lfCameraDistance < lImposterQualitySettingsVec4.y;
#if FORCE_ENABLE_FRAME_BLEND
    lbFrameBlendEnabled = true;
#elif FORCE_DISABLE_FRAME_BLEND
    lbFrameBlendEnabled = false;
#endif
    bool lbDepthReprojectionEnabled = lfCameraDistance < lImposterQualitySettingsVec4.z;
#if FORCE_ENABLE_DEPTH_REPROJECTION
    lbDepthReprojectionEnabled = true;
#elif FORCE_DISABLE_DEPTH_REPROJECTION
    lbDepthReprojectionEnabled = false;
#endif
    outFrameBlendEnabled        = lbFrameBlendEnabled;
    outDepthReprojectionEnabled = lbDepthReprojectionEnabled;
    vec3 lCameraAtVec3 = normalize( lLocalCameraPos );

    // We need to offset the camera relative to the height of the target - for some reason, not entirely sure why
    lProperLocalPos -= vec3( 0.0, lfImposterHeight * 0.5, 0.0 );
    lLocalCameraPos -= vec3( 0.0, lfImposterHeight * 0.5, 0.0 );

    vec2 lNormalisedGridCoordinate = HemiSphereToGrid(lCameraAtVec3);

    vec2 lGridCoordinate = lNormalisedGridCoordinate * kfNumImposterViewsMinusOne;
    vec3 lBlendWeights;
    float lGridOffsetB;
    {
        vec2 lGridOffset = fract(lGridCoordinate);
        lBlendWeights = CalculateBlendWeights(lGridOffset);

        // Corresponds to picking (0, 1) or (1, 0) as our second
        // coordinate relative to our base based on which fractional
        // component of our grid coordinate is larger.
        lGridOffsetB = ceil(lGridOffset.x - lGridOffset.y);
    }

    // Map back to 3D octahedron locations from nearest valid grid coordinates
    vec2 lImposterFrame0 = floor(lGridCoordinate);
    vec2 lImposterFrame1 = lImposterFrame0 + mix(vec2(0.0, 1.0), vec2(1.0, 0.0), lGridOffsetB);
    vec2 lImposterFrame2 = lImposterFrame0 + vec2(1.0, 1.0);

    vec3 lWeightedSpherePosition = vec3(0.0, 0.0, 0.0);

    lWeightedSpherePosition = normalize(lWeightedSpherePosition);

    outGridCoordinatePos = lImposterFrame0;

    vec3 lVertexToCamera = lLocalCameraPos - lProperLocalPos;
    vec3 lVertexToCameraNormalised = normalize( lVertexToCamera );

    vec2 lTexCoords0;
    vec2 lTexCoords1 = vec2(1.0, 1.0);
    vec2 lTexCoords2 = vec2(1.0, 1.0);
    if (lbFrameBlendEnabled)
    {
        outNormal = lCameraAtVec3;
        lWeightedSpherePosition = GridToHemiSphere(lImposterFrame0 * kfInvNumImposterViewsMinusOne);

        lTexCoords0 = ReprojectUVs(
            lTexCoordsVec2,
            lWeightedSpherePosition,
            lVertexToCamera,
            lVertexToCameraNormalised,
            lLocalCameraPos,
            lInvImposterDimensions,
            outFrameProjection0
        );

        lWeightedSpherePosition *= lBlendWeights.x;

        {
            vec3 lHemiSpherePos = GridToHemiSphere(lImposterFrame1 * kfInvNumImposterViewsMinusOne);
            lTexCoords1 = ReprojectUVs(
                lTexCoordsVec2,
                lHemiSpherePos,
                lVertexToCamera,
                lVertexToCameraNormalised,
                lLocalCameraPos,
                lInvImposterDimensions,
                outFrameProjection1
            );

            lWeightedSpherePosition += lHemiSpherePos * lBlendWeights.y;
        }

        {
            vec3 lHemiSpherePos = GridToHemiSphere(lImposterFrame2 * kfInvNumImposterViewsMinusOne);
            lTexCoords2 = ReprojectUVs(
                lTexCoordsVec2,
                GridToHemiSphere(lImposterFrame2 * kfInvNumImposterViewsMinusOne),
                lVertexToCamera,
                lVertexToCameraNormalised,
                lLocalCameraPos,
                lInvImposterDimensions,
                outFrameProjection2
            );
            lWeightedSpherePosition += lHemiSpherePos * lBlendWeights.z;
        }
    }
    else if ((SELECT_A_FRAME) || (lBlendWeights.x > lBlendWeights.y && lBlendWeights.x > lBlendWeights.z))
    {
        outNormal = GridToHemiSphere(lImposterFrame0 * kfInvNumImposterViewsMinusOne);

        lTexCoords0 = ReprojectUVs(
            lTexCoordsVec2,
            outNormal,
            lVertexToCamera,
            lVertexToCameraNormalised,
            lLocalCameraPos,
            lInvImposterDimensions,
            outFrameProjection0
        );
    }
    else if (lBlendWeights.y > lBlendWeights.z)
    {
        outNormal = GridToHemiSphere(lImposterFrame1 * kfInvNumImposterViewsMinusOne);

        lTexCoords0 = ReprojectUVs(
            lTexCoordsVec2,
            outNormal,
            lVertexToCamera,
            lVertexToCameraNormalised,
            lLocalCameraPos,
            lInvImposterDimensions,
            outFrameProjection0
        );

        lImposterFrame0 = lImposterFrame1;
    }
    else
    {
        outNormal = GridToHemiSphere(lImposterFrame2 * kfInvNumImposterViewsMinusOne);

        lTexCoords0 = ReprojectUVs(
            lTexCoordsVec2,
            outNormal,
            lVertexToCamera,
            lVertexToCameraNormalised,
            lLocalCameraPos,
            lInvImposterDimensions,
            outFrameProjection0
        );

        lImposterFrame0 = lImposterFrame2;
    }

    outBlendWeights = vec4(lBlendWeights, lGridOffsetB);

    outTexCoords0 = lTexCoords0;
    outTexCoords1 = lTexCoords1;
    outTexCoords2 = lTexCoords2;

    outImposterFrame0 = lImposterFrame0;
    outImposterFrame1 = lImposterFrame1;
    outImposterFrame2 = lImposterFrame2;
}

void TransformNormal(
    float lfBlendWeight,
    mat3 lWorldNormalMat3,
    vec4 lSampledNormalXYZ_DepthW,
    inout vec3 lWorldNormal,
    inout float lfDepth )
{
    lSampledNormalXYZ_DepthW.xyz = lSampledNormalXYZ_DepthW.xyz * 2.0 - 1.0;
    // is this normalise needed?
    lSampledNormalXYZ_DepthW.xyz = MUL( lWorldNormalMat3, lSampledNormalXYZ_DepthW.xyz );

    lWorldNormal += lfBlendWeight * lSampledNormalXYZ_DepthW.xyz;
    lfDepth += lfBlendWeight * lSampledNormalXYZ_DepthW.w;
}

void
ImposterSample(
    vec3 lBlendWeights, vec4 lImposterDataVec4, vec4 lImposterQualitySettingsVec4,
    mat3 lWorldNormalMat3, bool lbFrameBlendEnabled,
    vec2 lTexCoords0, vec2 lTexCoords1, vec2 lTexCoords2,
    SAMPLER2DARG(lDiffuseMap), SAMPLER2DARG( lMaskMap ), SAMPLER2DARG( lNormalMap ),
    out vec4 lOutColourVec4,
    out vec4 lOutMaskVec4,
    out vec3 lOutNormalVec4,
    out float lfOutDepth
    )
{
    lOutColourVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );
    lOutMaskVec4   = vec4( 0.0, 0.0, 0.0, 0.0 );
    lOutNormalVec4 = vec3( 0.0, 0.0, 0.0 );
    lfOutDepth     = 0.0;
    if (lbFrameBlendEnabled)
    {
        if (lBlendWeights.x > 0.0)
        {
            lOutColourVec4 = lTexCoords0.x >= 0.0 ? texture2D( lDiffuseMap, lTexCoords0.xy ) : vec4( 0.0, 0.0, 0.0, 0.0 );

            if (lOutColourVec4.a > kfImposterAlphaThreshold)
            {
                vec4 lSampledNormalXYZ_DepthW = texture2D( lNormalMap, lTexCoords0.xy );
                lOutColourVec4 *= lBlendWeights.x;
                vec4 lMaskVec4 = texture2D( lMaskMap, lTexCoords0.xy );
                TransformNormal( lBlendWeights.x, lWorldNormalMat3, lSampledNormalXYZ_DepthW, lOutNormalVec4, lfOutDepth );
                lOutMaskVec4 += lMaskVec4 * lBlendWeights.x;
            }
            else
            {
                lOutColourVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );
            }
        }

        if (lBlendWeights.y > 0.0)
        {
            vec4 lColour1Vec4 = lTexCoords1.x >= 0.0 ? texture2D( lDiffuseMap, lTexCoords1.xy ) : vec4( 0.0, 0.0, 0.0, 0.0 );

            if (lColour1Vec4.a > kfImposterAlphaThreshold)
            {
                vec4 lSampledNormalXYZ_DepthW = texture2D( lNormalMap, lTexCoords1.xy );
                lOutColourVec4 += lColour1Vec4 * lBlendWeights.y;
                vec4 lMaskVec4 = texture2D( lMaskMap, lTexCoords1.xy );
                TransformNormal( lBlendWeights.y, lWorldNormalMat3, lSampledNormalXYZ_DepthW, lOutNormalVec4, lfOutDepth );
                lOutMaskVec4 += lMaskVec4 * lBlendWeights.y;
            }
        }


        if (lBlendWeights.z > 0.0)
        {
            vec4 lColour2Vec4 = lTexCoords2.x >= 0.0 ? texture2D( lDiffuseMap, lTexCoords2.xy ) : vec4( 0.0, 0.0, 0.0, 0.0 );

            if (lColour2Vec4.a > kfImposterAlphaThreshold)
            {
                lOutColourVec4 += lColour2Vec4 * lBlendWeights.z;
                vec4 lSampledNormalXYZ_DepthW = texture2D( lNormalMap, lTexCoords2.xy );
                vec4 lMaskVec4 = texture2D( lMaskMap, lTexCoords2.xy );
                TransformNormal( lBlendWeights.z, lWorldNormalMat3, lSampledNormalXYZ_DepthW, lOutNormalVec4, lfOutDepth );
                lOutMaskVec4 += lMaskVec4 * lBlendWeights.z;
            }
        }
    }
    else
    {
        lOutColourVec4 = lTexCoords0.x >= 0.0 ? texture2D( lDiffuseMap, lTexCoords0.xy ) : vec4( 0.0, 0.0, 0.0, 0.0 );
        if (lOutColourVec4.a > 0.0)
        {
            lOutMaskVec4 += texture2D( lMaskMap, lTexCoords0.xy );
            TransformNormal( 1.0, lWorldNormalMat3, texture2D( lNormalMap, lTexCoords0.xy ), lOutNormalVec4, lfOutDepth );
        }
        else
        {
            lOutColourVec4 = vec4( 0.0, 0.0, 0.0, 0.0 );
        }
    }
}

vec2 RecalculateUV(
    const float kfInvNumImposterViews,
    const float lfClipDistance,
    vec4 lImposterQualitySettingsVec4,
    bool lbDepthReprojectionEnabled,
    vec2 lTexCoords,
    vec2 lImposterFrame,
    vec2 lFrameProjection,
    SAMPLER2DARG( lImposterMapNormal ),
    SAMPLER2DARG( lImposterMapDiffuse )
)
{
    if (lbDepthReprojectionEnabled)
    {
        lTexCoords.x = ((lTexCoords.x - 0.5) * lImposterQualitySettingsVec4.x) + 0.5;
        vec2 lSampleCoords = kfInvNumImposterViews * (lTexCoords + lImposterFrame);

        float lfAlpha = texture2D( lImposterMapDiffuse, lSampleCoords ).a;

        // Don't depth reproject where imposter fragments don't exist (this could be simplified possibly/made less expensive).
        if (lfAlpha > kfImposterAlphaThreshold)
        {
            // We divide the read depth value by the alpha to recover what the depth value is at the
            // edges of objects to prevent harsh discontinuities
            float lfDepth = texture2D( lImposterMapNormal, lSampleCoords ).w / lfAlpha;
            vec2 lfTexCoordsOffset = (lFrameProjection * (lfDepth - 0.5) * lfClipDistance);
            lfTexCoordsOffset.x *= lImposterQualitySettingsVec4.x;
            lTexCoords = lfTexCoordsOffset + lTexCoords;
        }
        else
        {
            lTexCoords.x = -1.0;
        }
    }

    if (lTexCoords.x < 0.0 || lTexCoords.x >= 1.0 || lTexCoords.y < 0.0 || lTexCoords.y >= 1.0)
    {
        lTexCoords.x = -1.0;
    }
    return lTexCoords;
}


void ImposterFrag(
    vec3 lBlendWeights, vec4 lImposterDataVec4, vec4 lImposterQualitySettingsVec4, vec3 lWorldNormal,
    mat3 lWorldNormalMat3, bool lbFrameBlendEnabled, bool lbDepthReprojectionEnabled,
    vec2 lTexCoords0, vec2 lTexCoords1, vec2 lTexCoords2,
    vec2 lImposterFrame0, vec2 lImposterFrame1, vec2 lImposterFrame2,
    vec2 lFrameProjection0, vec2 lFrameProjection1, vec2 lFrameProjection2,
    SAMPLER2DARG( lImposterMapDiffuse ), SAMPLER2DARG( lImposterMapMask ), SAMPLER2DARG( lImposterMapNormal ),
    out vec4 outDiffuse, out vec4 outMask, out vec3 outNormal, out float outDepth
)
{
    const float kfNumImposterViews = lImposterDataVec4.x;
    const float kfNumImposterViewsMinusOne = lImposterDataVec4.x - 1.0;
    const float kfInvNumImposterViews = lImposterDataVec4.y;
    const float kfInvNumImposterViewsMinusOne = lImposterDataVec4.z;
    const float kfClipDistance = lImposterDataVec4.w;

#if FORCE_ENABLE_FRAME_BLEND
    lbFrameBlendEnabled = true;
#elif FORCE_DISABLE_FRAME_BLEND
    lbFrameBlendEnabled = false;
#endif

#if FORCE_ENABLE_DEPTH_REPROJECTION
    lbDepthReprojectionEnabled = true;
#elif FORCE_DISABLE_DEPTH_REPROJECTION
    lbDepthReprojectionEnabled = false;
#endif

    if (lbFrameBlendEnabled)
    {

        lTexCoords0 = lBlendWeights.x == 0.0 ? vec2( -1.0, -1.0 ) : RecalculateUV( kfInvNumImposterViews, kfClipDistance, lImposterQualitySettingsVec4, lbDepthReprojectionEnabled, lTexCoords0, lImposterFrame0, lFrameProjection0, SAMPLER2DPARAM( lImposterMapNormal ), SAMPLER2DPARAM( lImposterMapDiffuse ) );
        lTexCoords1 = lBlendWeights.y == 0.0 ? vec2( -1.0, -1.0 ) : RecalculateUV( kfInvNumImposterViews, kfClipDistance, lImposterQualitySettingsVec4, lbDepthReprojectionEnabled, lTexCoords1, lImposterFrame1, lFrameProjection1, SAMPLER2DPARAM( lImposterMapNormal ), SAMPLER2DPARAM( lImposterMapDiffuse ) );
        lTexCoords2 = lBlendWeights.z == 0.0 ? vec2( -1.0, -1.0 ) : RecalculateUV( kfInvNumImposterViews, kfClipDistance, lImposterQualitySettingsVec4, lbDepthReprojectionEnabled, lTexCoords2, lImposterFrame2, lFrameProjection2, SAMPLER2DPARAM( lImposterMapNormal ), SAMPLER2DPARAM( lImposterMapDiffuse ) );

        if ( lTexCoords0.x == -1.0 && lTexCoords1.x == -1.0 && lTexCoords2.x == -1.0 )
        {
#if defined(D_FRAGMENT)
            discard;
#endif
        }

        lTexCoords0 = lTexCoords0.x >= 0.0 ? kfInvNumImposterViews * (lTexCoords0 + lImposterFrame0) : vec2(-1.0, -1.0);
        lTexCoords1 = lTexCoords1.x >= 0.0 ? kfInvNumImposterViews * (lTexCoords1 + lImposterFrame1) : vec2(-1.0, -1.0);
        lTexCoords2 = lTexCoords2.x >= 0.0 ? kfInvNumImposterViews * (lTexCoords2 + lImposterFrame2) : vec2(-1.0, -1.0);
    }
    else
    {
        lTexCoords0 = RecalculateUV( kfInvNumImposterViews, kfClipDistance, lImposterQualitySettingsVec4, lbDepthReprojectionEnabled, lTexCoords0, lImposterFrame0, lFrameProjection0, SAMPLER2DPARAM( lImposterMapNormal ), SAMPLER2DPARAM( lImposterMapDiffuse ) );
        
        if (lTexCoords0.x == -1.0 || lBlendWeights.x == 0.0)
        {
#if defined(D_FRAGMENT)
            discard;
#endif
        }
        
        lTexCoords0 = lTexCoords0.x >= 0.0 ? kfInvNumImposterViews * (lTexCoords0 + lImposterFrame0) : vec2(-1.0, -1.0);
    }

    ImposterSample(
        lBlendWeights, lImposterDataVec4, lImposterQualitySettingsVec4, lWorldNormalMat3, lbFrameBlendEnabled, lTexCoords0, lTexCoords1, lTexCoords2,
        SAMPLER2DPARAM( lImposterMapDiffuse ), SAMPLER2DPARAM( lImposterMapMask ), SAMPLER2DPARAM( lImposterMapNormal ),
        outDiffuse, outMask, outNormal, outDepth
    );
    outNormal = normalize( outNormal );
    outDepth = ( outDepth - 0.5 ) * kfClipDistance;
}