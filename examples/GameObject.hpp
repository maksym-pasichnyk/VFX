#pragma once

#include "Core.hpp"
#include "Mesh.hpp"

struct GameObject {
private:
    Arc<Mesh> mesh{};

public:
    explicit GameObject(const Arc<vfx::Context>& context);

    void render(vfx::CommandBuffer* cmd);
};