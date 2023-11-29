#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_raii.hpp>

#include "ManagedObject.hpp"

namespace gfx {
    struct Device;
    struct Surface;
    struct Adapter;

    struct InstanceConfiguration {
        std::string name    = {};
        uint32_t    version = 0;
    };

    struct Context : public ManagedObject {
        vk::DynamicLoader           loader;
        vk::raii::ContextDispatcher dispatcher;

        explicit Context() : dispatcher(loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr")) {}
    };

    struct Instance : public ManagedObject {
        rc<Context>                     context;
        vk::Instance                    handle;
        vk::raii::InstanceDispatcher    dispatcher;

        explicit Instance(rc<Context> context, vk::InstanceCreateInfo const& create_info);
        ~Instance() override;

        auto enumerateAdapters(this Instance& self) -> std::vector<rc<Adapter>>;
        auto wrapSurface(this Instance& self, vk::SurfaceKHR surface) -> rc<Surface>;
    };

    extern auto createInstance(const InstanceConfiguration& desc) -> rc<Instance>;
}