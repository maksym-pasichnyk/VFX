#include "Application.hpp"
#include "Device.hpp"
#include "Window.hpp"

#include "spdlog/spdlog.h"

extern VKAPI_ATTR auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32;

gfx::Application::Application() {
    vkDispatchLoaderDynamic.init(vkDynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

    vk::ApplicationInfo application_info = {};
    application_info.setPApplicationName("");
    application_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    application_info.setPEngineName("Gfx");
    application_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    application_info.setApiVersion(VK_API_VERSION_1_2);

    std::vector<const char*> layers = {};
    layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");

    bool enableApiValidation = [s = getenv("GFX_ENABLE_API_VALIDATION")] {
        return s && strcmp(s, "1") == 0;
    }();

    if (enableApiValidation) {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    auto available_extensions = vk::enumerateInstanceExtensionProperties(nullptr, vkDispatchLoaderDynamic);

    std::vector<const char*> extensions = {};
    for (auto& extension_properties : available_extensions) {
        extensions.emplace_back(extension_properties.extensionName);
    }

    auto enabled_validation_features = std::array{
//        vk::ValidationFeatureEnableEXT::eGpuAssisted,
//        vk::ValidationFeatureEnableEXT::eBestPractices,
        vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
    };
    vk::ValidationFeaturesEXT validation_features = {};
    validation_features.setEnabledValidationFeatures(enabled_validation_features);

    vk::InstanceCreateInfo instance_create_info = {};
    instance_create_info.setPNext(&validation_features);
#ifdef __APPLE__
    instance_create_info.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif
    instance_create_info.setPApplicationInfo(&application_info);
    instance_create_info.setPEnabledLayerNames(layers);
    instance_create_info.setPEnabledExtensionNames(extensions);

    vkInstance = vk::createInstance(instance_create_info, nullptr, vkDispatchLoaderDynamic);
    vkDispatchLoaderDynamic.init(vkInstance);

    if (enableApiValidation) {
        vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags = {};
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        vk::DebugUtilsMessageTypeFlagsEXT message_type_flags = {};
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = {};
        debug_create_info.setMessageSeverity(message_severity_flags);
        debug_create_info.setMessageType(message_type_flags);
        debug_create_info.setPfnUserCallback(debug_callback);

        vkDebugUtilsMessengerEXT = vkInstance.createDebugUtilsMessengerEXT(debug_create_info, nullptr, vkDispatchLoaderDynamic);
    }

    for (vk::PhysicalDevice gpu : vkInstance.enumeratePhysicalDevices(vkDispatchLoaderDynamic)) {
        mDevices.emplace_back(TransferPtr(new Device(this, gpu)));
    }
}

gfx::Application::~Application() {
    vkInstance.destroyDebugUtilsMessengerEXT(vkDebugUtilsMessengerEXT, nullptr, vkDispatchLoaderDynamic);
}

auto gfx::Application::devices() -> const std::vector<SharedPtr<Device>>& {
    return mDevices;
}

auto gfx::Application::alloc() -> SharedPtr<Application> {
    return TransferPtr(new Application());
}

VKAPI_ATTR auto VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32 {
    static constexpr const char* skip[] = {
        "UNASSIGNED-BestPractices-NonSuccess-Result",
        "VUID-vkCmdPushConstants-offset-01796",
        "VUID-vkCmdBindPipeline-pipeline-06197"
    };

    for (const char* msg : skip) {
        if (strstr(pCallbackData->pMessage, msg) != nullptr) {
            return VK_FALSE;
        }
    }

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