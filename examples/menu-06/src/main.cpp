#include "tiny_gltf.h"
#include "UIContext.hpp"
#include "UIRenderer.hpp"
#include "Application.hpp"
#include "NotSwiftUI/View.hpp"

#include <unordered_map>

struct Button : View {
private:
    sp<Text> text;
    UISize size_;

public:
    explicit Button(sp<Text> text, UISize size_)
        : text(std::move(text)), size_(size_) {}

public:
    void _draw(const sp<UIContext> &context, const UISize &size) override {
        auto textSize = text->_size(ProposedSize(size));
        auto translate = translation(textSize, size, Alignment::center());

        context->drawRectFilled(size, 5.0F);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        context->setFillColor(UIColor(0.0F, 0.0F, 0.0F, 1.0F));
        text->_draw(context, textSize);
        context->restoreState();
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        return size_;
    }
};

struct egui {
    struct SliderState {
        bool dragging = false;
    };

    inline static std::unordered_map<void*, SliderState> sliderStates;

    static void slider(sp<UIContext> ctx, std::reference_wrapper<float> current, float min, float max) {
        auto& state = sliderStates[&current.get()];

        UIPoint cursor(100.0F, 100.0F);

        float sliderHeight = 5.0F;
        float sliderWidth = 200.0F;
        float knobRadius = 10.0F;

        float sliderHalfHeight = sliderHeight * 0.5F;

        int x, y;
        Uint32 buttons = SDL_GetMouseState(&x, &y);

        if (state.dragging) {
            if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                auto mousePos = UIPoint(x, y) - cursor;
                current.get() = (mousePos.x / sliderWidth) * (max - min) + min;
            } else {
                state.dragging = false;
            }
        } else {
            if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                auto mousePos = UIPoint(x, y) - cursor;
                if (mousePos.x >= 0.0F && mousePos.x <= sliderWidth && mousePos.y >= -knobRadius && mousePos.y <= knobRadius) {
                    state.dragging = true;
                    current.get() = (mousePos.x / sliderWidth) * (max - min) + min;
                }
            }
        }

        current.get() = std::clamp(current.get(), min, max);

        ctx->saveState();
        ctx->translateBy(cursor.x, cursor.y);
        ctx->translateBy(0.0F, -sliderHalfHeight);
        ctx->drawRectFilled(UISize(sliderWidth, sliderHeight), 0.0F);
        ctx->translateBy(0.0F, +sliderHalfHeight);
        ctx->setFillColor(UIColor(1.0F, 0.0F, 0.0F, 1.0F));
        ctx->translateBy(-knobRadius, -knobRadius);
        float knobX = (current - min) / (max - min) * sliderWidth;
        ctx->translateBy(+knobX, 0.0F);
        ctx->drawCircleFilled(10.0F);
        ctx->translateBy(-knobX, 0.0F);
        ctx->translateBy(+knobRadius, +knobRadius);
        ctx->restoreState();
    }
};

struct Game : Application {
public:
    Game() : Application("Menu-06") {
        uiRenderer = sp<UIRenderer>::of(device);
        uiContext = sp<UIContext>::of(uiRenderer->drawList());

        auto font = uiRenderer->drawList()->_Data->Font;

        content =
            sp<VStack>::of(ViewBuilder::arrayOf(
                sp<Button>::of(sp<Text>::of("Start", font, 24.0F), UISize(150, 50)),
                sp<Button>::of(sp<Text>::of("Settings", font, 24.0F), UISize(150, 50)),
                sp<Button>::of(sp<Text>::of("Exit", font, 24.0F), UISize(150, 50))
            ), HorizontalAlignment::center(), 5.0F)
            ->frame(std::nullopt, std::nullopt);
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

        static float value = 0.0F;

        uiRenderer->resetForNewFrame();
        egui::slider(uiContext, std::ref(value), -1.0F, 1.0F);
        content->_draw(uiContext, getUISize(getWindowSize()));
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
    sp<UIContext> uiContext;
    sp<UIRenderer> uiRenderer;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}