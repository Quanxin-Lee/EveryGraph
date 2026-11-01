# Each test group owns its executable and its own add_test calls in its file.
# The groups are always added: the single-file topology defined every test
# executable regardless of BUILD_TESTING, and only its add_test calls were
# conditional, so that behavior is preserved here.
add_subdirectory(application)
add_subdirectory(data)
add_subdirectory(worker)

if(NOT BUILD_TESTING)
    return()
endif()

add_test(NAME planning_validation
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/validate_planning.py"
)
set_tests_properties(planning_validation PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    TIMEOUT 60
)

add_test(NAME planning_validator_tests
    COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tools -p test_validate_planning.py
)
set_tests_properties(planning_validator_tests PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    TIMEOUT 60
)

add_test(NAME module_boundary_check
    COMMAND "${CMAKE_COMMAND}"
        "-DEVERYGRAPH_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
        -P "${CMAKE_SOURCE_DIR}/tests/check_module_boundaries.cmake"
)
set_tests_properties(module_boundary_check PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    TIMEOUT 30
)
