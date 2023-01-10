#pragma once

#include "Object.hpp"
#include "Swapchain.hpp"

#include <SDL_events.h>

namespace gfx {
    struct View;
    struct Window;
    struct ViewDelegate : Referencing {
        virtual void update(float_t dt) {}
        virtual void draw(const SharedPtr<View>& view) = 0;
        virtual void keyUp(SDL_KeyboardEvent* event) {}
        virtual void keyDown(SDL_KeyboardEvent* event) {}
        virtual void mouseUp(SDL_MouseButtonEvent* event) {}
        virtual void mouseDown(SDL_MouseButtonEvent* event) {}
        virtual void mouseWheel(SDL_MouseWheelEvent* event) {}
    };

    struct View : Referencing {
        friend Window;

    private:
        SharedPtr<Swapchain> mSwapchain;
        SharedPtr<ViewDelegate> mDelegate;

    public:
        void draw();
        void update(float_t dt);
        auto delegate() -> SharedPtr<ViewDelegate>;
        void setSwapchain(SharedPtr<Swapchain> swapchain);
        void setDelegate(SharedPtr<ViewDelegate> delegate);

    public:
        void releaseDrawables();
        void setDevice(SharedPtr<Device> device);
        auto device() -> SharedPtr<Device>;
        auto nextDrawable() -> SharedPtr<Drawable>;
        auto drawableSize() -> vk::Extent2D;
        void setDrawableSize(const vk::Extent2D& drawableSize);
        auto pixelFormat() -> vk::Format;
        void setPixelFormat(vk::Format format);
        auto colorSpace() -> vk::ColorSpaceKHR;
        void setColorSpace(vk::ColorSpaceKHR colorSpace);
        auto displaySyncEnabled() -> bool;
        void setDisplaySyncEnabled(bool displaySyncEnabled);
        auto maximumDrawableCount() -> uint32_t;
        void setMaximumDrawableCount(uint32_t maximumDrawableCount);

    public:
        void keyUp(SDL_KeyboardEvent* event);
        void keyDown(SDL_KeyboardEvent* event);
        void mouseUp(SDL_MouseButtonEvent* event);
        void mouseDown(SDL_MouseButtonEvent* event);
        void mouseWheel(SDL_MouseWheelEvent* event);
    };
}