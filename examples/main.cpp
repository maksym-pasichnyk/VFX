#include <vector>

#include "queue.hpp"
#include "window.hpp"
#include "widgets.hpp"
#include "drawable.hpp"
#include "renderer.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

struct Demo : vfx::Application, vfx::WindowDelegate {
    Arc<vfx::Window> window{};
    Arc<vfx::Context> context{};
    Arc<vfx::Swapchain> swapchain{};

    Arc<Widgets> widgets{};
    Arc<Renderer> renderer{};

    Arc<vfx::CommandQueue> graphics_command_queue{};
    Arc<vfx::PipelineState> present_pipeline_state{};

    // todo: bindless
    vk::DescriptorPool descriptor_pool{};
    std::vector<vk::DescriptorSet> descriptor_sets{};

    Demo() {
        window = Arc<vfx::Window>::alloc(vfx::WindowDescription{
            .title = "Demo",
            .width = 800,
            .height = 600,
            .resizable = true,
        });
        window->delegate = this;

        context = vfx::createSystemDefaultContext();

        swapchain = Arc<vfx::Swapchain>::alloc(vfx::SwapchainDescription{
            .context = context,
            .surface = window->makeSurface(context),
            .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
            .pixelFormat = vk::Format::eB8G8R8A8Unorm,
            .presentMode = vk::PresentModeKHR::eFifo
        });

        renderer = Arc<Renderer>::alloc(context, swapchain->getPixelFormat());
        renderer->setDrawableSize(swapchain->getDrawableSize());

        widgets = Arc<Widgets>::alloc(context, window);

        graphics_command_queue = context->makeCommandQueue(16);

        createPresentPipelineState();
    }

    ~Demo() override {
        context->logical_device.waitIdle();
        context->logical_device.destroyDescriptorPool(descriptor_pool);
    }

    void run() {
        f64 time = glfwGetTime();
        while (!window->windowShouldClose()) {
            f64 now = glfwGetTime();
            f32 dt = f32(now - time);
            time = now;

            pollEvents();

            widgets->beginFrame();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(0, 0));
            ImGui::Begin("Stats");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
            widgets->endFrame();

            draw();
        }
    }

    void draw() {
        auto cmd = graphics_command_queue->makeCommandBuffer();
        cmd->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlags{});
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            image_memory_barrier.setOldLayout(vk::ImageLayout::eUndefined);
            image_memory_barrier.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(renderer->colorAttachmentTexture->image);
            image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            cmd->handle.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {},
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(renderer->drawableSize);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(renderer->drawableSize.width));
        viewport.setHeight(f32(renderer->drawableSize.height));
        viewport.setMaxDepth(1.f);

        cmd->setScissor(0, area);
        cmd->setViewport(0, viewport);

        auto renderer_rendering_info = vfx::RenderingInfo{};
        renderer_rendering_info.renderArea = area;
        renderer_rendering_info.layerCount = 1;
        renderer_rendering_info.colorAttachments[0].texture = renderer->colorAttachmentTexture;
        renderer_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        renderer_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        renderer_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        renderer_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

        cmd->beginRendering(renderer_rendering_info);
        renderer->draw(cmd);
        cmd->endRendering();

        auto widgets_rendering_info = vfx::RenderingInfo{};
        widgets_rendering_info.renderArea = area;
        widgets_rendering_info.layerCount = 1;

        widgets_rendering_info.colorAttachments[0].texture = renderer->colorAttachmentTexture;
        widgets_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        widgets_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
        widgets_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        cmd->beginRendering(widgets_rendering_info);
        widgets->draw(cmd);
        cmd->endRendering();

        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            image_memory_barrier.setDstAccessMask(vk::AccessFlags{});
            image_memory_barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
            image_memory_barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(renderer->colorAttachmentTexture->image);
            image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            cmd->handle.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        auto drawable = swapchain->nextDrawable();

        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlags{});
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            image_memory_barrier.setOldLayout(vk::ImageLayout::eUndefined);
            image_memory_barrier.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(drawable->texture->image);
            image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            cmd->handle.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {},
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        final_render_pass(cmd, drawable);

        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            image_memory_barrier.setDstAccessMask(vk::AccessFlags{});
            image_memory_barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
            image_memory_barrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(drawable->texture->image);
            image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            cmd->handle.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        cmd->end();
        cmd->submit();
        cmd->present(drawable);
    }

    void final_render_pass(vfx::CommandBuffer* cmd, vfx::Drawable* drawable) {
        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(drawable->texture->size);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawable->texture->size.width));
        viewport.setHeight(f32(drawable->texture->size.height));

        cmd->setScissor(0, area);
        cmd->setViewport(0, viewport);

        cmd->setPipelineState(present_pipeline_state);
        cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, present_pipeline_state->pipelineLayout, 0, descriptor_sets, {});

        auto present_rendering_info = vfx::RenderingInfo{};
        present_rendering_info.renderArea = area;
        present_rendering_info.layerCount = 1;
        present_rendering_info.colorAttachments[0].texture = drawable->texture;
        present_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        present_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        present_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        present_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

        cmd->beginRendering(present_rendering_info);
        cmd->draw(6, 1, 0, 0);
        cmd->endRendering();
    }

    void createPresentPipelineState() {
        vfx::PipelineStateDescription description{};

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.rasterizationState.lineWidth = 1.0f;

        description.vertexFunction = context->makeFunction(Assets::read_file("shaders/blit.vert.spv"), "main");
        description.fragmentFunction = context->makeFunction(Assets::read_file("shaders/blit.frag.spv"), "main");

        present_pipeline_state = context->makePipelineState(description);

        auto pool_sizes = std::array{
            vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1}
        };
        auto pool_create_info = vk::DescriptorPoolCreateInfo{};
        pool_create_info.setMaxSets(1);
        pool_create_info.setPoolSizes(pool_sizes);
        descriptor_pool = context->logical_device.createDescriptorPool(pool_create_info, nullptr);

        auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
        ds_allocate_info.setDescriptorPool(descriptor_pool);
        ds_allocate_info.setSetLayouts(present_pipeline_state->descriptorSetLayouts);
        descriptor_sets = context->logical_device.allocateDescriptorSets(ds_allocate_info);

        updateAttachmentDescriptors();
    }

    void updateAttachmentDescriptors() {
        const auto image_info = vk::DescriptorImageInfo{
            .sampler = renderer->sampler->handle,
            .imageView = renderer->colorAttachmentTexture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = descriptor_sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &image_info
        };
        context->logical_device.updateDescriptorSets(write_descriptor_set, {});
    }

    void windowDidResize() override {
        context->logical_device.waitIdle();

        swapchain->freeDrawables();
        swapchain->makeDrawables();

        renderer->setDrawableSize(swapchain->getDrawableSize());

        updateAttachmentDescriptors();
        draw();
    }
};

auto main() -> i32 {
    try {
        Demo demo{};
        demo.run();
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
