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

    struct Context;
    struct Buffer {
    public:
        Context* context{};
        vk::Buffer handle{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocationInfo{};
        vk::DeviceSize allocationSize = 0;

    public:
        Buffer();
        ~Buffer();

    public:
        void update(const void* src, u64 size, u64 offset);
        void setLabel(const std::string& name);
    };
}