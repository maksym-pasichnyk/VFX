#pragma once

#include "Object.hpp"
#include "NotSwiftUI/Core/Size.hpp"
#include "NotSwiftUI/Core/Point.hpp"
#include "NotSwiftUI/Core/Color.hpp"

#include <stack>

#include "imgui.h"
#include "imgui_internal.h"

struct UIContext : Object {
private:
    struct State {
        float x = 0.0F;
        float y = 0.0F;
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        Color fillColor = {1.0F, 1.0F, 1.0F, 1.0F};
    };

    ImDrawList* pDrawList;

    State mCurrentState = {};
    std::deque<State> mSavedStates = {};

public:
    explicit UIContext(ImDrawList* drawList) : pDrawList(drawList) {}

public:
    auto drawList() -> ImDrawList* {
        return pDrawList;
    }

    void setFillColor(const Color& color) {
        mCurrentState.fillColor = color;
    }

    void saveState() {
        mSavedStates.emplace_back(mCurrentState);
    }

    void restoreState() {
        mCurrentState = mSavedStates.back();
        mSavedStates.pop_back();

        pDrawList->_Data->Scale.x = mCurrentState.scaleX;
        pDrawList->_Data->Scale.y = mCurrentState.scaleY;
    }

    void translateBy(float x, float y) {
        mCurrentState.x += x;
        mCurrentState.y += y;
    }

    void scaleBy(float x, float y) {
        mCurrentState.scaleX *= x;
        mCurrentState.scaleY *= y;

        pDrawList->_Data->Scale.x = mCurrentState.scaleX;
        pDrawList->_Data->Scale.y = mCurrentState.scaleY;
    }

    void drawRect(const Size& size, float width, float rounding = 0.0F) {
        if (mCurrentState.fillColor.a == 0.0F) {
            return;
        }
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float x0 = mCurrentState.x;
        float y0 = mCurrentState.y;
        float x1 = mCurrentState.x + size.width;
        float y1 = mCurrentState.y + size.height;

        pDrawList->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), imColor, rounding, ImDrawFlags_RoundCornersAll, width);
    }

    void drawRectFilled(const Size& size, float rounding = 0.0F) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float x0 = mCurrentState.x;
        float y0 = mCurrentState.y;
        float x1 = mCurrentState.x + size.width;
        float y1 = mCurrentState.y + size.height;
        pDrawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), imColor, rounding, 0);
    }

    void drawCircle(float radius, float width) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float x = mCurrentState.x + radius;
        float y = mCurrentState.y + radius;
        pDrawList->AddCircle(ImVec2(x, y), radius, imColor, 0, width);
    }

    void drawCircleFilled(float radius) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float x = mCurrentState.x + radius;
        float y = mCurrentState.y + radius;
        pDrawList->AddCircleFilled(ImVec2(x, y), radius, imColor, 100);
    }

    void drawLine(const Point& p1, const Point& p2, float width) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float x0 = mCurrentState.x + p1.x;
        float y0 = mCurrentState.y + p1.y;
        float x1 = mCurrentState.x + p2.x;
        float y1 = mCurrentState.y + p2.y;
        pDrawList->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), imColor, width);
    }

    void drawText(std::string_view text, float fontSize, ImFont* font = nullptr, float wrap_width = 0.0F) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float_t x = mCurrentState.x;
        float_t y = mCurrentState.y;
        pDrawList->AddText(font, fontSize, ImVec2(x, y), imColor, text.begin(), text.end(), wrap_width);
    }
};
