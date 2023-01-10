#pragma once

#include "Object.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Function;
    struct ComputePipelineState final : Referencing {
        friend Device;

    public:
        SharedPtr<Device> mDevice;
        vk::Pipeline mPipeline = {};
        vk::PipelineLayout mPipelineLayout = {};
        std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts = {};

    private:
        explicit ComputePipelineState(SharedPtr<Device> device, const SharedPtr<Function>& function);
        ~ComputePipelineState() override;
    };
}