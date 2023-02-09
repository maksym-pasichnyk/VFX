#include "Application.hpp"

struct Game : Application {
public:
    Game() : Application("Circles-02") {
        content =
            HStack(VerticalAlignment::center(), std::nullopt, {
                VStack(HorizontalAlignment::center(), std::nullopt, {
                    HStack(VerticalAlignment::center(), std::nullopt, {
                        Shape(sp<Circle>::of()),
                        Shape(sp<Circle>::of()),
                        Shape(sp<Circle>::of()),
                    }),
                    HStack(VerticalAlignment::center(), std::nullopt, {
                        Shape(sp<Circle>::of()),
                        Shape(sp<Circle>::of()),
                        Shape(sp<Circle>::of()),
                    }),
                    HStack(VerticalAlignment::center(), std::nullopt, {
                        Shape(sp<Circle>::of()),
                        Shape(sp<Circle>::of()),
                        Shape(sp<Circle>::of()),
                    })
                })
            })
            ->border(Color{1, 1, 1, 0.25F}, 4);
    }

public:
    void update(float_t dt) override {
        uiRenderer->setCurrentContext();
        uiRenderer->setScreenSize(getUISize(getWindowSize()));

        ImGui::GetIO().DeltaTime = dt;
    }

    void render() override {
        auto drawable = swapchain.nextDrawable();
        auto drawableSize = swapchain.drawableSize();

        vk::Rect2D rendering_area = {};
        rendering_area.setOffset(vk::Offset2D{0, 0});
        rendering_area.setExtent(drawableSize);

        vk::Viewport rendering_viewport = {};
        rendering_viewport.setWidth(static_cast<float_t>(drawableSize.width));
        rendering_viewport.setHeight(static_cast<float_t>(drawableSize.height));
        rendering_viewport.setMinDepth(0.0f);
        rendering_viewport.setMaxDepth(1.0f);

        gfx::RenderingInfo rendering_info = {};
        rendering_info.renderArea = rendering_area;
        rendering_info.layerCount = 1;
        rendering_info.colorAttachments[0].texture = drawable.texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        commandBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandBuffer.imageBarrier(drawable.texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        commandBuffer.beginRendering(rendering_info);
        commandBuffer.setScissor(0, rendering_area);
        commandBuffer.setViewport(0, rendering_viewport);

        auto ctx = sp<UIContext>::of(uiRenderer->drawList());

        uiRenderer->resetForNewFrame();
        _drawView(content);
        uiRenderer->draw(commandBuffer);

        commandBuffer.endRendering();

        commandBuffer.imageBarrier(drawable.texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer.end();
        commandBuffer.submit();
        commandBuffer.present(drawable);
        commandBuffer.waitUntilCompleted();
    }

private:
    sp<View> content;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}