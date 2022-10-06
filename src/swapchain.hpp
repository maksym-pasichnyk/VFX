#include "context.hpp"
#include "pass.hpp"

namespace vfx {
    struct Swapchain {
        Context& context;

        vk::SurfaceKHR surface{};
        vk::SwapchainKHR swapchain{};
        std::vector<Box<Texture>> textures{};

        Box<RenderPass> render_pass;
        std::vector<vk::Framebuffer> framebuffers{};

        std::vector<vk::Fence> fences{};
        std::vector<vk::Fence> image_fences{};
        std::vector<vk::Semaphore> acquired_semaphores{};
        std::vector<vk::Semaphore> complete_semaphores{};

        vk::Format pixel_format = vk::Format::eUndefined;
        vk::Extent2D image_extent = {};

        u64 current_frame = 0;

        bool out_of_date = false;

        Swapchain(Context& context, Display& display) : context(context) {
            surface = display.create_surface(context.instance);

            create_swapchain();
            create_sync_objects();

            RenderPassDescription pass_description{};
            pass_description.definitions = {
                SubpassDescription{
                    .colorAttachments = {
                        vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}
                    }
                }
            };
            pass_description.attachments[0].format = pixel_format;
            pass_description.attachments[0].samples = vk::SampleCountFlagBits::e1;
            pass_description.attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
            pass_description.attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
            pass_description.attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            pass_description.attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            pass_description.attachments[0].initialLayout = vk::ImageLayout::eUndefined;
            pass_description.attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

            render_pass = context.makeRenderPass(pass_description);
            create_framebuffers();
        }

        ~Swapchain() {
            context.freeRenderPass(render_pass);

            destroy_sync_objects();
            destroy_swapchain();

            context.instance.destroySurfaceKHR(surface);
        }

        void create_swapchain() {
            const auto formats = context.physical_device.getSurfaceFormatsKHR(surface);
            const auto capabilities = context.physical_device.getSurfaceCapabilitiesKHR(surface);
            const auto present_modes = context.physical_device.getSurfacePresentModesKHR(surface);

            const auto request_formats = std::array {
                vk::Format::eB8G8R8A8Unorm,
                vk::Format::eR8G8B8A8Unorm,
                vk::Format::eB8G8R8Unorm,
                vk::Format::eR8G8B8Unorm
            };

            const auto request_modes = std::array {
                vk::PresentModeKHR::eFifo
            };

            auto surface_extent = select_surface_extent(vk::Extent2D{0, 0}, capabilities);
            auto surface_format = select_surface_format(formats, request_formats, vk::ColorSpaceKHR::eSrgbNonlinear);
            auto present_mode = select_present_mode(present_modes, request_modes);

            pixel_format = surface_format.format;
            image_extent = surface_extent;

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
                .imageFormat = surface_format.format,
                .imageColorSpace = surface_format.colorSpace,
                .imageExtent = surface_extent,
                .imageArrayLayers = 1,
                .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                .imageSharingMode = flag ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
                .queueFamilyIndexCount = flag ? static_cast<u32>(queue_family_indices.size()) : 0,
                .pQueueFamilyIndices = flag ? queue_family_indices.data() : nullptr,
                .preTransform = capabilities.currentTransform,
                .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                .presentMode = present_mode,
                .clipped = true,
                .oldSwapchain = nullptr
            };

            swapchain = context.logical_device.createSwapchainKHR(swapchain_create_info, nullptr);

            auto images = context.logical_device.getSwapchainImagesKHR(swapchain);

            textures.resize(images.size());
            image_fences.resize(images.size());

            for (u64 i = 0; i < images.size(); ++i) {
                const auto view_create_info = vk::ImageViewCreateInfo{
                    .image = images[i],
                    .viewType = vk::ImageViewType::e2D,
                    .format = pixel_format,
                    .subresourceRange = {
                        vk::ImageAspectFlagBits::eColor,
                        0, 1, 0, 1
                    }
                };

                auto view = context.logical_device.createImageView(view_create_info);

                textures[i] = Box<Texture>::alloc();
                textures[i]->width = surface_extent.width;
                textures[i]->height = surface_extent.height;
                textures[i]->format = surface_format.format;
                textures[i]->image = images[i];
                textures[i]->view = view;
                textures[i]->allocation = {};
            }
        }

        void destroy_swapchain() {
            for (auto& texture : textures) {
                context.freeTexture(texture);
            }
            for (auto& framebuffer : framebuffers) {
                context.logical_device.destroyFramebuffer(framebuffer);
            }
            context.logical_device.destroySwapchainKHR(swapchain);
        }

        void create_sync_objects() {
            fences.resize(Context::MAX_FRAMES_IN_FLIGHT);
            acquired_semaphores.resize(Context::MAX_FRAMES_IN_FLIGHT);
            complete_semaphores.resize(Context::MAX_FRAMES_IN_FLIGHT);

            for (u32 i = 0; i < Context::MAX_FRAMES_IN_FLIGHT; i++) {
                fences[i] = context.logical_device.createFence({.flags = vk::FenceCreateFlagBits::eSignaled});
                acquired_semaphores[i] = context.logical_device.createSemaphore({});
                complete_semaphores[i] = context.logical_device.createSemaphore({});
            }
        }

        void destroy_sync_objects() {
            for (u64 i = 0; i < Context::MAX_FRAMES_IN_FLIGHT; i++) {
                context.logical_device.destroyFence(fences[i]);
                context.logical_device.destroySemaphore(acquired_semaphores[i]);
                context.logical_device.destroySemaphore(complete_semaphores[i]);
            }
        }

        void create_framebuffers() {
            framebuffers.resize(textures.size());
            for (u64 i = 0; i < textures.size(); ++i) {
                auto attachments = std::array{ textures[i]->view };

                vk::FramebufferCreateInfo fb_create_info{};
                fb_create_info.setRenderPass(render_pass->handle);
                fb_create_info.setAttachments(attachments);
                fb_create_info.setWidth(textures[i]->width);
                fb_create_info.setHeight(textures[i]->height);
                fb_create_info.setLayers(1);

                framebuffers[i] = context.logical_device.createFramebuffer(fb_create_info);
            }
        }

        auto acquire_next_image(u32& image_index) -> bool {
            std::ignore = context.logical_device.waitForFences(fences[current_frame], true, std::numeric_limits<uint64_t>::max());
            auto result = context.logical_device.acquireNextImageKHR(
                swapchain,
                std::numeric_limits<uint64_t>::max(),
                acquired_semaphores[current_frame],
                nullptr,
                &image_index
            );
            if (result == vk::Result::eErrorOutOfDateKHR) {
                out_of_date = true;
            } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
                throw std::runtime_error("failed to acquire swapchain image");
            }
            return true;
        }

        void submit(u32 image_index, vk::CommandBuffer command_buffer) {
            if (auto fence = std::exchange(image_fences[image_index], fences[current_frame])) {
                std::ignore = context.logical_device.waitForFences(fence, true, std::numeric_limits<uint64_t>::max());
            }

            const auto stages = std::array{
                vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput}
            };

            auto submit_info = vk::SubmitInfo{};
            submit_info.setWaitDstStageMask(stages);
            submit_info.setCommandBuffers(command_buffer);
            submit_info.setWaitSemaphores(acquired_semaphores[current_frame]);
            submit_info.setSignalSemaphores(complete_semaphores[current_frame]);

            context.logical_device.resetFences(fences[current_frame]);
            context.graphics_queue.submit(submit_info, fences[current_frame]);
        }

        void present(u32 image_index) {
            auto present_info = vk::PresentInfoKHR{};
            present_info.setWaitSemaphores(complete_semaphores[current_frame]);
            present_info.setSwapchains(swapchain);
            present_info.setImageIndices(image_index);
            auto result = context.present_queue.presentKHR(present_info);

            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
                out_of_date = true;
            } else if (result != vk::Result::eSuccess) {
                throw std::runtime_error("failed to present swapchain image");
            }

            current_frame = (current_frame + 1) % Context::MAX_FRAMES_IN_FLIGHT;

            if (out_of_date) {
                out_of_date = false;

                context.logical_device.waitIdle();

                destroy_swapchain();
                create_swapchain();
                create_framebuffers();
            }
        }

    private:
        static auto select_surface_extent(const vk::Extent2D& extent, const vk::SurfaceCapabilitiesKHR &capabilities) -> vk::Extent2D {
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

        static auto select_surface_format(std::span<const vk::SurfaceFormatKHR> surface_formats, std::span<const vk::Format> request_formats, vk::ColorSpaceKHR request_color_space) -> vk::SurfaceFormatKHR {
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

        static auto select_present_mode(std::span<const vk::PresentModeKHR> present_modes, std::span<const vk::PresentModeKHR> request_modes) -> vk::PresentModeKHR {
            for (auto request_mode : request_modes) {
                for (auto present_mode : present_modes) {
                    if (request_mode == present_mode) {
                        return request_mode;
                    }
                }
            }
            return vk::PresentModeKHR::eFifo;
        }

    };
}