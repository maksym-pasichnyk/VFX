#pragma once

#include "Iter.hpp"

#include <string>
#include <vector>

static auto getResourceLocation(const std::string& name) -> std::string {
    auto parts = cxx::iter(name).split(':').map(ranges::to<std::string>()).collect();
    if (parts.empty()) {
        throw std::runtime_error("can't get resource location");
    }
    if (parts.size() == 1) {
        return parts.at(0);
    }
    return parts.at(1);
}