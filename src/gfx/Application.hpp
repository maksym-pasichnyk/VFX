#pragma once

#include "Object.hpp"
#include "Window.hpp"

#include <list>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

namespace gfx {
    struct Device;
    struct Window;
    struct Surface;

    struct Application final : Referencing<Application> {
        friend Device;
        friend Window;
        friend Surface;

    private:
        vk::Instance vkInstance = {};
        vk::DynamicLoader vkDynamicLoader = {};
        vk::DispatchLoaderDynamic vkDispatchLoaderDynamic = {};
        vk::DebugUtilsMessengerEXT vkDebugUtilsMessengerEXT = {};
        std::vector<SharedPtr<Device>> mDevices = {};

    private:
        Application();
        ~Application() override;

    public:
        auto devices() -> const std::vector<SharedPtr<Device>>&;

    public:
        static auto alloc() -> SharedPtr<Application>;
    };
}