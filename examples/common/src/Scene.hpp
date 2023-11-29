#pragma once

#include "Object.hpp"

#include <vector>

struct Scene : public ManagedObject {
private:
    std::vector<rc<Node>> mNodes;

public:
    explicit Scene(std::vector<rc<Node>> nodes) : mNodes(std::move(nodes)) {}
};
