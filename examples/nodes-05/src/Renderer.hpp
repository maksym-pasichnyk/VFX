#pragma once

#include "gfx/GFX.hpp"
#include "UIRenderer.hpp"

#include <SDL_events.h>

struct GraphView;
struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

public:
    void update();
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);

public:
    void keyUp(SDL_KeyboardEvent* event);
    void keyDown(SDL_KeyboardEvent* event);
    void mouseUp(SDL_MouseButtonEvent* event);
    void mouseDown(SDL_MouseButtonEvent* event);
    void mouseWheel(SDL_MouseWheelEvent* event);
    void screenResized(const vk::Extent2D& size);

private:
    UISize mScreenSize;

    gfx::SharedPtr<GraphView> mGraphView;
    gfx::SharedPtr<UIContext> mUIContext;
    gfx::SharedPtr<UIRenderer> mGuiRenderer;

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;
};
