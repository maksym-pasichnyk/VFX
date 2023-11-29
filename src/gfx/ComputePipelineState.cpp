#include "ComputePipelineState.hpp"

gfx::ComputePipelineState::ComputePipelineState(rc<Device> device) : device(std::move(device)) {}
gfx::ComputePipelineState::~ComputePipelineState() {
    for (auto& layout : descriptor_set_layouts) {
        this->device->handle.destroyDescriptorSetLayout(layout, nullptr, this->device->dispatcher);
    }
    this->device->handle.destroyPipelineLayout(this->pipeline_layout, nullptr, this->device->dispatcher);
    this->device->handle.destroyPipeline(this->pipeline, nullptr, this->device->dispatcher);
}