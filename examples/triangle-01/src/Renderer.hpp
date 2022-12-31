#pragma once

#include "gfx/GFX.hpp"
#include "GuiRenderer.hpp"

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device);
    ~Renderer() override = default;

private:
    void buildShaders();
    void buildBuffers();

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void setScreenSize(const vk::Extent2D& size);

private:
    gfx::SharedPtr<UIContext> mUIContext;

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer;
    gfx::SharedPtr<gfx::CommandQueue> mCommandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> mCommandBuffer;
    gfx::SharedPtr<gfx::DescriptorSet> mDescriptorSet;
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState;
};
