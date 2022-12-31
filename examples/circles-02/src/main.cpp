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
        mApplication = gfx::Application::alloc();
        mDevice = mApplication->devices().front();

        mWindow = gfx::Window::alloc(mApplication, 800, 600);
        mWindow->setTitle("Circles-02");
        mWindow->setResizable(true);
        mWindow->setDelegate(gfx::TransferPtr(new WindowDelegate(this)));

        mSwapchain = mWindow->swapchain();
        mSwapchain->setDevice(mDevice);
        mSwapchain->setColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
        mSwapchain->setPixelFormat(vk::Format::eB8G8R8A8Unorm);
        mSwapchain->setDisplaySyncEnabled(true);

        mRenderer = gfx::TransferPtr(new Renderer(mDevice));
        mRenderer->setScreenSize(mWindow->size());
    }

public:
    void run() {
        mApplicationRunning = true;
        while (mApplicationRunning) {
            pollEvents();

            mRenderer->draw(mSwapchain);
        }
    }

    void pollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    mApplicationRunning = false;
                    break;
                case SDL_WINDOWEVENT: {
                    auto pSDLWindow = SDL_GetWindowFromID(event.window.windowID);
                    auto pGFXWindow = static_cast<gfx::Window*>(SDL_GetWindowData(pSDLWindow, "this"));
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        pGFXWindow->windowDidResize();
                    } else if (event.window.event == SDL_WINDOWEVENT_ENTER) {
                        pGFXWindow->windowMouseEnter();
                    } else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
                        pGFXWindow->windowMouseExit();
                    } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        pGFXWindow->windowShouldClose();
                    }
                    break;
                }
            }
        }
    }

private:
    void screenResized(const vk::Extent2D& size) {
        mRenderer->setScreenSize(size);
    }

private:
    bool mApplicationRunning = {};

    gfx::SharedPtr<Renderer> mRenderer;
    gfx::SharedPtr<gfx::Device> mDevice;
    gfx::SharedPtr<gfx::Window> mWindow;
    gfx::SharedPtr<gfx::Swapchain> mSwapchain;
    gfx::SharedPtr<gfx::Application> mApplication;
};

auto main() -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);
    auto game = gfx::TransferPtr(new Game());
    game->run();
    return 0;
}