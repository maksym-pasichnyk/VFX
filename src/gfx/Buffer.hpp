#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include "Object.hpp"

namespace gfx {
    struct Device;
    struct Texture;
    struct CommandBuffer;
    struct DescriptorSet;
    struct RenderCommandEncoder;
    struct Buffer final : Referencing<Buffer> {
        friend Device;
        friend Texture;
        friend CommandBuffer;
        friend DescriptorSet;
        friend RenderCommandEncoder;

    private:
        SharedPtr<Device> mDevice;

        vk::Buffer vkBuffer;
        VmaAllocation vmaAllocation;

    private:
        explicit Buffer(SharedPtr<Device> device, const vk::BufferCreateInfo& buffer_create_info, const VmaAllocationCreateInfo& allocation_create_info);
        ~Buffer() override;

    public:
        auto contents() -> void*;
        auto length() -> vk::DeviceSize;
        auto didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void;

        void setLabel(const std::string& name);
    };
}