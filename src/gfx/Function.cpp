#include "Function.hpp"

gfx::Function::Function(const Library& library, std::string name, SpvReflectEntryPoint* entry_point) : library(library), name(std::move(name)), entry_point(entry_point) {}