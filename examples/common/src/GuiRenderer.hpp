#pragma once

#include "gfx/GFX.hpp"
#include "NotSwiftUI/View.hpp"
#include "NotSwiftUI/UIContext.hpp"

struct GuiRenderer : gfx::Referencing {
public:
    explicit GuiRenderer(gfx::SharedPtr<gfx::Device> device);

private:
    void buildShaders();

public:
    void draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd);
    void setScreenSize(const vk::Extent2D& size);

private:
    UISize mScreenSize = UISize(0.0F, 0.0F);

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Buffer> mIndexBuffer;
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer;

    gfx::SharedPtr<gfx::DescriptorSet> mDescriptorSet;
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState;
};