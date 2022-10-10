#pragma once

#include <types.hpp>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Texture;
    struct Swapchain;
    struct Drawable {
        u32 index{};
        Swapchain* layer{};
        Box<Texture> texture{};
        vk::Framebuffer framebuffer{};
    };
}