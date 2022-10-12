#include <vector>

#include "queue.hpp"
#include "window.hpp"
#include "widgets.hpp"
#include "drawable.hpp"
#include "renderer.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

struct Demo : vfx::Application {
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

        context = vfx::createSystemDefaultContext();
        swapchain = Arc<vfx::Swapchain>::alloc(context, window);

        renderer = Box<Renderer>::alloc(context, swapchain->getPixelFormat());
        renderer->setDrawableSize(swapchain->getDrawableSize());

        widgets = Box<Widgets>::alloc(context, window);

        graphics_command_queue = context->makeCommandQueue(16);

        create_present_pipeline_state();
    }

    ~Demo() override {
        context->logical_device.waitIdle();
        context->logical_device.destroyDescriptorPool(descriptor_pool);
        context->freePipelineState(present_pipeline_state);
        context->freeCommandQueue(graphics_command_queue);
    }

    void run() {
        f64 time = glfwGetTime();
        while (!window->windowShouldClose()) {
            f64 now = glfwGetTime();
            f32 dt = f32(now - time);
            time = now;

            pollEvents();

            draw();

            // todo: move to window resize event
            if (swapchain->getDrawableSize() != renderer->drawableSize) {
                context->logical_device.waitIdle();

                renderer->setDrawableSize(swapchain->getDrawableSize());
                update_descriptors();
            }
        }
    }

    void draw() {
        auto cmd = graphics_command_queue->makeCommandBuffer();
        cmd->handle.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        renderer->beginRendering(cmd);
        renderer->draw(cmd);

        widgets->begin_frame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::Begin("Stats");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        widgets->end_frame();

        widgets->draw(cmd);
        renderer->endRendering(cmd);

        auto drawable = swapchain->nextDrawable();
        final_render_pass(cmd, drawable);
        cmd->handle.end();
        cmd->submit();
        cmd->present(drawable);
    }

    void final_render_pass(vfx::CommandBuffer* cmd, vfx::Drawable* drawable) {
        auto clear_values = std::array{
            vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f})),
        };

        auto render_area = vk::Rect2D{};
        render_area.setExtent(drawable->texture->size);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(swapchain->getDefaultRenderPass()->handle);
        begin_info.setFramebuffer(drawable->framebuffer);
        begin_info.setRenderArea(render_area);
        begin_info.setClearValues(clear_values);

        cmd->beginRenderPass(begin_info, vk::SubpassContents::eInline);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawable->texture->size.width));
        viewport.setHeight(f32(drawable->texture->size.height));

        cmd->handle.setViewport(0, 1, &viewport);
        cmd->handle.setScissor(0, 1, &render_area);

        cmd->setPipelineState(present_pipeline_state);
        cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, present_pipeline_state->pipelineLayout, 0, descriptor_sets, {});

        cmd->draw(6, 1, 0, 0);
        cmd->endRenderPass();
    }

    void create_present_pipeline_state() {
        vfx::PipelineStateDescription description{};

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.inputAssemblyState.primitiveRestartEnable = false;

        description.rasterizationState.depthClampEnable        = false;
        description.rasterizationState.rasterizerDiscardEnable = false;
        description.rasterizationState.polygonMode             = vk::PolygonMode::eFill;
        description.rasterizationState.cullMode                = vk::CullModeFlagBits::eNone;
        description.rasterizationState.frontFace               = vk::FrontFace::eCounterClockwise;
        description.rasterizationState.depthBiasEnable         = false;
        description.rasterizationState.depthBiasConstantFactor = 0.0f;
        description.rasterizationState.depthBiasClamp          = 0.0f;
        description.rasterizationState.depthBiasSlopeFactor    = 0.0f;
        description.rasterizationState.lineWidth               = 1.0f;

        description.depthStencilState.depthTestEnable       = false;
        description.depthStencilState.depthWriteEnable      = false;
        description.depthStencilState.depthCompareOp        = vk::CompareOp::eNever;
        description.depthStencilState.depthBoundsTestEnable = false;
        description.depthStencilState.stencilTestEnable     = false;
        description.depthStencilState.front                 = vk::StencilOpState{};
        description.depthStencilState.back                  = vk::StencilOpState{};
        description.depthStencilState.minDepthBounds        = 0.0f;
        description.depthStencilState.maxDepthBounds        = 0.0f;

        description.shaders.emplace_back(vfx::ShaderDescription{
            .bytes = Assets::read_file("shaders/blit.vert.spv"),
            .entry = "main",
            .stage = vk::ShaderStageFlagBits::eVertex
        });
        description.shaders.emplace_back(vfx::ShaderDescription{
            .bytes = Assets::read_file("shaders/blit.frag.spv"),
            .entry = "main",
            .stage = vk::ShaderStageFlagBits::eFragment
        });
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

        update_descriptors();
    }

    void update_descriptors() {
        const auto image_info = vk::DescriptorImageInfo{
            .sampler = renderer->sampler,
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
