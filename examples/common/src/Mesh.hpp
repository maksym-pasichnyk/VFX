#pragma once

#include "gfx/GFX.hpp"
#include "Primitive.hpp"

#include "glm/glm.hpp"

struct Vertex {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec4 color;
};

struct Mesh : gfx::Referencing {
private:
    std::vector<Vertex> mVertices = {};
    std::vector<uint32_t> mIndices = {};
    std::vector<Primitive> mPrimitives = {};
    std::vector<glm::u32vec4> mJoints = {};
    std::vector<glm::f32vec4> mWeights = {};
    gfx::SharedPtr<gfx::Buffer> mIndexBuffer = {};
    gfx::SharedPtr<gfx::Buffer> mVertexBuffer = {};

public:
    explicit Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Primitive> primitives, std::vector<glm::u32vec4> joints, std::vector<glm::f32vec4> weights)
        : mVertices(std::move(vertices))
        , mIndices(std::move(indices))
        , mPrimitives(std::move(primitives))
        , mJoints(std::move(joints))
        , mWeights(std::move(weights)) {}

public:
    void uploadMeshData(const gfx::SharedPtr<gfx::Device>& device) {
        if (mIndices.empty()) {
            mIndexBuffer = {};
        } else {
            mIndexBuffer = device->newBuffer(vk::BufferUsageFlagBits::eIndexBuffer, mIndices.data(), mIndices.size() * sizeof(uint32_t), VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
        }
        if (mVertices.empty()) {
            mVertexBuffer = {};
        } else {
            mVertexBuffer = device->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, mVertices.data(), mVertices.size() * sizeof(Vertex), VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
        }
    }

    void draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd) {
        if (mIndexBuffer) {
            cmd->bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint32);
        }
        if (mVertexBuffer) {
            cmd->bindVertexBuffer(0, mVertexBuffer, 0);
        }

        for (auto& primitive : mPrimitives) {
            if (primitive.numIndices > 0) {
                cmd->drawIndexed(primitive.numIndices, 1, primitive.baseIndex, static_cast<int32_t>(primitive.baseVertex), 0);
            } else {
                cmd->draw(primitive.numVertices, 1, primitive.baseVertex, 0);
            }
        }
    }
};