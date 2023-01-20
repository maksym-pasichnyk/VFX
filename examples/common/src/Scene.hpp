#pragma once

#include "Object.hpp"

#include <vector>

struct Scene : Object {
private:
    std::vector<sp<Node>> mNodes;

public:
    explicit Scene(std::vector<sp<Node>> nodes) : mNodes(std::move(nodes)) {}
};
