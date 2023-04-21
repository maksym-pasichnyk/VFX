//
// Created by Maksym Pasichnyk on 22.04.2023.
//

#pragma once

#include "Iter.hpp"
#include "Mesh.hpp"
#include "Skin.hpp"
#include "Node.hpp"
#include "Scene.hpp"
#include "Assets.hpp"
#include "Animation.hpp"

#include "tiny_gltf.h"
#include "fmt/format.h"

struct GltfBundle {
    std::vector<sp<Skin>>       skins = {};
    std::vector<sp<Mesh>>       meshes = {};
    std::vector<sp<Scene>>      scenes = {};
    std::vector<sp<Animation>>  animations = {};

    static auto open(const std::string& path) -> GltfBundle {
        auto bytes = Assets::readFile(path);

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

        GltfBundle bundle;
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
            bundle.meshes.emplace_back(std::move(gfxMesh));
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

            bundle.skins.emplace_back(sp<Skin>::of(skin.skeleton, skin.joints, std::move(inverseBindMatrices)));
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
                skin = bundle.skins.at(node.skin);
            }
            sp<Mesh> mesh = {};
            if (node.mesh >= 0) {
                mesh = bundle.meshes.at(node.mesh);
            }
            nodes.emplace_back(sp<Node>::of(std::move(skin), std::move(mesh), position, rotation, scale));
        }

        for (size_t i = 0; i < model.nodes.size(); ++i) {
            for (size_t child : model.nodes.at(i).children) {
                nodes.at(child)->setParent(nodes.at(i));
            }
        }

        for (auto& scene : model.scenes) {
            bundle.scenes.emplace_back(sp<Scene>::of(cxx::iter(scene.nodes).map([&](int i) { return nodes[i]; }).collect()));
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

            bundle.animations.emplace_back(sp<Animation>::of(animation.name, std::move(samplers), std::move(channels), start, end));
        }
        return bundle;
    }
};
