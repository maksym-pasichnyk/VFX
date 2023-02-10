#pragma once

#include "Registry.hpp"

template<typename T>
struct Holder : Object {
private:
    sp<Registry<T>> registry;
    std::string key;
    T value;

public:
    explicit Holder(sp<Registry<T>> registry, std::string key, T value) : registry(std::move(registry)), key(std::move(key)), value(std::move(value)) {}

    auto getKey() const -> const std::string& {
        return key;
    }

    auto getValue() const -> const T& {
        return value;
    }
};