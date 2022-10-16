#pragma once

#include "types.hpp"

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Surface;
    struct SwapchainDescription {
        Arc<Context> context{};
        Arc<Surface> surface{};

        vk::ColorSpaceKHR  colorSpace = {};
        vk::Format         pixelFormat = {};
        vk::PresentModeKHR presentMode = {};
    };

    struct Drawable;
    struct RenderPass;
    struct CommandBuffer;
    struct Swapchain {
        friend Context;
        friend Drawable;
        friend CommandBuffer;

        explicit Swapchain(const SwapchainDescription& description);
        ~Swapchain();

    private:
        Arc<Context> context{};
        Arc<Surface> surface{};

        vk::Format pixelFormat = {};
        vk::Extent2D drawableSize = {};
        vk::ColorSpaceKHR colorSpace = {};
        vk::PresentModeKHR presentMode = {};

        vk::SwapchainKHR handle{};
        std::vector<Arc<Drawable>> drawables{};

    private:
        void makeGpuObjects();
        void freeGpuObjects();

    public:
        void makeDrawables();
        void freeDrawables();

    public:
        auto nextDrawable() -> Drawable*;
        auto getPixelFormat() -> vk::Format;
        auto getDrawableSize() -> vk::Extent2D;
    };
}