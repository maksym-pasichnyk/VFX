#include "window.hpp"
#include "context.hpp"
#include "swapchain.hpp"

#include "GLFW/glfw3.h"

vfx::WindowController::WindowController() {
    glfwInit();
}

vfx::WindowController::~WindowController() {
    glfwTerminate();
}

void vfx::WindowController::pollEvents() {
    glfwPollEvents();
}

vfx::Window::Window(Arc<WindowController> _controller, Context& context, const WindowDescription& description) : context(context) {
    controller = std::move(_controller);

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, description.resizable ? GLFW_TRUE : GLFW_FALSE);

    handle = glfwCreateWindow(i32(description.width), i32(description.height), description.title.c_str(), nullptr, nullptr);

    glfwCreateWindowSurface(context.instance, handle, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));

    swapchain = Arc<vfx::Swapchain>::alloc(context, surface);
}

vfx::Window::~Window() {
    swapchain = {};

    context.instance.destroySurfaceKHR(surface);
    glfwDestroyWindow(handle);
}

auto vfx::Window::getHandle() -> GLFWwindow* {
    return handle;
}

auto vfx::Window::getSwapchain() -> Arc<Swapchain> {
    return swapchain;
}

auto vfx::Window::windowShouldClose() -> bool {
    return glfwWindowShouldClose(handle);
}