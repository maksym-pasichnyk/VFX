add_library(common INTERFACE)
#src/Assets.hpp src/Delegate.hpp src/Signal.hpp src/simd.hpp)
#set_target_properties(common PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_include_directories(common INTERFACE src)