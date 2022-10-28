#include "Time.hpp"
#include "Math.hpp"
#include "Renderer.hpp"
#include "Application.hpp"

#include "imgui.h"
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

struct DemoApplication : Application, WindowDelegate {
private:
    Arc<Window> window{};

    Arc<vfx::Context> context{};
    Arc<vfx::Device> device{};
    Arc<vfx::Layer> layer{};

    Arc<Renderer> renderer{};

    i32 moveForward  = 0;
    i32 moveBackward = 0;
    i32 moveLeft     = 0;
    i32 moveRight    = 0;

    bool mouseGrabbed = false;
    bool ignoreFirstMove = true;

    glm::dvec2 delta = {};
    glm::dvec2 cursor = {};
    glm::vec3 cameraPosition = {};
    glm::vec3 cameraRotation = {};

public:
    DemoApplication() {
        window = Arc<Window>::alloc(800, 600);
        window->setTitle("Demo");
        window->delegate = this;

        setenv("VFX_ENABLE_API_VALIDATION", "1", 1);
        context = Arc<vfx::Context>::alloc();
        device = Arc<vfx::Device>::alloc(context);

        layer = Arc<vfx::Layer>::alloc(device);
        layer->surface = window->makeSurface(context);
        layer->colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        layer->pixelFormat = vk::Format::eB8G8R8A8Unorm;
        layer->displaySyncEnabled = true;
        layer->updateDrawables();

        renderer = Arc<Renderer>::alloc(device, layer, window);
    }

    ~DemoApplication() override {
        device->waitIdle();
    }

    void run() {
        grabMouse();

        cameraPosition = glm::vec3(0, 0, 10);
        cameraRotation = glm::vec3(0, 180, 0);

        f32 timeSinceStart = 0.0f;

        f64 time = glfwGetTime();
        while (!window->shouldClose()) {
            f64 now = glfwGetTime();
            f32 dt = f32(now - time);
            time = now;

            timeSinceStart += dt;

            pollEvents();

            Time::deltaTime = dt;
            Time::timeSinceStart = timeSinceStart;

            update();
            draw();
        }
    }

public:
    void update() {
        rotatePlayer();
        movePlayer();
    }

    void draw() {
        renderer->update();
        renderer->draw();
    }

private:
    void windowDidResize() override {
        ignoreFirstMove = true;

        device->waitIdle();
        layer->updateDrawables();

        renderer->resize();
        renderer->draw();
    }

    void windowShouldClose() override {}

    void windowKeyEvent(i32 keycode, i32 scancode, i32 action, i32 mods) override {
        if (keycode == GLFW_KEY_W) {
            switch (action) {
                case GLFW_PRESS:
                    moveForward = 1;
                    break;
                case GLFW_RELEASE:
                    moveForward = 0;
                    break;
                default:
                    break;
            }
        }
        if (keycode == GLFW_KEY_S) {
            switch (action) {
                case GLFW_PRESS:
                    moveBackward = 1;
                    break;
                case GLFW_RELEASE:
                    moveBackward = 0;
                    break;
                default:
                    break;
            }
        }
        if (keycode == GLFW_KEY_A) {
            switch (action) {
                case GLFW_PRESS:
                    moveLeft = 1;
                    break;
                case GLFW_RELEASE:
                    moveLeft = 0;
                    break;
                default:
                    break;
            }
        }
        if (keycode == GLFW_KEY_D) {
            switch (action) {
                case GLFW_PRESS:
                    moveRight = 1;
                    break;
                case GLFW_RELEASE:
                    moveRight = 0;
                    break;
                default:
                    break;
            }
        }

        if (keycode == GLFW_KEY_ESCAPE) {
            if (action == GLFW_PRESS) {
                releaseMouse();
            }
        }
    }

    void windowMouseEvent(i32 button, i32 action, i32 mods) override {
        if (ImGui::GetIO().WantCaptureMouse) {
            return;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                grabMouse();
            }
        }
    }

    void windowCursorEvent(f64 x, f64 y) override {
        if (ignoreFirstMove) {
            ignoreFirstMove = false;
            cursor = glm::dvec2(x, y);
        }
        if (mouseGrabbed) {
            delta += glm::dvec2(x, y) - cursor;
        }
        cursor = glm::vec2(x, y);
    }

    void windowMouseEnter() override {
        ignoreFirstMove = true;
    }

private:
    void grabMouse() {
        if (mouseGrabbed) {
            return;
        }

        glfwPollEvents();

        auto [sx, sy] = window->getSize();
        cursor = glm::dvec2(glm::ivec2(sx, sy) / 2);
        mouseGrabbed = true;
        ignoreFirstMove = true;
        glfwGetCursorPos(window->handle, &cursor.x, &cursor.y);
        glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void releaseMouse() {
        if (!mouseGrabbed) {
            return;
        }
        glfwPollEvents();

        auto [sx, sy] = window->getSize();
        cursor = glm::dvec2(glm::ivec2(sx, sy) / 2);
        mouseGrabbed = false;
        glfwSetCursorPos(window->handle, cursor.x, cursor.y);
        glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void movePlayer() {
        if (!mouseGrabbed) {
            return;
        }
        
        glm::ivec3 direction = glm::ivec3(
            moveRight - moveLeft,
            0,
            moveForward - moveBackward
        );

        if (direction != glm::ivec3()) {
            auto orientation = glm::mat3x3(glm::quat(glm::radians(cameraRotation)));
            auto velocity = glm::normalize(orientation * glm::vec3(direction)) * 10.0f;

            cameraPosition += velocity * Time::deltaTime;
        }

        renderer->setPositionAndRotation(cameraPosition, cameraRotation);
    }

    void rotatePlayer() {
        if (!mouseGrabbed) {
            return;
        }
        f64 d4 = 0.5 * 0.6 + 0.2;
        f64 d5 = d4 * d4 * d4 * 8.0;

        cameraRotation.x += f32(delta.y * d5) * Time::deltaTime;
        cameraRotation.y += f32(delta.x * d5) * Time::deltaTime;

        cameraRotation.x = glm::clamp(cameraRotation.x, -90.0f, 90.0f);

        delta = {};
    }
};

auto main() -> i32 {
    try {
        DemoApplication demo{};
        demo.run();
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
