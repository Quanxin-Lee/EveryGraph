cmake_minimum_required(VERSION 3.28)

project(EveryGraphPhase0 LANGUAGES CXX)

option(EVERYGRAPH_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(EVERYGRAPH_ENABLE_TSAN "Enable ThreadSanitizer separately from ASan and UBSan" OFF)

if(EVERYGRAPH_ENABLE_SANITIZERS AND EVERYGRAPH_ENABLE_TSAN)
    message(FATAL_ERROR "ASan/UBSan and TSan must use separate build directories and cannot be combined.")
endif()

find_package(Threads REQUIRED)
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)

add_library(everygraph_foundation STATIC
    modules/foundation/src/error.cpp
    modules/foundation/src/ids.cpp
)
target_include_directories(everygraph_foundation PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/foundation/include"
)
target_compile_features(everygraph_foundation PUBLIC cxx_std_20)
set_target_properties(everygraph_foundation PROPERTIES CXX_EXTENSIONS OFF)

add_library(everygraph_application STATIC
    modules/application/src/command_gateway.cpp
    modules/application/src/command_registry.cpp
    modules/application/src/document.cpp
    modules/application/src/value.cpp
)
target_include_directories(everygraph_application
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules/application/include"
)
target_link_libraries(everygraph_application PUBLIC everygraph_foundation)
target_compile_features(everygraph_application PUBLIC cxx_std_20)
set_target_properties(everygraph_application PROPERTIES CXX_EXTENSIONS OFF)

function(everygraph_assert_no_qt_linkage target)
    get_target_property(target_libraries ${target} LINK_LIBRARIES)
    if(target_libraries)
        foreach(library IN LISTS target_libraries)
            if(library MATCHES "^Qt[0-9]*::")
                message(FATAL_ERROR "${target} must not link a GUI toolkit dependency (${library}).")
            endif()
        endforeach()
    endif()
    get_target_property(interface_libraries ${target} INTERFACE_LINK_LIBRARIES)
    if(interface_libraries)
        foreach(library IN LISTS interface_libraries)
            if(library MATCHES "^Qt[0-9]*::")
                message(FATAL_ERROR "${target} must not export a GUI toolkit dependency (${library}).")
            endif()
        endforeach()
    endif()
endfunction()

everygraph_assert_no_qt_linkage(everygraph_foundation)
everygraph_assert_no_qt_linkage(everygraph_application)

add_executable(everygraph_toolchain_probe tools/toolchain_probe.cpp)
target_compile_features(everygraph_toolchain_probe PRIVATE cxx_std_20)
set_target_properties(everygraph_toolchain_probe PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_toolchain_probe PRIVATE Threads::Threads)

add_executable(everygraph_command_kernel_tests tests/application/command_kernel_tests.cpp)
target_compile_features(everygraph_command_kernel_tests PRIVATE cxx_std_20)
set_target_properties(everygraph_command_kernel_tests PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_command_kernel_tests PRIVATE everygraph_application)

set(everygraph_code_targets
    everygraph_foundation
    everygraph_application
    everygraph_toolchain_probe
    everygraph_command_kernel_tests
)

if(EVERYGRAPH_ENABLE_QT_PROBE)
    find_package(Qt6 6.12.0 EXACT REQUIRED COMPONENTS Core Gui Widgets)
    qt_standard_project_setup()

    qt_add_executable(everygraph_qt_probe MACOSX_BUNDLE tools/qt_probe/main.cpp)
    target_compile_features(everygraph_qt_probe PRIVATE cxx_std_20)
    set_target_properties(everygraph_qt_probe PROPERTIES CXX_EXTENSIONS OFF)
    target_link_libraries(everygraph_qt_probe PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

    list(APPEND everygraph_code_targets everygraph_qt_probe)
endif()

foreach(everygraph_code_target IN LISTS everygraph_code_targets)
    if(MSVC)
        target_compile_options(${everygraph_code_target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
        target_compile_options(${everygraph_code_target} PRIVATE
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

    foreach(everygraph_code_target IN LISTS everygraph_code_targets)
        target_compile_options(${everygraph_code_target} PRIVATE
            ${everygraph_sanitizer_flag} -fno-omit-frame-pointer
        )
        get_target_property(everygraph_target_type ${everygraph_code_target} TYPE)
        if(NOT everygraph_target_type STREQUAL "STATIC_LIBRARY")
            target_link_options(${everygraph_code_target} PRIVATE ${everygraph_sanitizer_flag})
        endif()
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

    add_test(NAME command_kernel_tests COMMAND everygraph_command_kernel_tests)
    set_tests_properties(command_kernel_tests PROPERTIES TIMEOUT 60)

    add_test(NAME module_boundary_check
        COMMAND "${CMAKE_COMMAND}"
            "-DEVERYGRAPH_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_module_boundaries.cmake"
    )
    set_tests_properties(module_boundary_check PROPERTIES TIMEOUT 30)

    if(EVERYGRAPH_ENABLE_QT_PROBE)
        add_test(NAME qt_probe_smoke COMMAND everygraph_qt_probe --smoke-test)
        set_tests_properties(qt_probe_smoke PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
            TIMEOUT 30
        )
    endif()
endif()
