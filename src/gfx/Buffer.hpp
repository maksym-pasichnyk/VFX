#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Device;
    struct Texture;
    struct CommandBuffer;
    struct RenderCommandEncoder;
    struct ComputeCommandEncoder;
    struct Buffer : ManagedObject<Buffer> {
        friend Device;
        friend Texture;
        friend CommandBuffer;
        friend RenderCommandEncoder;
        friend ComputeCommandEncoder;

    private:
        ManagedShared<Device>   device;
        vk::Buffer              raw;
        VmaAllocation           allocation;

    public:
        explicit Buffer(ManagedShared<Device> device);
        explicit Buffer(ManagedShared<Device> device, vk::Buffer raw, VmaAllocation allocation);
        ~Buffer() override;

    public:
        auto contents() -> void*;
        auto length() -> vk::DeviceSize;
        auto didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void;
        void setLabel(const std::string& name);
        auto descriptorInfo() const -> vk::DescriptorBufferInfo;
    };
}