#version 450

#include "globals.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in struct {
    vec4 color;
} v_in;

void main() {
    out_color = vec4(vec3(gl_FragCoord.w), 1.0f) * v_in.color;
}