#pragma once

#include "Object.hpp"
#include "glm/glm.hpp"

#include <vector>

struct Skin : public ManagedObject {
private:
    int32_t mSkeleton;
    std::vector<int32_t> mJoints;
    std::vector<glm::mat4x4> mInverseBindMatrices;

public:
    explicit Skin(int32_t skeleton, std::vector<int32_t> joints, std::vector<glm::mat4x4> inverseBindMatrices)
        : mSkeleton(skeleton), mJoints(std::move(joints)), mInverseBindMatrices(std::move(inverseBindMatrices)) {}
};
