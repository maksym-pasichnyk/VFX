#include "GameObject.hpp"
#include "DrawList.hpp"

GameObject::GameObject(const Arc<vfx::Context>& context) {
    mesh = Arc<Mesh>::alloc();

    DrawList drawList{};
    drawList.drawBox(glm::vec3(-1), glm::vec3(1), 0xFF0000FF);
    drawList.drawBox(glm::vec3(-10, -2, -10), glm::vec3(10, -2, 10), 0xFF0000FF);

    auto indices = std::span(drawList.indices);

    mesh->indexCount = indices.size();
    mesh->indexBuffer = context->makeBuffer(
        vfx::BufferUsage::Index,
        indices.size_bytes(),
        indices.data()
    );

    auto vertices = std::span(drawList.vertices);
    mesh->vertexCount = indices.size();
    mesh->vertexBuffer = context->makeBuffer(
        vfx::BufferUsage::Vertex,
        vertices.size_bytes(),
        vertices.data()
    );
}

void GameObject::draw(vfx::CommandBuffer* cmd) {
    cmd->bindIndexBuffer(mesh->indexBuffer, 0, vk::IndexType::eUint32);
    cmd->bindVertexBuffer(0, mesh->vertexBuffer, vk::DeviceSize{0});
    cmd->drawIndexed(mesh->indexCount, 1, 0, 0, 0);
}
