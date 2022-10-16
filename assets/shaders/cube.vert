#version 450 core

out gl_PerVertex {
	vec4 gl_Position;
};

layout(push_constant) uniform Globals {
	mat4 LocalToWorldMatrix;
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out struct {
	vec4 color;
} v_out;

void main() {
	v_out.color = in_color;

	gl_Position = LocalToWorldMatrix * vec4(in_position, 1);
}