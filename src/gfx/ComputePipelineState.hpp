#pragma once

#include "Device.hpp"

namespace gfx {
    struct Device;
    struct ComputePipelineState : ManagedObject<ComputePipelineState> {
        ManagedShared<Device>                   device;
        vk::Pipeline                            pipeline;
        vk::PipelineLayout                      pipeline_layout;
        std::vector<vk::DescriptorSetLayout>    descriptor_set_layouts;

        explicit ComputePipelineState(ManagedShared<Device> device);
        ~ComputePipelineState() override;
    };
}