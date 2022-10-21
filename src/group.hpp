#pragma once

#include "types.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Buffer;
    struct Context;
    struct Texture;
    struct Sampler;
    struct ResourceGroup {
    public:
        Context* context;
        vk::DescriptorPool pool{};
        vk::DescriptorSet set{};

    public:
        ResourceGroup();
        ~ResourceGroup();

    public:
        void setBuffer(const Arc<Buffer>& buffer, u64 offset, u32 binding);
        void setTexture(const Arc<Texture>& texture, u32 binding);
        void setSampler(const Arc<Sampler>& sampler, u32 binding);
    };
}