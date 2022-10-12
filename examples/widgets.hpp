#pragma once

#include "pass.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "assets.hpp"
#include "window.hpp"
#include "context.hpp"
#include "renderer.hpp"
#include "material.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.cpp"

struct Widgets {
    Widgets(const Arc<vfx::Context>& context, const Arc<vfx::Window>& window) : context(context) {
        IMGUI_CHECKVERSION();
        ctx = ImGui::CreateContext();

        ctx->IO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        ctx->IO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        ctx->IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

        ctx->IO.BackendPlatformName = "imgui_impl_glfw";
        ctx->IO.BackendRendererName = "imgui_impl_vulkan";

        ImGui_ImplGlfw_InitForVulkan(window->getHandle(), true);

        ImGui::StyleColorsDark(&ctx->Style);

        ctx->Style.WindowRounding = 0.0f;
        ctx->Style.ChildRounding = 0.0f;
        ctx->Style.FrameRounding = 0.0f;
        ctx->Style.GrabRounding = 0.0f;
        ctx->Style.PopupRounding = 0.0f;
        ctx->Style.ScrollbarRounding = 0.0f;

        create_pipeline_state();
        create_font_texture();

        for (auto& frame : frames) {
            frame = Arc<vfx::Mesh>::alloc(context);
        }
    }

    ~Widgets() {
        ImGui::DestroyContext(ctx);
        
        context->logical_device.destroyDescriptorPool(descriptor_pool);
        context->logical_device.destroySampler(font_sampler);
    }
    
    void create_pipeline_state() {
        vfx::PipelineStateDescription description{};

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

        description.rasterizationState.lineWidth = 1.0f;

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
        pipeline_state = context->makePipelineState(description);

        auto pool_sizes = std::array{
            vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1}
        };
        auto pool_create_info = vk::DescriptorPoolCreateInfo{};
        pool_create_info.setMaxSets(1);
        pool_create_info.setPoolSizes(pool_sizes);
        descriptor_pool = context->logical_device.createDescriptorPool(pool_create_info, nullptr);

        auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
        ds_allocate_info.setDescriptorPool(descriptor_pool);
        ds_allocate_info.setSetLayouts(pipeline_state->descriptorSetLayouts);
        descriptor_sets = context->logical_device.allocateDescriptorSets(ds_allocate_info);
    }

    void begin_frame() {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void end_frame() {
        ImGui::Render();
        current_frame = (current_frame + 1) % frames.size();
    }

    void draw(vfx::CommandBuffer* cmd) {
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

        auto& frame = frames[current_frame];

        if (draw_data->TotalVtxCount > 0) {
            frame->setIndexBufferParams(draw_data->TotalIdxCount, sizeof(ImDrawIdx));
            frame->setVertexBufferParams(draw_data->TotalVtxCount, sizeof(ImDrawVert));

            auto vtx_dst_offset = 0;
            auto idx_dst_offset = 0;
            for (i32 n = 0; n < draw_data->CmdListsCount; n++) {
                auto cmd_list = draw_data->CmdLists[n];
                frame->setVertexBufferData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, vtx_dst_offset);
                frame->setIndexBufferData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, idx_dst_offset);
                vtx_dst_offset += cmd_list->VtxBuffer.Size;
                idx_dst_offset += cmd_list->IdxBuffer.Size;
            }
        }

        cmd->setPipelineState(pipeline_state);
        cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_state->pipelineLayout, 0, descriptor_sets, {});

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

                    // todo: update texture
                    cmd->setScissor(0, scissor);
                    cmd->drawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, i32(pcmd->VtxOffset + global_vtx_offset), 0);
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
        font_texture = context->makeTexture(font_texture_description);

        auto font_sampler_description = vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eLinear,
            .minFilter = vk::Filter::eLinear,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat
        };
        font_sampler = context->logical_device.createSampler(font_sampler_description);
        font_texture->setPixelData(std::span(reinterpret_cast<const glm::u8vec4 *>(pixels), width * height));
        ctx->IO.Fonts->SetTexID(font_texture.get());

        const auto image_info = vk::DescriptorImageInfo{
            .sampler = font_sampler,
            .imageView = font_texture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = descriptor_sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &image_info
        };
        context->logical_device.updateDescriptorSets(write_descriptor_set, {});
    }

    void setup_render_state(ImDrawData* draw_data, vfx::CommandBuffer* cmd, Arc<vfx::Mesh>& mesh, i32 fb_width, i32 fb_height) {
        if (draw_data->TotalVtxCount > 0) {
            cmd->handle.bindVertexBuffers(0, mesh->vertexBuffer->handle, vk::DeviceSize{0});
            cmd->handle.bindIndexBuffer(mesh->indexBuffer->handle, 0, vk::IndexType::eUint16);
        }

        cmd->setViewport(0, vk::Viewport{0, 0, f32(fb_width), f32(fb_height), 0, 1});

        const auto pos = glm::vec2(draw_data->DisplayPos.x, draw_data->DisplayPos.y);
        const auto inv_size = 2.0f / glm::vec2(draw_data->DisplaySize.x, draw_data->DisplaySize.y);

        const auto transform = std::array {
            inv_size,
            -(pos * inv_size + 1.0f)
        };

        cmd->handle.pushConstants(
            pipeline_state->pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            0,
            std::span(transform).size_bytes(),
            transform.data()
        );
    }

    Arc<vfx::Context> context;

    ImGuiContext* ctx;
    u64 current_frame = 0;
    vk::Sampler font_sampler{};
    Arc<vfx::Texture> font_texture{};
    Arc<vfx::PipelineState> pipeline_state{};
    std::array<Arc<vfx::Mesh>, vfx::Context::MAX_FRAMES_IN_FLIGHT> frames{};

    // todo: bindless
    vk::DescriptorPool descriptor_pool{};
    std::vector<vk::DescriptorSet> descriptor_sets{};
};
