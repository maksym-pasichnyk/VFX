#include "Mesh.hpp"
#include "Assets.hpp"
#include "Application.hpp"
#include "glm/gtx/euler_angles.hpp"

#include <vk_mem_alloc.h>

struct GlobalSharedData {
    alignas(16) glm::mat4x4                 projection_matrix;
    alignas(16) glm::mat4x4                 world_to_camera_matrix;
    alignas(16) glm::mat4x4                 view_projection_matrix;
    alignas(16) glm::mat4x4                 inverse_view_projection_matrix;
    alignas(16) std::array<uint32_t, 16>    Volume;
    alignas(16) glm::vec3                   camera_position;
};

struct Camera {
    glm::mat4x4 proj_matrix = {};
    glm::mat4x4 view_matrix = {};
};

struct Transform {
    glm::vec3 position = {};
    glm::vec3 rotation = {};
    glm::vec3 scale = {};
};

struct Game : Application {
public:
    Game() : Application("Raytrace-07") {
        buildShaders();
        buildBuffers();

        SDL_SetRelativeMouseMode(SDL_TRUE);

        transform.position = glm::vec3(0, 0, -5);
        camera_position = transform.position;
    }

private:
    void buildShaders() {
        auto vert_library = device->newLibrary(Assets::readFile("shaders/simple_shader.vert.spv"));
        auto frag_library = device->newLibrary(Assets::readFile("shaders/simple_shader.frag.spv"));

        auto vertex_input_state = rc<gfx::VertexInputState>::init();
        vertex_input_state->bindings = {
            vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex)
        };
        vertex_input_state->attributes = {
            vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)),
            vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
        };

        auto description = gfx::RenderPipelineStateDescription::init();
        description->setVertexFunction(vert_library->newFunction("main"));
        description->setFragmentFunction(frag_library->newFunction("main"));
        description->setVertexInputState(vertex_input_state);
        description->colorAttachmentFormats()[0] = vk::Format::eB8G8R8A8Unorm;
        description->colorBlendAttachments()[0].setBlendEnable(false);

        render_pipeline_state = device->newRenderPipelineState(description);
    }

    void buildBuffers() {
        std::vector<Vertex> vertices{
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.F}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.F}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f, 1.F}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.F}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f, 1.F}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.F}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.F}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.F}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f, 1.F}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.F}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f, 1.F}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.F}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.F}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.F}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.F}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.F}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.F}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.F}},
            
            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.F}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.F}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f, 1.F}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.F}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.F}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.F}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.F}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.F}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.F}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.F}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.F}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.F}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.F}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.F}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.F}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.F}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.F}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.F}},
        };

        std::vector<Primitive> primitives;
        primitives.emplace_back(0, 0, 0, static_cast<uint32_t>(vertices.size()));

        cube = rc<Mesh>::init();
        cube->setVertices(vertices);
        cube->setPrimitives(primitives);
        cube->uploadMeshData(device);
    }

public:
    void update(float_t dt) override {
        camera.proj_matrix = perspective(glm::radians(60.0F), platform->getAspectRatio(), 0.03F, 1000.0F);

        glm::ivec2 input{
            (right ? 1 : 0) - (left ? 1 : 0),
            (forward ? 1 : 0) - (backward ? 1 : 0)
        };
        camera_rotation = glm::lerp(camera_rotation, transform.rotation, dt * 10.0F);

        auto yaw_pitch_roll = glm::quat(camera_rotation);
        if (input != glm::ivec2{}) {
            auto direction = yaw_pitch_roll * glm::vec3(input.x, 0, input.y);
            transform.position += glm::normalize(direction) * dt * 5.0F;
        }
        camera_position = glm::lerp(camera_position, transform.position, dt * 10.0F);

        camera.view_matrix = glm::inverse(
            glm::translate(glm::mat4x4(1.0F), camera_position) * glm::mat4(yaw_pitch_roll)
        );
    }

    void render() override {
        auto drawable = swapchain->nextDrawable();
        auto drawableSize = swapchain->drawableSize();

        vk::Rect2D rendering_area = {};
        rendering_area.setOffset(vk::Offset2D{0, 0});
        rendering_area.setExtent(drawableSize);

        vk::Viewport rendering_viewport = {};
        rendering_viewport.setWidth(static_cast<float_t>(drawableSize.width));
        rendering_viewport.setHeight(static_cast<float_t>(drawableSize.height));
        rendering_viewport.setMinDepth(0.0f);
        rendering_viewport.setMaxDepth(1.0f);

        gfx::RenderingInfo rendering_info = {};
        rendering_info.renderArea = rendering_area;
        rendering_info.layerCount = 1;
        rendering_info.colorAttachments[0].texture = drawable->texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        auto vp = camera.proj_matrix * camera.view_matrix;

        GlobalSharedData shader_data = {};
        shader_data.projection_matrix               = camera.proj_matrix;
        shader_data.world_to_camera_matrix          = camera.view_matrix;
        shader_data.view_projection_matrix          = vp;
        shader_data.inverse_view_projection_matrix  = glm::inverse(vp);
        shader_data.camera_position                 = glm::vec4(camera_position, 1.0F);

        fillVolume(shader_data.Volume);

        commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandBuffer->setImageLayout(drawable->texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        auto encoder = commandBuffer->newRenderCommandEncoder(rendering_info);
        encoder->setRenderPipelineState(render_pipeline_state);
        encoder->pushConstants(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(GlobalSharedData), &shader_data);

        encoder->setScissor(0, rendering_area);
        encoder->setViewport(0, rendering_viewport);
        cube->draw(encoder);
        encoder->endEncoding();

        commandBuffer->setImageLayout(drawable->texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer->end();
        commandBuffer->submit();
        commandBuffer->present(drawable);
        commandBuffer->waitUntilCompleted();
    }

    void keyDown(SDL_KeyboardEvent *event) override {
        if (event->keysym.sym == SDLK_c) {
            if (SDL_GetRelativeMouseMode()) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            } else {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
        }
        if (event->keysym.sym == SDLK_w) {
            forward = true;
        }
        if (event->keysym.sym == SDLK_s) {
            backward = true;
        }
        if (event->keysym.sym == SDLK_d) {
            right = true;
        }
        if (event->keysym.sym == SDLK_a) {
            left = true;
        }
    }

    void keyUp(SDL_KeyboardEvent *event) override {
        if (event->keysym.sym == SDLK_w) {
            forward = false;
        }
        if (event->keysym.sym == SDLK_s) {
            backward = false;
        }
        if (event->keysym.sym == SDLK_d) {
            right = false;
        }
        if (event->keysym.sym == SDLK_a) {
            left = false;
        }
    }

    void mouseMove(SDL_MouseMotionEvent *event) override {
        if (SDL_GetRelativeMouseMode()) {
            transform.rotation.y += glm::radians(static_cast<float>(event->xrel)) * 0.25F;
            transform.rotation.x += glm::radians(static_cast<float>(event->yrel)) * 0.25F;
        }
    }

    static void fillVolume(std::array<uint32_t, 16>& volume) {
        volume.fill(0u);

        int32_t radius = 4;// + static_cast<int32_t>(std::sin(time_since_start * 5.0F) * 4.0F);

        int32_t COUNT_VOXELS = 8;
        for (int32_t x = 0; x < COUNT_VOXELS; ++x) {
            for (int32_t y = 0; y < COUNT_VOXELS; ++y) {
                for (int32_t z = 0; z < COUNT_VOXELS; ++z) {
                    uint32_t index = x + y * COUNT_VOXELS + z * COUNT_VOXELS * COUNT_VOXELS;

                    int32_t center = COUNT_VOXELS / 2;
                    int32_t dx = x - center;
                    int32_t dy = y - center;
                    int32_t dz = z - center;
                    int32_t distance = dx * dx + dy * dy + dz * dz;
                    if (distance <= radius * radius) {
                        volume[index / 32] |= 1u << (index % 32);
                    }
//                    volume[index / 32] |= 1u << (index % 32);
                }
            }
        }
    }

    static auto perspective(float fov, float aspect, float zNear, float zFar) -> glm::mat4x4 {
        assert(abs(aspect - std::numeric_limits<float>::epsilon()) > static_cast<float>(0));

        auto r = glm::tan(fov * 0.5F);
        auto x = +1.0F / (r * aspect);
        auto y = -1.0F / (r);
        auto z = zFar / (zFar - zNear);
        auto a = 1.0F;
        auto b = -(zFar * zNear) / (zFar - zNear);

        return glm::mat4x4{
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, a,
            0, 0, b, 0
        };
    }
    
private:
    glm::vec3 camera_position = {};
    glm::vec3 camera_rotation = {};

    Camera camera;
    Transform transform;

    rc<Mesh> cube;

    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;

    rc<gfx::RenderPipelineState> render_pipeline_state;
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}