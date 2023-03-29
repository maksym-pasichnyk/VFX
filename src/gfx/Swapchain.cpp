#include "Drawable.hpp"
#include "Swapchain.hpp"

gfx::Swapchain::Swapchain(ManagedShared<Device> device, ManagedShared<Surface> surface) : device(std::move(device)), surface(std::move(surface)) {}
gfx::Swapchain::~Swapchain() {
    device->raii.raw.destroySwapchainKHR(raw, nullptr, device->raii.dispatcher);
}

auto gfx::Swapchain::drawableSize() -> vk::Extent2D {
    auto extent = drawables[0].texture->extent;
    return vk::Extent2D().setWidth(extent.width).setHeight(extent.height);
}

auto gfx::Swapchain::nextDrawable() -> Drawable {
    auto fence = device->raii.raw.createFenceUnique(vk::FenceCreateInfo(), nullptr, device->raii.dispatcher);

    uint32_t index;
    vk::Result result = device->raii.raw.acquireNextImageKHR(
        raw,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index,
        device->raii.dispatcher
    );
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vk::resultCheck(device->raii.raw.waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max(), device->raii.dispatcher), "waitForFences");

    return drawables[uint64_t(index)];
}

void gfx::Swapchain::configure(const SurfaceConfiguration& config) {
    auto capabilities = device->instance->getSurfaceCapabilitiesKHR(device->adapter, surface->raw);

    std::vector<uint32_t> queue_family_indices = {};
//    if (mGraphicsQueueFamilyIndex != mPresentQueueFamilyIndex) {
//        queue_family_indices.emplace_back(mGraphicsQueueFamilyIndex);
//        queue_family_indices.emplace_back(mPresentQueueFamilyIndex);
//    }

    vk::SwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.setSurface(surface->raw);
    swapchain_create_info.setMinImageCount(config.image_count);
    swapchain_create_info.setImageFormat(config.format);
    swapchain_create_info.setImageColorSpace(config.color_space);
    swapchain_create_info.setImageExtent(capabilities.currentExtent);
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
    swapchain_create_info.setPresentMode(config.present_mode);
    swapchain_create_info.setClipped(config.clipped);
    swapchain_create_info.setOldSwapchain(raw);

    auto old_swapchain = raw;

    raw = device->raii.raw.createSwapchainKHR(swapchain_create_info, nullptr, device->raii.dispatcher);
    auto images = device->raii.raw.getSwapchainImagesKHR(raw, device->raii.dispatcher);

    if (old_swapchain) {
        device->raii.raw.destroySwapchainKHR(old_swapchain, nullptr, device->raii.dispatcher);
    }

    drawables.resize(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        vk::ImageViewCreateInfo view_create_info = {};
        view_create_info.setImage(images[i]);
        view_create_info.setViewType(vk::ImageViewType::e2D);
        view_create_info.setFormat(config.format);
        view_create_info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        auto texture = MakeShared(new Texture(device));
        texture->extent.setWidth(capabilities.currentExtent.width);
        texture->extent.setHeight(capabilities.currentExtent.height);
        texture->extent.setDepth(1);
        texture->format = config.format;
        texture->image = images[i];
        texture->image_view = device->raii.raw.createImageView(view_create_info, VK_NULL_HANDLE, device->raii.dispatcher);
        texture->allocation = {};
        texture->subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
        texture->subresource.setLayerCount(1);
        texture->subresource.setLevelCount(1);

        drawables[i] = Drawable(raw, texture, uint32_t(i));
    }
}