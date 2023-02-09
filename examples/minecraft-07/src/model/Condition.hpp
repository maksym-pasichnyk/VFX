#pragma once

#include "Object.hpp"
#include "Predicate.hpp"
#include "block/StateDefinition.hpp"

#include <functional>

struct Block;
struct BlockState;
struct Condition : std::function<Predicate<sp<BlockState>>(const sp<StateDefinition<Block>>&)> {
    using function::function;

    static auto any(std::vector<Condition> conditions) {
        return [conditions = std::move(conditions)](auto& stateDefinition) {
            auto predicates = cxx::iter(conditions)
                .map([&](auto& condition) { return condition(stateDefinition); })
                .collect();
            return [predicates = std::move(predicates)](auto& blockState) {
                return cxx::iter(predicates).any([&](auto& predicate) {
                    return predicate(blockState);
                });
            };
        };
    }

    static auto all(std::vector<Condition> conditions) {
        return [conditions = std::move(conditions)](auto& stateDefinition) {
            auto predicates = cxx::iter(conditions)
                .map([&](auto& condition) { return condition(stateDefinition); })
                .collect();
            return [predicates = std::move(predicates)](auto& blockState) {
                return cxx::iter(predicates).any([&](auto& predicate) {
                    return predicate(blockState);
                });
            };
        };
    }
};
