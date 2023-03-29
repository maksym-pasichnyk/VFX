#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_raii.hpp>

namespace gfx::raii {
    struct Context {
        vk::DynamicLoader           loader;
        vk::raii::ContextDispatcher dispatcher;

        explicit Context() : dispatcher(loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr")) {}
    };

    struct Instance {
        vk::Instance                    raw;
        vk::raii::InstanceDispatcher    dispatcher;

        explicit Instance(vk::Instance raw, PFN_vkGetInstanceProcAddr getProcAddr) : raw(raw), dispatcher(getProcAddr, raw) {}
    };

    struct Device {
        vk::Device                  raw;
        vk::raii::DeviceDispatcher  dispatcher;

        explicit Device(vk::Device raw, PFN_vkGetDeviceProcAddr getProcAddr) : raw(raw), dispatcher(getProcAddr, raw) {}
    };
}

namespace gfx {
    struct Device;
    struct Surface;

    struct InstanceSettings {
        std::string name    = {};
        uint32_t    version = 0;
    };

    struct InstanceShared {
        raii::Context context;
        raii::Instance raii;
        vk::DebugUtilsMessengerEXT messenger;

        explicit InstanceShared(raii::Context context, raii::Instance raii, vk::DebugUtilsMessengerEXT messenger);
        ~InstanceShared();
    };

    struct Instance {
        std::shared_ptr<InstanceShared> shared;

        explicit Instance() : shared(nullptr) {}
        explicit Instance(std::shared_ptr<InstanceShared> shared) : shared(std::move(shared)) {}

        auto enumerateAdapters() -> std::vector<vk::PhysicalDevice>;
        auto createDevice(vk::PhysicalDevice adapter) -> Device;
        auto wrapSurface(vk::SurfaceKHR surface) -> Surface;

        auto getSurfaceCapabilitiesKHR(vk::PhysicalDevice adapter, vk::SurfaceKHR surface) -> vk::SurfaceCapabilitiesKHR;
    };

    extern auto createInstance(const InstanceSettings& desc) -> Instance;
}