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
        std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayoutArray = {};

    private:
        explicit ComputePipelineState(SharedPtr<Device> device, SharedPtr<Function> function);
        ~ComputePipelineState() override;
    };
}