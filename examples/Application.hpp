#pragma once

#include "Core.hpp"

struct WindowDelegate {
    virtual ~WindowDelegate() = default;

    virtual void windowDidResize() {}
    virtual void windowShouldClose() {}
    virtual void windowKeyEvent(i32 keycode, i32 scancode, i32 action, i32 mods) {}
    virtual void windowMouseEvent(i32 button, i32 action, i32 mods) {}
    virtual void windowCursorEvent(f64 x, f64 y) {};
    virtual void windowMouseEnter() {}
    virtual void windowMouseExit() {}
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
    auto makeSurface(const Arc<vfx::Context>& context) -> vk::UniqueSurfaceKHR;

    auto getSize() const -> std::array<i32, 2>;

private:
    void windowDidResize();
    void windowShouldClose();
    void windowKeyEvent(i32 keycode, i32 scancode, i32 action, i32 mods);
    void windowMouseEvent(i32 button, i32 action, i32 mods);
    void windowCursorEvent(f64 x, f64 y);
    void windowMouseEnter();
    void windowMouseExit();
};

struct Application {
protected:
    Application();
    ~Application();

protected:
    static void pollEvents();
};