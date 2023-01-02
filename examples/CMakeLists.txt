add_subdirectory(common)
add_subdirectory(triangle-01)
add_subdirectory(circles-02)
add_subdirectory(geometry-03)

target_compile_shaders(common
    "${CMAKE_SOURCE_DIR}/assets/shaders/gui.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/gui.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/default.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/default.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/shader.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/shader.frag"
)