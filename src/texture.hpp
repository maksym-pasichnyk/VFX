#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Texture {
        u32 width = 0;
        u32 height = 0;
        vk::Image image{};
        vk::ImageView view{};
        vk::Sampler sampler{};
        VmaAllocation allocation{};
    };
}