////////////////////////////////////////////////////////////////////////////////
///
///     @file       ShaderMillDefines.h
///     @author     Charlie Tangora
///     @date       
///
///     @brief      ShaderMillDefines
///
///     Copyright (c) 2020 Hello Games Ltd. All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      Compilation defines 

#ifndef D_SM_DEFINES
#define D_SM_DEFINES

#include "Common/CommonFragment.shader.h"

vec2
SkAnimateTexture(
    vec2  lTexCoord,
    vec2  lTexSize,
    float lfFrameNumber )
{
    vec2  lMappedUV;
    float lfFrameY = floor( (lfFrameNumber + 0.5) * lTexSize.x );

    lMappedUV.x = lTexCoord.x       * lTexSize.x + fract( (lfFrameNumber + 0.5) * lTexSize.x ) - lTexSize.x*0.5;
    lMappedUV.y = (1.0-lTexCoord.y) * lTexSize.y + fract( (lfFrameY      + 0.5) * lTexSize.y ) - lTexSize.y*0.5;
    lMappedUV.y = 1.0 - lMappedUV.y;

    return lMappedUV;
}

vec2
SkRotateUv(
    vec2  lTexCoord,
    vec2  lTexCenter,
    float lfRotation )
{
    float lfSin = sin( lfRotation );
    float lfCos = cos( lfRotation );

    mat4 lRotationMat4 = mat4( lfCos, -lfSin, 0, 0,
                               lfSin,  lfCos, 0, 0,
                               0,  0, 1, 0,
                               0,  0, 0, 1 );
    vec4 lCoord4 = vec4( lTexCoord - lTexCenter, 0, 1 );
    lCoord4 = MUL(lRotationMat4, lCoord4);
    lCoord4.xy += lTexCenter;

	return lCoord4.xy;
}

vec3
SkComputePerspectiveUV(
    vec3 lCameraPos,
    vec3 lFaceNormal,
    vec3 lWorldPosition,
    mat4 lWorldMatrix,
    vec3 lCentrePosition)
{
    /*vec3 difference = lCameraPos - lCentrePosition;
    float distance = length(difference);
    distance = sqrt(distance);
    distance = max(distance, 1.0);*/

    //Calc the plane line intersection to lookup the appropriate UV
    vec3 direction = lWorldPosition - lCameraPos;
    //Convert to local space
    mat4 lInverseMatrix = transpose(lWorldMatrix);
    direction = MUL(lInverseMatrix, vec4(direction, 0.0)).xyz;

    float scale = dot(direction, lFaceNormal);

#if 0
    //Bendy scaled version:
    float distance = 5.0;
    vec3 lLocalPos = lWorldPosition - lCentrePosition;
    direction = normalize(direction);
    float t = (distance - dot(lLocalPos, lFaceNormal)) / scale;
    direction = lLocalPos + (t * direction);

    //Now calculate the uv value much the same as before
    //Now, take the x/z and the y component of the vector and convert to uv's
    float xzComponent = (lFaceNormal.x == 0.0) ? direction.x : direction.z;

    float normalised = 0.5 / distance;
    //Assume values are for plane 1 unit away from us, with a FOV of 90 degrees
    vec3 uv = vec3(saturate(normalised * (xzComponent + distance)), 1.0 - saturate(normalised * (direction.y + distance)), 0.0);

    //Finally, set the third value to be the validity multiplier - i.e. is this uv actually within the plane?
    uv.z = ((abs(xzComponent) < distance) && (abs(direction.y) < distance) && (scale > 0.0)) ? 1.0 : 0.0;

#else

    //Fixed box version:
    direction /= scale;  //Normalise the length of the vector to lie exactly on a plane, a unit length away from us

    //Now, take the x/z and the y component of the vector and convert to uv's
    float xzComponent = (lFaceNormal.x == 0.0) ? direction.x : direction.z;

    //Assume values are for plane 1 unit away from us, with a FOV of 90 degrees
    vec3 uv = vec3(saturate(0.5 * (xzComponent + 1.0)), 1.0 - saturate(0.5 * (direction.y + 1.0)), 0.0);
    
    //Finally, set the third value to be the validity multiplier - i.e. is this uv actually within the plane?
    uv.z = ((abs(xzComponent) < 1.0) && (abs(direction.y) < 1.0) && (scale > 0.0)) ? 1.0 : 0.0;
#endif
    return uv;
}

#define SM_MAKE_VEC1( nme ) float nme = 0.0
#define SM_MAKE_VEC2( nme ) vec2 nme = vec2(0.0, 0.0)
#define SM_MAKE_VEC3( nme ) vec3 nme = vec3(0.0, 0.0, 0.0)
#define SM_MAKE_VEC4( nme ) vec4 nme = vec4(0.0, 0.0, 0.0, 1.0)

#define SM_INPUT( value, outLink ) outLink = value
#define SM_OUTPUT( value, inLink ) value = inLink

#if defined D_VERTEX
#define SM_READTEXTURE( inTexture, inUv, outColour ) outColour = texture2DLod( SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, inTexture),  inUv, 1.0 )
#else
#define SM_READTEXTURE( inTexture, inUv, outColour ) outColour = texture2D( SAMPLER_GETMAP(lUniforms.mpCustomPerMaterial, inTexture),  inUv )
#endif

#define SM_VALUE( value, outLink ) outLink = value

#define SM_IF( value, inLink0, inLink1, inLink2, outLink ) outLink = inLink0 != 0.0 ? inLink1 : inLink2
#define SM_MAX( value, inLink0, inLink1, outLink ) outLink = max(inLink0, inLink1)
#define SM_MIN( value, inLink0, inLink1, outLink ) outLink = min(inLink0, inLink1)

#define SM_ADD( value, inLink0, inLink1, outLink ) outLink = inLink0 + inLink1
#define SM_SUB( value, inLink0, inLink1, outLink ) outLink = inLink0 - inLink1
#define SM_MUL( value, inLink0, inLink1, outLink ) outLink = inLink0 * inLink1
#define SM_DIV( value, inLink0, inLink1, outLink ) outLink = inLink0 / inLink1

#define SM_ADDWITHSCALE( value, inLink0, inLink1, outLink ) outLink = inLink0 * value + inLink1

#define SM_FRACT( value, inLink, outLink ) outLink = fract( inLink )
#define SM_FLOOR( value, inLink, outLink ) outLink = floor( inLink )
#define SM_ROUND( value, inLink, outLink ) outLink = round( inLink )

#define SM_SIN( value, inLink, outLink ) outLink = sin( inLink * value )
#define SM_COS( value, inLink, outLink ) outLink = cos( inLink * value )

#define SM_SQUAREWAVE( value, inLink, outLink ) outLink = 1.0 - 2.0 * round(fract( inLink * value ))
#define SM_SAWTOOTH( value, inLink, outLink ) outLink = 2.0 * (( inLink * value ) - floor(0.5 + ( inLink * value )))

#define SM_CONTRAST( value, inLink, outLink ) outLink = saturate( (inLink-0.5) * value + 0.5 )
#define SM_CLAMP( value, inLink, outLink ) outLink = saturate( inLink )
#define SM_SMOOTHSTEP( value, inLink, outLink ) outLink = smoothstep( 0.0, 1.0, inLink )
#define SM_SMOOTHREMAP( value, inLink, outLink ) outLink = smoothstep( value.x, value.y, inLink )
#define SM_SATURATE( value, inLink, outLink ) outLink.x = dot(inLink.rgb, vec3(0.2126729, 0.7151522, 0.0721750)); outLink.rgb = lerp( outLink.xxx, inLink.rgb, value );

#define SM_BLEND( value, inLink0, inLink1, inBlend, outLink ) outLink = lerp( inLink0, inLink1, inBlend )

#define SM_COPY2( value, inVec, outVec ) outVec = inVec
#define SM_COPY3( value, inVec, outVec ) outVec = inVec
#define SM_COPY4( value, inVec, outVec ) outVec = inVec

#define SM_SPLAT1( value ) value
#define SM_SPLAT2( value ) vec2(value, value)
#define SM_SPLAT3( value ) vec3(value, value, value)
#define SM_SPLAT4( value ) vec4(value, value, value, value)

#define SM_RGBTOHSV( value, inVec, outVec ) outVec = RGBToHSV( inVec )
#define SM_HSVTORGB( value, inVec, outVec ) outVec = HSVToRGB( inVec )

#define SM_DECODENORMALMAP( value, inVec, outVec ) outVec = DecodeNormalMap( inVec )

#define SM_UVFLIPBOOK( value, inUv, inFrame, outUv ) outUv = SkAnimateTexture( inUv, value, inFrame )
#define SM_UVROTATE( value, inUv, inRotate, outUv ) outUv = SkRotateUv( inUv, value, inRotate )

#define SM_DEGREESTORADIANS( value, inVec, outVec ) outVec = ( inVec * 0.0174532925 )
#define SM_RADIANSTODEGREES( value, inVec, outVec ) outVec = ( inVec * (1.0 / 0.0174532925) )

#define SM_COMPUTEANGLE( value, inFulcrum, inA, inB, outCosAngle, outRadians ) \
 outCosAngle = dot( normalize( inA - inFulcrum ), normalize( inB - inFulcrum ) ); \
 outRadians = acos( outCosAngle )

#ifdef D_VERTEX
   #define SM_VERTEXINTERPOLATE( value, inVec, outVec ) OUT( mSkInterp_ ## value ) = inVec; outVec = inVec
#elif defined( D_FRAGMENT )
   #define SM_VERTEXINTERPOLATE( value, inVec, outVec ) outVec = IN( mSkInterp_ ## value ) 
#elif defined( D_GEOMETRY )
   #define SM_VERTEXINTERPOLATE( value, inVec, outVec ) outVec = IN( mSkInterp_ ## value, ii )
#endif

#define SM_NEGATE( value, inVec, outVec ) outVec = -inVec
#define SM_POWER( value, inVec, inExp, outVec ) outVec = pow( inVec, inExp )
#define SM_ONEMINUS( value, inVec, outVec ) outVec = 1.0 - inVec
#define SM_VECTORLENGTH( value, inVec, outVec ) outVec = length( inVec )

#define SM_COMPUTEPERSPECTIVEUV( value, inCameraPosition, inFaceNormal, inWorldPosition, outValue ) outValue = SkComputePerspectiveUV( inCameraPosition, inFaceNormal, inWorldPosition, lUniforms.mpCommonPerMesh.gWorldMat4, lSkNodePositionVec3 )
#define SM_INDEXMATCH( value, inIndex, outValue ) outValue = float( int( value ) == int( inIndex ) )

//-----------------------------------------------------------------------------
//      Include files


//-----------------------------------------------------------------------------
//      Global Data



#endif // SM_DEFINES
