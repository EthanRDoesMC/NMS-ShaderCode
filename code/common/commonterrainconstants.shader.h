////////////////////////////////////////////////////////////////////////////////
///
///     @file       CommonTerrainConstants
///     @author     strgiu
///     @date       
///
///     @brief      CommonTerrainConstants
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 
#ifndef D_COMMONTERRAINCONSTANTS
#define D_COMMONTERRAINCONSTANTS


//-----------------------------------------------------------------------------
//      Global Data

STATIC_CONST float kafTextureScales[9] =
{
      1.0 / 4.0,  1.0 / 24.0, 1.0 / 32.0, 1.0 / 64.0,
    1.0 / 128.0, 1.0 / 256.0, 1.0 / 2048.0, 1.0 / 8192.0, 1.0 / 16384.0
};

STATIC_CONST float kafTextureDistances[9] =
{
      0.0,   64.0,  224.0,  324.0,   576.0,
    900.0, 4000.0, 8000.0, 16000.0
};

//STATIC_CONST float kafTextureDistances[ 9 ] =
//{
//      0.0,   43.7,  252.4,  704.6,
//      1459.6, 2568.0, 4074.2, 6019.1, 8440.1
//};

STATIC_CONST float kfCrossing   = 0.75f;
STATIC_CONST float kfPower      = 40.0f;

#endif
