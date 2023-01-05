#define VMA_IMPLEMENTATION

#include "Renderer.hpp"

#include "SDL_events.h"
#include "spdlog/spdlog.h"

struct Game : gfx::Referencing {
private:
    struct WindowDelegate : gfx::WindowDelegate {
    private:
        Game* pGame;

    public:
        explicit WindowDelegate(Game* pGame) : pGame(pGame) {}

        void windowDidResize(const gfx::SharedPtr<gfx::Window> &sender) override {
            pGame->screenResized(sender->size());
        }
    };

public:
    Game() {
        application = gfx::Application::alloc();
        device = application->devices().front();

        window = gfx::Window::alloc(application, 800, 600);
        window->setTitle("Circles-02");
        window->setResizable(true);
        window->setDelegate(gfx::TransferPtr(new WindowDelegate(this)));

        swapchain = window->swapchain();
        swapchain->setDevice(device);
        swapchain->setColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
        swapchain->setPixelFormat(vk::Format::eB8G8R8A8Unorm);
        swapchain->setDisplaySyncEnabled(true);

        renderer = gfx::TransferPtr(new Renderer(device));
        renderer->screenResized(window->size());
    }

public:
    void run() {
        running = true;
        while (running) {
            pollEvents();

            renderer->draw(swapchain);
        }
    }

    void pollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_WINDOWEVENT: {
                    auto pSDLWindow = SDL_GetWindowFromID(event.window.windowID);
                    auto pGFXWindow = static_cast<gfx::Window*>(SDL_GetWindowData(pSDLWindow, "this"));
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                            pGFXWindow->performClose();
                            break;
                        case SDL_WINDOWEVENT_RESIZED:
                            pGFXWindow->performResize();
                            break;
                        default:
                            break;
                    }
                    break;
                }
            }
        }
    }

private:
    void screenResized(const vk::Extent2D& size) {
        renderer->screenResized(size);
    }

private:
    bool running = {};

    gfx::SharedPtr<Renderer> renderer;
    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::Window> window;
    gfx::SharedPtr<gfx::Swapchain> swapchain;
    gfx::SharedPtr<gfx::Application> application;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);
    auto game = gfx::TransferPtr(new Game());
    game->run();
    return 0;
}