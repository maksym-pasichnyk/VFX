#include "Assets.hpp"
#include "GraphView.hpp"
#include "UIContext.hpp"
#include "UIRenderer.hpp"
#include "Application.hpp"

struct Game : Application {
public:
    Game() : Application("Nodes-05") {
        uiRenderer = gfx::TransferPtr(new UIRenderer(device));
        graphView = gfx::TransferPtr(new GraphView());

        auto nodeA = graphView->addNode("Node A");
        nodeA->addOutput("Return Value", GraphView::Port::Capacity::eMulti);
        nodeA->setPosition(UIPoint(125, 110));

        auto nodeB = graphView->addNode("Node B");
        nodeB->addOutput("Return Value", GraphView::Port::Capacity::eMulti);
        nodeB->setPosition(UIPoint(125, 710));

        auto nodeC = graphView->addNode("Node C");
        nodeC->addInput("A", GraphView::Port::Capacity::eSingle);
        nodeC->addInput("B", GraphView::Port::Capacity::eSingle);
        nodeC->addOutput("Return Value", GraphView::Port::Capacity::eMulti);
        nodeC->setPosition(UIPoint(825, 330));
    }

public:
    void update(float_t dt) override {
        uiRenderer->setCurrentContext();
        uiRenderer->setScreenSize(getUISize(getWindowSize()));

        ImGui::GetIO().DeltaTime = dt;
        graphView->update();
    }

    void render() override {
        uiRenderer->setCurrentContext();

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

        auto ctx = gfx::TransferPtr(new UIContext(uiRenderer->drawList()));

        auto body = graphView
            ->overlay(
                gfx::TransferPtr(new Text(fmt::format("FPS {:.0F}", 1.0F / average), ctx->drawList()->_Data->Font, 24.0F))
                    ->fixedSize(true, true),
                Alignment::topLeading()
            )
            ->frame(std::nullopt, std::nullopt);

        uiRenderer->resetForNewFrame();
        body->draw(ctx, body->size(ProposedSize(getUISize(getWindowSize()))));
        uiRenderer->draw(commandBuffer);

        commandBuffer->endRendering();

        commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer->end();
        commandBuffer->submit();
        commandBuffer->present(drawable);
        commandBuffer->waitUntilCompleted();
    }

    void mouseUp(SDL_MouseButtonEvent* event) override {
        graphView->mouseUp(event);
    }

    void mouseDown(SDL_MouseButtonEvent* event) override {
        graphView->mouseDown(event);
    }

    void mouseWheel(SDL_MouseWheelEvent* event) override {
        graphView->mouseWheel(event);
    }

private:
    sp<GraphView> graphView;
    sp<UIContext> uiContext;
    sp<UIRenderer> uiRenderer;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}