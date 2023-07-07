////////////////////////////////////////////////////////////////////////////////
///
///     @file       SkinningVertex.shader.h
///     @author     Sean
///     @date       30 Jul 2009
///
///     @brief      SkinningVertex.shader
///
///     Copyright (c) 2009 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files


//-----------------------------------------------------------------------------
//      Constant Data


//-----------------------------------------------------------------------------
//      Global Data

#if defined ( D_PLATFORM_SWITCH )
	#define JOINT_TYPE    uvec4
#elif defined D_PLATFORM_GLSL
	#define JOINT_TYPE    vec4
#elif defined(D_PLATFORM_DX12)
	#define JOINT_TYPE    uint4
#elif defined D_PLATFORM_ORBIS
    #define JOINT_TYPE    int4
#elif defined(D_PLATFORM_METAL)
    #define JOINT_TYPE    uchar4
    #define USE_SKINNING_MACROS
#endif

//-----------------------------------------------------------------------------
//      Typedefs and Classes 


//-----------------------------------------------------------------------------
//    Functions 

//-----------------------------------------------------------------------------
///
///     GetJointMat
///
///     @brief      GetJointMat
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
mat4 
GetJointMat( 
    in CommonPerMeshUniforms   lMeshUniforms,
    in int                     liJointIndex )
{

    // Note: This matrix is transposed so vec/mat multiplications need to be done in reversed order
    mat4 lMat4;
    MAT4_SET_COLUMN( lMat4, 0, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 3 ) );
    MAT4_SET_COLUMN( lMat4, 1, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 3 + 1 ) );
    MAT4_SET_COLUMN( lMat4, 2, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 3 + 2 ) );
    MAT4_SET_COLUMN( lMat4, 3, vec4( 0.0, 0.0, 0.0, 1.0 ) );

    return lMat4;
}
#else
#define GetJointMat(lMeshUniforms, liJointIndex) \
mat4( \
    (lMeshUniforms).gaSkinMatrixRowsVec4[(liJointIndex)*3 + 0], \
    (lMeshUniforms).gaSkinMatrixRowsVec4[(liJointIndex)*3 + 1], \
    (lMeshUniforms).gaSkinMatrixRowsVec4[(liJointIndex)*3 + 2], \
    vec4( 0.0, 0.0, 0.0, 1.0 ) \
)
#endif

#if !defined( USE_SKINNING_MACROS )
#else
#define GetPrevJointMat(lMeshUniforms, liJointIndex) \
mat4( \
    (lMeshUniforms).gaPrevSkinMatrixRowsVec4[(liJointIndex)*3 + 0], \
    (lMeshUniforms).gaPrevSkinMatrixRowsVec4[(liJointIndex)*3 + 1], \
    (lMeshUniforms).gaPrevSkinMatrixRowsVec4[(liJointIndex)*3 + 2], \
    vec4( 0.0, 0.0, 0.0, 1.0 ) \
)
#endif

//-----------------------------------------------------------------------------
///
///     CalcSkinningMat
///
///     @brief      CalcSkinningMat
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
mat4 
CalcSkinningMat(
    in CommonPerMeshUniforms  lMeshUniforms,
    in vec4                   lWeightsVec4,
    in JOINT_TYPE             lJointsVec4 )
{
    mat4 laSkinningMatrix;

    laSkinningMatrix      = lWeightsVec4.x * GetJointMat( lMeshUniforms, int( lJointsVec4.x ) );
    if (lWeightsVec4.y > 0.001)    
        laSkinningMatrix += lWeightsVec4.y * GetJointMat( lMeshUniforms, int( lJointsVec4.y ) );
    if (lWeightsVec4.z > 0.001)
        laSkinningMatrix += lWeightsVec4.z * GetJointMat( lMeshUniforms, int( lJointsVec4.z ) );
    if (lWeightsVec4.w > 0.001)
        laSkinningMatrix += lWeightsVec4.w * GetJointMat( lMeshUniforms, abs( int( lJointsVec4.w ) ) );

    return  laSkinningMatrix;
}
#else
#define CalcSkinningMat(lMeshUniforms, lWeightsVec4, lJointsVec4) \
mat4( \
    lWeightsVec4.x * GetJointMat( lMeshUniforms, int( lJointsVec4.x ) )      + \
    lWeightsVec4.y * GetJointMat( lMeshUniforms, int( lJointsVec4.y ) )      + \
    lWeightsVec4.z * GetJointMat( lMeshUniforms, int( lJointsVec4.z ) )      + \
    lWeightsVec4.w * GetJointMat( lMeshUniforms, abs( int( lJointsVec4.w ) ) ) \
)
#endif


#ifdef D_OUTPUT_MOTION_VECTORS
//-----------------------------------------------------------------------------
///
///     GetPrevJointMat
///
///     @brief      GetPrevJointMat
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
mat4 
GetPrevJointMat( 
    in CommonPerMeshUniforms   lMeshUniforms,
    in int                     liJointIndex )
{

    // Note: This matrix is transposed so vec/mat multiplications need to be done in reversed order
    mat4 lMat4;
    MAT4_SET_COLUMN( lMat4, 0, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaPrevSkinMatrixRowsVec4, liJointIndex * 3 ) );
    MAT4_SET_COLUMN( lMat4, 1, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaPrevSkinMatrixRowsVec4, liJointIndex * 3 + 1 ) );
    MAT4_SET_COLUMN( lMat4, 2, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaPrevSkinMatrixRowsVec4, liJointIndex * 3 + 2 ) );
    MAT4_SET_COLUMN( lMat4, 3, vec4( 0.0, 0.0, 0.0, 1.0 ) );

    return lMat4;
}
#else
#define GetPrevJointMat(lMeshUniforms, liJointIndex) \
mat4( \
    (lMeshUniforms).gaPrevSkinMatrixRowsVec4[(liJointIndex)*3 + 0], \
    (lMeshUniforms).gaPrevSkinMatrixRowsVec4[(liJointIndex)*3 + 1], \
    (lMeshUniforms).gaPrevSkinMatrixRowsVec4[(liJointIndex)*3 + 2], \
    vec4( 0.0, 0.0, 0.0, 1.0 ) \
)
#endif

//-----------------------------------------------------------------------------
///
///     CalcPrevSkinningMat
///
///     @brief      CalcPrevSkinningMat
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
mat4 
CalcPrevSkinningMat(
    in CommonPerMeshUniforms  lMeshUniforms,
    in vec4                   lWeightsVec4,
    in JOINT_TYPE             lJointsVec4 )
{
    mat4 laSkinningMatrix;

    laSkinningMatrix      = lWeightsVec4.x * GetPrevJointMat( lMeshUniforms, int( lJointsVec4.x ) );
    if (lWeightsVec4.y > 0.001)    
        laSkinningMatrix += lWeightsVec4.y * GetPrevJointMat( lMeshUniforms, int( lJointsVec4.y ) );
    if (lWeightsVec4.z > 0.001)
        laSkinningMatrix += lWeightsVec4.z * GetPrevJointMat( lMeshUniforms, int( lJointsVec4.z ) );
    if (lWeightsVec4.w > 0.001)
        laSkinningMatrix += lWeightsVec4.w * GetPrevJointMat( lMeshUniforms, abs( int( lJointsVec4.w ) ) );

    return  laSkinningMatrix;
}
#else
#define CalcPrevSkinningMat(lMeshUniforms, lWeightsVec4, lJointsVec4) \
mat4( \
    lWeightsVec4.x * GetPrevJointMat( lMeshUniforms, int( lJointsVec4.x ) )      + \
    lWeightsVec4.y * GetPrevJointMat( lMeshUniforms, int( lJointsVec4.y ) )      + \
    lWeightsVec4.z * GetPrevJointMat( lMeshUniforms, int( lJointsVec4.z ) )      + \
    lWeightsVec4.w * GetPrevJointMat( lMeshUniforms, abs( int( lJointsVec4.w ) ) ) \
)
#endif
#endif // D_OUTPUT_MOTION_VECTORS

//-----------------------------------------------------------------------------
///
///     GetSkinningMatVec
///
///     @brief      GetSkinningMatVec
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
mat3 
GetSkinningMatVec( 
    in mat4 lSkinningMat4 )
{
    //return mat3( lSkinningMat4[0].xyz, lSkinningMat4[1].xyz, lSkinningMat4[2].xyz );

    mat3 lMat3;

    MAT3_SET_COLUMN( lMat3, 0, MAT4_GET_COLUMN( lSkinningMat4, 0 ) );
    MAT3_SET_COLUMN( lMat3, 1, MAT4_GET_COLUMN( lSkinningMat4, 1 ) );
    MAT3_SET_COLUMN( lMat3, 2, MAT4_GET_COLUMN( lSkinningMat4, 2 ) );

    return lMat3;
}


//-----------------------------------------------------------------------------
///
///     GetSkinPosition
///
///     @brief      GetSkinPosition
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
vec4 
GetSkinPosition( 
    in CommonPerMeshUniforms  lMeshUniforms,
    in vec4                   lPositionVec4,
    in vec4                   lWeightsVec4,
    in JOINT_TYPE             lJointsVec4 )
{
    vec4 lSkinnedPosVec4 = MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.x ) )) * lWeightsVec4.x;
    if (lWeightsVec4.y > 0.001)
        lSkinnedPosVec4 += MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.y ) )) * lWeightsVec4.y;
    if (lWeightsVec4.z > 0.001)
        lSkinnedPosVec4 += MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.z ) )) * lWeightsVec4.z;
    if (lWeightsVec4.w > 0.001)
        lSkinnedPosVec4 += MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.w ) )) * lWeightsVec4.w;

    // normalization step
    // lSkinnedPosVec4.w = 1.0;
    lSkinnedPosVec4 /= max( lSkinnedPosVec4.w, 0.000001 );

    return lSkinnedPosVec4;
}

#else

vec4 
GetSkinPosition( 
    in CommonPerMeshUniforms  lMeshUniforms,
    in vec4                   lPositionVec4,
    in vec4                   lWeightsVec4,
    in JOINT_TYPE             lJointsVec4 )
{
    vec4 lSkinnedPosVec4 = vec4(
        MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.x ) )) * lWeightsVec4.x +
        MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.y ) )) * lWeightsVec4.y +
        MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.z ) )) * lWeightsVec4.z +
        MUL(lPositionVec4, GetJointMat( lMeshUniforms, int( lJointsVec4.w ) )) * lWeightsVec4.w
    );
    return lSkinnedPosVec4 / max( lSkinnedPosVec4.w, 0.000001 );
}
#endif

//-----------------------------------------------------------------------------
///
///     GetSkinPosition
///
///     @brief      GetSkinPosition
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec4 
GetSkinPosition( 
    in vec4 lPositionVec4, 
    in mat4 lSkinningMat4 )
{
    vec4 lSkinnedPosVec4 = MUL( lPositionVec4, lSkinningMat4 );

    // normalization step
    lSkinnedPosVec4 /= max(lSkinnedPosVec4.w, 0.000001);
    
    return lSkinnedPosVec4;
}


//-----------------------------------------------------------------------------
///
///     GetSkinVector
///
///     @brief      GetSkinVector
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
vec3 
GetSkinVector( 
    in vec3 lVector3, 
    in mat3 lSkinningMat3 )
{
    return MUL( lVector3, lSkinningMat3 );
}


//-----------------------------------------------------------------------------
///
///     DecompressLdsLo
///
///     @brief      DecompressLdsLo
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
vec3
DecompressLdsLo(
    in vec4 lds )
{
    vec3 unpck;

    #ifdef D_PLATFORM_GLSL
    unpck.xy = unpackHalf2x16( floatBitsToInt( lds.x ) );
    #else
    uint bits = floatToIntBits( lds.x );
    unpck.x = f16tof32( bits | 0xffff );
    unpck.y = f16tof32( ( bits >> 16 ) | 0xffff );
    #endif

    unpck.z  = lds.y;
    return unpck;
}
#else
#define DecompressLdsLo(lds) \
    vec3( float2(as_type<half2>(floatToIntBits( (lds).x ))), (lds).y )
#endif

//-----------------------------------------------------------------------------
///
///     DecompressLdsHi
///
///     @brief      DecompressLdsHi
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
vec3
DecompressLdsHi(
    in vec4 lds )
{
    vec3 unpck;

    #ifdef D_PLATFORM_GLSL
    unpck.xy = unpackHalf2x16( floatBitsToInt( lds.z ) );
    #else
    uint bits = floatToIntBits( lds.z );
    unpck.x = f16tof32( bits | 0xffff );
    unpck.y = f16tof32( ( bits >> 16 ) | 0xffff );
    #endif

    unpck.z  = lds.w;
    return unpck;
}
#else
#define DecompressLdsHi(lds) \
    vec3( float2(as_type<half2>(floatToIntBits( (lds).z ))), (lds).w )
#endif

//-----------------------------------------------------------------------------
///
///     GetJointLdsWind
///
///     @brief      GetJointLdsWind
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
vec3 
GetJointLdsWind( 
    in CommonPerMeshUniforms   lMeshUniforms,
    in int                  liJointIndex,
    in vec3                 lPositionVec3,
    in vec3                 lEigCoeffs0,
    in vec3                 lEigCoeffs1,
    in vec4                 lShearValues )
{
    vec3 lOut = DecompressLdsLo( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 0 ) ) * lEigCoeffs0.x;
    lOut     += DecompressLdsHi( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 0 ) ) * lEigCoeffs0.y;
    lOut     += DecompressLdsLo( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 1 ) ) * lEigCoeffs0.z;

    // just check if one of the values is 0, on cpu it's set to nonzero if any of the last three eigens is active
    if( abs( lEigCoeffs1.x ) > 0 )
    {
        lOut     += DecompressLdsHi( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 1 ) ) * lEigCoeffs1.x;
        lOut     += DecompressLdsLo( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 2 ) ) * lEigCoeffs1.y;
        lOut     += DecompressLdsHi( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 2 ) ) * lEigCoeffs1.z;
    }

    if( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 3 ).w > 0 )
    {
        vec3 lBindPos = ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 3 ).xyz;

        vec2 lLocalPosVec2 = lPositionVec3.xz - lBindPos.xz;
        vec2 lShear = lShearValues.xy + lShearValues.zw * lLocalPosVec2;

        lOut.xz += lShear;
    }

    return lOut;
}
#else
vec3 GetShear( const vec3 lPositionVec3, const vec4 lShearValues, const vec3 lBindPos )
{
        vec2 lLocalPosVec2 = lPositionVec3.xz - lBindPos.xz;
        vec2 lShear = lShearValues.xy + lShearValues.zw * lLocalPosVec2;
        return vec3(lShear.x, 0, lShear.y);
}
#define GetJointLdsWind( lMeshUniforms, liJointIndex, lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) \
vec3( \
    DecompressLdsLo( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 0 ) ) * lEigCoeffs0.x + \
    DecompressLdsHi( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 0 ) ) * lEigCoeffs0.y + \
    DecompressLdsLo( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 1 ) ) * lEigCoeffs0.z + \
    step(abs( lEigCoeffs1.x ), 0) * ( \
        DecompressLdsHi( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 1 ) ) * lEigCoeffs1.x + \
        DecompressLdsLo( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 2 ) ) * lEigCoeffs1.y + \
        DecompressLdsHi( ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 2 ) ) * lEigCoeffs1.z \
    ) + \
    step(ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 3 ).w, 0) * \
        GetShear(lPositionVec3, lShearValues, ARRAY_LOOKUP_FS( lMeshUniforms, mpCommonPerMesh, gaSkinMatrixRowsVec4, liJointIndex * 4 + 3 ).xyz) \
)
#endif

//-----------------------------------------------------------------------------
///
///     GetSkinPosition
///
///     @brief      GetSkinPosition
///
///     @param      void
///     @return     Nothing.
///
//-----------------------------------------------------------------------------
#if !defined( USE_SKINNING_MACROS )
vec3 
GetLdsWindPosition( 
    in CommonPerMeshUniforms  lMeshUniforms,
    in vec3                   lPositionVec3,
    in vec4                   lWeightsVec4,
    in JOINT_TYPE             lJointsVec4,
    in vec3                   lEigCoeffs0,
    in vec3                   lEigCoeffs1,
    in vec4                   lShearValues  )
{
    vec3 lWindPosVec3 = lPositionVec3;

    lWindPosVec3 += GetJointLdsWind( lMeshUniforms, int( lJointsVec4.x ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.x;

    if( lWeightsVec4.y > 0.0001 )
    {
        lWindPosVec3 += GetJointLdsWind( lMeshUniforms, int( lJointsVec4.y ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.y;
    }

    if( lWeightsVec4.z > 0.0001 )
    {
        lWindPosVec3 += GetJointLdsWind( lMeshUniforms, int( lJointsVec4.z ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.z;
    }

    if( lWeightsVec4.w > 0.0001 )
    {
        lWindPosVec3 += GetJointLdsWind( lMeshUniforms, int( lJointsVec4.w ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.w;
    }


    return lWindPosVec3;
}
#else

#define GetLdsWindPosition( lMeshUniforms, lPositionVec3, lWeightsVec4, lJointsVec4, lEigCoeffs0, lEigCoeffs1, lShearValues ) \
( \
    lPositionVec3 + \
    GetJointLdsWind( lMeshUniforms, int( lJointsVec4.x ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.x + \
    GetJointLdsWind( lMeshUniforms, int( lJointsVec4.y ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.y + \
    GetJointLdsWind( lMeshUniforms, int( lJointsVec4.z ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.z + \
    GetJointLdsWind( lMeshUniforms, int( lJointsVec4.w ), lPositionVec3, lEigCoeffs0, lEigCoeffs1, lShearValues ) * lWeightsVec4.w   \
)
#endif
