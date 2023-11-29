#pragma once

#include "Instance.hpp"
#include "vk_mem_alloc.h"

namespace gfx {
    struct Buffer : public ManagedObject {
        rc<Device>    device;
        vk::Buffer    handle;
        VmaAllocation allocation;

        explicit Buffer(rc<Device> device, vk::Buffer handle, VmaAllocation allocation);
        ~Buffer() override;

        auto contents() -> void*;
        auto length() -> vk::DeviceSize;
        auto didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void;
        void setLabel(std::string const& name);
        auto descriptorInfo() const -> vk::DescriptorBufferInfo;
    };
}