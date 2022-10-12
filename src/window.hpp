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

    struct Context;
    struct Swapchain;
    struct Window {
    private:
        GLFWwindow* handle;

    public:
        explicit Window(const WindowDescription& description);
        ~Window();

    public:
        auto createSurface(const Arc<Context>& context) -> vk::SurfaceKHR;

        auto getHandle() -> GLFWwindow*;
        auto windowShouldClose() -> bool;
    };
}