#pragma once

#include "Object.hpp"

#include <vector>

struct Scene : ManagedObject<Scene> {
private:
    std::vector<ManagedShared<Node>> mNodes;

public:
    explicit Scene(std::vector<ManagedShared<Node>> nodes) : mNodes(std::move(nodes)) {}
};
