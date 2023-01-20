#pragma once

#include "Texture.hpp"

namespace gfx {
    struct Drawable final {
        Texture texture;
        uint32_t drawableIndex;
        vk::SwapchainKHR swapchain;

        explicit Drawable() : swapchain(nullptr), texture(), drawableIndex(-1) {}
        explicit Drawable(vk::SwapchainKHR swapchain, Texture texture, uint32_t drawableIndex);
    };

}