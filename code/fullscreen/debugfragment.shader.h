////////////////////////////////////////////////////////////////////////////////
///
///     @file       DebugFragment.h
///     @author     User
///     @date       
///
///     @brief      DebugFragment
///
///     Copyright (c) 2008 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------
//      Compilation defines 


//-----------------------------------------------------------------------------
//      Include files

#ifndef D_FRAGMENT
#define D_FRAGMENT
#endif
#include "Common/Defines.shader.h"
#include "Common/Common.shader.h"
#include "Common/CommonUniforms.shader.h"

#include "Fullscreen/PostCommon.h"

#include "Common/CommonPostProcess.shader.h"





// =================================================================================================

#ifdef D_DEBUG_SHADOW


//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_COLOUR_SRT
{
   vec2 newCoords = IN(mTexCoordsVec2);

   float lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBufferMap ), newCoords ).x;
   FRAGMENT_COLOUR = vec4( lFragCol, lFragCol, lFragCol, 1.0 );
}

#endif


// =================================================================================================

#ifdef D_DEBUG_TEXTURED


//-----------------------------------------------------------------------------
//      Global Data



//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
INPUT_SCREEN_POSITION

INPUT(vec2, mTexCoordsVec2, TEXCOORD0)
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_COLOUR_SRT
{
    vec2 newCoords = IN(mTexCoordsVec2);

    vec4 lFragCol = texture2D(SAMPLER_GETMAP(lUniforms.mpCustomPerMesh, gBufferMap), newCoords);
    FRAGMENT_COLOUR = vec4(lFragCol.rgb, 1.0);
}

#endif


// =================================================================================================

#ifdef D_DEBUG_FLATCOLOUR

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
    // INPUT( vec4, mWorldPositionVec4, TEXCOORD4 )
DECLARE_INPUT_END


//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_COLOUR_SRT
{
    FRAGMENT_COLOUR = vec4( 1.0, 1.0, 0.0, 1.0 );
}

#endif



// =================================================================================================
//
// D_DEBUG_HEX_SPLIT
//
// =================================================================================================

#ifdef D_DEBUG_HEX_SPLIT

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions

float blendScreen( float base, float blend )
{
    return 1.0 - ( ( 1.0 - base )*( 1.0 - blend ) );
}

vec3 blendScreen( vec3 base, vec3 blend )
{
    //return vec3( blendScreen( base.r, blend.r ), blendScreen( base.g, blend.g ), blendScreen( base.b, blend.b ) );
    return 1.0 - ( ( 1.0 - base )*( 1.0 - blend ) );
}

FRAGMENT_MAIN_COLOUR_SRT
{
    //Debug

    vec3 lFragCol = vec3( 1.0, 1.0, 1.0 );
    vec2 newCoords;

    newCoords = IN(mTexCoordsVec2);
    
    // Left Quadrant
    if ( newCoords.x < 0.25 )
    {
        newCoords.x *= 4.0;

        // Top Quadrant
        if ( newCoords.y > 0.75 )
        {
            newCoords.y = (newCoords.y - 0.75) * 4.0;

            //lFragCol = GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer11Map, newCoords ).xyz );
            lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer11Map ), newCoords ).xyz;
        }
        // Mid Top Quadrant
        else
        if ( newCoords.y > 0.5 )
        {
            newCoords.y = (newCoords.y - 0.5) * 4.0;
            
            lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer12Map ), newCoords ).xyz );
            lFragCol *= texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer12Map ), newCoords ).a;
            
//             float lfGlow = texture2D( lUniforms.mpCustomPerMesh.gBuffer9Map, newCoords ).a;
// 
//             if ( lfGlow > 0.68 )
//             {
//                 lFragCol = vec3(1.0,0.0,0.0);
//             }
//             else
//             if ( lfGlow > 0.35 )
//             {
//                 lFragCol = vec3(0.0,1.0,0.0);
//             }
//             else
//             if ( lfGlow > 0.0 )
//             {
//                 lFragCol = vec3(0.0,0.0,1.0);
//             }
        }
        // Mid Bottom Quadrant
        else
        if ( newCoords.y > 0.25 )
        {
            newCoords.y = (newCoords.y - 0.25) * 4.0;
            
            lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer13Map ), newCoords ).xyz );
            //lFragCol = ( texture2D( lUniforms.mpCustomPerMesh.gBuffer13Map, newCoords ).xyz );
            
//             float lfGlow = texture2D( lUniforms.mpCustomPerMesh.gBuffer10Map, newCoords ).a;
// 
//             if ( lfGlow > 0.68 )
//             {
//                 lFragCol = vec3(1.0,0.0,0.0);
//             }
//             else
//             if ( lfGlow > 0.35 )
//             {
//                 lFragCol = vec3(0.0,1.0,0.0);
//             }
//             else
//             if ( lfGlow > 0.0 )
//             {
//                 lFragCol = vec3(0.0,0.0,1.0);
//             }
        }
        // Bottom Quadrant
        else
        {
            newCoords.y *= 4.0;

            //lFragCol = GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer14Map, newCoords ).xyz );
            lFragCol = ( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer14Map ), newCoords ).xyz );
        }
    }
    // Bottom Quadrant
    else
    if ( newCoords.y < 0.25 )
    {
        newCoords.y *= 4.0;

        // Left Mid Quadrant
        if ( newCoords.x < 0.5 )
        {
            newCoords.x = ( newCoords.x - 0.25 ) * 4.0;

            // Bottom Left
            if ( newCoords.x < 0.5 && newCoords.y < 0.5 )
            {
                newCoords = newCoords * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer7Map ), newCoords ).xyz );
            }
            else
            // Bottom Right
            if ( newCoords.x > 0.5 && newCoords.y < 0.5 )
            {
                newCoords.x = (newCoords.x - 0.5) * 2.0;
                newCoords.y = newCoords.y * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer8Map ), newCoords ).xyz );
            }
            else
            // Top Left
            if ( newCoords.x < 0.5 && newCoords.y > 0.5 )
            {
                newCoords.y = (newCoords.y - 0.5) * 2.0;
                newCoords.x = newCoords.x * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), newCoords ).xyz );
            }
            // Top Right
            else
            {
                newCoords.x = ( newCoords.x - 0.5 ) * 2.0;
                newCoords.y = ( newCoords.y - 0.5 ) * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), newCoords ).xyz );
            }
        }
        // Right Mid Quadrant
        else
        if ( newCoords.x < 0.75 )
        {
            newCoords.x = ( newCoords.x - 0.5 ) * 4.0;

            // Bottom Left
            if ( newCoords.x < 0.5 && newCoords.y < 0.5 )
            {
                newCoords = newCoords * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer9Map ), newCoords ).xyz );
            }
            else
            // Bottom Right
            if ( newCoords.x > 0.5 && newCoords.y < 0.5 )
            {
                newCoords.x = (newCoords.x - 0.5) * 2.0;
                newCoords.y = newCoords.y * 2.0;

                //lFragCol = GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer10Map, newCoords ).xyz );
                lFragCol = float2vec3( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer10Map ), newCoords ).a );
            }
            else
            // Top Left
            if ( newCoords.x < 0.5 && newCoords.y > 0.5 )
            {
                newCoords.y = (newCoords.y - 0.5) * 2.0;
                newCoords.x = newCoords.x * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), newCoords ).xyz );
            }
            // Top Right
            else
            {
                newCoords.x = ( newCoords.x - 0.5 ) * 2.0;
                newCoords.y = ( newCoords.y - 0.5 ) * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer4Map ), newCoords ).xyz );
            }
        }
        // Right Quadrant
        else
        {
            newCoords.x = ( newCoords.x - 0.75 ) * 4.0;

            // Bottom Left
            if ( newCoords.x < 0.5 && newCoords.y < 0.5 )
            {
                newCoords = newCoords * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer11Map ), newCoords ).xyz );
            }
            else
            // Bottom Right
            if ( newCoords.x > 0.5 && newCoords.y < 0.5 )
            {
                newCoords.x = (newCoords.x - 0.5) * 2.0;
                newCoords.y = newCoords.y * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer12Map ), newCoords ).xyz );
            }
            else
            // Top Left
            if ( newCoords.x < 0.5 && newCoords.y > 0.5 )
            {
                newCoords.y = (newCoords.y - 0.5) * 2.0;
                newCoords.x = newCoords.x * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer5Map ), newCoords ).xyz );
            }
            // Top Right
            else
            {
                newCoords.x = ( newCoords.x - 0.5 ) * 2.0;
                newCoords.y = ( newCoords.y - 0.5 ) * 2.0;

                lFragCol = GammaCorrectOutput( texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer6Map ), newCoords ).xyz );
                //lFragCol = GammaCorrectOutput( CombineBloom( DEREF_PTR( lUniforms.mpCustomPerMesh ), newCoords ) );
            }
        }
    }
    // Top Right 3/4
    else
    {
        newCoords = (newCoords - vec2(0.25,0.25)) / 0.75;

        //lFragCol = texture2D( lUniforms.mpCustomPerMesh.gBufferMap, newCoords.xy ).rgb;
        lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBufferMap ), newCoords ).xyz;

        //vec4 lCloudCol = texture2D( lUniforms.mpCustomPerMesh.gBuffer14Map, newCoords );

        //lFragCol = (lFragCol * (1.0-lCloudCol.a)) + vec3(lCloudCol.a);
        //lFragCol = mix(lFragCol, vec3(lCloudCol), lCloudCol);
        //lFragCol = (lFragCol * (1.0-lCloudCol.a)) + (vec3(lCloudCol.xyz) );

        lFragCol = TonemapKodak( lFragCol ) / TonemapKodak( vec3(1.0, 1.0, 1.0) );
        lFragCol = GammaCorrectOutput( lFragCol );

        /*
        //if ( newCoords.x > 0.25 )
        {
            lFragCol = texture3DLod( lUniforms.mpCustomPerMesh.gColourLUT, lFragCol, 0.0 ).rgb;
        }
        */

        //lFragCol += GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer14Map, newCoords.xy ).rgb );
        
        //lFragCol = blendScreen( lFragCol, texture2D( lUniforms.mpCustomPerMesh.gBuffer11Map, newCoords.xy ).rgb );
        /*
        lFragCol = blendScreen( lFragCol, GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer11Map, newCoords.xy ).rgb ) );
        lFragCol = blendScreen( lFragCol, GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer8Map, newCoords.xy ).rgb ) );
        lFragCol = blendScreen( lFragCol, GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer12Map, newCoords.xy ).rgb ) * texture2D( lUniforms.mpCustomPerMesh.gBuffer12Map, newCoords.xy ).a );
        */
        //lFragCol = blendScreen( lFragCol, GammaCorrectOutput( texture2D( lUniforms.mpCustomPerMesh.gBuffer12Map, newCoords.xy ).rgb ) );

        /*
        if ( newCoords.x > 0.5 )
        {
            lFragCol = texture2D( lUniforms.mpCustomPerMesh.gBufferMap, newCoords.xy ).rgb;
        }
        */
    } 

    FRAGMENT_COLOUR = vec4( lFragCol.rgb, 1.0 );
}

#endif

#ifdef D_DEBUG_QUAD_SPLIT

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END

vec2
GetCoords(
    SAMPLER2DARG( lTexture ),
    float   lfTextureIndex,
    float   lfZoomIndex,
    bool    lbZoomForced,
    float   lfZoomScale,
    vec2    lFragCoords, 
    vec2    lZoomOffset )
{
    vec2 lTextureSize;
    vec2 lZoomCoords;

    lTextureSize    = GetTexResolution( lTexture );
    lZoomCoords     = lFragCoords / lfZoomScale + lZoomOffset;
    lFragCoords     = lfZoomIndex == lfTextureIndex || lbZoomForced ? lZoomCoords : lFragCoords;

    return  lFragCoords;
}

//-----------------------------------------------------------------------------
//      Functions

FRAGMENT_MAIN_COLOUR_SRT
{
    vec4    lFragCol        = float2vec4( 0.0 );
    vec2    lFragCoords     = IN( mTexCoordsVec2 );
    
    bool    lbZoomAll       = lUniforms.mpCustomPerMesh.gDebugZoomVec4.x == 42.0;
    float   lfZoomIndex     = lUniforms.mpCustomPerMesh.gDebugZoomVec4.x;
    float   lfZoomScale     = lUniforms.mpCustomPerMesh.gDebugZoomVec4.y;
    vec2    lZoomOffset     = lUniforms.mpCustomPerMesh.gDebugZoomVec4.zw;

    if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.x <= 0.0 )
    {
        lFragCoords     = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer0Map ),
                                     0.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

        lFragCol        = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0Map ), lFragCoords );
        FRAGMENT_COLOUR = vec4( lFragCol );
        return;
    }
    
    if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.y <= 0.0 )
    {
        lFragCoords     = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ),
                                     1.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

        lFragCol        = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lFragCoords );
        FRAGMENT_COLOUR = vec4( lFragCol );
        return;
    }

    if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.z <= 0.0 )
    {
        lFragCoords     = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ),
                                     2.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

        lFragCol        = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lFragCoords );
        FRAGMENT_COLOUR = vec4( lFragCol );
        return;
    }

    if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.w <= 0.0 )
    {
        lFragCoords     = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ),
                                     3.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

        lFragCol        = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lFragCoords );
        FRAGMENT_COLOUR = vec4( lFragCol );
        return;
    }

    // Top
    if ( lFragCoords.y < 0.5 )
    {
        lFragCoords.y *= 2.0;

        // Top Left
        if ( lFragCoords.x < 0.5 )
        {
            lFragCoords.x *= 2.0;

            if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.x <= 1.0 )
            {   
                lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer0Map ), 
                                         0.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer0Map ), lFragCoords );
            }
            else
            {
                if ( lFragCoords.y < 0.5 )
                {
                    lFragCoords.y *= 2.0;

                    // Top Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer000Map ),
                                                 0.000, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer000Map ), lFragCoords );
                    }
                    // Top Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer001Map ),
                                                 0.001, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer001Map ), lFragCoords );
                    }
                }
                else
                {
                    lFragCoords.y -= 0.5;
                    lFragCoords.y *= 2.0;

                    // Bottom Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer002Map ),
                                                 0.002, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer002Map ), lFragCoords );
                    }
                    // Bottom Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer003Map ), 
                                                 0.003, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer003Map ), lFragCoords );
                    }
                }
            }            
        }
        // Top Right
        else
        {
            lFragCoords.x -= 0.5;
            lFragCoords.x *= 2.0;

            if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.y <= 1.0 )
            {
                lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer1Map ), 
                                         1.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer1Map ), lFragCoords );
            }
            else
            {
                if ( lFragCoords.y < 0.5 )
                {
                    lFragCoords.y *= 2.0;

                    // Top Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer100Map ),
                                                 0.100, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer100Map ), lFragCoords );
                    }
                    // Top Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer101Map ),
                                                 0.101, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer101Map ), lFragCoords );
                    }
                }
                else
                {
                    lFragCoords.y -= 0.5;
                    lFragCoords.y *= 2.0;

                    // Bottom Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer102Map ),
                                                 0.102, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer102Map ), lFragCoords );
                    }
                    // Bottom Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer103Map ),
                                                 0.103, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer103Map ), lFragCoords );
                    }
                }
            }
        }
    }
    // Bottom
    else
    {
        lFragCoords.y -= 0.5;
        lFragCoords.y *= 2.0;

        if ( lFragCoords.x < 0.5 )
        {
            lFragCoords.x *= 2.0;

            if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.z <= 1.0 )
            {
                lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer2Map ),
                                         2.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer2Map ), lFragCoords );
            }
            else
            {
                if ( lFragCoords.y < 0.5 )
                {
                    lFragCoords.y *= 2.0;

                    // Top Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer200Map ),
                                                 0.200, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer200Map ), lFragCoords );
                    }
                    // Top Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer201Map ),
                                                 0.201, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer201Map ), lFragCoords );
                    }
                }
                else
                {
                    lFragCoords.y -= 0.5;
                    lFragCoords.y *= 2.0;

                    // Bottom Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer202Map ),
                                                 0.202, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer202Map ), lFragCoords );
                    }
                    // Bottom Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer203Map ),
                                                 0.203, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer203Map ), lFragCoords );
                    }
                }
            }
        }
        else
        {
            lFragCoords.x -= 0.5;
            lFragCoords.x *= 2.0;

            if ( lUniforms.mpCustomPerMesh.gDebugSplitVec4.w <= 1.0 )
            {
                lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer3Map ),
                                         3.0, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer3Map ), lFragCoords );
            }
            else
            {
                if ( lFragCoords.y < 0.5 )
                {
                    lFragCoords.y *= 2.0;

                    // Top Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer300Map ),
                                                 0.300, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer300Map ), lFragCoords );
                    }
                    // Top Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer301Map ),
                                                 0.301, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer301Map ), lFragCoords );
                    }
                }
                else
                {
                    lFragCoords.y -= 0.5;
                    lFragCoords.y *= 2.0;

                    // Bottom Left
                    if ( lFragCoords.x < 0.5 )
                    {
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer302Map ),
                                                 0.302, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer302Map ), lFragCoords );
                    }
                    // Bottom Right
                    else
                    {
                        lFragCoords.x -= 0.5;
                        lFragCoords.x *= 2.0;

                        lFragCoords = GetCoords( SAMPLER2DPARAM_SRT( lUniforms.mpCustomPerMesh, gBuffer303Map ),
                                                 0.303, lfZoomIndex, lbZoomAll, lfZoomScale, lFragCoords, lZoomOffset );

                        lFragCol    = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh, gBuffer303Map ), lFragCoords );
                    }
                }
            }
        }
    }

    FRAGMENT_COLOUR = vec4( lFragCol );
}

#endif

// =================================================================================================
//
// D_DEBUG_QUAD_COMBINE
//
// =================================================================================================

#ifdef D_DEBUG_QUAD_COMBINE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions


FRAGMENT_MAIN_COLOUR_SRT
{
    //Debug

    vec3 lFragCol = vec3( 1.0, 1.0, 1.0 );

    // Bottom Left
    if ( IN( mTexCoordsVec2 ).x < 0.5 && IN( mTexCoordsVec2 ).y < 0.5 )
    {
        lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBufferMap ), IN( mTexCoordsVec2 ).xy ).rgb;
    }
    else
    // Bottom Right
    if ( IN( mTexCoordsVec2 ).x > 0.5 && IN( mTexCoordsVec2 ).y < 0.5 )
    {
        lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer1Map ), IN( mTexCoordsVec2 ).xy ).rgb;
    }
    else
    // Top Left
    if ( IN( mTexCoordsVec2 ).x < 0.5 && IN( mTexCoordsVec2 ).y > 0.5 )
    {
        lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer2Map ), IN( mTexCoordsVec2 ).xy ).rgb;
    }
    // Top Right
    else
    {
        lFragCol = texture2D( SAMPLER_GETMAP( lUniforms.mpCustomPerMesh,gBuffer3Map ), IN( mTexCoordsVec2 ).xy ).rgb;
    }

    FRAGMENT_COLOUR = vec4( lFragCol.rgb, 1.0 );
}

#endif



// =================================================================================================
//
// D_DEBUG_CIRCLE
//
// =================================================================================================

#ifdef D_DEBUG_CIRCLE

//-----------------------------------------------------------------------------
//      Global Data

//-----------------------------------------------------------------------------
//      Typedefs and Classes 

DECLARE_INPUT
    INPUT_SCREEN_POSITION

    INPUT( vec2, mTexCoordsVec2,     TEXCOORD0 )
DECLARE_INPUT_END

//-----------------------------------------------------------------------------
//      Functions


FRAGMENT_MAIN_COLOUR_SRT
{
    //Debug

    vec2  lRatio     = lUniforms.mpPerFrame.gFrameBufferSizeVec4.zz / lUniforms.mpPerFrame.gFrameBufferSizeVec4.zw;
    vec2  lTexCoords = ( IN( mTexCoordsVec2 ).xy - 0.5 ) * 2.0 * lRatio;
    float lfRad      = 0.005;
    bool  lbInside   = lTexCoords.x * lTexCoords.x + lTexCoords.y * lTexCoords.y < lfRad;
    vec3  lFragCol   = lbInside ? vec3( 1.0, 0.0, 0.0 ) : float2vec3( 0.0 );

    FRAGMENT_COLOUR = vec4( lFragCol, 1.0 );
}

#endif
