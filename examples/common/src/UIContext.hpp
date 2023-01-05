#pragma once

#include "UISize.hpp"
#include "UIPoint.hpp"
#include "UIColor.hpp"

#include <stack>

#include "imgui.h"
#include "imgui_internal.h"

struct UIContext : gfx::Referencing {
private:
    struct State {
        float_t x = 0.0F;
        float_t y = 0.0F;
        float_t scaleX = 1.0F;
        float_t scaleY = 1.0F;
        UIColor fillColor = UIColor(1.0F, 1.0F, 1.0F, 1.0F);
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

    void setFillColor(const UIColor& color) {
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

    void translateBy(float_t x, float_t y) {
        mCurrentState.x += x;
        mCurrentState.y += y;
    }

    void scaleBy(float_t x, float_t y) {
        mCurrentState.scaleX *= x;
        mCurrentState.scaleY *= y;

        pDrawList->_Data->Scale.x = mCurrentState.scaleX;
        pDrawList->_Data->Scale.y = mCurrentState.scaleY;
    }

    struct UIPath {
        virtual ~UIPath() = default;

        virtual void stroke(ImDrawList* drawList) = 0;
    };

    void drawRect(const UISize& size, float_t width, float_t rounding = 0.0F) {
        if (mCurrentState.fillColor.a == 0.0F) {
            return;
        }
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float_t x0 = mCurrentState.x;
        float_t y0 = mCurrentState.y;
        float_t x1 = mCurrentState.x + size.width;
        float_t y1 = mCurrentState.y + size.height;

        pDrawList->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), imColor, rounding, ImDrawFlags_RoundCornersAll, width);
    }

    void drawRectFilled(const UISize& size, float_t rounding = 0.0F) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float_t x0 = mCurrentState.x;
        float_t y0 = mCurrentState.y;
        float_t x1 = mCurrentState.x + size.width;
        float_t y1 = mCurrentState.y + size.height;
        pDrawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), imColor, rounding, 0);
    }

    void drawCircle(float_t radius, float_t width) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float_t x = mCurrentState.x + radius;
        float_t y = mCurrentState.y + radius;
        pDrawList->AddCircle(ImVec2(x, y), radius, imColor, 0, width);
    }

    void drawCircleFilled(float_t radius) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float_t x = mCurrentState.x + radius;
        float_t y = mCurrentState.y + radius;
        pDrawList->AddCircleFilled(ImVec2(x, y), radius, imColor, 100);
    }

    void drawLine(const UIPoint& p1, const UIPoint& p2, float width) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));
        float_t x0 = mCurrentState.x + p1.x;
        float_t y0 = mCurrentState.y + p1.y;
        float_t x1 = mCurrentState.x + p2.x;
        float_t y1 = mCurrentState.y + p2.y;
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
