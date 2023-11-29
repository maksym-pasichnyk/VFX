#pragma once

#include "Device.hpp"

namespace gfx {
    struct TextureDescription {
        uint32_t                width   = {};
        uint32_t                height  = {};
        vk::Format              format  = {};
        vk::ImageUsageFlags     usage   = {};
        vk::ComponentMapping    mapping = {};
    };

    struct Texture : public ManagedObject {
        rc<Device>                  device;
        vk::Image                   image;
        vk::Format                  format;
        vk::Extent3D                extent;
        vk::ImageView               image_view;
        vk::ImageSubresourceRange   subresource;
        VmaAllocation               allocation;

        explicit Texture(rc<Device> device, vk::Image image, vk::Format format, vk::Extent3D extent, vk::ImageView image_view, vk::ImageSubresourceRange subresource, VmaAllocation allocation);
        ~Texture() override;

        void replaceRegion(this Texture& self, const void* data, uint64_t size);
        void setLabel(this Texture& self, std::string const& name);
    };
}