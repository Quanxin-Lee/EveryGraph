# The P2-002 data-model group: one Qt-free executable over the typed values, identity and
# immutable-version, unit and row-alignment families of the conformance plan. Each source
# file registers its cases by fixture ID, so the case list is the fixture evidence.
add_executable(everygraph_data_tests
    data_tests_main.cpp
    typed_value_tests.cpp
    identity_version_tests.cpp
    unit_tests.cpp
    alignment_tests.cpp
)
target_compile_features(everygraph_data_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_data_tests PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_data_tests PRIVATE everygraph_data)
# The harness cross-checks its executed case names against this task's fixture fragment, so the
# path is compiled in rather than resolved from the working directory CTest runs the tool in.
target_compile_definitions(everygraph_data_tests PRIVATE
    EG_FIXTURE_MANIFEST="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/manifest.json"
)
everygraph_assert_no_qt_linkage(everygraph_data_tests)
everygraph_register_code_target(everygraph_data_tests)

if(BUILD_TESTING)
    add_test(NAME data_tests COMMAND everygraph_data_tests)
    set_tests_properties(data_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        TIMEOUT 120
    )
endif()

# The P2-004 worksheet-edit group: one Qt-free executable over the row, cell, column and
# undo/repeat command families. It links the same data module and runs the same harness contract,
# so a fixture id declared by two groups is a build-and-test failure rather than a duplicate name.
add_executable(everygraph_data_command_tests
    commands/data_command_tests_main.cpp
    commands/row_edit_tests.cpp
    commands/cell_edit_tests.cpp
    commands/column_edit_tests.cpp
    commands/undo_repeat_tests.cpp
)
target_compile_features(everygraph_data_command_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_data_command_tests PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_data_command_tests PRIVATE everygraph_data)
target_compile_definitions(everygraph_data_command_tests PRIVATE
    EG_FIXTURE_MANIFEST="${CMAKE_CURRENT_SOURCE_DIR}/commands/manifest-p2-004.json"
    EG_FIXTURE_OTHER_MANIFESTS="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/manifest.json"
)
everygraph_assert_no_qt_linkage(everygraph_data_command_tests)
everygraph_register_code_target(everygraph_data_command_tests)

if(BUILD_TESTING)
    add_test(NAME data_command_tests COMMAND everygraph_data_command_tests)
    set_tests_properties(data_command_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        TIMEOUT 120
    )
endif()
