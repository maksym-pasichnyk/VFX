#version 450

layout(push_constant) uniform Globals {
    bool IsDepthAttachment;
    bool IsHDREnabled;
    float Exposure;
    float Gamma;
};

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler textureSampler;
layout(set = 0, binding = 1) uniform texture2D albedoTexture;
layout(set = 0, binding = 2) uniform texture2D depthTexture;

layout(location = 0) in struct {
    vec2 texcoord;
} v_in;

void main() {
    vec3 color;
    if (IsDepthAttachment) {
        float depthSample = texture(sampler2D(depthTexture, textureSampler), v_in.texcoord).r;
        color = vec3(depthSample / 0.01f);
    } else {
        color = texture(sampler2D(albedoTexture, textureSampler), v_in.texcoord).rgb;
    }

    if (IsHDREnabled) {
//        color = color / (1.0f + color);
        color = vec3(1.0) - exp(-color * Exposure);
        color = pow(color, vec3(1.0f / Gamma));
    }
    out_color = vec4(color, 1.0f);
}