#pragma once

#include "Core.hpp"

struct WindowDelegate {
    virtual ~WindowDelegate() = default;

    virtual void windowDidResize() {}
    virtual void windowShouldClose() {}
};

struct GLFWwindow;
struct Window {
public:
    GLFWwindow* handle = {};
    WindowDelegate* delegate = {};

public:
    Window(u32 width, u32 height);
    ~Window();

public:
    void setTitle(const std::string& title);
    void setResizable(bool resizable);

    auto shouldClose() -> bool;

    auto makeSurface(const Arc<vfx::Context>& context) -> Arc<vfx::Surface>;

private:
    void windowDidResize();
    void windowShouldClose();
};

struct Application {
protected:
    Application();
    ~Application();

protected:
    static void pollEvents();
};