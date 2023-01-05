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
        auto drawableSize() -> vk::Extent2D;
        void setTitle(const std::string& title);
        void setDelegate(SharedPtr<WindowDelegate> delegate);
        void setResizable(bool resizable);
        auto swapchain() -> SharedPtr<Swapchain>;
        auto getWindowNumber() -> uint32_t;
        auto native() -> SDL_Window*;

    public:
        void performClose();
        void performResize();

    public:
        static auto alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window>;
    };
}
