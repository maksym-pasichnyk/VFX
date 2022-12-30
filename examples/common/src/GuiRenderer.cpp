#include "GuiRenderer.hpp"
#include "Assets.hpp"
#include "simd.hpp"
#include "NotSwiftUI/View.hpp"
#include "NotSwiftUI/UIContext.hpp"

struct GuiShaderData {
    simd::float2 mScale;
    simd::float2 mTransform;
};

GuiRenderer::GuiRenderer(gfx::SharedPtr<gfx::Device> device) : mDevice(std::move(device)) {
    vk::SamplerCreateInfo sampler_create_info = {};
    sampler_create_info.setMagFilter(vk::Filter::eLinear);
    sampler_create_info.setMinFilter(vk::Filter::eLinear);
    sampler_create_info.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    sampler_create_info.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    sampler_create_info.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    sampler_create_info.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    sampler_create_info.setMinLod(-1000);
    sampler_create_info.setMaxLod(+1000);
    sampler_create_info.setMaxAnisotropy(1.0F);
    mTextureSampler = mDevice->newSampler(sampler_create_info);

    buildGui();
    buildShaders();
}

GuiRenderer::~GuiRenderer() {}

void GuiRenderer::buildGui() {
    mUIContext = gfx::TransferPtr(new UIContext());

    auto view = gfx::TransferPtr(new Rectangle())
        ->frame(100, 100)
        ->border({1, 0, 0, 1}, 2)
        ->frame(300, 300, Alignment::center())
        ->border({0, 1, 0, 1}, 2);

    auto body = view->frame(800, 600);
    body->draw(mUIContext, {0, 0}, {800, 600});

    mIndexBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer,
        mUIContext->draw_list.IdxBuffer.Data,
        mUIContext->draw_list.IdxBuffer.size_in_bytes(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    mVertexBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eVertexBuffer,
        mUIContext->draw_list.VtxBuffer.Data,
        mUIContext->draw_list.VtxBuffer.size_in_bytes(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

void GuiRenderer::buildShaders() {
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
    description.attachments[0].setBlendEnable(false);
//    description.attachments[0].setColorBlendOp(vk::BlendOp::eAdd);
//    description.attachments[0].setSrcColorBlendFactor(vk::BlendFactor::eOne);
//    description.attachments[0].setDstColorBlendFactor(vk::BlendFactor::eOne);

    description.setVertexFunction(vertexFunction);
    description.setFragmentFunction(fragmentFunction);

    mRenderPipelineState = mDevice->newRenderPipelineState(description);
}

void GuiRenderer::draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd) {
    // todo: content scale

    GuiShaderData gui_shader_data = {};
    gui_shader_data.mScale = 2.0F / simd::float2{800, 600};

    cmd->setRenderPipelineState(mRenderPipelineState);
    cmd->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GuiShaderData), &gui_shader_data);
    cmd->bindVertexBuffer(0, mVertexBuffer, 0);
    cmd->bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint16);
    cmd->drawIndexed(mUIContext->draw_list.IdxBuffer.size(), 1, 0, 0, 0);
}