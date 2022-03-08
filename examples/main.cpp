#include <map>
#include <ui.hpp>
#include <shader.hpp>
#include <display.hpp>
#include <context.hpp>
#include <glm/ext.hpp>

#include "pipeline.hpp"

struct DefaultVertexFormat {
    glm::vec3 position;
    glm::u8vec4 color;
};

auto main() -> i32 {
    Display display{800, 600, "Demo", true};
    Context context{display};

    RenderPipelineSettings settings{};
    RenderPipeline pipeline{context, settings};

    UI ui{context, pipeline.graph};

    std::vector<u32> indices{
        0, 1, 2, 0, 2, 3
    };
    std::vector<DefaultVertexFormat> vertices{
        DefaultVertexFormat{.position = glm::vec3(-1.0, -0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
        DefaultVertexFormat{.position = glm::vec3(-1.0,  0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
        DefaultVertexFormat{.position = glm::vec3( 0.0,  0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
        DefaultVertexFormat{.position = glm::vec3( 0.0, -0.5, -1), .color = glm::u8vec4(0, 0, 255, 255)},
    };

    static constexpr auto clip = glm::mat4{
        1.0f,  0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 1.0f
    };

    const auto aspect = f32(context.surface_extent.width) / f32(context.surface_extent.height);
    const auto projection = clip * glm::infinitePerspective(glm::radians(60.0f), aspect, 0.1f);

    auto vtx = context.create_buffer(GraphicsBuffer::Target::Vertex, std::span(vertices).size_bytes());
    auto idx = context.create_buffer(GraphicsBuffer::Target::Index, std::span(indices).size_bytes());

    context.update_buffer(vtx, vertices.data(), std::span(vertices).size_bytes(), 0);
    context.update_buffer(idx, indices.data(), std::span(indices).size_bytes(), 0);

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

            auto properties = CameraProperties{};
            properties.projection = projection;
            properties.view = glm::mat4(1.0);// glm::inverse(camera.local_to_world_matrix());

            pipeline.set_camera_properties(properties);
            pipeline.begin_render_pass(cmd);

            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.material->pipeline);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.material->pipeline_layout, 0, pipeline.material->descriptor_sets[context.current_frame], {});
            cmd.bindVertexBuffers(0, vtx->buffer, vk::DeviceSize{0});
            cmd.bindIndexBuffer(idx->buffer, 0, vk::IndexType::eUint32);
            cmd.drawIndexed(indices.size(), 1, 0, 0, 0);

            ImGui::NewFrame();
            ImGui::Begin("Demo");

            ImGui::End();
            ImGui::Render();

            ui.draw(cmd);

            cmd.endRenderPass();
            context.end_frame();
        }
    }

    context.destroy_buffer(vtx);
    context.destroy_buffer(idx);
    context.logical_device.waitIdle();

    return 0;
}
