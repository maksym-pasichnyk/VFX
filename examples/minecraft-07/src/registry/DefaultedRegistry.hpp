#pragma once

#include "Object.hpp"

#include <map>
#include <string>

template<typename T>
struct Registry : Object {
    virtual auto get(const std::string& key) const -> std::optional<T> = 0;
    virtual auto getKey(const T& value) const -> std::optional<std::string> = 0;
};

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

template<typename T>
struct MappedRegistry : Registry<T> {
protected:
    std::vector<sp<Holder<T>>> holders = {};
    std::map<T, sp<Holder<T>>> byValue = {};
    std::map<std::string, sp<Holder<T>>> byKey = {};

public:
    auto get(const std::string& key) const -> std::optional<T> override {
        auto it = byKey.find(key);
        if (it != byKey.end()) {
            return it->second->getValue();
        }
        return std::nullopt;
    }

    auto getKey(const T& value) const -> std::optional<std::string> override {
        auto it = byValue.find(value);
        if (it != byValue.end()) {
            return it->second->getKey();
        }
        return std::nullopt;
    }

    virtual void registerMapping(const std::string& key, const T& value) {
        if (byKey.contains(key)) {
            throw std::runtime_error("Adding duplicate key '" + key + "' to registry");
        }
        if (byValue.contains(value)) {
            throw std::runtime_error("Adding duplicate value '" + key + "' to registry");
        }

        auto holder = getOrCreateHolder(key, value);

        byKey.emplace(key, holder);
        byValue.emplace(value, holder);
        holders.emplace_back(holder);
    }

    auto getOrCreateHolder(const std::string& key, const T& value) -> sp<Holder<T>> {
        return sp<Holder<T>>::of(RetainPtr(this), key, value);
    }

    auto values() {
        return cxx::iter(holders).map(std::mem_fn(&Holder<T>::getValue));
    }
};

template<typename T>
struct DefaultedRegistry : MappedRegistry<T> {
private:
    std::string defaultKey;
    T defaultValue;

    explicit DefaultedRegistry(std::string defaultKey, T defaultValue) : defaultKey(std::move(defaultKey)), defaultValue(std::move(defaultValue)) {}

    auto get(const T& value) const -> std::optional<T> override {
        return MappedRegistry<T>::get(value).value_or(defaultValue);
    }

    auto getKey(const T& value) const -> std::optional<std::string> override {
        return MappedRegistry<T>::getKey(value).value_or(defaultKey);
    }
};