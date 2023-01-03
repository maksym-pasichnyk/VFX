#pragma once

#include "gfx/Object.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <vector>

struct Node : gfx::Referencing {
private:
    Node* mParent = {};

    gfx::SharedPtr<Skin> mSkin;
    gfx::SharedPtr<Mesh> mMesh;
    std::list<gfx::SharedPtr<Node>> mChildren = {};

    glm::vec3 mPosition;
    glm::quat mRotation;
    glm::vec3 mScale;

public:
    explicit Node(gfx::SharedPtr<Skin> skin, gfx::SharedPtr<Mesh> mesh, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
        : mSkin(std::move(skin)), mMesh(std::move(mesh)), mPosition(position), mRotation(rotation), mScale(scale) {}

    auto transform() -> glm::mat4 {
        return glm::scale(glm::translate(glm::mat4(1.0f), mPosition) * glm::mat4_cast(mRotation), mScale);
    }

    void setParent(const gfx::SharedPtr<Node>& node) {
        if (mParent == node.get() || node.get() == this) {
            return;
        }
        if (mParent) {
            erase(mParent->mChildren, gfx::RetainPtr(this));
        }
        mParent = node.get();
        if (mParent) {
            mParent->mChildren.emplace_back(gfx::RetainPtr(this));
        }
    }
};
