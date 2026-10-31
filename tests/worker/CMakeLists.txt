add_executable(everygraph_test_worker test_worker_main.cpp)
target_compile_features(everygraph_test_worker PRIVATE cxx_std_20)
set_target_properties(everygraph_test_worker PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_test_worker PRIVATE everygraph_application)
everygraph_register_code_target(everygraph_test_worker)
everygraph_assert_no_qt_linkage(everygraph_test_worker)

add_executable(everygraph_worker_protocol_tests worker_protocol_tests.cpp)
target_compile_features(everygraph_worker_protocol_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_worker_protocol_tests PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_worker_protocol_tests PRIVATE everygraph_application)
target_compile_definitions(everygraph_worker_protocol_tests PRIVATE
    "EVERYGRAPH_TEST_WORKER_PROGRAM=\"$<TARGET_FILE:everygraph_test_worker>\""
)
everygraph_register_code_target(everygraph_worker_protocol_tests)
everygraph_assert_no_qt_linkage(everygraph_worker_protocol_tests)

if(BUILD_TESTING)
    add_test(NAME worker_protocol_tests COMMAND everygraph_worker_protocol_tests)
    set_tests_properties(worker_protocol_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        TIMEOUT 180
    )
endif()
