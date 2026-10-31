add_library(everygraph_presentation_qt STATIC
    include/everygraph/presentation/command_dispatcher.hpp
    include/everygraph/presentation/document_host.hpp
    include/everygraph/presentation/project_tree_model.hpp
    include/everygraph/presentation/properties_panel.hpp
    include/everygraph/presentation/window_placement.hpp
    include/everygraph/presentation/workbench_icons.hpp
    include/everygraph/presentation/workbench_window.hpp
    src/command_dispatcher.cpp
    src/document_host.cpp
    src/project_tree_model.cpp
    src/properties_panel.cpp
    src/window_placement.cpp
    src/workbench_icons.cpp
    src/workbench_window.cpp
)
target_include_directories(everygraph_presentation_qt
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(everygraph_presentation_qt
    PUBLIC everygraph_application Qt6::Core Qt6::Gui Qt6::Widgets
)
target_compile_features(everygraph_presentation_qt PUBLIC cxx_std_20)
set_target_properties(everygraph_presentation_qt PROPERTIES CXX_EXTENSIONS OFF AUTOMOC ON)
everygraph_register_code_target(everygraph_presentation_qt)

# Dependency direction is adapters -> application -> foundation. The kernel is
# already asserted Qt-free by its own CMake files; this guard keeps the reverse
# edge from ever appearing.
foreach(everygraph_kernel_target IN ITEMS everygraph_foundation everygraph_application)
    get_target_property(everygraph_kernel_libraries ${everygraph_kernel_target} LINK_LIBRARIES)
    get_target_property(everygraph_kernel_interface_libraries ${everygraph_kernel_target} INTERFACE_LINK_LIBRARIES)
    foreach(everygraph_kernel_library IN LISTS everygraph_kernel_libraries everygraph_kernel_interface_libraries)
        if(everygraph_kernel_library MATCHES "everygraph_presentation_qt|everygraph_desktop|^Qt[0-9]*::")
            message(FATAL_ERROR
                "${everygraph_kernel_target} must not depend on the presentation layer (${everygraph_kernel_library})."
            )
        endif()
    endforeach()
endforeach()
