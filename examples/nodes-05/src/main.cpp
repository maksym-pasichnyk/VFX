#define VMA_IMPLEMENTATION

#include "Renderer.hpp"
#include "SDL_events.h"

#include <chrono>

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
        window->setTitle("Nodes-05");
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
        using seconds = std::chrono::duration<float_t, std::chrono::seconds::period>;

        auto previous = std::chrono::steady_clock::now();

        ImGui::SetCurrentContext(&im_gui_context);

        float_t accumulate = 0.0F;
        float_t fixed_delta_time = 1.0F / 60.0F;

        running = true;
        while (running) {
            auto current = std::chrono::steady_clock::now();
            auto elapsed = current - previous;
            previous = current;

            accumulate += seconds(elapsed).count();

            pollEvents();

            // fixed update
            im_gui_context.IO.DeltaTime = fixed_delta_time;
            while (accumulate >= fixed_delta_time) {
                accumulate -= fixed_delta_time;
                renderer->update();
            }

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
                case SDL_KEYUP:
                    keyUp(&event.key);
                    break;
                case SDL_KEYDOWN:
                    keyDown(&event.key);
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouseUp(&event.button);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    mouseDown(&event.button);
                    break;
                case SDL_MOUSEWHEEL:
                    mouseWheel(&event.wheel);
                    break;
            }
        }
    }

private:
    void keyUp(SDL_KeyboardEvent* event) {
        renderer->keyUp(event);
    }

    void keyDown(SDL_KeyboardEvent* event) {
        renderer->keyDown(event);
    }

    void mouseUp(SDL_MouseButtonEvent* event) {
        renderer->mouseUp(event);
    }

    void mouseDown(SDL_MouseButtonEvent* event) {
        renderer->mouseDown(event);
    }

    void mouseWheel(SDL_MouseWheelEvent* event) {
        renderer->mouseWheel(event);
    }

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

    ImFontAtlas im_font_atlas = {};
    ImGuiContext im_gui_context = {&im_font_atlas};
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);
    auto game = gfx::TransferPtr(new Game());
    game->run();
    return 0;
}