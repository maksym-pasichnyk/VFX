#pragma once

#include "Texture.hpp"

namespace gfx {
    struct Drawable final {
        ManagedShared<Texture>  texture;
        uint32_t                drawableIndex;
        vk::SwapchainKHR        swapchain;

        explicit Drawable();
        explicit Drawable(vk::SwapchainKHR swapchain, ManagedShared<Texture> texture, uint32_t drawableIndex);
    };
}