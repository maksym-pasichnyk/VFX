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
    void buildBuffers();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(gfx::CommandBuffer cmd);
    void setScale(float scale);
    void setScreenSize(const Size& size);

private:
    Size                        mScreenSize         = Size::zero();

    ImDrawList                  mDrawList;
    ImFontAtlas                 mFontAtlas          = {};
    ImGuiContext                mGuiContext         = {&mFontAtlas};
    ImDrawListSharedData        mDrawListSharedData = {};

    vk::DeviceSize              dynamic_buffer_offset      = {};

    gfx::Device                 mDevice;
    gfx::Buffer                 dynamic_buffer;
    gfx::Texture                mFontTexture;
    gfx::Sampler                mFontSampler;
    gfx::RenderPipelineState    render_pipeline_state;
};