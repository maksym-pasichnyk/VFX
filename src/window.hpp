#pragma once

#include "types.hpp"

#include <string>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace vfx {
    struct WindowDescription {
        std::string title = {};
        u32 width = 0;
        u32 height = 0;
        bool resizable = false;
    };

    struct WindowController final {
    public:
        WindowController();
        ~WindowController();

    public:
        virtual void pollEvents() final;
    };

    struct Context;
    struct Swapchain;
    struct Window {
    private:
        Context& context;
        GLFWwindow* handle;
        vk::SurfaceKHR surface{};
        Arc<Swapchain> swapchain{};
        Arc<WindowController> controller;

    public:
        Window(Arc<WindowController> _controller, Context& context, const WindowDescription& description);
        ~Window();

    public:
        auto getHandle() -> GLFWwindow*;
        auto getSwapchain() -> Arc<Swapchain>;
        auto windowShouldClose() -> bool;
    };
}