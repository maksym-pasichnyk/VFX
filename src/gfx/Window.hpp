#pragma once

#include "gfx/Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Application;

    struct Window final : Referencing<Window> {
        friend Application;

    private:
        void* pWindow = {};
        vk::SurfaceKHR vkSurface = {};
        SharedPtr<Application> mApplication = {};

    private:
        Window(SharedPtr<Application> application, int32_t width, int32_t height);
        ~Window() override;

    public:
        auto shouldClose() -> bool;
        auto surface() -> vk::SurfaceKHR;

    public:
        static auto alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window>;
    };
}
