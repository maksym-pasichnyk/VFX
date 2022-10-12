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
        Arc<Window> window;
        Arc<Context> context;

        vk::SurfaceKHR surface;
        vk::SwapchainKHR handle{};

        Arc<RenderPass> renderPass = {};
        std::vector<Arc<Drawable>> drawables{};

        vk::Format pixelFormat = {};
        vk::Extent2D drawableSize = {};
        vk::ColorSpaceKHR colorSpace = {};
        vk::PresentModeKHR presentMode = {};

    public:
        Swapchain(const Arc<Context>& context, const Arc<Window>& window);
        ~Swapchain();

    private:
        void create_render_pass();

    public:
        void makeGpuObjects();
        void freeGpuObjects();

        auto nextDrawable() -> Drawable*;

        auto getPixelFormat() -> vk::Format;
        auto getDrawableSize() -> vk::Extent2D;
        auto getDefaultRenderPass() -> const Arc<RenderPass>&;
    };
}