#pragma once

#include "Graphics.hpp"
#include "Canvas.hpp"
#include "ImGuiBackend.hpp"
#include "NotSwiftUI/NotSwiftUI.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <SDL_video.h>
#include <SDL_vulkan.h>
#include <SDL_events.h>

struct ShaderData {
    alignas(16) glm::mat4x4 g_proj_matrix;
    alignas(16) glm::mat4x4 g_view_matrix;
};

class WindowPlatform : public ManagedObject {
public:
    explicit WindowPlatform(const char* title, uint32_t width, uint32_t height) {
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    }
    ~WindowPlatform() override {
        SDL_DestroyWindow(window);
    }

public:
    auto createSurface(const rc<gfx::Instance>& instance) -> rc<gfx::Surface> {
        VkSurfaceKHR raw_surface;
        SDL_Vulkan_CreateSurface(window, instance->handle, &raw_surface);
        return instance->wrapSurface(raw_surface);
    }

    auto getAspectRatio() -> float {
        int32_t width;
        int32_t height;
        SDL_GetWindowSize(window, &width, &height);

        return static_cast<float>(width) / static_cast<float>(height);
    }

    auto getWindowSize() -> vk::Extent2D {
        int32_t width;
        int32_t height;
        SDL_GetWindowSize(window, &width, &height);

        vk::Extent2D size;
        size.setWidth(width);
        size.setHeight(height);
        return size;
    }

    auto getDrawableSize() -> vk::Extent2D {
        int width;
        int height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);

        vk::Extent2D size;
        size.setWidth(width);
        size.setHeight(height);
        return size;
    }

private:
    SDL_Window* window = nullptr;
};

struct Application {
public:
    explicit Application(const char* title) {
        platform = MakeShared<WindowPlatform>(title, 800, 600);
        instance = gfx::createInstance(gfx::InstanceConfiguration{
            .name = title,
            .version = 1
        });
        adapter = instance->enumerateAdapters().front();

        {
            auto queue_priorities = std::array{
                1.0F
            };
            auto queue_create_infos = std::array{
                vk::DeviceQueueCreateInfo()
                    .setQueueFamilyIndex(0)
                    .setQueuePriorities(queue_priorities)
            };
            auto extensions = std::array{
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
            };
            auto synchronization_2_features = vk::PhysicalDeviceSynchronization2Features()
                .setSynchronization2(VK_TRUE);
            auto portability_subset_features = vk::PhysicalDevicePortabilitySubsetFeaturesKHR()
                .setPNext(&synchronization_2_features)
                .setImageViewFormatSwizzle(VK_TRUE);
            auto timeline_semaphore_features = vk::PhysicalDeviceTimelineSemaphoreFeatures()
                .setPNext(&portability_subset_features)
                .setTimelineSemaphore(VK_TRUE);
            auto dynamic_rendering_features = vk::PhysicalDeviceDynamicRenderingFeatures()
                .setPNext(&timeline_semaphore_features)
                .setDynamicRendering(VK_TRUE);
            auto features_2 = vk::PhysicalDeviceFeatures2()
                .setPNext(&dynamic_rendering_features);
            auto create_info = vk::DeviceCreateInfo()
                .setPNext(&features_2)
                .setQueueCreateInfos(queue_create_infos)
                .setPEnabledExtensionNames(extensions);
            device = adapter->createDevice(create_info);
        }



        surface = platform->createSurface(instance);
        swapchain = device->createSwapchain(surface);

        gfx::SurfaceConfiguration surface_config = {};
        surface_config.format = vk::Format::eB8G8R8A8Unorm;
        surface_config.color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        surface_config.image_count = 3;
        surface_config.present_mode = vk::PresentModeKHR::eFifo;
        surface_config.clipped = true;
        swapchain->configure(surface_config);

        commandQueue = device->newCommandQueue();
        commandBuffer = commandQueue->newCommandBuffer();

        imgui = MakeShared<ImGuiBackend>(device);
        canvas = MakeShared<Canvas>(imgui->drawList());
    }

public:
    void run() {
        auto previous = std::chrono::steady_clock::now();

        while (true) {
            using seconds = std::chrono::duration<float, std::chrono::seconds::period>;

            auto current = std::chrono::steady_clock::now();
            auto elapsed = seconds(current - previous).count();
            previous = current;

            accumulateTotal -= accumulate[accumulateIndex];
            accumulate[accumulateIndex] = elapsed;
            accumulateTotal += accumulate[accumulateIndex];
            accumulateIndex = (accumulateIndex + 1) % static_cast<int32_t>(std::size(accumulate));
            accumulateCount = std::min(accumulateCount + 1, static_cast<int32_t>(std::size(accumulate)));
            average = accumulateTotal / static_cast<float>(accumulateCount);

            if (_pollEvents()) {
                break;
            }

            imgui->setCurrentContext();
            imgui->setScreenSize(getUISize(platform->getWindowSize()));

            update(elapsed);
            render();
        }
    }

public:
    virtual void update(float dt) {}
    virtual void render() {}
    virtual void keyUp(SDL_KeyboardEvent* event) {}
    virtual void keyDown(SDL_KeyboardEvent* event) {}
    virtual void mouseUp(SDL_MouseButtonEvent* event) {}
    virtual void mouseDown(SDL_MouseButtonEvent* event) {}
    virtual void mouseMove(SDL_MouseMotionEvent* event) {}
    virtual void mouseWheel(SDL_MouseWheelEvent* event) {}
    virtual void performClose(uint32_t windowId) {}
    virtual void performResize(uint32_t windowId) {
        gfx::SurfaceConfiguration config;
        config.format = vk::Format::eB8G8R8A8Unorm;
        config.color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        config.image_count = 3;
        config.present_mode = vk::PresentModeKHR::eFifo;
        config.clipped = true;

        swapchain->configure(config);
    }

protected:
    auto _pollEvents() -> bool {
        bool quit = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    quit = true;
                    break;
                }
                case SDL_WINDOWEVENT: {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_CLOSE: {
                            performClose(event.window.windowID);
                            break;
                        }
//                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED: {
                            performResize(event.window.windowID);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
                case SDL_KEYUP: {
                    keyUp(&event.key);
                    break;
                }
                case SDL_KEYDOWN: {
                    keyDown(&event.key);
                    break;
                }
                case SDL_MOUSEBUTTONUP: {
                    mouseUp(&event.button);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    mouseDown(&event.button);
                    break;
                }
                case SDL_MOUSEMOTION: {
                    mouseMove(&event.motion);
                    break;
                }
                case SDL_MOUSEWHEEL: {
                    mouseWheel(&event.wheel);
                    break;
                }
                default: {
                    break;
                }
            }
        }
        return quit;
    }

    auto _drawView(const rc<View>& view) {
        auto uiSize = getUISize(platform->getWindowSize());
        auto childSize = view->getPreferredSize(ProposedSize(uiSize));
        auto translate = view->translation(childSize, uiSize, Alignment::center());

        canvas->saveState();
        canvas->translateBy(translate.x, translate.y);
        view->_draw(canvas, childSize);
        canvas->restoreState();
    }

public:
    static auto getPerspectiveProjection(float fovy, float aspect, float zNear, float zFar) -> glm::mat4x4 {
        float range = tan(fovy * 0.5F);

        float x = +1.0F / (range * aspect);
        float y = -1.0F / (range);
        float z = zFar / (zFar - zNear);
        float a = 1.0F;
        float b = -(zFar * zNear) / (zFar - zNear);

        return glm::mat4x4 {
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, a,
            0, 0, b, 0
        };
    }

    static auto getUISize(const vk::Extent2D& size) -> Size {
        auto w = static_cast<float_t>(size.width);
        auto h = static_cast<float_t>(size.height);
        return Size{w, h};
    }

//todo: make this private
protected:
    rc<WindowPlatform>       platform        = {};
    float_t                             average         = {};
    float_t                             accumulate[60]  = {};
    float_t                             accumulateTotal = {};
    int32_t                             accumulateCount = {};
    int32_t                             accumulateIndex = {};

    rc<gfx::Adapter>         adapter         = {};
    rc<gfx::Device>          device          = {};
    rc<gfx::Instance>        instance        = {};
    rc<gfx::Surface>         surface         = {};

    rc<gfx::Swapchain>       swapchain       = {};
    rc<gfx::CommandQueue>    commandQueue    = {};
    rc<gfx::CommandBuffer>   commandBuffer   = {};

    rc<Canvas>               canvas;
    rc<ImGuiBackend>         imgui;
};
