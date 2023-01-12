add_executable(menu-06 src/main.cpp)
set_target_properties(menu-06 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(menu-06 PRIVATE common gfx)