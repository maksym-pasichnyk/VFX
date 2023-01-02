add_executable(geometry-03 src/main.cpp src/Renderer.cpp src/Renderer.hpp)
set_target_properties(geometry-03 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(geometry-03 PRIVATE common gfx)