#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Window;
    struct Texture;
    struct Drawable;
    struct Swapchain;
    struct Application;
    struct CommandBuffer;

    struct Surface final : Referencing<Surface> {
        friend Window;
        friend Swapchain;

    private:
        SharedPtr<gfx::Application> mApplication;
        vk::SurfaceKHR vkSurface;

    private:
        Surface(SharedPtr<Application> application, vk::SurfaceKHR surface);
        ~Surface() override;
    };

    struct Swapchain final : Referencing<Swapchain> {
        friend Window;
        friend CommandBuffer;

    private:
        SharedPtr<Device> mDevice;
        SharedPtr<Surface> mSurface = {};
        std::vector<SharedPtr<Drawable>> mDrawables = {};

        vk::Format mPixelFormat = {};
        vk::Extent2D mDrawableSize = {};
        vk::SwapchainKHR vkSwapchain = {};
        vk::ColorSpaceKHR mColorSpace = {};

        bool mDisplaySyncEnabled = {};
        uint32_t mMaximumDrawableCount = 3;

    private:
        explicit Swapchain(SharedPtr<Surface> surface);
        ~Swapchain() override;

    private:
        void createDrawables();

    public:
        void releaseDrawables();
        void setDevice(SharedPtr<Device> device);
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
        auto maximumDrawableCount() -> uint32_t;
        void setMaximumDrawableCount(uint32_t maximumDrawableCount);
    };
}