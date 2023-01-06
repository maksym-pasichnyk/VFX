#pragma once

#include "Object.hpp"
#include "Window.hpp"

#include <list>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

namespace gfx {
    struct Device;
    struct Window;
    struct Surface;
    struct Application;

    struct ApplicationDelegate : Referencing {
        virtual void applicationDidFinishLaunching(const SharedPtr<Application>& sender) = 0;
    };

    struct Application final : Referencing {
        friend Device;
        friend Window;
        friend Surface;

    private:
        bool mRunning = {};

        vk::Instance vkInstance = {};
        vk::DynamicLoader vkDynamicLoader = {};
        vk::DispatchLoaderDynamic vkDispatchLoaderDynamic = {};
        vk::DebugUtilsMessengerEXT vkDebugUtilsMessengerEXT = {};
        std::list<SharedPtr<Window>> mWindows = {};
        std::vector<SharedPtr<Device>> mDevices = {};
        SharedPtr<ApplicationDelegate> mDelegate = {};

    private:
        Application();
        ~Application() override;

    public:
        void run();
        auto devices() -> const std::vector<SharedPtr<Device>>&;
        auto delegate() -> SharedPtr<ApplicationDelegate>;
        void setDelegate(SharedPtr<ApplicationDelegate> delegate);

    private:
        void pollEvents();
        static auto _getWindowById(uint32_t id) -> gfx::Window*;

    public:
        static auto sharedApplication() -> SharedPtr<Application>;
    };
}