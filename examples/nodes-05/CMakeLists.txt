add_executable(nodes-05 ../common/src/UIRenderer.cpp ../common/src/UIRenderer.hpp src/main.cpp src/Renderer.cpp src/Renderer.hpp)
set_target_properties(nodes-05 PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_link_libraries(nodes-05 PRIVATE common gfx)