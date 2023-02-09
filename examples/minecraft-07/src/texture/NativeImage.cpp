#include "NativeImage.hpp"
#include "spdlog/spdlog.h"
#include "stb_image.h"

#include "Assets.hpp"

NativeImage::NativeImage(int32_t width, int32_t height, int32_t channels) : width(width), height(height), channels(channels) {
    pixels.resize(width * height * channels);
}

NativeImage::NativeImage(int32_t width, int32_t height, int32_t channels, std::vector<uint8_t> pixels) : width(width), height(height), channels(channels), pixels(std::move(pixels)) {}

auto NativeImage::getSizeX() const -> int32_t {
    return width;
}

auto NativeImage::getSizeY() const -> int32_t {
    return height;
}

auto NativeImage::getPixelRGBA(int32_t x, int32_t y) const -> uint32_t {
    uint32_t color;
    std::memcpy(&color, &pixels[(y * width + x) * 4], sizeof(uint32_t));
    return color;
}

void NativeImage::setPixelRGBA(int32_t x, int32_t y, uint32_t color) {
    std::memcpy(&pixels[(y * width + x) * 4], &color, sizeof(uint32_t));
}

auto NativeImage::read(const std::string& path) -> sp<NativeImage> {
//    stbi_set_flip_vertically_on_load(true);

    auto bytes = Assets::readFile(path);

    int32_t width = 0;
    int32_t height = 0;
    auto raw = stbi_load_from_memory((stbi_uc*) bytes.data(), int32_t(bytes.size()), &width, &height, nullptr, 4);
    if (raw == nullptr) {
        spdlog::error("Failed to load image '{}'", path);
        return {};
    }
    std::vector<uint8_t> pixels{};
    pixels.assign(raw, raw + width * height * 4);
    stbi_image_free(raw);

    return sp<NativeImage>::of(int32_t(width), int32_t(height), 4, std::move(pixels));
}
