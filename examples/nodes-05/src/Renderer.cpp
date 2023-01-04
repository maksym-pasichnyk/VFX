#include <SDL_mouse.h>
#include "Renderer.hpp"
#include "Assets.hpp"

struct GraphView : View {
public:
    struct Node;
    struct Port : gfx::Referencing {
        friend Node;
        friend GraphView;

    private:
        Node* pNode;
        size_t mIndex;
        std::string mName;

    private:
        explicit Port(Node* node, size_t index, std::string name)
            : pNode(node), mIndex(index), mName(std::move(name)) {}
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

        void addInput(std::string name) {
            mInputs.emplace_back(gfx::TransferPtr(new Port(this, mInputs.size(), std::move(name))));
        }

        void addOutput(std::string name) {
            mOutputs.emplace_back(gfx::TransferPtr(new Port(this, mOutputs.size(), std::move(name))));
        }

    private:
        void draw(const gfx::SharedPtr<UIContext> &context, float zoomScale) {
            ImVec2 textSize1 = context->drawList()->_Data->Font->CalcTextSizeA(36.0F, FLT_MAX, FLT_MAX, mText.data(), mText.data() + mText.size(), nullptr);

            context->saveState();
            context->translateBy(mPosition.x / zoomScale, mPosition.y / zoomScale);
            context->setFillColor(UIColor::rgba32(28, 28, 28, 255));
            context->drawRectFilled(mSize / zoomScale, 5.0F / zoomScale);
            context->setFillColor(UIColor(0, 0, 0, 1));
            context->drawRect(mSize / zoomScale, 2.0F, 5.0F);
            context->drawLine(UIPoint(0, 50.0F) / zoomScale, UIPoint(mSize.width, 50.0F) / zoomScale, 2.0F);
            context->setFillColor(UIColor(1, 1, 1, 1));
            context->translateBy(mSize.width * 0.5F / zoomScale, 50.0F * 0.5F / zoomScale);
            context->translateBy(-textSize1.x * 0.5F / zoomScale, -textSize1.y * 0.5F / zoomScale);
            context->drawText(mText, 36.0F / zoomScale);
            context->restoreState();

            for (auto& port : mInputs) {
                UIPoint slot = _getInputSlot(port->mIndex);

                context->saveState();
                context->translateBy(slot.x / zoomScale, slot.y / zoomScale);
                context->translateBy(-10.0F / zoomScale, -10.0F / zoomScale);
                context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
                context->drawCircleFilled(10 / zoomScale);
                context->setFillColor(UIColor::rgba32(37, 150, 190, 255));
                context->drawCircle(10.0F / zoomScale, 2.0F);
                context->restoreState();

                if (!port->mName.empty()) {
                    ImVec2 textSize2 = context->drawList()->_Data->Font->CalcTextSizeA(28.0F, FLT_MAX, FLT_MAX, port->mName.data(), port->mName.data() + port->mName.size(), nullptr);

                    context->saveState();
                    context->translateBy(slot.x / zoomScale, slot.y / zoomScale);
                    context->translateBy(20.0F / zoomScale, -textSize2.y * 0.5F / zoomScale);
                    context->drawText(port->mName, 28.0F / zoomScale);
                    context->restoreState();
                }
            }

            for (auto& port : mOutputs) {
                UIPoint slot = _getOutputSlot(port->mIndex);

                context->saveState();
                context->translateBy(slot.x / zoomScale, slot.y / zoomScale);
                context->translateBy(-10.0F / zoomScale, -10.0F / zoomScale);
                context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
                context->drawCircleFilled(10 / zoomScale);
                context->setFillColor(UIColor::rgba32(37, 150, 190, 255));
                context->drawCircle(10.0F / zoomScale, 2.0F);
                context->restoreState();

                if (!port->mName.empty()) {
                    ImVec2 textSize3 = context->drawList()->_Data->Font->CalcTextSizeA(28.0F, FLT_MAX, FLT_MAX, port->mName.data(), port->mName.data() + port->mName.size(), nullptr);

                    context->saveState();
                    context->translateBy(slot.x / zoomScale, slot.y / zoomScale);
                    context->translateBy(-20.0F / zoomScale, 0.0F);
                    context->translateBy(-textSize3.x / zoomScale, -textSize3.y * 0.5F / zoomScale);
                    context->drawText(port->mName, 28.0F / zoomScale);
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

private:
    auto _size(const ProposedSize &proposed) -> UISize override {
        return proposed.orMax();
    }

    void _draw(const gfx::SharedPtr<UIContext> &context, const UISize& size) override {
        context->saveState();
        context->setFillColor(UIColor::rgba32(45, 45, 45, 255));
        context->drawRectFilled(size);
        context->setFillColor(UIColor(0.0F, 0.0F, 0.0F, 0.25F));

        float_t cellSize = 50.0F / mZoomScale;
        for (int32_t i = 0; i < static_cast<int32_t>(std::floor(static_cast<float_t>(size.width) / cellSize)); ++i) {
            float_t x = static_cast<float_t>(i) * cellSize;
            context->drawLine(UIPoint(x, 0.0F), UIPoint(x, size.height), 2.0F);
        }
        for (int32_t i = 0; i < static_cast<int32_t>(std::floor(static_cast<float_t>(size.height) / cellSize)); ++i) {
            float_t y = static_cast<float_t>(i) * cellSize;
            context->drawLine(UIPoint(0.0F, y), UIPoint(size.width, y), 2.0F);
        }
        context->restoreState();

        context->saveState();
        for (auto& node : mNodes) {
            node->draw(context, mZoomScale);
        }

        for (auto& link : mLinks) {
            UIPoint pointA = link->mPortA->pNode->_getOutputSlot(link->mPortA->mIndex);
            UIPoint pointD = link->mPortB->pNode->_getInputSlot(link->mPortB->mIndex);

            UIPoint pointB = pointA + UIPoint(std::abs(pointD.x - pointA.x), 0.0F);
            UIPoint pointC = pointD - UIPoint(std::abs(pointD.x - pointA.x), 0.0F);

            ImVec2 p0 = ImVec2(pointA.x / mZoomScale, pointA.y / mZoomScale);
            ImVec2 p1 = ImVec2(pointB.x / mZoomScale, pointB.y / mZoomScale);
            ImVec2 p2 = ImVec2(pointC.x / mZoomScale, pointC.y / mZoomScale);
            ImVec2 p3 = ImVec2(pointD.x / mZoomScale, pointD.y / mZoomScale);

            context->drawList()->PathLineTo(p0);
            context->drawList()->PathBezierCubicCurveTo(p1, p2, p3);
            context->drawList()->PathStroke(IM_COL32(255, 255, 255, 255), 0, 2.0F);

            context->saveState();
            context->translateBy(p0.x - 5.0F / mZoomScale, p0.y - 5.0F / mZoomScale);
            context->drawCircleFilled(5.0F / mZoomScale);
            context->restoreState();

            context->saveState();
            context->translateBy(p3.x - 5.0F / mZoomScale, p3.y - 5.0F / mZoomScale);
            context->drawCircleFilled(5.0F / mZoomScale);
            context->restoreState();
        }

        context->restoreState();
    }

public:
    auto zoomScale() -> float_t {
        return mZoomScale;
    }

    void setZoomScale(float_t zoomScale) {
        mZoomScale = std::max(zoomScale, 1.0F);
    }

    auto addNode(std::string text) -> gfx::SharedPtr<Node> {
        return mNodes.emplace_back(gfx::TransferPtr(new GraphView::Node(std::move(text))));
    }

    void addLink(gfx::SharedPtr<Node> nodeA, gfx::SharedPtr<Node> nodeB, size_t slotA, size_t slotB) {
        auto portA = nodeA->mOutputs.at(slotA);
        auto portB = nodeB->mInputs.at(slotB);

        mLinks.emplace_back(gfx::TransferPtr(new GraphView::Link(std::move(portA), std::move(portB))));
    }

    auto findNodeAt(int32_t x, int32_t y) -> gfx::SharedPtr<Node> {
        for (auto& node : mNodes) {
            float_t x1 = x * mZoomScale - node->mPosition.x;
            float_t y1 = y * mZoomScale - node->mPosition.y;

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
};

Renderer::Renderer(gfx::SharedPtr<gfx::Device> device_) : device(std::move(device_)) {
    commandQueue = device->newCommandQueue();
    commandBuffer = commandQueue->commandBuffer();

    mGuiRenderer = gfx::TransferPtr(new UIRenderer(device));
    mGraphView = gfx::TransferPtr(new GraphView());

    auto nodeA = mGraphView->addNode("Node A");
    nodeA->addOutput("");
    nodeA->setPosition(UIPoint(125, 110));

    auto nodeB = mGraphView->addNode("Node B");
    nodeB->addOutput("");
    nodeB->setPosition(UIPoint(125, 710));

    auto nodeC = mGraphView->addNode("Node C");
    nodeC->addInput("A");
    nodeC->addInput("B");
    nodeC->addOutput("Result");
    nodeC->setPosition(UIPoint(825, 330));

    mGraphView->addLink(nodeA, nodeC, 0, 0);
    mGraphView->addLink(nodeB, nodeC, 0, 1);

    mGraphView->setZoomScale(2.0F);
}

void Renderer::update(float_t dt) {
//    int32_t x;
//    int32_t y;
//    uint32_t buttonState = SDL_GetMouseState(&x, &y);
//    if ((buttonState & SDL_BUTTON_LMASK) != 0) {
//        if (auto node = mGraphView->findNodeAt(x, y)) {
//            fmt::print("Clicked {}\n", node->text());
//        }
//    }
}

void Renderer::draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain) {
    auto drawable = swapchain->nextDrawable();
    auto drawableSize = swapchain->drawableSize();

    vk::Rect2D rendering_area = {};
    rendering_area.setOffset(vk::Offset2D{0, 0});
    rendering_area.setExtent(drawableSize);

    vk::Viewport rendering_viewport = {};
    rendering_viewport.setWidth(static_cast<float_t>(drawableSize.width));
    rendering_viewport.setHeight(static_cast<float_t>(drawableSize.height));
    rendering_viewport.setMinDepth(0.0f);
    rendering_viewport.setMaxDepth(1.0f);

    gfx::RenderingInfo rendering_info = {};
    rendering_info.setRenderArea(rendering_area);
    rendering_info.setLayerCount(1);
    rendering_info.colorAttachments()[0].setTexture(drawable->texture());
    rendering_info.colorAttachments()[0].setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
    rendering_info.colorAttachments()[0].setLoadOp(vk::AttachmentLoadOp::eClear);
    rendering_info.colorAttachments()[0].setStoreOp(vk::AttachmentStoreOp::eStore);

    commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

    commandBuffer->beginRendering(rendering_info);
    commandBuffer->setScissor(0, rendering_area);
    commandBuffer->setViewport(0, rendering_viewport);

    auto ctx = gfx::TransferPtr(new UIContext(mGuiRenderer->drawList()));

    auto body = mGraphView->frame(mScreenSize.width, mScreenSize.height);
    mGuiRenderer->resetForNewFrame();
    body->draw(ctx, body->size(ProposedSize(mScreenSize)));
    mGuiRenderer->draw(commandBuffer);

    commandBuffer->endRendering();

    commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
    commandBuffer->end();
    commandBuffer->submit();
    commandBuffer->present(drawable);
    commandBuffer->waitUntilCompleted();
}

void Renderer::screenResized(const vk::Extent2D& size) {
    mScreenSize = UISize(static_cast<float_t>(size.width), static_cast<float_t>(size.height));
    mGuiRenderer->setScreenSize(mScreenSize);
}

