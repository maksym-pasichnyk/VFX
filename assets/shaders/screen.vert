#version 450 core

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 projection;
    mat4 view;
} global;

const vec2 vertices[6] = vec2[](
    vec2(-1, -1),
    vec2(-1,  1),
    vec2( 1, -1),

    vec2( 1, -1),
    vec2(-1,  1),
    vec2( 1,  1)
);

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
}