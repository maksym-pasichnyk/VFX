#pragma once

#include "gfx/GFX.hpp"
#include "GuiRenderer.hpp"

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device);
    ~Renderer() override = default;

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void setScreenSize(const vk::Extent2D& size);

private:
    gfx::SharedPtr<UIContext> mUIContext;
    gfx::SharedPtr<GuiRenderer> mGuiRenderer;

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::CommandQueue> mCommandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> mCommandBuffer;
};
