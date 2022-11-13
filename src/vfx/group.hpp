#pragma once

#include "types.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Buffer;
    struct Device;
    struct Texture;
    struct Sampler;
    struct ResourceGroup {
    public:
        Device* device{};
        vk::DescriptorSet set{};
        vk::DescriptorPool pool{};

    public:
        ResourceGroup();
        ~ResourceGroup();

    public:
        void setBuffer(const Arc<Buffer>& buffer, u64 offset, u32 binding);
        void setStorageBuffer(const Arc<Buffer>& buffer, u64 offset, u32 binding);
        void setTexture(const Arc<Texture>& texture, u32 binding);
        void setSampler(const Arc<Sampler>& sampler, u32 binding);
        void setStorageImage(const Arc<Texture>& texture, u32 binding);
        void setLabel(const std::string& name);
    };
}