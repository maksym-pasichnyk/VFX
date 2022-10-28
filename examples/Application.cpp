#include "Application.hpp"
#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"

Application::Application() {
    glfwInit();
}

Application::~Application() {
    glfwTerminate();
}

void Application::pollEvents() {
    glfwPollEvents();
}

Window::Window(u32 width, u32 height) {
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    handle = glfwCreateWindow(i32(width), i32(height), "", nullptr, nullptr);

    glfwSetWindowUserPointer(handle, this);
    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, i32 width, i32 height) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        self->windowDidResize();
    });

    glfwSetWindowCloseCallback(handle, [](GLFWwindow* window) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        self->windowShouldClose();
    });

    glfwSetKeyCallback(handle, [](GLFWwindow* window, i32 keycode, i32 scancode, i32 action, i32 mods) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        self->windowKeyEvent(keycode, scancode, action, mods);
    });

    glfwSetMouseButtonCallback(handle, [](GLFWwindow* window, i32 button, i32 action, i32 mods) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        self->windowMouseEvent(button, action, mods);
    });

    glfwSetCursorPosCallback(handle, [](GLFWwindow* window, f64 x, f64 y) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        self->windowCursorEvent(x, y);
    });

    glfwSetCursorEnterCallback(handle, [](GLFWwindow* window, i32 entered) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        if (entered == GLFW_FALSE) {
            self->windowMouseEnter();
        } else {
            self->windowMouseExit();
        }
    });
}

Window::~Window() {
    glfwDestroyWindow(handle);
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(handle, title.c_str());
}

void Window::setResizable(bool resizable) {
    glfwSetWindowAttrib(handle, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
}

auto Window::shouldClose() -> bool {
    return glfwWindowShouldClose(handle) != GLFW_FALSE;
}

auto Window::makeSurface(const Arc<vfx::Context>& context) -> vk::UniqueSurfaceKHR {
    VkSurfaceKHR surface{};
    glfwCreateWindowSurface(*context->instance, handle, nullptr, &surface);
    return vk::UniqueSurfaceKHR(surface, vk::UniqueSurfaceKHR::ObjectDestroy(*context->instance, VK_NULL_HANDLE, context->interface));
}

auto Window::getSize() const -> std::array<i32, 2> {
    std::array<i32, 2> out{};
    glfwGetWindowSize(handle, &out[0], &out[1]);
    return out;
}

void Window::windowDidResize() {
    if (delegate) {
        delegate->windowDidResize();
    }
}

void Window::windowShouldClose() {
    if (delegate) {
        delegate->windowShouldClose();
    }
}

void Window::windowKeyEvent(i32 keycode, i32 scancode, i32 action, i32 mods) {
    if (delegate) {
        delegate->windowKeyEvent(keycode, scancode, action, mods);
    }
}

void Window::windowMouseEvent(i32 button, i32 action, i32 mods) {
    if (delegate) {
        delegate->windowMouseEvent(button, action, mods);
    }
}

void Window::windowCursorEvent(f64 x, f64 y) {
    if (delegate) {
        delegate->windowCursorEvent(x, y);
    }
}

void Window::windowMouseEnter() {
    if (delegate) {
        delegate->windowMouseEnter();
    }
}

void Window::windowMouseExit() {
    if (delegate) {
        delegate->windowMouseExit();
    }
}


