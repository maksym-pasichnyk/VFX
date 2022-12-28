#include "Window.hpp"
#include "Application.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

gfx::Window::Window(SharedPtr<Application> application, int32_t width, int32_t height) : mApplication(std::move(application)) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(width, height, "", nullptr, nullptr);
    glfwCreateWindowSurface(mApplication->vkInstance, static_cast<GLFWwindow*>(pWindow), nullptr, reinterpret_cast<VkSurfaceKHR*>(&vkSurface));

    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(pWindow), this);
    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(pWindow), [](GLFWwindow* window, int32_t width, int32_t height) {
        auto thiz = static_cast<Window*>(glfwGetWindowUserPointer(window));
        thiz->windowDidResize();
    });
    glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(pWindow), [](GLFWwindow* window) {
        auto thiz = static_cast<Window*>(glfwGetWindowUserPointer(window));
        thiz->windowShouldClose();
    });
    glfwSetKeyCallback(static_cast<GLFWwindow*>(pWindow), [](GLFWwindow* window, int32_t keycode, int32_t scancode, int32_t action, int32_t mods) {
        auto thiz = static_cast<Window*>(glfwGetWindowUserPointer(window));
        thiz->windowKeyEvent(keycode, scancode, action, mods);
    });
    glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(pWindow), [](GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
        auto thiz = static_cast<Window*>(glfwGetWindowUserPointer(window));
        thiz->windowMouseEvent(button, action, mods);
    });
    glfwSetCursorPosCallback(static_cast<GLFWwindow*>(pWindow), [](GLFWwindow* window, double_t x, double_t y) {
        auto thiz = static_cast<Window*>(glfwGetWindowUserPointer(window));
        thiz->windowCursorEvent(x, y);
    });
    glfwSetCursorEnterCallback(static_cast<GLFWwindow*>(pWindow), [](GLFWwindow* window, int32_t entered) {
        auto thiz = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (entered == GLFW_FALSE) {
            thiz->windowMouseEnter();
        } else {
            thiz->windowMouseExit();
        }
    });
}

gfx::Window::~Window() {
    mApplication->vkInstance.destroySurfaceKHR(vkSurface, nullptr, mApplication->vkDispatchLoaderDynamic);
    glfwDestroyWindow(static_cast<GLFWwindow*>(pWindow));
    glfwTerminate();
}

void gfx::Window::close() {
    glfwSetWindowShouldClose(static_cast<GLFWwindow*>(pWindow), GLFW_TRUE);
}

auto gfx::Window::shouldClose() -> bool {
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(pWindow)) != GLFW_FALSE;
}

auto gfx::Window::size() -> vk::Extent2D {
    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

auto gfx::Window::surface() -> vk::SurfaceKHR {
    return vkSurface;
}

void gfx::Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(static_cast<GLFWwindow*>(pWindow), title.c_str());
}

void gfx::Window::setDelegate(SharedPtr<WindowDelegate> delegate) {
    mDelegate = std::move(delegate);
}

void gfx::Window::setResizable(bool resizable) {
    glfwSetWindowAttrib(static_cast<GLFWwindow*>(pWindow), GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
}

void gfx::Window::windowDidResize() {
    if (mDelegate) {
        mDelegate->windowDidResize();
    }
}
void gfx::Window::windowShouldClose() {
    if (mDelegate) {
        mDelegate->windowShouldClose();
    }
}
void gfx::Window::windowKeyEvent(int32_t keycode, int32_t scancode, int32_t action, int32_t mods) {
    if (mDelegate) {
        mDelegate->windowKeyEvent(keycode, scancode, action, mods);
    }
}
void gfx::Window::windowMouseEvent(int32_t button, int32_t action, int32_t mods) {
    if (mDelegate) {
        mDelegate->windowMouseEvent(button, action, mods);
    }
}
void gfx::Window::windowCursorEvent(double_t x, double_t y) {
    if (mDelegate) {
        mDelegate->windowCursorEvent(x, y);
    }
}
void gfx::Window::windowMouseEnter() {
    if (mDelegate) {
        mDelegate->windowMouseEnter();
    }
}
void gfx::Window::windowMouseExit() {
    if (mDelegate) {
        mDelegate->windowMouseExit();
    }
}

auto gfx::Window::alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window> {
    return TransferPtr(new Window(std::move(application), width, height));
}