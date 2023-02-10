#pragma once

#include <map>
#include <set>
#include <string>

#include "Material.hpp"
#include "StateDefinition.hpp"

struct Block : Object {
    sp<Material> material;
    sp<StateDefinition<Block>> stateDefinition = {};

    explicit Block(sp<Material> material) : material(std::move(material)) {}

    void createBlockStateDefinition(const std::vector<sp<Property>>& properties) {
        auto builder = StateDefinition<Block>::Builder(RetainPtr(this));
        cxx::iter(properties).for_each(std::bind_front(&decltype(builder)::add, &builder));
        stateDefinition = builder.build();
    }

    auto getStateDefinition() const -> const sp<StateDefinition<Block>>& {
        return stateDefinition;
    }

    auto getDefaultState() const -> const sp<BlockState>& {
        return stateDefinition->getDefaultState();
    }

    auto getMaterial() const -> const sp<Material>& {
        return material;
    }
};