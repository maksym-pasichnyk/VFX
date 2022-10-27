#version 450 core

#include "globals.glsl"

out gl_PerVertex {
	vec4 gl_Position;
};

layout(set = 0, binding = 0) uniform SceneConstantData { SceneConstants sceneConstants; };

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out struct {
	vec4 color;
} v_out;

void main() {
	vec4 position = sceneConstants.ViewProjectionMatrix * objectConstants.transform * vec4(in_position, 1);

	v_out.color = in_color;

	gl_Position = position;
}