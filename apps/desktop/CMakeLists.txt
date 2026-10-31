add_library(everygraph_desktop_fakes STATIC
    src/fake_project.cpp
    src/fake_project.hpp
)
target_include_directories(everygraph_desktop_fakes
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_link_libraries(everygraph_desktop_fakes PUBLIC everygraph_application)
target_compile_features(everygraph_desktop_fakes PUBLIC cxx_std_20)
set_target_properties(everygraph_desktop_fakes PROPERTIES CXX_EXTENSIONS OFF)
everygraph_assert_no_qt_linkage(everygraph_desktop_fakes)
everygraph_register_code_target(everygraph_desktop_fakes)

qt_add_executable(everygraph_desktop MACOSX_BUNDLE src/main.cpp)
target_link_libraries(everygraph_desktop PRIVATE
    everygraph_presentation_qt
    everygraph_desktop_fakes
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
target_compile_features(everygraph_desktop PRIVATE cxx_std_20)
set_target_properties(everygraph_desktop PROPERTIES CXX_EXTENSIONS OFF)
everygraph_register_code_target(everygraph_desktop)

if(NOT BUILD_TESTING)
    return()
endif()

qt_add_executable(everygraph_workbench_tests tests/workbench_tests.cpp)
target_link_libraries(everygraph_workbench_tests PRIVATE
    everygraph_presentation_qt
    everygraph_desktop_fakes
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)
target_compile_features(everygraph_workbench_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_workbench_tests PROPERTIES CXX_EXTENSIONS OFF)
everygraph_register_code_target(everygraph_workbench_tests)

if(VCPKG_INSTALLED_DIR AND APPLE)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(everygraph_qt_plugin_dir
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
    else()
        set(everygraph_qt_plugin_dir
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
    endif()
    set(everygraph_workbench_test_environment
        "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${everygraph_qt_plugin_dir}")
else()
    set(everygraph_workbench_test_environment "QT_QPA_PLATFORM=offscreen")
endif()

add_test(NAME workbench_tests COMMAND everygraph_workbench_tests)
set_tests_properties(workbench_tests PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    ENVIRONMENT "${everygraph_workbench_test_environment}"
    TIMEOUT 120
)

add_test(NAME workbench_smoke COMMAND everygraph_desktop --smoke-test)
set_tests_properties(workbench_smoke PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    ENVIRONMENT "${everygraph_workbench_test_environment}"
    TIMEOUT 60
)

add_test(NAME shell_boundary_check
    COMMAND "${CMAKE_COMMAND}"
        "-DEVERYGRAPH_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_shell_boundaries.cmake"
)
set_tests_properties(shell_boundary_check PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    TIMEOUT 30
)
