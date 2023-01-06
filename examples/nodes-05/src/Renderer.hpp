#pragma once

#include "gfx/GFX.hpp"
#include "UIRenderer.hpp"

#include <SDL_events.h>

struct GraphView;
struct Renderer : gfx::ViewDelegate {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

public:
    void update(float_t dt) override;
    void draw(const gfx::SharedPtr<gfx::View>& view) override;

public:
    void keyUp(SDL_KeyboardEvent* event) override;
    void keyDown(SDL_KeyboardEvent* event) override;
    void mouseUp(SDL_MouseButtonEvent* event) override;
    void mouseDown(SDL_MouseButtonEvent* event) override;
    void mouseWheel(SDL_MouseWheelEvent* event) override;

    void screenResized(const vk::Extent2D& size);

private:
    UISize mScreenSize;

    float_t mAverage = {};
    float_t mAccumulate[60] = {};
    float_t mAccumulateTotal = {};
    int32_t mAccumulateCount = {};
    int32_t mAccumulateIndex = {};

    gfx::SharedPtr<GraphView> mGraphView;
    gfx::SharedPtr<UIContext> mUIContext;
    gfx::SharedPtr<UIRenderer> mGuiRenderer;

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;
};
