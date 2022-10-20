#include "Application.hpp"
#include "spdlog/spdlog.h"
#include "GLFW/glfw3.h"

#include "context.hpp"



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
//    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

    handle = glfwCreateWindow(i32(width), i32(height), "", nullptr, nullptr);

    glfwSetWindowUserPointer(handle, this);
    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        if (self->delegate) {
            self->delegate->windowDidResize();
        }
    });

    glfwSetWindowCloseCallback(handle, [](GLFWwindow* window) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        if (self->delegate) {
            self->delegate->windowShouldClose();
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

auto Window::makeSurface(const Arc<vfx::Context>& context) -> Arc<vfx::Surface> {
    VkSurfaceKHR surface{};
    glfwCreateWindowSurface(*context->instance, handle, nullptr, &surface);

    auto out = Arc<vfx::Surface>::alloc();
    out->context = &*context;
    out->handle = surface;
    return out;
}
