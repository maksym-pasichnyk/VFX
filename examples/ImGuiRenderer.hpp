#pragma once

#include "Core.hpp"
#include "Mesh.hpp"
#include "Application.hpp"

struct ImDrawData;
struct ImGuiContext;
struct ImGuiRenderer {
public:
    ImGuiRenderer(const Arc<vfx::Device>& device, const Arc<Window>& window);
    ~ImGuiRenderer();

public:
    void beginFrame();
    void endFrame();
    void draw(vfx::CommandBuffer* cmd);

private:
    void createFontTexture();
    void createPipelineState();
    void setupRenderState(ImDrawData* data, vfx::CommandBuffer* cmd, const Arc<Mesh>& mesh, i32 width, i32 height);

private:
    ImGuiContext* ctx;
    Arc<vfx::Device> device{};
    Arc<vfx::Sampler> fontSampler{};
    Arc<vfx::Texture> fontTexture{};
    Arc<vfx::PipelineState> pipelineState{};
    Arc<vfx::ResourceGroup> resourceGroup{};
};
