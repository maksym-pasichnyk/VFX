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
    ModelConstants constants{};

public:
    explicit GameObject(const Arc<vfx::Context>& context);

public:
    void draw(vfx::CommandBuffer* cmd) override;
};