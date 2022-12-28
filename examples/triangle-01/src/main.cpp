#define VMA_IMPLEMENTATION

#include "Renderer.hpp"

#include "GLFW/glfw3.h"

struct Game : gfx::WindowDelegate {
public:
    Game() {
        mApplication = gfx::Application::alloc();
        mDevice = mApplication->devices().front();
        mWindow = gfx::Window::alloc(mApplication, 800, 600);
        mWindow->setTitle("Triangle-01");
        mWindow->setDelegate(gfx::RetainPtr(this));
        mSwapchain = gfx::Swapchain::alloc(mDevice);
        mSwapchain->setSurface(mWindow->surface());
        mSwapchain->setDrawableSize(mWindow->size());
        mSwapchain->setColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
        mSwapchain->setPixelFormat(vk::Format::eB8G8R8A8Unorm);

        mRenderer = gfx::TransferPtr(new Renderer(mDevice));
    }

public:
    void run() {
        while (!mWindow->shouldClose()) {
            glfwPollEvents();

            mRenderer->draw(mSwapchain);
        }
    }

public:
    void windowDidResize() override {
        mDevice->waitIdle();

        mSwapchain->setDrawableSize(mWindow->size());
        mSwapchain->releaseDrawables();
    }

private:
    gfx::SharedPtr<Renderer> mRenderer;
    gfx::SharedPtr<gfx::Window> mWindow;
    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Swapchain> mSwapchain;
    gfx::SharedPtr<gfx::Application> mApplication;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);
    auto game = gfx::TransferPtr(new Game());
    game->run();
    return 0;
}