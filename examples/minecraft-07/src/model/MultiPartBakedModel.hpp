#pragma once

#include "Object.hpp"
#include "Predicate.hpp"
#include "BakedModel.hpp"
#include "block/BlockState.hpp"

struct MultiPartBakedModel : BakedModel {
    struct Builder;

    std::vector<std::pair<Predicate<sp<BlockState>>, sp<BakedModel>>> selectors = {};

    void drawQuads(std::optional<Direction> facing, const Consumer& consumer) override {

    }
};

struct MultiPartBakedModel::Builder {
    std::vector<std::pair<Predicate<sp<BlockState>>, sp<BakedModel>>> selectors = {};

    auto add(Predicate<sp<BlockState>> predicate, sp<BakedModel> baked) -> Builder& {
        selectors.emplace_back(std::move(predicate), std::move(baked));
        return *this;
    }

    auto build() -> sp<BakedModel> {
        return sp<MultiPartBakedModel>::of();
    }
};