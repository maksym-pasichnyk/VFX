#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_raii.hpp>

#include "ManagedObject.hpp"

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
    struct Adapter;

    struct InstanceConfiguration {
        std::string name    = {};
        uint32_t    version = 0;
    };

    struct Instance : ManagedObject<Instance> {
        raii::Context               context;
        raii::Instance              raii;
        vk::DebugUtilsMessengerEXT  messenger;

        explicit Instance(raii::Context context, raii::Instance raii, vk::DebugUtilsMessengerEXT messenger);
        ~Instance() override;

        auto enumerateAdapters() -> std::vector<ManagedShared<Adapter>>;
        auto createDevice(ManagedShared<Adapter> adapter) -> ManagedShared<Device>;
        auto wrapSurface(vk::SurfaceKHR surface) -> ManagedShared<Surface>;

        auto getSurfaceCapabilitiesKHR(vk::PhysicalDevice adapter, vk::SurfaceKHR surface) -> vk::SurfaceCapabilitiesKHR;
    };

    extern auto createInstance(const InstanceConfiguration& desc) -> ManagedShared<Instance>;
}