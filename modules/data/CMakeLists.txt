add_library(everygraph_data STATIC
    src/cell.cpp
    src/chunk.cpp
    src/commands.cpp
    src/commands/worksheet_edit.cpp
    src/dataset.cpp
    src/datetime.cpp
    src/diagnostics.cpp
    src/document.cpp
    src/logical_type.cpp
    src/provenance.cpp
    src/row_domain.cpp
    src/units.cpp
)
target_include_directories(everygraph_data
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
# The kernel is consumed through its public interfaces only, which is what lets a create or
# append publish a revision without a second mutation path. The data core itself stays free of
# any GUI toolkit dependency and of any storage or Arrow object.
target_link_libraries(everygraph_data PUBLIC everygraph_foundation everygraph_application)
target_compile_features(everygraph_data PUBLIC cxx_std_20)
set_target_properties(everygraph_data PROPERTIES CXX_EXTENSIONS OFF)
everygraph_assert_no_qt_linkage(everygraph_data)
everygraph_register_code_target(everygraph_data)
