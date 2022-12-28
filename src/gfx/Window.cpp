#include "Window.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Application.hpp"

#include "SDL_video.h"
#include "SDL_vulkan.h"

gfx::Window::Window(SharedPtr<Application> application, int32_t width, int32_t height) : mApplication(std::move(application)) {
    pWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowData(pWindow, "this", this);

    SDL_Vulkan_CreateSurface(pWindow, mApplication->vkInstance, reinterpret_cast<VkSurfaceKHR*>(&vkSurface));
}

gfx::Window::~Window() {
    mSwapchain = {};
    mApplication->vkInstance.destroySurfaceKHR(vkSurface, nullptr, mApplication->vkDispatchLoaderDynamic);
}

void gfx::Window::close() {
    SDL_DestroyWindow(pWindow);
}

auto gfx::Window::size() -> vk::Extent2D {
    int width, height;
    SDL_Vulkan_GetDrawableSize(pWindow, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

void gfx::Window::setTitle(const std::string& title) {
    SDL_SetWindowTitle(pWindow, title.c_str());
}

void gfx::Window::setDelegate(SharedPtr<WindowDelegate> delegate) {
    mDelegate = std::move(delegate);
}

void gfx::Window::setResizable(bool resizable) {
    SDL_SetWindowResizable(pWindow, resizable ? SDL_TRUE : SDL_FALSE);
}

void gfx::Window::setSwapchain(SharedPtr<Swapchain> swapchain) {
    mSwapchain = std::move(swapchain);
    mSwapchain->vkSurface = vkSurface;
    mSwapchain->setDrawableSize(size());
}

auto gfx::Window::swapchain() -> SharedPtr<Swapchain> {
    return mSwapchain;
}

auto gfx::Window::getWindowNumber() -> uint32_t {
    return SDL_GetWindowID(pWindow);
}

void gfx::Window::windowDidResize() {
    if (mSwapchain) {
        mSwapchain->mDevice->waitIdle();
        mSwapchain->setDrawableSize(size());
        mSwapchain->releaseDrawables();
    }

    if (mDelegate) {
        mDelegate->windowDidResize(RetainPtr(this));
    }
}

void gfx::Window::windowShouldClose() {
    if (mDelegate) {
        if (mDelegate->windowShouldClose(RetainPtr(this))) {
            close();
        }
    }
}

void gfx::Window::windowKeyEvent(int32_t keycode, int32_t scancode, int32_t action, int32_t mods) {
    if (mDelegate) {
        mDelegate->windowKeyEvent(RetainPtr(this), keycode, scancode, action, mods);
    }
}

void gfx::Window::windowMouseEvent(int32_t button, int32_t action, int32_t mods) {
    if (mDelegate) {
        mDelegate->windowMouseEvent(RetainPtr(this), button, action, mods);
    }
}

void gfx::Window::windowCursorEvent(double_t x, double_t y) {
    if (mDelegate) {
        mDelegate->windowCursorEvent(RetainPtr(this), x, y);
    }
}

void gfx::Window::windowMouseEnter() {
    if (mDelegate) {
        mDelegate->windowMouseEnter(RetainPtr(this));
    }
}

void gfx::Window::windowMouseExit() {
    if (mDelegate) {
        mDelegate->windowMouseExit(RetainPtr(this));
    }
}

auto gfx::Window::alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window> {
    return TransferPtr(new Window(std::move(application), width, height));
}