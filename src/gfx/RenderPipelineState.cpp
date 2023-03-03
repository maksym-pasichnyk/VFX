#include "RenderPipelineState.hpp"

auto gfx::RenderPipelineColorBlendAttachmentStateArray::operator[](size_t i) -> vk::PipelineColorBlendAttachmentState& {
    if (elements.size() >= i) {
        elements.resize(i + 1, vk::PipelineColorBlendAttachmentState{
            .colorWriteMask =
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
        });
    }
    return elements[i];
}

auto gfx::RenderPipelineColorAttachmentFormatArray::operator[](size_t i) -> vk::Format& {
    if (elements.size() >= i) {
        elements.resize(i + 1, vk::Format::eUndefined);
    }
    return elements[i];
}

gfx::RenderPipelineStateShared::RenderPipelineStateShared(gfx::Device device) : device(std::move(device)) {}

gfx::RenderPipelineStateShared::~RenderPipelineStateShared() {
    for (auto& layout : bind_group_layouts) {
        device.shared->raii.raw.destroyDescriptorSetLayout(layout, nullptr, device.shared->raii.dispatcher);
    }
    device.shared->raii.raw.destroyPipelineLayout(pipeline_layout, nullptr, device.shared->raii.dispatcher);
    device.shared->raii.raw.destroyPipeline(pipeline, nullptr, device.shared->raii.dispatcher);
}