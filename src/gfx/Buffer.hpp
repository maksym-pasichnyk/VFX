#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Device;
    struct Texture;
    struct CommandBuffer;
    struct DescriptorSet;

    struct BufferShared {
        Device device;
        vk::Buffer raw;
        VmaAllocation allocation;

        explicit BufferShared(Device device);
        explicit BufferShared(Device device, vk::Buffer raw, VmaAllocation allocation);
        ~BufferShared();
    };

    struct Buffer final {
        std::shared_ptr<BufferShared> shared;

        explicit Buffer();
        explicit Buffer(std::shared_ptr<BufferShared> shared);

        auto contents() -> void*;
        auto length() -> vk::DeviceSize;
        auto didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void;
        void setLabel(const std::string& name);
    };
}