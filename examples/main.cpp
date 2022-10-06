#include <map>
#include <entt/entt.hpp>

#include "widgets.hpp"
#include "display.hpp"
#include "renderer.hpp"

struct MeshRenderer {
    bool enable = true;
    Geometry* geometry{};
    vfx::Material* material{};
};

struct Scene {
    entt::registry registry{};

    auto create_entity() -> entt::entity {
        return registry.create();
    }
};

struct Entity {
    entt::entity id;
    Scene* scene;

    template<typename T>
    void get() {
        scene->registry.get<T>(id);
    }

    template<typename T, typename... Args>
    auto emplace(Args&&... args) -> T& {
        return scene->registry.emplace<T>(id, std::forward<Args>(args)...);
    }
};

struct Demo {
    Box<Display> display{};
    Box<vfx::Context> context{};
    Box<vfx::Swapchain> swapchain{};

    Box<Renderer> renderer{};
    Box<Widgets> widgets{};

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
        widgets = Box<Widgets>::alloc(*context, renderer->render_pass);

        create_present_swapchain_material();
    }

    ~Demo() {
        context->freeMaterial(present_swapchain_material);
    }

    void run() {
        static std::vector<Geometry> geometries;

        geometries.resize(2);

        std::vector<u32> indices{
            0, 1, 2, 0, 2, 3
        };
        {
            std::vector<DefaultVertexFormat> vertices{
                DefaultVertexFormat{.position = glm::vec3(-1.0, -0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
                DefaultVertexFormat{.position = glm::vec3(-1.0,  0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
                DefaultVertexFormat{.position = glm::vec3( 0.0,  0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
                DefaultVertexFormat{.position = glm::vec3( 0.0, -0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
            };

            context->set_vertices(&geometries[0], vertices);
            context->set_indices(&geometries[0], indices);
        }
        {
            std::vector<DefaultVertexFormat> vertices{
                DefaultVertexFormat{.position = glm::vec3(0.0, -0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
                DefaultVertexFormat{.position = glm::vec3(0.0,  0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
                DefaultVertexFormat{.position = glm::vec3(1.0,  0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
                DefaultVertexFormat{.position = glm::vec3(1.0, -0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
            };

            context->set_vertices(&geometries[1], vertices);
            context->set_indices(&geometries[1], indices);
        }

        Scene scene{};
        Camera camera{60.0f, display->get_aspect()};

        auto g0 = Entity{scene.create_entity(), &scene};
        auto g1 = Entity{scene.create_entity(), &scene};

        g0.emplace<MeshRenderer>(true, &geometries[0], nullptr);
        g1.emplace<MeshRenderer>(true, &geometries[1], nullptr);

        auto last_time = std::chrono::high_resolution_clock::now();
        while (!display->should_close()) {
            const auto current_time = std::chrono::high_resolution_clock::now();
            const auto delta_time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - last_time).count();
            last_time = current_time;

            display->poll_events();

            widgets->set_delta_time(delta_time);
            widgets->update(*display);

            u32 image_index;
            if (swapchain->acquire_next_image(image_index)) {
                auto cmd = context->command_buffers[swapchain->current_frame];
                cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

                renderer->begin_rendering(cmd);
                renderer->setup(cmd, camera);

                for (auto&& [_, mesh_renderer] : scene.registry.view<MeshRenderer>().each()) {
                    if (!mesh_renderer.enable || !mesh_renderer.geometry) {
                        continue;
                    }
                    cmd.bindVertexBuffers(0, mesh_renderer.geometry->vtx->buffer, vk::DeviceSize{0});
                    cmd.bindIndexBuffer(mesh_renderer.geometry->idx->buffer, 0, vk::IndexType::eUint32);
                    cmd.drawIndexed(mesh_renderer.geometry->idx_count, 1, 0, 0, 0);
                }

                widgets->begin_frame();
                ImGui::Begin("Stats");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
                widgets->end_frame();

                widgets->draw(cmd);
                renderer->end_rendering(cmd);

                // todo: move to better place
                {
                    auto clear_values = std::array{
                        vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({1.0f, 1.0f, 1.0f, 1.0f}))
                    };

                    auto area = vk::Rect2D{};
                    area.setExtent(swapchain->image_extent);

                    auto begin_info = vk::RenderPassBeginInfo{};
                    begin_info.setRenderPass(swapchain->render_pass->handle);
                    begin_info.setFramebuffer(swapchain->framebuffers[image_index]);
                    begin_info.setRenderArea(area);
                    begin_info.setClearValues(clear_values);

                    auto viewport = vk::Viewport{};
                    viewport.setWidth(f32(swapchain->image_extent.width));
                    viewport.setHeight(f32(swapchain->image_extent.height));
                    viewport.setMaxDepth(1.f);

                    cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
                    cmd.setViewport(0, viewport);
                    cmd.setScissor(0, area);

                    cmd.bindPipeline(present_swapchain_material->pipeline_bind_point, present_swapchain_material->pipeline);
                    cmd.bindDescriptorSets(present_swapchain_material->pipeline_bind_point, present_swapchain_material->pipeline_layout, 0, present_swapchain_material->descriptor_sets, {});

                    cmd.draw(6, 1, 0, 0);

                    cmd.endRenderPass();
                    cmd.end();
                }

                swapchain->submit(image_index, cmd);
                swapchain->present(image_index);
            }
        }
        context->logical_device.waitIdle();

        for (auto& geometry : geometries) {
            context->freeBuffer(geometry.vtx);
            context->freeBuffer(geometry.idx);
        }
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
