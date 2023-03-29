#pragma once

#include "Device.hpp"

#include <spirv_reflect.h>

namespace gfx {
    struct Device;
    struct Function;

    struct Library : ManagedObject<Library> {
        ManagedShared<Device> device;
        vk::ShaderModule raw;
        SpvReflectShaderModule spvReflectShaderModule;

        explicit Library(ManagedShared<Device> device);
        ~Library() override;

        auto newFunction(std::string name) -> ManagedShared<Function>;
    };
}