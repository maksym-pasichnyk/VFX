#include "ImGuiRenderer.hpp"
#include "Math.hpp"
#include "Mesh.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.cpp"

ImGuiRenderer::ImGuiRenderer(const Arc<vfx::Context>& context, const Arc<Window>& window) : context(context) {
    IMGUI_CHECKVERSION();
    ctx = ImGui::CreateContext();

    ctx->IO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    ctx->IO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    ctx->IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    ctx->IO.BackendPlatformName = "imgui_impl_glfw";
    ctx->IO.BackendRendererName = "imgui_impl_vulkan";

    ImGui_ImplGlfw_InitForVulkan(window->handle, true);

    ImGui::StyleColorsDark(&ctx->Style);

    ctx->Style.WindowRounding = 0.0f;
    ctx->Style.ChildRounding = 0.0f;
    ctx->Style.FrameRounding = 0.0f;
    ctx->Style.GrabRounding = 0.0f;
    ctx->Style.PopupRounding = 0.0f;
    ctx->Style.ScrollbarRounding = 0.0f;

    createPipelineState();
    createFontTexture();
}

ImGuiRenderer::~ImGuiRenderer() {
    ImGui::DestroyContext(ctx);
}

void ImGuiRenderer::beginFrame() {
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::endFrame() {
    ImGui::Render();
}

void ImGuiRenderer::draw(vfx::CommandBuffer* cmd) {
    ImGuiViewportP* viewport = ctx->Viewports[0];
    if (!viewport->DrawDataP.Valid) {
        return;
    }

    ImDrawData* data = std::addressof(viewport->DrawDataP);

    if (data->TotalVtxCount <= 0) {
        return;
    }

    ImVec2 display_pos = data->DisplayPos;
    ImVec2 display_size = data->DisplaySize;
    ImVec2 fb_scale = data->FramebufferScale;
    ImRect display_rect = ImRect(display_pos, display_pos + display_size);

    ImVec2 fb_size = display_size * fb_scale;
    i32 fb_width = static_cast<i32>(fb_size.x);
    i32 fb_height = static_cast<i32>(fb_size.y);
    if (fb_width <= 0 || fb_height <= 0) {
        return;
    }

    auto mesh = Arc<Mesh>::alloc();

    mesh->indexCount = data->TotalIdxCount;
    mesh->vertexCount = data->TotalVtxCount;

    mesh->indexBuffer = context->makeBuffer(vfx::BufferUsage::Index, mesh->indexCount * sizeof(ImDrawIdx));
    mesh->vertexBuffer = context->makeBuffer(vfx::BufferUsage::Vertex, mesh->vertexCount * sizeof(ImDrawVert));

    auto indexBufferOffset = 0;
    auto vertexBufferOffset = 0;
    for (ImDrawList* drawList : std::span(data->CmdLists, data->CmdListsCount)) {
        mesh->indexBuffer->update(
            drawList->IdxBuffer.Data,
            drawList->IdxBuffer.Size * sizeof(ImDrawIdx),
            indexBufferOffset * sizeof(ImDrawIdx)
        );
        mesh->vertexBuffer->update(
            drawList->VtxBuffer.Data,
            drawList->VtxBuffer.Size * sizeof(ImDrawVert),
            vertexBufferOffset * sizeof(ImDrawVert)
        );
        indexBufferOffset += drawList->IdxBuffer.Size;
        vertexBufferOffset += drawList->VtxBuffer.Size;
    }

    cmd->setPipelineState(pipelineState);
    cmd->handle->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineState->pipelineLayout, 0, descriptor_sets, {});

    setupRenderState(data, cmd, mesh, fb_width, fb_height);

    u32 global_vtx_offset = 0;
    u32 global_idx_offset = 0;
    for (ImDrawList* drawList : std::span(data->CmdLists, data->CmdListsCount)) {
        for (ImDrawCmd& drawCmd : std::span(drawList->CmdBuffer.Data, drawList->CmdBuffer.Size)) {
            if (drawCmd.UserCallback != nullptr) {
                if (drawCmd.UserCallback == ImDrawCallback_ResetRenderState) {
                    setupRenderState(data, cmd, mesh, fb_width, fb_height);
                } else {
                    drawCmd.UserCallback(drawList, &drawCmd);
                }
            } else {
                auto clipRect = ImRect(drawCmd.ClipRect);
                if (!clipRect.Overlaps(display_rect)) {
                    continue;
                }
                clipRect.ClipWithFull(display_rect);

                ImVec2 offset = (clipRect.Min - display_pos) * fb_scale;
                ImVec2 extent = clipRect.GetSize() * fb_scale;

                vk::Rect2D scissor{};
                scissor.setOffset(vk::Offset2D{i32(offset.x), i32(offset.y)});
                scissor.setExtent(vk::Extent2D{u32(extent.x), u32(extent.y)});

                // todo: update texture
                cmd->setScissor(0, scissor);
                cmd->drawIndexed(drawCmd.ElemCount, 1, drawCmd.IdxOffset + global_idx_offset, i32(drawCmd.VtxOffset + global_vtx_offset), 0);
            }
        }
        global_idx_offset += drawList->IdxBuffer.Size;
        global_vtx_offset += drawList->VtxBuffer.Size;
    }
}

void ImGuiRenderer::createFontTexture() {
    uint8_t *pixels;
    i32 width, height;
    ctx->IO.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    auto font_texture_description = vfx::TextureDescription{
        .format = vk::Format::eR8G8B8A8Srgb,
        .width = u32(width),
        .height = u32(height),
        .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
    };
    fontTexture = context->makeTexture(font_texture_description);

    auto font_sampler_description = vk::SamplerCreateInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat
    };
    fontSampler = context->makeSampler(font_sampler_description);
    fontTexture->setPixelData(std::span(reinterpret_cast<const glm::u8vec4 *>(pixels), width * height));
    ctx->IO.Fonts->SetTexID(fontTexture.get());

    const auto image_info = vk::DescriptorImageInfo{
        .sampler = fontSampler->handle,
        .imageView = fontTexture->view,
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
    context->device->updateDescriptorSets(write_descriptor_set, {});
}

void ImGuiRenderer::createPipelineState() {
    vfx::PipelineStateDescription description{};

    vfx::PipelineVertexDescription vertexDescription{};
    vertexDescription.layouts = {{
        {0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex}
    }};
    vertexDescription.attributes = {{
        {0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos) },
        {1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv) },
        {2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col) }
    }};
    description.vertexDescription = vertexDescription;

    description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;

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

    auto vertexLibrary = context->makeLibrary(Assets::readFile("shaders/imgui.vert.spv"));
    auto fragmentLibrary = context->makeLibrary(Assets::readFile("shaders/imgui.frag.spv"));

    description.vertexFunction = vertexLibrary->makeFunction("main");
    description.fragmentFunction = fragmentLibrary->makeFunction("main");

    pipelineState = context->makePipelineState(description);

    auto pool_sizes = std::array{
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1}
    };
    auto pool_create_info = vk::DescriptorPoolCreateInfo{};
    pool_create_info.setMaxSets(1);
    pool_create_info.setPoolSizes(pool_sizes);
    descriptor_pool = context->device->createDescriptorPoolUnique(pool_create_info);

    auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
    ds_allocate_info.setDescriptorPool(*descriptor_pool);
    ds_allocate_info.setSetLayouts(pipelineState->descriptorSetLayouts);
    descriptor_sets = context->device->allocateDescriptorSets(ds_allocate_info);
}

void ImGuiRenderer::setupRenderState(ImDrawData* data, vfx::CommandBuffer* cmd, const Arc<Mesh>& mesh, i32 width, i32 height) {
    if (data->TotalVtxCount > 0) {
        cmd->bindVertexBuffer(0, mesh->vertexBuffer, 0);
        cmd->bindIndexBuffer(mesh->indexBuffer, 0, vk::IndexType::eUint16);
    }

    cmd->setViewport(0, vk::Viewport{0, 0, f32(width), f32(height), 0, 1});

    const auto pos = glm::vec2(data->DisplayPos.x, data->DisplayPos.y);
    const auto inv_size = 2.0f / glm::vec2(data->DisplaySize.x, data->DisplaySize.y);

    const auto transform = std::array {
        inv_size,
        -(pos * inv_size + 1.0f)
    };

    cmd->handle->pushConstants(
        pipelineState->pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        std::span(transform).size_bytes(),
        transform.data()
    );
}
