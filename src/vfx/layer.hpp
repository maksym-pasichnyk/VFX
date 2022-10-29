#pragma once

#include "types.hpp"

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Layer;
    struct Device;
    struct Texture;
    struct Drawable {
        u32 index{};
        Layer* layer{};
        Arc<Texture> texture{};
    };

    struct Layer {
    public:
        Arc<Device> device = {};
        vk::UniqueFence fence = {};
        vk::UniqueSurfaceKHR surface = {};
        vk::UniqueSwapchainKHR swapchain = {};
        std::vector<Arc<Drawable>> drawables = {};

        vk::Format pixelFormat = {};
        vk::Extent2D drawableSize = {};
        vk::ColorSpaceKHR colorSpace = {};
        bool displaySyncEnabled = false;

    public:
        Layer(const Arc<Device>& device);

    public:
        void updateDrawables();
        auto nextDrawable() -> Drawable*;

    private:
        void freeDrawables();
    };
}