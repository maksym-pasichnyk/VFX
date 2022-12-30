set(IMGUI_VERSION 1.89.1)

FetchContent_Declare(imgui
    URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC "${imgui_SOURCE_DIR}/imconfig.h" "${imgui_SOURCE_DIR}/imgui.cpp" "${imgui_SOURCE_DIR}/imgui.h" "${imgui_SOURCE_DIR}/imgui_demo.cpp" "${imgui_SOURCE_DIR}/imgui_draw.cpp" "${imgui_SOURCE_DIR}/imgui_internal.h" "${imgui_SOURCE_DIR}/imgui_tables.cpp" "${imgui_SOURCE_DIR}/imgui_widgets.cpp" "${imgui_SOURCE_DIR}/imstb_rectpack.h" "${imgui_SOURCE_DIR}/imstb_textedit.h" "${imgui_SOURCE_DIR}/imstb_truetype.h")
set_target_properties(imgui PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)

add_library(common STATIC src/GuiRenderer.cpp src/GuiRenderer.hpp src/Assets.hpp src/Delegate.hpp src/Signal.hpp src/simd.hpp src/NotSwiftUI/UIContext.hpp src/NotSwiftUI/Alignment.hpp src/NotSwiftUI/View.hpp)
set_target_properties(common PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_compile_options(common PUBLIC -fenable-matrix -Wno-nullability-completeness)
target_link_libraries(common PUBLIC gfx imgui)
target_include_directories(common PUBLIC src ${imgui_SOURCE_DIR})
