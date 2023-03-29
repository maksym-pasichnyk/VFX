#include "UIRenderer.hpp"
#include "UIContext.hpp"
#include "Assets.hpp"
#include "simd.hpp"

#include "glm/glm.hpp"

struct GuiShaderData {
    simd::float2 scale;
};

UIRenderer::UIRenderer(const ManagedShared<gfx::Device>& device)
: device(device)
, mDrawList(&mDrawListSharedData) {
    buildFonts();
    buildShaders();
    buildBuffers();

    mDrawListSharedData.CurveTessellationTol = 0.10F;
}

void UIRenderer::buildFonts() {
    ImFontConfig font_cfg = {};
    font_cfg.SizePixels = 72.0F;

    mFontAtlas.AddFontDefault(&font_cfg);
    mFontAtlas.Build();

    std::vector<uint32_t> pixels = {};
    pixels.resize(mFontAtlas.TexWidth * mFontAtlas.TexHeight);
    for (size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = IM_COL32(255, 255, 255, mFontAtlas.TexPixelsAlpha8[i]);
    }

    gfx::TextureSettings font_texture_description;
    font_texture_description.width = static_cast<uint32_t>(mFontAtlas.TexWidth);
    font_texture_description.height = static_cast<uint32_t>(mFontAtlas.TexHeight);
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

    mFontAtlas.SetTexID(font_texture->image);

    mDrawListSharedData.Font = mFontAtlas.Fonts.front();
    mDrawListSharedData.FontSize = mDrawListSharedData.Font->FontSize;
    mDrawListSharedData.TexUvWhitePixel = mFontAtlas.TexUvWhitePixel;
}

void UIRenderer::buildShaders() {
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
    description.depthStencilState.depth_test_enable = false;

    description.colorBlendAttachments[0].setBlendEnable(true);
    description.colorBlendAttachments[0].setColorBlendOp(vk::BlendOp::eAdd);
    description.colorBlendAttachments[0].setAlphaBlendOp(vk::BlendOp::eAdd);
    description.colorBlendAttachments[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    description.colorBlendAttachments[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);

    render_pipeline_state = device->newRenderPipelineState(description);
}

void UIRenderer::buildBuffers() {
    dynamic_buffer = device->newBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
        5ULL * 1024ULL * 1024ULL,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
    );
}

void UIRenderer::resetForNewFrame() {
    dynamic_buffer_offset = 0;

    mDrawList._ResetForNewFrame();
    mDrawList.PushClipRect(ImVec2(0, 0), ImVec2(mScreenSize.width, mScreenSize.height));
    mDrawList.PushTextureID(mFontAtlas.TexID);
}

void UIRenderer::setCurrentContext() {
    ImGui::SetCurrentContext(&mGuiContext);
}

auto UIRenderer::drawList() -> ImDrawList* {
    return &mDrawList;
}

void UIRenderer::draw(const ManagedShared<gfx::RenderCommandEncoder>& encoder) {
    if (mDrawList.IdxBuffer.Size == 0) {
        return;
    }

    GuiShaderData gui_shader_data = {};
    gui_shader_data.scale = 2.0F / simd::float2{mScreenSize.width, mScreenSize.height};

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
    dynamic_buffer_offset += mDrawList.VtxBuffer.Size * sizeof(ImDrawVert);

    vk::DeviceSize index_buffer_offset  = dynamic_buffer_offset;
    dynamic_buffer_offset += mDrawList.IdxBuffer.Size * sizeof(ImDrawIdx);

    std::memcpy(static_cast<std::byte*>(dynamic_buffer->contents()) + vertex_buffer_offset, mDrawList.VtxBuffer.Data, mDrawList.VtxBuffer.Size * sizeof(ImDrawVert));
    std::memcpy(static_cast<std::byte*>(dynamic_buffer->contents()) + index_buffer_offset, mDrawList.IdxBuffer.Data, mDrawList.IdxBuffer.Size * sizeof(ImDrawIdx));


//    mDescriptorSet.setSampler(font_sampler, 0);
//    mDescriptorSet.setTexture(font_texture, 1);

    encoder->setRenderPipelineState(render_pipeline_state);
    encoder->bindDescriptorSet(descriptor_set, 0);
    encoder->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GuiShaderData), &gui_shader_data);
    encoder->bindVertexBuffer(0, dynamic_buffer, vertex_buffer_offset);
    encoder->bindIndexBuffer(dynamic_buffer, index_buffer_offset, vk::IndexType::eUint16);

    for (auto& drawCmd : std::span(mDrawList.CmdBuffer.Data, mDrawList.CmdBuffer.Size)) {
        encoder->drawIndexed(drawCmd.ElemCount, 1, drawCmd.IdxOffset, static_cast<int32_t>(drawCmd.VtxOffset), 0);
    }
}

void UIRenderer::setScreenSize(const Size& size) {
    mScreenSize = size;
}