add_executable(circles-02 src/main.cpp src/Renderer.cpp src/Renderer.hpp)
set_target_properties(circles-02 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(circles-02 PRIVATE common gfx)

target_compile_shaders(circles-02
    "${CMAKE_SOURCE_DIR}/assets/shaders/gui.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/gui.vert"
)