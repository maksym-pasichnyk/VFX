cmake_minimum_required(VERSION 3.21)
project(VFX)

set(GLFW_LIBRARY_TYPE "STATIC")

set(SPIRV_REFLECT_EXAMPLES OFF)
set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(deps/glfw)
add_subdirectory(deps/spdlog)
add_subdirectory(deps/SPIRV-Reflect)
add_subdirectory(deps/VulkanMemoryAllocator)

function(target_compile_shaders TARGET)
    set(SPIRV_BINARY_FILES)

    foreach(SOURCE_FILE ${ARGN})
#        get_filename_component(FILE_NAME ${SOURCE_FILE} NAME)

        set(SPIRV "${SOURCE_FILE}.spv")
        add_custom_command(
            OUTPUT ${SPIRV}
#            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders"
            COMMAND glslc ${SOURCE_FILE} -o ${SPIRV}
            DEPENDS ${SOURCE_FILE}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach()

    add_custom_target(${TARGET}_shaders DEPENDS ${SPIRV_BINARY_FILES})
    add_dependencies(${TARGET} ${TARGET}_shaders)
endfunction()

add_library(VFX STATIC
    src/vfx/types.hpp
    src/vfx/context.hpp
    src/vfx/texture.hpp
    src/vfx/buffer.hpp
    src/vfx/pass.hpp
    src/vfx/material.hpp
    src/vfx/assets.hpp
    src/vfx/delegate.hpp
    src/vfx/signal.hpp
    src/vfx/queue.hpp
    src/vfx/queue.cpp
    src/vfx/texture.cpp
    src/vfx/buffer.cpp
    src/vfx/context.cpp
    src/vfx/material.cpp
    src/vfx/pass.cpp
    src/vfx/group.cpp
    src/vfx/group.hpp
    src/vfx/layer.cpp
    src/vfx/layer.hpp
    src/vfx/device.cpp
    src/vfx/device.hpp
        src/vfx/vfx.hpp)
set_target_properties(VFX PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(VFX PUBLIC
    glfw
    spdlog
    Vulkan::Vulkan
    spirv-reflect-static
    VulkanMemoryAllocator
)
target_include_directories(VFX PUBLIC src)
target_compile_definitions(VFX PUBLIC
    -D_USE_MATH_DEFINES
    -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    -DVULKAN_HPP_NO_UNION_CONSTRUCTORS
    -DVULKAN_HPP_NO_DEFAULT_DISPATCHER
    -DVK_NO_PROTOTYPES
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_MACOS_MVK
    -DGLFW_INCLUDE_NONE
    -DGLFW_INCLUDE_VULKAN
)