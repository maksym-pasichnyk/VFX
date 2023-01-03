#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in struct {
    vec4 color;
} vs_in;

void main() {
    out_color = vs_in.color;
}