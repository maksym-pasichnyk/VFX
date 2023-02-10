#pragma once

#include <map>
#include <string>
#include <vector>
#include <variant>
#include <concepts>

#include "Enum.hpp"

struct JsonElement;

struct JsonArray : std::vector<JsonElement> {
    using vector::vector;
};

struct JsonObject : std::map<std::string, JsonElement> {
    using map::map;

    template<typename U>
    auto value_or(const std::string& key, U&& u) const -> U;
};

struct JsonElement : Enum<long long, double, bool, std::string, JsonArray, JsonObject> {
    using Enum::Enum;

    template<typename T>
    [[nodiscard]] auto get() const -> T {
        if constexpr (std::same_as<T, bool>) {
            return as<bool>();
        } else if constexpr (std::floating_point<T> or std::integral<T>) {
            if (is<double>()) {
                return static_cast<T>(as<double>());
            }
            return static_cast<T>(as<long long>());
        } else {
            return as<T>();
        }
    }

    [[nodiscard]] auto contains(const std::string& key) const -> bool {
        return is<JsonObject>() and as<JsonObject>().contains(key);
    }

    [[nodiscard]] auto at(const std::string& key) -> JsonElement& {
        return as<JsonObject>().at(key);
    }

    [[nodiscard]] auto at(const std::string& key) const -> const JsonElement& {
        return as<JsonObject>().at(key);
    }
};

template<typename U>
inline auto JsonObject::value_or(const std::string& key, U&& u) const -> U {
    auto it = find(key);
    if (it != end()) {
        return it->second.get<U>();
    }
    return std::forward<U>(u);
}