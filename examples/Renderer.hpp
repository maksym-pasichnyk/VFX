#pragma once

#include "Core.hpp"

struct Window;
struct GameObject;
struct ImGuiRenderer;
struct Renderer {
    enum class Example {
        SDF,
        Cube
    };

public:
    Renderer(
        const Arc<vfx::Device>& device,
        const Arc<vfx::Layer>& layer,
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
    Arc<vfx::Device> device{};
    Arc<vfx::Layer> layer{};

    Arc<ImGuiRenderer> imgui{};

    Arc<vfx::CommandQueue> commandQueue{};
    Arc<vfx::PipelineState> sdfPipelineState{};
    Arc<vfx::PipelineState> cubePipelineState{};
    Arc<vfx::PipelineState> presentPipelineState{};

    Arc<vfx::Sampler> sampler{};
    Arc<vfx::Texture> colorAttachmentTexture{};
    Arc<vfx::Texture> depthAttachmentTexture{};

    Arc<vfx::ResourceGroup> sdfResourceGroup{};
    Arc<vfx::ResourceGroup> cubeResourceGroup{};
    Arc<vfx::ResourceGroup> presentResourceGroup{};

    Arc<GameObject> gameObject{};

    SceneConstants sceneConstants{};
    Arc<vfx::Buffer> sceneConstantsBuffer{};

    Example example = Example::Cube;

    bool enableHDR = true;
    f32 exposure = 1.0f;
    f32 gamma = 2.2f;
};
