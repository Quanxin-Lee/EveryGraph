if(NOT DEFINED EVERYGRAPH_SOURCE_DIR)
    message(FATAL_ERROR "EVERYGRAPH_SOURCE_DIR must be provided")
endif()

set(presentation_directory "${EVERYGRAPH_SOURCE_DIR}/modules/presentation_qt")
set(desktop_directory "${EVERYGRAPH_SOURCE_DIR}/apps/desktop")
foreach(required_directory IN ITEMS "${presentation_directory}" "${desktop_directory}")
    if(NOT IS_DIRECTORY "${required_directory}")
        message(FATAL_ERROR "Expected shell directory is missing: ${required_directory}")
    endif()
endforeach()

set(violations "")

file(GLOB_RECURSE kernel_public_headers
    "${EVERYGRAPH_SOURCE_DIR}/modules/foundation/include/everygraph/*.hpp"
    "${EVERYGRAPH_SOURCE_DIR}/modules/foundation/include/everygraph/*.h"
    "${EVERYGRAPH_SOURCE_DIR}/modules/application/include/everygraph/*.hpp"
    "${EVERYGRAPH_SOURCE_DIR}/modules/application/include/everygraph/*.h"
)
foreach(header IN LISTS kernel_public_headers)
    file(READ "${header}" contents)
    string(REPLACE "\n" ";" lines "${contents}")
    foreach(line IN LISTS lines)
        if(line MATCHES "#[ \t]*include[ \t]*[<\"]Qt")
            list(APPEND violations "${header}: Qt include: ${line}")
        elseif(line MATCHES "(^|[^A-Za-z0-9_])Q[A-Z][A-Za-z0-9_]*")
            list(APPEND violations "${header}: Qt type token: ${line}")
        endif()
    endforeach()
endforeach()

foreach(kernel_cmake IN ITEMS
        "${EVERYGRAPH_SOURCE_DIR}/modules/foundation/CMakeLists.txt"
        "${EVERYGRAPH_SOURCE_DIR}/modules/application/CMakeLists.txt")
    file(READ "${kernel_cmake}" contents)
    if(contents MATCHES "presentation_qt|apps/desktop|Qt[0-9]*::")
        list(APPEND violations
            "${kernel_cmake}: kernel target definition references a UI target or Qt")
    endif()
endforeach()

file(READ "${EVERYGRAPH_SOURCE_DIR}/CMakeLists.txt" root_cmake)
if(NOT root_cmake MATCHES "option\\(EVERYGRAPH_ENABLE_DESKTOP_SHELL[^)]*OFF\\)")
    list(APPEND violations
        "CMakeLists.txt: EVERYGRAPH_ENABLE_DESKTOP_SHELL is not an opt-in OFF option")
endif()

file(READ "${desktop_directory}/CMakeLists.txt" desktop_cmake)
if(NOT desktop_cmake MATCHES "everygraph_assert_no_qt_linkage\\(everygraph_desktop_fakes\\)")
    list(APPEND violations
        "apps/desktop/CMakeLists.txt: everygraph_desktop_fakes has no Qt-linkage assertion")
endif()

# Widgets and the composition root may read snapshots through CommandQuery and
# call bindings, but only presentation/src/command_dispatcher.cpp encloses
# command requests. The fake command handlers are the registered kernel commands
# and are excluded here because they are the mutation implementation, not UI
# code.
set(widget_sources "")
file(GLOB_RECURSE presentation_sources
    "${presentation_directory}/include/*.hpp"
    "${presentation_directory}/src/*.cpp"
)
foreach(source IN LISTS presentation_sources)
    if(NOT source MATCHES "command_dispatcher\\.(hpp|cpp)$")
        list(APPEND widget_sources "${source}")
    endif()
endforeach()

file(GLOB_RECURSE desktop_sources
    "${desktop_directory}/src/*.cpp"
    "${desktop_directory}/src/*.hpp"
)
foreach(source IN LISTS desktop_sources)
    if(NOT source MATCHES "fake_project\\.(hpp|cpp)$")
        list(APPEND widget_sources "${source}")
    endif()
endforeach()

foreach(source IN LISTS widget_sources)
    file(READ "${source}" contents)
    string(REPLACE "\n" ";" lines "${contents}")
    foreach(line IN LISTS lines)
        if(line MATCHES "(gateway|gateway_)\\.(execute|undo|redo)[ \t]*\\(")
            list(APPEND violations "${source}: widget submits a kernel request: ${line}")
        elseif(line MATCHES "\\.(create_object|set_property|erase_object)[ \t]*\\(")
            list(APPEND violations "${source}: widget mutates a transaction: ${line}")
        elseif(line MATCHES "CommandTransaction")
            list(APPEND violations "${source}: widget touches CommandTransaction: ${line}")
        elseif(line MATCHES "CommandGateway")
            list(APPEND violations
                "${source}: widget holds the command gateway instead of CommandQuery: ${line}")
        elseif(line MATCHES "#[ \t]*include[ \t]*[<\"].*command_gateway\\.hpp")
            list(APPEND violations
                "${source}: widget includes the command gateway header: ${line}")
        endif()
    endforeach()
endforeach()

file(GLOB_RECURSE vendor_assets
    "${desktop_directory}/*.png"
    "${desktop_directory}/*.jpg"
    "${desktop_directory}/*.jpeg"
    "${desktop_directory}/*.svg"
    "${desktop_directory}/*.ico"
    "${desktop_directory}/*.icns"
    "${desktop_directory}/*.webp"
    "${desktop_directory}/*.gif"
    "${presentation_directory}/*.png"
    "${presentation_directory}/*.jpg"
    "${presentation_directory}/*.jpeg"
    "${presentation_directory}/*.svg"
    "${presentation_directory}/*.ico"
    "${presentation_directory}/*.icns"
    "${presentation_directory}/*.webp"
    "${presentation_directory}/*.gif"
)
if(vendor_assets)
    list(APPEND violations
        "Vendor or bundled image assets found in the shell: ${vendor_assets}")
endif()

file(GLOB_RECURSE qt_resource_files
    "${desktop_directory}/*.qrc"
    "${presentation_directory}/*.qrc"
)
if(qt_resource_files)
    list(APPEND violations
        "Bundled Qt resource files found in the shell: ${qt_resource_files}")
endif()

foreach(source IN LISTS widget_sources)
    file(READ "${source}" contents)
    if(contents MATCHES "fromTheme[ \t]*\\(")
        list(APPEND violations "${source}: icons must be painted, not taken from a theme")
    endif()
endforeach()

if(violations)
    string(REPLACE ";" "\n" joined_violations "${violations}")
    message(FATAL_ERROR "shell_boundary_check failed:\n${joined_violations}")
endif()

message(STATUS
    "shell_boundary_check: kernel public headers are Qt-free, kernel targets do not link the shell, widgets read through CommandQuery, they never hold the command gateway or submit a request, and the shell bundles no vendor image assets")
