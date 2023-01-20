#include "Device.hpp"
#include "Library.hpp"
#include "Function.hpp"

gfx::LibraryShared::LibraryShared(gfx::Device device) : device(std::move(device)) {}

gfx::LibraryShared::~LibraryShared() {
    device.handle().destroyShaderModule(raw, nullptr, device.dispatcher());
    spvReflectDestroyShaderModule(&spvReflectShaderModule);
}

gfx::Library::Library() : shared(nullptr) {}
gfx::Library::Library(std::shared_ptr<LibraryShared> shared) : shared(std::move(shared)) {}

auto gfx::Library::newFunction(std::string name) -> Function {
    for (auto& sep : std::span(shared->spvReflectShaderModule.entry_points, shared->spvReflectShaderModule.entry_point_count)) {
        if (name == sep.name) {
            return Function(*this, std::move(name), &sep);
        }
    }
    throw std::runtime_error("Entry point not found");
}
