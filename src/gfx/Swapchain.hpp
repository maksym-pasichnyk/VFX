#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Swapchain;
    struct Device;
    struct Texture;
    struct Drawable;
    struct CommandBuffer;

    struct Swapchain final : Referencing<Swapchain> {
        friend CommandBuffer;

    private:
        SharedPtr<Device> mDevice;
        vk::SurfaceKHR vkSurface = {};
        vk::SwapchainKHR vkSwapchain = {};
        std::vector<SharedPtr<Drawable>> mDrawables = {};

        vk::Format mPixelFormat = {};
        vk::Extent2D mDrawableSize = {};
        vk::ColorSpaceKHR mColorSpace = {};
        bool mDisplaySyncEnabled = {};

    private:
        explicit Swapchain(SharedPtr<Device> device);
        ~Swapchain() override;

    private:
        void createDrawables();

    public:
        void releaseDrawables();
        auto nextDrawable() -> SharedPtr<Drawable>;
        auto drawableSize() -> vk::Extent2D;
        auto pixelFormat() -> vk::Format;
        void setPixelFormat(vk::Format format);
        auto colorSpace() -> vk::ColorSpaceKHR;
        void setColorSpace(vk::ColorSpaceKHR colorSpace);
        auto surface() -> vk::SurfaceKHR;
        void setSurface(vk::SurfaceKHR surface);
        auto displaySyncEnabled() -> bool;
        void setDisplaySyncEnabled(bool displaySyncEnabled);

    public:
        static auto alloc(SharedPtr<Device> device) -> SharedPtr<Swapchain>;
    };
}