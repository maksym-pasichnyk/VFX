#include "Assets.hpp"
#include "GraphView.hpp"
#include "Canvas.hpp"
#include "ImGuiBackend.hpp"
#include "Application.hpp"

#include "fmt/core.h"

struct Game : Application {
public:
    Game() : Application("Nodes-05") {
        graphView = rc<GraphView>::init();

        auto nodeA = graphView->addNode("Node A");
        nodeA->addOutput("Return Value", GraphView::Port::Capacity::eMulti);
        nodeA->setPosition(Point{125, 110});

        auto nodeB = graphView->addNode("Node B");
        nodeB->addOutput("Return Value", GraphView::Port::Capacity::eMulti);
        nodeB->setPosition(Point{125, 710});

        auto nodeC = graphView->addNode("Node C");
        nodeC->addInput("A", GraphView::Port::Capacity::eSingle);
        nodeC->addInput("B", GraphView::Port::Capacity::eSingle);
        nodeC->addOutput("Return Value", GraphView::Port::Capacity::eMulti);
        nodeC->setPosition(Point{825, 330});
    }

public:
    void update(float dt) override {
        ImGui::GetIO().DeltaTime = dt;
        graphView->update();
    }

    void render() override {
        imgui->setCurrentContext();

        auto drawable = swapchain->nextDrawable();
        auto drawableSize = swapchain->drawableSize();

        vk::Rect2D rendering_area = {};
        rendering_area.setOffset(vk::Offset2D{0, 0});
        rendering_area.setExtent(drawableSize);

        vk::Viewport rendering_viewport = {};
        rendering_viewport.setWidth(static_cast<float>(drawableSize.width));
        rendering_viewport.setHeight(static_cast<float>(drawableSize.height));
        rendering_viewport.setMinDepth(0.0f);
        rendering_viewport.setMaxDepth(1.0f);

        gfx::RenderingInfo rendering_info = {};
        rendering_info.renderArea = rendering_area;
        rendering_info.layerCount = 1;
        rendering_info.colorAttachments[0].texture = drawable->texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandBuffer->setImageLayout(drawable->texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        auto view = graphView->overlay(
            Text(fmt::format("FPS {:.0F}", 1.0F / average), 24.0F)->fixedSize(true, true),
            Alignment::topLeading()
        );

        imgui->resetForNewFrame();
        _drawView(view);

        auto encoder = commandBuffer->newRenderCommandEncoder(rendering_info);
        encoder->setScissor(0, rendering_area);
        encoder->setViewport(0, rendering_viewport);
        imgui->draw(encoder);
        encoder->endEncoding();

        commandBuffer->setImageLayout(drawable->texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
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
    rc<GraphView> graphView;
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}