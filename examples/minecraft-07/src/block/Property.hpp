#pragma once

#include "Object.hpp"

#include <set>
#include <string>

struct Property : Object {
    std::string name;
    std::set<std::string> values = {};

    explicit Property(std::string name, std::set<std::string> values) : name(std::move(name)), values(std::move(values)) {}

    auto getName() const -> const std::string& {
        return name;
    }

    auto getPossibleValues() const -> const std::set<std::string>& {
        return values;
    }
};