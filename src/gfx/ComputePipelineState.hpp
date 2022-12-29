#pragma once

#include "Object.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Function;
    struct ComputePipelineState final : Referencing<ComputePipelineState> {
        friend Device;

    public:
        SharedPtr<Device> mDevice;
        vk::Pipeline vkPipeline = {};
        vk::PipelineLayout vkPipelineLayout = {};
        std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayouts = {};

    private:
        explicit ComputePipelineState(SharedPtr<Device> device, const SharedPtr<Function>& function);
        ~ComputePipelineState() override;
    };
}