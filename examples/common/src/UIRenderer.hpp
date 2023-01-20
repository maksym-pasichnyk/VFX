#pragma once

#include "Core.hpp"
#include "UIContext.hpp"

struct UIRenderer : Object {
public:
    explicit UIRenderer(gfx::Device device);

private:
    void buildFonts();
    void buildShaders();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(gfx::CommandBuffer cmd);
    void setScale(float_t scale);
    void setScreenSize(const UISize& size);

private:
    float_t mScale = 1.0F;
    UISize mScreenSize = UISize(0.0F, 0.0F);

    ImDrawList mDrawList;
    ImFontAtlas mFontAtlas = {};
    ImGuiContext mGuiContext = {&mFontAtlas};
    ImDrawListSharedData mDrawListSharedData = {};

    gfx::Device mDevice;
    gfx::Buffer mIndexBuffer;
    gfx::Buffer mVertexBuffer;
    gfx::Texture mFontTexture;
    gfx::Sampler mFontSampler;
    gfx::DescriptorSet mDescriptorSet;
    gfx::RenderPipelineState mRenderPipelineState;
};