#include "buffer.hpp"
#include "context.hpp"

void vfx::Buffer::update(const void* src, u64 size, u64 offset) {
    void* ptr = nullptr;
    vmaMapMemory(context->allocator, allocation, &ptr);
    auto dst = static_cast<std::byte*>(ptr) + offset;
    memcpy(dst, src, size);
    vmaUnmapMemory(context->allocator, allocation);
}