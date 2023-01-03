#pragma once

#include "gfx/GFX.hpp"
#include "UIRenderer.hpp"

struct UINodeEditor;

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void screenResized(const vk::Extent2D& size);

private:
    UISize mScreenSize;

    gfx::SharedPtr<UIContext> mUIContext;
    gfx::SharedPtr<UINodeEditor> mNodeEditor;
    gfx::SharedPtr<UIRenderer> mGuiRenderer;

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;
};
