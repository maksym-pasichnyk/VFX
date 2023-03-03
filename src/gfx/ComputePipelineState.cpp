#include "ComputePipelineState.hpp"

gfx::ComputePipelineStateShared::ComputePipelineStateShared(gfx::Device device) : device(std::move(device)) {}
gfx::ComputePipelineStateShared::~ComputePipelineStateShared() {
    for (auto& layout : bind_group_layouts) {
        device.shared->raii.raw.destroyDescriptorSetLayout(layout, nullptr, device.shared->raii.dispatcher);
    }

    device.shared->raii.raw.destroyPipelineLayout(pipeline_layout, nullptr, device.shared->raii.dispatcher);
    device.shared->raii.raw.destroyPipeline(pipeline, nullptr, device.shared->raii.dispatcher);
}

gfx::ComputePipelineState::ComputePipelineState() : shared(nullptr) {}
gfx::ComputePipelineState::ComputePipelineState(std::shared_ptr<ComputePipelineStateShared> shared) : shared(std::move(shared)) {}
