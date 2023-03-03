#version 450

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 fragVertexColor;
layout(location = 1) out vec3 fragOrigin;
layout(location = 2) out vec3 fragDirection;

layout(push_constant) uniform SceneConstants {
    mat4x4 ProjectionMatrix;
    mat4x4 WorldToCameraMatrix;
    mat4x4 ViewProjectionMatrix;
    mat4x4 InverseViewProjectionMatrix;
    uint   Volume[16];
    vec3   CameraPosition;
};

void main(){
    gl_Position = ViewProjectionMatrix * vec4(vertexPosition, 1.0);
    vec4 worldPos = WorldToCameraMatrix * vec4(vertexPosition, 1.0);

    fragOrigin = vertexPosition;
    fragDirection = vertexPosition - CameraPosition;

    fragVertexColor = vertexColor;
}