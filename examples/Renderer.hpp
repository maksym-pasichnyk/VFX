#pragma once

#include "Core.hpp"

struct Window;
struct ImGuiRenderer;
struct GameObject;
struct Renderer {
    enum class Example {
        SDF,
        Cube
    };

    struct Globals {
        vfx::float4x4 ViewMatrix;
        vfx::float4x4 ProjectionMatrix;
        vfx::float4x4 ViewProjectionMatrix;
        vfx::float4x4 InverseViewProjectionMatrix;
        vfx::float3   CameraPosition;
        vfx::int2     Resolution;
        vfx::float1   Time;

        // todo: move to per-object data
        vfx::float4x4 ModelMatrix;
    };

public:
    Renderer(
        const Arc<vfx::Context>& context,
        const Arc<vfx::Swapchain>& swapchain,
        const Arc<Window>& window
    );

    void draw();
    void update();
    void resize();

private:
    void createSampler();
    void createAttachments();
    void createSDFPipelineState();
    void createCubePipelineState();
    void createPresentPipelineState();
    void updateAttachmentDescriptors();

    void drawGui();
    void encodePresent(vfx::CommandBuffer* cmd, vfx::Drawable* drawable);

public:
    Arc<vfx::Context> context{};
    Arc<vfx::Swapchain> swapchain{};
    Arc<ImGuiRenderer> imgui{};

    Arc<vfx::CommandQueue> commandQueue{};
    Arc<vfx::PipelineState> sdfPipelineState{};
    Arc<vfx::PipelineState> cubePipelineState{};
    Arc<vfx::PipelineState> presentPipelineState{};

    Arc<vfx::Sampler> sampler{};
    Arc<vfx::Texture> colorAttachmentTexture{};
    Arc<vfx::Texture> depthAttachmentTexture{};

    // todo: bindless
    vk::UniqueDescriptorPool descriptor_pool{};
    std::vector<vk::DescriptorSet> descriptor_sets{};

    Arc<GameObject> gameObject{};

    Globals globals{};
    Example example = Example::Cube;

    bool enableHDR = true;
    f32 exposure = 1.0f;
    f32 gamma = 2.2f;
};
