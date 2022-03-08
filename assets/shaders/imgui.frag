#version 450 core

layout(location = 0) out vec4 fColor;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

layout(set = 0, binding = 0) uniform sampler2D sTexture;

void main() {
    fColor = In.Color * texture(sTexture, In.UV.st);
}