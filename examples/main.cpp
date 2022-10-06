#include <map>
#include <display.hpp>
#include <context.hpp>
#include <swapchain.hpp>

#include "widgets.hpp"
#include "pipeline.hpp"

#include <entt/entt.hpp>

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
    Display display{800, 600, "Demo", true};
    vfx::Context context{display};
    vfx::Swapchain swapchain{context, display};

    DefaultRenderPipeline pipeline{context, swapchain.surface_format.format};

    vfx::Material* blit_material;

    Widgets widgets{context, pipeline.pass.handle};

    Demo() {
        vfx::MaterialDescription description{};

        description.create_attachment(vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA
        });
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
        blit_material = context.create_material(description, swapchain.render_pass.handle, 0);

        const auto image_info = vk::DescriptorImageInfo{
            .sampler = pipeline.color_sampler,
            .imageView = pipeline.color->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = blit_material->descriptor_sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &image_info
        };
        context.logical_device.updateDescriptorSets(write_descriptor_set, {});
    }

    ~Demo() {
        context.destroy_material(blit_material);
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

            context.set_vertices(&geometries[0], vertices);
            context.set_indices(&geometries[0], indices);
        }
        {
            std::vector<DefaultVertexFormat> vertices{
                DefaultVertexFormat{.position = glm::vec3(0.0, -0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
                DefaultVertexFormat{.position = glm::vec3(0.0,  0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
                DefaultVertexFormat{.position = glm::vec3(1.0,  0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
                DefaultVertexFormat{.position = glm::vec3(1.0, -0.5, -1), .color = glm::u8vec4(255, 0, 0, 255)},
            };

            context.set_vertices(&geometries[1], vertices);
            context.set_indices(&geometries[1], indices);
        }

        Scene scene{};
        Camera camera{60.0f, display.get_aspect()};

        auto g0 = Entity{scene.create_entity(), &scene};
        auto g1 = Entity{scene.create_entity(), &scene};

        g0.emplace<MeshRenderer>(true, &geometries[0], nullptr);
        g1.emplace<MeshRenderer>(true, &geometries[1], nullptr);

        auto last_time = std::chrono::high_resolution_clock::now();
        while (!display.should_close()) {
            const auto current_time = std::chrono::high_resolution_clock::now();
            const auto delta_time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - last_time).count();
            last_time = current_time;

            display.poll_events();

            widgets.set_delta_time(delta_time);
            widgets.update(display);

            if (swapchain.acquire_next_image()) {
                auto cmd = context.command_buffers[swapchain.current_frame];
                cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

                pipeline.begin_rendering(cmd);
                pipeline.setup(cmd, camera);

                for (auto&& [_, renderer] : scene.registry.view<MeshRenderer>().each()) {
                    if (!renderer.enable || !renderer.geometry) {
                        continue;
                    }
                    cmd.bindVertexBuffers(0, renderer.geometry->vtx->buffer, vk::DeviceSize{0});
                    cmd.bindIndexBuffer(renderer.geometry->idx->buffer, 0, vk::IndexType::eUint32);
                    cmd.drawIndexed(renderer.geometry->idx_count, 1, 0, 0, 0);
                }

                widgets.begin_frame();
                ImGui::Begin("Stats");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
                widgets.end_frame();

                widgets.draw(cmd);
                pipeline.end_rendering(cmd);

                // todo: move to better place
                {
                    auto clear_values = std::array{
                        vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({1.0f, 1.0f, 1.0f, 1.0f}))
                    };

                    auto area = vk::Rect2D{};
                    area.setExtent(swapchain.surface_extent);

                    auto begin_info = vk::RenderPassBeginInfo{};
                    begin_info.setRenderPass(swapchain.render_pass.handle);
                    begin_info.setFramebuffer(swapchain.framebuffers[swapchain.image_index]);
                    begin_info.setRenderArea(area);
                    begin_info.setClearValues(clear_values);

                    auto viewport = vk::Viewport{};
                    viewport.setWidth(f32(area.extent.width));
                    viewport.setHeight(f32(area.extent.height));
                    viewport.setMaxDepth(1.f);

                    cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
                    cmd.setViewport(0, viewport);
                    cmd.setScissor(0, area);

                    cmd.bindPipeline(blit_material->pipeline_bind_point, blit_material->pipeline);
                    cmd.bindDescriptorSets(blit_material->pipeline_bind_point, blit_material->pipeline_layout, 0, blit_material->descriptor_sets, {});

                    cmd.draw(6, 1, 0, 0);

                    cmd.endRenderPass();
                    cmd.end();
                }

                swapchain.submit(cmd);
                swapchain.present();
            }
        }
        context.logical_device.waitIdle();

        for (auto& geometry : geometries) {
            context.destroy_buffer(geometry.vtx);
            context.destroy_buffer(geometry.idx);
        }
    }
};

auto main() -> i32 {
    Demo demo{};
    demo.run();
    return 0;
}
