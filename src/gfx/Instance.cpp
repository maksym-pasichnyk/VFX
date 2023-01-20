#define VMA_IMPLEMENTATION

#include "Instance.hpp"
#include "Surface.hpp"
#include "Device.hpp"

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

gfx::InstanceShared::InstanceShared(raii::Context context, raii::Instance instance, vk::DebugUtilsMessengerEXT messenger) : context(std::move(context)), instance(std::move(instance)), messenger(messenger) {}

gfx::InstanceShared::~InstanceShared() {
    instance.raw.destroy(messenger, nullptr, instance.dispatcher);
    instance.raw.destroy(nullptr, instance.dispatcher);
}

auto gfx::Instance::init(const InstanceSettings& desc) -> Instance {
    raii::Context context;

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

    auto instance = raii::Instance(vk::createInstance(create_info, nullptr, context.dispatcher), context.dispatcher.vkGetInstanceProcAddr);

    auto message_severity_flags = vk::DebugUtilsMessageSeverityFlagsEXT{};
    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    auto message_type_flags = vk::DebugUtilsMessageTypeFlagsEXT{};
    message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
    message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT()
        .setMessageSeverity(message_severity_flags)
        .setMessageType(message_type_flags)
        .setPfnUserCallback(debug_utils_messenger_callback);

    auto messenger = instance.raw.createDebugUtilsMessengerEXT(debug_create_info, nullptr, instance.dispatcher);

    return Instance(std::make_shared<InstanceShared>(std::move(context), std::move(instance), std::move(messenger)));
}

auto gfx::Instance::createDevice(vk::PhysicalDevice adapter) -> Device {
    auto queue_family_properties = adapter.getQueueFamilyProperties(shared->instance.dispatcher);

    float_t queue_priority = 1.0F;
    uint32_t queue_family_index = 0;
    for (uint32_t i = 0; i < queue_family_properties.size(); ++i) {
        if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            queue_family_index = i;
            break;
        }
    }

    auto queue_create_info = vk::DeviceQueueCreateInfo()
        .setQueueFamilyIndex(queue_family_index)
        .setQueuePriorities(queue_priority);

    auto queue_create_infos = std::array{
        queue_create_info
    };

    auto layers = std::vector<const char*>{};

    auto extensions = std::vector<const char*>{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
    };

    auto synchronization_2_features = vk::PhysicalDeviceSynchronization2Features()
        .setSynchronization2(VK_TRUE);

    auto portability_subset_features = vk::PhysicalDevicePortabilitySubsetFeaturesKHR()
        .setPNext(&synchronization_2_features)
        .setImageViewFormatSwizzle(VK_TRUE);

    auto timeline_semaphore_features = vk::PhysicalDeviceTimelineSemaphoreFeatures()
        .setPNext(&portability_subset_features)
        .setTimelineSemaphore(VK_TRUE);

    auto dynamic_rendering_features = vk::PhysicalDeviceDynamicRenderingFeatures()
        .setPNext(&timeline_semaphore_features)
        .setDynamicRendering(VK_TRUE);

    auto features_2 = adapter.getFeatures2(shared->instance.dispatcher)
        .setPNext(&dynamic_rendering_features);

    auto create_info = vk::DeviceCreateInfo()
        .setPNext(&features_2)
        .setQueueCreateInfos(queue_create_infos)
        .setPEnabledLayerNames(layers)
        .setPEnabledExtensionNames(extensions);

    auto device = raii::Device(adapter.createDevice(create_info, nullptr, shared->instance.dispatcher), shared->instance.dispatcher.vkGetDeviceProcAddr);

    VmaVulkanFunctions functions = {};
    functions.vkGetDeviceProcAddr = shared->instance.dispatcher.vkGetDeviceProcAddr;
    functions.vkGetInstanceProcAddr = shared->instance.dispatcher.vkGetInstanceProcAddr;

    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.physicalDevice = adapter;
    allocator_create_info.device = device.raw;
    allocator_create_info.pVulkanFunctions = &functions;
    allocator_create_info.instance = shared->instance.raw;
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_2;

    VmaAllocator allocator;
    vmaCreateAllocator(&allocator_create_info, &allocator);

    auto queue = device.raw.getQueue(queue_family_index, 0, device.dispatcher);

    return Device(std::make_shared<DeviceShared>(*this, device, adapter, queue_family_index, 0, queue, allocator));
}

auto gfx::Instance::wrapSurface(vk::SurfaceKHR surface) -> Surface {
    return Surface(std::make_shared<SurfaceShared>(*this, surface));
}

auto gfx::Instance::handle() -> vk::Instance {
    return shared->instance.raw;
}

auto gfx::Instance::dispatcher() -> const vk::raii::InstanceDispatcher& {
    return shared->instance.dispatcher;
}

auto gfx::Instance::enumerateAdapters() -> std::vector<vk::PhysicalDevice> {
    return shared->instance.raw.enumeratePhysicalDevices(shared->instance.dispatcher);
}