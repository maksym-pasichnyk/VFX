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

    struct Application {
    public:
        Application();
        virtual ~Application();

    public:
        virtual void pollEvents() final;
    };

    struct WindowDelegate {
        virtual void windowDidResize() = 0;
    };

    struct Context;
    struct Swapchain;
    struct Window {
        friend Context;
        friend Swapchain;

    private:
        GLFWwindow* handle;

    public:
        WindowDelegate* delegate{};

    public:
        explicit Window(const WindowDescription& description);
        ~Window();

    private:
        void windowDidResize();
        auto createSurface(const Arc<Context>& context) -> vk::SurfaceKHR;

    public:
        auto getHandle() -> GLFWwindow*;
        auto windowShouldClose() -> bool;
    };
}