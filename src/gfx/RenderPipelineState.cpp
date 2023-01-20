#include "RenderPipelineState.hpp"

gfx::RenderPipelineStateShared::RenderPipelineStateShared(gfx::Device device) : device(std::move(device)) {}

gfx::RenderPipelineStateShared::~RenderPipelineStateShared() {
    for (auto& layout : bind_group_layouts) {
        device.handle().destroyDescriptorSetLayout(layout, nullptr, device.dispatcher());
    }
    device.handle().destroyPipelineLayout(pipeline_layout, nullptr, device.dispatcher());
    device.handle().destroyPipeline(pipeline, nullptr, device.dispatcher());
}
