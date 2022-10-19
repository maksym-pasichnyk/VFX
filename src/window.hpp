#pragma once

#include "types.hpp"

#include <string>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Swapchain;
    struct Surface {
    public:
        Surface();
        ~Surface();

    public:
        Context* context;
        vk::SurfaceKHR handle;
    };
}