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

    struct Swapchain : public ManagedObject {
        rc<Device>              device;
        rc<Surface>             surface;
        vk::SwapchainKHR        handle;
        std::vector<Drawable>   drawables;

        explicit Swapchain(rc<Device> device, rc<Surface> surface);
        ~Swapchain() override;

        auto nextDrawable(this Swapchain& self) -> Drawable;
        auto drawableSize(this Swapchain const& self) -> vk::Extent2D;

        void configure(this Swapchain& self, const SurfaceConfiguration& config);
    };
}