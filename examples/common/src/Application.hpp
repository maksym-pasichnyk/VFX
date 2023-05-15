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

class WindowPlatform : public ManagedObject<WindowPlatform> {
public:
    explicit WindowPlatform(const char* title, uint32_t width, uint32_t height) {
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    }
    ~WindowPlatform() override {
        SDL_DestroyWindow(window);
    }

public:
    auto createSurface(const ManagedShared<gfx::Instance>& instance) -> ManagedShared<gfx::Surface> {
        VkSurfaceKHR raw_surface;
        SDL_Vulkan_CreateSurface(window, instance->raii.raw, &raw_surface);
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

        gfx::InstanceConfiguration instance_config = {};
        instance_config.name = title;
        instance_config.version = 1;

        instance = gfx::createInstance(instance_config);
        adapter = instance->enumerateAdapters().front();
        device = instance->createDevice(adapter);

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
        commandBuffer = commandQueue->commandBuffer();

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

    auto _drawView(const ManagedShared<View>& view) {
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
    ManagedShared<WindowPlatform>       platform        = {};
    float_t                             average         = {};
    float_t                             accumulate[60]  = {};
    float_t                             accumulateTotal = {};
    int32_t                             accumulateCount = {};
    int32_t                             accumulateIndex = {};

    ManagedShared<gfx::Adapter>         adapter         = {};
    ManagedShared<gfx::Device>          device          = {};
    ManagedShared<gfx::Instance>        instance        = {};
    ManagedShared<gfx::Surface>         surface         = {};

    ManagedShared<gfx::Swapchain>       swapchain       = {};
    ManagedShared<gfx::CommandQueue>    commandQueue    = {};
    ManagedShared<gfx::CommandBuffer>   commandBuffer   = {};

    ManagedShared<Canvas>               canvas;
    ManagedShared<ImGuiBackend>         imgui;
};
