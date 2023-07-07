#version 440 core
#extension GL_ARB_gpu_shader5 : enable

in vec2 mkLocalPositionVec4; // must be called Vec4 due to app strcmp
in vec2 mkTexCoordsVec4; // must be called Vec4 due to app strcmp
// DECLARE_INPUT
//     INPUT( vec2, mkLocalPositionVec4, POSITION0 ) // must be called Vec4 due to app strcmp
//     INPUT( vec2, mkTexCoordsVec4,     TEXCOORD0 ) // must be called Vec4 due to app strcmp
// DECLARE_END



out vec2 mTexCoordsVec2;
out vec2 mPosVec2;
// DECLARE_OUTPUT
//     OUTPUT_SCREEN_POSITION
//     OUTPUT( vec2,   mTexCoordsVec2,     TEXCOORD0 )
//     OUTPUT( vec2,   mPosVec2,           TEXCOORD1 )
// DECLARE_END


uniform vec2 viewSize;

void main()
//VERTEX_MAIN_SRT
{

    mTexCoordsVec2         = mkTexCoordsVec4;
    mPosVec2               = mkLocalPositionVec4;

    gl_Position            =
    // SCREEN_POSITION = 
                             vec4( 2.0 * mkLocalPositionVec4.x / viewSize.x - 1.0, 
                                   1.0 - 2.0 * mkLocalPositionVec4.y / viewSize.y, 
                                   0, 
                                   1 );

}



