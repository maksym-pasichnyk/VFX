#include "Assets.hpp"
#include "Renderer.hpp"
#include "GraphView.hpp"

using Capacity = GraphView::Port::Capacity;

Renderer::Renderer(gfx::SharedPtr<gfx::Device> device_) : device(std::move(device_)) {
    commandQueue = device->newCommandQueue();
    commandBuffer = commandQueue->commandBuffer();

    mGuiRenderer = gfx::TransferPtr(new UIRenderer(device));
    mGraphView = gfx::TransferPtr(new GraphView());

    auto nodeA = mGraphView->addNode("Node A");
    nodeA->addOutput("", Capacity::eSingle);
    nodeA->setPosition(UIPoint(125, 110));

    auto nodeB = mGraphView->addNode("Node B");
    nodeB->addOutput("", Capacity::eMulti);
    nodeB->setPosition(UIPoint(125, 710));

    auto nodeC = mGraphView->addNode("Node C");
    nodeC->addInput("A", Capacity::eMulti);
    nodeC->addInput("B", Capacity::eMulti);
    nodeC->addOutput("Result", Capacity::eMulti);
    nodeC->setPosition(UIPoint(825, 330));

    mGraphView->addLink(nodeA, nodeC, 0, 0);
    mGraphView->addLink(nodeB, nodeC, 0, 0);
    mGraphView->addLink(nodeB, nodeC, 0, 1);
    mGraphView->setZoomScale(2.0F);
}

void Renderer::update(float_t dt) {
    mGraphView->update(dt);
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

    commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

    commandBuffer->beginRendering(rendering_info);
    commandBuffer->setScissor(0, rendering_area);
    commandBuffer->setViewport(0, rendering_viewport);

    auto ctx = gfx::TransferPtr(new UIContext(mGuiRenderer->drawList()));

    auto body = mGraphView->frame(mScreenSize.width, mScreenSize.height);
    mGuiRenderer->resetForNewFrame();
    body->draw(ctx, body->size(ProposedSize(mScreenSize)));
    mGuiRenderer->draw(commandBuffer);

    commandBuffer->endRendering();

    commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
    commandBuffer->end();
    commandBuffer->submit();
    commandBuffer->present(drawable);
    commandBuffer->waitUntilCompleted();
}

void Renderer::screenResized(const vk::Extent2D& size) {
    mScreenSize = UISize(static_cast<float_t>(size.width), static_cast<float_t>(size.height));
    mGuiRenderer->setScreenSize(mScreenSize);
}
