#set(IMGUI_VERSION 1.89.1)
#FetchContent_Declare(imgui URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
#FetchContent_MakeAvailable(imgui)

set(RANGES_VERSION 0.12.0)
FetchContent_Declare(ranges URL "https://github.com/ericniebler/range-v3/archive/${RANGES_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(ranges)

set(FMT_VERSION 9.1.0)
FetchContent_Declare(fmt URL "https://github.com/fmtlib/fmt/archive/${FMT_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(fmt)

set(TINYGLTF_VERSION 2.7.0)
FetchContent_Declare(tinygltf URL "https://github.com/syoyo/tinygltf/archive/refs/tags/v${TINYGLTF_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(tinygltf)

set(GLM_VERSION 0.9.9.8)
FetchContent_Declare(glm URL "https://github.com/g-truc/glm/archive/refs/tags/${GLM_VERSION}.zip" DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(physfs
    GIT_REPOSITORY https://github.com/icculus/physfs.git
    GIT_TAG main
)
FetchContent_MakeAvailable(physfs)

set(imgui_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

add_library(imgui STATIC "${imgui_SOURCE_DIR}/imconfig.h" "${imgui_SOURCE_DIR}/imgui.cpp" "${imgui_SOURCE_DIR}/imgui.h" "${imgui_SOURCE_DIR}/imgui_draw.cpp" "${imgui_SOURCE_DIR}/imgui_internal.h" "${imgui_SOURCE_DIR}/imgui_tables.cpp" "${imgui_SOURCE_DIR}/imgui_widgets.cpp" "${imgui_SOURCE_DIR}/imstb_rectpack.h" "${imgui_SOURCE_DIR}/imstb_textedit.h" "${imgui_SOURCE_DIR}/imstb_truetype.h")
set_target_properties(imgui PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_include_directories(imgui PUBLIC  ${imgui_SOURCE_DIR})

add_library(common STATIC src/Assets.hpp src/Delegate.hpp src/Signal.hpp src/simd.hpp src/UIContext.hpp src/NotSwiftUI/Core/Alignment.hpp src/NotSwiftUI/Views/View.hpp src/NotSwiftUI/Core/Size.hpp src/NotSwiftUI/Core/Point.hpp src/NotSwiftUI/Core/Color.hpp src/Iter.hpp src/Primitive.hpp src/Skin.hpp src/Node.hpp src/Mesh.hpp src/Scene.hpp src/Animation.hpp src/UIRenderer.hpp src/UIRenderer.cpp src/GraphView.hpp src/Application.hpp src/Object.hpp src/NotSwiftUI/Core/Rect.hpp src/NotSwiftUI/Core/ProposedSize.hpp src/NotSwiftUI/Views/Button.hpp src/NotSwiftUI/Views/Text.hpp src/NotSwiftUI/Views/Slider.hpp src/NotSwiftUI/Views/HStack.hpp src/NotSwiftUI/Views/VStack.hpp src/NotSwiftUI/NotSwiftUI.hpp src/NotSwiftUI/Shapes/Shape.hpp src/NotSwiftUI/Views/ShapeView.hpp src/NotSwiftUI/Shapes/Circle.hpp src/NotSwiftUI/Shapes/Border.hpp src/NotSwiftUI/Shapes/Rectangle.hpp src/NotSwiftUI/Modifiers/ForegroundColor.hpp src/NotSwiftUI/Modifiers/FlexibleFrame.hpp src/NotSwiftUI/Modifiers/FixedFrame.hpp src/NotSwiftUI/Modifiers/FixedSize.hpp src/NotSwiftUI/Views/Overlay.hpp src/Graphics.hpp src/Enum.hpp src/JsonElement.hpp src/JsonParser.cpp src/JsonParser.hpp src/filesystem.hpp src/filesystem.cpp src/fstream.hpp src/fstream.cpp)
set_target_properties(common PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_compile_options(common PUBLIC -fenable-matrix -Wno-nullability-completeness)
target_link_libraries(common PUBLIC gfx fmt glm imgui range-v3 tinygltf physfs-static)
target_include_directories(common PUBLIC src)