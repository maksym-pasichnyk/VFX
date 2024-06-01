#include "Device.hpp"
#include "Library.hpp"
#include "Function.hpp"

gfx::Library::Library(rc<Device> device, vk::ShaderModuleCreateInfo const& create_info) : device(std::move(device)) {
    this->handle = this->device->handle.createShaderModule(create_info, VK_NULL_HANDLE, this->device->dispatcher);
    spvReflectCreateShaderModule(create_info.codeSize, create_info.pCode, &this->spvReflectShaderModule);
}

gfx::Library::~Library() {
    device->handle.destroyShaderModule(handle, nullptr, device->dispatcher);
    spvReflectDestroyShaderModule(&spvReflectShaderModule);
}

auto gfx::Library::newFunction(this Library& self, std::string name) -> rc<Function> {
    for (auto& sep : std::span(self.spvReflectShaderModule.entry_points, self.spvReflectShaderModule.entry_point_count)) {
        if (name == sep.name) {
            return rc<Function>(new Function(self.shared_from_this(), std::move(name), &sep));
        }
    }
    return {};
}
