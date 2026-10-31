cmake_minimum_required(VERSION 3.28)

project(EveryGraphPhase0 LANGUAGES CXX)

option(EVERYGRAPH_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(EVERYGRAPH_ENABLE_TSAN "Enable ThreadSanitizer separately from ASan and UBSan" OFF)
option(EVERYGRAPH_ENABLE_QT_PROBE "Build the opt-in Qt 6.12 Widgets qualification probe" OFF)
option(EVERYGRAPH_ENABLE_DESKTOP_SHELL "Build the Qt workbench shell prototype from the pinned desktop-shell graph" OFF)

if(EVERYGRAPH_ENABLE_SANITIZERS AND EVERYGRAPH_ENABLE_TSAN)
    message(FATAL_ERROR "ASan/UBSan and TSan must use separate build directories and cannot be combined.")
endif()

find_package(Threads REQUIRED)
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
include(CTest)

# Every target is defined in the directory that owns its sources, so adding a
# module never edits this file. Outputs stay in the top-level binary directory,
# where the single-file topology placed them and where the documented tooling
# commands and the SBOM generator look for them. Qt's qt_standard_project_setup
# also sets CMAKE_RUNTIME_OUTPUT_DIRECTORY on Windows only when it is unset, so
# this must be assigned before any subdirectory is added.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

define_property(GLOBAL PROPERTY EVERYGRAPH_CODE_TARGETS
    BRIEF_DOCS "EveryGraph-owned build targets"
    FULL_DOCS "Targets that receive the shared warning and sanitizer policy. Each directory registers its own targets, because a subdirectory cannot add to a parent-scope variable."
)

function(everygraph_register_code_target target)
    set_property(GLOBAL APPEND PROPERTY EVERYGRAPH_CODE_TARGETS "${target}")
endfunction()

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

add_subdirectory(modules/foundation)
add_subdirectory(modules/application)
add_subdirectory(tools)
add_subdirectory(tests)

if(EVERYGRAPH_ENABLE_DESKTOP_SHELL)
    find_package(Qt6 6.12.0 EXACT REQUIRED COMPONENTS Core Gui Widgets Test)
    qt_standard_project_setup()
    add_subdirectory(modules/presentation_qt)
    add_subdirectory(apps/desktop)
endif()

get_property(everygraph_code_targets GLOBAL PROPERTY EVERYGRAPH_CODE_TARGETS)

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
