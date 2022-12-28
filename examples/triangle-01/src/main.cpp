#define VMA_IMPLEMENTATION

#include "Renderer.hpp"

#include "GLFW/glfw3.h"

struct Game : gfx::Referencing<Game> {
public:
    Game() {
        glfwInit();
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        mApplication = gfx::Application::alloc();
        mDevice = mApplication->devices().front();
        mWindow = gfx::Window::alloc(mApplication, 800, 600);
        mSwapchain = gfx::Swapchain::alloc(mDevice);
        mSwapchain->setSurface(mWindow->surface());
        mSwapchain->setColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
        mSwapchain->setPixelFormat(vk::Format::eB8G8R8A8Unorm);

        mRenderer = gfx::TransferPtr(new Renderer(mDevice));
    }

    ~Game() override {
        glfwTerminate();
    }

public:
    void run() {
        while (!mWindow->shouldClose()) {
            glfwPollEvents();

            mRenderer->draw(mSwapchain);
        }
        mDevice->waitIdle();
    }

private:
    gfx::SharedPtr<gfx::Application> mApplication;

    gfx::SharedPtr<Renderer> mRenderer;
    gfx::SharedPtr<gfx::Window> mWindow;
    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Swapchain> mSwapchain;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    auto game = gfx::TransferPtr(new Game());
    game->run();

    return 0;
}