#include "window.hpp"
#include "context.hpp"
#include "swapchain.hpp"

#include "GLFW/glfw3.h"

vfx::Application::Application() {
    glfwInit();
}

vfx::Application::~Application() {
    glfwTerminate();
}

void vfx::Application::pollEvents() {
    glfwPollEvents();
}

vfx::Window::Window(const WindowDescription& description) {
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, description.resizable ? GLFW_TRUE : GLFW_FALSE);

    handle = glfwCreateWindow(i32(description.width), i32(description.height), description.title.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(handle, this);
    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self->handle != window) {
            return;
        }
        self->windowDidResize();
    });
}

vfx::Window::~Window() {
    glfwDestroyWindow(handle);
}

auto vfx::Window::createSurface(const Arc<Context>& context) -> vk::SurfaceKHR {
    VkSurfaceKHR surface{};
    glfwCreateWindowSurface(context->instance, handle, nullptr, &surface);
    return surface;
}

auto vfx::Window::getHandle() -> GLFWwindow* {
    return handle;
}

auto vfx::Window::windowShouldClose() -> bool {
    return glfwWindowShouldClose(handle);
}

void vfx::Window::windowDidResize() {
    if (delegate) {
        delegate->windowDidResize();
    }
}