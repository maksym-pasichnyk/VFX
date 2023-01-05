#pragma once

#include "gfx/GFX.hpp"
#include "UIRenderer.hpp"

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

private:
    void buildShaders();
    void buildBuffers();

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void screenResized(const vk::Extent2D& size);

private:
    gfx::SharedPtr<UIContext> mUIContext;

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::Buffer> vertexBuffer;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;
    gfx::SharedPtr<gfx::DescriptorSet> descriptorSet;
    gfx::SharedPtr<gfx::RenderPipelineState> renderPipelineState;
};
