#pragma once

#include "gfx/GFX.hpp"
#include "GuiRenderer.hpp"

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device);
    ~Renderer() override = default;

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);

private:
    void buildShaders();
    void buildBuffers();

private:
    gfx::SharedPtr<GuiRenderer> mGuiRenderer;

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::CommandQueue> mCommandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> mCommandBuffer;
};
