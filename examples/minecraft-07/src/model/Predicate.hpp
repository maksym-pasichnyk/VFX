#pragma once

#include <functional>

template<typename T>
struct Predicate : std::function<bool(const T&)> {
    using std::function<bool(const T&)>::function;
};