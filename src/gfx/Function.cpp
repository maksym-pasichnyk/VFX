#include "Function.hpp"

gfx::Function::Function(rc<Library> library, std::string name, SpvReflectEntryPoint* entry_point)
    : library(std::move(library))
    , name(std::move(name))
    , entry_point(entry_point) {}