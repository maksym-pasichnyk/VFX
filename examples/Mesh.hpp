#pragma once

#include "Core.hpp"

struct Primitive {
    u64 indexCount = 0;
    u64 indexOffset = 0;
};

struct Mesh {
    u64 indexCount = 0;
    u64 vertexCount = 0;

    Arc<vfx::Buffer> indexBuffer = {};
    Arc<vfx::Buffer> vertexBuffer = {};
};