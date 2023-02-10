#pragma once

#include "Holder.hpp"
#include "Registry.hpp"
#include "MappedRegistry.hpp"

template<typename T>
struct DefaultedRegistry : MappedRegistry<T> {
private:
    std::string defaultKey;
    T defaultValue;

public:
    explicit DefaultedRegistry(std::string defaultKey, T defaultValue) : defaultKey(std::move(defaultKey)), defaultValue(std::move(defaultValue)) {}

    auto get(const T& value) const -> std::optional<T> override {
        return MappedRegistry<T>::get(value).value_or(defaultValue);
    }

    auto getKey(const T& value) const -> std::optional<std::string> override {
        return MappedRegistry<T>::getKey(value).value_or(defaultKey);
    }
};