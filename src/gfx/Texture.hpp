#pragma once

#include "Object.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Swapchain;
    struct Device;
    struct Drawable;
    struct CommandQueue;
    struct CommandBuffer;
    struct DescriptorSet;
    struct RenderCommandEncoder;

    struct TextureDescription {
        vk::Format format = vk::Format::eUndefined;
        uint32_t width = 0;
        uint32_t height = 0;
        vk::ImageUsageFlags usage = {};
        vk::ComponentMapping components = {};
    };

    struct Texture final : Referencing {
        friend Swapchain;
        friend Device;
        friend DescriptorSet;
        friend CommandBuffer;
        friend RenderCommandEncoder;

    private:
        SharedPtr<Device> mDevice;

        vk::Image vkImage = {};
        vk::Format vkFormat = {};
        vk::Extent3D vkExtent = {};
        vk::ImageView vkImageView = {};
        vk::ImageSubresourceRange vkImageSubresourceRange = {};

        VmaAllocation vmaAllocation = {};

    private:
        explicit Texture(SharedPtr<Device> device);
        explicit Texture(SharedPtr<Device> device, const TextureDescription& description);

        ~Texture() override;

    public:
        void replaceRegion(const void* data, uint64_t size);
        void setLabel(const std::string& name);
    };
}