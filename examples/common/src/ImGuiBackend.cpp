#include "ImGuiBackend.hpp"
#include "Canvas.hpp"
#include "Assets.hpp"
#include "simd.hpp"

#include "glm/glm.hpp"

struct GuiShaderData {
    simd::float2 scale;
};

ImGuiBackend::ImGuiBackend(const ManagedShared<gfx::Device>& device) : device(device) {
    buildFonts();
    buildShaders();
    buildBuffers();

    im_shared_data.CurveTessellationTol = 0.10F;
}

void ImGuiBackend::buildFonts() {
    ImFontConfig font_cfg = {};
    font_cfg.SizePixels = 72.0F;

    im_font_atlas.AddFontDefault(&font_cfg);
    im_font_atlas.Build();

    std::vector<uint32_t> pixels = {};
    pixels.resize(im_font_atlas.TexWidth * im_font_atlas.TexHeight);
    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = IM_COL32(255, 255, 255, im_font_atlas.TexPixelsAlpha8[i]);
    }

    gfx::TextureDescription font_texture_description;
    font_texture_description.width = static_cast<uint32_t>(im_font_atlas.TexWidth);
    font_texture_description.height = static_cast<uint32_t>(im_font_atlas.TexHeight);
    font_texture_description.format = vk::Format::eR8G8B8A8Unorm;
    font_texture_description.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    font_texture = device->newTexture(font_texture_description);
    font_texture->replaceRegion(pixels.data(), pixels.size() * sizeof(uint32_t));

    vk::SamplerCreateInfo font_sampler_description = {};
    font_sampler_description.setMagFilter(vk::Filter::eLinear);
    font_sampler_description.setMinFilter(vk::Filter::eLinear);
    font_sampler_description.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    font_sampler_description.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    font_sampler_description.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    font_sampler_description.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    font_sampler = device->newSampler(font_sampler_description);

    im_font_atlas.SetTexID(font_texture->image);

    im_shared_data.Font = im_font_atlas.Fonts.front();
    im_shared_data.FontSize = im_shared_data.Font->FontSize;
    im_shared_data.TexUvWhitePixel = im_font_atlas.TexUvWhitePixel;
}

void ImGuiBackend::buildShaders() {
    auto vertexLibrary      = device->newLibrary(Assets::readFile("shaders/gui.vert.spv"));
    auto fragmentLibrary    = device->newLibrary(Assets::readFile("shaders/gui.frag.spv"));

    gfx::RenderPipelineStateDescription description;
    description.vertexFunction      = vertexLibrary->newFunction("main");
    description.fragmentFunction    = fragmentLibrary->newFunction("main");
    description.vertexInputState    = {
        .bindings = {
            vk::VertexInputBindingDescription{0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex}
        },
        .attributes = {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos)},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv)},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col)}
        }
    };

    description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;

    description.colorBlendAttachments[0].setBlendEnable(true);
    description.colorBlendAttachments[0].setColorBlendOp(vk::BlendOp::eAdd);
    description.colorBlendAttachments[0].setAlphaBlendOp(vk::BlendOp::eAdd);
    description.colorBlendAttachments[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    description.colorBlendAttachments[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);

    render_pipeline_state = device->newRenderPipelineState(description);
}

void ImGuiBackend::buildBuffers() {
    dynamic_buffer = device->newBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
        5ULL * 1024ULL * 1024ULL,
        gfx::StorageMode::eShared,
        0);
}

void ImGuiBackend::resetForNewFrame() {
    dynamic_buffer_offset = 0;

    im_draw_list._ResetForNewFrame();
    im_draw_list.PushClipRect(ImVec2(0, 0), ImVec2(screen_size.width, screen_size.height));
    im_draw_list.PushTextureID(im_font_atlas.TexID);
}

void ImGuiBackend::setCurrentContext() {
    ImGui::SetCurrentContext(&im_gui_context);
}

auto ImGuiBackend::drawList() -> ImDrawList* {
    return &im_draw_list;
}

void ImGuiBackend::draw(const ManagedShared<gfx::RenderCommandEncoder>& encoder) {
    if (im_draw_list.IdxBuffer.Size == 0) {
        return;
    }

    GuiShaderData gui_shader_data = {};
    gui_shader_data.scale = 2.0F / simd::float2{screen_size.width, screen_size.height};

    auto descriptor_set = encoder->getCommandBuffer()->newDescriptorSet(render_pipeline_state, 0);

    vk::DescriptorImageInfo sampler_info = {};
    sampler_info.setSampler(font_sampler->raw);

    vk::DescriptorImageInfo image_info = {};
    image_info.setImageView(font_texture->image_view);
    image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::WriteDescriptorSet writes[2] = {};
    writes[0].setDstSet(descriptor_set);
    writes[0].setDstBinding(0);
    writes[0].setDstArrayElement(0);
    writes[0].setDescriptorCount(1);
    writes[0].setDescriptorType(vk::DescriptorType::eSampler);
    writes[0].setPImageInfo(&sampler_info);

    writes[1].setDstSet(descriptor_set);
    writes[1].setDstBinding(1);
    writes[1].setDstArrayElement(0);
    writes[1].setDescriptorCount(1);
    writes[1].setDescriptorType(vk::DescriptorType::eSampledImage);
    writes[1].setPImageInfo(&image_info);

    device->raii.raw.updateDescriptorSets(writes, {}, device->raii.dispatcher);

    vk::DeviceSize vertex_buffer_offset = dynamic_buffer_offset;
    dynamic_buffer_offset += im_draw_list.VtxBuffer.Size * sizeof(ImDrawVert);

    vk::DeviceSize index_buffer_offset  = dynamic_buffer_offset;
    dynamic_buffer_offset += im_draw_list.IdxBuffer.Size * sizeof(ImDrawIdx);

    std::memcpy(static_cast<std::byte*>(dynamic_buffer->contents()) + vertex_buffer_offset, im_draw_list.VtxBuffer.Data, im_draw_list.VtxBuffer.Size * sizeof(ImDrawVert));
    std::memcpy(static_cast<std::byte*>(dynamic_buffer->contents()) + index_buffer_offset, im_draw_list.IdxBuffer.Data, im_draw_list.IdxBuffer.Size * sizeof(ImDrawIdx));

    encoder->setRenderPipelineState(render_pipeline_state);
    encoder->bindDescriptorSet(descriptor_set, 0);
    encoder->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GuiShaderData), &gui_shader_data);
    encoder->bindVertexBuffer(0, dynamic_buffer, vertex_buffer_offset);
    encoder->bindIndexBuffer(dynamic_buffer, index_buffer_offset, vk::IndexType::eUint16);

    for (auto& drawCmd : std::span(im_draw_list.CmdBuffer.Data, im_draw_list.CmdBuffer.Size)) {
        encoder->drawIndexed(drawCmd.ElemCount, 1, drawCmd.IdxOffset, static_cast<int32_t>(drawCmd.VtxOffset), 0);
    }
}

void ImGuiBackend::setScreenSize(const Size& size) {
    screen_size = size;
}