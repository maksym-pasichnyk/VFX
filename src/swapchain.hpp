#pragma once

#include "context.hpp"
#include "drawable.hpp"

namespace vfx {
    struct Swapchain {
    public:
        Context& context;

        vk::SurfaceKHR surface{};
        vk::SwapchainKHR swapchain{};
        std::vector<Arc<Drawable>> drawables{};

        vk::Format pixelFormat = {};
        vk::Extent2D drawableSize = {};
        vk::ColorSpaceKHR colorSpace = {};
        vk::PresentModeKHR presentMode = {};
        Arc<vfx::RenderPass> renderPass = {};

    public:
        Swapchain(Context& context, Display& display);
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
    };
}