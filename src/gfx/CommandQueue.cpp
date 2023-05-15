#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"

gfx::CommandQueue::CommandQueue(ManagedShared<Device> device, vk::CommandPool raw) : device(std::move(device)), raw(raw) {}
gfx::CommandQueue::~CommandQueue() {
    device->raii.raw.destroyCommandPool(raw, nullptr, device->raii.dispatcher);
}

auto gfx::CommandQueue::commandBuffer() -> ManagedShared<CommandBuffer> {
    return MakeShared<CommandBuffer>(device, shared_from_this());
}