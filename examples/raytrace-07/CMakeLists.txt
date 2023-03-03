add_executable(raytrace-07 src/main.cpp)
set_target_properties(raytrace-07 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(raytrace-07 PRIVATE common gfx)