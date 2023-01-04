#pragma once

#include "NotSwiftUI/UIContext.hpp"

#include <SDL_mouse.h>

struct GraphView : View {
private:
    enum class Interaction {
        eNone,
        eDragGrid,
        eDragNode,
    };

public:
    struct Node;
    struct Link;

    struct Port : gfx::Referencing {
        friend Node;
        friend GraphView;

    public:
        enum class Capacity {
            eSingle,
            eMulti
        };

    private:
        Node* pNode;
        size_t mIndex;
        std::string mName;
        Capacity mCapacity;

        std::set<gfx::SharedPtr<Link>> mLinks = {};

    private:
        explicit Port(Node* node, size_t index, std::string name, Capacity capacity)
            : pNode(node), mIndex(index), mName(std::move(name)), mCapacity(capacity) {}
    };

    struct Node : gfx::Referencing {
        friend GraphView;

    private:
        std::string mText;
        UISize mSize = UISize(450.0F, 250.0F);
        UIPoint mPosition = UIPoint(0.0F, 0.0F);

        std::vector<gfx::SharedPtr<Port>> mInputs = {};
        std::vector<gfx::SharedPtr<Port>> mOutputs = {};

    private:
        explicit Node(std::string text) : mText(std::move(text)) {}

    public:
        auto size() -> const UISize& {
            return mSize;
        }

        void setSize(const UISize& size) {
            mSize = size;
        }

        auto position() -> const UIPoint& {
            return mPosition;
        }

        void setPosition(const UIPoint& point) {
            mPosition = point;
        }

        auto text() -> const std::string& {
            return mText;
        }

        void setText(std::string text) {
            mText = std::move(text);
        }

        void addInput(std::string name, Port::Capacity capacity) {
            mInputs.emplace_back(gfx::TransferPtr(new Port(this, mInputs.size(), std::move(name), capacity)));
        }

        void addOutput(std::string name, Port::Capacity capacity) {
            mOutputs.emplace_back(gfx::TransferPtr(new Port(this, mOutputs.size(), std::move(name), capacity)));
        }

        auto getInput(int32_t index) -> gfx::SharedPtr<Port> {
            return mInputs.at(index);
        }

        auto getOutput(int32_t index) -> gfx::SharedPtr<Port> {
            return mOutputs.at(index);
        }

    private:
        void draw(const gfx::SharedPtr<UIContext> &context, bool selected) {
            ImVec2 textSize1 = context->drawList()->_Data->Font->CalcTextSizeA(36.0F, FLT_MAX, FLT_MAX, mText.data(), mText.data() + mText.size(), nullptr);

            context->saveState();
            context->translateBy(mPosition.x, mPosition.y);
            context->setFillColor(UIColor::rgba32(28, 28, 28, 245));
            context->drawRectFilled(mSize, 5.0F);
            if (selected) {
                context->setFillColor(UIColor::rgba32(4, 156, 227, 255));
            } else {
                context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
            }
            context->drawRect(mSize, 2.0F, 5.0F);
            context->drawLine(UIPoint(0, 50.0F), UIPoint(mSize.width, 50.0F), 2.0F);
            context->setFillColor(UIColor(1, 1, 1, 1));
            context->translateBy(mSize.width * 0.5F, 50.0F * 0.5F);
            context->translateBy(-textSize1.x * 0.5F, -textSize1.y * 0.5F);
            context->drawText(mText, 36.0F);
            context->restoreState();

            for (auto& port : mInputs) {
                UIPoint slot = _getInputSlot(port->mIndex);

                context->saveState();
                context->translateBy(slot.x, slot.y);

                context->translateBy(-10.0F, -10.0F);
                context->setFillColor(UIColor::rgba32(37, 150, 190, 255));
                context->drawCircleFilled(10.0F);

                if (port->mLinks.empty()) {
                    context->translateBy(2.0F, 2.0F);
                    context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
                    context->drawCircleFilled(8.0F);
                }

                context->restoreState();

                if (!port->mName.empty()) {
                    ImVec2 textSize2 = context->drawList()->_Data->Font->CalcTextSizeA(28.0F, FLT_MAX, FLT_MAX, port->mName.data(), port->mName.data() + port->mName.size(), nullptr);

                    context->saveState();
                    context->translateBy(slot.x, slot.y);
                    context->translateBy(20.0F, -textSize2.y * 0.5F);
                    context->drawText(port->mName, 28.0F);
                    context->restoreState();
                }
            }

            for (auto& port : mOutputs) {
                UIPoint slot = _getOutputSlot(port->mIndex);

                context->saveState();
                context->translateBy(slot.x, slot.y);
                context->translateBy(-10.0F, -10.0F);

                context->setFillColor(UIColor::rgba32(37, 150, 190, 255));
                context->drawCircleFilled(10.0F);

                if (port->mLinks.empty()) {
                    context->translateBy(2.0F, 2.0F);
                    context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
                    context->drawCircleFilled(8.0F);
                }

                context->restoreState();

                if (!port->mName.empty()) {
                    ImVec2 textSize3 = context->drawList()->_Data->Font->CalcTextSizeA(28.0F, FLT_MAX, FLT_MAX, port->mName.data(), port->mName.data() + port->mName.size(), nullptr);

                    context->saveState();
                    context->translateBy(slot.x, slot.y);
                    context->translateBy(-20.0F, 0.0F);
                    context->translateBy(-textSize3.x, -textSize3.y * 0.5F);
                    context->drawText(port->mName, 28.0F);
                    context->restoreState();
                }
            }
        }

        auto _getInputSlot(size_t i) -> UIPoint {
            float_t x = mPosition.x + 10.0F + 10.0F + 10.0F;
            float_t y = mPosition.y + 10.0F + 10.0F + 15.0F + 50.0F + 30.0F * i;
            return UIPoint(x, y);
        }

        auto _getOutputSlot(size_t i) -> UIPoint {
            float_t x = mPosition.x + 10.0F + mSize.width - 20.0F - 10.0F;
            float_t y = mPosition.y + 10.0F + 10.0F + 15.0F + 50.0F + 30.0F * i;
            return UIPoint(x, y);
        }
    };

    struct Link : gfx::Referencing {
        friend GraphView;

    private:
        gfx::SharedPtr<Port> mPortA;
        gfx::SharedPtr<Port> mPortB;

    private:
        explicit Link(gfx::SharedPtr<Port> portA, gfx::SharedPtr<Port> portB)
            : mPortA(std::move(portA)), mPortB(std::move(portB)){}
    };

private:
    float_t mZoomScale = 1.0F;
    std::list<gfx::SharedPtr<Node>> mNodes = {};
    std::list<gfx::SharedPtr<Link>> mLinks = {};

    UIPoint mGridOffset = UIPoint();
    UIPoint mMousePosition = UIPoint();
    UIPoint mStartDragPosition = UIPoint();
    Interaction mInteraction = Interaction::eNone;
    gfx::SharedPtr<Node> mSelectedNode = {};

private:
    auto _size(const ProposedSize &proposed) -> UISize override {
        return proposed.orMax();
    }

    void _draw(const gfx::SharedPtr<UIContext> &context, const UISize& size) override {
        float invScale = 1.0F / mZoomScale;

        context->saveState();
        context->setFillColor(UIColor::rgba32(45, 45, 45, 255));
        context->drawRectFilled(size);
        context->setFillColor(UIColor(0.0F, 0.0F, 0.0F, 0.25F));

        float_t cellSize = 50.0F * invScale;
        float_t x = std::fmod(mGridOffset.x * invScale, cellSize);
        float_t y = std::fmod(mGridOffset.y * invScale, cellSize);
        for (; x < size.width; x += cellSize) {
            context->drawLine(UIPoint(x, 0.0F), UIPoint(x, size.height), 1.0F);
        }
        for (; y < size.height; y += cellSize) {
            context->drawLine(UIPoint(0.0F, y), UIPoint(size.width, y), 1.0F);
        }
        context->restoreState();

        context->saveState();
        context->translateBy(mGridOffset.x, mGridOffset.y);
        context->scaleBy(invScale, invScale);

        for (auto& link : mLinks) {
            UIPoint pointA = link->mPortA->pNode->_getOutputSlot(link->mPortA->mIndex);
            UIPoint pointD = link->mPortB->pNode->_getInputSlot(link->mPortB->mIndex);

            UIPoint pointB = pointA + UIPoint(std::abs(pointD.x - pointA.x), 0.0F);
            UIPoint pointC = pointD - UIPoint(std::abs(pointD.x - pointA.x), 0.0F);

            ImVec2 p0 = ImVec2(mGridOffset.x + pointA.x, mGridOffset.y + pointA.y);
            ImVec2 p1 = ImVec2(mGridOffset.x + pointB.x, mGridOffset.y + pointB.y);
            ImVec2 p2 = ImVec2(mGridOffset.x + pointC.x, mGridOffset.y + pointC.y);
            ImVec2 p3 = ImVec2(mGridOffset.x + pointD.x, mGridOffset.y + pointD.y);

            context->drawList()->PathLineTo(p0);
            context->drawList()->PathBezierCubicCurveTo(p1, p2, p3);
            context->drawList()->PathStroke(IM_COL32(255, 255, 255, 255), 0, 5.0F);
        }

        for (auto& node : mNodes) {
            node->draw(context, node == mSelectedNode);
        }

        context->restoreState();
    }

    auto findNodeAt(int32_t x, int32_t y) -> gfx::SharedPtr<Node> {
        for (auto& node : ranges::reverse_view(mNodes)) {
            float_t x1 = static_cast<float_t>(x) * mZoomScale - node->mPosition.x - mGridOffset.x;
            float_t y1 = static_cast<float_t>(y) * mZoomScale - node->mPosition.y - mGridOffset.y;

            if (x1 < 0.0F || x1 > node->mSize.width) {
                continue;
            }
            if (y1 < 0.0F || y1 > node->mSize.height) {
                continue;
            }
            return node;
        }
        return {};
    }

public:
    auto zoomScale() -> float_t {
        return mZoomScale;
    }

    void setZoomScale(float_t zoomScale) {
        mZoomScale = std::max(zoomScale, 1.0F);
    }

    auto addNode(std::string text) -> gfx::SharedPtr<Node> {
        return mNodes.emplace_back(gfx::TransferPtr(new Node(std::move(text))));
    }

    auto addLink(const gfx::SharedPtr<Node>& nodeA, const gfx::SharedPtr<Node>& nodeB, size_t slotA, size_t slotB) -> gfx::SharedPtr<Link> {
        auto portA = nodeA->mOutputs.at(slotA);
        auto portB = nodeB->mInputs.at(slotB);

        for (auto& link : mLinks) {
            if (link->mPortA == portA && link->mPortB == portB) {
                return {};
            }
        }
        if (portA->mCapacity == Port::Capacity::eSingle) {
            removeLinks(portA);
        }
        if (portB->mCapacity == Port::Capacity::eSingle) {
            removeLinks(portB);
        }

        auto link = mLinks.emplace_back(gfx::TransferPtr(new Link(portA, portB)));
        portA->mLinks.emplace(link);
        portB->mLinks.emplace(link);
        return link;
    }

    void removeLinks(const gfx::SharedPtr<Port>& port) {
        for (auto& link : auto(port->mLinks)) {
            link->mPortA->mLinks.erase(link);
            link->mPortB->mLinks.erase(link);
            mLinks.erase(std::find(mLinks.begin(), mLinks.end(), link));
        }
    }

    void update(float_t dt) {
        int32_t x, y;
        uint32_t mouseState = SDL_GetMouseState(&x, &y);

        mStartDragPosition = mMousePosition;
        mMousePosition = UIPoint(static_cast<float_t>(x), static_cast<float_t>(y));

        if (mInteraction == Interaction::eNone) {
            if ((mouseState & SDL_BUTTON_LMASK) != 0) {
                mSelectedNode = findNodeAt(x, y);
                if (mSelectedNode) {
                    mInteraction = Interaction::eDragNode;
                    mStartDragPosition = mMousePosition;

                    mNodes.erase(std::find(mNodes.begin(), mNodes.end(), mSelectedNode));
                    mNodes.emplace_back(mSelectedNode);
                } else {
                    mInteraction = Interaction::eDragGrid;
                    mStartDragPosition = mMousePosition;
                }
            }
        }

        UIPoint dragOffset = (mMousePosition - mStartDragPosition) * mZoomScale;

        if (mInteraction == Interaction::eDragNode) {
            if ((mouseState & SDL_BUTTON_LMASK) != 0) {
                mSelectedNode->setPosition(mSelectedNode->position() + dragOffset);
            } else {
                mInteraction = Interaction::eNone;
            }
        }

        if (mInteraction == Interaction::eDragGrid) {
            if ((mouseState & SDL_BUTTON_LMASK) != 0) {
                mGridOffset += dragOffset;
            } else {
                mInteraction = Interaction::eNone;
            }
        }
    }
};
