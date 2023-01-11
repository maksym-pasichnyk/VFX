#pragma once

#include "Object.hpp"
#include "Device.hpp"

#include <map>
#include <set>
#include <tuple>
#include <vector>
#include <concepts>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Buffer;
    struct Device;
    struct Drawable;
    struct CommandQueue;
    struct DescriptorSet;
    struct RenderPipelineState;
    struct ComputePipelineState;

    struct CommandQueue final : Referencing {
        friend Device;
        friend CommandBuffer;
        friend RenderCommandEncoder;

    private:
        explicit CommandQueue(SharedPtr<Device> device);
        ~CommandQueue() override;

    public:
        auto commandBuffer() -> SharedPtr<CommandBuffer>;

    private:
        SharedPtr<Device> mDevice;
        vk::CommandPool mCommandPool = {};
    };
}