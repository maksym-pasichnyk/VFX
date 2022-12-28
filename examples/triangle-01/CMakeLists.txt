add_executable(triangle-01 src/main.cpp src/Renderer.cpp src/Renderer.hpp)
set_target_properties(triangle-01 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(triangle-01 PRIVATE common gfx)

target_compile_shaders(triangle-01
    "${CMAKE_SOURCE_DIR}/assets/shaders/default.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/default.vert"
)