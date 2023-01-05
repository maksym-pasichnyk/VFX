#include "Window.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Application.hpp"

#include "SDL_video.h"
#include "SDL_vulkan.h"

gfx::Window::Window(SharedPtr<Application> application, int32_t width, int32_t height) : mApplication(std::move(application)) {
    pWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowData(pWindow, "this", this);

    VkSurfaceKHR vkSurface;
    SDL_Vulkan_CreateSurface(pWindow, mApplication->vkInstance, &vkSurface);

    mSurface = TransferPtr(new Surface(mApplication, vkSurface));
    mSwapchain = TransferPtr(new gfx::Swapchain(mSurface));
    mSwapchain->setDrawableSize(drawableSize());
}

gfx::Window::~Window() {
    SDL_DestroyWindow(pWindow);
}

void gfx::Window::close() {
//    SDL_DestroyWindow(pWindow);
}

auto gfx::Window::size() -> vk::Extent2D {
    int width, height;
    SDL_GetWindowSize(pWindow, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

auto gfx::Window::drawableSize() -> vk::Extent2D {
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

auto gfx::Window::swapchain() -> SharedPtr<Swapchain> {
    return mSwapchain;
}

auto gfx::Window::getWindowNumber() -> uint32_t {
    return SDL_GetWindowID(pWindow);
}

auto gfx::Window::native() -> SDL_Window* {
    return pWindow;
}

void gfx::Window::performClose() {
    if (mDelegate) {
        if (mDelegate->windowShouldClose(RetainPtr(this))) {
            close();
        }
    }
}

void gfx::Window::performResize() {
    if (mSwapchain) {
        mSwapchain->mDevice->waitIdle();
        mSwapchain->setDrawableSize(drawableSize());
        mSwapchain->releaseDrawables();
    }

    if (mDelegate) {
        mDelegate->windowDidResize(RetainPtr(this));
    }
}

auto gfx::Window::alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window> {
    return TransferPtr(new Window(std::move(application), width, height));
}