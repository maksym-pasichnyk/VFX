#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"
#include "Swapchain.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

gfx::CommandQueueShared::CommandQueueShared(Device device, vk::CommandPool raw) : device(std::move(device)), raw(raw) {}
gfx::CommandQueueShared::~CommandQueueShared() {
    device.shared->raii.raw.destroyCommandPool(raw, nullptr, device.shared->raii.dispatcher);
}

gfx::CommandQueue::CommandQueue() : shared(nullptr) {}
gfx::CommandQueue::CommandQueue(std::shared_ptr<CommandQueueShared> shared) : shared(std::move(shared)) {}

auto gfx::CommandQueue::commandBuffer() -> gfx::CommandBuffer {
    return CommandBuffer(std::make_shared<CommandBufferShared>(shared->device, *this));
}