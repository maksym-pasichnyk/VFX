#pragma once

#include "Object.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <list>
#include <vector>

struct Node : Object {
private:
    Node* mParent = {};

    sp<Skin> mSkin;
    sp<Mesh> mMesh;
    std::list<sp<Node>> mChildren = {};

    glm::vec3 mPosition;
    glm::quat mRotation;
    glm::vec3 mScale;

public:
    explicit Node(sp<Skin> skin, sp<Mesh> mesh, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
        : mSkin(std::move(skin)), mMesh(std::move(mesh)), mPosition(position), mRotation(rotation), mScale(scale) {}

    auto transform() -> glm::mat4 {
        return glm::scale(glm::translate(glm::mat4(1.0f), mPosition) * glm::mat4_cast(mRotation), mScale);
    }

    void setParent(const sp<Node>& node) {
        if (mParent == node.get() || node.get() == this) {
            return;
        }
        if (mParent) {
            erase(mParent->mChildren, RetainPtr(this));
        }
        mParent = node.get();
        if (mParent) {
            mParent->mChildren.emplace_back(RetainPtr(this));
        }
    }
};
