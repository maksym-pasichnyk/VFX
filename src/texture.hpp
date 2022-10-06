#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct TextureDescription {
        vk::Format format = vk::Format::eUndefined;
        u32 width = 0;
        u32 height = 0;
        vk::ImageUsageFlags usage = {};
        vk::ImageAspectFlags aspect = {};
    };

    struct Texture {
        u32 width = 0;
        u32 height = 0;
        vk::Format format{};
        vk::Image image{};
        vk::ImageView view{};
//        vk::Sampler sampler{};
        VmaAllocation allocation{};
    };
}