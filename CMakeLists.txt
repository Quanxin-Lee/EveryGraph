cmake_minimum_required(VERSION 3.28)

project(EveryGraphPhase0 LANGUAGES CXX)

option(EVERYGRAPH_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(EVERYGRAPH_ENABLE_TSAN "Enable ThreadSanitizer separately from ASan and UBSan" OFF)
option(EVERYGRAPH_ENABLE_QT_PROBE "Build the Qt Widgets qualification probe and offscreen smoke test" OFF)

if(EVERYGRAPH_ENABLE_SANITIZERS AND EVERYGRAPH_ENABLE_TSAN)
    message(FATAL_ERROR "ASan/UBSan and TSan must use separate build directories and cannot be combined.")
endif()

find_package(Threads REQUIRED)
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)

add_executable(everygraph_toolchain_probe tools/toolchain_probe.cpp)
target_compile_features(everygraph_toolchain_probe PRIVATE cxx_std_20)
set_target_properties(everygraph_toolchain_probe PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_toolchain_probe PRIVATE Threads::Threads)

set(everygraph_probe_targets everygraph_toolchain_probe)

if(EVERYGRAPH_ENABLE_QT_PROBE)
    find_package(Qt6 6.12.0 EXACT REQUIRED COMPONENTS Core Gui Widgets)
    qt_standard_project_setup()

    qt_add_executable(everygraph_qt_probe MACOSX_BUNDLE tools/qt_probe/main.cpp)
    target_compile_features(everygraph_qt_probe PRIVATE cxx_std_20)
    set_target_properties(everygraph_qt_probe PROPERTIES CXX_EXTENSIONS OFF)
    target_link_libraries(everygraph_qt_probe PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

    list(APPEND everygraph_probe_targets everygraph_qt_probe)
endif()

foreach(everygraph_probe_target IN LISTS everygraph_probe_targets)
    if(MSVC)
        target_compile_options(${everygraph_probe_target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
        target_compile_options(${everygraph_probe_target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow
        )
    endif()
endforeach()

if(EVERYGRAPH_ENABLE_SANITIZERS OR EVERYGRAPH_ENABLE_TSAN)
    if(MSVC OR NOT CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
        message(FATAL_ERROR "The configured sanitizers require a supported Clang or GCC toolchain.")
    endif()

    if(EVERYGRAPH_ENABLE_SANITIZERS)
        set(everygraph_sanitizer_flag -fsanitize=address,undefined)
    else()
        set(everygraph_sanitizer_flag -fsanitize=thread)
    endif()

    foreach(everygraph_probe_target IN LISTS everygraph_probe_targets)
        target_compile_options(${everygraph_probe_target} PRIVATE
            ${everygraph_sanitizer_flag} -fno-omit-frame-pointer
        )
        target_link_options(${everygraph_probe_target} PRIVATE ${everygraph_sanitizer_flag})
    endforeach()
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME toolchain_probe COMMAND everygraph_toolchain_probe)
    set_tests_properties(toolchain_probe PROPERTIES TIMEOUT 30)

    add_test(NAME planning_validation
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_planning.py"
    )
    set_tests_properties(planning_validation PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        TIMEOUT 60
    )

    add_test(NAME planning_validator_tests
        COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tools -p test_validate_planning.py
    )
    set_tests_properties(planning_validator_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        TIMEOUT 60
    )

    if(EVERYGRAPH_ENABLE_QT_PROBE)
        add_test(NAME qt_probe_smoke COMMAND everygraph_qt_probe --smoke-test)
        set_tests_properties(qt_probe_smoke PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
            TIMEOUT 30
        )
    endif()
endif()
