#ifndef VFX_GLOBALS
#define VFX_GLOBALS

layout(push_constant) uniform Globals {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewProjectionMatrix;
    mat4 InverseViewProjectionMatrix;

    vec3 CameraPosition;

    ivec2 Resolution;
    float Time;

    mat4 ModelMatrix;
};

#endif