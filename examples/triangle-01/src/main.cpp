#include "simd.hpp"
#include "tiny_gltf.h"
#include "fmt/format.h"

#include "Assets.hpp"
#include "Application.hpp"
#include "NotSwiftUI/View.hpp"

struct Game : Application {
public:
    Game() : Application("Triangle-01") {
        buildShaders();
        buildBuffers();
    }

public:
    void update(float_t dt) override {
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

        commandBuffer->setRenderPipelineState(renderPipelineState);
        commandBuffer->bindDescriptorSet(descriptorSet, 0);

        commandBuffer->beginRendering(rendering_info);
        commandBuffer->setScissor(0, rendering_area);
        commandBuffer->setViewport(0, rendering_viewport);

        commandBuffer->draw(3, 1, 0, 0);
        commandBuffer->endRendering();

        commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer->end();
        commandBuffer->submit();
        commandBuffer->present(drawable);
        commandBuffer->waitUntilCompleted();
    }

private:
    void buildShaders() {
        auto pVertexLibrary = device->newLibrary(Assets::readFile("shaders/default.vert.spv"));
        auto pFragmentLibrary = device->newLibrary(Assets::readFile("shaders/default.frag.spv"));

        auto pVertexFunction = pVertexLibrary->newFunction("main");
        auto pFragmentFunction = pFragmentLibrary->newFunction("main");

        gfx::RenderPipelineStateDescription description = {};

        description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        description.attachments[0].setBlendEnable(false);

        description.setVertexFunction(pVertexFunction);
        description.setFragmentFunction(pFragmentFunction);

        renderPipelineState = device->newRenderPipelineState(description);
        descriptorSet = device->newDescriptorSet(renderPipelineState->mDescriptorSetLayouts[0], {
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1}
        });
    }

    void buildBuffers() {
        struct Vertex {
            simd::float3 position;
            simd::float3 color;
        };

        Vertex vertices[] = {
            {{-0.8F, +0.8F, +0.0F}, {1.0F, 0.0F, 0.0F}},
            {{+0.0F, -0.8F, +0.0F}, {0.0F, 1.0F, 0.0F}},
            {{+0.8F, +0.8F, +0.0F}, {0.0F, 0.0F, 1.0F}}
        };

        vertexBuffer = device->newBuffer(vk::BufferUsageFlagBits::eStorageBuffer, sizeof(vertices), VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
        std::memcpy(vertexBuffer->contents(), vertices, sizeof(vertices));
        vertexBuffer->didModifyRange(0, vertexBuffer->length());
        descriptorSet->setStorageBuffer(vertexBuffer, 0, 0);
    }

private:
    sp<View> content;
    sp<gfx::Buffer> vertexBuffer;
    sp<gfx::DescriptorSet> descriptorSet;
    sp<gfx::RenderPipelineState> renderPipelineState;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}