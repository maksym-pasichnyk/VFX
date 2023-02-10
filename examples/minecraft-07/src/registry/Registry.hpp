#pragma once

#include "Object.hpp"

#include <string>
#include <optional>

template<typename T>
struct Registry : Object {
    virtual auto get(const std::string& key) const -> std::optional<T> = 0;
    virtual auto getKey(const T& value) const -> std::optional<std::string> = 0;
};