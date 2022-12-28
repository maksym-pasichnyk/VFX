cmake_minimum_required(VERSION 3.21)
project(gfx)

set(SPIRV_REFLECT_EXAMPLES OFF)
set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

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

add_library(gfx STATIC src/gfx/Application.hpp src/gfx/Window.hpp src/gfx/Window.cpp src/gfx/Texture.hpp src/gfx/Buffer.hpp src/gfx/ComputePipelineState.hpp src/gfx/CommandQueue.hpp src/gfx/CommandQueue.cpp src/gfx/Texture.cpp src/gfx/Buffer.cpp src/gfx/Application.cpp src/gfx/ComputePipelineState.cpp src/gfx/DescriptorSet.cpp src/gfx/DescriptorSet.hpp src/gfx/Swapchain.cpp src/gfx/Swapchain.hpp src/gfx/Device.cpp src/gfx/Device.hpp src/gfx/Object.hpp src/gfx/Drawable.cpp src/gfx/Drawable.hpp src/gfx/Sampler.cpp src/gfx/Sampler.hpp src/gfx/CommandBuffer.cpp src/gfx/CommandBuffer.hpp src/gfx/Library.cpp src/gfx/Library.hpp src/gfx/Function.hpp src/gfx/Function.cpp src/gfx/RenderPipelineState.cpp src/gfx/RenderPipelineState.hpp src/gfx/GFX.hpp)
set_target_properties(gfx PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(gfx
PUBLIC
    spdlog
    Vulkan::Vulkan
    ${SDL2_LIBRARIES}
    VulkanMemoryAllocator
PRIVATE
    spirv-reflect-static
)
target_include_directories(gfx PUBLIC "${SDL2_INCLUDE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(gfx PUBLIC
    -D_USE_MATH_DEFINES
    -DVK_NO_PROTOTYPES
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_MACOS_MVK
    -DVK_USE_PLATFORM_METAL_EXT
    -DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS
    -DVULKAN_HPP_NO_UNION_CONSTRUCTORS
    -DVULKAN_HPP_NO_DEFAULT_DISPATCHER
)

add_subdirectory(examples)