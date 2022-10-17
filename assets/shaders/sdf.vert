#version 450 core

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out struct {
	vec2 fragCoord;
} v_out;

layout(push_constant) uniform Globals {
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	mat4 ViewProjectionMatrix;
	vec3 CameraPosition;

	ivec2 Resolution;
	float Time;

	mat4 ModelMatrix;
};

const vec2 vertices[6] = vec2[](
	vec2(-1, -1), vec2(-1,  1),
	vec2( 1, -1), vec2( 1, -1),
	vec2(-1,  1), vec2( 1,  1)
);

const vec2 texcoords[6] = vec2[](
	vec2(0, 1), vec2(0, 0),
	vec2(1, 1), vec2(1, 1),
	vec2(0, 0), vec2(1, 0)
);

void main() {
	v_out.fragCoord = texcoords[gl_VertexIndex] * vec2(Resolution);

	gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
}