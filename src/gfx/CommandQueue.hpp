#pragma once

#include "Device.hpp"
#include "ManagedObject.hpp"

namespace gfx {
    struct CommandBuffer;
    struct CommandQueue : public ManagedObject {
        rc<Device>      device;
        vk::CommandPool handle;

        explicit CommandQueue(rc<Device> device, vk::CommandPool handle);
        ~CommandQueue() override;

        auto newCommandBuffer(this CommandQueue& self) -> rc<CommandBuffer>;
    };
}