#pragma once

#include "BakedModel.hpp"

struct WeightedBakedModel final : BakedModel {
    struct Builder;

private:
    std::vector<std::pair<sp<BakedModel>, int32_t>> entries;

public:
    explicit WeightedBakedModel(std::vector<std::pair<sp<BakedModel>, int32_t>> entries)
        : entries(std::move(entries)) {}

    void drawQuads(std::optional<Direction> facing, const Consumer& consumer) override {
        entries.at(0).first->drawQuads(facing, consumer);
    }
};

struct WeightedBakedModel::Builder {
    std::vector<std::pair<sp<BakedModel>, int32_t>> entries = {};

    auto add(sp<BakedModel> model, int32_t weight) -> Builder& {
        entries.emplace_back(std::move(model), weight);
        return *this;
    }

    auto build() -> sp<BakedModel> {
        if (entries.empty()) {
            return {};
        }
        if (entries.size() == 1) {
            return entries.at(0).first;
        }
        return sp<WeightedBakedModel>::of(entries);
    }
};