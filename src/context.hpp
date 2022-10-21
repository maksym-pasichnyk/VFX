#pragma once

#include "types.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

namespace vfx {
    struct Context {
    public:
        vk::DynamicLoader dl{};
        vk::UniqueInstance instance{};
        vk::UniqueDebugUtilsMessengerEXT debug_utils{};

    public:
        Context();
    };
//    extern auto createSystemDefaultContext() -> Arc<Device>;
}