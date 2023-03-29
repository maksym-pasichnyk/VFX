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

gfx::RenderPipelineState::RenderPipelineState(ManagedShared<Device> device) : device(std::move(device)) {}

gfx::RenderPipelineState::~RenderPipelineState() {
    for (auto& layout : bind_group_layouts) {
        device->raii.raw.destroyDescriptorSetLayout(layout, nullptr, device->raii.dispatcher);
    }
    device->raii.raw.destroyPipelineLayout(pipeline_layout, nullptr, device->raii.dispatcher);
    device->raii.raw.destroyPipeline(pipeline, nullptr, device->raii.dispatcher);
}