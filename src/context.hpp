#pragma once

#include <list>
#include <glm/vec4.hpp>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

#include "pass.hpp"
#include "mesh.hpp"
#include "queue.hpp"
#include "types.hpp"
#include "buffer.hpp"
#include "display.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "spirv_reflect.h"

namespace vfx {
    struct ContextDescription {
        std::string app_name = {};
        vk::InstanceCreateFlags flags{};
        std::vector<const char*> layers{};
        std::vector<const char*> extensions{};

        bool enable_debug = false;
    };

    struct Context {
    public:
        static constexpr auto MAX_FRAMES_IN_FLIGHT = 3u;

        vk::DynamicLoader dl{};
        vk::Instance instance{};
        VmaAllocator allocator{};
        vk::DebugUtilsMessengerEXT debug_utils{};

        u32 present_family{};
        u32 graphics_family{};
        vk::Queue present_queue{};
        vk::Queue graphics_queue{};
        vk::PhysicalDevice physical_device{};

        vk::Device logical_device{};
        vk::Format depth_format{};

    public:
        explicit Context(const ContextDescription& description) {
            create_instance(description);
            select_physical_device();
            create_logical_device();
            create_memory_allocator();
        }

        ~Context() {
            vmaDestroyAllocator(allocator);
            logical_device.destroy();

            if (debug_utils) {
                instance.destroyDebugUtilsMessengerEXT(debug_utils);
            }
            instance.destroy();
        }

    private:
        void create_instance(const ContextDescription& description) {
            vk::defaultDispatchLoaderDynamic.init(dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

            auto application_info = vk::ApplicationInfo{
                .pApplicationName = description.app_name.c_str(),
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "Vulkan",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_API_VERSION_1_2
            };

            auto instance_create_info = vk::InstanceCreateInfo {};
            instance_create_info.setFlags(description.flags);
            instance_create_info.setPApplicationInfo(&application_info);
            instance_create_info.setPEnabledLayerNames(description.layers);
            instance_create_info.setPEnabledExtensionNames(description.extensions);
            instance = vk::createInstance(instance_create_info);
            vk::defaultDispatchLoaderDynamic.init(instance);

            if (description.enable_debug) {
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
                debug_utils = instance.createDebugUtilsMessengerEXT(debug_create_info);
            }
        }

        void select_physical_device() {
            for (auto device : instance.enumeratePhysicalDevices()) {
                const auto families = find_queue_families(instance, device);
                if (!families) {
                    continue;
                }

                physical_device = device;
                graphics_family = families->first;
                present_family = families->second;
                break;
            }
            depth_format = select_depth_format(physical_device);
        }

        void create_logical_device() {
            f32 queue_priority = 1.0f;
            auto queue_create_infos = std::vector<vk::DeviceQueueCreateInfo>{};

            const auto graphics_queue_create_info = vk::DeviceQueueCreateInfo {
                .queueFamilyIndex = graphics_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority
            };
            queue_create_infos.emplace_back(graphics_queue_create_info);

            if (graphics_family != present_family) {
                const auto present_queue_create_info = vk::DeviceQueueCreateInfo {
                    .queueFamilyIndex = present_family,
                    .queueCount = 1,
                    .pQueuePriorities = &queue_priority
                };

                queue_create_infos.emplace_back(present_queue_create_info);
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
                VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
                VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
            };

            auto timeline_semaphore_features = vk::PhysicalDeviceTimelineSemaphoreFeatures{
                .timelineSemaphore = true
            };

            auto dynamic_rendering_features = vk::PhysicalDeviceDynamicRenderingFeatures{
                .pNext = &timeline_semaphore_features,
                .dynamicRendering = true,
            };

            const auto features_2 = vk::PhysicalDeviceFeatures2{
                .pNext = &dynamic_rendering_features,
                .features = vk::PhysicalDeviceFeatures{
                    .fillModeNonSolid = VK_TRUE,
                    .samplerAnisotropy = VK_TRUE
                }
            };

            const auto device_create_info = vk::DeviceCreateInfo {
                .pNext = &features_2,
                .queueCreateInfoCount = u32(std::size(queue_create_infos)),
                .pQueueCreateInfos = std::data(queue_create_infos),
    //                .enabledLayerCount = std::size(enabledLayers),
    //                .ppEnabledLayerNames = std::data(enabledLayers),
                .enabledExtensionCount = std::size(device_extensions),
                .ppEnabledExtensionNames = std::data(device_extensions),
    //            .pEnabledFeatures = &features
            };

            logical_device = physical_device.createDevice(device_create_info, nullptr);
            vk::defaultDispatchLoaderDynamic.init(logical_device);

            present_queue = logical_device.getQueue(present_family, 0);
            graphics_queue = logical_device.getQueue(graphics_family, 0);
        }

        void create_memory_allocator() {
            const auto functions = VmaVulkanFunctions{
                .vkGetInstanceProcAddr = vk::defaultDispatchLoaderDynamic.vkGetInstanceProcAddr,
                .vkGetDeviceProcAddr = vk::defaultDispatchLoaderDynamic.vkGetDeviceProcAddr,
                .vkGetPhysicalDeviceProperties = vk::defaultDispatchLoaderDynamic.vkGetPhysicalDeviceProperties,
                .vkGetPhysicalDeviceMemoryProperties = vk::defaultDispatchLoaderDynamic.vkGetPhysicalDeviceMemoryProperties,
                .vkAllocateMemory = vk::defaultDispatchLoaderDynamic.vkAllocateMemory,
                .vkFreeMemory = vk::defaultDispatchLoaderDynamic.vkFreeMemory,
                .vkMapMemory = vk::defaultDispatchLoaderDynamic.vkMapMemory,
                .vkUnmapMemory = vk::defaultDispatchLoaderDynamic.vkUnmapMemory,
                .vkFlushMappedMemoryRanges = vk::defaultDispatchLoaderDynamic.vkFlushMappedMemoryRanges,
                .vkInvalidateMappedMemoryRanges = vk::defaultDispatchLoaderDynamic.vkInvalidateMappedMemoryRanges,
                .vkBindBufferMemory = vk::defaultDispatchLoaderDynamic.vkBindBufferMemory,
                .vkBindImageMemory = vk::defaultDispatchLoaderDynamic.vkBindImageMemory,
                .vkGetBufferMemoryRequirements = vk::defaultDispatchLoaderDynamic.vkGetBufferMemoryRequirements,
                .vkGetImageMemoryRequirements = vk::defaultDispatchLoaderDynamic.vkGetImageMemoryRequirements,
                .vkCreateBuffer = vk::defaultDispatchLoaderDynamic.vkCreateBuffer,
                .vkDestroyBuffer = vk::defaultDispatchLoaderDynamic.vkDestroyBuffer,
                .vkCreateImage = vk::defaultDispatchLoaderDynamic.vkCreateImage,
                .vkDestroyImage = vk::defaultDispatchLoaderDynamic.vkDestroyImage,
                .vkCmdCopyBuffer = vk::defaultDispatchLoaderDynamic.vkCmdCopyBuffer,
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
                /// Fetch "vkGetBufferMemoryRequirements2" on Vulkan >= 1.1, fetch "vkGetBufferMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
                .vkGetBufferMemoryRequirements2KHR = vk::defaultDispatchLoaderDynamic.vkGetBufferMemoryRequirements2KHR,
                /// Fetch "vkGetImageMemoryRequirements 2" on Vulkan >= 1.1, fetch "vkGetImageMemoryRequirements2KHR" when using VK_KHR_dedicated_allocation extension.
                .vkGetImageMemoryRequirements2KHR = vk::defaultDispatchLoaderDynamic.vkGetImageMemoryRequirements2KHR,
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
                /// Fetch "vkBindBufferMemory2" on Vulkan >= 1.1, fetch "vkBindBufferMemory2KHR" when using VK_KHR_bind_memory2 extension.
                .vkBindBufferMemory2KHR = vk::defaultDispatchLoaderDynamic.vkBindBufferMemory2KHR,
                /// Fetch "vkBindImageMemory2" on Vulkan >= 1.1, fetch "vkBindImageMemory2KHR" when using VK_KHR_bind_memory2 extension.
                .vkBindImageMemory2KHR = vk::defaultDispatchLoaderDynamic.vkBindImageMemory2KHR,
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
                .vkGetPhysicalDeviceMemoryProperties2KHR = vk::defaultDispatchLoaderDynamic.vkGetPhysicalDeviceMemoryProperties2KHR,
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

        auto create_shader_module(std::span<const char> bytes) const -> vk::ShaderModule {
            auto create_info = vk::ShaderModuleCreateInfo{
                .codeSize = bytes.size(),
                .pCode    = reinterpret_cast<const u32 *>(bytes.data())
            };
            return logical_device.createShaderModule(create_info);
        }

    public:
        auto makeRenderPass(const RenderPassDescription& description) -> Arc<RenderPass> {
            std::vector<vk::SubpassDescription> subpasses{};
            subpasses.resize(description.definitions.size());

            for (u64 i = 0; i < description.definitions.size(); ++i) {
                subpasses[i].flags = {};
                subpasses[i].pipelineBindPoint = description.definitions[i].pipelineBindPoint;
                if (!description.definitions[i].inputAttachments.empty()) {
                    subpasses[i].setInputAttachments(description.definitions[i].inputAttachments);
                }
                if (!description.definitions[i].colorAttachments.empty()) {
                    subpasses[i].setColorAttachments(description.definitions[i].colorAttachments);
                }
                if (!description.definitions[i].resolveAttachments.empty()) {
                    subpasses[i].setResolveAttachments(description.definitions[i].resolveAttachments);
                }
                if (description.definitions[i].depthStencilAttachment.has_value()) {
                    subpasses[i].setPDepthStencilAttachment(&*description.definitions[i].depthStencilAttachment);
                }
                if (!description.definitions[i].preserveAttachments.empty()) {
                    subpasses[i].setPreserveAttachments(description.definitions[i].preserveAttachments);
                }
            }

            auto create_info = vk::RenderPassCreateInfo{};
            create_info.setSubpasses(subpasses);
            create_info.setAttachments(description.attachments.elements);
            create_info.setDependencies(description.dependencies);

            auto out = Box<RenderPass>::alloc();
            out->handle = logical_device.createRenderPass(create_info);
            return out;
        }

        void freeRenderPass(const Arc<RenderPass>& pass) {
            logical_device.destroyRenderPass(pass->handle);
        }

        auto makeTexture(const TextureDescription& description) -> Arc<Texture> {
            const auto image_create_info = static_cast<VkImageCreateInfo>(vk::ImageCreateInfo{
                .imageType = vk::ImageType::e2D,
                .format = description.format,
                .extent = {
                    .width = description.width,
                    .height = description.height,
                    .depth = 1
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .usage = description.usage
            });

            VkImage image;
            VmaAllocation allocation;

            const auto allocation_create_info = VmaAllocationCreateInfo{.usage = VMA_MEMORY_USAGE_AUTO};
            vmaCreateImage(allocator, &image_create_info, &allocation_create_info, &image, &allocation, nullptr);

            const auto view_create_info = vk::ImageViewCreateInfo{
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = description.format,
                .subresourceRange = vk::ImageSubresourceRange{
                    .aspectMask     = description.aspect,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                }
            };
            auto view = logical_device.createImageView(view_create_info);

            auto out = Box<Texture>::alloc();
            out->context = this;
            out->size.width = description.width;
            out->size.height = description.height;
            out->format = description.format;
            out->image = image;
            out->view = view;
            out->allocation = allocation;
            return out;
        }

        auto freeTexture(const Arc<Texture>& texture) {
            logical_device.destroyImageView(texture->view);
            if (texture->allocation != nullptr) {
                vmaDestroyImage(allocator, texture->image, texture->allocation);
            }
        }

        auto makeBuffer(BufferUsage target, u64 size) -> Arc<Buffer> {
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
            auto out = Box<Buffer>::alloc();
            out->context = this;
            out->handle = buffer;
            out->allocation = allocation;
            out->allocationInfo = allocation_info;
            out->allocationSize = size;
            return out;
        }

        void freeBuffer(const Arc<Buffer>& buffer) {
            vmaDestroyBuffer(allocator, buffer->handle, buffer->allocation);
        }

        auto makeMesh() -> Arc<Mesh> {
            auto out = Box<Mesh>::alloc();
            out->context = this;
            return out;
        }

        void freeMesh(const Arc<Mesh>& mesh) {
            if (mesh->vertexBuffer) {
                freeBuffer(mesh->vertexBuffer);
            }
            if (mesh->indexBuffer) {
                freeBuffer(mesh->indexBuffer);
            }
        }

        auto makePipelineState(const PipelineStateDescription& description) -> Arc<PipelineState> {
            auto out = Box<PipelineState>::alloc();
            out->description = description;

            struct DescriptorSetLayoutDescription {
                std::vector<vk::DescriptorSetLayoutBinding> bindings{};
            };

            std::map<u32, DescriptorSetLayoutDescription> descriptor_set_layouts_table{};

            i32 maxSet = -1;
            std::map<vk::DescriptorType, u32> descriptor_set_bindings_table{};
            std::vector<vk::PushConstantRange> constant_ranges{};

            out->modules.reserve(description.shaders.size());
            for (auto& stage : description.shaders) {
                out->modules.emplace_back(create_shader_module(stage.bytes));

                SpvReflectShaderModule spv_module{};
                spvReflectCreateShaderModule(stage.bytes.size(), stage.bytes.data(), &spv_module);

                auto stage_flags = vk::ShaderStageFlagBits(spv_module.shader_stage);

                auto refl_constant_blocks = std::span(
                    spv_module.push_constant_blocks,
                    spv_module.push_constant_block_count
                );

                constant_ranges.reserve(refl_constant_blocks.size());
                for (auto& refl_block : refl_constant_blocks) {
                    auto& constant_range = constant_ranges.emplace_back();

                    constant_range.setSize(refl_block.size);
                    constant_range.setOffset(refl_block.offset);
                    constant_range.setStageFlags(stage_flags);
                }

                auto refl_descriptor_sets = std::span(
                    spv_module.descriptor_sets,
                    spv_module.descriptor_set_count
                );

                for (auto& refl_set : refl_descriptor_sets) {
                    maxSet = std::max(maxSet, i32(refl_set.set));

                    auto refl_descriptor_bindings = std::span(
                        refl_set.bindings,
                        refl_set.binding_count
                    );

                    for (auto& refl_binding : refl_descriptor_bindings) {
                        auto binding = vk::DescriptorSetLayoutBinding{
                            .binding = refl_binding->binding,
                            .descriptorType = vk::DescriptorType(refl_binding->descriptor_type),
                            .descriptorCount = refl_binding->count,
                            .stageFlags = stage_flags,
                            .pImmutableSamplers = nullptr
                        };
                        descriptor_set_layouts_table[refl_set.set].bindings.emplace_back(binding);
                        descriptor_set_bindings_table[binding.descriptorType] += Context::MAX_FRAMES_IN_FLIGHT;
                    }
                }

                spvReflectDestroyShaderModule(&spv_module);
            }

            /*create descriptor set layouts*/ {
                out->descriptorSetLayouts.resize(maxSet + 1);
                for (u32 i = 0; i < out->descriptorSetLayouts.size(); ++i) {
                    auto dsl_create_info = vk::DescriptorSetLayoutCreateInfo{};
                    dsl_create_info.setBindings(descriptor_set_layouts_table[i].bindings);
                    out->descriptorSetLayouts[i] = logical_device.createDescriptorSetLayout(dsl_create_info);
                }
            }

            /*create pipeline layout*/ {
                auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo{};
                pipeline_layout_create_info.setSetLayouts(out->descriptorSetLayouts);
                pipeline_layout_create_info.setPushConstantRanges(constant_ranges);
                out->pipelineLayout = logical_device.createPipelineLayout(pipeline_layout_create_info);
            }

            return out;
        }

        void freePipelineState(const Arc<PipelineState>& pipelineState) {
            logical_device.destroyPipelineLayout(pipelineState->pipelineLayout);

            for (auto& module : pipelineState->modules) {
                logical_device.destroyShaderModule(module);
            }

            for (auto& layout : pipelineState->descriptorSetLayouts) {
                logical_device.destroyDescriptorSetLayout(layout);
            }
        }

        auto makeCommandQueue(u32 count) -> Arc<CommandQueue> {
            auto out = Box<CommandQueue>::alloc();
            const auto pool_create_info = vk::CommandPoolCreateInfo {
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = graphics_family
            };
            out->context = this;
            out->queue = logical_device.getQueue(graphics_family, 0);
            out->pool = logical_device.createCommandPool(pool_create_info);

            const auto command_buffers_allocate_info = vk::CommandBufferAllocateInfo{
                .commandPool = out->pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = count
            };
            out->handles = logical_device.allocateCommandBuffers(command_buffers_allocate_info);

            out->list.resize(count);
            out->fences.resize(count);
            out->semaphores.resize(count);
            for (size_t i = 0; i < count; ++i) {
                vk::FenceCreateInfo fence_create_info{};
                fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);
                out->fences[i] = logical_device.createFence(fence_create_info);

                vk::SemaphoreCreateInfo semaphore_create_info{};
                out->semaphores[i] = logical_device.createSemaphore(semaphore_create_info);

                out->list[i].owner = out.get();
                out->list[i].fence = out->fences[i];
                out->list[i].handle = out->handles[i];
                out->list[i].semaphore = out->semaphores[i];
            }
            return out;
        }

        void freeCommandQueue(const Arc<CommandQueue>& queue) {
            queue->clearCommandBuffers();

            logical_device.freeCommandBuffers(queue->pool, queue->handles);

            for (auto& semaphore : queue->semaphores) {
                logical_device.destroySemaphore(semaphore);
            }

            for (auto& fence : queue->fences) {
                logical_device.destroyFence(fence);
            }
            logical_device.destroyCommandPool(queue->pool);
        }

    private:
        static constexpr auto get_buffer_usage_from_target(BufferUsage target) -> vk::BufferUsageFlags {
            using Type = std::underlying_type_t<BufferUsage>;

            auto flags = vk::BufferUsageFlags{};
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::Vertex)) {
                flags |= vk::BufferUsageFlagBits::eVertexBuffer;
            }
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::Index)) {
                flags |= vk::BufferUsageFlagBits::eIndexBuffer;
            }
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopySrc)) {
                flags |= vk::BufferUsageFlagBits::eTransferSrc;
            }
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopyDst)) {
                flags |= vk::BufferUsageFlagBits::eTransferDst;
            }
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::Constant)) {
                flags |= vk::BufferUsageFlagBits::eUniformBuffer;
            }
            return flags;
        }

        static constexpr auto get_memory_usage_from_target(BufferUsage target) -> VmaMemoryUsage {
            using Type = std::underlying_type_t<BufferUsage>;

            auto flags = VMA_MEMORY_USAGE_CPU_TO_GPU;
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopySrc)) {
                flags = VMA_MEMORY_USAGE_CPU_ONLY;
            }
            if (static_cast<Type>(target) & static_cast<Type>(BufferUsage::CopyDst)) {
                flags = VMA_MEMORY_USAGE_GPU_ONLY;
            }
            return flags;
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

        static auto find_queue_families(vk::Instance instance, vk::PhysicalDevice device) -> std::optional<std::pair<u32, u32>> {
            const auto properties = device.getQueueFamilyProperties();

            u32 graphics_family = -1;
            u32 present_family = -1;
            for (u32 i = 0; i < u32(properties.size()); i++) {
                if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    graphics_family = i;
                }

                if (glfwGetPhysicalDevicePresentationSupport(instance, device, i)) {
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
}