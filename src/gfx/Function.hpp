#pragma once

#include "Library.hpp"
#include "spirv_reflect.h"

#include <string>

namespace gfx {
    struct Function final : ManagedObject<Function> {
        ManagedShared<Library>  library     = {};
        std::string             name        = {};
        SpvReflectEntryPoint*   entry_point = {};

        explicit Function() = default;
        explicit Function(ManagedShared<Library> library, std::string name, SpvReflectEntryPoint* entry_point);
    };
}