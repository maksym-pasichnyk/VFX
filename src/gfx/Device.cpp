#include "Device.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "Library.hpp"
#include "Context.hpp"
#include "CommandQueue.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

#include <set>

gfx::Device::Device(Context* context, vk::PhysicalDevice gpu) : pContext(context), vkPhysicalDevice(gpu) {
    std::vector<std::vector<float_t>> queue_priorities = {};
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos = {};

    auto queue_family_properties = gpu.getQueueFamilyProperties(context->vkDispatchLoaderDynamic);
    queue_priorities.resize(queue_family_properties.size());

    for (size_t i = 0; i < queue_family_properties.size(); i++) {
        queue_priorities[i].resize(queue_family_properties[i].queueCount, 1.0F);

        vk::DeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.setQueueFamilyIndex(i);
        queue_create_info.setQueuePriorities(queue_priorities[i]);
        queue_create_infos.emplace_back(queue_create_info);
    }

    std::vector<const char*> layers = {};

    auto available_layers = gpu.enumerateDeviceLayerProperties(context->vkDispatchLoaderDynamic);
    for (auto& layer : available_layers) {
        layers.emplace_back(layer.layerName);
    }

    std::vector<const char*> extensions = {};

    auto available_extensions = gpu.enumerateDeviceExtensionProperties(nullptr, context->vkDispatchLoaderDynamic);
    for (auto& extension : available_extensions) {
        if (std::strcmp(extension.extensionName, "VK_AMD_negative_viewport_height") == 0) {
            continue;
        }

        extensions.emplace_back(extension.extensionName);
    }

    vk::PhysicalDeviceSynchronization2Features synchronization_2_features = {};
    synchronization_2_features.setSynchronization2(VK_TRUE);

    vk::PhysicalDevicePortabilitySubsetFeaturesKHR portability_subset_features = {};
    portability_subset_features.setPNext(&synchronization_2_features);
    portability_subset_features.setImageViewFormatSwizzle(VK_TRUE);

    vk::PhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = {};
    timeline_semaphore_features.setPNext(&portability_subset_features);
    timeline_semaphore_features.setTimelineSemaphore(VK_TRUE);

    vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {};
    dynamic_rendering_features.setPNext(&timeline_semaphore_features);
    dynamic_rendering_features.setDynamicRendering(VK_TRUE);

    vk::PhysicalDeviceFeatures2 features_2 = gpu.getFeatures2(context->vkDispatchLoaderDynamic);
    features_2.setPNext(&dynamic_rendering_features);

    vk::DeviceCreateInfo device_create_info = {};
    device_create_info.setPNext(&features_2);
    device_create_info.setQueueCreateInfos(queue_create_infos);
    device_create_info.setPEnabledLayerNames(layers);
    device_create_info.setPEnabledExtensionNames(extensions);

    vkDevice = gpu.createDevice(device_create_info, nullptr, context->vkDispatchLoaderDynamic);
    vkDispatchLoaderDynamic.init(context->vkDispatchLoaderDynamic.vkGetInstanceProcAddr);
    vkDispatchLoaderDynamic.init(context->vkInstance);
    vkDispatchLoaderDynamic.init(vkDevice);

    VmaVulkanFunctions functions = {};
    functions.vkGetDeviceProcAddr = vkDispatchLoaderDynamic.vkGetDeviceProcAddr;
    functions.vkGetInstanceProcAddr = vkDispatchLoaderDynamic.vkGetInstanceProcAddr;

    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.physicalDevice = gpu;
    allocator_create_info.device = vkDevice;
    allocator_create_info.pVulkanFunctions = &functions;
    allocator_create_info.instance = context->vkInstance;
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_2;

    vmaCreateAllocator(&allocator_create_info, &vmaAllocator);

    // find a graphics queue family index
    vkGraphicsQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
        if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            vkGraphicsQueueFamilyIndex = i;
            break;
        }
    }
    if (vkGraphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        spdlog::error("No graphics queue family index found");
        exit(1);
    }

    // find a present queue family index
#ifdef __APPLE__
    vkPresentQueueFamilyIndex = vkGraphicsQueueFamilyIndex;
#else
    vkPresentQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
        assert(0);
    }
    if (vkPresentQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        spdlog::error("No present queue family index found");
        exit(1);
    }
#endif

    // find a compute queue family index
    vkComputeQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < queue_family_properties.size(); i++) {
        if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eCompute) {
            vkComputeQueueFamilyIndex = i;
            break;
        }
    }
    if (vkComputeQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        spdlog::error("No compute queue family index found");
        exit(1);
    }
}

gfx::Device::~Device() {
    vmaDestroyAllocator(vmaAllocator);
    vkDispatchLoaderDynamic.vkDestroyDevice(vkDevice, nullptr);
}

void gfx::Device::waitIdle() {
    vkDevice.waitIdle(vkDispatchLoaderDynamic);
}

auto gfx::Device::newTexture(const TextureDescription& description) -> SharedPtr<Texture> {
    return TransferPtr(new Texture(RetainPtr(this), description));
}

auto gfx::Device::newSampler(const vk::SamplerCreateInfo& info) -> SharedPtr<Sampler> {
    return TransferPtr(new Sampler(RetainPtr(this), info));
}

auto gfx::Device::newBuffer(vk::BufferUsageFlags usage, uint64_t size, VmaAllocationCreateFlags options) -> SharedPtr<Buffer> {
    vk::BufferCreateInfo buffer_create_info = {};
    buffer_create_info.setSize(static_cast<vk::DeviceSize>(size));
    buffer_create_info.setUsage(usage);

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.flags = options;
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

    return TransferPtr(new Buffer(RetainPtr(this), buffer_create_info, allocation_create_info));
}

auto gfx::Device::newBuffer(vk::BufferUsageFlags usage, const void* pointer, uint64_t size, VmaAllocationCreateFlags options) -> SharedPtr<Buffer> {
    // todo: use transfer operation if buffer is not mappable
    auto id = newBuffer(usage, size, options);
    std::memcpy(id->contents(), pointer, size);
    id->didModifyRange(0, size);
    return id;
}

auto gfx::Device::newLibrary(const std::vector<char>& bytes) -> SharedPtr<Library> {
    vk::ShaderModuleCreateInfo module_create_info = {};
    module_create_info.setCodeSize(bytes.size());
    module_create_info.setPCode(reinterpret_cast<const uint32_t *>(bytes.data()));

    return TransferPtr(new Library(RetainPtr(this), module_create_info));
}

auto gfx::Device::newRenderPipelineState(const gfx::RenderPipelineStateDescription& description) -> SharedPtr<RenderPipelineState> {
    return TransferPtr(new RenderPipelineState(RetainPtr(this), description));
}

auto gfx::Device::newComputePipelineState(const SharedPtr<Function>& function) -> SharedPtr<ComputePipelineState> {
    return TransferPtr(new ComputePipelineState(RetainPtr(this), function));
}

auto gfx::Device::newCommandQueue() -> SharedPtr<CommandQueue> {
    return TransferPtr(new CommandQueue(RetainPtr(this)));
}

// todo: get sizes from layout
auto gfx::Device::newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> SharedPtr<DescriptorSet> {
    auto id = new DescriptorSet(RetainPtr(this));

    vk::DescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.setMaxSets(1);
    pool_create_info.setPoolSizes(sizes);
    id->vkDescriptorPool = vkDevice.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, vkDispatchLoaderDynamic);

    vk::DescriptorSetAllocateInfo ds_allocate_info = {};
    ds_allocate_info.setDescriptorPool(id->vkDescriptorPool);
    ds_allocate_info.setDescriptorSetCount(1);
    ds_allocate_info.setPSetLayouts(&layout);
    id->vkDescriptorSet = vkDevice.allocateDescriptorSets(ds_allocate_info, vkDispatchLoaderDynamic)[0];

    return TransferPtr(id);
}