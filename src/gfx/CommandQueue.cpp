#include "Swapchain.hpp"
#include "Buffer.hpp"
#include "Device.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

gfx::CommandQueue::CommandQueue(SharedPtr<Device> device) : mDevice(std::move(device)) {
    vkComputeQueue = mDevice->vkDevice.getQueue(mDevice->vkComputeQueueFamilyIndex, 0, mDevice->vkDispatchLoaderDynamic);
    vkPresentQueue = mDevice->vkDevice.getQueue(mDevice->vkPresentQueueFamilyIndex, 0, mDevice->vkDispatchLoaderDynamic);
    vkGraphicsQueue = mDevice->vkDevice.getQueue(mDevice->vkGraphicsQueueFamilyIndex, 0, mDevice->vkDispatchLoaderDynamic);

    vk::CommandPoolCreateInfo pool_create_info = {};
    pool_create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    pool_create_info.setQueueFamilyIndex(mDevice->vkGraphicsQueueFamilyIndex);

    vkCommandPool = mDevice->vkDevice.createCommandPool(pool_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);
}

gfx::CommandQueue::~CommandQueue() {
    mDevice->vkDevice.destroyCommandPool(vkCommandPool, nullptr, mDevice->vkDispatchLoaderDynamic);
}

auto gfx::CommandQueue::commandBuffer() -> SharedPtr<gfx::CommandBuffer> {
    return TransferPtr(new CommandBuffer(mDevice, this, true));
}

auto gfx::CommandQueue::commandBufferWithUnretainedReferences() -> SharedPtr<gfx::CommandBuffer> {
    return TransferPtr(new CommandBuffer(mDevice, this, false));
}