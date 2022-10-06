#pragma once

#include <assets.hpp>
#include <pass.hpp>
#include <display.hpp>
#include <context.hpp>
#include <material.hpp>

#include <imgui.h>
#include <imgui_internal.h>

struct Widgets {
    static constexpr auto buttons = std::array{
        MouseButton::Left,
        MouseButton::Right,
        MouseButton::Middle,
    };

    static constexpr auto keycodes = std::array {
        KeyCode::eTab,
        KeyCode::eLeft,
        KeyCode::eRight,
        KeyCode::eUp,
        KeyCode::eDown,
        KeyCode::ePageUp,
        KeyCode::ePageDown,
        KeyCode::eHome,
        KeyCode::eEnd,
        KeyCode::eInsert,
        KeyCode::eDelete,
        KeyCode::eBackspace,
        KeyCode::eSpace,
        KeyCode::eEnter,
        KeyCode::eEscape,
        KeyCode::eKeyPadEnter,
        KeyCode::eA,
        KeyCode::eC,
        KeyCode::eV,
        KeyCode::eX,
        KeyCode::eY,
        KeyCode::eZ
    };

    Widgets(vfx::Context& context, vk::RenderPass pass) : context(context) {
        IMGUI_CHECKVERSION();
        ctx = ImGui::CreateContext();

        ctx->IO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        ctx->IO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        ctx->IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        ctx->IO.BackendPlatformName = "imgui_impl_glfw";
        ctx->IO.BackendRendererName = "imgui_impl_vulkan";

    // Keyboard mapping. ImGui will use those indices to peek into the ctx->IO.KeysDown[] array.
        ctx->IO.KeyMap[ImGuiKey_Tab] = i32(KeyCode::eTab);
        ctx->IO.KeyMap[ImGuiKey_LeftArrow] = i32(KeyCode::eLeft);
        ctx->IO.KeyMap[ImGuiKey_RightArrow] = i32(KeyCode::eRight);
        ctx->IO.KeyMap[ImGuiKey_UpArrow] = i32(KeyCode::eUp);
        ctx->IO.KeyMap[ImGuiKey_DownArrow] = i32(KeyCode::eDown);
        ctx->IO.KeyMap[ImGuiKey_PageUp] = i32(KeyCode::ePageUp);
        ctx->IO.KeyMap[ImGuiKey_PageDown] = i32(KeyCode::ePageDown);
        ctx->IO.KeyMap[ImGuiKey_Home] = i32(KeyCode::eHome);
        ctx->IO.KeyMap[ImGuiKey_End] = i32(KeyCode::eEnd);
        ctx->IO.KeyMap[ImGuiKey_Insert] = i32(KeyCode::eInsert);
        ctx->IO.KeyMap[ImGuiKey_Delete] = i32(KeyCode::eDelete);
        ctx->IO.KeyMap[ImGuiKey_Backspace] = i32(KeyCode::eBackspace);
        ctx->IO.KeyMap[ImGuiKey_Space] = i32(KeyCode::eSpace);
        ctx->IO.KeyMap[ImGuiKey_Enter] = i32(KeyCode::eEnter);
        ctx->IO.KeyMap[ImGuiKey_Escape] = i32(KeyCode::eEscape);
        ctx->IO.KeyMap[ImGuiKey_KeyPadEnter] = i32(KeyCode::eKeyPadEnter);
        ctx->IO.KeyMap[ImGuiKey_A] = i32(KeyCode::eA);
        ctx->IO.KeyMap[ImGuiKey_C] = i32(KeyCode::eC);
        ctx->IO.KeyMap[ImGuiKey_V] = i32(KeyCode::eV);
        ctx->IO.KeyMap[ImGuiKey_X] = i32(KeyCode::eX);
        ctx->IO.KeyMap[ImGuiKey_Y] = i32(KeyCode::eY);
        ctx->IO.KeyMap[ImGuiKey_Z] = i32(KeyCode::eZ);

        ImGui::StyleColorsDark(&ctx->Style);

        ctx->Style.WindowRounding = 0.0f;
        ctx->Style.ChildRounding = 0.0f;
        ctx->Style.FrameRounding = 0.0f;
        ctx->Style.GrabRounding = 0.0f;
        ctx->Style.PopupRounding = 0.0f;
        ctx->Style.ScrollbarRounding = 0.0f;

        create_font_texture();

        vfx::MaterialDescription description{};

        description.bindings = {
            {0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex}
        };

        description.attributes = {
            {0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos) },
            {1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv) },
            {2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col) }
        };

        description.create_attachment(vk::PipelineColorBlendAttachmentState{
            .blendEnable = true,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                              vk::ColorComponentFlagBits::eG |
                              vk::ColorComponentFlagBits::eB |
                              vk::ColorComponentFlagBits::eA
        });
        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.inputAssemblyState.primitiveRestartEnable = false;

        description.rasterizationState.depthClampEnable        = false;
        description.rasterizationState.rasterizerDiscardEnable = false;
        description.rasterizationState.polygonMode             = vk::PolygonMode::eFill;
        description.rasterizationState.cullMode                = vk::CullModeFlagBits::eNone;
        description.rasterizationState.frontFace               = vk::FrontFace::eCounterClockwise;
        description.rasterizationState.depthBiasEnable         = false;
        description.rasterizationState.depthBiasConstantFactor = 0.0f;
        description.rasterizationState.depthBiasClamp          = 0.0f;
        description.rasterizationState.depthBiasSlopeFactor    = 0.0f;
        description.rasterizationState.lineWidth               = 1.0f;

        description.depthStencilState.depthTestEnable       = false;
        description.depthStencilState.depthWriteEnable      = false;
        description.depthStencilState.depthCompareOp        = vk::CompareOp::eNever;
        description.depthStencilState.depthBoundsTestEnable = false;
        description.depthStencilState.stencilTestEnable     = false;
        description.depthStencilState.front                 = vk::StencilOpState{};
        description.depthStencilState.back                  = vk::StencilOpState{};
        description.depthStencilState.minDepthBounds        = 0.0f;
        description.depthStencilState.maxDepthBounds        = 0.0f;

        description.shaders.emplace_back(vfx::ShaderDescription{
            .bytes = Assets::read_file("shaders/imgui.vert.spv"),
            .entry = "main",
            .stage = vk::ShaderStageFlagBits::eVertex
        });
        description.shaders.emplace_back(vfx::ShaderDescription{
            .bytes = Assets::read_file("shaders/imgui.frag.spv"),
            .entry = "main",
            .stage = vk::ShaderStageFlagBits::eFragment
        });
        material = context.create_material(description, pass, 0);

        const auto image_info = vk::DescriptorImageInfo{
            .sampler = font_sampler,
            .imageView = font_texture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = material->descriptor_sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &image_info
        };
        context.logical_device.updateDescriptorSets(write_descriptor_set, {});
    }

    ~Widgets() {
        ImGui::DestroyContext(ctx);

        context.logical_device.destroySampler(font_sampler);
        context.destroy_texture(font_texture);
        context.destroy_material(material);

        for (auto& frame : frames) {
            if (frame.vtx != nullptr) {
                context.destroy_buffer(frame.vtx);
            }
            if (frame.idx != nullptr) {
                context.destroy_buffer(frame.idx);
            }
        }
    }

    void update(Display& display) {
        set_display_size(display.get_size());
        set_display_scale(display.get_scale());
        set_mouse_position(display.get_mouse_position());

        for (auto button : buttons) {
            set_mouse_pressed(i32(button), display.get_mouse_pressed(button));
        }
        for (auto keycode : keycodes) {
            set_key_pressed(i32(keycode), display.get_key_pressed(keycode));
        }
    }

    void set_display_size(const glm::ivec2& size) {
        ctx->IO.DisplaySize = {
            static_cast<f32>(size.x),
            static_cast<f32>(size.y)
        };
    }

    void set_display_scale(const glm::vec2& scale) {
        ctx->IO.DisplayFramebufferScale = {
            scale.x,
            scale.y
        };
    }

    void set_delta_time(f32 delta) {
        ctx->IO.DeltaTime = delta;
    }

    void set_mouse_position(const glm::vec2& pos) {
        ctx->IO.MousePos = { pos.x, pos.y };
    }

    void set_mouse_pressed(i32 button, bool flag) {
        ctx->IO.MouseDown[button] = flag;
    }

    void set_key_pressed(i32 button, bool flag) {
        ctx->IO.KeysDown[button] = flag;
    }

    void begin_frame() {
        ImGui::NewFrame();
    }
    void end_frame() {
        ImGui::Render();

        current_frame += 1;
        current_frame %= vfx::Context::MAX_FRAMES_IN_FLIGHT;
    }

    void draw(vk::CommandBuffer cmd) {
        const auto viewport = ctx->Viewports[0];
        if (!viewport->DrawDataP.Valid) {
            return;
        }

        const auto draw_data = std::addressof(viewport->DrawDataP);

        const auto display_pos = draw_data->DisplayPos;
        const auto display_size = draw_data->DisplaySize;
        const auto fb_scale = draw_data->FramebufferScale;
        const auto display_rect = ImRect(display_pos, display_pos + display_size);

        const auto fb_size = display_size * fb_scale;
        const auto fb_width = static_cast<i32>(fb_size.x);
        const auto fb_height = static_cast<i32>(fb_size.y);
        if (fb_width <= 0 || fb_height <= 0) {
            return;
        }

        auto frame = &frames[current_frame];

        if (draw_data->TotalVtxCount > 0) {
            context.set_vertex_buffer_params(frame, draw_data->TotalVtxCount, sizeof(ImDrawVert));
            context.set_index_buffer_params(frame, draw_data->TotalIdxCount, sizeof(ImDrawIdx));

            auto vtx_dst_offset = 0;
            auto idx_dst_offset = 0;
            for (i32 n = 0; n < draw_data->CmdListsCount; n++) {
                auto cmd_list = draw_data->CmdLists[n];
                context.set_vertex_buffer_data(frame, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, vtx_dst_offset);
                context.set_index_buffer_data(frame, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, idx_dst_offset);
                vtx_dst_offset += cmd_list->VtxBuffer.Size;
                idx_dst_offset += cmd_list->IdxBuffer.Size;
            }
        }

        cmd.bindPipeline(material->pipeline_bind_point, material->pipeline);
        cmd.bindDescriptorSets(material->pipeline_bind_point, material->pipeline_layout, 0, material->descriptor_sets, {});

        setup_render_state(draw_data, cmd, frame, fb_width, fb_height);

        u32 global_vtx_offset = 0;
        u32 global_idx_offset = 0;
        for (i32 n = 0; n < draw_data->CmdListsCount; n++) {
            const auto cmd_list = draw_data->CmdLists[n];
            for (i32 cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                auto pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != nullptr) {
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                        setup_render_state(draw_data, cmd, frame, fb_width, fb_height);
                    } else {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                } else {
                    auto clipRect = ImRect(pcmd->ClipRect);
                    if (!clipRect.Overlaps(display_rect)) {
                        continue;
                    }
                    clipRect.ClipWithFull(display_rect);

                    const auto offset = (clipRect.Min - display_pos) * fb_scale;
                    const auto extent = clipRect.GetSize() * fb_scale;

                    const auto scissor = vk::Rect2D{
                        .offset = vk::Offset2D {
                            static_cast<i32>(offset.x),
                            static_cast<i32>(offset.y)
                        },
                        .extent = vk::Extent2D {
                            static_cast<u32>(extent.x),
                            static_cast<u32>(extent.y)
                        }
                    };

                    cmd.setScissor(0, 1, &scissor);
                    cmd.drawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, i32(pcmd->VtxOffset + global_vtx_offset), 0);
                }
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
        }
    }

    void set_current_context() {
        ImGui::SetCurrentContext(ctx);
    }

    auto want_capture_mouse() -> bool {
        return ctx->IO.WantCaptureMouse;
    }

    void add_input_character(char ch) {
        ctx->IO.AddInputCharacter(ch);
    }

    void add_scroll_mouse(f32 x, f32 y) {
        ctx->IO.MouseWheelH += x;
        ctx->IO.MouseWheel += y;
    }

    void create_font_texture() {
        uint8_t *pixels;
        i32 width, height;
        ctx->IO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        font_texture = context.create_texture(
            u32(width),
            u32(height),
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            vk::ImageAspectFlagBits::eColor
    //        vk::ImageLayout::eShaderReadOnlyOptimal
        );
        font_sampler = context.logical_device.createSampler(vk::SamplerCreateInfo{
    //            .magFilter = vk::Filter::eLinear,
    //            .minFilter = vk::Filter::eLinear,
    //            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
            .maxAnisotropy = 1.0f,
            .minLod = -1000,
            .maxLod = 1000
        });
        context.set_texture_data(font_texture, std::span(reinterpret_cast<const glm::u8vec4 *>(pixels), width * height));
        ctx->IO.Fonts->SetTexID(font_texture);
    }

    void setup_render_state(ImDrawData* draw_data, vk::CommandBuffer cmd, Geometry* rb, i32 fb_width, i32 fb_height) {
        if (draw_data->TotalVtxCount > 0) {
            cmd.bindVertexBuffers(0, rb->vtx->buffer, vk::DeviceSize{0});
            cmd.bindIndexBuffer(rb->idx->buffer, 0, vk::IndexType::eUint16);
        }

        cmd.setViewport(0, vk::Viewport{0, 0, f32(fb_width), f32(fb_height), 0, 1});

        const auto pos = glm::vec2(draw_data->DisplayPos.x, draw_data->DisplayPos.y);
        const auto inv_size = 2.0f / glm::vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y);

        const auto transform = std::array {
            inv_size,
            -(pos * inv_size + 1.0f)
        };

        cmd.pushConstants(
            material->pipeline_layout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            std::span(transform).size_bytes(),
            transform.data()
        );
    }

    vfx::Context& context;

    ImGuiContext* ctx;
    vfx::Material* material;
    vfx::Texture* font_texture;
    vk::Sampler font_sampler;
    u64 current_frame = 0;
    std::array<Geometry, vfx::Context::MAX_FRAMES_IN_FLIGHT> frames{};
};
