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

gfx::RenderPipelineState::RenderPipelineState(rc<Device> device, rc<RenderPipelineStateDescription> description)
: device(std::move(device))
, description(std::move(description)) {}

gfx::RenderPipelineState::~RenderPipelineState() {
    for (auto& layout : descriptorSetLayouts) {
        device->handle.destroyDescriptorSetLayout(layout, nullptr, device->dispatcher);
    }

    device->handle.destroyPipelineLayout(pipelineLayout, nullptr, device->dispatcher);
    device->handle.destroyPipelineCache(pipelineCache, nullptr, device->dispatcher);

    for (auto [_, pipeline] : pipelines) {
        device->handle.destroyPipeline(pipeline, nullptr, device->dispatcher);
    }
}