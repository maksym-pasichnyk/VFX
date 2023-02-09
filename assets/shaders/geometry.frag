#version 450 core

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler in_sampler;
layout(set = 0, binding = 1) uniform texture2D in_texture;

layout(location = 0) in struct {
    vec4 color;
	vec2 uv;
} vs_in;

void main() {
    out_color = texture(sampler2D(in_texture, in_sampler), vs_in.uv) * vs_in.color;
}