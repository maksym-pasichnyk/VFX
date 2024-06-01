#include "Adapter.hpp"
#include "Drawable.hpp"
#include "Swapchain.hpp"

gfx::Swapchain::Swapchain(rc<Device> device, rc<Surface> surface) : device(std::move(device)), surface(std::move(surface)) {}
gfx::Swapchain::~Swapchain() {
    device->handle.destroySwapchainKHR(handle, nullptr, device->dispatcher);
}

auto gfx::Swapchain::drawableSize(this Swapchain const& self) -> vk::Extent2D {
    auto extent = self.drawables[0]->texture->extent;
    return vk::Extent2D().setWidth(extent.width).setHeight(extent.height);
}

auto gfx::Swapchain::nextDrawable(this Swapchain& self) -> rc<Drawable> {
    auto fence = self.device->handle.createFenceUnique(vk::FenceCreateInfo(), nullptr, self.device->dispatcher);

    uint32_t image_index;
    auto result = self.device->handle.acquireNextImageKHR(
        self.handle,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &image_index,
        self.device->dispatcher
    );
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vk::resultCheck(self.device->handle.waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max(), self.device->dispatcher), "waitForFences");
    return self.drawables[uint64_t(image_index)];
}

void gfx::Swapchain::configure(this Swapchain& self, const SurfaceConfiguration& config) {
    auto capabilities = self.device->adapter->getSurfaceCapabilities(self.surface);

    std::vector<uint32_t> queue_family_indices = {};
//    if (mGraphicsQueueFamilyIndex != mPresentQueueFamilyIndex) {
//        queue_family_indices.emplace_back(mGraphicsQueueFamilyIndex);
//        queue_family_indices.emplace_back(mPresentQueueFamilyIndex);
//    }

    vk::SwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.setSurface(self.surface->handle);
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
    swapchain_create_info.setOldSwapchain(self.handle);

    auto old_swapchain = self.handle;
    self.handle = self.device->handle.createSwapchainKHR(swapchain_create_info, nullptr, self.device->dispatcher);
    auto images = self.device->handle.getSwapchainImagesKHR(self.handle, self.device->dispatcher);

    if (old_swapchain) {
        self.device->handle.destroySwapchainKHR(old_swapchain, nullptr, self.device->dispatcher);
    }

    self.drawables.resize(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        vk::ImageViewCreateInfo view_create_info = {};
        view_create_info.setImage(images[i]);
        view_create_info.setViewType(vk::ImageViewType::e2D);
        view_create_info.setFormat(config.format);
        view_create_info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        auto texture = rc<Texture>::init(
            self.device,
            images[i],
            config.format,
            vk::Extent3D(
                capabilities.currentExtent.width,
                capabilities.currentExtent.height,
                1
            ),
            self.device->handle.createImageView(view_create_info, VK_NULL_HANDLE, self.device->dispatcher),
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1),
            nullptr
        );
        self.drawables[i] = rc<Drawable>::init(self.handle, std::move(texture), uint32_t(i));
    }
}