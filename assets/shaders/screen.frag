#version 450

layout(location = 0) out vec4 out_color;

layout (input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput input_color_attachment;

void main() {
    out_color = subpassLoad(input_color_attachment);
}