#include "Mesh.hpp"
#include "Assets.hpp"
#include "Application.hpp"

#include "block/Block.hpp"
#include "block/Material.hpp"
#include "registry/DefaultedRegistry.hpp"

#include "model/ModelManager.hpp"
#include "loaders/BlockLoader.hpp"
#include "loaders/MaterialLoader.hpp"
#include "texture/TextureManager.hpp"

#include "fmt/format.h"
#include "glm/gtx/euler_angles.hpp"
#include "filesystem.hpp"

sp<MappedRegistry<sp<Block>>> BLOCK = sp<MappedRegistry<sp<Block>>>::of();
sp<MappedRegistry<sp<Material>>> MATERIAL = sp<MappedRegistry<sp<Material>>>::of();

Application* global;

struct Game : Application {
public:
    Game() : Application("Minecraft-07") {
        global = this;

        buildShaders();
        buildBuffers();

        auto atlas = textureManager->getTexture("textures/atlas/blocks.png");

        vk::SamplerCreateInfo sampler_description = {};
        sampler_description.setMagFilter(vk::Filter::eNearest);
        sampler_description.setMinFilter(vk::Filter::eNearest);
        sampler_description.setMipmapMode(vk::SamplerMipmapMode::eLinear);
        sampler_description.setAddressModeU(vk::SamplerAddressMode::eRepeat);
        sampler_description.setAddressModeV(vk::SamplerAddressMode::eRepeat);
        sampler_description.setAddressModeW(vk::SamplerAddressMode::eRepeat);
        sampler = device.newSampler(sampler_description);

        descriptorSet.setSampler(sampler, 0);
        descriptorSet.setTexture(atlas->getTexture(), 1);

        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

private:
    void buildShaders() {
        auto vertexLibrary = device.newLibrary(Assets::readFile("shaders/geometry.vert.spv"));
        auto fragmentLibrary = device.newLibrary(Assets::readFile("shaders/geometry.frag.spv"));

        gfx::RenderPipelineStateDescription description;
        description.vertexFunction = vertexLibrary.newFunction("main");
        description.fragmentFunction = fragmentLibrary.newFunction("main");
        description.vertexDescription = {
            .layouts = {{
                vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex}
            }},
            .attributes = {{
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
                vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
                vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
            }}
        };
        description.inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

        description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        description.attachments[0].setBlendEnable(false);

        description.depthAttachmentFormat = vk::Format::eD32Sfloat;
        description.depthStencilState.setDepthTestEnable(true);
        description.depthStencilState.setDepthWriteEnable(true);
        description.depthStencilState.setDepthCompareOp(vk::CompareOp::eLess);

        renderPipelineState = device.newRenderPipelineState(description);
        descriptorSet = device.newDescriptorSet(renderPipelineState.shared->bind_group_layouts[0], {
            vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1},
            vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1}
        });

        gfx::TextureSettings depth_texture_settings;
        depth_texture_settings.width = 800 * 2;
        depth_texture_settings.height = 600 * 2;
        depth_texture_settings.format = vk::Format::eD32Sfloat;
        depth_texture_settings.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

        depthTexture = device.newTexture(depth_texture_settings);
    }

    void buildBuffers() {
        materialLoader.reload();
        blockLoader.reload();

        textureManager = sp<TextureManager>::of();
        modelManager = sp<ModelManager>::of(textureManager);
        modelManager->reload();

        std::vector<uint32_t> indices;
        std::vector<Vertex> vertices;

        using enum Direction;

        auto states = BLOCK->values()
            .flatmap([](auto block) {
                return block->getStateDefinition()->getPossibleStates();
            })
            .collect();

        int gridX = std::ceil(std::sqrt(states.size()));

        for (size_t i = 0; i < states.size(); i++) {
            auto state = states[i];
            auto model = modelManager->getModel(state);

            if (!model) {
                fmt::print("No model for {}\n", BLOCK->getKey(state->getBlock()).value());
                continue;
            }

            auto x = i % gridX;
            auto y = i / gridX;

            using enum Direction;
            for (auto facing : std::array{UP, DOWN, SOUTH, NORTH, EAST, WEST}) {
                model->drawQuads(facing, [&](auto& face) {
                    for (int32_t i : {0, 1, 2, 0, 2, 3}) {
                        indices.emplace_back(vertices.size() + i);
                    }
                    for (auto& vertex : face.vertices) {
                        vertices.emplace_back(Vertex{
                            .position = vertex.position + glm::vec3(x * 2, 0, y * 2),
                            .color = vertex.color,
                            .uv = vertex.texcoord
                        });
                    }
                });
            }
        }

        std::vector<Primitive> primitives;
        primitives.emplace_back(0, 0, indices.size(), vertices.size());

        cube = sp<Mesh>::of(
            std::move(vertices),
            std::move(indices),
            std::move(primitives),
            std::vector<glm::u32vec4>{},
            std::vector<glm::f32vec4>{}
        );

        cube->uploadMeshData(device);
    }

public:
    void update(float_t dt) override {
        int32_t x, y;
        SDL_GetRelativeMouseState(&x, &y);

        const auto d4 = 0.5f * 0.6F + 0.2F;
        const auto d5 = d4 * d4 * d4 * 8.0f;

        rotation.x += float_t(y) * d5 * dt * 9.0f;
        rotation.y += float_t(x) * d5 * dt * 9.0f;
        rotation.x = glm::clamp(rotation.x, -90.0f, 90.0f);

        auto keys = SDL_GetKeyboardState(nullptr);
        auto direction = glm::ivec3();
        if (keys[SDL_SCANCODE_W]) {
            direction += glm::vec3(0, 0, 1);
        }
        if (keys[SDL_SCANCODE_S]) {
            direction += glm::vec3(0, 0, -1);
        }
        if (keys[SDL_SCANCODE_A]) {
            direction += glm::vec3(-1, 0, 0);
        }
        if (keys[SDL_SCANCODE_D]) {
            direction += glm::vec3(1, 0, 0);
        }
        if (keys[SDL_SCANCODE_SPACE]) {
            position += glm::vec3(0, 1, 0) * dt * 20.0F;
        }
        if (keys[SDL_SCANCODE_LSHIFT]) {
            position += glm::vec3(0, -1, 0) * dt * 20.0F;
        }

        auto orientation = glm::yawPitchRoll(
            glm::radians(rotation.y),
            glm::radians(rotation.x),
            glm::radians(rotation.z)
        );

        if (direction != glm::ivec3()) {
            auto velocity = glm::normalize(glm::mat3(orientation) * glm::vec3(direction)) * dt * 20.0F;
            position += velocity;
        }

        g_proj_matrix = getPerspectiveProjection(glm::radians(60.0F), getAspectRatio(), 0.03F, 1000.0F);
        g_view_matrix = glm::inverse(glm::translate(glm::mat4(1.0f), position) * orientation);
    }

    void render() override {
        auto drawable = swapchain.nextDrawable();
        auto drawableSize = swapchain.drawableSize();

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
        rendering_info.colorAttachments[0].texture = drawable.texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        rendering_info.depthAttachment.texture = depthTexture;
        rendering_info.depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        rendering_info.depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        rendering_info.depthAttachment.clearDepth = 1.0F;

        ShaderData shader_data = {};
        shader_data.g_proj_matrix = g_proj_matrix;
        shader_data.g_view_matrix = g_view_matrix;

        commandBuffer.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        commandBuffer.imageBarrier(drawable.texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);
        commandBuffer.imageBarrier(depthTexture, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eEarlyFragmentTests, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eDepthStencilAttachmentWrite);

        commandBuffer.setRenderPipelineState(renderPipelineState);
        commandBuffer.bindDescriptorSet(descriptorSet, 0);
        commandBuffer.pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);

        commandBuffer.beginRendering(rendering_info);
        commandBuffer.setScissor(0, rendering_area);
        commandBuffer.setViewport(0, rendering_viewport);
        cube->draw(commandBuffer);
        commandBuffer.endRendering();

        commandBuffer.imageBarrier(drawable.texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer.imageBarrier(depthTexture, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilReadOnlyOptimal, vk::PipelineStageFlagBits2::eEarlyFragmentTests, vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::AccessFlagBits2::eShaderRead);

        commandBuffer.end();
        commandBuffer.submit();
        commandBuffer.present(drawable);
        commandBuffer.waitUntilCompleted();
    }

private:
    BlockLoader blockLoader{};
    MaterialLoader materialLoader{};

    sp<ModelManager> modelManager;
    sp<TextureManager> textureManager;

    sp<Mesh> cube;
    gfx::Sampler sampler;

    glm::vec3 position = {5, 2, -5};
    glm::vec3 rotation = {0, 0, 0};

    glm::mat4x4 g_proj_matrix = {};
    glm::mat4x4 g_view_matrix = {};

    gfx::DescriptorSet descriptorSet;
    gfx::RenderPipelineState renderPipelineState;

    gfx::Texture depthTexture;
};

auto main(int argc, char** argv) -> int32_t {
    cxx::filesystem::init(argv[0]);
    cxx::filesystem::mount("assets", {}, true);
    cxx::filesystem::mount("assets/minecraft.zip", {}, true);

    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}