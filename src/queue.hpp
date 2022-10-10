#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Drawable;
    struct CommandBuffer final {
    public:
        vk::Queue queue{};
        vk::Fence fence{};
        vk::Semaphore semaphore{};
        vk::CommandBuffer handle{};

    public:
        void submit();
        void present(Drawable* drawable);
    };

    struct CommandQueue final {
        friend Context;

    private:
        Context* context{};

        vk::Queue queue{};
        vk::CommandPool pool{};

        std::vector<vk::Fence> fences{};
        std::vector<vk::Semaphore> semaphores{};
        std::vector<vk::CommandBuffer> handles{};

        std::vector<CommandBuffer> list{};

    public:
        auto makeCommandBuffer() -> CommandBuffer*;
    };
}