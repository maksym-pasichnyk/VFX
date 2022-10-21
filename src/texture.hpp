#pragma once

#include "types.hpp"

#include <glm/vec4.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct TextureDescription {
        vk::Format format = vk::Format::eUndefined;
        u32 width = 0;
        u32 height = 0;
        vk::ImageUsageFlags usage = {};
        vk::ComponentMapping components = {};
    };

    struct Sampler {
    public:
        Context* context{};
        vk::Sampler handle{};

    public:
        Sampler();
        ~Sampler();

    public:
        void setLabel(const std::string& name);
    };

    struct Texture {
    public:
        Context* context{};
        vk::Extent2D size{};
        vk::Format format{};
        vk::Image image{};
        vk::ImageView view{};
        VmaAllocation allocation{};
        vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eNone;

    public:
        Texture();
        ~Texture();

    public:
        void setPixelData(std::span<const glm::u8vec4> pixels);
        void setLabel(const std::string& name);
    };
}