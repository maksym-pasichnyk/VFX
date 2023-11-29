#pragma once

#include "Device.hpp"

#include <spirv_reflect.h>

namespace gfx {
    struct Device;
    struct Function;

    struct Library : public ManagedObject {
        rc<Device> device;
        vk::ShaderModule handle;
        SpvReflectShaderModule spvReflectShaderModule;

        explicit Library(rc<Device> device, vk::ShaderModuleCreateInfo const& create_info);
        ~Library() override;

        auto newFunction(this Library& self, std::string name) -> rc<Function>;
    };
}