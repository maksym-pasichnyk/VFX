#pragma once

#include "gfx/GFX.hpp"
#include "UIRenderer.hpp"

struct GraphView;

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

public:
    void update(float_t dt);
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
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
