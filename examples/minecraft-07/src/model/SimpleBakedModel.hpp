#pragma once

#include "BakedModel.hpp"

struct SimpleBakedModel : BakedModel {
    struct Builder;

private:
    bool hasAmbientOcclusion = false;
    std::vector<BakedQuad> unculledFaces{};
    std::array<std::vector<BakedQuad>, 6> culledFaces{};

public:
    SimpleBakedModel(std::vector<BakedQuad> unculledFaces, std::array<std::vector<BakedQuad>, 6> culledFaces, bool hasAmbientOcclusion)
        : hasAmbientOcclusion(hasAmbientOcclusion), unculledFaces(std::move(unculledFaces)), culledFaces(std::move(culledFaces)) {}

    void drawQuads(std::optional<Direction> facing, const Consumer& consumer) override {
        if (facing.has_value()) {
            cxx::iter(culledFaces.at(size_t(*facing))).for_each(consumer);
        } else {
            cxx::iter(unculledFaces).for_each(consumer);
        }
    }
};

struct SimpleBakedModel::Builder {
    bool hasAmbientOcclusion = false;
    std::vector<BakedQuad> unculledFaces{};
    std::array<std::vector<BakedQuad>, 6> culledFaces{};

    explicit Builder(bool hasAmbientOcclusion) : hasAmbientOcclusion(hasAmbientOcclusion) {}

    auto addCulledFace(Direction direction, BakedQuad quad) -> Builder& {
        culledFaces.at(size_t(direction)).emplace_back(quad);
        return *this;
    }

    auto addUnculledFace(BakedQuad quad) -> Builder& {
        unculledFaces.emplace_back(quad);
        return *this;
    }

    auto build() -> sp<BakedModel> {
        return sp<SimpleBakedModel>::of(unculledFaces, culledFaces, hasAmbientOcclusion);
    }
};
