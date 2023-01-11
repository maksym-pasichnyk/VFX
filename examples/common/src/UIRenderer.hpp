#pragma once

#include "Core.hpp"
#include "UIContext.hpp"

struct UIRenderer : gfx::Referencing {
public:
    explicit UIRenderer(sp<gfx::Device> device);

private:
    void buildFonts();
    void buildShaders();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(const sp<gfx::CommandBuffer>& cmd);
    void setScale(float_t scale);
    void setScreenSize(const UISize& size);

private:
    float_t mScale = 1.0F;
    UISize mScreenSize = UISize(0.0F, 0.0F);

    ImDrawList mDrawList;
    ImFontAtlas mFontAtlas = {};
    ImGuiContext mGuiContext = {&mFontAtlas};
    ImDrawListSharedData mDrawListSharedData = {};

    sp<gfx::Device> mDevice;
    sp<gfx::Buffer> mIndexBuffer = {};
    sp<gfx::Buffer> mVertexBuffer = {};
    sp<gfx::Texture> mFontTexture = {};
    sp<gfx::Sampler> mFontSampler = {};
    sp<gfx::DescriptorSet> mDescriptorSet = {};
    sp<gfx::RenderPipelineState> mRenderPipelineState = {};
};