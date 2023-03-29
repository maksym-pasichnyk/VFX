#pragma once

#include "Device.hpp"
#include "Surface.hpp"

namespace gfx {
    struct Drawable;

    struct SurfaceConfiguration {
        vk::Format          format       = {};
        vk::ColorSpaceKHR   color_space  = {};
        uint32_t            image_count  = {};
        vk::PresentModeKHR  present_mode = {};
        bool                clipped      = {};
    };

    struct Swapchain : ManagedObject<Swapchain> {
        ManagedShared<Device>   device;
        ManagedShared<Surface>  surface;
        vk::SwapchainKHR        raw;
        std::vector<Drawable>   drawables;

        explicit Swapchain(ManagedShared<Device> device, ManagedShared<Surface> surface);
        ~Swapchain() override;

        auto nextDrawable() -> Drawable;
        auto drawableSize() -> vk::Extent2D;

        void configure(const SurfaceConfiguration& config);
    };
}