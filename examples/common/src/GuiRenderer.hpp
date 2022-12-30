#pragma once

#include "gfx/GFX.hpp"
#include "NotSwiftUI/UIContext.hpp"

struct GuiRenderer : gfx::Referencing {
public:
    explicit GuiRenderer(gfx::SharedPtr<gfx::Device> device);
    ~GuiRenderer() override;

private:
    void buildGui();
    void buildShaders();

public:
    void draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd);

private:
    gfx::SharedPtr<UIContext> mUIContext;

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Buffer> mIndexBuffer;
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer;

    gfx::SharedPtr<gfx::Texture> mTexture;
    gfx::SharedPtr<gfx::Sampler> mTextureSampler;

    gfx::SharedPtr<gfx::DescriptorSet> mDescriptorSet;
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState;
};