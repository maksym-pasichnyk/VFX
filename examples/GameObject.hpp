#pragma once

#include "Core.hpp"
#include "Mesh.hpp"

struct Renderer;
struct Renderable {
    virtual ~Renderable() = default;

    virtual void draw(vfx::CommandBuffer* cmd) = 0;
};

struct GameObject : Renderable {
private:
    Arc<Mesh> mesh{};
    glm::vec3 position = {};
    glm::vec3 rotation = {};
    glm::vec3 scale    = {};

public:
    explicit GameObject(const Arc<vfx::Device>& device);

public:
    void draw(vfx::CommandBuffer* cmd) override;
    auto getTransformMatrix() const -> glm::mat4x4;
};