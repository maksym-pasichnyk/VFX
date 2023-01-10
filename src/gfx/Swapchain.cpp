#include "Swapchain.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"

#include "spdlog/spdlog.h"

#include <set>

gfx::Swapchain::Swapchain(Context* context, vk::SurfaceKHR surface)
: pContext(context), mSurface(surface) {}

gfx::Swapchain::~Swapchain() {
    releaseDrawables();
    if (vkSwapchain) {
        mDevice->vkDevice.destroySwapchainKHR(vkSwapchain, nullptr, mDevice->vkDispatchLoaderDynamic);
    }
    pContext->vkInstance.destroySurfaceKHR(mSurface, nullptr, pContext->vkDispatchLoaderDynamic);
}

void gfx::Swapchain::createDrawables() {
    vk::SurfaceCapabilitiesKHR capabilities = mDevice->vkPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface, mDevice->vkDispatchLoaderDynamic);

    auto unique_family_indices = std::set<uint32_t>{
        mDevice->vkGraphicsQueueFamilyIndex,
        mDevice->vkPresentQueueFamilyIndex
    };

    auto queue_family_indices = std::vector<uint32_t>(
        unique_family_indices.begin(),
        unique_family_indices.end()
    );

    vk::SwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.setSurface(mSurface);
    swapchain_create_info.setMinImageCount(mMaximumDrawableCount);
    swapchain_create_info.setImageFormat(mPixelFormat);
    swapchain_create_info.setImageColorSpace(mColorSpace);
    swapchain_create_info.setImageExtent(mDrawableSize);
    swapchain_create_info.setImageArrayLayers(1);
    swapchain_create_info.setImageUsage(capabilities.supportedUsageFlags);
    if (queue_family_indices.size() > 1) {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapchain_create_info.setQueueFamilyIndices(queue_family_indices);
    } else {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    swapchain_create_info.setPreTransform(capabilities.currentTransform);
    swapchain_create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchain_create_info.setPresentMode(mDisplaySyncEnabled ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
    swapchain_create_info.setClipped(true);
    swapchain_create_info.setOldSwapchain(vkSwapchain);

    vk::SwapchainKHR oldSwapchain = vkSwapchain;
    vkSwapchain = mDevice->vkDevice.createSwapchainKHR(swapchain_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);

    if (oldSwapchain) {
        mDevice->vkDevice.destroySwapchainKHR(oldSwapchain, nullptr, mDevice->vkDispatchLoaderDynamic);
    }
    auto images = mDevice->vkDevice.getSwapchainImagesKHR(vkSwapchain, mDevice->vkDispatchLoaderDynamic);

    mDrawables.resize(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        vk::ImageViewCreateInfo view_create_info = {};
        view_create_info.setImage(images[i]);
        view_create_info.setViewType(vk::ImageViewType::e2D);
        view_create_info.setFormat(mPixelFormat);
        view_create_info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        auto texture = TransferPtr(new Texture(mDevice));
        texture->vkExtent.setWidth(mDrawableSize.width);
        texture->vkExtent.setHeight(mDrawableSize.height);
        texture->vkExtent.setDepth(1);
        texture->vkFormat = mPixelFormat;
        texture->vkImage = images[i];
        texture->vkImageView = mDevice->vkDevice.createImageView(view_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
        texture->vmaAllocation = {};
        texture->vkImageSubresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        texture->vkImageSubresourceRange.setLayerCount(1);
        texture->vkImageSubresourceRange.setLevelCount(1);

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
    vk::UniqueFence fence = mDevice->vkDevice.createFenceUnique(fence_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);

    uint32_t index;
    vk::Result result = mDevice->vkDevice.acquireNextImageKHR(
        vkSwapchain,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index,
        mDevice->vkDispatchLoaderDynamic
    );
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vk::resultCheck(mDevice->vkDevice.waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max(), mDevice->vkDispatchLoaderDynamic), "waitForFences");

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