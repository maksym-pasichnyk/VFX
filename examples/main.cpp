#include <map>

#include "queue.hpp"
#include "widgets.hpp"
#include "display.hpp"
#include "renderer.hpp"

struct Demo {
    Arc<Display> display{};
    Arc<vfx::Context> context{};
    Arc<vfx::Swapchain> swapchain{};

    Arc<Renderer> renderer{};
    Arc<Widgets> widgets{};

    Arc<vfx::CommandQueue> graphics_command_queue{};
    Arc<vfx::Material> present_swapchain_material{};

    Demo() {
        display = Arc<Display>::alloc(800, 600, "Demo", true);
        context = Arc<vfx::Context>::alloc(vfx::ContextDescription{
            .app_name = "Demo",
            .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
            .layers = {
                "VK_LAYER_KHRONOS_validation"
            },
            .extensions = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                "VK_EXT_metal_surface",
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            },
            .enable_debug = true
        });
        swapchain = Box<vfx::Swapchain>::alloc(*context, *display);
        renderer = Box<Renderer>::alloc(*context, swapchain->pixelFormat);
        renderer->setDrawableSize(swapchain->drawableSize);

        widgets = Box<Widgets>::alloc(*context, *display, *renderer);

        graphics_command_queue = context->makeCommandQueue(16);

        create_present_swapchain_material();
    }

    ~Demo() {
        context->freeMaterial(present_swapchain_material);
        context->freeCommandQueue(graphics_command_queue);
    }

    void run() {
        f64 time = glfwGetTime();
        while (!display->should_close()) {
            f64 now = glfwGetTime();
            f32 dt = f32(now - time);
            time = now;

            display->poll_events();

            draw();
        }
        context->logical_device.waitIdle();
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

        widgets->draw(cmd->handle);
        renderer->endRendering(cmd);

        auto drawable = swapchain->nextDrawable();
        final_render_pass(cmd->handle, drawable);
        cmd->handle.end();
        cmd->submit();
        cmd->present(drawable);

        if (swapchain->drawableSize != renderer->drawableSize) {
            context->logical_device.waitIdle();

            renderer->setDrawableSize(swapchain->drawableSize);
            update_descriptors();
        }
    }

    void final_render_pass(vk::CommandBuffer cmd, vfx::Drawable* drawable) {
        auto clear_values = std::array{
            vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f})),
        };

        auto render_area = vk::Rect2D{};
        render_area.setExtent(drawable->texture->size);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(swapchain->renderPass->handle);
        begin_info.setFramebuffer(drawable->framebuffer);
        begin_info.setRenderArea(render_area);
        begin_info.setClearValues(clear_values);

        cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawable->texture->size.width));
        viewport.setHeight(f32(drawable->texture->size.height));

        cmd.setViewport(0, 1, &viewport);
        cmd.setScissor(0, 1, &render_area);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, present_swapchain_material->pipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, present_swapchain_material->pipeline_layout, 0, present_swapchain_material->descriptor_sets, {});

        cmd.draw(6, 1, 0, 0);

        cmd.endRenderPass();
    }

    void create_present_swapchain_material() {
        vfx::MaterialDescription description{};

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
        present_swapchain_material = context->makeMaterial(description, swapchain->renderPass, 0);

        update_descriptors();
    }

    void update_descriptors() {
        const auto image_info = vk::DescriptorImageInfo{
            .sampler = renderer->sampler,
            .imageView = renderer->colorAttachmentTexture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = present_swapchain_material->descriptor_sets[0],
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
