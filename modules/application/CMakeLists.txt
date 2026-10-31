add_library(everygraph_application STATIC
    src/command_gateway.cpp
    src/command_registry.cpp
    src/document.cpp
    src/value.cpp
)
target_include_directories(everygraph_application
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(everygraph_application PUBLIC everygraph_foundation)
target_compile_features(everygraph_application PUBLIC cxx_std_20)
set_target_properties(everygraph_application PROPERTIES CXX_EXTENSIONS OFF)
everygraph_assert_no_qt_linkage(everygraph_application)
everygraph_register_code_target(everygraph_application)
