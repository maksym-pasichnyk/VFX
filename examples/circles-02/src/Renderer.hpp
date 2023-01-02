#pragma once

#include "gfx/GFX.hpp"
#include "GuiRenderer.hpp"

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void screenResized(const vk::Extent2D& size);

private:
    gfx::SharedPtr<UIContext> mUIContext;
    gfx::SharedPtr<GuiRenderer> mGuiRenderer;

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;
};
