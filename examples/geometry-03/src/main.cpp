#include "Mesh.hpp"
#include "Iter.hpp"
#include "Skin.hpp"
#include "Node.hpp"
#include "Scene.hpp"
#include "Assets.hpp"
#include "Animation.hpp"
#include "Application.hpp"

#include "tiny_gltf.h"
#include "fmt/format.h"

struct GltfBundle {
    std::vector<sp<Skin>>       skins = {};
    std::vector<sp<Mesh>>       meshes = {};
    std::vector<sp<Scene>>      scenes = {};
    std::vector<sp<Animation>>  animations = {};
};

struct Game : Application {
public:
    Game() : Application("Geometry-03") {
        buildShaders();
        buildBuffers();
    }

private:
    void buildShaders() {
        auto vertexLibrary = device->newLibrary(Assets::readFile("shaders/geometry.vert.spv"));
        auto fragmentLibrary = device->newLibrary(Assets::readFile("shaders/geometry.frag.spv"));

        gfx::RenderPipelineStateDescription description;
        description.vertexFunction = vertexLibrary->newFunction("main");
        description.fragmentFunction = fragmentLibrary->newFunction("main");
        description.vertexInputState = {
            .bindings = {
                vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex}
            },
            .attributes = {
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
                vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
                vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
            }
        };

        description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        description.colorBlendAttachments[0].setBlendEnable(false);

        renderPipelineState = device->newRenderPipelineState(description);
        sampler = device->newSampler(vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
        });

        texture = device->newTexture(gfx::TextureSettings{
            .width = 1,
            .height = 1,
            .format = vk::Format::eR8G8B8A8Unorm,
            .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        });

        std::array<uint8_t, 4> orange = {255, 127, 0, 255};
        texture->replaceRegion(orange.data(), sizeof(orange));
    }

    void buildBuffers() {
        auto bytes = Assets::readFile("models/Fox.glb");

        std::string err = {};
        std::string warn = {};
        tinygltf::Model model = {};
        tinygltf::TinyGLTF loader = {};

        bool loaded = loader.LoadBinaryFromMemory(&model, &err, &warn, reinterpret_cast<const unsigned char*>(bytes.data()), bytes.size());
        if (!warn.empty()) {
            fmt::print(stderr, "{}\n", warn);
        }
        if (!err.empty()) {
            fmt::print(stderr, "{}\n", err);
        }
        if (!loaded) {
            throw std::runtime_error("Failed to load gltf file!");
        }

        for (auto& mesh : model.meshes) {
            std::vector<Vertex> vertices = {};
            std::vector<uint32_t> indices = {};
            std::vector<Primitive> primitives = {};
            std::vector<glm::u32vec4> joints = {};
            std::vector<glm::f32vec4> weights = {};

            for (auto& primitive : mesh.primitives) {
                std::vector<glm::vec3> positions = {};
                std::vector<glm::vec2> texcoords = {};

                uint32_t baseIndex = 0;
                uint32_t baseVertex = 0;
                uint32_t numIndices = 0;
                uint32_t numVertices = 0;

                for (const auto& [name, attrib] : primitive.attributes) {
                    auto& accessor = model.accessors[attrib];
                    auto& bufferView = model.bufferViews[accessor.bufferView];
                    auto& buffer = model.buffers[bufferView.buffer];
                    auto stride = accessor.ByteStride(bufferView);
                    auto* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                    if (name == "POSITION") {
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        positions.reserve(accessor.count);
                        for (size_t i = 0; i < accessor.count; ++i) {
                            glm::vec3 position;
                            std::memcpy(&position, data + i * stride, sizeof(glm::vec3));
                            positions.emplace_back(position);
                        }
                    } else if (name == "TEXCOORD_0") {
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        texcoords.reserve(accessor.count);
                        for (size_t i = 0; i < accessor.count; ++i) {
                            glm::vec2 texcoord;
                            std::memcpy(&texcoord, data + i * stride, sizeof(glm::vec2));
                            texcoords.emplace_back(texcoord);
                        }
                    } else if (name == "JOINTS_0") {
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
                        joints.reserve(accessor.count);
                        for (size_t i = 0; i < accessor.count; ++i) {
                            glm::u16vec4 joint;
                            std::memcpy(&joint, data + i * stride, sizeof(glm::u16vec4));
                            joints.emplace_back(joint);
                        }
                    } else if (name == "WEIGHTS_0") {
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        assert(stride == 16);
                        weights.reserve(accessor.count);
                        for (size_t i = 0; i < accessor.count; ++i) {
                            glm::f32vec4 weight;
                            std::memcpy(&weight, data + i * stride, sizeof(glm::f32vec4));
                            weights.emplace_back(weight);
                        }
                    }
                }

                if (primitive.indices >= 0) {
                    auto& accessor = model.accessors[primitive.indices];
                    auto& bufferView = model.bufferViews[accessor.bufferView];
                    auto& buffer = model.buffers[bufferView.buffer];
                    auto stride = accessor.ByteStride(bufferView);
                    auto* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;
                    indices.reserve(accessor.count);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        uint32_t index;
                        std::memcpy(&index, data + i * stride, sizeof(uint32_t));
                        indices.emplace_back(index);
                    }
                    numIndices += accessor.count;
                }

                for (const auto& [position, texcoord] : ranges::views::zip(positions, texcoords)) {
                    Vertex vertex = {};
                    vertex.position = position * 0.01F;
                    vertex.color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);

                    vertices.emplace_back(vertex);
                    numVertices += 1;
                }

                primitives.emplace_back(baseIndex, baseVertex, numIndices, numVertices);
            }

            auto gfxMesh = sp<Mesh>::of(std::move(vertices), std::move(indices), std::move(primitives), std::move(joints), std::move(weights));
            gfxMesh->uploadMeshData(device);
            gltf_bundle.meshes.emplace_back(std::move(gfxMesh));
        }

        for (auto& skin : model.skins) {
            std::vector<glm::mat4x4> inverseBindMatrices = {};

            if (skin.inverseBindMatrices >= 0) {
                auto& accessor = model.accessors[skin.inverseBindMatrices];
                auto& bufferView = model.bufferViews[accessor.bufferView];
                auto& buffer = model.buffers[bufferView.buffer];
                auto stride = accessor.ByteStride(bufferView);
                auto* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                for (size_t i = 0; i < accessor.count; ++i) {
                    glm::mat4x4 inverseBindMatrix;
                    std::memcpy(&inverseBindMatrix, data + i * stride, sizeof(glm::mat4x4));
                    inverseBindMatrices.emplace_back(inverseBindMatrix);
                }
            }

            gltf_bundle.skins.emplace_back(sp<Skin>::of(skin.skeleton, skin.joints, std::move(inverseBindMatrices)));
        }

        std::vector<sp<Node>> nodes = {};

        for (auto& node : model.nodes) {
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

            if (!node.translation.empty()) {
                assert(node.translation.size() == 3);
                auto x = static_cast<float>(node.translation[0]);
                auto y = static_cast<float>(node.translation[1]);
                auto z = static_cast<float>(node.translation[2]);
                position = glm::vec3(x, y, z);
            }
            if (!node.rotation.empty()) {
                assert(node.rotation.size() == 4);
                auto x = static_cast<float>(node.rotation[0]);
                auto y = static_cast<float>(node.rotation[1]);
                auto z = static_cast<float>(node.rotation[2]);
                auto w = static_cast<float>(node.rotation[3]);
                rotation = glm::quat(w, x, y, z);
            }
            if (!node.scale.empty()) {
                assert(node.scale.size() == 3);
                auto x = static_cast<float>(node.scale[0]);
                auto y = static_cast<float>(node.scale[1]);
                auto z = static_cast<float>(node.scale[2]);
                scale = glm::vec3(x, y, z);
            }
            sp<Skin> skin = {};
            if (node.skin >= 0) {
                skin = gltf_bundle.skins.at(node.skin);
            }
            sp<Mesh> mesh = {};
            if (node.mesh >= 0) {
                mesh = gltf_bundle.meshes.at(node.mesh);
            }
            nodes.emplace_back(sp<Node>::of(std::move(skin), std::move(mesh), position, rotation, scale));
        }

        for (size_t i = 0; i < model.nodes.size(); ++i) {
            for (size_t child : model.nodes.at(i).children) {
                nodes.at(child)->setParent(nodes.at(i));
            }
        }

        for (auto& scene : model.scenes) {
            gltf_bundle.scenes.emplace_back(sp<Scene>::of(cxx::iter(scene.nodes).map([&](int i) { return nodes[i]; }).collect()));
        }

        for (auto& animation : model.animations) {
            float start = std::numeric_limits<float>::max();
            float end = std::numeric_limits<float_t>::min();

            std::vector<AnimationSampler> samplers = {};
            std::vector<AnimationChannel> channels = {};

            for (const auto& sampler : animation.samplers) {
                auto inputs = [&] {
                    std::vector<float_t> inputs = {};
                    auto& accessor = model.accessors[sampler.input];
                    auto& bufferView = model.bufferViews[accessor.bufferView];
                    auto& buffer = model.buffers[bufferView.buffer];
                    auto stride = accessor.ByteStride(bufferView);
                    auto* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                    for (size_t i = 0; i < accessor.count; ++i) {
                        float_t input;
                        std::memcpy(&input, data + i * stride, sizeof(float_t));

                        if (input < start) {
                            start = input;
                        }
                        if (input > end) {
                            end = input;
                        }
                        inputs.emplace_back(input);
                    }
                    return inputs;
                }();

                auto outputs = [&] {
                    std::vector<glm::f32vec4> outputs = {};
                    auto& accessor = model.accessors[sampler.output];
                    auto& bufferView = model.bufferViews[accessor.bufferView];
                    auto& buffer = model.buffers[bufferView.buffer];
                    auto stride = accessor.ByteStride(bufferView);
                    auto* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                    if (accessor.type == TINYGLTF_TYPE_VEC3) {
                        for (size_t i = 0; i < accessor.count; ++i) {
                            glm::vec3 output;
                            std::memcpy(&output, data + i * stride, sizeof(glm::vec3));
                            outputs.emplace_back(output, 0.0F);
                        }
                    } else if (accessor.type == TINYGLTF_TYPE_VEC4) {
                        for (size_t i = 0; i < accessor.count; ++i) {
                            glm::vec4 output;
                            std::memcpy(&output, data + i * stride, sizeof(glm::vec4));
                            outputs.emplace_back(output);
                        }
                    } else {
                        fmt::print(stderr, "Unknown output type {}\n", accessor.type);
                    }

                    return outputs;
                }();

                samplers.emplace_back(sampler.interpolation, std::move(inputs), std::move(outputs));
            }

            channels.reserve(animation.channels.size());
            for (const auto& channel : animation.channels) {
                channels.emplace_back(channel.target_path, channel.sampler, channel.target_node);
            }

            gltf_bundle.animations.emplace_back(sp<Animation>::of(animation.name, std::move(samplers), std::move(channels), start, end));
        }
    }

public:
    void update(float_t dt) override {
        camera_projection_matrix = getPerspectiveProjection(glm::radians(60.0F), getAspectRatio(), 0.03F, 1000.0F);
        world_to_camera_matrix = glm::lookAtLH(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0, 1, 0));
        world_to_camera_matrix = glm::rotate(world_to_camera_matrix, angle, glm::vec3(0, 1, 0));
        angle += dt * 5.0F;
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
        rendering_info.colorAttachments[0].texture = drawable.texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        ShaderData shader_data = {};
        shader_data.g_proj_matrix = camera_projection_matrix;
        shader_data.g_view_matrix = world_to_camera_matrix;

        commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        auto descriptorSet = commandBuffer->newDescriptorSet(renderPipelineState, 0);

        vk::DescriptorImageInfo sampler_info = {};
        sampler_info.setSampler(sampler->raw);

        vk::DescriptorImageInfo texture_info = {};
        texture_info.setImageView(texture->image_view);
        texture_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        vk::WriteDescriptorSet writes[2] = {};
        writes[0].setDstSet(descriptorSet);
        writes[0].setDstBinding(0);
        writes[0].setDstArrayElement(0);
        writes[0].setDescriptorType(vk::DescriptorType::eSampler);
        writes[0].setDescriptorCount(1);
        writes[0].setPImageInfo(&sampler_info);

        writes[1].setDstSet(descriptorSet);
        writes[1].setDstBinding(1);
        writes[1].setDstArrayElement(0);
        writes[1].setDescriptorType(vk::DescriptorType::eSampledImage);
        writes[1].setDescriptorCount(1);
        writes[1].setPImageInfo(&texture_info);

        device->raii.raw.updateDescriptorSets(2, writes, 0, nullptr, device->raii.dispatcher);

        commandBuffer->setImageLayout(drawable.texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        auto encoder = commandBuffer->newRenderCommandEncoder(rendering_info);
        encoder->setRenderPipelineState(renderPipelineState);
        encoder->bindDescriptorSet(descriptorSet, 0);
        encoder->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);
        encoder->setScissor(0, rendering_area);
        encoder->setViewport(0, rendering_viewport);
        gltf_bundle.meshes.front()->draw(encoder);
        encoder->endEncoding();

        commandBuffer->setImageLayout(drawable.texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer->end();
        commandBuffer->submit();
        commandBuffer->present(drawable);
        commandBuffer->waitUntilCompleted();
    }

private:
    float_t angle = 0.0F;
    glm::mat4x4 camera_projection_matrix = {};
    glm::mat4x4 world_to_camera_matrix = {};

    GltfBundle  gltf_bundle;

    ManagedShared<gfx::Sampler>             sampler;
    ManagedShared<gfx::Texture>             texture;
    ManagedShared<gfx::RenderPipelineState> renderPipelineState;
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}