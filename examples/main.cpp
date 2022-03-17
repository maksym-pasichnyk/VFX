#include <map>
#include <display.hpp>
#include <context.hpp>
#include <glm/ext.hpp>

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

    DefaultRenderPipeline pipeline{context, swapchain};

    Widgets widgets{context, pipeline.pass.handle};

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

        Scene* scene = new Scene();

        Camera camera{context, 60.0f, display.get_aspect()};

        auto g0 = Entity{scene->create_entity(), scene};
        auto g1 = Entity{scene->create_entity(), scene};

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

                pipeline.begin_frame(cmd);
                pipeline.setup(cmd, &camera);

                for (auto&& [_, renderer] : scene->registry.view<MeshRenderer>().each()) {
                    if (!renderer.enable || !renderer.geometry) {
                        continue;
                    }
                    cmd.bindVertexBuffers(0, renderer.geometry->vtx->buffer, vk::DeviceSize{0});
                    cmd.bindIndexBuffer(renderer.geometry->idx->buffer, 0, vk::IndexType::eUint32);
                    cmd.drawIndexed(renderer.geometry->idx_count, 1, 0, 0, 0);
                }

                widgets.begin_frame();
                ImGui::Begin("Demo");

                ImGui::End();
                widgets.end_frame();

                widgets.draw(cmd);

                cmd.endRenderPass();
                cmd.end();

                swapchain.submit();
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
