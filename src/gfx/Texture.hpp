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

    struct Texture : ManagedObject<Texture> {
        ManagedShared<Device>       device;
        vk::Image                   image;
        vk::Format                  format;
        vk::Extent3D                extent;
        vk::ImageView               image_view;
        vk::ImageSubresourceRange   subresource;
        VmaAllocation               allocation;

        explicit Texture(ManagedShared<Device> device);
        explicit Texture(ManagedShared<Device> device, vk::Image image, vk::Format format, vk::Extent3D extent, vk::ImageView image_view, vk::ImageSubresourceRange subresource, VmaAllocation allocation);
        ~Texture() override;

        void replaceRegion(const void* data, uint64_t size);
        void setLabel(const std::string& name);
    };
}