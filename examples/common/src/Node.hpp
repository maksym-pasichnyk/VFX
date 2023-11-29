#pragma once

#include "Object.hpp"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <list>
#include <vector>

struct Node : public ManagedObject {
private:
    Node* mParent = {};

    rc<Skin> mSkin;
    rc<Mesh> mMesh;
    std::list<rc<Node>> mChildren = {};

    glm::vec3 mPosition;
    glm::quat mRotation;
    glm::vec3 mScale;

public:
    explicit Node(rc<Skin> skin, rc<Mesh> mesh, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
        : mSkin(std::move(skin)), mMesh(std::move(mesh)), mPosition(position), mRotation(rotation), mScale(scale) {}

    auto transform() -> glm::mat4 {
        return glm::scale(glm::translate(glm::mat4(1.0f), mPosition) * glm::mat4_cast(mRotation), mScale);
    }

    void setParent(const rc<Node>& node) {
        if (mParent == node.get() || node.get() == this) {
            return;
        }
        if (mParent) {
            erase(mParent->mChildren, shared_from_this());
        }
        mParent = node.get();
        if (mParent) {
            mParent->mChildren.emplace_back(shared_from_this());
        }
    }
};
