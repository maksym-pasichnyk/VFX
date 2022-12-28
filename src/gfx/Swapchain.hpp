#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Window;
    struct Texture;
    struct Drawable;
    struct CommandBuffer;

    struct Swapchain final : Referencing<Swapchain> {
        friend Window;
        friend CommandBuffer;

    private:
        SharedPtr<Device> mDevice;
        std::vector<SharedPtr<Drawable>> mDrawables = {};

        vk::SurfaceKHR vkSurface = {};
        vk::SwapchainKHR vkSwapchain = {};

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
        auto device() -> SharedPtr<Device>;
        auto nextDrawable() -> SharedPtr<Drawable>;
        auto drawableSize() -> vk::Extent2D;
        void setDrawableSize(const vk::Extent2D& drawableSize);
        auto pixelFormat() -> vk::Format;
        void setPixelFormat(vk::Format format);
        auto colorSpace() -> vk::ColorSpaceKHR;
        void setColorSpace(vk::ColorSpaceKHR colorSpace);
        auto displaySyncEnabled() -> bool;
        void setDisplaySyncEnabled(bool displaySyncEnabled);

    public:
        static auto alloc(SharedPtr<Device> device) -> SharedPtr<Swapchain>;
    };
}