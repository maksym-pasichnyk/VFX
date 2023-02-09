#pragma once

#include "Object.hpp"

#include <set>
#include <list>
#include <string>
#include <vector>

struct NativeImage;
struct TexturePacker {
private:
    struct Holder {
        std::string name;
        int32_t width;
        int32_t height;
        sp<NativeImage> image;

        Holder(std::string name, int32_t width, int32_t height, sp<NativeImage> image);
    };
public:
    struct Slot {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
        std::list<Slot> children = {};
        std::optional<Holder> stored = {};

        Slot(int32_t x, int32_t y, int32_t width, int32_t height);

        auto add(const Holder& holder) -> bool;
    };

private:
    int32_t storageX = 0;
    int32_t storageY = 0;

    std::list<Slot> slots = {};
    std::vector<Holder> holders = {};

public:
    void emplace(const std::string& name, const sp<NativeImage>& image);

    void pack(int32_t& out_width, int32_t& out_height);

    void gather(auto&& fn) {
        recurse(slots, fn);
    }

private:
    auto expand(const Holder& holder) -> bool;
    auto addToStorage(const Holder& holder) -> bool;

    static void recurse(const std::list<Slot>& items, auto&& fn) {
        for (auto& item : items) {
            if (item.stored.has_value()) {
                fn(item.stored->name, item.stored->image, item.x, item.y, item.width, item.height);
            }
            recurse(item.children, std::forward<decltype(fn)>(fn));
        }
    }
};
