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
        vk::Instance vkInstance = {};
        vk::DynamicLoader vkDynamicLoader = {};
        vk::DispatchLoaderDynamic vkDispatchLoaderDynamic = {};
        vk::DebugUtilsMessengerEXT vkDebugUtilsMessenger = {};

        std::vector<SharedPtr<Device>> mDevices = {};

    public:
        Context();
        ~Context() override;
    };
}