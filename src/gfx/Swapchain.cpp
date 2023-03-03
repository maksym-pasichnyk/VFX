#include "Drawable.hpp"
#include "Swapchain.hpp"

gfx::SwapchainShared::SwapchainShared(Device device, Surface surface) : device(std::move(device)), surface(std::move(surface)) {}
gfx::SwapchainShared::~SwapchainShared() {
    device.shared->raii.raw.destroySwapchainKHR(raw, nullptr, device.shared->raii.dispatcher);
}

auto gfx::Swapchain::drawableSize() -> vk::Extent2D {
    auto extent = shared->drawables[0].texture.shared->extent;
    return vk::Extent2D().setWidth(extent.width).setHeight(extent.height);
}

auto gfx::Swapchain::nextDrawable() -> Drawable {
    auto fence = shared->device.shared->raii.raw.createFenceUnique(vk::FenceCreateInfo(), nullptr, shared->device.shared->raii.dispatcher);

    uint32_t index;
    vk::Result result = shared->device.shared->raii.raw.acquireNextImageKHR(
        shared->raw,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index,
        shared->device.shared->raii.dispatcher
    );
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vk::resultCheck(shared->device.shared->raii.raw.waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max(), shared->device.shared->raii.dispatcher), "waitForFences");

    return shared->drawables[uint64_t(index)];
}

void gfx::Swapchain::configure(const SurfaceConfiguration& config) {
    auto capabilities = shared->device.shared->instance.getSurfaceCapabilitiesKHR(shared->device.shared->adapter, shared->surface.shared->raw);

    std::vector<uint32_t> queue_family_indices = {};
//    if (shared->mGraphicsQueueFamilyIndex != shared->mPresentQueueFamilyIndex) {
//        queue_family_indices.emplace_back(shared->mGraphicsQueueFamilyIndex);
//        queue_family_indices.emplace_back(shared->mPresentQueueFamilyIndex);
//    }

    vk::SwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.setSurface(shared->surface.shared->raw);
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
    swapchain_create_info.setOldSwapchain(shared->raw);

    auto old_swapchain = shared->raw;

    shared->raw = shared->device.shared->raii.raw.createSwapchainKHR(swapchain_create_info, nullptr, shared->device.shared->raii.dispatcher);
    auto images = shared->device.shared->raii.raw.getSwapchainImagesKHR(shared->raw, shared->device.shared->raii.dispatcher);

    if (old_swapchain) {
        shared->device.shared->raii.raw.destroySwapchainKHR(old_swapchain, nullptr, shared->device.shared->raii.dispatcher);
    }

    shared->drawables.resize(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        vk::ImageViewCreateInfo view_create_info = {};
        view_create_info.setImage(images[i]);
        view_create_info.setViewType(vk::ImageViewType::e2D);
        view_create_info.setFormat(config.format);
        view_create_info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        auto texture = std::make_shared<TextureShared>(shared->device);
        texture->extent.setWidth(capabilities.currentExtent.width);
        texture->extent.setHeight(capabilities.currentExtent.height);
        texture->extent.setDepth(1);
        texture->format = config.format;
        texture->image = images[i];
        texture->image_view = shared->device.shared->raii.raw.createImageView(view_create_info, VK_NULL_HANDLE, shared->device.shared->raii.dispatcher);
        texture->allocation = {};
        texture->subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
        texture->subresource.setLayerCount(1);
        texture->subresource.setLevelCount(1);

        shared->drawables[i] = Drawable(shared->raw, Texture(texture), uint32_t(i));
    }
}