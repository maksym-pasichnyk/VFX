#pragma once

#include "types.hpp"

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Window;
    struct Context;
    struct Drawable;
    struct RenderPass;
    struct CommandBuffer;
    struct Swapchain {
        friend Window;
        friend Context;
        friend Drawable;
        friend CommandBuffer;

    private:
        Context& context;

        vk::SurfaceKHR surface;
        vk::SwapchainKHR handle{};

        Arc<RenderPass> renderPass = {};
        std::vector<Arc<Drawable>> drawables{};

        vk::Format pixelFormat = {};
        vk::Extent2D drawableSize = {};
        vk::ColorSpaceKHR colorSpace = {};
        vk::PresentModeKHR presentMode = {};

    public:
        Swapchain(Context& context, vk::SurfaceKHR surface);
        ~Swapchain();

    private:
        void create_swapchain();
        void destroy_swapchain();
        void create_render_pass();
        void create_drawables();
        void destroy_drawables();

    public:
        void rebuild();
        auto nextDrawable() -> Drawable*;

        auto getPixelFormat() -> vk::Format;
        auto getDrawableSize() -> vk::Extent2D;
        auto getDefaultRenderPass() -> const Arc<RenderPass>&;
    };
}