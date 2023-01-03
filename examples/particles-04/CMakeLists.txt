add_executable(particles-04 src/main.cpp src/Renderer.cpp src/Renderer.hpp)
set_target_properties(particles-04 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(particles-04 PRIVATE common gfx)