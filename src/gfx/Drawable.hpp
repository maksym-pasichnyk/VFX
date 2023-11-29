#pragma once

#include "Texture.hpp"

namespace gfx {
    struct Drawable final {
        rc<Texture>      texture;
        uint32_t         drawableIndex;
        vk::SwapchainKHR swapchain;

        explicit Drawable();
        explicit Drawable(vk::SwapchainKHR swapchain, rc<Texture> texture, uint32_t drawableIndex);
    };
}