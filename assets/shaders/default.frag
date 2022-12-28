#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in struct {
    vec3 color;
} vs_in;

void main() {
    out_color = vec4(vs_in.color, 1.0F);
}