#version 450 core

#include "globals.glsl"

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out struct {
	vec4 color;
	float depth;
} v_out;

void main() {
	vec4 position = ViewProjectionMatrix * ModelMatrix * vec4(in_position, 1);

	v_out.color = in_color;
	v_out.depth = (position.z + position.w - 0.01f * 2.0f) * 0.5f;

	gl_Position = position;
}