#pragma once

#include "Object.hpp"
#include "Graphics.hpp"
#include "UIContext.hpp"

struct UIRenderer : Object {
public:
    explicit UIRenderer(const ManagedShared<gfx::Device>& device);

private:
    void buildFonts();
    void buildShaders();
    void buildBuffers();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(const ManagedShared<gfx::CommandBuffer>& cmd);
    void setScale(float scale);
    void setScreenSize(const Size& size);

private:
    Size                                    mScreenSize             = Size::zero();

    ImDrawList                              mDrawList;
    ImFontAtlas                             mFontAtlas              = {};
    ImGuiContext                            mGuiContext             = {&mFontAtlas};
    ImDrawListSharedData                    mDrawListSharedData     = {};

    ManagedShared<gfx::Device>              device                  = {};
    ManagedShared<gfx::Texture>             font_texture            = {};
    ManagedShared<gfx::Sampler>             font_sampler            = {};
    ManagedShared<gfx::Buffer>              dynamic_buffer          = {};
    vk::DeviceSize                          dynamic_buffer_offset   = {};
    ManagedShared<gfx::RenderPipelineState> render_pipeline_state   = {};
};