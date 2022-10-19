#version 450

layout(push_constant) uniform Globals {
    bool IsDepthAttachment;
};

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform texture2D mainTexture;
layout(set = 0, binding = 1) uniform sampler mainSampler;

layout(location = 0) in struct {
    vec2 texcoord;
} v_in;

void main() {
    if (IsDepthAttachment) {
        float depthSample = texture(sampler2D(mainTexture, mainSampler), v_in.texcoord).r;

        float depth = (1.0f - depthSample) / 0.01f;

        out_color = vec4(depth.xxx, 1.0f);
    } else {
        out_color = texture(sampler2D(mainTexture, mainSampler), v_in.texcoord);
    }
}