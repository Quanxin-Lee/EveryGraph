add_library(everygraph_foundation STATIC
    src/error.cpp
    src/ids.cpp
)
target_include_directories(everygraph_foundation PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_features(everygraph_foundation PUBLIC cxx_std_20)
set_target_properties(everygraph_foundation PROPERTIES CXX_EXTENSIONS OFF)
everygraph_assert_no_qt_linkage(everygraph_foundation)
everygraph_register_code_target(everygraph_foundation)
