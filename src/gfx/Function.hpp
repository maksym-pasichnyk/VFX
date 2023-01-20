#pragma once

#include "Library.hpp"
#include "spirv_reflect.h"

#include <string>

namespace gfx {
    struct Function final {
        Library library;
        std::string name;
        SpvReflectEntryPoint* entry_point;

        explicit Function() = default;
        explicit Function(const Library& library, std::string name, SpvReflectEntryPoint* entry_point);
    };
}