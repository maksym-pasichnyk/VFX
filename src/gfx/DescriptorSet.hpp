#pragma once

#include "Object.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Buffer;
    struct Device;
    struct Texture;
    struct Sampler;
    struct CommandBuffer;
    struct DescriptorSet final : Referencing {
        friend Device;
        friend CommandBuffer;

    private:
        SharedPtr<Device> mDevice = {};
        vk::DescriptorSet vkDescriptorSet = {};
        vk::DescriptorPool vkDescriptorPool = {};

    private:
        explicit DescriptorSet(SharedPtr<Device> device);
        ~DescriptorSet() override;

    public:
        void setBuffer(const SharedPtr<Buffer>& buffer, uint64_t offset, uint32_t binding);
        void setStorageBuffer(const SharedPtr<Buffer>& buffer, uint64_t offset, uint32_t binding);
        void setTexture(const SharedPtr<Texture>& texture, uint32_t binding);
        void setSampler(const SharedPtr<Sampler>& sampler, uint32_t binding);
        void setStorageImage(const SharedPtr<Texture>& texture, uint32_t binding);
        void setLabel(const std::string& name);
    };
}