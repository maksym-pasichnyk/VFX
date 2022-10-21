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
    vk::UniqueDescriptorPool sdfPipelineStateDescriptorPool{};
    vk::DescriptorSet sdfPipelineStateDescriptorSet{};

    vk::UniqueDescriptorPool cubePipelineStateDescriptorPool{};
    vk::DescriptorSet cubePipelineStateDescriptorSet{};

    vk::UniqueDescriptorPool presentPipelineStateDescriptorPool{};
    std::vector<vk::DescriptorSet> presentPipelineStateDescriptorSets{};

    Arc<GameObject> gameObject{};

    SceneConstants sceneConstants{};
    Arc<vfx::Buffer> sceneConstantsBuffer{};

    ModelConstants modelConstants{};

    Example example = Example::Cube;

    bool enableHDR = true;
    f32 exposure = 1.0f;
    f32 gamma = 2.2f;
};
