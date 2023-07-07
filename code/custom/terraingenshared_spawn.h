//-----------------------------------------------------------------------------
//Terrain gen core code shared between CPU and GPU
//This can be included from headers in c++ code
//-----------------------------------------------------------------------------

#ifndef _TGENSHARED_SPAWN_H_
#define _TGENSHARED_SPAWN_H_
#include "Custom/TerrainGenShared_Core.h"
#include "Custom/TerrainGenShared_RegionDecorator.h"
#include "Custom/SharedBegin.inl"


#ifdef D_PLATFORM_GLSL

	#define  eTileType int
	#define  eGroundColourIndex int
	#define  eCoverageType int
	#define  eVoxelType int
	#define  ESpawnObjectStage int


    const int ETileType_Air = 0;
    const int ETileType_Base = 1;
    const int ETileType_Rock = 2; 
    const int ETileType_Mountain = 3;
    const int ETileType_Underwater= 4;
    const int ETileType_Cave = 5;
    const int ETileType_Dirt = 6;
    const int ETileType_Liquid = 7;
    const int ETileType_Substance = 8;
    const int ETileType_NumTypes = 9;

    const int EGroundColourIndex_Auto = 0;
    const int EGroundColourIndex_Main = 1;
    const int EGroundColourIndex_Patch = 2;
    const int EGroundColourIndex_NumTypes = 3;

    const int ECoverageType_Total = 0;
    const int ECoverageType_SmoothPatch = 1;
    const int ECoverageType_GridPatch = 2;
    const int ECoverageType_NumTypes = 3;

    const int EVoxel_Air = 0;
    const int EVoxel_Base = 1;
    const int EVoxel_Rock = 2;
    const int EVoxel_Mountain = 3;
    const int EVoxel_Underwater = 4;
    const int EVoxel_Cave = 5;
    const int EVoxel_Dirt = 6;
    const int EVoxel_Liquid = 7;
    const int EVoxel_Substance_Res1 = 8;
    const int EVoxel_Substance_Res2 = 9;
    const int EVoxel_Substance_Res3 = 10;
    const int EVoxel_Num = 11;

    const int eSpawnStage_Complete = 0;
    const int eSpawnStage_CheckTileType = 1;
    const int eSpawnStage_CheckHeight = 2;
    const int eSpawnStage_CheckPosition = 3;
    const int eSpawnStage_CheckDensity = 4;
    const int eSpawnStage_HasSpawnDensity = 5;
    const int eSpawnStage_CheckNoise = 6;
    const int eSpawnStage_CheckLargeObjects = 7;
    const int eSpawnStage_GetPosition = 8;
    const int eSpawnStage_GetMatrix = 9;
    const int eSpawnStage_InteractionDist = 10;
    const int eSpawnStage_InteractionDot = 11;
    const int eSpawnStage_CheckBaseBuilding = 12;
    const int eSpawnStage_Num = 13;


#else
	
enum eTileType
{
    ETileType_Air,
    ETileType_Base,
    ETileType_Rock,
    ETileType_Mountain,
    ETileType_Underwater,
    ETileType_Cave,
    ETileType_Dirt,
    ETileType_Liquid,
    ETileType_Substance,
    ETileType_NumTypes = 9
};
enum eGroundColourIndex
{
    EGroundColourIndex_Auto,
    EGroundColourIndex_Main,
    EGroundColourIndex_Patch,
    EGroundColourIndex_NumTypes = 3
};
enum eCoverageType
{
    ECoverageType_Total,
    ECoverageType_SmoothPatch,
    ECoverageType_GridPatch,

    ECoverageType_NumTypes = 3
};
enum eVoxelType
{
    EVoxel_Air,
    EVoxel_Base,
    EVoxel_Rock,
    EVoxel_Mountain,
    EVoxel_Underwater,
    EVoxel_Cave,
    EVoxel_Dirt,
    EVoxel_Liquid,
    EVoxel_Substance_Res1,
    EVoxel_Substance_Res2,
    EVoxel_Substance_Res3,
    EVoxel_Num
};
enum ESpawnObjectStage
{
    eSpawnStage_Complete,
    eSpawnStage_CheckTileType,
    eSpawnStage_CheckHeight,
    eSpawnStage_CheckPosition,
    eSpawnStage_CheckDensity,
    eSpawnStage_HasSpawnDensity,
    eSpawnStage_CheckNoise,
    eSpawnStage_CheckLargeObjects,
    eSpawnStage_GetPosition,
    eSpawnStage_GetMatrix,
    eSpawnStage_InteractionDist,
    eSpawnStage_InteractionDot,
    eSpawnStage_CheckBaseBuilding,
    eSpawnStage_Num
};

#endif

struct sSpawnObjectGlobalParams
{
    mat4x4              mRegionCubeMatrix;
    mat4x4              mRegionInvCubeMatrix;
    vec3                mRegionNormal;
    vec3                mRegionVoxelScale;
    int                 mRegionVoxelsX;
    int                 mRegionVoxelsY;
    int                 mRegionVoxelsZ;
    int                 mRegionBorder;

    vec3                mRegionMapPosition;
    float               mfRegionMapRadius;

    float               mfEnvMinPlacementBlendValuePatch;
    float               mfEnvMaxPlacementBlendValuePatch;
    float               mfEnvMinPlacementObjectScale;

    float               mfPlanetSeaLevel;
    vec3                mPlanetPosition;
};
struct sSpawnObjectPerTypeParams
{
    int                 meTileType;             //eTileType
    int                 meGroundColourIndex;    //eGroundColourIndex
    int                 mbRelativeToSeaLevel;
    float               mfMinHeight;
    float               mfMaxHeight;
    float               mfMaxDensity;
    float               mfSlopeDensity;
    float               mfFlatDensity;
    float               mfSlopeMultiplier;
    int                 mbNoiseActive;
    float               mfNoiseRegionScale;
    float               mfNoisePatchSize;
    float               mfNoiseCoverage;
    int                 meNoiseCoverageType;    //eCoverageType
    uint                muNoiseSeed;

};
struct sSpawnObjectPerInstanceParams
{
    vec3                mPosition;
    vec3                mNormal;
    float               mfSlopeValue;
    float               mfRatio;
    int                 meMaterial;             //eVoxelType
    int                 meSecondaryMaterial;    //eVoxelType
    uint                muRandSeed;
    float               mfDensity;
};
struct sSpawnObjectResult
{
    float               mfBlend;
    float               mfNoise;
};

//-----------------------------------------------------------------------------
//Structures/flags for filtering down the types list
//-----------------------------------------------------------------------------
#define K_REGIONFILTER_CURRENT 0x1
#define K_REGIONFILTER_CHECKTILETYPE    (0x1 << (uint)eSpawnStage_CheckTileType)
#define K_REGIONFILTER_CHECKHEIGHT      (0x1 << (uint)eSpawnStage_CheckHeight)
#define K_REGIONFILTER_CHECKPOSITION    (0x1 << (uint)eSpawnStage_CheckPosition)
#define K_REGIONFILTER_CHECKDENSITY     (0x1 << (uint)eSpawnStage_CheckDensity)
#define K_REGIONFILTER_CHECKNOISE       (0x1 << (uint)eSpawnStage_CheckNoise)

struct cTkFilterObjectTypesComputeUniforms
{
    sObjectTypePriorityMap              mPriorityMap MTL_ID(0);
    sSpawnObjectGlobalParams            mGlobalSpawnParams;
#ifdef D_PLATFORM_ORBIS			
    ROBUFF(sAddObjectPosIndex)          maPosEntriesBuffer;
	ROBUFF(sRegionHeightResult)         maPositions;
	ROBUFF(uint)                        maPositionSeeds;
    ROBUFF(sSpawnObjectPerTypeParams)   maPerTypeSpawnParams;
    RWBUFF(uint64)                      mauFilterBits;
    RWBUFF(int)                         maiSpawnResultStages;
    RWBUFF(sSpawnObjectResult)          maSpawnResults;
#endif	
    uint                                muFilterFlags;
    uint                                muPosEntriesCountGDSAddress;
};

bool
TrySpawnObject_CheckTileType(
    INOUTPARAM(sSpawnObjectResult)              lOutResult,
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams);

bool
TrySpawnObject_CheckHeight(
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams);

bool
TrySpawnObject_CheckPosition(
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams);

bool
TrySpawnObject_CheckDensity(
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams);

bool
TrySpawnObject_CheckNoise(
    INOUTPARAM(sSpawnObjectResult)              lOutResult,
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams);

int 
TrySpawnObject(
    INOUTPARAM(sSpawnObjectResult)              lOutResult,
    INPARAM(sSpawnObjectGlobalParams)           lGlobalParams,
    INPARAM(sSpawnObjectPerTypeParams)          lPerTypeParams,
    INPARAM(sSpawnObjectPerInstanceParams)      lPerInstanceParams,
    uint                                        luTestsToPerformMask=0xffffffff);

#include "Custom/SharedEnd.inl"
#endif


