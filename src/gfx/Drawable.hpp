#pragma once

#include "Texture.hpp"

namespace gfx {
    struct Drawable final : ManagedObject {
        rc<Texture>      texture;
        uint32_t         drawableIndex;
        vk::SwapchainKHR swapchain;

        explicit Drawable(vk::SwapchainKHR swapchain, rc<Texture> texture, uint32_t drawableIndex);
    };
}