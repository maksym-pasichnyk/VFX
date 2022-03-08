#pragma once

#include <list>
#include <types.hpp>
#include <buffer.hpp>
#include <shader.hpp>
#include <display.hpp>
#include <texture.hpp>
#include <glm/vec4.hpp>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

struct Geometry {
    GraphicsBuffer* vtx = nullptr;
    GraphicsBuffer* idx = nullptr;

    u64 vtx_buf_size = 0;
    u64 idx_buf_size = 0;

    i32 vtx_count = 0;
    i32 idx_count = 0;

    u64 vtx_stride = 0;
    u64 idx_stride = 0;
};

struct MaterialDescription {
    struct Resource {
        u32 binding;
        vk::DescriptorType type;
        vk::ShaderStageFlags stage;
        u32 uniform_buffer_size;
    };

    std::vector<Resource> resources{};
    std::vector<vk::PushConstantRange> constants{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    vk::PipelineTessellationStateCreateInfo tessellationState = {};
    vk::PipelineViewportStateCreateInfo viewportState = {};
    vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
    vk::PipelineMultisampleStateCreateInfo multisampleState = {};
    vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
//    vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
    vk::PipelineLayout layout = {};
//    vk::Pipeline basePipelineHandle = {};
//    i32 basePipelineIndex = {};

    std::vector<vk::DynamicState> dynamic_states = {};
    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};
    std::vector<vk::VertexInputBindingDescription> bindings = {};
    std::vector<vk::VertexInputAttributeDescription> attributes = {};
    std::vector<vk::PipelineColorBlendAttachmentState> attachments = {};

    void create_sampler_resource(u32 binding, vk::ShaderStageFlags stage) {
        resources.emplace_back(Resource{binding, vk::DescriptorType::eCombinedImageSampler, stage, 0});
    }

    void create_uniform_buffer_resource(u32 binding, vk::ShaderStageFlags stage, u32 size) {
        resources.emplace_back(Resource{binding, vk::DescriptorType::eUniformBuffer, stage, size});
    }

    void create_input_attachment_resource(u32 binding, vk::ShaderStageFlags stage) {
        resources.emplace_back(Resource{binding, vk::DescriptorType::eInputAttachment, stage, 0});
    }

    auto create_constant(vk::ShaderStageFlags stage, u32 offset, u32 size) {
        constants.emplace_back(vk::PushConstantRange{stage, offset, size});
    }

    auto create_binding(vk::VertexInputRate rate) -> u32 {
        auto binding = u32(bindings.size());
        bindings.emplace_back(vk::VertexInputBindingDescription{
            .binding   = binding,
            .stride    = 0,
            .inputRate = rate
        });
        return binding;
    }

    auto create_attribute(u32 binding, vk::Format format, u32 size) -> u32 {
        auto location = u32(attributes.size());
        attributes.emplace_back(vk::VertexInputAttributeDescription{
            .location = location,
            .binding  = binding,
            .format   = format,
            .offset   = bindings[binding].stride,
        });
        bindings[binding].stride += size;
        return location;
    }

    auto create_attachment(
        bool blendEnable,
        vk::BlendFactor srcColorBlendFactor = vk::BlendFactor::eZero,
        vk::BlendFactor dstColorBlendFactor = vk::BlendFactor::eZero,
        vk::BlendOp colorBlendOp = vk::BlendOp::eAdd,
        vk::BlendFactor srcAlphaBlendFactor = vk::BlendFactor::eZero,
        vk::BlendFactor dstAlphaBlendFactor = vk::BlendFactor::eZero,
        vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd,
        vk::ColorComponentFlags colorWriteMask = {}
    ) -> u32 {
        auto attachment = u32(attachments.size());
        attachments.emplace_back(vk::PipelineColorBlendAttachmentState{
            .blendEnable = blendEnable ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = srcColorBlendFactor,
            .dstColorBlendFactor = dstColorBlendFactor,
            .colorBlendOp = colorBlendOp,
            .srcAlphaBlendFactor = srcAlphaBlendFactor,
            .dstAlphaBlendFactor = dstAlphaBlendFactor,
            .alphaBlendOp = alphaBlendOp,
            .colorWriteMask = colorWriteMask,
        });
        return attachment;
    }
};

struct Material {
    vk::Pipeline pipeline;
    vk::PipelineLayout pipeline_layout;
    vk::DescriptorPool descriptor_pool;
    vk::DescriptorSetLayout descriptor_set_layout;
    std::vector<vk::DescriptorSet> descriptor_sets;
};

struct Context {
    static constexpr auto MAX_FRAMES_IN_FLIGHT = 3u;

    Display& display;

    vk::DynamicLoader dl;
    vk::Instance instance;
    vk::SurfaceKHR surface;

    u32 present_family;
    u32 graphics_family;
    vk::Queue present_queue;
    vk::Queue graphics_queue;
    vk::Device logical_device;
    vk::PhysicalDevice physical_device;
    vk::DebugUtilsMessengerEXT debug_utils;

    vk::Format depth_format;
    vk::Extent2D surface_extent;
    vk::PresentModeKHR present_mode;
    vk::SurfaceFormatKHR surface_format;
    vk::SurfaceCapabilitiesKHR capabilities;

    u32 min_image_count = 0;

    u32 image_index = 0;
    u64 frame_index = 0;
    u64 current_frame = 0;
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> views;

    std::vector<vk::Fence> fences;
    std::vector<vk::Fence> image_fences;
    std::vector<vk::Semaphore> acquired_semaphores;
    std::vector<vk::Semaphore> complete_semaphores;

    std::vector<vk::CommandPool> command_pools;

    VmaAllocator allocator;

    std::vector<vk::CommandBuffer> command_buffers;
    std::list<std::pair<u64, std::function<void()>>> deleters;

    explicit Context(Display& display) : display(display) {
        create_instance();
        create_surface();
        select_physical_device();
        create_logical_device();
        create_memory_resource();
        create_swapchain();
        create_sync_objects();

        command_pools.resize(MAX_FRAMES_IN_FLIGHT);
        command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            const auto create_info = vk::CommandPoolCreateInfo {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = graphics_family
            };
            command_pools[i] = logical_device.createCommandPool(create_info);

            const auto allocate_info = vk::CommandBufferAllocateInfo{
                .commandPool = command_pools[i],
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1
            };
            command_buffers[i] = logical_device.allocateCommandBuffers(allocate_info)[0];
        }
    }

    ~Context() {
        for (auto& [_, deleter] : deleters) {
            deleter();
        }
        for (u64 i = 0; i < images.size(); i++) {
            logical_device.destroyImageView(views[i]);
        }
        for (u64 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

            logical_device.destroyFence(fences[i]);
            logical_device.destroySemaphore(acquired_semaphores[i]);
            logical_device.destroySemaphore(complete_semaphores[i]);

            logical_device.freeCommandBuffers(command_pools[i], command_buffers[i]);
            logical_device.destroyCommandPool(command_pools[i]);
        }

        logical_device.destroySwapchainKHR(swapchain);

        vmaDestroyAllocator(allocator);
        logical_device.destroy();

        instance.destroySurfaceKHR(surface);
        instance.destroyDebugUtilsMessengerEXT(debug_utils);
        instance.destroy();
    }

    void create_instance() {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        u32 count = 0;
        auto raw_extensions = glfwGetRequiredInstanceExtensions(&count);
        auto extensions = std::vector<const char *>(raw_extensions, raw_extensions + count);

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        const auto layers = std::array{
            "VK_LAYER_KHRONOS_validation"
        };

        auto app_info = vk::ApplicationInfo{
            .pApplicationName = nullptr,
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "SRP",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_2
        };

        const auto instance_create_info = vk::InstanceCreateInfo {
            .pApplicationInfo = &app_info,
            .enabledLayerCount = std::size(layers),
            .ppEnabledLayerNames = std::data(layers),
            .enabledExtensionCount = u32(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };
        instance = vk::createInstance(instance_create_info);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

        const auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT{
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = debug_callback
        };
        debug_utils = instance.createDebugUtilsMessengerEXT(debug_create_info);
    }

    void create_surface() {
        surface = display.create_surface(instance);
    }

    void select_physical_device() {
        for (auto device : instance.enumeratePhysicalDevices()) {
            const auto families = find_queue_families(device, surface);
            if (!families) {
                continue;
            }

            u32 surfaceFormatCount = 0;
            std::ignore = device.getSurfaceFormatsKHR(surface, &surfaceFormatCount, nullptr);
            if (surfaceFormatCount == 0) {
                continue;
            }

            u32 presentModeCount = 0;
            std::ignore = device.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);
            if (presentModeCount == 0) {
                continue;
            }

            physical_device = device;
            graphics_family = families->first;
            present_family = families->second;
            break;
        }
    }

    void create_logical_device() {
        const auto queuePriority = 1.0f;

        auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>{};

        const auto graphicsQueueCreateInfo = vk::DeviceQueueCreateInfo {
            .queueFamilyIndex = graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.emplace_back(graphicsQueueCreateInfo);

        if (graphics_family != present_family) {
            const auto presentQueueCreateInfo = vk::DeviceQueueCreateInfo {
                .queueFamilyIndex = present_family,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };

            queueCreateInfos.emplace_back(presentQueueCreateInfo);
        }

        static constexpr auto device_extensions = std::array{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
#ifdef __APPLE__
            VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
            VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
            VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME
//                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        const auto features = vk::PhysicalDeviceFeatures{
            .fillModeNonSolid = VK_TRUE,
            .samplerAnisotropy = VK_TRUE
        };

        auto imagelessFramebufferFeatures = vk::PhysicalDeviceImagelessFramebufferFeatures{
            .imagelessFramebuffer = VK_TRUE
        };

//            auto dynamicRenderingFeaturesKHR = vk::PhysicalDeviceDynamicRenderingFeaturesKHR{
//                .dynamicRendering = true
//            };

        const auto physicalDeviceFeatures2 = vk::PhysicalDeviceFeatures2{
//            .pNext = &dynamicRenderingFeaturesKHR,
            .pNext = &imagelessFramebufferFeatures,
            .features = features
        };

        const auto deviceCreateInfo = vk::DeviceCreateInfo {
            .pNext = &physicalDeviceFeatures2,
            .queueCreateInfoCount = u32(std::size(queueCreateInfos)),
            .pQueueCreateInfos = std::data(queueCreateInfos),
//                .enabledLayerCount = std::size(enabledLayers),
//                .ppEnabledLayerNames = std::data(enabledLayers),
            .enabledExtensionCount = std::size(device_extensions),
            .ppEnabledExtensionNames = std::data(device_extensions),
//            .pEnabledFeatures = &features
        };

        logical_device = physical_device.createDevice(deviceCreateInfo, nullptr);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(logical_device);

        present_queue = logical_device.getQueue(present_family, 0);
        graphics_queue = logical_device.getQueue(graphics_family, 0);
    }

    void create_memory_resource() {
        const auto functions = VmaVulkanFunctions{
            .vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr,
            .vkGetPhysicalDeviceProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceProperties,
            .vkGetPhysicalDeviceMemoryProperties = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties,
            .vkAllocateMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkAllocateMemory,
            .vkFreeMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkFreeMemory,
            .vkMapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkMapMemory,
            .vkUnmapMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkUnmapMemory,
            .vkFlushMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkFlushMappedMemoryRanges,
            .vkInvalidateMappedMemoryRanges = VULKAN_HPP_DEFAULT_DISPATCHER.vkInvalidateMappedMemoryRanges,
            .vkBindBufferMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory,
            .vkBindImageMemory = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory,
            .vkGetBufferMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements,
            .vkGetImageMemoryRequirements = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements,
            .vkCreateBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateBuffer,
            .vkDestroyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyBuffer,
            .vkCreateImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkCreateImage,
            .vkDestroyImage = VULKAN_HPP_DEFAULT_DISPATCHER.vkDestroyImage,
            .vkCmdCopyBuffer = VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdCopyBuffer,
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
            /// Fetch "vkGetBufferMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetBufferMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
            .vkGetBufferMemoryRequirements2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetBufferMemoryRequirements2KHR,
            /// Fetch "vkGetImageMemoryRequirements 2" on Vulkan >= 1.1, fetch "vkGetImageMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
            .vkGetImageMemoryRequirements2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetImageMemoryRequirements2KHR,
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
            /// Fetch "vkBindBufferMemory2" on Vulkan >= 1.1, fetch "vkBindBufferMemory2KHR" when using VK_KHR_bind_memory2 extension.
            .vkBindBufferMemory2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindBufferMemory2KHR,
            /// Fetch "vkBindImageMemory2" on Vulkan >= 1.1, fetch "vkBindImageMemory2KHR" when using VK_KHR_bind_memory2 extension.
            .vkBindImageMemory2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkBindImageMemory2KHR,
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
            .vkGetPhysicalDeviceMemoryProperties2KHR = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetPhysicalDeviceMemoryProperties2KHR,
#endif
        };

        const auto allocatorCreateInfo = VmaAllocatorCreateInfo{
            .physicalDevice = physical_device,
            .device = logical_device,
            .pVulkanFunctions = &functions,
            .instance = instance,
            .vulkanApiVersion = VK_API_VERSION_1_2
        };

        vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    }

    void create_swapchain() {
        depth_format = select_depth_format(physical_device);
        capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);

        const auto formats = physical_device.getSurfaceFormatsKHR(surface);
        const auto presentModes = physical_device.getSurfacePresentModesKHR(surface);

        const auto request_formats = std::array {
            vk::Format::eB8G8R8A8Unorm,
            vk::Format::eR8G8B8A8Unorm,
            vk::Format::eB8G8R8Unorm,
            vk::Format::eR8G8B8Unorm
        };

        const auto request_modes = std::array {
            vk::PresentModeKHR::eFifo
        };

        surface_extent = select_surface_extent(vk::Extent2D{0, 0}, capabilities);
        surface_format = select_surface_format(formats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);
        present_mode = select_present_mode(presentModes, request_modes);

        min_image_count = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0) {
            min_image_count = std::min(min_image_count, capabilities.maxImageCount);
        }

        const auto queue_family_indices = std::array{
            graphics_family,
            present_family
        };

        const auto flag = graphics_family != present_family;

        const auto swapchain_create_info = vk::SwapchainCreateInfoKHR {
            .surface = surface,
            .minImageCount = min_image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = surface_extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = flag ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = flag ? static_cast<u32>(queue_family_indices.size()) : 0,
            .pQueueFamilyIndices = flag ? queue_family_indices.data() : nullptr,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = true,
            .oldSwapchain = nullptr
        };

        swapchain = logical_device.createSwapchainKHR(swapchain_create_info, nullptr);
        images = logical_device.getSwapchainImagesKHR(swapchain);

        views.resize(images.size());
        for (u64 i = 0; i < images.size(); ++i) {
            const auto view_create_info = vk::ImageViewCreateInfo{
                .image = images[i],
                .viewType = vk::ImageViewType::e2D,
                .format = surface_format.format,
                .subresourceRange = {
                    vk::ImageAspectFlagBits::eColor,
                    0, 1, 0, 1
                }
            };
            views[i] = logical_device.createImageView(view_create_info);
        }
    }

    void create_sync_objects() {
        image_fences.resize(images.size());

        fences.resize(MAX_FRAMES_IN_FLIGHT);
        acquired_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        complete_semaphores.resize(MAX_FRAMES_IN_FLIGHT);

        for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            fences[i] = logical_device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
            acquired_semaphores[i] = logical_device.createSemaphore({});
            complete_semaphores[i] = logical_device.createSemaphore({});
        }
    }

    void recreate_swapchain() {
        const auto queue_family_indices = std::array{
            graphics_family,
            present_family
        };

        const auto flag = graphics_family != present_family;

        const auto swapchain_create_info = vk::SwapchainCreateInfoKHR {
            .surface = surface,
            .minImageCount = min_image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = surface_extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = flag ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = flag ? static_cast<u32>(queue_family_indices.size()) : 0,
            .pQueueFamilyIndices = flag ? queue_family_indices.data() : nullptr,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = present_mode,
            .clipped = true,
            .oldSwapchain = swapchain
        };

        swapchain = logical_device.createSwapchainKHR(swapchain_create_info, nullptr);
        images = logical_device.getSwapchainImagesKHR(swapchain);

        for (u64 i = 0; i < images.size(); ++i) {
            logical_device.destroyImageView(views[i]);

            const auto view_create_info = vk::ImageViewCreateInfo{
                .image = images[i],
                .viewType = vk::ImageViewType::e2D,
                .format = surface_format.format,
                .subresourceRange = {
                    vk::ImageAspectFlagBits::eColor,
                    0, 1, 0, 1
                }
            };
            views[i] = logical_device.createImageView(view_create_info);
        }
    }

    void present() {
        auto present_info = vk::PresentInfoKHR{};
        present_info.setWaitSemaphores(complete_semaphores[current_frame]);
        present_info.setSwapchains(swapchain);
        present_info.setImageIndices(image_index);
        auto result = present_queue.presentKHR(present_info);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
//            spdlog::error("out of date swapchain image");
//            recreate_swapchain();
            return;
        }
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swapchain image");
        }
    }

    auto begin_frame() -> bool {
        for (auto it = deleters.begin(); it != deleters.end();) {
            auto& [frame, deleter] = *it;
            if (frame_index > frame + 3) {
                deleter();
                it = deleters.erase(it);
            } else {
                it++;
            }
        }

        std::ignore = logical_device.waitForFences(fences[current_frame], true, std::numeric_limits<uint64_t>::max());
        auto result = logical_device.acquireNextImageKHR(
            swapchain,
            std::numeric_limits<uint64_t>::max(),
            acquired_semaphores[current_frame],
            nullptr,
            &image_index
        );
        if (result == vk::Result::eErrorOutOfDateKHR) {
//            spdlog::error("out of date swapchain image");
//            recreate_swapchain();
            return false;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire swapchain image");
        }

        command_buffers[current_frame].begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        return true;
    }

    void end_frame() {
        command_buffers[current_frame].end();

        if (auto fence = std::exchange(image_fences[image_index], fences[current_frame])) {
            std::ignore = logical_device.waitForFences(fence, true, std::numeric_limits<uint64_t>::max());
        }

        const auto stages = std::array{
            vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput}
        };

        auto submit_info = vk::SubmitInfo{};
        submit_info.setWaitDstStageMask(stages);
        submit_info.setCommandBuffers(command_buffers[current_frame]);
        submit_info.setWaitSemaphores(acquired_semaphores[current_frame]);
        submit_info.setSignalSemaphores(complete_semaphores[current_frame]);

        logical_device.resetFences(fences[current_frame]);
        graphics_queue.submit(submit_info, fences[current_frame]);

        present();

        frame_index += 1;
        current_frame = frame_index % MAX_FRAMES_IN_FLIGHT;
    }

    auto create_shader_module(std::span<const char> bytes) const {
        auto create_info = vk::ShaderModuleCreateInfo{
            .codeSize = bytes.size(),
            .pCode    = reinterpret_cast<const u32 *>(bytes.data())
        };
        return logical_device.createShaderModule(create_info);
    }

    auto create_texture_2d(u32 width, u32 height, vk::Format format, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect) -> Texture* {
        const auto image_create_info = static_cast<VkImageCreateInfo>(vk::ImageCreateInfo{
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {
                .width = width,
                .height = height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .usage = usage
        });

        VkImage image;
        VmaAllocation allocation;

        const auto allocation_create_info = VmaAllocationCreateInfo{.usage = VMA_MEMORY_USAGE_GPU_ONLY};
        vmaCreateImage(allocator, &image_create_info, &allocation_create_info, &image, &allocation, nullptr);

        const auto view_create_info = vk::ImageViewCreateInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .subresourceRange = vk::ImageSubresourceRange{
                aspect,
                0, 1, 0, 1
            }
        };
        auto view = logical_device.createImageView(view_create_info);

        return new Texture{
            .width = width,
            .height = height,
            .image = image,
            .view = view,
            .sampler = {},
            .allocation = allocation
        };
    }

    auto destroy_texture(Texture* texture) {
        deleters.emplace_back(frame_index, [this, texture] {
            logical_device.destroyImageView(texture->view);
            vmaDestroyImage(allocator, texture->image, texture->allocation);
            if (texture->sampler) {
                logical_device.destroySampler(texture->sampler);
            }
            delete texture;
        });
    }

    void set_texture_data(Texture* texture, std::span<const glm::u8vec4> pixels) {
        auto tmp = create_buffer(GraphicsBuffer::Target::CopySrc, static_cast<int>(pixels.size_bytes()));
        update_buffer(tmp, pixels.data(), pixels.size_bytes(), 0);

        const auto copy_barrier = vk::ImageMemoryBarrier{
            .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eTransferDstOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = texture->image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            }
        };

        const auto region = vk::BufferImageCopy{
            .imageSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .layerCount = 1
            },
            .imageExtent = {
                .width = texture->width,
                .height = texture->height,
                .depth = 1
            }
        };

        const auto use_barrier = vk::ImageMemoryBarrier{
            .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
            .dstAccessMask = vk::AccessFlagBits::eShaderRead,
            .oldLayout = vk::ImageLayout::eTransferDstOptimal,
            .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = texture->image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            }
        };

        const auto pool_create_info = vk::CommandPoolCreateInfo {
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = graphics_family
        };
        auto cmd_pool = logical_device.createCommandPool(pool_create_info);

        const auto cmd_allocate_info = vk::CommandBufferAllocateInfo{
            .commandPool = cmd_pool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1
        };
        auto cmd = logical_device.allocateCommandBuffers(cmd_allocate_info)[0];

        auto fence = logical_device.createFence({});

        cmd.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {copy_barrier});
        cmd.copyBufferToImage(tmp->buffer, texture->image, vk::ImageLayout::eTransferDstOptimal, {region});
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, {use_barrier});
        cmd.end();

        auto submit_info = vk::SubmitInfo{};
        submit_info.setCommandBuffers(cmd);
        graphics_queue.submit(1, &submit_info, fence);

        logical_device.waitForFences(fence, false, std::numeric_limits<uint64_t>::max());
        logical_device.freeCommandBuffers(cmd_pool, cmd);
        logical_device.destroyCommandPool(cmd_pool);
        logical_device.destroyFence(fence);
        destroy_buffer(tmp);
    }

    auto create_buffer(GraphicsBuffer::Target target, u64 size) -> GraphicsBuffer* {
        const auto buffer_create_info = static_cast<VkBufferCreateInfo>(vk::BufferCreateInfo {
            .size = static_cast<vk::DeviceSize>(size),
            .usage = get_buffer_usage_from_target(target)
        });

        const auto allocation_create_info = VmaAllocationCreateInfo {
            .usage = get_memory_usage_from_target(target)
        };

        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;

        vmaCreateBuffer(
            allocator,
            &buffer_create_info,
            &allocation_create_info,
            &buffer,
            &allocation,
            &allocation_info
        );
        return new GraphicsBuffer{
            buffer,
            allocation,
            allocation_info
        };
    }

    void destroy_buffer(GraphicsBuffer* gb) {
        deleters.emplace_back(frame_index, [this, gb] {
            vmaDestroyBuffer(allocator, gb->buffer, gb->allocation);
            delete gb;
        });
    }

    void update_buffer(GraphicsBuffer* gb, const void* src, u64 size, u64 offset) {
        void* ptr = nullptr;
        vmaMapMemory(allocator, gb->allocation, &ptr);
        auto dst = static_cast<std::byte*>(ptr) + offset;
        memcpy(dst, src, size);
        vmaUnmapMemory(allocator, gb->allocation);
    }

    void set_vertex_buffer_params(Geometry* geometry, i32 count, u64 stride) {
        const auto buf_size = static_cast<i32>(count * stride);

        geometry->vtx_count = count;
        geometry->vtx_stride = stride;
        if (buf_size > geometry->vtx_buf_size) {
            geometry->vtx_buf_size = buf_size;
            if (geometry->vtx != nullptr) {
                destroy_buffer(geometry->vtx);
            }
            geometry->vtx = create_buffer(GraphicsBuffer::Target::Vertex, buf_size);
        }
    }

    void set_index_buffer_params(Geometry* geometry, i32 count, u64 stride) {
        const auto buf_size = static_cast<i32>(count * stride);

        geometry->idx_count = count;
        geometry->idx_stride = stride;
        if (buf_size > geometry->idx_buf_size) {
            geometry->idx_buf_size = buf_size;
            if (geometry->idx != nullptr) {
                destroy_buffer(geometry->idx);
            }
            geometry->idx = create_buffer(GraphicsBuffer::Target::Index, buf_size);
        }
    }

    void set_vertex_buffer_data(Geometry* geometry, const void* src, u64 size, u64 offset) {
        update_buffer(geometry->vtx, src, size * geometry->vtx_stride, offset * geometry->vtx_stride);
    }

    void set_index_buffer_data(Geometry* geometry, const void* src, u64 size, u64 offset) {
        update_buffer(geometry->idx, src, size * geometry->idx_stride, offset * geometry->idx_stride);
    }

    auto create_shader_effect() -> ShaderEffect* {
        return new ShaderEffect();
    }

    auto destroy_shader_effect(ShaderEffect* effect) {
        for (auto& stage : effect->stages) {
            logical_device.destroyShaderModule(stage.module);
        }
        logical_device.destroyPipeline(effect->pipeline);
        logical_device.destroyPipelineLayout(effect->layout);

        if (effect->resources.pool) {
            logical_device.destroyDescriptorPool(effect->resources.pool);
        }
        if (effect->resources.layout) {
            logical_device.destroyDescriptorSetLayout(effect->resources.layout);
        }
        delete effect;
    }

    void create_shader_effect_layout(ShaderEffect* effect) {
        auto create_info = vk::PipelineLayoutCreateInfo{};
        if (effect->resources.layout) {
            create_info.setSetLayouts(effect->resources.layout);
            create_info.setPushConstantRanges(effect->resources.constants);
        }
        effect->layout = logical_device.createPipelineLayout(create_info);
    }

    void create_shader_effect_descriptors(ShaderEffect* effect) {
        auto layouts = std::array{
            effect->resources.layout,
            effect->resources.layout,
            effect->resources.layout
        };
        auto allocate_info = vk::DescriptorSetAllocateInfo{};
        allocate_info.setDescriptorPool(effect->resources.pool);
        allocate_info.setSetLayouts(layouts);
        effect->resources.descriptors = logical_device.allocateDescriptorSets(allocate_info);
    }

    auto create_material(const MaterialDescription& description, vk::RenderPass pass, u32 subpass) -> Material* {
        std::map<vk::DescriptorType, u32> dp_table{};
        std::vector<vk::DescriptorPoolSize> dp_sizes{};
        std::vector<vk::DescriptorSetLayoutBinding> dsl_bindings{};

        for (auto& resource : description.resources) {
            dp_table[resource.type] += Context::MAX_FRAMES_IN_FLIGHT;
            dsl_bindings.emplace_back(vk::DescriptorSetLayoutBinding{
                .binding = resource.binding,
                .descriptorType = resource.type,
                .descriptorCount = 1,
                .stageFlags = resource.stage,
                .pImmutableSamplers = nullptr
            });
        }

        dp_sizes.reserve(dp_table.size());
        for (auto [type, count] : dp_table) {
            dp_sizes.emplace_back(vk::DescriptorPoolSize{type, count});
        }

        auto material = new Material();

        auto dp_create_info = vk::DescriptorPoolCreateInfo{};
        dp_create_info.setMaxSets(Context::MAX_FRAMES_IN_FLIGHT);
        dp_create_info.setPoolSizes(dp_sizes);
        material->descriptor_pool = logical_device.createDescriptorPool(dp_create_info, nullptr);

        auto dsl_create_info = vk::DescriptorSetLayoutCreateInfo{};
        dsl_create_info.setBindings(dsl_bindings);
        material->descriptor_set_layout = logical_device.createDescriptorSetLayout(dsl_create_info);

        auto pipeline_layput_create_info = vk::PipelineLayoutCreateInfo{};
        if (material->descriptor_set_layout) {
            pipeline_layput_create_info.setSetLayouts(material->descriptor_set_layout);
            pipeline_layput_create_info.setPushConstantRanges(description.constants);
        }
        material->pipeline_layout = logical_device.createPipelineLayout(pipeline_layput_create_info);

        auto ds_layouts = std::array{
            material->descriptor_set_layout,
            material->descriptor_set_layout,
            material->descriptor_set_layout
        };
        auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
        ds_allocate_info.setDescriptorPool(material->descriptor_pool);
        ds_allocate_info.setSetLayouts(ds_layouts);
        material->descriptor_sets = logical_device.allocateDescriptorSets(ds_allocate_info);

        auto vertexInputState = vk::PipelineVertexInputStateCreateInfo{};
        vertexInputState.setVertexBindingDescriptions(description.bindings);
        vertexInputState.setVertexAttributeDescriptions(description.attributes);

        auto colorBlendState = vk::PipelineColorBlendStateCreateInfo{};
        colorBlendState.setAttachments(description.attachments);

        auto dynamicState = vk::PipelineDynamicStateCreateInfo{};
        dynamicState.setDynamicStates(description.dynamic_states);

        auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{};
        pipeline_create_info.pVertexInputState = &vertexInputState;
        pipeline_create_info.pInputAssemblyState = &description.inputAssemblyState;
        pipeline_create_info.pViewportState = &description.viewportState;
        pipeline_create_info.pRasterizationState = &description.rasterizationState;
        pipeline_create_info.pMultisampleState = &description.multisampleState;
        pipeline_create_info.pDepthStencilState = &description.depthStencilState;
        pipeline_create_info.pColorBlendState = &colorBlendState;
        pipeline_create_info.pDynamicState = &dynamicState;
        pipeline_create_info.layout = material->pipeline_layout;
        pipeline_create_info.renderPass = pass;
        pipeline_create_info.subpass = subpass;
        pipeline_create_info.basePipelineHandle = nullptr;
        pipeline_create_info.basePipelineIndex = 0;

        pipeline_create_info.setStages(description.stages);

        auto result = logical_device.createGraphicsPipelines(
            {},
            1,
            &pipeline_create_info,
            nullptr,
            &material->pipeline
        );

        if (result != vk::Result::eSuccess) {
            spdlog::error("{}", vk::to_string(result));
        }

        for (auto& stage : description.stages) {
            logical_device.destroyShaderModule(stage.module);
        }

        return material;
    }

    auto destroy_material(Material* material) {
//        for (auto& stage : material->stages) {
//            logical_device.destroyShaderModule(stage.module);
//        }
        logical_device.destroyPipeline(material->pipeline);
        logical_device.destroyPipelineLayout(material->pipeline_layout);

        if (material->descriptor_pool) {
            logical_device.destroyDescriptorPool(material->descriptor_pool);
        }
        if (material->descriptor_set_layout) {
            logical_device.destroyDescriptorSetLayout(material->descriptor_set_layout);
        }
        delete material;
    }

    static constexpr auto get_buffer_usage_from_target(GraphicsBuffer::Target target) -> vk::BufferUsageFlags {
        using Type = std::underlying_type_t<GraphicsBuffer::Target>;

        auto flags = vk::BufferUsageFlags{};
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::Vertex)) {
            flags |= vk::BufferUsageFlagBits::eVertexBuffer;
        }
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::Index)) {
            flags |= vk::BufferUsageFlagBits::eIndexBuffer;
        }
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::CopySrc)) {
            flags |= vk::BufferUsageFlagBits::eTransferSrc;
        }
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::CopyDst)) {
            flags |= vk::BufferUsageFlagBits::eTransferDst;
        }
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::Constant)) {
            flags |= vk::BufferUsageFlagBits::eUniformBuffer;
        }
        return flags;
    }

    static constexpr auto get_memory_usage_from_target(GraphicsBuffer::Target target) -> VmaMemoryUsage {
        using Type = std::underlying_type_t<GraphicsBuffer::Target>;

        auto flags = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::CopySrc)) {
            flags = VMA_MEMORY_USAGE_CPU_ONLY;
        }
        if (static_cast<Type>(target) & static_cast<Type>(GraphicsBuffer::Target::CopyDst)) {
            flags = VMA_MEMORY_USAGE_GPU_ONLY;
        }
        return flags;
    }

    static auto select_surface_extent(const vk::Extent2D& extent, const vk::SurfaceCapabilitiesKHR &capabilities) -> vk::Extent2D {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
            return capabilities.currentExtent;
        }

        const auto minExtent = capabilities.minImageExtent;
        const auto maxExtent = capabilities.maxImageExtent;

        return {
                std::clamp(extent.width, minExtent.width, maxExtent.width),
                std::clamp(extent.height, minExtent.height, maxExtent.height)
        };
    }

    static auto select_surface_format(std::span<const vk::SurfaceFormatKHR> surface_formats, std::span<const vk::Format> request_formats, vk::ColorSpaceKHR request_color_space) -> vk::SurfaceFormatKHR {
        if (surface_formats.size() == 1) {
            if (surface_formats.front().format == vk::Format::eUndefined) {
                return vk::SurfaceFormatKHR {
                        .format = request_formats.front(),
                        .colorSpace = request_color_space
                };
            }
            return surface_formats.front();
        }

        for (auto&& request_format : request_formats) {
            for (auto&& surface_format : surface_formats) {
                if (surface_format.format == request_format && surface_format.colorSpace == request_color_space) {
                    return surface_format;
                }
            }
        }
        return surface_formats.front();
    }

    static auto select_present_mode(std::span<const vk::PresentModeKHR> present_modes, std::span<const vk::PresentModeKHR> request_modes) -> vk::PresentModeKHR {
        for (auto request_mode : request_modes) {
            for (auto present_mode : present_modes) {
                if (request_mode == present_mode) {
                    return request_mode;
                }
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    static auto get_supported_format(vk::PhysicalDevice device, std::span<const vk::Format> formats, vk::FormatFeatureFlags flags) -> vk::Format {
        for (auto format : formats) {
            const auto properties = device.getFormatProperties(format);
            if ((properties.optimalTilingFeatures & flags) == flags) {
                return format;
            }
        }
        for (auto format : formats) {
            const auto properties = device.getFormatProperties(format);
            if ((properties.linearTilingFeatures & flags) == flags) {
                return format;
            }
        }
        return vk::Format::eUndefined;
    }

    static auto select_depth_format(vk::PhysicalDevice device) -> vk::Format {
        static constexpr auto formats = std::array{
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint,
            vk::Format::eD32Sfloat
        };
        return get_supported_format(
            device,
            formats,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    }

    static auto find_queue_families(vk::PhysicalDevice device, vk::SurfaceKHR surface) -> std::optional<std::pair<u32, u32>> {
        const auto properties = device.getQueueFamilyProperties();

        u32 graphics_family = -1;
        u32 present_family = -1;
        for (u32 i = 0; i < u32(properties.size()); i++) {
            if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphics_family = i;
            }

            if (device.getSurfaceSupportKHR(i, surface)) {
                present_family = i;
            }

            if ((graphics_family != -1) && (present_family != -1)) {
                return std::pair{ graphics_family, present_family };
            }
        }
        return std::nullopt;
    }

    static VKAPI_ATTR auto VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    ) -> VkBool32 {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            spdlog::info("{}", pCallbackData->pMessage);
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
};
