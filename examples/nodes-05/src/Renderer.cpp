#include "Renderer.hpp"
#include "Assets.hpp"

#include "NotSwiftUI/View.hpp"

struct UINode;
struct UINodeEditor;
struct UINodeConnection : gfx::Referencing {
    friend UINodeEditor;

private:
    size_t mIndexA = 0;
    size_t mIndexB = 0;
    gfx::SharedPtr<UINode> mNodeA = {};
    gfx::SharedPtr<UINode> mNodeB = {};

public:
    UINodeConnection(int32_t indexA, size_t indexB, gfx::SharedPtr<UINode> nodeA, gfx::SharedPtr<UINode> nodeB)
        : mIndexA(indexA), mIndexB(indexB), mNodeA(std::move(nodeA)), mNodeB(std::move(nodeB)) {}
};

struct UINode : View {
    friend UINodeEditor;

private:
    UISize mSize = UISize(350.0F, 250.0F);
    UIPoint mPosition = UIPoint(0.0F, 0.0F);

    size_t mInputs = {};
    size_t mOutputs = {};
    std::string mText = "Node";

public:
    explicit UINode(size_t inputs, size_t outputs)
        : mInputs(inputs), mOutputs(outputs) {}

public:
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
    
private:
    auto _size(const ProposedSize &proposed) -> UISize override {
        return mSize;
    }

    void _draw(const gfx::SharedPtr<UIContext> &context, const UISize& size) override {
        context->saveState();
        context->translateBy(mPosition.x, mPosition.y);
        context->setFillColor(UIColor::rgba32(28, 28, 28, 255));
        context->drawRectFilled(size, 5.0F);
        context->setFillColor(UIColor(0, 0, 0, 1));
        context->drawRect(size, 2.0F, 5.0F);
        context->drawLine(UIPoint(0, 50.0F), UIPoint(size.width, 50.0F), 2.0F);
        context->setFillColor(UIColor(1, 1, 1, 1));

        ImVec2 textSize = context->drawList()->_Data->Font->CalcTextSizeA(36.0F, FLT_MAX, FLT_MAX, mText.data(), mText.data() + mText.size(), nullptr);

        context->translateBy((size.width - textSize.x) * 0.5F, (50.0F - textSize.y) * 0.5F);
        context->drawText(mText, 36.0F);
        context->restoreState();

        float_t currentOutputX = mPosition.x + size.width - 20.0F - 10.0F;
        float_t currentOutputY = mPosition.y + 15.0F + 50.0F;
        for (size_t i : ranges::views::iota(0zu, mOutputs)) {
            context->saveState();
            context->translateBy(currentOutputX, currentOutputY);
            context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
            context->drawCircleFilled(10);
            context->setFillColor(UIColor::rgba32(37, 150, 190, 255));
            context->drawCircle(10.0F, 2.0F);
            context->restoreState();
            currentOutputY += 30.0F;
        }

        float_t currentInputX = mPosition.x + 10.0F;
        float_t currentInputY = mPosition.y + 15.0F + 50.0F;
        for (size_t i : ranges::views::iota(0zu, mInputs)) {
            context->saveState();
            context->translateBy(currentInputX, currentInputY);
            context->setFillColor(UIColor::rgba32(0, 0, 0, 255));
            context->drawCircleFilled(10);
            context->setFillColor(UIColor::rgba32(37, 150, 190, 255));
            context->drawCircle(10.0F, 2.0F);
            context->restoreState();
            currentInputY += 30.0F;
        }
    }

    auto getInputPosition(size_t index) -> std::optional<UIPoint> {
        if (index >= mInputs) {
            return std::nullopt;
        }
        float_t x = mPosition.x + 10.0F + 10.0F;
        float_t y = mPosition.y + 10.0F + 15.0F + 50.0F;
        return UIPoint(x, y + 30.0F * index);
    }

    auto getOutputPosition(size_t index) -> std::optional<UIPoint> {
        if (index >= mOutputs) {
            return std::nullopt;
        }
        float_t x = mPosition.x + 10.0F + mSize.width - 20.0F - 10.0F;
        float_t y = mPosition.y + 10.0F + 15.0F + 50.0F;
        return UIPoint(x, y + 30.0F * index);
    }
};

struct UINodeEditor : View {
private:
    std::list<gfx::SharedPtr<UINode>> mNodes = {};
    std::list<gfx::SharedPtr<UINodeConnection>> mConnections = {};

public:
    void addNode(const gfx::SharedPtr<UINode>& node) {
        auto it = std::find(mNodes.begin(), mNodes.end(), node);
        if (it == mNodes.end()) {
            mNodes.emplace_back(node);
        }
    }

    void removeNode(const gfx::SharedPtr<UINode>& node) {
        // todo: remove connections

        auto it = std::find(mNodes.begin(), mNodes.end(), node);
        if (it != mNodes.end()) {
            mNodes.erase(it);
        }
    }

    void addConnection(gfx::SharedPtr<UINode> nodeA, gfx::SharedPtr<UINode> nodeB, int32_t indexA, size_t indexB) {
        // todo: check connections
        mConnections.emplace_back(gfx::TransferPtr(new UINodeConnection(indexA, indexB, nodeA, nodeB)));
    }

private:
    void _draw(const gfx::SharedPtr<UIContext> &context, const UISize& size) override {
        context->saveState();
        context->setFillColor(UIColor::rgba32(45, 45, 45, 255));
        context->drawRectFilled(size);
        context->setFillColor(UIColor(0.0F, 0.0F, 0.0F, 0.25F));

        float_t cellSize = 50.0F;
        for (int32_t i = 0; i < static_cast<int32_t>(std::floor(static_cast<float_t>(size.width) / cellSize)); ++i) {
            float_t x = static_cast<float_t>(i) * cellSize;
            context->drawLine(UIPoint(x, 0.0F), UIPoint(x, size.height), 2.0F);
        }
        for (int32_t i = 0; i < static_cast<int32_t>(std::floor(static_cast<float_t>(size.height) / cellSize)); ++i) {
            float_t y = static_cast<float_t>(i) * cellSize;
            context->drawLine(UIPoint(0.0F, y), UIPoint(size.width, y), 2.0F);
        }
        context->restoreState();

        for (auto& node : mNodes) {
            node->draw(context, node->View::size(ProposedSize(size)));
        }

        for (auto& connection : mConnections) {
            UIPoint pointA = connection->mNodeA->getOutputPosition(connection->mIndexA).value();
            UIPoint pointD = connection->mNodeB->getInputPosition(connection->mIndexB).value();

            float_t distance = std::abs(pointD.x - pointA.x);

            UIPoint pointB = pointA + UIPoint(distance, 0.0F);
            UIPoint pointC = pointD - UIPoint(distance, 0.0F);

            ImVec2 p0 = ImVec2(pointA.x, pointA.y);
            ImVec2 p1 = ImVec2(pointB.x, pointB.y);
            ImVec2 p2 = ImVec2(pointC.x, pointC.y);
            ImVec2 p3 = ImVec2(pointD.x, pointD.y);

            context->drawList()->PathLineTo(p0);
            context->drawList()->PathBezierCubicCurveTo(p1, p2, p3);
            context->drawList()->PathStroke(IM_COL32(255, 255, 255, 255), 0, 2.0F);
        }
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        return proposed.orMax();
    }
};

Renderer::Renderer(gfx::SharedPtr<gfx::Device> device_) : device(std::move(device_)) {
    commandQueue = device->newCommandQueue();
    commandBuffer = commandQueue->commandBuffer();

    mGuiRenderer = gfx::TransferPtr(new UIRenderer(device));
    mNodeEditor = gfx::TransferPtr(new UINodeEditor());

    auto nodeA = gfx::TransferPtr(new UINode(0, 1));
    nodeA->setText("Node A");
    nodeA->setPosition(UIPoint(125, 110));
    mNodeEditor->addNode(nodeA);

    auto nodeB = gfx::TransferPtr(new UINode(0, 1));
    nodeB->setText("Node B");
    nodeB->setPosition(UIPoint(125, 710));
    mNodeEditor->addNode(nodeB);

    auto nodeC = gfx::TransferPtr(new UINode(2, 0));
    nodeC->setText("Node C");
    nodeC->setPosition(UIPoint(825, 330));
    mNodeEditor->addNode(nodeC);

    mNodeEditor->addConnection(nodeA, nodeC, 0, 0);
    mNodeEditor->addConnection(nodeB, nodeC, 0, 1);
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

    mGuiRenderer->resetForNewFrame();
    mNodeEditor->draw(ctx, mScreenSize);
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

