#include <map>

#include "widgets.hpp"
#include "display.hpp"
#include "renderer.hpp"

// todo: get rid of current frame
// todo: recycle fences, command buffers, semaphores
struct Demo {
    Box<Display> display{};
    Box<vfx::Context> context{};
    Box<vfx::Swapchain> swapchain{};

    Box<Renderer> renderer{};
    Box<Widgets> widgets{};

    u64 current_frame = 0;

    std::vector<vk::Fence> fences{};
    std::vector<vk::Semaphore> semaphores{};

    vk::CommandPool graphics_command_pool{};
    std::vector<vk::CommandBuffer> graphics_command_buffers{};

    Box<vfx::Material> present_swapchain_material{};

    Demo() {
        display = Box<Display>::alloc(800, 600, "Demo", true);
        context = Box<vfx::Context>::alloc(vfx::ContextDescription{
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

        renderer = Box<Renderer>::alloc(*context, *swapchain);
        widgets = Box<Widgets>::alloc(*context, *display, *renderer);

        create_gpu_objects();
        create_present_swapchain_material();
    }

    ~Demo() {
        context->freeMaterial(present_swapchain_material);
        destroy_gpu_objects();
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
        Camera camera{60.0f, display->get_aspect()};

        context->logical_device.waitForFences(fences[current_frame], true, std::numeric_limits<uint64_t>::max());
        context->logical_device.resetFences(fences[current_frame]);

        auto cmd = graphics_command_buffers[current_frame];
        cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        renderer->begin_rendering(cmd);
        renderer->draw(cmd, camera);

        widgets->begin_frame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::Begin("Stats");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        widgets->end_frame();

        widgets->draw(cmd);
        renderer->end_rendering(cmd);

        // todo: move to better place
        {
            auto drawable = swapchain->getNextDrawable();

            auto clear_values = std::array{
                vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({1.0f, 1.0f, 1.0f, 1.0f}))
            };

            auto area = vk::Rect2D{};
            area.setExtent(swapchain->size);

            auto begin_info = vk::RenderPassBeginInfo{};
            begin_info.setRenderPass(swapchain->render_pass->handle);
            begin_info.setFramebuffer(drawable->framebuffer);
            begin_info.setRenderArea(area);
            begin_info.setClearValues(clear_values);

            auto viewport = vk::Viewport{};
            viewport.setWidth(f32(swapchain->size.width));
            viewport.setHeight(f32(swapchain->size.height));
            viewport.setMaxDepth(1.f);

            cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
            cmd.setViewport(0, viewport);
            cmd.setScissor(0, area);

            cmd.bindPipeline(present_swapchain_material->pipeline_bind_point, present_swapchain_material->pipeline);
            cmd.bindDescriptorSets(present_swapchain_material->pipeline_bind_point, present_swapchain_material->pipeline_layout, 0, present_swapchain_material->descriptor_sets, {});

            cmd.draw(6, 1, 0, 0);

            cmd.endRenderPass();
            cmd.end();

            swapchain->present(
                cmd,
                fences[current_frame],
                semaphores[current_frame],
                drawable
            );
        }

        current_frame = (current_frame + 1) % vfx::Context::MAX_FRAMES_IN_FLIGHT;
    }

    void create_gpu_objects() {
        const auto pool_create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = context->graphics_family
        };
        graphics_command_pool = context->logical_device.createCommandPool(pool_create_info);

        const auto command_buffers_allocate_info = vk::CommandBufferAllocateInfo{
            .commandPool = graphics_command_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = vfx::Context::MAX_FRAMES_IN_FLIGHT
        };
        graphics_command_buffers = context->logical_device.allocateCommandBuffers(command_buffers_allocate_info);

        fences.resize(vfx::Context::MAX_FRAMES_IN_FLIGHT);
        semaphores.resize(vfx::Context::MAX_FRAMES_IN_FLIGHT);
        for (u32 i = 0; i < vfx::Context::MAX_FRAMES_IN_FLIGHT; i++) {
            fences[i] = context->logical_device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
            semaphores[i] = context->logical_device.createSemaphore({});
        }
    }

    void destroy_gpu_objects() {
        for (u64 i = 0; i < vfx::Context::MAX_FRAMES_IN_FLIGHT; i++) {
            context->logical_device.destroyFence(fences[i]);
            context->logical_device.destroySemaphore(semaphores[i]);
        }
        context->logical_device.freeCommandBuffers(graphics_command_pool, graphics_command_buffers);
        context->logical_device.destroyCommandPool(graphics_command_pool);
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
        present_swapchain_material = context->makeMaterial(description, swapchain->render_pass, 0);

        const auto image_info = vk::DescriptorImageInfo{
            .sampler = renderer->sampler,
            .imageView = renderer->color_texture->view,
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
    Demo demo{};
    demo.run();
    return 0;
}
