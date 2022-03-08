#pragma once

#include <vector>
#include <types.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

enum class KeyCode {
    /* Printable keys */
    eSpace,
    eApostrophe,
    eComma,
    eMinus,
    ePeriod,
    eSlash,
    e0,
    e1,
    e2,
    e3,
    e4,
    e5,
    e6,
    e7,
    e8,
    e9,
    eSemicolon,
    eEqual,
    eA,
    eB,
    eC,
    eD,
    eE,
    eF,
    eG,
    eH,
    eI,
    eJ,
    eK,
    eL,
    eM,
    eN,
    eO,
    eP,
    eQ,
    eR,
    eS,
    eT,
    eU,
    eV,
    eW,
    eX,
    eY,
    eZ,
    eLelfBracket,
    eBackslash,
    eRightBracket,
    eGraceAccent,
    eWorld1,
    eWorld2,
    /* Function keys */
    eEscape,
    eEnter,
    eTab,
    eBackspace,
    eInsert,
    eDelete,
    eRight,
    eLeft,
    eDown,
    eUp,
    ePageUp,
    ePageDown,
    eHome,
    eEnd,
    eCapsLock,
    eScrollLock,
    eNumLock,
    ePrintScreen,
    ePause,
    eF1,
    eF2,
    eF3,
    eF4,
    eF5,
    eF6,
    eF7,
    eF8,
    eF9,
    eF10,
    eF11,
    eF12,
    eF13,
    eF14,
    eF15,
    eF16,
    eF17,
    eF18,
    eF19,
    eF20,
    eF21,
    eF22,
    eF23,
    eF24,
    eF25,
    eKeyPad0,
    eKeyPad1,
    eKeyPad2,
    eKeyPad3,
    eKeyPad4,
    eKeyPad5,
    eKeyPad6,
    eKeyPad7,
    eKeyPad8,
    eKeyPad9,
    eKeyPadDecimal,
    eKeyPadDivide,
    eKeyPadMultiply,
    eKeyPadSubstract,
    eKeyPadAdd,
    eKeyPadEnter,
    eKeyPadEqual,
    eLeftShift,
    eLeftControl,
    eLeftAlt,
    eLeftSuper,
    eRightShift,
    eRightControl,
    eRightAlt,
    eRightSuper,
    eMenu
};

enum class MouseButton {
    Left,
    Right,
    Middle
};

struct Display {
    GLFWwindow* window;
    std::array<int, 5> mouse_buttons;
    std::array<int, 128> key_codes;

    Display(i32 width, i32 height, const char* title, bool resizable) {
        glfwInit();
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        mouse_buttons.fill(GLFW_KEY_UNKNOWN);
        mouse_buttons[size_t(MouseButton::Left)] = GLFW_MOUSE_BUTTON_LEFT;
        mouse_buttons[size_t(MouseButton::Right)] = GLFW_MOUSE_BUTTON_RIGHT;
        mouse_buttons[size_t(MouseButton::Middle)] = GLFW_MOUSE_BUTTON_MIDDLE;

        key_codes.fill(GLFW_KEY_UNKNOWN);
        key_codes[size_t(KeyCode::eSpace)] = GLFW_KEY_SPACE;
        key_codes[size_t(KeyCode::eApostrophe)] = GLFW_KEY_APOSTROPHE;
        key_codes[size_t(KeyCode::eComma)] = GLFW_KEY_COMMA;
        key_codes[size_t(KeyCode::eMinus)] = GLFW_KEY_MINUS;
        key_codes[size_t(KeyCode::ePeriod)] = GLFW_KEY_PERIOD;
        key_codes[size_t(KeyCode::eSlash)] = GLFW_KEY_SLASH;
        key_codes[size_t(KeyCode::e0)] = GLFW_KEY_0;
        key_codes[size_t(KeyCode::e1)] = GLFW_KEY_1;
        key_codes[size_t(KeyCode::e2)] = GLFW_KEY_2;
        key_codes[size_t(KeyCode::e3)] = GLFW_KEY_3;
        key_codes[size_t(KeyCode::e4)] = GLFW_KEY_4;
        key_codes[size_t(KeyCode::e5)] = GLFW_KEY_5;
        key_codes[size_t(KeyCode::e6)] = GLFW_KEY_6;
        key_codes[size_t(KeyCode::e7)] = GLFW_KEY_7;
        key_codes[size_t(KeyCode::e8)] = GLFW_KEY_8;
        key_codes[size_t(KeyCode::e9)] = GLFW_KEY_9;
        key_codes[size_t(KeyCode::eSemicolon)] = GLFW_KEY_SEMICOLON;
        key_codes[size_t(KeyCode::eEqual)] = GLFW_KEY_EQUAL;
        key_codes[size_t(KeyCode::eA)] = GLFW_KEY_A;
        key_codes[size_t(KeyCode::eB)] = GLFW_KEY_B;
        key_codes[size_t(KeyCode::eC)] = GLFW_KEY_C;
        key_codes[size_t(KeyCode::eD)] = GLFW_KEY_D;
        key_codes[size_t(KeyCode::eE)] = GLFW_KEY_E;
        key_codes[size_t(KeyCode::eF)] = GLFW_KEY_F;
        key_codes[size_t(KeyCode::eG)] = GLFW_KEY_G;
        key_codes[size_t(KeyCode::eH)] = GLFW_KEY_H;
        key_codes[size_t(KeyCode::eI)] = GLFW_KEY_I;
        key_codes[size_t(KeyCode::eJ)] = GLFW_KEY_J;
        key_codes[size_t(KeyCode::eK)] = GLFW_KEY_K;
        key_codes[size_t(KeyCode::eL)] = GLFW_KEY_L;
        key_codes[size_t(KeyCode::eM)] = GLFW_KEY_M;
        key_codes[size_t(KeyCode::eN)] = GLFW_KEY_N;
        key_codes[size_t(KeyCode::eO)] = GLFW_KEY_O;
        key_codes[size_t(KeyCode::eP)] = GLFW_KEY_P;
        key_codes[size_t(KeyCode::eQ)] = GLFW_KEY_Q;
        key_codes[size_t(KeyCode::eR)] = GLFW_KEY_R;
        key_codes[size_t(KeyCode::eS)] = GLFW_KEY_S;
        key_codes[size_t(KeyCode::eT)] = GLFW_KEY_T;
        key_codes[size_t(KeyCode::eU)] = GLFW_KEY_U;
        key_codes[size_t(KeyCode::eV)] = GLFW_KEY_V;
        key_codes[size_t(KeyCode::eW)] = GLFW_KEY_W;
        key_codes[size_t(KeyCode::eX)] = GLFW_KEY_X;
        key_codes[size_t(KeyCode::eY)] = GLFW_KEY_Y;
        key_codes[size_t(KeyCode::eZ)] = GLFW_KEY_Z;
        key_codes[size_t(KeyCode::eLelfBracket)] = GLFW_KEY_LEFT_BRACKET;
        key_codes[size_t(KeyCode::eBackslash)] = GLFW_KEY_BACKSLASH;
        key_codes[size_t(KeyCode::eRightBracket)] = GLFW_KEY_RIGHT_BRACKET;
        key_codes[size_t(KeyCode::eGraceAccent)] = GLFW_KEY_GRAVE_ACCENT;
        key_codes[size_t(KeyCode::eWorld1)] = GLFW_KEY_WORLD_1;
        key_codes[size_t(KeyCode::eWorld2)] = GLFW_KEY_WORLD_2;

        key_codes[size_t(KeyCode::eEscape)] = GLFW_KEY_ESCAPE;
        key_codes[size_t(KeyCode::eEnter)] = GLFW_KEY_ENTER;
        key_codes[size_t(KeyCode::eTab)] = GLFW_KEY_TAB;
        key_codes[size_t(KeyCode::eBackspace)] = GLFW_KEY_BACKSPACE;
        key_codes[size_t(KeyCode::eInsert)] = GLFW_KEY_INSERT;
        key_codes[size_t(KeyCode::eDelete)] = GLFW_KEY_DELETE;
        key_codes[size_t(KeyCode::eRight)] = GLFW_KEY_RIGHT;
        key_codes[size_t(KeyCode::eLeft)] = GLFW_KEY_LEFT;
        key_codes[size_t(KeyCode::eDown)] = GLFW_KEY_DOWN;
        key_codes[size_t(KeyCode::eUp)] = GLFW_KEY_UP;
        key_codes[size_t(KeyCode::ePageUp)] = GLFW_KEY_PAGE_UP;
        key_codes[size_t(KeyCode::ePageDown)] = GLFW_KEY_PAGE_DOWN;
        key_codes[size_t(KeyCode::eHome)] = GLFW_KEY_HOME;
        key_codes[size_t(KeyCode::eEnd)] = GLFW_KEY_END;
        key_codes[size_t(KeyCode::eCapsLock)] = GLFW_KEY_CAPS_LOCK;
        key_codes[size_t(KeyCode::eScrollLock)] = GLFW_KEY_SCROLL_LOCK;
        key_codes[size_t(KeyCode::eNumLock)] = GLFW_KEY_NUM_LOCK;
        key_codes[size_t(KeyCode::ePrintScreen)] = GLFW_KEY_PRINT_SCREEN;
        key_codes[size_t(KeyCode::ePause)] = GLFW_KEY_PAUSE;
        key_codes[size_t(KeyCode::eF1)] = GLFW_KEY_F1;
        key_codes[size_t(KeyCode::eF2)] = GLFW_KEY_F2;
        key_codes[size_t(KeyCode::eF3)] = GLFW_KEY_F3;
        key_codes[size_t(KeyCode::eF4)] = GLFW_KEY_F4;
        key_codes[size_t(KeyCode::eF5)] = GLFW_KEY_F5;
        key_codes[size_t(KeyCode::eF6)] = GLFW_KEY_F6;
        key_codes[size_t(KeyCode::eF7)] = GLFW_KEY_F7;
        key_codes[size_t(KeyCode::eF8)] = GLFW_KEY_F8;
        key_codes[size_t(KeyCode::eF9)] = GLFW_KEY_F9;
        key_codes[size_t(KeyCode::eF10)] = GLFW_KEY_F10;
        key_codes[size_t(KeyCode::eF11)] = GLFW_KEY_F11;
        key_codes[size_t(KeyCode::eF12)] = GLFW_KEY_F12;
        key_codes[size_t(KeyCode::eF13)] = GLFW_KEY_F13;
        key_codes[size_t(KeyCode::eF14)] = GLFW_KEY_F14;
        key_codes[size_t(KeyCode::eF15)] = GLFW_KEY_F15;
        key_codes[size_t(KeyCode::eF16)] = GLFW_KEY_F16;
        key_codes[size_t(KeyCode::eF17)] = GLFW_KEY_F17;
        key_codes[size_t(KeyCode::eF18)] = GLFW_KEY_F18;
        key_codes[size_t(KeyCode::eF19)] = GLFW_KEY_F19;
        key_codes[size_t(KeyCode::eF20)] = GLFW_KEY_F20;
        key_codes[size_t(KeyCode::eF21)] = GLFW_KEY_F21;
        key_codes[size_t(KeyCode::eF22)] = GLFW_KEY_F22;
        key_codes[size_t(KeyCode::eF23)] = GLFW_KEY_F23;
        key_codes[size_t(KeyCode::eF24)] = GLFW_KEY_F24;
        key_codes[size_t(KeyCode::eF25)] = GLFW_KEY_F25;
        key_codes[size_t(KeyCode::eKeyPad0)] = GLFW_KEY_KP_0;
        key_codes[size_t(KeyCode::eKeyPad1)] = GLFW_KEY_KP_1;
        key_codes[size_t(KeyCode::eKeyPad2)] = GLFW_KEY_KP_2;
        key_codes[size_t(KeyCode::eKeyPad3)] = GLFW_KEY_KP_3;
        key_codes[size_t(KeyCode::eKeyPad4)] = GLFW_KEY_KP_4;
        key_codes[size_t(KeyCode::eKeyPad5)] = GLFW_KEY_KP_5;
        key_codes[size_t(KeyCode::eKeyPad6)] = GLFW_KEY_KP_6;
        key_codes[size_t(KeyCode::eKeyPad7)] = GLFW_KEY_KP_7;
        key_codes[size_t(KeyCode::eKeyPad8)] = GLFW_KEY_KP_8;
        key_codes[size_t(KeyCode::eKeyPad9)] = GLFW_KEY_KP_9;
        key_codes[size_t(KeyCode::eKeyPadDecimal)] = GLFW_KEY_KP_DECIMAL;
        key_codes[size_t(KeyCode::eKeyPadDivide)] = GLFW_KEY_KP_DIVIDE;
        key_codes[size_t(KeyCode::eKeyPadMultiply)] = GLFW_KEY_KP_MULTIPLY;
        key_codes[size_t(KeyCode::eKeyPadSubstract)] = GLFW_KEY_KP_SUBTRACT;
        key_codes[size_t(KeyCode::eKeyPadAdd)] = GLFW_KEY_KP_ADD;
        key_codes[size_t(KeyCode::eKeyPadEnter)] = GLFW_KEY_KP_ENTER;
        key_codes[size_t(KeyCode::eKeyPadEqual)] = GLFW_KEY_KP_EQUAL;
        key_codes[size_t(KeyCode::eLeftShift)] = GLFW_KEY_LEFT_SHIFT;
        key_codes[size_t(KeyCode::eLeftControl)] = GLFW_KEY_LEFT_CONTROL;
        key_codes[size_t(KeyCode::eLeftAlt)] = GLFW_KEY_LEFT_ALT;
        key_codes[size_t(KeyCode::eLeftSuper)] = GLFW_KEY_LEFT_SUPER;
        key_codes[size_t(KeyCode::eRightShift)] = GLFW_KEY_RIGHT_SHIFT;
        key_codes[size_t(KeyCode::eRightControl)] = GLFW_KEY_RIGHT_CONTROL;
        key_codes[size_t(KeyCode::eRightAlt)] = GLFW_KEY_RIGHT_ALT;
        key_codes[size_t(KeyCode::eRightSuper)] = GLFW_KEY_RIGHT_SUPER;
        key_codes[size_t(KeyCode::eMenu)] = GLFW_KEY_MENU;
    }

    ~Display() {
        glfwDestroyWindow(window);
    }

    auto get_instance_extensions() -> std::vector<const char*> {
        u32 count = 0;
        auto extensions = glfwGetRequiredInstanceExtensions(&count);
        return {extensions, extensions + count};
    }

    auto create_surface(vk::Instance instance) -> vk::SurfaceKHR {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        return surface;
    }

    auto should_close() -> bool {
        return glfwWindowShouldClose(window);
    }

    void poll_events() {
        glfwPollEvents();
    }

    auto get_size() const -> glm::ivec2 {
        glm::ivec2 size{};
        glfwGetWindowSize(window, &size.x, &size.y);
        return size;
    }

    auto get_scale() const -> glm::vec2 {
        glm::ivec2 size{};
        glfwGetFramebufferSize(window, &size.x, &size.y);
        return glm::vec2(size) / glm::vec2(get_size());
    }
    auto get_aspect() const -> f32 {
        const auto size = get_size();
        return f32(size.x) / f32(size.y);
    }

    auto has_focus() const -> bool {
        return glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
    }

    auto get_mouse_position() const -> glm::vec2 {
        glm::dvec2 pos;
        glfwGetCursorPos(window, &pos.x, &pos.y);
        return {pos};
    }

    void set_mouse_position(const glm::vec2& pos) {
        glfwSetCursorPos(window, pos.x, pos.y);
    }

    auto get_mouse_pressed(MouseButton button) const -> bool {
        return glfwGetMouseButton(window, mouse_buttons[static_cast<size_t>(button)]) == GLFW_PRESS;
    }

    auto get_key_pressed(KeyCode keycode) const -> bool {
        return glfwGetKey(window, key_codes[static_cast<size_t>(keycode)]) == GLFW_PRESS;
    }
};