#pragma once

#include <list>
#include <pass.hpp>
#include <types.hpp>
#include <buffer.hpp>
#include <display.hpp>
#include <texture.hpp>
#include <material.hpp>
#include <glm/vec4.hpp>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

#include "spirv_reflect.h"

struct DefaultVertexFormat {
    glm::vec3 position;
    glm::u8vec4 color;
};

struct Geometry {
    Box<vfx::Buffer> vtx = {};
    Box<vfx::Buffer> idx = {};

    u64 vtx_buf_size = 0;
    u64 idx_buf_size = 0;

    i32 vtx_count = 0;
    i32 idx_count = 0;

    u64 vtx_stride = 0;
    u64 idx_stride = 0;
};

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
        auto makeRenderPass(const RenderPassDescription& description) -> Box<RenderPass> {
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

        void freeRenderPass(const Box<RenderPass>& pass) {
            logical_device.destroyRenderPass(pass->handle);
        }

        auto makeTexture(const TextureDescription& description) -> Box<Texture> {
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
            out->size.width = description.width;
            out->size.height = description.height;
            out->format = description.format;
            out->image = image;
            out->view = view;
            out->allocation = allocation;
            return out;
        }

        auto freeTexture(const Box<Texture>& texture) {
            logical_device.destroyImageView(texture->view);
            if (texture->allocation != nullptr) {
                vmaDestroyImage(allocator, texture->image, texture->allocation);
            }
        }

        void set_texture_data(Texture* texture, std::span<const glm::u8vec4> pixels) {
            auto tmp = create_buffer(BufferUsage::CopySrc, static_cast<int>(pixels.size_bytes()));
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
                    .width = texture->size.width,
                    .height = texture->size.height,
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
            std::ignore = graphics_queue.submit(1, &submit_info, fence);

            std::ignore = logical_device.waitForFences(fence, false, std::numeric_limits<uint64_t>::max());
            logical_device.freeCommandBuffers(cmd_pool, cmd);
            logical_device.destroyCommandPool(cmd_pool);
            logical_device.destroyFence(fence);
            freeBuffer(tmp);
        }

        auto create_buffer(BufferUsage target, u64 size) -> Box<Buffer> {
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
            out->buffer = buffer;
            out->allocation = allocation;
            out->allocation_info = allocation_info;
            return out;
        }

        void freeBuffer(const Box<Buffer>& gb) {
            vmaDestroyBuffer(allocator, gb->buffer, gb->allocation);
        }

        void update_buffer(const Box<Buffer>& gb, const void* src, u64 size, u64 offset) {
            void* ptr = nullptr;
            vmaMapMemory(allocator, gb->allocation, &ptr);
            auto dst = static_cast<std::byte*>(ptr) + offset;
            memcpy(dst, src, size);
            vmaUnmapMemory(allocator, gb->allocation);
        }

        void set_vertices(Geometry* geometry, std::span<const DefaultVertexFormat> vertices) {
            set_vertex_buffer_params(geometry, i32(vertices.size()), sizeof(DefaultVertexFormat));
            set_vertex_buffer_data(geometry, vertices.data(), vertices.size(), 0);
        }

        void set_indices(Geometry* geometry, std::span<const u32> indices) {
            set_index_buffer_params(geometry, i32(indices.size()), sizeof(u32));
            set_index_buffer_data(geometry, indices.data(), indices.size(), 0);
        }

        void set_vertex_buffer_params(Geometry* geometry, i32 count, u64 stride) {
            const auto buf_size = static_cast<i32>(count * stride);

            geometry->vtx_count = count;
            geometry->vtx_stride = stride;
            if (buf_size > geometry->vtx_buf_size) {
                geometry->vtx_buf_size = buf_size;
                if (geometry->vtx != nullptr) {
                    freeBuffer(geometry->vtx);
                }
                geometry->vtx = create_buffer(BufferUsage::Vertex, buf_size);
            }
        }

        void set_index_buffer_params(Geometry* geometry, i32 count, u64 stride) {
            const auto buf_size = static_cast<i32>(count * stride);

            geometry->idx_count = count;
            geometry->idx_stride = stride;
            if (buf_size > geometry->idx_buf_size) {
                geometry->idx_buf_size = buf_size;
                if (geometry->idx != nullptr) {
                    freeBuffer(geometry->idx);
                }
                geometry->idx = create_buffer(BufferUsage::Index, buf_size);
            }
        }

        void set_vertex_buffer_data(Geometry* geometry, const void* src, u64 size, u64 offset) {
            update_buffer(geometry->vtx, src, size * geometry->vtx_stride, offset * geometry->vtx_stride);
        }

        void set_index_buffer_data(Geometry* geometry, const void* src, u64 size, u64 offset) {
            update_buffer(geometry->idx, src, size * geometry->idx_stride, offset * geometry->idx_stride);
        }

        auto makeMaterial(const MaterialDescription& description, const Box<RenderPass>& pass, u32 subpass) -> Box<Material> {
            auto out = Box<Material>::alloc();
            out->pipeline_bind_point = vk::PipelineBindPoint::eGraphics;

            struct DescriptorSetLayoutDescription {
                std::vector<vk::DescriptorSetLayoutBinding> bindings{};
            };

            std::map<u32, DescriptorSetLayoutDescription> descriptor_set_layouts_table{};

            i32 maxSet = -1;
            std::map<vk::DescriptorType, u32> descriptor_set_bindings_table{};
            std::vector<vk::PushConstantRange> constant_ranges{};
            std::vector<vk::PipelineShaderStageCreateInfo> stages{};

            stages.reserve(description.shaders.size());
            for (auto& stage : description.shaders) {
                stages.emplace_back(vk::PipelineShaderStageCreateInfo{
                    .stage  = stage.stage,
                    .module = create_shader_module(stage.bytes),
                    .pName = stage.entry.c_str()
                });

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
                      vk::DescriptorSetLayoutBinding binding{
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
                out->descriptor_set_layouts.resize(maxSet + 1);
                for (u32 i = 0; i < out->descriptor_set_layouts.size(); ++i) {
                    auto dsl_create_info = vk::DescriptorSetLayoutCreateInfo{};
                    dsl_create_info.setBindings(descriptor_set_layouts_table[i].bindings);
                    out->descriptor_set_layouts[i] = logical_device.createDescriptorSetLayout(dsl_create_info);
                }
            }

            /*create pipeline layout*/ {
                auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo{};
                pipeline_layout_create_info.setSetLayouts(out->descriptor_set_layouts);
                pipeline_layout_create_info.setPushConstantRanges(constant_ranges);
                out->pipeline_layout = logical_device.createPipelineLayout(pipeline_layout_create_info);
            }

            /*allocate descriptors*/
            if (!out->descriptor_set_layouts.empty()) {
                std::vector<vk::DescriptorPoolSize> pool_sizes{};
                pool_sizes.reserve(descriptor_set_bindings_table.size());
                for (auto& [type, count] : descriptor_set_bindings_table) {
                    pool_sizes.emplace_back(vk::DescriptorPoolSize{type, count});
                }

                auto pool_create_info = vk::DescriptorPoolCreateInfo{};
                pool_create_info.setMaxSets(out->descriptor_set_layouts.size());
                pool_create_info.setPoolSizes(pool_sizes);
                out->descriptor_pool = logical_device.createDescriptorPool(pool_create_info, nullptr);

                auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
                ds_allocate_info.setDescriptorPool(out->descriptor_pool);
                ds_allocate_info.setSetLayouts(out->descriptor_set_layouts);
                out->descriptor_sets = logical_device.allocateDescriptorSets(ds_allocate_info);
            }

            auto vertex_input_state = vk::PipelineVertexInputStateCreateInfo{};
            vertex_input_state.setVertexBindingDescriptions(description.bindings);
            vertex_input_state.setVertexAttributeDescriptions(description.attributes);

            auto color_blend_state = vk::PipelineColorBlendStateCreateInfo{};
            color_blend_state.setAttachments(description.attachments.elements);

            std::array dynamic_states = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };

            vk::PipelineViewportStateCreateInfo viewport_state = {};
            viewport_state.viewportCount = 1;
            viewport_state.pViewports = nullptr;
            viewport_state.scissorCount = 1;
            viewport_state.pScissors = nullptr;

            auto dynamic_state = vk::PipelineDynamicStateCreateInfo{};
            dynamic_state.setDynamicStates(dynamic_states);

            auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{};
            pipeline_create_info.pVertexInputState = &vertex_input_state;
            pipeline_create_info.pInputAssemblyState = &description.inputAssemblyState;
            pipeline_create_info.pViewportState = &viewport_state;
            pipeline_create_info.pRasterizationState = &description.rasterizationState;
            pipeline_create_info.pMultisampleState = &description.multisampleState;
            pipeline_create_info.pDepthStencilState = &description.depthStencilState;
            pipeline_create_info.pColorBlendState = &color_blend_state;
            pipeline_create_info.pDynamicState = &dynamic_state;
            pipeline_create_info.layout = out->pipeline_layout;
            pipeline_create_info.renderPass = pass ? pass->handle : nullptr;
            pipeline_create_info.subpass = subpass;
            pipeline_create_info.basePipelineHandle = nullptr;
            pipeline_create_info.basePipelineIndex = 0;

            pipeline_create_info.setStages(stages);

            auto result = logical_device.createGraphicsPipelines(
                {},
                1,
                &pipeline_create_info,
                nullptr,
                &out->pipeline
            );

            if (result != vk::Result::eSuccess) {
                spdlog::error("{}", vk::to_string(result));
            }

            for (auto& stage : stages) {
                logical_device.destroyShaderModule(stage.module);
            }

            return out;
        }

        auto freeMaterial(const Box<Material>& material) {
            logical_device.destroyPipeline(material->pipeline);
            logical_device.destroyPipelineLayout(material->pipeline_layout);

            for (auto& layout : material->descriptor_set_layouts) {
                logical_device.destroyDescriptorSetLayout(layout);
            }

            if (material->descriptor_pool) {
                logical_device.destroyDescriptorPool(material->descriptor_pool);
            }
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