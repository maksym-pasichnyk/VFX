#include "tiny_gltf.h"
#include "fmt/format.h"
#include "UIContext.hpp"
#include "UIRenderer.hpp"
#include "Application.hpp"
#include "NotSwiftUI/View.hpp"

struct Button : View {
private:
    sp<Text> text;
    UISize size_;

public:
    explicit Button(sp<Text> text, UISize size_)
        : text(std::move(text)), size_(size_) {}

public:
    void _draw(const sp<UIContext> &context, const UISize &size) override {
        context->drawRectFilled(size, 5.0F);
        auto textSize = text->size(ProposedSize(size));
        align(context, textSize, size, Alignment::center());
        context->saveState();
        context->setFillColor(UIColor(0.0F, 0.0F, 0.0F, 1.0F));
        text->draw(context, textSize);
        context->restoreState();
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        return size_;
    }
};

struct Game : Application {
public:
    Game() : Application("Menu-06") {
        uiRenderer = gfx::TransferPtr(new UIRenderer(device));

        auto font = uiRenderer->drawList()->_Data->Font;

        content =
            gfx::TransferPtr(new VStack({
                gfx::TransferPtr(new Button(gfx::TransferPtr(new Text("Start", font, 24.0F)), UISize(150, 50))),
                gfx::TransferPtr(new Button(gfx::TransferPtr(new Text("Settings", font, 24.0F)), UISize(150, 50))),
                gfx::TransferPtr(new Button(gfx::TransferPtr(new Text("Exit", font, 24.0F)), UISize(150, 50))),
            }, HorizontalAlignment::center(), 5.0F))
            ->frame(std::nullopt, std::nullopt);
    }

public:
    void update(float_t dt) override {
        uiRenderer->setCurrentContext();
        uiRenderer->setScreenSize(getUISize(getWindowSize()));

        ImGui::GetIO().DeltaTime = dt;
    }

    void render() override {
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

        uiRenderer->resetForNewFrame();
        content->draw(ctx, getUISize(getWindowSize()));
        uiRenderer->draw(commandBuffer);

        commandBuffer->endRendering();

        commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer->end();
        commandBuffer->submit();
        commandBuffer->present(drawable);
        commandBuffer->waitUntilCompleted();
    }

private:
    sp<View> content;
    sp<UIRenderer> uiRenderer;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}