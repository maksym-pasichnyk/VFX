#version 450 core

layout (location = 0) out vec4 fs_color;

layout (location = 0) in vec4 vs_color;

void main() {
    fs_color = vs_color;
}