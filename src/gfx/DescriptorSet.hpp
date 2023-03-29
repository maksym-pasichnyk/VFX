#pragma once

#include "Instance.hpp"

#include <vector>

namespace gfx {
    struct Buffer;
    struct Device;
    struct Texture;
    struct Sampler;

    struct DescriptorSet : ManagedObject<DescriptorSet> {
        ManagedShared<Device>   device;
        vk::DescriptorSet       raw;
        vk::DescriptorPool      pool;

        explicit DescriptorSet(ManagedShared<Device> device, vk::DescriptorSet raw, vk::DescriptorPool pool);
        ~DescriptorSet() override;

        void setBuffer(const Buffer& buffer, uint64_t offset, uint32_t binding);
        void setStorageBuffer(const Buffer& buffer, uint64_t offset, uint32_t binding);
        void setTexture(const Texture& texture, uint32_t binding);
        void setSampler(Sampler const& sampler, uint32_t binding);
        void setStorageImage(const Texture& texture, uint32_t binding);
        void setLabel(const std::string& name);
    };
}