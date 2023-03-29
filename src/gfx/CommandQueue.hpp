#pragma once

#include "Device.hpp"
#include "ManagedObject.hpp"

namespace gfx {
    struct CommandBuffer;
    struct CommandQueue : ManagedObject<CommandQueue> {
        ManagedShared<Device>   device;
        vk::CommandPool         raw;

        explicit CommandQueue(ManagedShared<Device> device, vk::CommandPool raw);
        ~CommandQueue() override;

        auto commandBuffer() -> ManagedShared<CommandBuffer>;
    };
}