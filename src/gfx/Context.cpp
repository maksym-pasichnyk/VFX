#define VMA_IMPLEMENTATION

#include "Context.hpp"
#include "Device.hpp"
#include "spdlog/spdlog.h"

extern VKAPI_ATTR auto VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32;

gfx::Context::Context() {
    mDispatchLoaderDynamic.init(mDynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

    vk::ApplicationInfo application_info = {};
    application_info.setPApplicationName("");
    application_info.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    application_info.setPEngineName("Gfx");
    application_info.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    application_info.setApiVersion(VK_API_VERSION_1_2);

    std::vector<const char*> required_instance_layers = {};
    required_instance_layers.emplace_back("VK_LAYER_KHRONOS_synchronization2");

    if (strcmp(getenv("GFX_ENABLE_API_VALIDATION") ?: "0", "1") == 0) {
        required_instance_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    auto instance_extension_properties = vk::enumerateInstanceExtensionProperties(nullptr, mDispatchLoaderDynamic);

    std::vector<const char*> required_instance_extensions = {};
    for (auto& extension_properties : instance_extension_properties) {
        required_instance_extensions.emplace_back(extension_properties.extensionName);
    }

    std::vector<vk::ValidationFeatureEnableEXT> enabled_validation_features = {};
    enabled_validation_features.emplace_back(vk::ValidationFeatureEnableEXT::eSynchronizationValidation);

    vk::ValidationFeaturesEXT validation_features = {};
    validation_features.setEnabledValidationFeatures(enabled_validation_features);

    vk::InstanceCreateInfo instance_create_info = {};
    instance_create_info.setPNext(&validation_features);
    instance_create_info.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
    instance_create_info.setPApplicationInfo(&application_info);
    instance_create_info.setPEnabledLayerNames(required_instance_layers);
    instance_create_info.setPEnabledExtensionNames(required_instance_extensions);

    mInstance = vk::createInstance(instance_create_info, nullptr, mDispatchLoaderDynamic);
    mDispatchLoaderDynamic.init(mInstance);

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
    debug_create_info.setPfnUserCallback(debugCallback);

    mDebugUtilsMessenger = mInstance.createDebugUtilsMessengerEXT(debug_create_info, nullptr, mDispatchLoaderDynamic);

    for (vk::PhysicalDevice gpu : mInstance.enumeratePhysicalDevices(mDispatchLoaderDynamic)) {
        mDevices.emplace_back(TransferPtr(new Device(this, gpu)));
    }
}

gfx::Context::~Context() {
    mInstance.destroyDebugUtilsMessengerEXT(mDebugUtilsMessenger, nullptr, mDispatchLoaderDynamic);
}

auto gfx::Context::devices() -> const std::vector<SharedPtr<Device>>& {
    return mDevices;
}

auto gfx::Context::alloc() -> SharedPtr<Context> {
    return TransferPtr(new Context());
}

VKAPI_ATTR auto VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32 {
//    static constexpr const char* skip[] = {
//        "UNASSIGNED-BestPractices-NonSuccess-Result",
//        "VUID-vkCmdPushConstants-offset-01796",
//        "VUID-vkCmdBindPipeline-pipeline-06197"
//    };
//    for (const char* msg : skip) {
//        if (strstr(pCallbackData->pMessage, msg) != nullptr) {
//            return VK_FALSE;
//        }
//    }

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