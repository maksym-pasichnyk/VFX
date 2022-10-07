#pragma once

#include "pass.hpp"
#include "assets.hpp"
#include "display.hpp"
#include "context.hpp"
#include "renderer.hpp"
#include "material.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.cpp"

struct Widgets {
    Widgets(vfx::Context& context, Display& display, Renderer& renderer) : context(context) {
        IMGUI_CHECKVERSION();
        ctx = ImGui::CreateContext();

        ctx->IO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        ctx->IO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        ctx->IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        ctx->IO.BackendPlatformName = "imgui_impl_glfw";
        ctx->IO.BackendRendererName = "imgui_impl_vulkan";

        ImGui_ImplGlfw_InitForVulkan(display.window, true);

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

        description.attachments[0].blendEnable = true;
        description.attachments[0].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        description.attachments[0].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        description.attachments[0].colorBlendOp = vk::BlendOp::eAdd;
        description.attachments[0].srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        description.attachments[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;
        description.attachments[0].alphaBlendOp = vk::BlendOp::eAdd;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

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
        font_material = context.makeMaterial(description, renderer.render_pass, 0);

        const auto image_info = vk::DescriptorImageInfo{
            .sampler = font_sampler,
            .imageView = font_texture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = font_material->descriptor_sets[0],
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
        context.freeTexture(font_texture);
        context.freeMaterial(font_material);

        for (auto& frame : frames) {
            if (frame.vtx != nullptr) {
                context.freeBuffer(frame.vtx);
            }
            if (frame.idx != nullptr) {
                context.freeBuffer(frame.idx);
            }
        }
    }

    void begin_frame() {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void end_frame() {
        ImGui::Render();
        current_frame = (current_frame + 1) % vfx::Context::MAX_FRAMES_IN_FLIGHT;
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

        cmd.bindPipeline(font_material->pipeline_bind_point, font_material->pipeline);
        cmd.bindDescriptorSets(font_material->pipeline_bind_point, font_material->pipeline_layout, 0, font_material->descriptor_sets, {});

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

    void create_font_texture() {
        uint8_t *pixels;
        i32 width, height;
        ctx->IO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        auto font_texture_description = vfx::TextureDescription{
            .format = vk::Format::eR8G8B8A8Unorm,
            .width = u32(width),
            .height = u32(height),
            .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            .aspect = vk::ImageAspectFlagBits::eColor
        };
        font_texture = context.makeTexture(font_texture_description);

        auto font_sampler_description = vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat
        };
        font_sampler = context.logical_device.createSampler(font_sampler_description);

        context.set_texture_data(font_texture.get(), std::span(reinterpret_cast<const glm::u8vec4 *>(pixels), width * height));
        ctx->IO.Fonts->SetTexID(font_texture.get());
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
            font_material->pipeline_layout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            std::span(transform).size_bytes(),
            transform.data()
        );
    }

    vfx::Context& context;

    ImGuiContext* ctx;
    u64 current_frame = 0;
    vk::Sampler font_sampler;
    Box<vfx::Texture> font_texture;
    Box<vfx::Material> font_material;
    std::array<Geometry, vfx::Context::MAX_FRAMES_IN_FLIGHT> frames{};
};
