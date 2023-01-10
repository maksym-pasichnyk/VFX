#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Context;
    struct Texture;
    struct Drawable;
    struct Swapchain;
    struct CommandBuffer;

    struct Surface final : Referencing {
        friend Swapchain;

    private:

    public:
//    private:
        Surface(Context* context, vk::SurfaceKHR surface);
        ~Surface() override;
    };

    struct Swapchain final : Referencing {
        friend CommandBuffer;

    private:
        vk::SurfaceKHR mSurface = {};
        SharedPtr<Device> mDevice = {};
        SharedPtr<Context> mContext = {};
        std::vector<SharedPtr<Drawable>> mDrawables = {};

        vk::Format mPixelFormat = {};
        vk::Extent2D mDrawableSize = {};
        vk::SwapchainKHR mSwapchain = {};
        vk::ColorSpaceKHR mColorSpace = {};

        bool mDisplaySyncEnabled = {};
        uint32_t mMaximumDrawableCount = 3;

    public:
        explicit Swapchain(SharedPtr<Context> context, vk::SurfaceKHR surface);
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