#include "swapchain.hpp"
#include "drawable.hpp"
#include "context.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "pass.hpp"

#include "spdlog/spdlog.h"

#include <set>

namespace vfx {
    inline auto select_surface_extent(const vk::Extent2D& extent, const vk::SurfaceCapabilitiesKHR &capabilities) -> vk::Extent2D {
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

    inline auto select_surface_format(std::span<const vk::SurfaceFormatKHR> surface_formats, std::span<const vk::Format> request_formats, vk::ColorSpaceKHR request_color_space) -> vk::SurfaceFormatKHR {
        if (surface_formats.size() == 1) {
            if (surface_formats.front().format == vk::Format::eUndefined) {
                return vk::SurfaceFormatKHR {
                    .format = request_formats.front(),
                    .colorSpace = request_color_space
                };
            }
            return surface_formats.front();
        }

        for (auto&& request_format : request_formats) {
            for (auto&& surface_format : surface_formats) {
                if (surface_format.format == request_format && surface_format.colorSpace == request_color_space) {
                    return surface_format;
                }
            }
        }
        return surface_formats.front();
    }

    inline auto select_present_mode(std::span<const vk::PresentModeKHR> present_modes, std::span<const vk::PresentModeKHR> request_modes) -> vk::PresentModeKHR {
        for (auto request_mode : request_modes) {
            for (auto present_mode : present_modes) {
                if (request_mode == present_mode) {
                    return request_mode;
                }
            }
        }
        return vk::PresentModeKHR::eFifo;
    }
}

vfx::Swapchain::Swapchain(const Arc<Context>& context) : context(context) {
    fence = context->device->createFenceUnique({});
}

vfx::Swapchain::~Swapchain() {
    freeDrawables();
}

void vfx::Swapchain::updateDrawables() {
    drawables.clear();

    const auto capabilities = context->gpu.getSurfaceCapabilitiesKHR(surface->handle);
    drawableSize = select_surface_extent(vk::Extent2D{0, 0}, capabilities);

    u32 min_image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0) {
        min_image_count = std::min(min_image_count, capabilities.maxImageCount);
    }
    auto unique_family_indices = std::set<u32>{
        context->graphics_queue_family_index,
        context->present_queue_family_index
    };

    auto queue_family_indices = std::vector<u32>(
        unique_family_indices.begin(),
        unique_family_indices.end()
    );

    vk::SwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.setSurface(surface->handle);
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
    swapchain_create_info.setOldSwapchain(*handle);

    handle = context->device->createSwapchainKHRUnique(swapchain_create_info, nullptr);

    auto images = context->device->getSwapchainImagesKHR(*handle);

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

        auto view = context->device->createImageView(view_create_info);

        drawables[i] = Arc<Drawable>::alloc();
        drawables[i]->index = u32(i);
        drawables[i]->swapchain = this;
        drawables[i]->texture = Arc<Texture>::alloc();
        drawables[i]->texture->context = &*context;
        drawables[i]->texture->size = drawableSize;
        drawables[i]->texture->format = pixelFormat;
        drawables[i]->texture->image = images[i];
        drawables[i]->texture->view = view;
        drawables[i]->texture->allocation = {};
        drawables[i]->texture->aspect = vk::ImageAspectFlagBits::eColor;
    }
}

void vfx::Swapchain::freeDrawables() {
    drawables.clear();
    handle = {};
}

auto vfx::Swapchain::nextDrawable() -> vfx::Drawable* {
    u32 index;
    auto result = context->device->acquireNextImageKHR(
        *handle,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index
    );
    std::ignore = context->device->waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max());
    std::ignore = context->device->resetFences(1, &*fence);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        spdlog::info("Swapchain is out of date");
    } else if (result == vk::Result::eSuboptimalKHR) {
        spdlog::info("Swapchain is suboptimal");
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    return &*drawables[u64(index)];
}
