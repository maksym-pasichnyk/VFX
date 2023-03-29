#pragma once

#include "Device.hpp"

namespace gfx {
    struct TextureSettings {
        uint32_t                width   = {};
        uint32_t                height  = {};
        vk::Format              format  = {};
        vk::ImageUsageFlags     usage   = {};
        vk::ComponentMapping    mapping = {};
    };

    struct TextureShared {
        Device                      device;
        vk::Image                   image;
        vk::Format                  format;
        vk::Extent3D                extent;
        vk::ImageView               image_view;
        vk::ImageSubresourceRange   subresource;
        VmaAllocation               allocation;

        explicit TextureShared(Device device);
        explicit TextureShared(Device device, vk::Image image, vk::Format format, vk::Extent3D extent, vk::ImageView image_view, vk::ImageSubresourceRange subresource, VmaAllocation allocation);

        ~TextureShared();
    };

    struct Texture final {
        std::shared_ptr<TextureShared> shared;

        explicit Texture();
        explicit Texture(std::shared_ptr<TextureShared> shared);

        void replaceRegion(const void* data, uint64_t size);
        void setLabel(const std::string& name);
    };
}