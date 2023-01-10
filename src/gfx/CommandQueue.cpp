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
    mComputeQueue = mDevice->mDevice.getQueue(mDevice->mComputeQueueFamilyIndex, 0, mDevice->mDispatchLoaderDynamic);
    mPresentQueue = mDevice->mDevice.getQueue(mDevice->mPresentQueueFamilyIndex, 0, mDevice->mDispatchLoaderDynamic);
    mGraphicsQueue = mDevice->mDevice.getQueue(mDevice->mGraphicsQueueFamilyIndex, 0, mDevice->mDispatchLoaderDynamic);

    vk::CommandPoolCreateInfo pool_create_info = {};
    pool_create_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    pool_create_info.setQueueFamilyIndex(mDevice->mGraphicsQueueFamilyIndex);

    mCommandPool = mDevice->mDevice.createCommandPool(pool_create_info, nullptr, mDevice->mDispatchLoaderDynamic);
}

gfx::CommandQueue::~CommandQueue() {
    mDevice->mDevice.destroyCommandPool(mCommandPool, nullptr, mDevice->mDispatchLoaderDynamic);
}

auto gfx::CommandQueue::commandBuffer() -> SharedPtr<gfx::CommandBuffer> {
    return TransferPtr(new CommandBuffer(mDevice, this, true));
}

auto gfx::CommandQueue::commandBufferWithUnretainedReferences() -> SharedPtr<gfx::CommandBuffer> {
    return TransferPtr(new CommandBuffer(mDevice, this, false));
}