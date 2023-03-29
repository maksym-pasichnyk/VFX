#include "Assets.hpp"
#include "Application.hpp"

struct Game : Application {
public:
    Game() : Application("Triangle-01") {
        buildShaders();
        buildBuffers();
    }

public:
    void update(float dt) override {
    }

    void render() override {
        auto drawable = swapchain.nextDrawable();
        auto drawableSize = swapchain.drawableSize();

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
        rendering_info.colorAttachments[0].texture = drawable.texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        commandBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandBuffer.setImageLayout(drawable.texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        auto descriptor_set = commandBuffer.newDescriptorSet(renderPipelineState.shared->bind_group_layouts.front(), {
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1}
        });
        vk::DescriptorBufferInfo buffer_info = {};
        buffer_info.setBuffer(vertexBuffer.shared->raw);
        buffer_info.setOffset(0);
        buffer_info.setRange(VK_WHOLE_SIZE);

        vk::WriteDescriptorSet buffer_write_info = {};
        buffer_write_info.setDstSet(descriptor_set);
        buffer_write_info.setDstBinding(0);
        buffer_write_info.setDstArrayElement(0);
        buffer_write_info.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        buffer_write_info.setDescriptorCount(1);
        buffer_write_info.setPBufferInfo(&buffer_info);

        device.shared->raii.raw.updateDescriptorSets({buffer_write_info}, {}, device.shared->raii.dispatcher);

        commandBuffer.setRenderPipelineState(renderPipelineState);
        commandBuffer.bindDescriptorSet(descriptor_set, 0);

        commandBuffer.beginRendering(rendering_info);
        commandBuffer.setScissor(0, rendering_area);
        commandBuffer.setViewport(0, rendering_viewport);

        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRendering();

        commandBuffer.setImageLayout(drawable.texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer.end();
        commandBuffer.submit();
        commandBuffer.present(drawable);
        commandBuffer.waitUntilCompleted();
    }

private:
    void buildShaders() {
        auto vertexLibrary = device.newLibrary(Assets::readFile("shaders/default.vert.spv"));
        auto fragmentLibrary = device.newLibrary(Assets::readFile("shaders/default.frag.spv"));

        gfx::RenderPipelineStateDescription description;
        description.vertexFunction = vertexLibrary.newFunction("main");
        description.fragmentFunction = fragmentLibrary.newFunction("main");

        description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        description.colorBlendAttachments[0].setBlendEnable(false);

        renderPipelineState = device.newRenderPipelineState(description);
    }

    void buildBuffers() {
        struct Vertex {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 color;
        };

        Vertex vertices[] = {
            {{-0.8F, +0.8F, +0.0F}, {1.0F, 0.0F, 0.0F}},
            {{+0.0F, -0.8F, +0.0F}, {0.0F, 1.0F, 0.0F}},
            {{+0.8F, +0.8F, +0.0F}, {0.0F, 0.0F, 1.0F}}
        };

        vertexBuffer = device.newBuffer(vk::BufferUsageFlagBits::eStorageBuffer, sizeof(vertices), VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
        std::memcpy(vertexBuffer.contents(), vertices, sizeof(vertices));
        vertexBuffer.didModifyRange(0, vertexBuffer.length());
    }

private:
    sp<View> content;
    gfx::Buffer vertexBuffer;
    gfx::RenderPipelineState renderPipelineState;
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}