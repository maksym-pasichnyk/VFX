#pragma once

#include "gfx/Object.hpp"

#include <vector>

struct Scene : gfx::Referencing {
private:
    std::vector<sp<Node>> mNodes;

public:
    explicit Scene(std::vector<sp<Node>> nodes) : mNodes(std::move(nodes)) {}
};
