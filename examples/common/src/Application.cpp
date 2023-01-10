#include "Application.hpp"
#include "Window.hpp"

#include "gfx/Context.hpp"
#include "gfx/Device.hpp"
#include "gfx/View.hpp"

#include <SDL_events.h>

Application::Application() {
    context = TransferPtr(new gfx::Context());
}

Application::~Application() {
    for (auto& window : mWindows) {
        window->_destroy();
    }
}

void Application::run() {
    if (mDelegate) {
        mDelegate->applicationDidFinishLaunching(RetainPtr(this));
    }
    using seconds = std::chrono::duration<float_t, std::chrono::seconds::period>;
    auto previous = std::chrono::steady_clock::now();

    float_t accumulate = 0.0F;

    mRunning = true;
    while (mRunning) {
        auto current = std::chrono::steady_clock::now();
        auto elapsed = current - previous;
        previous = current;

        auto delta_time = seconds(elapsed).count();

        accumulate += delta_time;

        pollEvents();

        for (auto it = mWindows.begin(); it != mWindows.end();) {
            if ((*it)->mShouldClose) {
                (*it)->_destroy();
                it = mWindows.erase(it);
            } else {
                it++;
            }
        }

        for (auto& window : mWindows) {
            window->view()->update(delta_time);
        }

        for (auto& window : mWindows) {
            window->view()->draw();
        }
    }
}

auto Application::delegate() -> gfx::SharedPtr<ApplicationDelegate> {
    return mDelegate;
}

void Application::setDelegate(gfx::SharedPtr<ApplicationDelegate> delegate) {
    mDelegate = std::move(delegate);
}

auto Application::_getWindowById(uint32_t id) -> Window* {
    return static_cast<Window*>(SDL_GetWindowData(SDL_GetWindowFromID(id), "this"));
}

void Application::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                mRunning = false;
                break;
            }
            case SDL_WINDOWEVENT: {
                auto sender = _getWindowById(event.window.windowID);
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE: {
                        sender->performClose();
                        break;
                    }
                    case SDL_WINDOWEVENT_RESIZED: {
                        sender->performResize();
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if (auto sender = _getWindowById(event.key.windowID)) {
                    sender->view()->keyUp(&event.key);
                }
                break;
            }
            case SDL_KEYDOWN: {
                if (auto sender = _getWindowById(event.key.windowID)) {
                    sender->view()->keyDown(&event.key);
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (auto sender = _getWindowById(event.button.windowID)) {
                    sender->view()->mouseUp(&event.button);
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (auto sender = _getWindowById(event.button.windowID)) {
                    sender->view()->mouseDown(&event.button);
                }
                break;
            }
            case SDL_MOUSEWHEEL: {
                if (auto sender = _getWindowById(event.wheel.windowID)) {
                    sender->view()->mouseWheel(&event.wheel);
                }
                break;
            }
            default: {
                break;
            }
        }
    }
}

auto Application::sharedApplication() -> gfx::SharedPtr<Application> {
    static auto application = TransferPtr(new Application());
    return application;
}