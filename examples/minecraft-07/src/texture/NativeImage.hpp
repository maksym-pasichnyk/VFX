#pragma once

#include "Object.hpp"

#include <vector>

struct NativeImage : Object {
private:
    int32_t width;
    int32_t height;
    int32_t channels;
    std::vector<uint8_t> pixels = {};

public:
    NativeImage(int32_t width, int32_t height, int32_t channels);
    NativeImage(int32_t width, int32_t height, int32_t channels, std::vector<uint8_t> pixels);

    auto getSizeX() const -> int32_t;
    auto getSizeY() const -> int32_t;
    auto getPixelRGBA(int32_t x, int32_t y) const -> uint32_t;
    void setPixelRGBA(int32_t x, int32_t y, uint32_t color);

public:
    static auto read(const std::string& path) -> sp<NativeImage>;
};
