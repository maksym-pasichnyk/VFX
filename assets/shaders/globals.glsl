#ifndef VFX_GLOBALS
#define VFX_GLOBALS

layout(set = 0, binding = 0) uniform SceneConstants {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewProjectionMatrix;
    mat4 InverseViewProjectionMatrix;

    vec3 CameraPosition;

    ivec2 Resolution;
    float Time;
};

layout(push_constant) uniform ModelConstants {
    mat4 ModelMatrix;
};

#endif