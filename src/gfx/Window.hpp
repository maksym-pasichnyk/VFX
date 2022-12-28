#pragma once

#include "gfx/Object.hpp"

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

namespace gfx {
    struct Application;
    struct WindowDelegate : Referencing<WindowDelegate> {
        virtual void windowDidResize() {}
        virtual void windowShouldClose() {}
        virtual void windowKeyEvent(int32_t keycode, int32_t scancode, int32_t action, int32_t mods) {}
        virtual void windowMouseEvent(int32_t button, int32_t action, int32_t mods) {}
        virtual void windowCursorEvent(double_t x, double_t y) {}
        virtual void windowMouseEnter() {}
        virtual void windowMouseExit() {}
    };

    struct Window final : Referencing<Window> {
        friend Application;

    private:
        GLFWwindow* pWindow = {};
        vk::SurfaceKHR vkSurface = {};
        SharedPtr<Application> mApplication = {};
        SharedPtr<WindowDelegate> mDelegate = {};

    private:
        Window(SharedPtr<Application> application, int32_t width, int32_t height);
        ~Window() override;

    public:
        void close();
        auto shouldClose() -> bool;
        auto size() -> vk::Extent2D;
        auto surface() -> vk::SurfaceKHR;
        void setTitle(const std::string& title);
        void setDelegate(SharedPtr<WindowDelegate> delegate);
        void setResizable(bool resizable);

    public:
        void windowDidResize();
        void windowShouldClose();
        void windowKeyEvent(int32_t keycode, int32_t scancode, int32_t action, int32_t mods);
        void windowMouseEvent(int32_t button, int32_t action, int32_t mods);
        void windowCursorEvent(double_t x, double_t y);
        void windowMouseEnter();
        void windowMouseExit();

    public:
        static auto alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window>;
    };
}
