#pragma once

#include "gfx/Object.hpp"

#include "imgui.h"
#include "imgui_internal.h"

struct UISize {
    float_t width;
    float_t height;
};

struct UIPoint {
    float_t x;
    float_t y;
};

struct UIColor {
    float_t r;
    float_t g;
    float_t b;
    float_t a;
};

struct UIContext : gfx::Referencing {
    ImDrawListSharedData draw_list_shared_data = {};
    ImDrawList draw_list;

    explicit UIContext() : draw_list(&draw_list_shared_data) {
        draw_list._ResetForNewFrame();
    }
};
