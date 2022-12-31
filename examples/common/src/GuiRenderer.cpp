#include "GuiRenderer.hpp"
#include "Assets.hpp"
#include "simd.hpp"
#include "NotSwiftUI/View.hpp"
#include "NotSwiftUI/UIContext.hpp"

struct GuiShaderData {
    simd::float2 scale;
};

GuiRenderer::GuiRenderer(gfx::SharedPtr<gfx::Device> device) : mDevice(std::move(device)) {
    buildShaders();
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
    description.attachments[0].setBlendEnable(true);
    description.attachments[0].setColorBlendOp(vk::BlendOp::eAdd);
    description.attachments[0].setAlphaBlendOp(vk::BlendOp::eAdd);
    description.attachments[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
    description.attachments[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);

    description.setVertexFunction(vertexFunction);
    description.setFragmentFunction(fragmentFunction);

    mRenderPipelineState = mDevice->newRenderPipelineState(description);
}

void GuiRenderer::draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd) {
    // todo: content scale

    auto ctx = gfx::TransferPtr(new UIContext());

    auto view =
        gfx::TransferPtr(new HStack({
            gfx::TransferPtr(new VStack({
                gfx::TransferPtr(new HStack({
                    gfx::TransferPtr(new Circle()),
                    gfx::TransferPtr(new Circle()),
                    gfx::TransferPtr(new Circle()),
                })),
                gfx::TransferPtr(new HStack({
                    gfx::TransferPtr(new Circle()),
                    gfx::TransferPtr(new Circle()),
                    gfx::TransferPtr(new Circle()),
                })),
                gfx::TransferPtr(new HStack({
                    gfx::TransferPtr(new Circle()),
                    gfx::TransferPtr(new Circle()),
                    gfx::TransferPtr(new Circle()),
                })),
            })),
        }))
        ->border(UIColor(1, 1, 1, 0.25F), 4);

    view->size(ProposedSize(mScreenSize));
    view->draw(ctx, mScreenSize);

    mIndexBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer,
        ctx->mDrawList.IdxBuffer.Data,
        ctx->mDrawList.IdxBuffer.size_in_bytes(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    mVertexBuffer = mDevice->newBuffer(
        vk::BufferUsageFlagBits::eVertexBuffer,
        ctx->mDrawList.VtxBuffer.Data,
        ctx->mDrawList.VtxBuffer.size_in_bytes(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    GuiShaderData gui_shader_data = {};
    gui_shader_data.scale = 2.0F / simd::float2{mScreenSize.width, mScreenSize.height};

    cmd->setRenderPipelineState(mRenderPipelineState);
    cmd->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GuiShaderData), &gui_shader_data);
    cmd->bindVertexBuffer(0, mVertexBuffer, 0);
    cmd->bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint16);

    for (size_t i = 0; i < ctx->mDrawList.CmdBuffer.Size; ++i) {
        cmd->drawIndexed(ctx->mDrawList.CmdBuffer[i].ElemCount, 1, ctx->mDrawList.CmdBuffer[i].IdxOffset, ctx->mDrawList.CmdBuffer[i].VtxOffset, 0);
    }
}

void GuiRenderer::setScreenSize(const vk::Extent2D& size) {
    mScreenSize = UISize(static_cast<float_t>(size.width), static_cast<float_t>(size.height));
}