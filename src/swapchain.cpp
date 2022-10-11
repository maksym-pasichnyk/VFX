#include "swapchain.hpp"
#include "drawable.hpp"
#include "context.hpp"
#include "texture.hpp"
#include "pass.hpp"

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

vfx::Swapchain::Swapchain(Context& context, vk::SurfaceKHR surface) : context(context), surface(surface) {
    const auto formats = context.physical_device.getSurfaceFormatsKHR(surface);
    const auto request_formats = std::array {
        vk::Format::eB8G8R8A8Unorm,
        vk::Format::eR8G8B8A8Unorm,
        vk::Format::eB8G8R8Unorm,
        vk::Format::eR8G8B8Unorm
    };
    auto surface_format = select_surface_format(formats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);

    colorSpace = surface_format.colorSpace;
    pixelFormat = surface_format.format;
    presentMode = vk::PresentModeKHR::eFifo;

    create_render_pass();
    create_swapchain();
    create_drawables();
}

vfx::Swapchain::~Swapchain() {
    destroy_drawables();
    destroy_swapchain();
    context.freeRenderPass(renderPass);
}

void vfx::Swapchain::create_swapchain() {
    const auto capabilities = context.physical_device.getSurfaceCapabilitiesKHR(surface);
    drawableSize = select_surface_extent(vk::Extent2D{0, 0}, capabilities);

    u32 min_image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0) {
        min_image_count = std::min(min_image_count, capabilities.maxImageCount);
    }

    const auto queue_family_indices = std::array{
        context.graphics_family,
        context.present_family
    };

    const auto flag = context.graphics_family != context.present_family;

    const auto swapchain_create_info = vk::SwapchainCreateInfoKHR {
        .surface = surface,
        .minImageCount = min_image_count,
        .imageFormat = pixelFormat,
        .imageColorSpace = colorSpace,
        .imageExtent = drawableSize,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = flag ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        .queueFamilyIndexCount = flag ? u32(queue_family_indices.size()) : 0,
        .pQueueFamilyIndices = flag ? queue_family_indices.data() : nullptr,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true,
        .oldSwapchain = nullptr
    };

    handle = context.logical_device.createSwapchainKHR(swapchain_create_info, nullptr);
}

void vfx::Swapchain::destroy_swapchain() {
    context.logical_device.destroySwapchainKHR(handle);
}

void vfx::Swapchain::create_render_pass() {
    vfx::RenderPassDescription pass_description{};
    pass_description.definitions = {
        vfx::SubpassDescription{
            .colorAttachments = {
                vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}
            }
        }
    };
    pass_description.attachments[0].format = pixelFormat;
    pass_description.attachments[0].samples = vk::SampleCountFlagBits::e1;
    pass_description.attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    pass_description.attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    pass_description.attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    pass_description.attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    pass_description.attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    pass_description.attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

    renderPass = context.makeRenderPass(pass_description);
}

void vfx::Swapchain::create_drawables() {
    auto images = context.logical_device.getSwapchainImagesKHR(handle);

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

        auto view = context.logical_device.createImageView(view_create_info);

        drawables[i] = Arc<Drawable>::alloc();
        drawables[i]->index = u32(i);
        drawables[i]->layer = this;
        drawables[i]->texture = Box<Texture>::alloc();
        drawables[i]->texture->context = &context;
        drawables[i]->texture->size = drawableSize;
        drawables[i]->texture->format = pixelFormat;
        drawables[i]->texture->image = images[i];
        drawables[i]->texture->view = view;
        drawables[i]->texture->allocation = {};

        auto attachments = std::array{
            drawables[i]->texture->view
        };
        vk::FramebufferCreateInfo fb_create_info{};
        fb_create_info.setRenderPass(renderPass->handle);
        fb_create_info.setAttachments(attachments);
        fb_create_info.setWidth(drawableSize.width);
        fb_create_info.setHeight(drawableSize.height);
        fb_create_info.setLayers(1);

        drawables[i]->framebuffer = context.logical_device.createFramebuffer(fb_create_info);
    }
}

void vfx::Swapchain::destroy_drawables() {
    for (auto& drawable : drawables) {
        context.freeTexture(drawable->texture);
        context.logical_device.destroyFramebuffer(drawable->framebuffer);
    }
}

void vfx::Swapchain::rebuild() {
    context.logical_device.waitIdle();

    destroy_drawables();
    destroy_swapchain();
    create_swapchain();
    create_drawables();
}

auto vfx::Swapchain::nextDrawable() -> vfx::Drawable* {
    // todo: recycle fences
    auto fence = context.logical_device.createFence({});

    u32 index;
    auto result = context.logical_device.acquireNextImageKHR(
        handle,
        std::numeric_limits<uint64_t>::max(),
        nullptr, // image_available_semaphores[current_frame],
        fence,
        &index
    );
    std::ignore = context.logical_device.waitForFences(1, &fence, true, std::numeric_limits<uint64_t>::max());
    context.logical_device.destroyFence(fence);

    if (result == vk::Result::eErrorOutOfDateKHR) {
//                rebuild();
    } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    return drawables[u64(index)].get();
}

auto vfx::Swapchain::getPixelFormat() -> vk::Format {
    return pixelFormat;
}

auto vfx::Swapchain::getDrawableSize() -> vk::Extent2D {
    return drawableSize;
}

auto vfx::Swapchain::getDefaultRenderPass() -> const Arc<RenderPass>& {
    return renderPass;
}