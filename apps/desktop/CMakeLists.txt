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

qt_add_executable(everygraph_preferences_tests tests/preferences_tests.cpp)
target_link_libraries(everygraph_preferences_tests PRIVATE
    everygraph_presentation_qt
    everygraph_desktop_fakes
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)
target_compile_features(everygraph_preferences_tests PRIVATE cxx_std_20)
target_compile_definitions(everygraph_preferences_tests PRIVATE
    EVERYGRAPH_DESKTOP_BINARY="$<TARGET_FILE:everygraph_desktop>")
set_target_properties(everygraph_preferences_tests PROPERTIES CXX_EXTENSIONS OFF)
everygraph_register_code_target(everygraph_preferences_tests)
# The restart test launches the desktop binary, so building the test alone
# must build that binary too.
add_dependencies(everygraph_preferences_tests everygraph_desktop)

if(VCPKG_INSTALLED_DIR AND APPLE)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(everygraph_qt_plugin_dir
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
    else()
        set(everygraph_qt_plugin_dir
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
    endif()
    set(everygraph_workbench_test_environment
        "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${everygraph_qt_plugin_dir};EVERYGRAPH_PREFERENCES_PATH=${CMAKE_BINARY_DIR}/workbench-smoke-preferences.json")
else()
    set(everygraph_workbench_test_environment
        "QT_QPA_PLATFORM=offscreen;EVERYGRAPH_PREFERENCES_PATH=${CMAKE_BINARY_DIR}/workbench-smoke-preferences.json")
endif()

set(everygraph_accessibility_test_sources tests/accessibility_tests.cpp)
if(APPLE)
    enable_language(OBJCXX)
    list(APPEND everygraph_accessibility_test_sources
        tests/native_pinyin_ime.hpp
        tests/native_pinyin_ime.mm
    )
endif()
qt_add_executable(everygraph_accessibility_tests ${everygraph_accessibility_test_sources})
target_link_libraries(everygraph_accessibility_tests PRIVATE
    everygraph_presentation_qt
    everygraph_desktop_fakes
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)
if(APPLE)
    target_link_libraries(everygraph_accessibility_tests PRIVATE
        "-framework Cocoa"
        "-framework Carbon"
    )
endif()
target_compile_features(everygraph_accessibility_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_accessibility_tests PROPERTIES
    CXX_EXTENSIONS OFF
    OBJCXX_STANDARD 20
    OBJCXX_EXTENSIONS OFF
)
everygraph_register_code_target(everygraph_accessibility_tests)

add_test(NAME workbench_tests COMMAND everygraph_workbench_tests)
set_tests_properties(workbench_tests PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    ENVIRONMENT "${everygraph_workbench_test_environment}"
    TIMEOUT 120
)

add_test(NAME preferences_tests COMMAND everygraph_preferences_tests)
set_tests_properties(preferences_tests PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    ENVIRONMENT "${everygraph_workbench_test_environment}"
    TIMEOUT 120
)

set(everygraph_accessibility_1x_environment
    "${everygraph_workbench_test_environment};QT_SCALE_FACTOR=1;EVERYGRAPH_EXPECTED_SCALE=1;EVERYGRAPH_SCREENSHOT_DIR=${CMAKE_BINARY_DIR}/accessibility-evidence/1x"
)
set(everygraph_accessibility_2x_environment
    "${everygraph_workbench_test_environment};QT_SCALE_FACTOR=2;EVERYGRAPH_EXPECTED_SCALE=2;EVERYGRAPH_SCREENSHOT_DIR=${CMAKE_BINARY_DIR}/accessibility-evidence/2x"
)
add_test(NAME accessibility_1x COMMAND everygraph_accessibility_tests)
set_tests_properties(accessibility_1x PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    ENVIRONMENT "${everygraph_accessibility_1x_environment}"
    TIMEOUT 180
)
add_test(NAME accessibility_2x COMMAND everygraph_accessibility_tests)
set_tests_properties(accessibility_2x PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    ENVIRONMENT "${everygraph_accessibility_2x_environment}"
    TIMEOUT 180
)

if(APPLE)
    set(everygraph_accessibility_native_environment
        "QT_QPA_PLATFORM=cocoa;EVERYGRAPH_SCREENSHOT_DIR=${CMAKE_BINARY_DIR}/accessibility-evidence/native;EVERYGRAPH_PREFERENCES_PATH=${CMAKE_BINARY_DIR}/accessibility-native-preferences.json"
    )
    if(VCPKG_INSTALLED_DIR)
        list(APPEND everygraph_accessibility_native_environment
            "QT_PLUGIN_PATH=${everygraph_qt_plugin_dir}")
    endif()
    add_test(NAME accessibility_focus_native
        COMMAND everygraph_accessibility_tests
            focus_order_cycles_without_trap
            preferences_focus_order_is_explicit
    )
    set_tests_properties(accessibility_focus_native PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        ENVIRONMENT "${everygraph_accessibility_native_environment}"
        TIMEOUT 120
    )
    add_test(NAME accessibility_ime_native
        COMMAND everygraph_accessibility_tests native_pinyin_ime_composes_text)
    set_tests_properties(accessibility_ime_native PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        ENVIRONMENT "${everygraph_accessibility_native_environment}"
        TIMEOUT 120
        # Qt Test exits 0 when it skips, so without this the entry would report a
        # pass while the real composition check never ran.
        SKIP_REGULAR_EXPRESSION "the native composition check was skipped"
    )
endif()

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
