#pragma once

#include "types.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace vfx {
//    enum class BufferUsage {
//        Vertex   = 1 << 0,
//        Index    = 1 << 1,
//        CopySrc  = 1 << 2,
//        CopyDst  = 1 << 3,
//        Constant = 1 << 4
//    };

    struct Device;
    struct Buffer {
    public:
        Device* device{};
        vk::Buffer handle{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocationInfo{};
        vk::DeviceSize allocationSize = 0;

    public:
        Buffer();
        ~Buffer();

    public:
        auto map() const -> void*;
        void unmap() const;

        void update(const void* src, u64 size, u64 offset) const;
        void setLabel(const std::string& name) const;
    };
}