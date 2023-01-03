#version 450 core

layout(push_constant) uniform ShaderData {
    mat4x4 g_proj_matrix;
    mat4x4 g_view_matrix;
};

layout (location = 0) in vec3 in_vertex_position;
layout (location = 1) in vec3 in_position;
layout (location = 2) in vec4 in_color;

layout (location = 0) out vec4 vs_color;

void main() {
    vec4 clip_pos = g_view_matrix * vec4(in_position, 1) + vec4(in_vertex_position, 0);

    gl_Position = g_proj_matrix * clip_pos;

    vs_color = in_color;
}