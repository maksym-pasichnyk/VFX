#pragma once

#include "Library.hpp"
#include "spirv_reflect.h"

#include <string>

namespace gfx {
    struct Function final : public ManagedObject {
        rc<Library>  library;
        std::string             name;
        SpvReflectEntryPoint*   entry_point;

        explicit Function(rc<Library> library, std::string name, SpvReflectEntryPoint* entry_point);
    };
}