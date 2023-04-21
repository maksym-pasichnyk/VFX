#pragma once

#include "Object.hpp"
#include "Graphics.hpp"
#include "Canvas.hpp"

struct ImGuiBackend : Object {
public:
    explicit ImGuiBackend(const ManagedShared<gfx::Device>& device);

private:
    void buildFonts();
    void buildShaders();
    void buildBuffers();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(const ManagedShared<gfx::RenderCommandEncoder>& encoder);
    void setScreenSize(const Size& size);

private:
    Size                                    screen_size             = Size::zero();

    ImFontAtlas                             im_font_atlas           = {};
    ImGuiContext                            im_gui_context          = {&im_font_atlas};
    ImDrawListSharedData                    im_shared_data          = {};
    ImDrawList                              im_draw_list            = {&im_shared_data};

    ManagedShared<gfx::Device>              device                  = {};
    ManagedShared<gfx::Texture>             font_texture            = {};
    ManagedShared<gfx::Sampler>             font_sampler            = {};
    ManagedShared<gfx::Buffer>              dynamic_buffer          = {};
    vk::DeviceSize                          dynamic_buffer_offset   = {};
    ManagedShared<gfx::RenderPipelineState> render_pipeline_state   = {};
};