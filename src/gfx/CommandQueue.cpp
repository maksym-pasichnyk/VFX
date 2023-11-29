#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "ComputePipelineState.hpp"

gfx::CommandQueue::CommandQueue(rc<Device> device, vk::CommandPoolCreateInfo const& create_info) : device(std::move(device)), handle() {
    this->handle = this->device->handle.createCommandPool(create_info, nullptr, this->device->dispatcher);
}

gfx::CommandQueue::~CommandQueue() {
    this->device->handle.destroyCommandPool(handle, nullptr, device->dispatcher);
}

auto gfx::CommandQueue::newCommandBuffer(this CommandQueue& self) -> rc<CommandBuffer> {
    return MakeShared<CommandBuffer>(self.device, self.shared_from_this());
}