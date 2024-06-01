#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "ComputePipelineState.hpp"

gfx::CommandQueue::CommandQueue(rc<Device> device, vk::CommandPool handle) : device(std::move(device)), handle(handle) {}

gfx::CommandQueue::~CommandQueue() {
    this->device->handle.destroyCommandPool(handle, nullptr, device->dispatcher);
}

auto gfx::CommandQueue::newCommandBuffer(this CommandQueue& self) -> rc<CommandBuffer> {
    return rc<CommandBuffer>::init(self.device, self.shared_from_this());
}