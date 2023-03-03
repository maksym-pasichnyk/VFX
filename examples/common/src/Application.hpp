#pragma once

#include "Graphics.hpp"
#include "UIContext.hpp"
#include "UIRenderer.hpp"
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

struct Application {
public:
    explicit Application(const char* title) {
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        gfx::InstanceSettings desc = {};
        desc.name = title;
        desc.version = 1;

        instance = gfx::createInstance(desc);
        adapter = instance.enumerateAdapters().front();
        device = instance.createDevice(adapter);

        VkSurfaceKHR raw_surface;
        SDL_Vulkan_CreateSurface(window, instance.shared->raii.raw, &raw_surface);

        // todo: merge surface with swapchain?
        surface = instance.wrapSurface(raw_surface);
        swapchain = device.createSwapchain(surface);

        gfx::SurfaceConfiguration config;
        config.format = vk::Format::eB8G8R8A8Unorm;
        config.color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
        config.image_count = 3;
        config.present_mode = vk::PresentModeKHR::eFifo;
        config.clipped = true;
        swapchain.configure(config);

        commandQueue = device.newCommandQueue();
        commandBuffer = commandQueue.commandBuffer();

        uiRenderer = sp<UIRenderer>::of(device);
        uiContext = sp<UIContext>::of(uiRenderer->drawList());
    }

    ~Application() {
        SDL_DestroyWindow(window);
    }

public:
    void run() {
        auto previous = std::chrono::steady_clock::now();

        running = true;
        while (running) {
            using seconds = std::chrono::duration<float_t, std::chrono::seconds::period>;

            auto current = std::chrono::steady_clock::now();
            auto elapsed = seconds(current - previous).count();
            previous = current;

            accumulateTotal -= accumulate[accumulateIndex];
            accumulate[accumulateIndex] = elapsed;
            accumulateTotal += accumulate[accumulateIndex];
            accumulateIndex = (accumulateIndex + 1) % static_cast<int32_t>(std::size(accumulate));
            accumulateCount = std::min(accumulateCount + 1, static_cast<int32_t>(std::size(accumulate)));
            average = accumulateTotal / static_cast<float_t>(accumulateCount);

            _pollEvents();

            uiRenderer->setCurrentContext();
            uiRenderer->setScreenSize(getUISize(getWindowSize()));

            update(elapsed);
            render();
        }
    }

    auto getAspectRatio() -> float_t {
        int32_t width;
        int32_t height;
        SDL_GetWindowSize(window, &width, &height);

        return static_cast<float_t>(width) / static_cast<float_t>(height);
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

public:
    virtual void update(float_t dt) {}
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

        swapchain.configure(config);
    }

protected:
    void _pollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    running = false;
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
    }

    auto _drawView(const sp<View>& view) {
        auto uiSize = getUISize(getWindowSize());
        auto childSize = view->_size(ProposedSize(uiSize));
        auto translate = view->translation(childSize, uiSize, Alignment::center());

        uiContext->saveState();
        uiContext->translateBy(translate.x, translate.y);
        view->_draw(uiContext, childSize);
        uiContext->restoreState();
    }

public:
    static auto getPerspectiveProjection(float_t fovy, float_t aspect, float_t zNear, float_t zFar) -> glm::mat4x4 {
        float_t range = tan(fovy * 0.5F);

        float_t x = +1.0F / (range * aspect);
        float_t y = -1.0F / (range);
        float_t z = zFar / (zFar - zNear);
        float_t a = 1.0F;
        float_t b = -(zFar * zNear) / (zFar - zNear);

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
//protected:
    bool running{};
    SDL_Window* window;

    float_t average = {};
    float_t accumulate[60] = {};
    float_t accumulateTotal = {};
    int32_t accumulateCount = {};
    int32_t accumulateIndex = {};

    vk::PhysicalDevice adapter;
    gfx::Device device;
    gfx::Instance instance;
    gfx::Surface surface;

    gfx::Swapchain swapchain;
    gfx::CommandQueue commandQueue;
    gfx::CommandBuffer commandBuffer;

    sp<UIContext> uiContext;
    sp<UIRenderer> uiRenderer;
};
