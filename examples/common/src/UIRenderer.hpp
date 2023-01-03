#pragma once

#include "gfx/GFX.hpp"
#include "NotSwiftUI/View.hpp"
#include "NotSwiftUI/UIContext.hpp"

struct UIRenderer : gfx::Referencing {
public:
    explicit UIRenderer(gfx::SharedPtr<gfx::Device> device);

private:
    void buildFonts();
    void buildShaders();

public:
    void resetForNewFrame();
    auto drawList() -> ImDrawList*;
    void draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd);
    void setScreenSize(const UISize& size);

private:
    UISize mScreenSize = UISize(0.0F, 0.0F);

    ImDrawList mDrawList;
    ImFontAtlas mFontAtlas = {};
    ImDrawListSharedData mDrawListSharedData = {};

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Buffer> mIndexBuffer = {};
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer = {};
    gfx::SharedPtr<gfx::Texture> mFontTexture = {};
    gfx::SharedPtr<gfx::Sampler> mFontSampler = {};
    gfx::SharedPtr<gfx::DescriptorSet> mDescriptorSet = {};
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState = {};
};