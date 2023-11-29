#pragma once

#include "Object.hpp"
#include "NotSwiftUI/Core/Size.hpp"
#include "NotSwiftUI/Core/Point.hpp"
#include "NotSwiftUI/Core/Color.hpp"

#include <stack>

#include "imgui.h"
#include "imgui_internal.h"

struct Canvas : public ManagedObject {
private:
    struct State {
        float x = 0.0F;
        float y = 0.0F;
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        Color fillColor = {1.0F, 1.0F, 1.0F, 1.0F};
    };

    ImDrawList*         draw_list;
    State               current_state = {};
    std::deque<State>   saved_states = {};

public:
    explicit Canvas(ImDrawList* drawList) : draw_list(drawList) {}

public:
    auto drawList() -> ImDrawList* {
        return draw_list;
    }

    void setFillColor(const Color& color) {
        current_state.fillColor = color;
    }

    void saveState() {
        saved_states.emplace_back(current_state);
    }

    void restoreState() {
        current_state = saved_states.back();
        saved_states.pop_back();

        draw_list->_Data->Scale.x = current_state.scaleX;
        draw_list->_Data->Scale.y = current_state.scaleY;
    }

    void translateBy(float x, float y) {
        current_state.x += x;
        current_state.y += y;
    }

    void scaleBy(float x, float y) {
        current_state.scaleX *= x;
        current_state.scaleY *= y;

        draw_list->_Data->Scale.x = current_state.scaleX;
        draw_list->_Data->Scale.y = current_state.scaleY;
    }

    void drawRect(const Size& size, float width, float rounding = 0.0F) {
        if (current_state.fillColor.a == 0.0F) {
            return;
        }
        ImU32 imColor = static_cast<ImU32>(ImColor(
            current_state.fillColor.r,
            current_state.fillColor.g,
            current_state.fillColor.b,
            current_state.fillColor.a
        ));
        float x0 = current_state.x;
        float y0 = current_state.y;
        float x1 = current_state.x + size.width;
        float y1 = current_state.y + size.height;

        draw_list->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), imColor, rounding, ImDrawFlags_RoundCornersAll, width);
    }

    void drawRectFilled(const Size& size, float rounding = 0.0F) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            current_state.fillColor.r,
            current_state.fillColor.g,
            current_state.fillColor.b,
            current_state.fillColor.a
        ));
        float x0 = current_state.x;
        float y0 = current_state.y;
        float x1 = current_state.x + size.width;
        float y1 = current_state.y + size.height;
        draw_list->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), imColor, rounding, 0);
    }

    void drawCircle(float radius, float width) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            current_state.fillColor.r,
            current_state.fillColor.g,
            current_state.fillColor.b,
            current_state.fillColor.a
        ));
        float x = current_state.x + radius;
        float y = current_state.y + radius;
        draw_list->AddCircle(ImVec2(x, y), radius, imColor, 0, width);
    }

    void drawCircleFilled(float radius) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            current_state.fillColor.r,
            current_state.fillColor.g,
            current_state.fillColor.b,
            current_state.fillColor.a
        ));
        float x = current_state.x + radius;
        float y = current_state.y + radius;
        draw_list->AddCircleFilled(ImVec2(x, y), radius, imColor, 100);
    }

    void drawLine(const Point& p1, const Point& p2, float width) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            current_state.fillColor.r,
            current_state.fillColor.g,
            current_state.fillColor.b,
            current_state.fillColor.a
        ));
        float x0 = current_state.x + p1.x;
        float y0 = current_state.y + p1.y;
        float x1 = current_state.x + p2.x;
        float y1 = current_state.y + p2.y;
        draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), imColor, width);
    }

    void drawText(std::string_view text, float fontSize, ImFont* font = nullptr, float wrap_width = 0.0F) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            current_state.fillColor.r,
            current_state.fillColor.g,
            current_state.fillColor.b,
            current_state.fillColor.a
        ));
        float_t x = current_state.x;
        float_t y = current_state.y;
        draw_list->AddText(font, fontSize, ImVec2(x, y), imColor, text.begin(), text.end(), wrap_width);
    }
};
