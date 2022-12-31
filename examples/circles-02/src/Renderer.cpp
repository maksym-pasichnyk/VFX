#include "Renderer.hpp"

#include "Assets.hpp"

Renderer::Renderer(gfx::SharedPtr<gfx::Device> device) : mDevice(std::move(device)) {
    mCommandQueue = mDevice->newCommandQueue();
    mCommandBuffer = mCommandQueue->commandBuffer();

    mGuiRenderer = gfx::TransferPtr(new GuiRenderer(mDevice));
}

void Renderer::draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain) {
    auto drawable = swapchain->nextDrawable();
    auto drawableSize = swapchain->drawableSize();

    vk::Rect2D rendering_area = {};
    rendering_area.setOffset(vk::Offset2D{0, 0});
    rendering_area.setExtent(drawableSize);

    vk::Viewport rendering_viewport = {};
    rendering_viewport.setWidth(static_cast<float_t>(drawableSize.width));
    rendering_viewport.setHeight(static_cast<float_t>(drawableSize.height));
    rendering_viewport.setMinDepth(0.0f);
    rendering_viewport.setMaxDepth(1.0f);

    gfx::RenderingInfo rendering_info = {};
    rendering_info.setRenderArea(rendering_area);
    rendering_info.setLayerCount(1);
    rendering_info.colorAttachments()[0].setTexture(drawable->texture());
    rendering_info.colorAttachments()[0].setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
    rendering_info.colorAttachments()[0].setLoadOp(vk::AttachmentLoadOp::eClear);
    rendering_info.colorAttachments()[0].setStoreOp(vk::AttachmentStoreOp::eStore);

    mCommandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    mCommandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

    mCommandBuffer->beginRendering(rendering_info);
    mCommandBuffer->setScissor(0, rendering_area);
    mCommandBuffer->setViewport(0, rendering_viewport);

    mGuiRenderer->draw(mCommandBuffer);

    mCommandBuffer->endRendering();

    mCommandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
    mCommandBuffer->end();
    mCommandBuffer->submit();
    mCommandBuffer->present(drawable);
    mCommandBuffer->waitUntilCompleted();
}

void Renderer::setScreenSize(const vk::Extent2D& size) {
    mGuiRenderer->setScreenSize(size);
}

