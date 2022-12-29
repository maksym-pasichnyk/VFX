set(IMGUI_VERSION 1.89.1)

FetchContent_Declare(imgui
    URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(imgui)

add_library(common INTERFACE)
#src/Assets.hpp src/Delegate.hpp src/Signal.hpp src/simd.hpp)
#set_target_properties(common PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_include_directories(common INTERFACE src "${imgui_SOURCE_DIR}")