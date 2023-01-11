#include "Swapchain.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"

#include "spdlog/spdlog.h"

#include <set>
#include <SDL_vulkan.h>

class SDL_Window;

gfx::Swapchain::Swapchain(SharedPtr<Context> context, SDL_Window* window)
: mContext(std::move(context)) {
    SDL_Vulkan_CreateSurface(window, mContext->mInstance, reinterpret_cast<VkSurfaceKHR*>(&mSurface));

    int width;
    int height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);

    mDrawableSize.width = width;
    mDrawableSize.height = height;
}

gfx::Swapchain::~Swapchain() {
    releaseDrawables();
    if (mSwapchain) {
        mDevice->mDevice.destroySwapchainKHR(mSwapchain, nullptr, mDevice->mDispatchLoaderDynamic);
    }
    mContext->mInstance.destroySurfaceKHR(mSurface, nullptr, mContext->mDispatchLoaderDynamic);
}

void gfx::Swapchain::createDrawables() {
    vk::SurfaceCapabilitiesKHR capabilities = mDevice->mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface, mDevice->mDispatchLoaderDynamic);

    std::vector<uint32_t> queue_family_indices = {};
    if (mDevice->mGraphicsQueueFamilyIndex != mDevice->mPresentQueueFamilyIndex) {
        queue_family_indices.emplace_back(mDevice->mGraphicsQueueFamilyIndex);
        queue_family_indices.emplace_back(mDevice->mPresentQueueFamilyIndex);
    }

    vk::SwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.setSurface(mSurface);
    swapchain_create_info.setMinImageCount(mMaximumDrawableCount);
    swapchain_create_info.setImageFormat(mPixelFormat);
    swapchain_create_info.setImageColorSpace(mColorSpace);
    swapchain_create_info.setImageExtent(mDrawableSize);
    swapchain_create_info.setImageArrayLayers(1);
    swapchain_create_info.setImageUsage(capabilities.supportedUsageFlags);
    if (queue_family_indices.empty()) {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    } else {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapchain_create_info.setQueueFamilyIndices(queue_family_indices);
    }
    swapchain_create_info.setPreTransform(capabilities.currentTransform);
    swapchain_create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchain_create_info.setPresentMode(mDisplaySyncEnabled ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
    swapchain_create_info.setClipped(true);
    swapchain_create_info.setOldSwapchain(mSwapchain);

    vk::SwapchainKHR swapchain = mSwapchain;
    mSwapchain = mDevice->mDevice.createSwapchainKHR(swapchain_create_info, nullptr, mDevice->mDispatchLoaderDynamic);
    if (swapchain) {
        mDevice->mDevice.destroySwapchainKHR(swapchain, nullptr, mDevice->mDispatchLoaderDynamic);
    }
    auto images = mDevice->mDevice.getSwapchainImagesKHR(mSwapchain, mDevice->mDispatchLoaderDynamic);

    mDrawables.resize(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        vk::ImageViewCreateInfo view_create_info = {};
        view_create_info.setImage(images[i]);
        view_create_info.setViewType(vk::ImageViewType::e2D);
        view_create_info.setFormat(mPixelFormat);
        view_create_info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        auto texture = TransferPtr(new Texture(mDevice));
        texture->mExtent.setWidth(mDrawableSize.width);
        texture->mExtent.setHeight(mDrawableSize.height);
        texture->mExtent.setDepth(1);
        texture->mFormat = mPixelFormat;
        texture->mImage = images[i];
        texture->mImageView = mDevice->mDevice.createImageView(view_create_info, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);
        texture->mAllocation = {};
        texture->mImageSubresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        texture->mImageSubresourceRange.setLayerCount(1);
        texture->mImageSubresourceRange.setLevelCount(1);

        mDrawables[i] = TransferPtr(new Drawable(this, texture, uint32_t(i)));
    }
}

auto gfx::Swapchain::device() -> SharedPtr<Device> {
    return mDevice;
}

void gfx::Swapchain::setDevice(SharedPtr<Device> device) {
    mDevice = std::move(device);
}

void gfx::Swapchain::releaseDrawables() {
    mDevice->waitIdle();
    mDrawables.clear();
}

auto gfx::Swapchain::drawableSize() -> vk::Extent2D {
    return mDrawableSize;
}

void gfx::Swapchain::setDrawableSize(const vk::Extent2D& drawableSize) {
    mDrawableSize = drawableSize;
}

auto gfx::Swapchain::nextDrawable() -> SharedPtr<Drawable> {
    if (mDrawables.empty()) {
        createDrawables();
    }

    vk::FenceCreateInfo fence_create_info = {};
    vk::UniqueFence fence = mDevice->mDevice.createFenceUnique(fence_create_info, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);

    uint32_t index;
    vk::Result result = mDevice->mDevice.acquireNextImageKHR(
        mSwapchain,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index,
        mDevice->mDispatchLoaderDynamic
    );
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vk::resultCheck(mDevice->mDevice.waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max(), mDevice->mDispatchLoaderDynamic), "waitForFences");

    return mDrawables[uint64_t(index)];
}

auto gfx::Swapchain::pixelFormat() -> vk::Format {
    return mPixelFormat;
}

void gfx::Swapchain::setPixelFormat(vk::Format format) {
    mPixelFormat = format;
}

auto gfx::Swapchain::colorSpace() -> vk::ColorSpaceKHR {
    return mColorSpace;
}

void gfx::Swapchain::setColorSpace(vk::ColorSpaceKHR colorSpace) {
    mColorSpace = colorSpace;
}

auto gfx::Swapchain::displaySyncEnabled() -> bool {
    return mDisplaySyncEnabled;
}

void gfx::Swapchain::setDisplaySyncEnabled(bool displaySyncEnabled) {
    mDisplaySyncEnabled = displaySyncEnabled;
}

auto gfx::Swapchain::maximumDrawableCount() -> uint32_t {
    return mMaximumDrawableCount;
}

void gfx::Swapchain::setMaximumDrawableCount(uint32_t maximumDrawableCount) {
    mMaximumDrawableCount = maximumDrawableCount;
}

auto gfx::Swapchain::alloc(SharedPtr<Context> context, SDL_Window* window) -> SharedPtr<gfx::Swapchain> {
    return TransferPtr(new Swapchain(context, window));
}