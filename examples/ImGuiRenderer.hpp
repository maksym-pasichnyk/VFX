#pragma once

#include "Core.hpp"
#include "Application.hpp"

struct ImDrawData;
struct ImGuiContext;
struct ImGuiRenderer {
public:
    ImGuiRenderer(const Arc<vfx::Context>& context, const Arc<Window>& window);
    ~ImGuiRenderer();

public:
    void beginFrame();
    void endFrame();
    void draw(vfx::CommandBuffer* cmd);

private:
    void createFontTexture();
    void createPipelineState();
    void setupRenderState(ImDrawData* data, vfx::CommandBuffer* cmd, const Arc<vfx::Mesh>& mesh, i32 width, i32 height);

private:
    ImGuiContext* ctx;
    Arc<vfx::Context> context{};
    Arc<vfx::Sampler> fontSampler{};
    Arc<vfx::Texture> fontTexture{};
    Arc<vfx::PipelineState> pipelineState{};

    // todo: bindless
    vk::UniqueDescriptorPool descriptor_pool{};
    std::vector<vk::DescriptorSet> descriptor_sets{};
};
