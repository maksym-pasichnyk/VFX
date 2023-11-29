#pragma once

#include "Object.hpp"
#include "Graphics.hpp"
#include "Canvas.hpp"

struct ImGuiBackend : public ManagedObject {
public:
    explicit ImGuiBackend(const rc<gfx::Device>& device);

private:
    void buildFonts();
    void buildShaders();
    void buildBuffers();

public:
    void resetForNewFrame();
    void setCurrentContext();

    auto drawList() -> ImDrawList*;
    void draw(const rc<gfx::RenderCommandEncoder>& encoder);
    void setScreenSize(const Size& size);

private:
    Size                                    screen_size             = Size::zero();

    ImFontAtlas                             im_font_atlas           = {};
    ImGuiContext                            im_gui_context          = {&im_font_atlas};
    ImDrawListSharedData                    im_shared_data          = {};
    ImDrawList                              im_draw_list            = {&im_shared_data};

    rc<gfx::Device>              device                  = {};
    rc<gfx::Texture>             font_texture            = {};
    rc<gfx::Sampler>             font_sampler            = {};
    rc<gfx::Buffer>              dynamic_buffer          = {};
    vk::DeviceSize                          dynamic_buffer_offset   = {};
    rc<gfx::RenderPipelineState> render_pipeline_state   = {};
};