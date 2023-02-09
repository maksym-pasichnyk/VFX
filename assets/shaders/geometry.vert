#version 450 core

layout(push_constant) uniform ShaderData {
	mat4x4 g_proj_matrix;
	mat4x4 g_view_matrix;
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out struct {
	vec4 color;
	vec2 uv;
} vs_out;

void main() {
	gl_Position = g_proj_matrix * g_view_matrix * vec4(in_position, 1);

	vs_out.color = in_color;
	vs_out.uv = in_uv;
}