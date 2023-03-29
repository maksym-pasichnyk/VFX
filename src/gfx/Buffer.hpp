#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Device;
    struct Buffer : ManagedObject<Buffer> {
        ManagedShared<Device>   device;
        vk::Buffer              raw;
        VmaAllocation           allocation;

        explicit Buffer(ManagedShared<Device> device);
        explicit Buffer(ManagedShared<Device> device, vk::Buffer raw, VmaAllocation allocation);
        ~Buffer() override;

        auto contents() -> void*;
        auto length() -> vk::DeviceSize;
        auto didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void;
        void setLabel(const std::string& name);
    };
}