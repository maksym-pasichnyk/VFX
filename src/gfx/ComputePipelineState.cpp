#include "ComputePipelineState.hpp"

gfx::ComputePipelineState::ComputePipelineState(ManagedShared<Device> device) : device(std::move(device)) {}
gfx::ComputePipelineState::~ComputePipelineState() {
    for (auto& layout : descriptor_set_layouts) {
        device->raii.raw.destroyDescriptorSetLayout(layout, nullptr, device->raii.dispatcher);
    }
    device->raii.raw.destroyPipelineLayout(pipeline_layout, nullptr, device->raii.dispatcher);
    device->raii.raw.destroyPipeline(pipeline, nullptr, device->raii.dispatcher);
}