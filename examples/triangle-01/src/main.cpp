#include "Renderer.hpp"

struct Game : gfx::ApplicationDelegate {
private:
    void applicationDidFinishLaunching(const gfx::SharedPtr<gfx::Application>& sender) override {
        device = sender->devices().front();

        auto window = gfx::Window::alloc(800, 600);
        window->setTitle("Triangle-01");
        window->setResizable(true);

        auto swapchain = window->swapchain();
        swapchain->setDevice(device);
        swapchain->setColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
        swapchain->setPixelFormat(vk::Format::eB8G8R8A8Unorm);
        swapchain->setDisplaySyncEnabled(true);

        auto renderer = gfx::TransferPtr(new Renderer(device));
        renderer->screenResized(window->size());

        window->view()->setDelegate(renderer);
    }

private:
    gfx::SharedPtr<gfx::Device> device = {};
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    auto application = gfx::Application::sharedApplication();
    application->setDelegate(gfx::TransferPtr(new Game()));
    application->run();
    return 0;
}