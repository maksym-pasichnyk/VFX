#pragma once

#include "UISize.hpp"
#include "UIPoint.hpp"
#include "UIColor.hpp"
#include "Alignment.hpp"

#include <stack>

#include "imgui.h"
#include "imgui_internal.h"

struct UIContext : gfx::Referencing {
    struct State {
        float_t x = 0.0F;
        float_t y = 0.0F;
        UIColor fillColor = UIColor(1.0F, 1.0F, 1.0F, 1.0F);
    };

    ImDrawListSharedData mDrawListSharedData = {};
    ImDrawList mDrawList;

    State mCurrentState = {};
    std::deque<State> mSavedStates = {};

    explicit UIContext() : mDrawList(&mDrawListSharedData) {
        mDrawList._ResetForNewFrame();
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
    }

    void translateBy(float_t x, float_t y) {
        mCurrentState.x += x;
        mCurrentState.y += y;
    }

    void drawRect(const UISize& size, float_t width) {
        float_t x0 = mCurrentState.x;
        float_t y0 = mCurrentState.y;
        float_t x1 = mCurrentState.x + size.width;
        float_t y1 = mCurrentState.y + size.height;

        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));

        mDrawList.AddRectFilled(ImVec2(x0, y0), ImVec2(x0 + width, y1), imColor, 0.0F, 0);
        mDrawList.AddRectFilled(ImVec2(x0, y1 - width), ImVec2(x1 - width, y1), imColor, 0.0F, 0);
        mDrawList.AddRectFilled(ImVec2(x0 + width, y0), ImVec2(x1, y0 + width), imColor, 0.0F, 0);
        mDrawList.AddRectFilled(ImVec2(x1 - width, y0 + width), ImVec2(x1, y1), imColor, 0.0F, 0);
    }

    void drawRectFilled(const UISize& size) {
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
        mDrawList.AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), imColor, 0.0F, 0);
    }

    void drawCircleFilled(float_t radius) {
        ImU32 imColor = static_cast<ImU32>(ImColor(
            mCurrentState.fillColor.r,
            mCurrentState.fillColor.g,
            mCurrentState.fillColor.b,
            mCurrentState.fillColor.a
        ));

        mDrawList.AddCircleFilled(ImVec2(mCurrentState.x + radius, mCurrentState.y + radius), radius, imColor, 100);
    }

    // todo: move to View
    void align(const UISize& childSize, const UISize& parentSize, const Alignment& alignment) {
        UIPoint childPoint = alignment.point(childSize);
        UIPoint parentPoint = alignment.point(parentSize);
        translateBy(parentPoint.x - childPoint.x, parentPoint.y - childPoint.y);
    }
};
