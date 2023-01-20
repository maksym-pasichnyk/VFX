#include "ComputePipelineState.hpp"

gfx::ComputePipelineStateShared::ComputePipelineStateShared(gfx::Device device) : device(std::move(device)) {}
gfx::ComputePipelineStateShared::~ComputePipelineStateShared() {
    for (auto& layout : bind_group_layouts) {
        device.handle().destroyDescriptorSetLayout(layout, nullptr, device.dispatcher());
    }

    device.handle().destroyPipelineLayout(pipeline_layout, nullptr, device.dispatcher());
    device.handle().destroyPipeline(pipeline, nullptr, device.dispatcher());
}

gfx::ComputePipelineState::ComputePipelineState() : shared(nullptr) {}
gfx::ComputePipelineState::ComputePipelineState(std::shared_ptr<ComputePipelineStateShared> shared) : shared(std::move(shared)) {}
