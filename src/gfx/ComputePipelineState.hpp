#pragma once

#include "Device.hpp"

namespace gfx {
    struct Device;
    struct ComputePipelineState : public ManagedObject {
        rc<Device>                              device;
        vk::Pipeline                            pipeline;
        vk::PipelineLayout                      pipeline_layout;
        std::vector<vk::DescriptorSetLayout>    descriptor_set_layouts;

        explicit ComputePipelineState(rc<Device> device);
        ~ComputePipelineState() override;
    };
}