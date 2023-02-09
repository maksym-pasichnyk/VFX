#pragma once

#include "Iter.hpp"
#include "Object.hpp"
#include "block/Property.hpp"
#include "block/BlockState.hpp"

#include <map>
#include <list>
#include <string>

template<typename O>
struct StateDefinition : Object {
    struct Builder;

    sp<O> owner;
    sp<BlockState> defaultState = {};
    std::list<sp<BlockState>> states = {};
    std::map<std::string, sp<Property>> properties = {};

    explicit StateDefinition(const sp<O>& owner, std::map<std::string, sp<Property>> _properties) : owner(owner), properties(std::move(_properties)) {
        auto stream = std::vector{std::list<std::pair<sp<Property>, std::string>>{}};

        for (auto& property : ranges::views::values(properties)) {
            stream = cxx::iter(stream)
                .flatmap([&](auto& values) {
                    return cxx::iter(property->getPossibleValues()).map([&](auto& value) {
                        auto copy = values;
                        copy.emplace_back(property, value);
                        return copy;
                    });
                })
                .collect();
        }

        std::map<std::map<sp<Property>, std::string>, sp<BlockState>> map = {};

        for (auto& item : stream) {
            auto values = cxx::iter(item).to<std::map<sp<Property>, std::string>>();
            auto state = sp<BlockState>::of(owner, values);

            map.emplace(values, state);
            states.emplace_back(state);
        }

        for (auto& state : states) {
            state->populateNeighbours(map);
        }

        defaultState = states.front();
    }

    auto getDefaultState() const -> const sp<BlockState>& {
        return defaultState;
    }

    auto getPossibleStates() const -> const std::list<sp<BlockState>>& {
        return states;
    }

    auto getProperty(const std::string& name) -> const sp<Property>& {
        return properties.at(name);
    }
};

template<typename O>
struct StateDefinition<O>::Builder {
    sp<O> owner;
    std::map<std::string, sp<Property>> properties = {};

    explicit Builder(const sp<O>& owner) : owner(owner) {}

    auto add(const sp<Property>& property) -> Builder& {
        if (properties.contains(property->getName())) {
            throw std::runtime_error("Property is already exist: " + property->getName());
        }
        properties.emplace(property->getName(), property);
        return *this;
    }

    auto build() -> sp<StateDefinition> {
        return sp<StateDefinition>::of(owner, properties);
    }
};