#include "Time.hpp"
#include "Renderer.hpp"
#include "Application.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

struct DemoApplication : Application, WindowDelegate {
private:
    Arc<Window> window{};

    Arc<vfx::Context> context{};
    Arc<vfx::Device> device{};
    Arc<vfx::Layer> layer{};

    Arc<Renderer> renderer{};

public:
    DemoApplication() {
        window = Arc<Window>::alloc(800, 600);
        window->setTitle("Demo");
        window->delegate = this;

        setenv("VFX_ENABLE_API_VALIDATION", "1", 1);
        context = Arc<vfx::Context>::alloc();
        device = Arc<vfx::Device>::alloc(context);

        layer = Arc<vfx::Layer>::alloc(device);
        layer->surface = window->makeSurface(context);
        layer->colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        layer->pixelFormat = vk::Format::eB8G8R8A8Unorm;
        layer->displaySyncEnabled = true;
        layer->updateDrawables();

        renderer = Arc<Renderer>::alloc(device, layer, window);
    }

    ~DemoApplication() override {
        device->handle->waitIdle();
    }

    void run() {
        f32 timeSinceStart = 0.0f;

        f64 time = glfwGetTime();
        while (!window->shouldClose()) {
            f64 now = glfwGetTime();
            f32 dt = f32(now - time);
            time = now;

            timeSinceStart += dt;

            pollEvents();

            Time::deltaTime = dt;
            Time::timeSinceStart = timeSinceStart;

            renderer->update();
            renderer->draw();
        }
    }

public:
    void windowDidResize() override {
        device->handle->waitIdle();
        layer->updateDrawables();

        renderer->resize();
        renderer->draw();
    }

    void windowShouldClose() override {}
};

auto main() -> i32 {
    try {
        DemoApplication demo{};
        demo.run();
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
