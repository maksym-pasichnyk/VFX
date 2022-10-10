#pragma once

//#include "context.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Drawable;
    struct CommandBuffer {
    public:
        vk::Queue queue{};
        vk::Fence fence{};
        vk::Semaphore semaphore{};
        vk::CommandBuffer handle{};

    public:
        void submit();
        void present(Drawable* drawable);
    };

    struct CommandQueue {
    public:
        Context* context{};

        vk::Queue queue{};
        vk::CommandPool pool{};
        std::vector<CommandBuffer> command_buffers{};

    public:
        auto makeCommandBuffer() -> CommandBuffer*;
    };
}