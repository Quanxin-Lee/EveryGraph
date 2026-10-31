add_executable(everygraph_command_kernel_tests command_kernel_tests.cpp)
target_compile_features(everygraph_command_kernel_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_command_kernel_tests PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_command_kernel_tests PRIVATE everygraph_application)
everygraph_register_code_target(everygraph_command_kernel_tests)

if(BUILD_TESTING)
    add_test(NAME command_kernel_tests COMMAND everygraph_command_kernel_tests)
    set_tests_properties(command_kernel_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        TIMEOUT 60
    )
endif()
