# NMS-ShaderCode
Shader code taken from the initial macOS release of No Man's Sky


The main difference between .h and .bin seems to be that .bin uses -> syntax while .h uses . syntax


`//TF_BEGIN` and `//TF_END` refer to [The Forge](theforge.dev) and sections enclosed by these commits should be assumed to deal with macOS and iOS (and thus, Metal).

## How to comprehend NMS shaders

NMS shaders are written in GLSL. There are a few types:

- standard GLSL (fragment, compute, vertex, geometry...)
- "common" shaders - treat these like header files
- .glsl, .hlsl - these should be assumed to come from elsewhere, ie an SDK
- .shader.bin in the root directory - these are "shader resources"; think of them as sorta like build targets. 

(to be continued)