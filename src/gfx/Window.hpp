#pragma once

#include "gfx/Object.hpp"

#include <vulkan/vulkan.hpp>

struct SDL_Window;

namespace gfx {
    struct Window;
    struct Surface;
    struct Swapchain;
    struct Application;

    struct WindowDelegate : Referencing {
        virtual void windowDidResize(const SharedPtr<Window>& sender) {}
        virtual auto windowShouldClose(const SharedPtr<Window>& sender) -> bool {
            return true;
        }
        virtual void windowKeyEvent(const SharedPtr<Window>& sender, int32_t keycode, int32_t scancode, int32_t action, int32_t mods) {}
        virtual void windowMouseEvent(const SharedPtr<Window>& sender, int32_t button, int32_t action, int32_t mods) {}
        virtual void windowCursorEvent(const SharedPtr<Window>& sender, double_t x, double_t y) {}
        virtual void windowMouseEnter(const SharedPtr<Window>& sender) {}
        virtual void windowMouseExit(const SharedPtr<Window>& sender) {}
    };

    struct Window final : Referencing {
        friend Application;

    private:
        SDL_Window* pWindow = {};
        SharedPtr<Surface> mSurface = {};
        SharedPtr<Swapchain> mSwapchain = {};
        SharedPtr<Application> mApplication = {};
        SharedPtr<WindowDelegate> mDelegate = {};

    private:
        Window(SharedPtr<Application> application, int32_t width, int32_t height);
        ~Window() override;

    public:
        void close();
        auto size() -> vk::Extent2D;
        void setTitle(const std::string& title);
        void setDelegate(SharedPtr<WindowDelegate> delegate);
        void setResizable(bool resizable);
        auto swapchain() -> SharedPtr<Swapchain>;
        auto getWindowNumber() -> uint32_t;

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
