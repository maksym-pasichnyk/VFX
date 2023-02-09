#pragma once

#include "Object.hpp"
#include "Graphics.hpp"
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
    void setScreenSize(const Size& size);

private:
    float_t mScale = 1.0F;
    Size mScreenSize = Size::zero();

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