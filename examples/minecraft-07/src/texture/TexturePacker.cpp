#include "TexturePacker.hpp"
#include "NativeImage.hpp"
#include "range/v3/algorithm/sort.hpp"
#include "spdlog/spdlog.h"

static constexpr auto smallestEncompassingPowerOfTwo(int32_t value) -> int32_t {
    int32_t i = value - 1;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    return i + 1;
}

TexturePacker::Holder::Holder(std::string name, int32_t width, int32_t height, sp<NativeImage> image)
    : name(std::move(name)), width(width), height(height), image(std::move(image)) {}

void TexturePacker::emplace(const std::string& name, const sp<NativeImage>& image) {
    holders.emplace_back(name, image->getSizeX(), image->getSizeY(), image);
}

void TexturePacker::pack(int32_t& out_width, int32_t& out_height) {
    auto sorted = holders;
    ranges::sort(sorted, [](auto& a, auto& b) -> bool {
        return std::tie(b.width, b.height) < std::tie(a.width, a.height);
    });

    for (auto& holder : sorted) {
        if (!addToStorage(holder)) {
            throw std::runtime_error("Failed to pack texture");
        }
    }

    out_width = storageX;
    out_height = storageY;
}

auto TexturePacker::expand(const Holder& holder) -> bool {
    int32_t current_width = smallestEncompassingPowerOfTwo(int32_t(storageX));
    int32_t current_height = smallestEncompassingPowerOfTwo(int32_t(storageY));
    int32_t require_width = smallestEncompassingPowerOfTwo(int32_t(storageX + holder.width));
    int32_t require_height = smallestEncompassingPowerOfTwo(int32_t(storageY + holder.height));

    bool grow_width = current_width != require_width;
    bool grow_height = current_height != require_height;

    if ((grow_width ^ grow_height) ? grow_width : current_width <= current_height) {
        if (storageY == 0) {
            storageY = holder.height;
        }
        slots.emplace_back(storageX, 0, holder.width, storageY).add(holder);
        storageX += holder.width;
    } else {
        slots.emplace_back(0, storageY, storageX, holder.height).add(holder);
        storageY += holder.height;
    }
    return true;
}

auto TexturePacker::addToStorage(const Holder& holder) -> bool {
    for (auto& slot : slots) {
        if (slot.add(holder)) {
            return true;
        }
    }
    return expand(holder);
}

TexturePacker::Slot::Slot(int32_t x, int32_t y, int32_t width, int32_t height) : x(x), y(y), width(width), height(height) {}

std::set<TexturePacker::Slot*> counter = {};

auto TexturePacker::Slot::add(const Holder& holder) -> bool {
    if (stored.has_value() || width < holder.width || height < holder.height) {
        return false;
    }

    if (width == holder.width && height == holder.height) {
        stored = holder;
        return true;
    }

    if (children.empty()) {
        children.emplace_back(x, y, holder.width, holder.height);

        if (width > holder.width) {
            children.emplace_back(x + holder.width, y, width - holder.width, height);
        }
        if (height > holder.height) {
            children.emplace_back(x, y + holder.height, holder.width, height - holder.height);
        }
    }


    for (auto& slot : children) {
        if (slot.add(holder)) {
            return true;
        }
    }

    return false;
}
