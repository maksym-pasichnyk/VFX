#pragma once

#include "types.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Layer;
    struct Device;
    struct Drawable;
    struct CommandQueue;
    struct ResourceGroup;

    struct TextureDescription {
        vk::Format format = vk::Format::eUndefined;
        u32 width = 0;
        u32 height = 0;
        vk::ImageUsageFlags usage = {};
        vk::ComponentMapping components = {};
    };

    struct Sampler {
    public:
        Device* device{};
        vk::Sampler handle{};

    public:
        Sampler();
        ~Sampler();

    public:
        void setLabel(const std::string& name);
    };

    struct Texture {
    public:
        Device* device{};
        vk::Extent2D size{};
        vk::Format format{};
        vk::Image image{};
        vk::ImageView view{};
        VmaAllocation allocation{};
        vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eNone;

    public:
        Texture();
        ~Texture();

    public:
        void update(const void* data, u64 _size);
        void setLabel(const std::string& name);
    };
}