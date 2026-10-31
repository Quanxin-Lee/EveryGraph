add_executable(everygraph_toolchain_probe toolchain_probe.cpp)
target_compile_features(everygraph_toolchain_probe PRIVATE cxx_std_20)
set_target_properties(everygraph_toolchain_probe PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(everygraph_toolchain_probe PRIVATE Threads::Threads)
everygraph_register_code_target(everygraph_toolchain_probe)

if(BUILD_TESTING)
    add_test(NAME toolchain_probe COMMAND everygraph_toolchain_probe)
    set_tests_properties(toolchain_probe PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        TIMEOUT 30
    )

    add_test(NAME catalog_validation
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/validate_catalog_children.py"
    )
    set_tests_properties(catalog_validation PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        TIMEOUT 60
    )

    # The discovery start directory is relative, so unittest resolves "tools"
    # against the working directory; pinning the source root is what makes the
    # registration independent of the directory CTest would otherwise default to.
    add_test(NAME catalog_validator_tests
        COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tools -p test_catalog_children.py
    )
    set_tests_properties(catalog_validator_tests PROPERTIES
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        TIMEOUT 60
    )
endif()

if(EVERYGRAPH_ENABLE_QT_PROBE)
    find_package(Qt6 6.12.0 EXACT REQUIRED COMPONENTS Core Gui Widgets)
    qt_standard_project_setup()

    qt_add_executable(everygraph_qt_probe MACOSX_BUNDLE qt_probe/main.cpp)
    target_compile_features(everygraph_qt_probe PRIVATE cxx_std_20)
    set_target_properties(everygraph_qt_probe PROPERTIES CXX_EXTENSIONS OFF)
    target_link_libraries(everygraph_qt_probe PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
    everygraph_register_code_target(everygraph_qt_probe)

    if(BUILD_TESTING)
        add_test(NAME qt_probe_smoke COMMAND everygraph_qt_probe --smoke-test)
        if(VCPKG_INSTALLED_DIR AND APPLE)
            if(CMAKE_BUILD_TYPE MATCHES "Debug")
                set(everygraph_qt_plugin_dir
                    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
            else()
                set(everygraph_qt_plugin_dir
                    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
            endif()
            set_tests_properties(qt_probe_smoke PROPERTIES
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${everygraph_qt_plugin_dir}"
                TIMEOUT 30
            )
        else()
            set_tests_properties(qt_probe_smoke PROPERTIES
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
                TIMEOUT 30
            )
        endif()
    endif()
endif()
