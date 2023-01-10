#include "Renderer.hpp"

struct Game : ApplicationDelegate {
private:
    void applicationDidFinishLaunching(const gfx::SharedPtr<Application>& sender) override {
        device = sender->context->mDevices.front();

        auto window = Window::alloc(800, 600);
        window->setTitle("Particles-04");
        window->setResizable(true);

        auto renderer = gfx::TransferPtr(new Renderer(device));
        renderer->screenResized(window->size());

        auto view = window->view();
        view->setDevice(device);
        view->setColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
        view->setPixelFormat(vk::Format::eB8G8R8A8Unorm);
        view->setDisplaySyncEnabled(true);
        view->setDelegate(renderer);
    }

private:
    gfx::SharedPtr<gfx::Device> device = {};
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    auto application = Application::sharedApplication();
    application->setDelegate(gfx::TransferPtr(new Game()));
    application->run();
    return 0;
}