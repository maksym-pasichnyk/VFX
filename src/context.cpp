#include "context.hpp"
#include "spdlog/spdlog.h"

namespace vfx {
    inline VKAPI_ATTR auto VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    ) -> VkBool32 {
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
}

vfx::Context::Context() {
    interface.init(dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

    auto application_info = vk::ApplicationInfo{
        .pApplicationName = "",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Vulkan",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };

    auto enableApiValidation = [s = getenv("VFX_ENABLE_API_VALIDATION")] {
        return s && strcmp(s, "1") == 0;
    }();

    std::vector<const char*> extensions {
        VK_KHR_SURFACE_EXTENSION_NAME,
        "VK_EXT_metal_surface",
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    };

    std::vector<const char*> layers {
        "VK_LAYER_KHRONOS_synchronization2"
    };

    if (enableApiValidation) {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    auto enabled_validation_features = std::array{
//        vk::ValidationFeatureEnableEXT::eGpuAssisted,
//        vk::ValidationFeatureEnableEXT::eBestPractices,
        vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
    };
    auto validation_features = vk::ValidationFeaturesEXT{};
    validation_features.setEnabledValidationFeatures(enabled_validation_features);

    auto instance_create_info = vk::InstanceCreateInfo {};
    instance_create_info.setPNext(&validation_features);
#ifdef __APPLE__
    instance_create_info.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif
    instance_create_info.setPApplicationInfo(&application_info);
    instance_create_info.setPEnabledLayerNames(layers);
    instance_create_info.setPEnabledExtensionNames(extensions);
    instance = vk::createInstanceUnique(instance_create_info, VK_NULL_HANDLE, interface);
    interface.init(*instance);

    if (enableApiValidation) {
        vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags{};
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
        message_severity_flags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{};
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        message_type_flags |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        const auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT{
            .messageSeverity = message_severity_flags,
            .messageType = message_type_flags,
            .pfnUserCallback = debug_callback
        };
        debug_utils = instance->createDebugUtilsMessengerEXTUnique(debug_create_info, VK_NULL_HANDLE, interface);
    }
}