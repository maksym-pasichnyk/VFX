#include "Window.hpp"
#include "Application.hpp"

#include "gfx/View.hpp"
#include "gfx/Device.hpp"
#include "gfx/Swapchain.hpp"

#include "SDL_video.h"
#include "SDL_vulkan.h"

Window::Window(int32_t width, int32_t height) {
    auto application = Application::sharedApplication();

    pWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowData(pWindow, "this", this);

    VkSurfaceKHR surface;
    SDL_Vulkan_CreateSurface(pWindow, application->context->vkInstance, &surface);

    auto swapchain = TransferPtr(new gfx::Swapchain(application->context, surface));

    mView = TransferPtr(new gfx::View());
    mView->setSwapchain(swapchain);
    mView->setDrawableSize(drawableSize());

    application->mWindows.emplace_back(RetainPtr(this));
}

void Window::_destroy() {
    mView = {};
    SDL_DestroyWindow(pWindow);
}

void Window::close() {
    mShouldClose = true;
}

auto Window::size() -> vk::Extent2D {
    int width, height;
    SDL_GetWindowSize(pWindow, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

auto Window::drawableSize() -> vk::Extent2D {
    int width, height;
    SDL_Vulkan_GetDrawableSize(pWindow, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

void Window::setTitle(const std::string& title) {
    SDL_SetWindowTitle(pWindow, title.c_str());
}

void Window::setDelegate(gfx::SharedPtr<WindowDelegate> delegate) {
    mDelegate = std::move(delegate);
}

void Window::setResizable(bool resizable) {
    SDL_SetWindowResizable(pWindow, resizable ? SDL_TRUE : SDL_FALSE);
}

auto Window::view() -> gfx::SharedPtr<gfx::View> {
    return mView;
}

auto Window::getWindowNumber() -> uint32_t {
    return SDL_GetWindowID(pWindow);
}

auto Window::native() -> SDL_Window* {
    return pWindow;
}

void Window::performClose() {
    bool flag = true;
    if (mDelegate) {
        flag = mDelegate->windowShouldClose(RetainPtr(this));
    }
    if (flag) {
        close();
    }
}

void Window::performResize() {
    mView->setDrawableSize(drawableSize());
    mView->releaseDrawables();

    if (mDelegate) {
        mDelegate->windowDidResize(RetainPtr(this));
    }
}

auto Window::alloc(int32_t width, int32_t height) -> gfx::SharedPtr<Window> {
    return TransferPtr(new Window(width, height));
}