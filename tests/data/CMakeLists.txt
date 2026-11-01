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
