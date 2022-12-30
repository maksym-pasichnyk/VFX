#version 450 core

struct Vertex {
	vec3 position;
	vec3 color;
};

layout(binding = 0) readonly buffer VertexInput {
	Vertex[] vertices;
};

layout(location = 0) out struct {
	vec3 color;
} vs_out;

void main() {
	vs_out.color = vertices[gl_VertexIndex].color;

	gl_Position = vec4(vertices[gl_VertexIndex].position, 1);
}