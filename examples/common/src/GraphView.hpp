#pragma once

#include "NotSwiftUI/NotSwiftUI.hpp"

#include <set>
#include <list>

#include <SDL_mouse.h>
#include <SDL_events.h>
#include <SDL_keyboard.h>

struct GraphView : View {
private:
    enum class Interaction {
        eNone,
        eZoom,
        eDragGrid,
        eDragNode,
        eDragLink
    };

public:
    struct Node;
    struct Link;

    struct Port : ManagedObject<Port> {
        friend Node;
        friend GraphView;

    public:
        enum class Capacity {
            eSingle,
            eMulti
        };

        enum class Direction {
            eInput,
            eOutput
        };

    private:
        Node* pNode;
        size_t mIndex;
        std::string mName;
        Capacity mCapacity;
        Direction mDirection;

        std::set<ManagedShared<Link>> mLinks = {};

    private:
        explicit Port(Node* node, size_t index, std::string name, Capacity capacity, Direction direction)
            : pNode(node), mIndex(index), mName(std::move(name)), mCapacity(capacity), mDirection(direction) {}
    };

    struct Node : ManagedObject<Node> {
        friend GraphView;

    private:
        std::string mText;
        Size mSize = Size{450.0F, 250.0F};
        Point mPosition = Point::zero();

        std::vector<ManagedShared<Port>> mInputs = {};
        std::vector<ManagedShared<Port>> mOutputs = {};

    private:
        explicit Node(std::string text) : mText(std::move(text)) {}

    public:
        auto size() -> const Size& {
            return mSize;
        }

        void setSize(const Size& size) {
            mSize = size;
        }

        auto position() -> const Point& {
            return mPosition;
        }

        void setPosition(const Point& point) {
            mPosition = point;
        }

        auto text() -> const std::string& {
            return mText;
        }

        void setText(std::string text) {
            mText = std::move(text);
        }

        void addInput(std::string name, Port::Capacity capacity) {
            mInputs.emplace_back(TransferPtr(new Port(this, mInputs.size(), std::move(name), capacity, Port::Direction::eInput)));
        }

        void addOutput(std::string name, Port::Capacity capacity) {
            mOutputs.emplace_back(TransferPtr(new Port(this, mOutputs.size(), std::move(name), capacity, Port::Direction::eOutput)));
        }

    private:
        void draw(const ManagedShared<Canvas> &canvas, bool selected) {
            ImVec2 textSize1 = canvas->drawList()->_Data->Font->CalcTextSizeA(36.0F, FLT_MAX, FLT_MAX, mText.data(), mText.data() + mText.size(), nullptr);

            canvas->saveState();
            canvas->translateBy(mPosition.x, mPosition.y);
            canvas->setFillColor(Color::rgba32(28, 28, 28, 245));
            canvas->drawRectFilled(mSize, 5.0F);
            if (selected) {
                canvas->setFillColor(Color::rgba32(4, 156, 227, 255));
            } else {
                canvas->setFillColor(Color::rgba32(0, 0, 0, 255));
            }
            canvas->drawRect(mSize, 2.0F, 5.0F);
            canvas->drawLine(Point{0, 50.0F}, Point{mSize.width, 50.0F}, 2.0F);
            canvas->setFillColor(Color::rgba32(255, 255, 255, 255));
            canvas->translateBy(mSize.width * 0.5F, 50.0F * 0.5F);
            canvas->translateBy(-textSize1.x * 0.5F, -textSize1.y * 0.5F);
            canvas->drawText(mText, 36.0F);
            canvas->restoreState();

            for (auto& port : mInputs) {
                Point slot = _getSlotPosition(port);

                canvas->saveState();
                canvas->translateBy(slot.x, slot.y);

                canvas->translateBy(-10.0F, -10.0F);
                canvas->setFillColor(Color::rgba32(37, 150, 190, 255));
                canvas->drawCircleFilled(10.0F);

                if (port->mLinks.empty()) {
                    canvas->translateBy(2.0F, 2.0F);
                    canvas->setFillColor(Color::rgba32(0, 0, 0, 255));
                    canvas->drawCircleFilled(8.0F);
                }

                canvas->restoreState();

                if (!port->mName.empty()) {
                    ImVec2 textSize2 = canvas->drawList()->_Data->Font->CalcTextSizeA(28.0F, FLT_MAX, FLT_MAX, port->mName.data(), port->mName.data() + port->mName.size(), nullptr);

                    canvas->saveState();
                    canvas->translateBy(slot.x, slot.y);
                    canvas->translateBy(20.0F, -textSize2.y * 0.5F);
                    canvas->drawText(port->mName, 28.0F);
                    canvas->restoreState();
                }
            }

            for (auto& port : mOutputs) {
                Point slot = _getSlotPosition(port);

                canvas->saveState();
                canvas->translateBy(slot.x, slot.y);
                canvas->translateBy(-10.0F, -10.0F);

                canvas->setFillColor(Color::rgba32(37, 150, 190, 255));
                canvas->drawCircleFilled(10.0F);

                if (port->mLinks.empty()) {
                    canvas->translateBy(2.0F, 2.0F);
                    canvas->setFillColor(Color::rgba32(0, 0, 0, 255));
                    canvas->drawCircleFilled(8.0F);
                }

                canvas->restoreState();

                if (!port->mName.empty()) {
                    ImVec2 textSize3 = canvas->drawList()->_Data->Font->CalcTextSizeA(28.0F, FLT_MAX, FLT_MAX, port->mName.data(), port->mName.data() + port->mName.size(), nullptr);

                    canvas->saveState();
                    canvas->translateBy(slot.x, slot.y);
                    canvas->translateBy(-20.0F, 0.0F);
                    canvas->translateBy(-textSize3.x, -textSize3.y * 0.5F);
                    canvas->drawText(port->mName, 28.0F);
                    canvas->restoreState();
                }
            }
        }

        auto _getSlotPosition(const ManagedShared<Port>& port) -> Point {
            if (port->mDirection == Port::Direction::eInput) {
                float x = mPosition.x + 10.0F + 10.0F + 10.0F;
                float y = mPosition.y + 10.0F + 10.0F + 15.0F + 50.0F + 30.0F * static_cast<float>(port->mIndex);
                return Point{x, y};
            } else {
                float x = mPosition.x + 10.0F + mSize.width - 20.0F - 10.0F;
                float y = mPosition.y + 10.0F + 10.0F + 15.0F + 50.0F + 30.0F * static_cast<float_t>(port->mIndex);
                return Point{x, y};
            }
        }
    };

    struct Link : ManagedObject<Link> {
        friend GraphView;

    private:
        ManagedShared<Port> mPortA;
        ManagedShared<Port> mPortB;

    private:
        explicit Link(ManagedShared<Port> portA, ManagedShared<Port> portB)
            : mPortA(std::move(portA)), mPortB(std::move(portB)){}
    };

private:
    float_t mZoomScale = 2.0F;
    float_t mTargetZoomScale = 2.0F;
    Point mTargetZoomPoint = Point();

    std::list<ManagedShared<Node>> mNodes = {};
    std::list<ManagedShared<Link>> mLinks = {};

    Point mGridOffset = Point();
    Point mStartPosition = Point();
    Point mMousePosition = Point();
    Point mCursorPosition = Point();
    Interaction mInteraction = Interaction::eNone;

    ManagedShared<Node> mSelectedNode = {};
    ManagedShared<Port> mSelectedPort = {};
    ManagedShared<Link> mSelectedLink = {};

private:
    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        return proposed.orMax();
    }

    void _draw(const ManagedShared<Canvas> &canvas, const Size& size) override {
        float invScale = 1.0F / mZoomScale;

        canvas->saveState();
        canvas->setFillColor(Color::rgba32(45, 45, 45, 255));
        canvas->drawRectFilled(size);
        canvas->setFillColor(Color{0.0F, 0.0F, 0.0F, 0.25F});

        float_t cellSize = 50.0F * invScale;
        float_t x = std::fmod(mGridOffset.x * invScale, cellSize);
        float_t y = std::fmod(mGridOffset.y * invScale, cellSize);
        while (x < size.width) {
            canvas->drawLine(Point{x, 0.0F}, Point{x, size.height}, 1.0F);
            x += cellSize;
        }
        while (y < size.height) {
            canvas->drawLine(Point{0.0F, y}, Point{size.width, y}, 1.0F);
            y += cellSize;
        }
        canvas->restoreState();

        canvas->saveState();
        canvas->translateBy(mGridOffset.x, mGridOffset.y);
        canvas->scaleBy(invScale, invScale);

        for (auto& link : mLinks) {
            Point pointA = link->mPortA->pNode->_getSlotPosition(link->mPortA);
            Point pointD = link->mPortB->pNode->_getSlotPosition(link->mPortB);

            drawLink(canvas, pointA, pointD);

//            int32_t indexA = canvas->drawList()->VtxBuffer.Size;
//            if (mSelectedLink == link) {
//                drawLink(canvas, pointA, pointD, IM_COL32(37, 150, 190, 255));
//            } else {
//                drawLink(canvas, pointA, pointD, IM_COL32(255, 255, 255, 255));
//            }
//            int32_t indexB = canvas->drawList()->VtxBuffer.Size;
//
//            for (int32_t i = indexA; i < indexB; ++i) {
//                auto& vtx = canvas->drawList()->VtxBuffer[i];
//                auto p = UIPoint(vtx.pos.x, vtx.pos.y) - mMousePosition;
//
//                if (std::abs(p.x) <= 10 && std::abs(p.y) <= 10) {
//                    mSelectedLink = link;
//                    break;
//                }
//            }
        }

        if (mInteraction == Interaction::eDragLink) {
            drawLink(canvas, mSelectedPort->pNode->_getSlotPosition(mSelectedPort), mCursorPosition);
        }

        for (auto& node : mNodes) {
            node->draw(canvas, node == mSelectedNode);
        }

        canvas->restoreState();
    }

    void drawLink(const ManagedShared<Canvas> &canvas, const Point& pointA, const Point& pointD, ImU32 color = IM_COL32(255, 255, 255, 255)) {
        Point pointB = pointA + Point{std::abs(pointD.x - pointA.x), 0.0F};
        Point pointC = pointD - Point{std::abs(pointD.x - pointA.x), 0.0F};

        ImVec2 p1 = ImVec2(mGridOffset.x + pointA.x, mGridOffset.y + pointA.y);
        ImVec2 p2 = ImVec2(mGridOffset.x + pointB.x, mGridOffset.y + pointB.y);
        ImVec2 p3 = ImVec2(mGridOffset.x + pointC.x, mGridOffset.y + pointC.y);
        ImVec2 p4 = ImVec2(mGridOffset.x + pointD.x, mGridOffset.y + pointD.y);

        canvas->drawList()->PathLineTo(p1);
        canvas->drawList()->PathBezierCubicCurveTo(p2, p3, p4);
        canvas->drawList()->PathStroke(color, 0, 5.0F);
    }

    auto findNodeAt(int32_t x, int32_t y) -> ManagedShared<Node> {
        auto uiPoint = Point{static_cast<float_t>(x), static_cast<float_t>(y)};

        for (auto& node : ranges::reverse_view(mNodes)) {
            Point p = uiPoint - node->mPosition;
            if (p.x < 0.0F || p.x > node->mSize.width) {
                continue;
            }
            if (p.y < 0.0F || p.y > node->mSize.height) {
                continue;
            }
            return node;
        }
        return {};
    }

    auto findPortAt(const ManagedShared<Node>& node, int32_t x, int32_t y) -> ManagedShared<Port> {
        auto uiPoint = Point{static_cast<float_t>(x), static_cast<float_t>(y)};

        for (auto& port : node->mInputs) {
            Point p = uiPoint - node->_getSlotPosition(port);
            if (std::abs(p.x) <= 10.0F && std::abs(p.y) <= 10.0F) {
                return port;
            }
        }
        for (auto& port : node->mOutputs) {
            Point p = uiPoint - node->_getSlotPosition(port);
            if (std::abs(p.x) <= 10.0F && std::abs(p.y) <= 10.0F) {
                return port;
            }
        }
        return {};
    }

    auto addLink(const ManagedShared<Port>& portA, const ManagedShared<Port>& portB) -> ManagedShared<Link> {
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

        auto link = mLinks.emplace_back(TransferPtr(new Link(portA, portB)));
        portA->mLinks.emplace(link);
        portB->mLinks.emplace(link);
        return link;
    }

    void removeNode(const ManagedShared<Node>& node) {
        for (auto& port : node->mInputs) {
            if (port == mSelectedPort) {
                mSelectedPort = {};
            }
            removeLinks(port);
        }
        for (auto& port : node->mOutputs) {
            if (port == mSelectedPort) {
                mSelectedPort = {};
            }
            removeLinks(port);
        }
        mNodes.erase(std::find(mNodes.begin(), mNodes.end(), node));

        if (node == mSelectedNode) {
            mSelectedNode = {};
        }
    }

    void removeLinks(const ManagedShared<Port>& port) {
        for (auto& link : auto(port->mLinks)) {
            link->mPortA->mLinks.erase(link);
            link->mPortB->mLinks.erase(link);
            mLinks.erase(std::find(mLinks.begin(), mLinks.end(), link));
        }
    }

public:
    auto addNode(std::string text) -> ManagedShared<Node> {
        return mNodes.emplace_back(TransferPtr(new Node(std::move(text))));
    }

    void update() {
        if (mInteraction == Interaction::eZoom) {
            float_t zoomSpeed = std::min(ImGui::GetIO().DeltaTime * 10.0F, 1.0F);

            mZoomScale = std::lerp(mZoomScale, mTargetZoomScale, zoomSpeed);
            mGridOffset.x = std::lerp(mGridOffset.x, mTargetZoomPoint.x, zoomSpeed);
            mGridOffset.y = std::lerp(mGridOffset.y, mTargetZoomPoint.y, zoomSpeed);

            if (std::abs(mZoomScale - mTargetZoomScale) < 0.01F) {
                mInteraction = Interaction::eNone;
                mZoomScale = mTargetZoomScale;
                mGridOffset = mTargetZoomPoint;
            }
        }

        int32_t x, y;
        SDL_GetMouseState(&x, &y);

        auto uiPoint = Point{static_cast<float_t>(x), static_cast<float_t>(y)};

        mStartPosition = mMousePosition;
        mMousePosition = uiPoint;
        mCursorPosition = mMousePosition * mZoomScale - mGridOffset;

        auto drag = (mMousePosition - mStartPosition) * mZoomScale;
        if (mInteraction == Interaction::eDragNode) {
            mSelectedNode->setPosition(mSelectedNode->position() + drag);
        }

        if (mInteraction == Interaction::eDragGrid) {
            mGridOffset += drag;
        }

        if (mInteraction == Interaction::eNone) {
            if (mSelectedNode) {
                const Uint8* keys = SDL_GetKeyboardState(nullptr);

                if (keys[SDL_SCANCODE_BACKSPACE]) {
                    removeNode(mSelectedNode);
                }
            }
        }
    }

    void mouseUp(SDL_MouseButtonEvent* event) {
        if (event->button == SDL_BUTTON_LEFT) {
            if (mInteraction == Interaction::eDragNode) {
                mInteraction = Interaction::eNone;
            }
            if (mInteraction == Interaction::eDragLink) {
                auto node = findNodeAt(mCursorPosition.x, mCursorPosition.y);
                if (node && node != mSelectedNode) {
                    auto port = findPortAt(node, mCursorPosition.x, mCursorPosition.y);
                    if (port && port->mDirection == Port::Direction::eInput) {
                        addLink(mSelectedPort, port);
                    }
                }
                mInteraction = Interaction::eNone;
                mSelectedPort = {};
            }
            if (mInteraction == Interaction::eDragGrid) {
                mInteraction = Interaction::eNone;
            }
        }
    }

    void mouseDown(SDL_MouseButtonEvent* event) {
        if (mInteraction == Interaction::eNone) {
            if (event->button == SDL_BUTTON_LEFT) {
                mStartPosition = mMousePosition;

                mSelectedNode = findNodeAt(mCursorPosition.x, mCursorPosition.y);
                if (mSelectedNode) {
                    mSelectedPort = findPortAt(mSelectedNode, mCursorPosition.x, mCursorPosition.y);
                    if (mSelectedPort && mSelectedPort->mDirection == Port::Direction::eOutput) {
                        mInteraction = Interaction::eDragLink;
                    } else {
                        mInteraction = Interaction::eDragNode;
                        mSelectedPort = {};
                    }

                    mNodes.erase(std::find(mNodes.begin(), mNodes.end(), mSelectedNode));
                    mNodes.emplace_back(mSelectedNode);
                } else {
                    mInteraction = Interaction::eDragGrid;
                }
            }
            if (event->button == SDL_BUTTON_RIGHT) {
                mSelectedNode = findNodeAt(mCursorPosition.x, mCursorPosition.y);
                if (!mSelectedNode) {
                    mSelectedNode = addNode("Node");
                    mSelectedNode->setPosition(mCursorPosition);
                }
            }
        }
    }

    void mouseWheel(SDL_MouseWheelEvent* event) {
        if (mInteraction == Interaction::eNone || mInteraction == Interaction::eZoom) {
            mInteraction = Interaction::eZoom;

            mTargetZoomScale -= static_cast<float_t>(event->y);
            mTargetZoomScale = std::min(std::max(mTargetZoomScale, 1.0F), 5.0F);
            mTargetZoomPoint = mGridOffset + mMousePosition * (mTargetZoomScale - mZoomScale);
        }
    }
};
