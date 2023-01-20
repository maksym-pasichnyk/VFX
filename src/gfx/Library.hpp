#pragma once

#include "Device.hpp"

#include <spirv_reflect.h>

namespace gfx {
    struct Device;
    struct Function;

    struct LibraryShared {
        Device device;
        vk::ShaderModule raw;
        SpvReflectShaderModule spvReflectShaderModule;

        explicit LibraryShared(Device device);
        ~LibraryShared();
    };

    struct Library final {
        std::shared_ptr<LibraryShared> shared;

        explicit Library();
        explicit Library(std::shared_ptr<LibraryShared> shared);

        auto newFunction(std::string name) -> Function;
    };
}