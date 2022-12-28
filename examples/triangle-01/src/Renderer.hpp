#pragma once

#include "gfx/GFX.hpp"

struct Renderer : gfx::Referencing<Renderer> {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device);
    ~Renderer() override = default;

public:
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);

private:
    void buildShaders();
    void buildBuffers();

private:
    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer;
    gfx::SharedPtr<gfx::CommandQueue> mCommandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> mCommandBuffer;
    gfx::SharedPtr<gfx::DescriptorSet> mDescriptorSet;
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState;
};
