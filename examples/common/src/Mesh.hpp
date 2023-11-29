#pragma once

#include "Object.hpp"
#include "Graphics.hpp"
#include "Primitive.hpp"

#include "glm/glm.hpp"

struct Vertex {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec2 uv;
};

struct Mesh : public ManagedObject {
private:
    std::vector<Vertex>         vertices_       = {};
    std::vector<uint32_t>       indices_        = {};
    std::vector<Primitive>      primitives_     = {};
    std::vector<glm::u32vec4>   joints_         = {};
    std::vector<glm::f32vec4>   weights_        = {};
    rc<gfx::Buffer>  index_buffer_   = {};
    rc<gfx::Buffer>  vertex_buffer_  = {};

public:
    void setVertices(std::vector<Vertex> vertices) {
        vertices_ = std::move(vertices);
    }

    void setIndices(std::vector<uint32_t> indices) {
        indices_ = std::move(indices);
    }

    void setPrimitives(std::vector<Primitive> primitives) {
        primitives_ = std::move(primitives);
    }

    void setJoints(std::vector<glm::u32vec4> joints) {
        joints_ = std::move(joints);
    }

    void setWeights(std::vector<glm::f32vec4> weights) {
        weights_ = std::move(weights);
    }

    auto getVertices() const -> const std::vector<Vertex>& {
        return vertices_;
    }

    auto getIndices() const -> const std::vector<uint32_t>& {
        return indices_;
    }

    auto getPrimitives() const -> const std::vector<Primitive>& {
        return primitives_;
    }

    auto getJoints() const -> const std::vector<glm::u32vec4>& {
        return joints_;
    }

    auto getWeights() const -> const std::vector<glm::f32vec4>& {
        return weights_;
    }

public:
    void uploadMeshData(const rc<gfx::Device>& device) {
        if (indices_.empty()) {
            index_buffer_ = {};
        } else {
            index_buffer_ = device->newBuffer(vk::BufferUsageFlagBits::eIndexBuffer, indices_.data(), indices_.size() * sizeof(uint32_t), gfx::StorageMode::eShared);
        }
        if (vertices_.empty()) {
            vertex_buffer_ = {};
        } else {
            vertex_buffer_ = device->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, vertices_.data(), vertices_.size() * sizeof(Vertex), gfx::StorageMode::eShared);
        }
    }

    void draw(const rc<gfx::RenderCommandEncoder>& encoder) {
        if (index_buffer_) {
            encoder->bindIndexBuffer(index_buffer_, 0, vk::IndexType::eUint32);
        }
        if (vertex_buffer_) {
            encoder->bindVertexBuffer(0, vertex_buffer_, 0);
        }

        for (auto& primitive : primitives_) {
            if (primitive.numIndices > 0) {
                encoder->drawIndexed(primitive.numIndices, 1, primitive.baseIndex, static_cast<int32_t>(primitive.baseVertex), 0);
            } else {
                encoder->draw(primitive.numVertices, 1, primitive.baseVertex, 0);
            }
        }
    }
};
