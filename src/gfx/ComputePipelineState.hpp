#pragma once

#include "Device.hpp"

namespace gfx {
    struct Device;

    struct ComputePipelineStateShared {
        Device device;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;
        std::vector<vk::DescriptorSetLayout> bind_group_layouts;

        explicit ComputePipelineStateShared(Device device);
        ~ComputePipelineStateShared();
    };

    struct ComputePipelineState final {
        std::shared_ptr<ComputePipelineStateShared> shared;

        explicit ComputePipelineState();
        explicit ComputePipelineState(std::shared_ptr<ComputePipelineStateShared> shared);
    };
}