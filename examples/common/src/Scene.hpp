#pragma once

#include "gfx/Object.hpp"

#include <vector>

struct Scene : gfx::Referencing {
private:
    std::vector<gfx::SharedPtr<Node>> mNodes;

public:
    explicit Scene(std::vector<gfx::SharedPtr<Node>> nodes) : mNodes(std::move(nodes)) {}
};
