#pragma once

#include "Device.hpp"
#include "Surface.hpp"

namespace gfx {
    struct Drawable;

    struct SwapchainShared {
        Device device;
        Surface surface;
        vk::SwapchainKHR raw;
        std::vector<Drawable> drawables;

        explicit SwapchainShared(Device device, Surface surface, vk::SwapchainKHR raw, std::vector<Drawable> drawables);
        ~SwapchainShared();
    };

    struct Swapchain final {
        std::shared_ptr<SwapchainShared> shared;

        explicit Swapchain() : shared(nullptr) {}
        explicit Swapchain(std::shared_ptr<SwapchainShared> shared) : shared(std::move(shared)) {}

        auto nextDrawable() -> Drawable;
        auto drawableSize() -> vk::Extent2D;
    };
}