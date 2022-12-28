#include "Window.hpp"
#include "Application.hpp"

#include "GLFW/glfw3.h"

gfx::Window::Window(SharedPtr<Application> application, int32_t width, int32_t height) : mApplication(std::move(application)) {
    pWindow = glfwCreateWindow(width, height, "", nullptr, nullptr);
    glfwCreateWindowSurface(mApplication->vkInstance, static_cast<GLFWwindow*>(pWindow), nullptr, reinterpret_cast<VkSurfaceKHR*>(&vkSurface));
}

gfx::Window::~Window() {
    mApplication->vkInstance.destroySurfaceKHR(vkSurface, nullptr, mApplication->vkDispatchLoaderDynamic);
    glfwDestroyWindow(static_cast<GLFWwindow*>(pWindow));
}

auto gfx::Window::shouldClose() -> bool {
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(pWindow)) != GLFW_FALSE;
}

auto gfx::Window::surface() -> vk::SurfaceKHR {
    return vkSurface;
}


auto gfx::Window::alloc(SharedPtr<Application> application, int32_t width, int32_t height) -> SharedPtr<Window> {
    return TransferPtr(new Window(std::move(application), width, height));
}