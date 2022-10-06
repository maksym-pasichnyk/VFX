#version 450

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D main_texture;

layout(location = 0) in struct {
    vec2 texcoord;
} v_in;

void main() {
    out_color = texture(main_texture, v_in.texcoord);
}