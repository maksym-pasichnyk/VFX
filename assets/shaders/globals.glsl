#ifndef VFX_GLOBALS
#define VFX_GLOBALS

struct SceneConstants {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewProjectionMatrix;
    mat4 InverseViewProjectionMatrix;

    vec3 CameraPosition;

    ivec2 Resolution;
    float Time;
};

struct ObjectConstants {
    mat4 transform;
};

struct MaterialConstants {
    vec4 color;
    vec3 ambient;
};

struct LightConstants {
    vec3 position;
    vec3 color;

    float brightness;
    float intensity;
};

layout(push_constant) uniform ObjectConstantData {
    ObjectConstants objectConstants;
};


#endif