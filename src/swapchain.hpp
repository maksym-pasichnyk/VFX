#pragma once

#include "types.hpp"

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Surface;
    struct Drawable;
    struct Swapchain {
    public:
        Arc<Context> context = {};
        Arc<Surface> surface = {};

        vk::Format pixelFormat = {};
        vk::Extent2D drawableSize = {};
        vk::ColorSpaceKHR colorSpace = {};
        bool displaySyncEnabled = false;

        vk::UniqueFence fence = {};
        vk::UniqueSwapchainKHR handle = {};
        std::vector<Arc<Drawable>> drawables{};

    public:
        explicit Swapchain(const Arc<Context>& context);
        ~Swapchain();

    public:
        auto nextDrawable() -> Drawable*;
        void updateDrawables();

    private:
        void freeDrawables();
    };
}