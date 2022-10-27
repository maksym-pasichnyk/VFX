#include "GameObject.hpp"
#include "DrawList.hpp"

GameObject::GameObject(const Arc<vfx::Device>& device) {
    mesh = Arc<Mesh>::alloc();

    DrawList drawList{};
    drawList.drawBox(glm::vec3(-1), glm::vec3(1), 0xFF0000FF);
    drawList.drawBox(glm::vec3(-10, -2, -10), glm::vec3(10, -2, 10), 0xFF0000FF);

    auto indices = std::span(drawList.indices);

    mesh->indexCount = indices.size();
    mesh->indexBuffer = device->makeBuffer(
        vk::BufferUsageFlagBits::eIndexBuffer,
        indices.size_bytes(),
        indices.data(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    );

    auto vertices = std::span(drawList.vertices);
    mesh->vertexCount = indices.size();
    mesh->vertexBuffer = device->makeBuffer(
        vk::BufferUsageFlagBits::eVertexBuffer,
        vertices.size_bytes(),
        vertices.data(),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    );
}

void GameObject::draw(vfx::CommandBuffer* cmd) {
    cmd->bindIndexBuffer(mesh->indexBuffer, 0, vk::IndexType::eUint32);
    cmd->bindVertexBuffer(0, mesh->vertexBuffer, vk::DeviceSize{0});
    cmd->drawIndexed(mesh->indexCount, 1, 0, 0, 0);
}

auto GameObject::getTransformMatrix() const -> glm::mat4x4 {
    glm::mat4x4 orientation = glm::yawPitchRoll(
        glm::radians(rotation.y),
        glm::radians(rotation.x),
        glm::radians(rotation.z)
    );
    return glm::inverse(glm::translate(glm::mat4(1.0f), position) * orientation);
}
