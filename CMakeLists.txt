cmake_minimum_required(VERSION 3.21)
project(VFX)

set(CMAKE_CXX_STANDARD 20)
set(GLFW_LIBRARY_TYPE "STATIC")

find_package(Vulkan REQUIRED)

add_subdirectory(deps/glm)
add_subdirectory(deps/glfw)
add_subdirectory(deps/entt)
add_subdirectory(deps/spdlog)
add_subdirectory(deps/range-v3)
add_subdirectory(deps/optional)
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

add_library(imgui STATIC
    deps/imgui/imgui.h
    deps/imgui/imgui.cpp
    deps/imgui/imconfig.h
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_internal.h
    deps/imgui/imgui_widgets.cpp
    deps/imgui/misc/cpp/imgui_stdlib.h
    deps/imgui/misc/cpp/imgui_stdlib.cpp
)
set_target_properties(imgui PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(imgui PUBLIC deps/imgui)
target_compile_definitions(imgui PUBLIC -DIMGUI_DEFINE_MATH_OPERATORS)

add_library(VFX STATIC
    src/types.hpp
    src/context.hpp
    src/texture.hpp
    src/buffer.hpp
    src/display.hpp
        src/pass.hpp
    src/material.hpp
    src/assets.hpp
    src/vulkan.cpp
        src/delegate.hpp src/signal.hpp)
target_include_directories(VFX PUBLIC src)
target_link_libraries(VFX PUBLIC
    glm
    glfw
    EnTT
    imgui
    spdlog
    range-v3
    tl::optional
    Vulkan::Vulkan
    VulkanMemoryAllocator
)
target_compile_definitions(VFX PUBLIC
    -D_USE_MATH_DEFINES
    -DGLM_FORCE_XYZW_ONLY
    -DGLM_FORCE_DEPTH_ZERO_TO_ONE
    -DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    -DVULKAN_HPP_NO_UNION_CONSTRUCTORS
    -DVK_NO_PROTOTYPES
    -DVK_USE_PLATFORM_MACOS_MVK
    -DGLFW_INCLUDE_NONE
    -DGLFW_INCLUDE_VULKAN
)

add_executable(VFX-example
    examples/main.cpp
    examples/widgets.hpp
    examples/pipeline.hpp
)
target_link_libraries(VFX-example PRIVATE VFX)
target_compile_shaders(VFX-example
    assets/shaders/default.frag
    assets/shaders/default.vert
    assets/shaders/imgui.frag
    assets/shaders/imgui.vert
)