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
        friend Texture;

    private:
        uint32_t mWidth = {};
        uint32_t mHeight = {};
        vk::Format mFormat = {};
        vk::ImageUsageFlags mImageUsageFlags = {};
        vk::ComponentMapping mComponentMapping = {};

    public:
        void setWidth(uint32_t width) {
            mWidth = width;
        }
        void setHeight(uint32_t height) {
            mHeight = height;
        }
        void setFormat(vk::Format format) {
            mFormat = format;
        }
        void setImageUsageFlags(vk::ImageUsageFlags imageUsageFlags) {
            mImageUsageFlags = imageUsageFlags;
        }
        void setComponentMapping(vk::ComponentMapping componentMapping) {
            mComponentMapping = componentMapping;
        }
    };

    struct Texture final : Referencing {
        friend Swapchain;
        friend Device;
        friend DescriptorSet;
        friend CommandBuffer;
        friend RenderCommandEncoder;

    private:
        SharedPtr<Device> mDevice;

        vk::Image mImage = {};
        vk::Format mFormat = {};
        vk::Extent3D mExtent = {};
        vk::ImageView mImageView = {};
        vk::ImageSubresourceRange mImageSubresourceRange = {};

        VmaAllocation mAllocation = {};

    private:
        explicit Texture(SharedPtr<Device> device);
        explicit Texture(SharedPtr<Device> device, const TextureDescription& description);

        ~Texture() override;

    public:
        void replaceRegion(const void* data, uint64_t size);
        void setLabel(const std::string& name);
    };
}