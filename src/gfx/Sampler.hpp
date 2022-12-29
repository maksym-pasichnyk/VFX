#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct CommandBuffer;
    struct DescriptorSet;
    struct RenderCommandEncoder;

    struct Sampler final : Referencing {
        friend Device;
        friend CommandBuffer;
        friend DescriptorSet;
        friend RenderCommandEncoder;

    private:
        SharedPtr<Device> mDevice;
        vk::Sampler vkSampler;

    private:
        explicit Sampler(SharedPtr<Device> device, const vk::SamplerCreateInfo& info);
        ~Sampler() override;

    public:
        void setLabel(const std::string& name);
    };
}