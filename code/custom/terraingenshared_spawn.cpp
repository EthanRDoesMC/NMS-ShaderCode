//-----------------------------------------------------------------------------
//Terrain gen cpp code shared between CPU and GPU
//This can be included from a single c++ file in c++ code
//-----------------------------------------------------------------------------

#ifndef _TGENSHARED_SPAWN_CPP_
#define _TGENSHARED_SPAWN_CPP_
#include "Custom/TerrainGenShared_Spawn.h"
#include "Custom/TerrainGenShared_Noise.h"
#include "Custom/SharedBegin.inl"

//TK_DISABLE_OPTIMIZATION

float
SpawnObject_GetHeightAboveGround(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lWorldPosition)
{
    vec3 lOffset = (lWorldPosition - lParams.mRegionMapPosition);
    return length(lOffset) - lParams.mfRegionMapRadius;
}
vec3 
SpawnObject_WorldOffsetToMapPosition(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lWorldOffset, OUTPARAM(vec3) lNormal )
{         
    vec3 lLocalSurfacePos;
    vec3 lMapPos;
    float   lfHeight;

    //get / check height
    lfHeight = length(lWorldOffset);
    if( lfHeight <= 1.525878906e-5f )
        return vec3( 0.0f );
                        
    // Set local world to the surface of the sphere
    lLocalSurfacePos = lWorldOffset * lParams.mfRegionMapRadius / lfHeight;

    // Map back to the cube
    lMapPos = ProjectSphereToCube( lLocalSurfacePos, lNormal, lParams.mfRegionMapRadius );

    // Add the height back in
    lMapPos += lNormal * (lfHeight - lParams.mfRegionMapRadius);

    return lMapPos;
}
vec3 
SpawnObject_MapPositionToWorldOffset(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lMapPosition, vec3 lNormal )
{
    // Transform into unit cube space
    float   lfRadius = lParams.mfRegionMapRadius;
    float   lfHeight = dot( lMapPosition, lNormal ) - lfRadius;

    // Set cube position to be at the surface of cube
    vec3 lMapSurfacePos = lMapPosition - lNormal*lfHeight;

    // Transform to sphere
    vec3 lWorldOffset = ProjectCubeToSphere( lMapSurfacePos, lNormal );

    // Push back out to correct height
    lWorldOffset *= (lfRadius + lfHeight) / length(lWorldOffset);

    return lWorldOffset;
}

vec3
SpawnObject_TransformLocalToMapPos( INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lLocalPos ) 
{
    vec3 lMapPos = MUL( vec4(lLocalPos,1), lParams.mRegionCubeMatrix ).xyz;
    return lMapPos;
}
vec3
SpawnObject_TransformMapToLocalPos(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lMapPos )
{
    vec3 lLocalPosition = MUL( vec4(lMapPos,1),  lParams.mRegionInvCubeMatrix ).xyz;
    return lLocalPosition;
}
vec3 
SpawnObject_Spherify(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lLocalPosition )
{
    // Transform into unit cube space
    vec3 lMapPosition = SpawnObject_TransformLocalToMapPos(lParams, lLocalPosition );
    return SpawnObject_MapPositionToWorldOffset(lParams,  lMapPosition, lParams.mRegionNormal );        
}

vec3 
SpawnObject_UnSpherify(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lWorldOffset )
{
    vec3 lNormal;
    vec3 lMapPosition   = SpawnObject_WorldOffsetToMapPosition(lParams,  lWorldOffset, lNormal );
    vec3 lLocalPosition = SpawnObject_TransformMapToLocalPos(lParams,  lMapPosition );

    return lLocalPosition;
}


bool
SpawnObject_PointIsInsideRegionBorder(INPARAM(sSpawnObjectGlobalParams) lParams, vec3 lWorldOffset)
{
    vec3 lLocalPosition = SpawnObject_UnSpherify( lParams, lWorldOffset ) / lParams.mRegionVoxelScale;

    if( lLocalPosition.x <= 0.0f )
        return false;
    if( lLocalPosition.z <= 0.0f )
        return false;

    if( lLocalPosition.x >= (float)( lParams.mRegionVoxelsX - lParams.mRegionBorder ) )
        return false;                                                                                                      
    if( lLocalPosition.z >= (float)( lParams.mRegionVoxelsZ - lParams.mRegionBorder ) )
        return false;

    return true;
}


//-----------------------------------------------------------------------------
//Check we are attempting to spawn onto the correct tile type
//-----------------------------------------------------------------------------
bool
TrySpawnObject_CheckTileType(
    INOUTPARAM(sSpawnObjectResult)              lOutResult,
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams)
{
    if( lPerTypeParams.meTileType != ETileType_Liquid )
    {
        // Only spawn object on ground of correct tile type and height
        if( lPerTypeParams.meTileType != lPerInstanceParams.meMaterial &&  lPerTypeParams.meTileType != lPerInstanceParams.meSecondaryMaterial )
        {
            return false;
        }

        lOutResult.mfBlend      = 1.0f;

        if( lPerInstanceParams.meMaterial != lPerInstanceParams.meSecondaryMaterial )
        {
            if( lPerTypeParams.meGroundColourIndex != EGroundColourIndex_Auto )
            {
                return false;
            }

            if( lPerTypeParams.meTileType == lPerInstanceParams.meMaterial )
            {
                if( lPerInstanceParams.mfRatio > 0.5f )
                {
                    return false;
                }

                lOutResult.mfBlend = 1.0f - max( lPerInstanceParams.mfRatio, 0.0f ) / 0.5f;
            }

            if( lPerTypeParams.meTileType == lPerInstanceParams.meSecondaryMaterial )
            {
                if( lPerInstanceParams.mfRatio < 0.5f )
                {
                    return false;
                }

                lOutResult.mfBlend = 1.0f - max( ( 1.0f - lPerInstanceParams.mfRatio ), 0.0f ) / 0.5f;
            }
        }
    }
    else
    {
        lOutResult.mfBlend      = 1.0f;

        // Spawn object on water surface, check that the max-height tile material is under the water
        if( lPerInstanceParams.meMaterial != EVoxel_Underwater )
        {
            return false;
        }
    }

    return true;
}


//-----------------------------------------------------------------------------
//Check the spawn point is the correct height above the ground
//-----------------------------------------------------------------------------
bool
TrySpawnObject_CheckHeight(
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams)
{
    if( lPerTypeParams.meTileType != ETileType_Liquid )
    {
        float lfHeightAboveGround = SpawnObject_GetHeightAboveGround( lGlobalParams, lPerInstanceParams.mPosition );

        if( (bool)lPerTypeParams.mbRelativeToSeaLevel )
        {
            if( ( lPerTypeParams.mfMaxHeight < 128.0f && lPerTypeParams.mfMaxHeight < lfHeightAboveGround - lGlobalParams.mfPlanetSeaLevel ) ||
                lPerTypeParams.mfMinHeight > lfHeightAboveGround - lGlobalParams.mfPlanetSeaLevel )
            {
                return false;
            }
        }
        else
        {
            if( ( lPerTypeParams.mfMaxHeight < 128.0f && lPerTypeParams.mfMaxHeight < lfHeightAboveGround ) ||
                lPerTypeParams.mfMinHeight > lfHeightAboveGround )
            {
                return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//Makes sure we aren't spawning outside of the border of the region, as these
//spawns will be handled by the neighboring region
//-----------------------------------------------------------------------------
bool
TrySpawnObject_CheckPosition(
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams)
{
    if( !SpawnObject_PointIsInsideRegionBorder(lGlobalParams, lPerInstanceParams.mPosition - lGlobalParams.mPlanetPosition ) )
    {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
//Checks the object can be spawned at this location based off its maximum
//density parameters, and (assuming it passes the basic test) density settings
//related to the slope the object is on. 
//-----------------------------------------------------------------------------
bool
TrySpawnObject_CheckDensity(
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams)
{	
    // Density forces spacing between objects of same type
    // TK_PROFILER_EMBED( Foliage_Density, ePPPage_Regions, 100.0f );

    // early out if slope doesn't matter
    if( lPerInstanceParams.mfDensity > lPerTypeParams.mfMaxDensity )
    {
        return false;
    }

    if( lPerInstanceParams.mfDensity <= min( lPerTypeParams.mfSlopeDensity, lPerTypeParams.mfFlatDensity ) )
    {
        return true;
    }

    vec3 lUp = normalize(lPerInstanceParams.mPosition - lGlobalParams.mPlanetPosition);

    float lfCosAngle    = saturate(dot(lPerInstanceParams.mNormal, lUp));
    float lfAngleDegs   = degrees(acos(lfCosAngle));

    float lfAngle        = clamp( lfAngleDegs * lPerTypeParams.mfSlopeMultiplier, 0.0f, 90.0f ) / 90.0f;
    float lfPatchDensity = lerp( lPerTypeParams.mfFlatDensity, lPerTypeParams.mfSlopeDensity, smoothstep5(0.0, 1.0, lfAngle));

    // Adjust for region scale so that density values are the same regardless
    if (lPerInstanceParams.mfDensity > lfPatchDensity)
    {
        return false;
    }

    return true;
}


//-----------------------------------------------------------------------------
///
///     SpawnObject_ScaleNoise
///
///     @brief      SpawnObject_ScaleNoise
///
///     @param      float lfLayerRegionRatio
///     @param      float lfLayerHeight
///     @param      float lfNoise
///     @return     float
///
//-----------------------------------------------------------------------------
float
SpawnObject_ScaleNoise(
float            lfLayerRegionRatio,
float            lfLayerHeight,
float            lfNoise )
{
    if( abs( lfNoise ) > 0.0f )
    {
        float lfAmount = lfLayerRegionRatio;

        float lfRatioInv   = 1.0f - lfAmount;
        float lfRatioRecip = 1.0f / lfAmount;

        lfNoise = ( max( lfNoise - lfRatioInv, 0.0f ) );
        lfNoise = lfNoise * lfRatioRecip;

        lfNoise *= lfLayerHeight;
    }
    return lfNoise;
}

STATIC_CONST int X_NOISE_GEN    = 1619;
STATIC_CONST int Y_NOISE_GEN    = 31337;
STATIC_CONST int Z_NOISE_GEN    = 6971;
STATIC_CONST int SEED_NOISE_GEN = 1013;

float
ValueNoise3D(
    int x,
    int y,
    int z,
    int seed )
{
    // All constants are primes and must remain prime in order for this noise
    // function to work correctly.
    uint64 n = (
        X_NOISE_GEN    * x
        + Y_NOISE_GEN    * y
        + Z_NOISE_GEN    * z
        + SEED_NOISE_GEN * seed )
        & 0x7fffffff;
    n = ( n >> 13 ) ^ n;
    n = ( n * ( n * n * 60493 + 19990303 ) + 1376312589 ) & 0x7fffffff;

    return 1.0f - ((float)n / 1073741824.0f);
}


//-----------------------------------------------------------------------------
///
///     TrySpawnObject_CheckNoise
///
///     @brief      TrySpawnObject_CheckNoise
///
///     @param      INOUTPARAM
///     @param      sSpawnObjectResult
///     @param      lOutResult
///     @param      INPARAM
///     @param      sSpawnObjectGlobalParams
///     @param      lGlobalParams
///     @param      INPARAM
///     @param      sSpawnObjectPerTypeParams
///     @param      lPerTypeParams
///     @param      INPARAM
///     @param      sSpawnObjectPerInstanceParams
///     @param      lPerInstanceParams
///     @return     int
///
//-----------------------------------------------------------------------------
bool
TrySpawnObject_CheckNoise(
    INOUTPARAM( sSpawnObjectResult )              lOutResult,
    INPARAM( sSpawnObjectGlobalParams )           lGlobalParams,
    INPARAM( sSpawnObjectPerTypeParams )          lPerTypeParams,
    INPARAM( sSpawnObjectPerInstanceParams )      lPerInstanceParams )
{
    float lfNoise        = 0.0f;
#if !defined ( D_PLATFORM_NX64 )
    float lfScale        = 0.0f;
#endif

    if( lPerTypeParams.meNoiseCoverageType == ECoverageType_SmoothPatch )
    {
        TKASSERT( lPerTypeParams.mbNoiseActive );
        if( (bool)lPerTypeParams.mbNoiseActive )
        {
            lfNoise = GenerateNoise2D(
                lPerInstanceParams.mPosition - lGlobalParams.mPlanetPosition,
                lPerTypeParams.mfNoisePatchSize,    //width
                lPerTypeParams.mfNoiseRegionScale,  //region scale
                lPerTypeParams.mfNoiseCoverage,     //region ratio
                1.0f,
                (int)lPerTypeParams.muNoiseSeed,
                TRUE ); // IsLegacy



            lfNoise = SpawnObject_ScaleNoise( sqrt( lPerTypeParams.mfNoiseCoverage ), 1.0f, lfNoise );

            lfNoise = clamp( lfNoise, 0.0f, 1.0f );

            if( abs( lfNoise ) <  1.525878906e-5f )
            {
                return false;
            }

            lfNoise     = smoothstep5( 0.0f, 0.5f, lfNoise ) * 0.5f;
            lfNoise    += 0.5f;
        }
    }
    else if( lPerTypeParams.meNoiseCoverageType == ECoverageType_GridPatch )
    {
        // Work out the grid square you are in
        vec3 lNormal;
        vec3 lMapPosition = ProjectSphereToCube( normalize( lPerInstanceParams.mPosition - lGlobalParams.mPlanetPosition ) * lGlobalParams.mfRegionMapRadius, lNormal, lGlobalParams.mfRegionMapRadius );

        float lfX, lfZ;
        if( abs( lNormal.x ) >= 1.525878906e-5f )
        {
            lfX = lMapPosition.y;
            lfZ = lMapPosition.z;
        }
        else if( abs( lNormal.y ) >= 1.525878906e-5f )
        {
            lfX = lMapPosition.x;
            lfZ = lMapPosition.z;
        }
        else
        {
            lfX = lMapPosition.x;
            lfZ = lMapPosition.y;
        }

        float lfGridChance  = lPerTypeParams.mfNoiseRegionScale;
#if !defined ( D_PLATFORM_NX64 )
        float lfGridSizeMax = lPerTypeParams.mfNoisePatchSize;
#endif
        lfX                *= 1.0f / lPerTypeParams.mfNoisePatchSize;
        lfZ                *= 1.0f / lPerTypeParams.mfNoisePatchSize;

        vec3  lCentrePos = vec3( floor( lfX )  + 0.5f, floor( lfZ ) + 0.5f, 0.0f );
        vec3  lSamplePos = vec3( lfX, lfZ, 0.0f );
        float lfDistance = length( lSamplePos - lCentrePos );

        int liFloorX = (int)floor( lfX );
        int liFloorZ = (int)floor( lfZ );

        //static int kiGrid = 1;
        //if( kiGrid == 0 )
        //{
        //    if( ( liFloorX % 2 == 0 ) != ( liFloorZ % 2 == 0 ) )
        //    {
        //        lfNoise = 1.0f;
        //    }
        //    else
        //    {
        //        lfNoise = 0.0f;
        //    }
        //}
        //else if( kiGrid== 1 )
        {         
            lfNoise =  ValueNoise3D( liFloorX, liFloorZ, 0, (int)lPerTypeParams.muNoiseSeed );
            lfNoise = lfNoise * 0.5f + 0.5f;
        }

        if( lfDistance > lPerTypeParams.mfNoiseCoverage / lPerTypeParams.mfNoisePatchSize )
        {
            return false;
        }

        if( lfNoise > lfGridChance )
        {
            return false;
        }
        else
        {
            lOutResult.mfNoise = lfNoise;
            return true;
        }
    }
    else
    {
        lfNoise = 1.0f;
    }

    if( abs( lfNoise ) <  1.525878906e-5f )
    {
        return false;
    }

    lOutResult.mfNoise = lfNoise;;
    
    return true;
}


//-----------------------------------------------------------------------------
//Main spawn function that does a series of different tests to validate a 
//potential spawn, and returns either the stage that failed or eSpawnStage_Complete
//on success
//-----------------------------------------------------------------------------
int 
TrySpawnObject(
    INOUTPARAM(sSpawnObjectResult)              lOutResult,
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams,
    uint                                        luTestsToPerformMask)
{
    if ((luTestsToPerformMask & (1<<eSpawnStage_CheckTileType))!=0)
        if (!TrySpawnObject_CheckTileType(lOutResult,lGlobalParams,lPerTypeParams,lPerInstanceParams))
            return eSpawnStage_CheckTileType;

    if ((luTestsToPerformMask & (1<<eSpawnStage_CheckHeight))!=0)
        if (!TrySpawnObject_CheckHeight(lGlobalParams,lPerTypeParams,lPerInstanceParams))
            return eSpawnStage_CheckHeight;

    if ((luTestsToPerformMask & (1<<eSpawnStage_CheckPosition))!=0)
        if (!TrySpawnObject_CheckPosition(lGlobalParams,lPerTypeParams,lPerInstanceParams))
            return eSpawnStage_CheckPosition;

    if ((luTestsToPerformMask & (1<<eSpawnStage_CheckDensity))!=0)
        if (!TrySpawnObject_CheckDensity(lGlobalParams,lPerTypeParams,lPerInstanceParams))
            return eSpawnStage_CheckDensity;

    if ((luTestsToPerformMask & (1<<eSpawnStage_CheckNoise))!=0)
        if (!TrySpawnObject_CheckNoise(lOutResult,lGlobalParams,lPerTypeParams,lPerInstanceParams))
            return eSpawnStage_CheckNoise;       

    return eSpawnStage_Complete;
}

#include "Custom/SharedEnd.inl"
#endif
