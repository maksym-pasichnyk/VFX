#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in struct {
    vec4 color;
} v_in;

void main() {
    out_color = v_in.color;
}