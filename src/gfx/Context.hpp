#pragma once

#include "Object.hpp"

#include <list>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

namespace gfx {
    struct Device;
    struct Swapchain;
    struct Context : Referencing {
        friend Device;
        friend Swapchain;

    private:
        vk::Instance mInstance = {};
        vk::DynamicLoader mDynamicLoader = {};
        vk::DispatchLoaderDynamic mDispatchLoaderDynamic = {};
        vk::DebugUtilsMessengerEXT mDebugUtilsMessenger = {};

        std::vector<SharedPtr<Device>> mDevices = {};

    private:
        Context();
        ~Context() override;

    public:
        auto devices() -> const std::vector<SharedPtr<Device>>&;

    public:
        static auto alloc() -> SharedPtr<Context>;
    };
}