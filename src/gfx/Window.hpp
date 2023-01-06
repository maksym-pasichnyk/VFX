#pragma once

#include "gfx/Object.hpp"

#include <vulkan/vulkan.hpp>

struct SDL_Window;

namespace gfx {
    struct View;
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

        SharedPtr<View> mView = {};
        SharedPtr<Surface> mSurface = {};
        SharedPtr<Swapchain> mSwapchain = {};
        SharedPtr<WindowDelegate> mDelegate = {};

        bool mShouldClose = {};

    private:
        explicit Window(int32_t width, int32_t height);

    private:
        void _destroy();

    public:
        void close();
        auto size() -> vk::Extent2D;
        auto drawableSize() -> vk::Extent2D;
        void setTitle(const std::string& title);
        void setDelegate(SharedPtr<WindowDelegate> delegate);
        void setResizable(bool resizable);
        auto view() -> SharedPtr<View>;
        auto swapchain() -> SharedPtr<Swapchain>;
        auto getWindowNumber() -> uint32_t;
        auto native() -> SDL_Window*;

    public:
        void performClose();
        void performResize();

    public:
        static auto alloc(int32_t width, int32_t height) -> SharedPtr<Window>;
    };
}
