#include "mesh.hpp"
#include "buffer.hpp"
#include "context.hpp"

void vfx::Mesh::setVertexBufferData(const void* src, u64 size, u64 offset) {
    vertexBuffer->update(src, size * vertexStride, offset * vertexStride);
}

void vfx::Mesh::setIndexBufferData(const void* src, u64 size, u64 offset) {
    indexBuffer->update(src, size * indexStride, offset * indexStride);
}

void vfx::Mesh::setIndexBufferParams(i32 count, u64 stride) {
    auto size = static_cast<vk::DeviceSize>(count * stride);

    indexCount = count;
    indexStride = stride;
    if (!indexBuffer || size > indexBuffer->allocationSize) {
        if (indexBuffer != nullptr) {
            context->freeBuffer(indexBuffer);
        }
        indexBuffer = context->makeBuffer(BufferUsage::Index, size);
    }
}

void vfx::Mesh::setVertexBufferParams(i32 count, u64 stride) {
    auto size = static_cast<vk::DeviceSize>(count * stride);

    vertexCount = count;
    vertexStride = stride;
    if (!vertexBuffer || size > vertexBuffer->allocationSize) {
        if (vertexBuffer != nullptr) {
            context->freeBuffer(vertexBuffer);
        }
        vertexBuffer = context->makeBuffer(BufferUsage::Vertex, size);
    }
}

void vfx::Mesh::setIndices(std::span<const u32> indices) {
    setIndexBufferParams(i32(indices.size()), sizeof(u32));
    setIndexBufferData(indices.data(), indices.size(), 0);
}

void vfx::Mesh::setVertices(std::span<const Vertex> vertices) {
    setVertexBufferParams(i32(vertices.size()), sizeof(Vertex));
    setVertexBufferData(vertices.data(), vertices.size(), 0);
}
