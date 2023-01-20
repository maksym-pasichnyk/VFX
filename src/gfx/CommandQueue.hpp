#pragma once

#include "Device.hpp"

namespace gfx {
    struct CommandQueueShared {
        Device device;
        vk::CommandPool raw;

        explicit CommandQueueShared(Device device, vk::CommandPool raw);
        ~CommandQueueShared();
    };

    struct CommandQueue final {
        std::shared_ptr<CommandQueueShared> shared;

        explicit CommandQueue();
        explicit CommandQueue(std::shared_ptr<CommandQueueShared> shared);

        auto commandBuffer() -> CommandBuffer;
    };
}