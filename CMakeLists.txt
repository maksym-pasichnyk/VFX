cmake_minimum_required(VERSION 3.21)
project(gfx)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON)

set(SPIRV_REFLECT_EXAMPLES OFF)
set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)

include(FetchContent)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

set(VMA_VERSION 3.0.1)
FetchContent_Declare(VulkanMemoryAllocator URL "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/v${VMA_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

set(SPDLOG_VERSION 1.11.0)
FetchContent_Declare(spdlog URL "https://github.com/gabime/spdlog/archive/v${SPDLOG_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(spdlog)

set(SPIRV_REFLECT_VERSION 1.3.236.0)
FetchContent_Declare(SPIRV-Reflect URL "https://github.com/KhronosGroup/SPIRV-Reflect/archive/refs/tags/sdk-${SPIRV_REFLECT_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(SPIRV-Reflect)

function(target_compile_shaders TARGET)
    set(SPIRV_BINARY_FILES)

    foreach(SOURCE_FILE ${ARGN})
#        get_filename_component(FILE_NAME ${SOURCE_FILE} NAME)
#        set(OUTPUT_FILE "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")

        set(OUTPUT_FILE "${SOURCE_FILE}.spv")
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND glslc ${SOURCE_FILE} -o ${OUTPUT_FILE}
            DEPENDS ${SOURCE_FILE}
        )
        list(APPEND SPIRV_BINARY_FILES ${OUTPUT_FILE})
    endforeach()

#    set(SPIRV_MODULE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.spv")
#    add_custom_command(
#        OUTPUT ${SPIRV_MODULE}
#        COMMAND spirv-link -o ${SPIRV_MODULE} ${SPIRV_BINARY_FILES}
#        DEPENDS ${SPIRV_BINARY_FILES}
#        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#    )

#    add_custom_target(${TARGET}_shaders DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.spv")
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
    spirv-reflect-static
    VulkanMemoryAllocator
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