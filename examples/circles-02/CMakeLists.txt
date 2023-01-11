add_executable(circles-02 src/main.cpp)
set_target_properties(circles-02 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(circles-02 PRIVATE common gfx)