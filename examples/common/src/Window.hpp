#pragma once

#include "gfx/Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct View;
    struct Swapchain;
}

struct Window;
struct SDL_Window;
struct Application;

struct WindowDelegate : gfx::Referencing {
    virtual void windowDidResize(const gfx::SharedPtr<Window>& sender) {}
    virtual auto windowShouldClose(const gfx::SharedPtr<Window>& sender) -> bool {
        return true;
    }
};

struct Window final : gfx::Referencing {
    friend Application;

private:
    SDL_Window* pWindow = {};

    gfx::SharedPtr<gfx::View> mView = {};
    gfx::SharedPtr<WindowDelegate> mDelegate = {};

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
    void setDelegate(gfx::SharedPtr<WindowDelegate> delegate);
    void setResizable(bool resizable);
    auto view() -> gfx::SharedPtr<gfx::View>;
    auto getWindowNumber() -> uint32_t;
    auto native() -> SDL_Window*;

public:
    void performClose();
    void performResize();

public:
    static auto alloc(int32_t width, int32_t height) -> gfx::SharedPtr<Window>;
};
