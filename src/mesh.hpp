#pragma once

#include "types.hpp"

#include <span>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Vertex {
        glm::vec3 position{};
        u32 color{};
    };

    struct Buffer;
    struct Context;
    struct Mesh {
    public:
        Arc<Context> context;

        i32 indexCount = 0;
        u64 indexStride = 0;

        i32 vertexCount = 0;
        u64 vertexStride = 0;

        Arc<vfx::Buffer> indexBuffer{};
        Arc<vfx::Buffer> vertexBuffer{};

    public:
        Mesh(const Arc<Context>& context) : context(context) {}

    public:
        void setIndexBufferData(const void* src, u64 size, u64 offset);
        void setVertexBufferData(const void* src, u64 size, u64 offset);

        void setIndexBufferParams(i32 count, u64 stride);
        void setVertexBufferParams(i32 count, u64 stride);

        void setVertices(std::span<const Vertex> vertices);
        void setIndices(std::span<const u32> indices);
    };
}