#version 450 core

layout(push_constant) uniform GuiShaderData {
	vec2 mScale;
};

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec4 in_color;

layout(location = 0) out struct {
	vec4 color;
} vs_out;

void main() {
	vs_out.color = in_color;

	gl_Position = vec4(in_position * mScale - 1.0F, 0, 1);
}