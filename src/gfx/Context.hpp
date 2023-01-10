#pragma once

#include "Object.hpp"

#include <list>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

namespace gfx {
    struct Device;
    struct Context : Referencing {
    public:
        vk::Instance mInstance = {};
        vk::DynamicLoader mDynamicLoader = {};
        vk::DispatchLoaderDynamic mDispatchLoaderDynamic = {};
        vk::DebugUtilsMessengerEXT mDebugUtilsMessenger = {};

        std::vector<SharedPtr<Device>> mDevices = {};

    public:
        Context();
        ~Context() override;
    };
}