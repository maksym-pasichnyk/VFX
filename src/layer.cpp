#include "layer.hpp"
#include "device.hpp"
#include "context.hpp"
#include "texture.hpp"

#include "spdlog/spdlog.h"

#include <set>

namespace vfx {
    inline auto select_surface_extent(const vk::Extent2D& extent, const vk::SurfaceCapabilitiesKHR& capabilities) -> vk::Extent2D {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
            return capabilities.currentExtent;
        }

        const auto minExtent = capabilities.minImageExtent;
        const auto maxExtent = capabilities.maxImageExtent;

        return {
            std::clamp(extent.width, minExtent.width, maxExtent.width),
            std::clamp(extent.height, minExtent.height, maxExtent.height)
        };
    }
}

vfx::Layer::Layer(const Arc<Device>& device) : device(device) {
    fence = device->handle->createFenceUnique({});
}

void vfx::Layer::updateDrawables() {
    drawables.clear();

    const auto capabilities = device->gpu.getSurfaceCapabilitiesKHR(*surface);
    drawableSize = select_surface_extent(vk::Extent2D{0, 0}, capabilities);

    u32 min_image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0) {
        min_image_count = std::min(min_image_count, capabilities.maxImageCount);
    }
    auto unique_family_indices = std::set<u32>{
        device->graphics_queue_family_index,
        device->present_queue_family_index
    };

    auto queue_family_indices = std::vector<u32>(
        unique_family_indices.begin(),
        unique_family_indices.end()
    );

    vk::SwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.setSurface(*surface);
    swapchain_create_info.setMinImageCount(min_image_count);
    swapchain_create_info.setImageFormat(pixelFormat);
    swapchain_create_info.setImageColorSpace(colorSpace);
    swapchain_create_info.setImageExtent(drawableSize);
    swapchain_create_info.setImageArrayLayers(1);
    swapchain_create_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    if (queue_family_indices.size() > 1) {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapchain_create_info.setQueueFamilyIndices(queue_family_indices);
    } else {
        swapchain_create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    }
    swapchain_create_info.setPreTransform(capabilities.currentTransform);
    swapchain_create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchain_create_info.setPresentMode(displaySyncEnabled ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
    swapchain_create_info.setClipped(true);
    swapchain_create_info.setOldSwapchain(*swapchain);

    swapchain = device->handle->createSwapchainKHRUnique(swapchain_create_info, nullptr);

    auto images = device->handle->getSwapchainImagesKHR(*swapchain);

    drawables.resize(images.size());
    for (u64 i = 0; i < images.size(); ++i) {
        const auto view_create_info = vk::ImageViewCreateInfo{
            .image = images[i],
            .viewType = vk::ImageViewType::e2D,
            .format = pixelFormat,
            .subresourceRange = {
                vk::ImageAspectFlagBits::eColor,
                0, 1, 0, 1
            }
        };

        auto view = device->handle->createImageView(view_create_info);

        drawables[i] = Arc<Drawable>::alloc();
        drawables[i]->index = u32(i);
        drawables[i]->layer = this;
        drawables[i]->texture = Arc<Texture>::alloc();
        drawables[i]->texture->device = &*device;
        drawables[i]->texture->size = drawableSize;
        drawables[i]->texture->format = pixelFormat;
        drawables[i]->texture->image = images[i];
        drawables[i]->texture->view = view;
        drawables[i]->texture->allocation = {};
        drawables[i]->texture->aspect = vk::ImageAspectFlagBits::eColor;
    }
}

void vfx::Layer::freeDrawables() {
    drawables.clear();
    swapchain = {};
}

auto vfx::Layer::nextDrawable() -> vfx::Drawable* {
    u32 index;
    auto result = device->handle->acquireNextImageKHR(
        *swapchain,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index
    );
    std::ignore = device->handle->waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max());
    std::ignore = device->handle->resetFences(1, &*fence);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        spdlog::info("Swapchain is out of date");
    } else if (result == vk::Result::eSuboptimalKHR) {
        spdlog::info("Swapchain is suboptimal");
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    return &*drawables[u64(index)];
}