#include "Function.hpp"
#include "Library.hpp"

gfx::Function::Function(SharedPtr<Library> library, std::string name) : mLibrary(std::move(library)), mFunctionName(std::move(name)) {}