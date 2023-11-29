#define VMA_IMPLEMENTATION

#include "Instance.hpp"
#include "Surface.hpp"
#include "Device.hpp"
#include "Adapter.hpp"

#include "spdlog/spdlog.h"

static VKAPI_ATTR auto VKAPI_CALL debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32 {
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        spdlog::debug("{}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info("{}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("{}", pCallbackData->pMessage);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::error("{}", pCallbackData->pMessage);
    } else {
        spdlog::info("{}", pCallbackData->pMessage);
    }
    return VK_FALSE;
}

gfx::Instance::Instance(rc<Context> context, vk::InstanceCreateInfo const& create_info)
: context(std::move(context))
, handle(vk::createInstance(create_info, nullptr, this->context->dispatcher))
, dispatcher(this->context->dispatcher.vkGetInstanceProcAddr, this->handle) {}

gfx::Instance::~Instance() {
    this->handle.destroy(nullptr, this->dispatcher);
}

auto gfx::createInstance(InstanceConfiguration const& desc) -> rc<Instance> {
    auto context = MakeShared<Context>();

    auto app_info = vk::ApplicationInfo()
        .setPApplicationName(desc.name.c_str())
        .setApplicationVersion(desc.version)
        .setPEngineName("Dragon")
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(VK_API_VERSION_1_2);

    auto layers = std::vector<const char*> {
        "VK_LAYER_KHRONOS_synchronization2",
        "VK_LAYER_KHRONOS_validation"
    };

    auto extensions = std::vector<const char*> {
        "VK_KHR_surface",
        "VK_EXT_debug_utils",
        "VK_MVK_macos_surface",
        "VK_KHR_portability_enumeration",
        "VK_KHR_get_physical_device_properties2"
    };

    auto create_info = vk::InstanceCreateInfo()
        .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
        .setPApplicationInfo(&app_info)
        .setPEnabledLayerNames(layers)
        .setPEnabledExtensionNames(extensions);

//    auto message_severity_flags = vk::DebugUtilsMessageSeverityFlagsEXT{};
//    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
//    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
//    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
//    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
//    auto message_type_flags = vk::DebugUtilsMessageTypeFlagsEXT{};
//    message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
//    message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
//    message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
//    auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT()
//        .setMessageSeverity(message_severity_flags)
//        .setMessageType(message_type_flags)
//        .setPfnUserCallback(debug_utils_messenger_callback);
//    auto messenger = instance.handle.createDebugUtilsMessengerEXT(debug_create_info, nullptr, instance.dispatcher);
    return MakeShared<Instance>(std::move(context), create_info);
}

auto gfx::Instance::wrapSurface(this Instance& self, vk::SurfaceKHR surface) -> rc<Surface> {
    return MakeShared<Surface>(self.shared_from_this(), surface);
}

auto gfx::Instance::enumerateAdapters(this Instance& self) -> std::vector<rc<Adapter>> {
    auto physical_devices = self.handle.enumeratePhysicalDevices(self.dispatcher);

    std::vector<rc<Adapter>> adapters = {};
    adapters.reserve(physical_devices.size());
    for (auto& physical_device : physical_devices) {
        adapters.emplace_back(rc(new Adapter(self.shared_from_this(), physical_device)));
    }
    return adapters;
}