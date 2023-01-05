#include "UIRenderer.hpp"
#include "UIContext.hpp"
#include "Assets.hpp"
#include "simd.hpp"

#include "glm/glm.hpp"

struct GuiShaderData {
    simd::float2 scale;
};

UIRenderer::UIRenderer(gfx::SharedPtr<gfx::Device> device)
: mDevice(std::move(device))
, mDrawList(&mDrawListSharedData) {
    buildFonts();
    buildShaders();

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

    gfx::TextureDescription font_texture_description = {};
    font_texture_description.format = vk::Format::eR8G8B8A8Unorm;
    font_texture_description.width = static_cast<uint32_t>(mFontAtlas.TexWidth);
    font_texture_description.height = static_cast<uint32_t>(mFontAtlas.TexHeight);
    font_texture_description.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    mFontTexture = mDevice->newTexture(font_texture_description);
    mFontTexture->replaceRegion(pixels.data(), pixels.size() * sizeof(uint32_t));

    vk::SamplerCreateInfo font_sampler_description = {};
    font_sampler_description.setMagFilter(vk::Filter::eLinear);
    font_sampler_description.setMinFilter(vk::Filter::eLinear);
    font_sampler_description.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    font_sampler_description.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    font_sampler_description.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    font_sampler_description.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    mFontSampler = mDevice->newSampler(font_sampler_description);

    mFontAtlas.SetTexID(mFontTexture.get());

    mDrawListSharedData.Font = mFontAtlas.Fonts.front();
    mDrawListSharedData.FontSize = mDrawListSharedData.Font->FontSize;
    mDrawListSharedData.TexUvWhitePixel = mFontAtlas.TexUvWhitePixel;
}

void UIRenderer::buildShaders() {
    auto vertexLibrary = mDevice->newLibrary(Assets::readFile("shaders/gui.vert.spv"));
    auto fragmentLibrary = mDevice->newLibrary(Assets::readFile("shaders/gui.frag.spv"));

    auto vertexFunction = vertexLibrary->newFunction("main");
    auto fragmentFunction = fragmentLibrary->newFunction("main");

    gfx::RenderPipelineVertexDescription vertexDescription = {};
    vertexDescription.layouts = {{
        vk::VertexInputBindingDescription{0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex}
    }};
    vertexDescription.attributes = {{
        vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos)},
        vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv)},
        vk::VertexInputAttributeDescription{2, 0, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col)}
    }};

    gfx::RenderPipelineStateDescription description = {};
    description.vertexDescription = vertexDescription;
    description.inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

    description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
    description.depthStencilState.setDepthTestEnable(false);
    description.attachments[0].setBlendEnable(true);
    description.attachments[0].setColorBlendOp(vk::BlendOp::eAdd);
    description.attachments[0].setAlphaBlendOp(vk::BlendOp::eAdd);
    description.attachments[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    description.attachments[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);

    description.setVertexFunction(vertexFunction);
    description.setFragmentFunction(fragmentFunction);

    mRenderPipelineState = mDevice->newRenderPipelineState(description);
    mDescriptorSet = mDevice->newDescriptorSet(mRenderPipelineState->vkDescriptorSetLayouts.front(), {
        vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1},
        vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1},
    });

    mDescriptorSet->setSampler(mFontSampler, 0);
    mDescriptorSet->setTexture(mFontTexture, 1);
}

void UIRenderer::resetForNewFrame() {
    mDrawList._ResetForNewFrame();
    mDrawList.PushClipRect(ImVec2(0, 0), ImVec2(mScreenSize.width, mScreenSize.height));
    mDrawList.PushTextureID(mFontAtlas.TexID);
}

auto UIRenderer::drawList() -> ImDrawList* {
    return &mDrawList;
}

void UIRenderer::draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd) {
    if (mDrawList.IdxBuffer.Size == 0) {
        return;
    }

    mIndexBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer,
        mDrawList.IdxBuffer.Data,
        mDrawList.IdxBuffer.size_in_bytes(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    mVertexBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eVertexBuffer,
        mDrawList.VtxBuffer.Data,
        mDrawList.VtxBuffer.size_in_bytes(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    GuiShaderData gui_shader_data = {};
    gui_shader_data.scale = mScale * 2.0F / simd::float2{mScreenSize.width, mScreenSize.height};

    cmd->setRenderPipelineState(mRenderPipelineState);
    cmd->bindDescriptorSet(mDescriptorSet, 0);
    cmd->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GuiShaderData), &gui_shader_data);
    cmd->bindVertexBuffer(0, mVertexBuffer, 0);
    cmd->bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint16);

    for (auto& drawCmd : std::span(mDrawList.CmdBuffer.Data, mDrawList.CmdBuffer.Size)) {
        cmd->drawIndexed(drawCmd.ElemCount, 1, drawCmd.IdxOffset, static_cast<int32_t>(drawCmd.VtxOffset), 0);
    }
}

void UIRenderer::setScale(float_t scale) {
    mScale = scale;
}

void UIRenderer::setScreenSize(const UISize& size) {
    mScreenSize = size;
}