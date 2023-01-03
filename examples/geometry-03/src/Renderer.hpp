#pragma once

#include "gfx/GFX.hpp"
#include "glm/glm.hpp"

struct Node;
struct Skin;
struct Mesh;
struct Scene;
struct Animation;

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

private:
    void buildShaders();
    void buildBuffers();

public:
    void update(float_t dt);
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void screenResized(const vk::Extent2D& size);

private:
    glm::f32vec2 screenSize = {};
    glm::mat4x4 g_proj_matrix = {};
    glm::mat4x4 g_view_matrix = {};

    std::vector<gfx::SharedPtr<Skin>> skins = {};
    std::vector<gfx::SharedPtr<Mesh>> meshes = {};
    std::vector<gfx::SharedPtr<Scene>> scenes = {};
    std::vector<gfx::SharedPtr<Animation>> animations = {};

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;
    gfx::SharedPtr<gfx::RenderPipelineState> renderPipelineState;
};
