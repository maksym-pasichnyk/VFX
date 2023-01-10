#include "View.hpp"
#include "Device.hpp"
#include "Drawable.hpp"
#include "Swapchain.hpp"

void gfx::View::draw() {
    if (mDelegate) {
        mDelegate->draw(RetainPtr(this));
    }
}

void gfx::View::update(float_t dt) {
    if (mDelegate) {
        mDelegate->update(dt);
    }
}

auto gfx::View::delegate() -> SharedPtr<ViewDelegate> {
    return mDelegate;
}

void gfx::View::setSwapchain(SharedPtr<Swapchain> swapchain) {
    mSwapchain = std::move(swapchain);
}

void gfx::View::setDelegate(SharedPtr<ViewDelegate> delegate) {
    mDelegate = std::move(delegate);
}

void gfx::View::releaseDrawables() {
    return mSwapchain->releaseDrawables();
}
void gfx::View::setDevice(SharedPtr<Device> device) {
    return mSwapchain->setDevice(std::move(device));
}
auto gfx::View::device() -> SharedPtr<Device> {
    return mSwapchain->device();
}
auto gfx::View::nextDrawable() -> SharedPtr<Drawable> {
    return mSwapchain->nextDrawable();
}
auto gfx::View::drawableSize() -> vk::Extent2D {
    return mSwapchain->drawableSize();
}
void gfx::View::setDrawableSize(const vk::Extent2D& drawableSize) {
    return mSwapchain->setDrawableSize(drawableSize);
}
auto gfx::View::pixelFormat() -> vk::Format {
    return mSwapchain->pixelFormat();
}
void gfx::View::setPixelFormat(vk::Format format) {
    return mSwapchain->setPixelFormat(format);
}
auto gfx::View::colorSpace() -> vk::ColorSpaceKHR {
    return mSwapchain->colorSpace();
}
void gfx::View::setColorSpace(vk::ColorSpaceKHR colorSpace) {
    return mSwapchain->setColorSpace(colorSpace);
}
auto gfx::View::displaySyncEnabled() -> bool {
    return mSwapchain->displaySyncEnabled();
}
void gfx::View::setDisplaySyncEnabled(bool displaySyncEnabled) {
    return mSwapchain->setDisplaySyncEnabled(displaySyncEnabled);
}
auto gfx::View::maximumDrawableCount() -> uint32_t {
    return mSwapchain->maximumDrawableCount();
}
void gfx::View::setMaximumDrawableCount(uint32_t maximumDrawableCount) {
    return mSwapchain->setMaximumDrawableCount(maximumDrawableCount);
}

void gfx::View::keyUp(SDL_KeyboardEvent* event) {
    if (mDelegate) {
        mDelegate->keyUp(event);
    }
}

void gfx::View::keyDown(SDL_KeyboardEvent* event) {
    if (mDelegate) {
        mDelegate->keyDown(event);
    }
}

void gfx::View::mouseUp(SDL_MouseButtonEvent* event) {
    if (mDelegate) {
        mDelegate->mouseUp(event);
    }
}

void gfx::View::mouseDown(SDL_MouseButtonEvent* event) {
    if (mDelegate) {
        mDelegate->mouseDown(event);
    }
}

void gfx::View::mouseWheel(SDL_MouseWheelEvent* event) {
    if (mDelegate) {
        mDelegate->mouseWheel(event);
    }
}