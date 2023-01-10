#pragma once

#include "Window.hpp"
#include "gfx/Object.hpp"
#include "gfx/Context.hpp"
#include "gfx/Context.hpp"

struct Window;
struct Application;

struct ApplicationDelegate : gfx::Referencing {
    virtual void applicationDidFinishLaunching(const gfx::SharedPtr<Application>& sender) = 0;
};

struct Application final : gfx::Referencing {
    friend Window;

//private:
    bool mRunning = {};
    gfx::SharedPtr<gfx::Context> context = {};
    std::list<gfx::SharedPtr<Window>> mWindows = {};
    gfx::SharedPtr<ApplicationDelegate> mDelegate = {};

private:
    Application();
    ~Application() override;

public:
    void run();
    auto delegate() -> gfx::SharedPtr<ApplicationDelegate>;
    void setDelegate(gfx::SharedPtr<ApplicationDelegate> delegate);

private:
    void pollEvents();
    static auto _getWindowById(uint32_t id) -> Window*;

public:
    static auto sharedApplication() -> gfx::SharedPtr<Application>;
};