#version 450

#include "globals.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in struct {
    vec4 color;
    float depth;
} v_in;

void main() {
    float depth = gl_FragCoord.z / gl_FragCoord.w;

//    if (abs(depth - v_in.depth) < 0.0001f) {
//        discard;
//    }

    out_color = vec4(vec3(1.0f / depth), 1.0f) * v_in.color;
    gl_FragDepth = 1.0f - 0.01f * 1.0f / v_in.depth;
}