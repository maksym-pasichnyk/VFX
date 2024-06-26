#include "Application.hpp"

struct Game : Application {
public:
    Game() : Application("Menu-06") {
        content =
            VStack(HorizontalAlignment::center(), 5.0F, {
                Button(Text("Start", 24.0F))->frame(150, 50),
                Button(Text("Settings", 24.0F))->frame(150, 50),
                Button(Text("Exit", 24.0F))->frame(150, 50)
            });
    }

public:
    void update(float dt) override {
        ImGui::GetIO().DeltaTime = dt;
    }

    void render() override {
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

        imgui->resetForNewFrame();
        _drawView(content);

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

private:
    rc<View> content;
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}