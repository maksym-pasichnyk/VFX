#include "Device.hpp"
#include "Library.hpp"
#include "Function.hpp"

gfx::Library::Library(ManagedShared<Device> device) : device(std::move(device)) {}

gfx::Library::~Library() {
    device->raii.raw.destroyShaderModule(raw, nullptr, device->raii.dispatcher);
    spvReflectDestroyShaderModule(&spvReflectShaderModule);
}

auto gfx::Library::newFunction(std::string name) -> ManagedShared<Function> {
    for (auto& sep : std::span(spvReflectShaderModule.entry_points, spvReflectShaderModule.entry_point_count)) {
        if (name == sep.name) {
            return MakeShared(new Function(shared_from_this(), std::move(name), &sep));
        }
    }
    return {};
}
