#include <map>
#include <ui.hpp>
#include <display.hpp>
#include <context.hpp>
#include <glm/ext.hpp>

#include "pipeline.hpp"

struct Demo {
    Display display{800, 600, "Demo", true};
    Context context{display};

    RenderPipelineSettings settings{};
    RenderPipeline pipeline{context, settings};

    UI ui{context, pipeline.graph};

    std::vector<Geometry> geometries;

    Demo() {
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
    }

    ~Demo() {
        for (auto& geometry : geometries) {
            context.destroy_buffer(geometry.vtx);
            context.destroy_buffer(geometry.idx);
        }
    }

    void run() {
        Camera camera{60.0f, display.get_aspect()};

        auto last_time = std::chrono::high_resolution_clock::now();
        while (!display.should_close()) {
            const auto current_time = std::chrono::high_resolution_clock::now();
            const auto delta_time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - last_time).count();
            last_time = current_time;

            display.poll_events();

            ui.set_delta_time(delta_time);
            ui.update(display);

            if (context.begin_frame()) {
                auto cmd = context.command_buffers[context.current_frame];

                pipeline.set_camera_properties(camera);
                pipeline.begin_render_pass(cmd);

                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.material->pipeline);
                cmd.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipeline.material->pipeline_layout,
                    0,
                    pipeline.material->descriptor_sets[context.current_frame],
                    {}
                );

                for (auto& geometry : geometries) {
                    cmd.bindVertexBuffers(0, geometry.vtx->buffer, vk::DeviceSize{0});
                    cmd.bindIndexBuffer(geometry.idx->buffer, 0, vk::IndexType::eUint32);
                    cmd.drawIndexed(geometry.idx_count, 1, 0, 0, 0);
                }

                ImGui::NewFrame();
                ImGui::Begin("Demo");

                ImGui::End();
                ImGui::Render();

                ui.draw(cmd);

                cmd.endRenderPass();
                context.end_frame();
            }
        }
        context.logical_device.waitIdle();
    }
};

auto main() -> i32 {
    Demo demo{};
    demo.run();
    return 0;
}
