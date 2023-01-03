add_subdirectory(common)
add_subdirectory(triangle-01)
add_subdirectory(circles-02)
add_subdirectory(geometry-03)
add_subdirectory(particles-04)

target_compile_shaders(common
    "${CMAKE_SOURCE_DIR}/assets/shaders/gui.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/gui.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/default.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/default.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/geometry.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/geometry.frag"
    "${CMAKE_SOURCE_DIR}/assets/shaders/particles.vert"
    "${CMAKE_SOURCE_DIR}/assets/shaders/particles.frag"
)