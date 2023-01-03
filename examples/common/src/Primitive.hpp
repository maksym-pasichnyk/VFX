#pragma once

#include <cstdint>

struct Primitive {
    uint32_t baseIndex;
    uint32_t baseVertex;
    uint32_t numIndices;
    uint32_t numVertices;

    explicit Primitive(uint32_t baseIndex, uint32_t baseVertex, uint32_t numIndices, uint32_t numVertices)
        : baseIndex(baseIndex), baseVertex(baseVertex), numIndices(numIndices), numVertices(numVertices) {}
};