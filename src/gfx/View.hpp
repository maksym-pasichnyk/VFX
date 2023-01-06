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

    private:
        explicit View(SharedPtr<Swapchain> swapchain);

    public:
        void draw();
        void update(float_t dt);
        auto delegate() -> SharedPtr<ViewDelegate>;
        void setDelegate(SharedPtr<ViewDelegate> delegate);
        auto nextDrawable() -> SharedPtr<Drawable>;
        auto drawableSize() -> vk::Extent2D;

    public:
        void keyUp(SDL_KeyboardEvent* event);
        void keyDown(SDL_KeyboardEvent* event);
        void mouseUp(SDL_MouseButtonEvent* event);
        void mouseDown(SDL_MouseButtonEvent* event);
        void mouseWheel(SDL_MouseWheelEvent* event);
    };
}