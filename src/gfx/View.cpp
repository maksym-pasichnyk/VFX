#include "View.hpp"
#include "Drawable.hpp"
#include "Swapchain.hpp"

gfx::View::View(gfx::SharedPtr<gfx::Swapchain> swapchain) : mSwapchain(std::move(swapchain)) {}

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

void gfx::View::setDelegate(SharedPtr<ViewDelegate> delegate) {
    mDelegate = std::move(delegate);
}

auto gfx::View::nextDrawable() -> SharedPtr<Drawable> {
    return mSwapchain->nextDrawable();
}

auto gfx::View::drawableSize() -> vk::Extent2D {
    return mSwapchain->drawableSize();
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