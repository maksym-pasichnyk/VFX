#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 projection;
    mat4 view;
} global;

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out struct {
	vec4 color;
} v_out;

void main() {
	v_out.color = in_color;

	gl_Position = global.projection * global.view * vec4(in_pos, 1);
}