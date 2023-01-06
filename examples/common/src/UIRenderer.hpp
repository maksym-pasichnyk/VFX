#pragma once

#include "gfx/GFX.hpp"
#include "UIContext.hpp"

struct UIRenderer : gfx::Referencing {
public:
    explicit UIRenderer(gfx::SharedPtr<gfx::Device> device);

private:
    void buildFonts();
    void buildShaders();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd);
    void setScale(float_t scale);
    void setScreenSize(const UISize& size);

private:
    float_t mScale = 1.0F;
    UISize mScreenSize = UISize(0.0F, 0.0F);

    ImDrawList mDrawList;
    ImFontAtlas mFontAtlas = {};
    ImGuiContext mGuiContext = {&mFontAtlas};
    ImDrawListSharedData mDrawListSharedData = {};

    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Buffer> mIndexBuffer = {};
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer = {};
    gfx::SharedPtr<gfx::Texture> mFontTexture = {};
    gfx::SharedPtr<gfx::Sampler> mFontSampler = {};
    gfx::SharedPtr<gfx::DescriptorSet> mDescriptorSet = {};
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState = {};
};