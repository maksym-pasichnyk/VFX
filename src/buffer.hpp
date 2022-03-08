#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

struct GraphicsBuffer {
    enum class Target {
        Vertex   = 1 << 0,
        Index    = 1 << 1,
        CopySrc  = 1 << 2,
        CopyDst  = 1 << 3,
        Constant = 1 << 4
    };
    vk::Buffer buffer{};
    VmaAllocation allocation{};
    VmaAllocationInfo allocation_info{};
};
