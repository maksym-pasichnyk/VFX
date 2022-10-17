#version 450

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform texture2D mainTexture;
layout(set = 0, binding = 1) uniform sampler mainSampler;

layout(location = 0) in struct {
    vec2 texcoord;
} v_in;

void main() {
    out_color = texture(sampler2D(mainTexture, mainSampler), v_in.texcoord);
}