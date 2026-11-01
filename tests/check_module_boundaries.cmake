if(NOT DEFINED EVERYGRAPH_SOURCE_DIR)
    message(FATAL_ERROR "EVERYGRAPH_SOURCE_DIR must be provided")
endif()

set(module_directories
    "${EVERYGRAPH_SOURCE_DIR}/modules/foundation"
    "${EVERYGRAPH_SOURCE_DIR}/modules/application"
    "${EVERYGRAPH_SOURCE_DIR}/modules/data"
)

set(violations "")
foreach(module_directory IN LISTS module_directories)
    if(NOT IS_DIRECTORY "${module_directory}")
        message(FATAL_ERROR "Expected module directory is missing: ${module_directory}")
    endif()
    file(GLOB_RECURSE module_files
        "${module_directory}/*.hpp"
        "${module_directory}/*.h"
        "${module_directory}/*.cpp"
        "${module_directory}/*.cc"
    )
    foreach(module_file IN LISTS module_files)
        file(READ "${module_file}" contents)
        string(REPLACE "\n" ";" lines "${contents}")
        foreach(line IN LISTS lines)
            if(line MATCHES "#[ \t]*include[ \t]*[<\"]Qt")
                list(APPEND violations "${module_file}: ${line}")
            elseif(line MATCHES "(^|[^A-Za-z0-9_])Q[A-Z][A-Za-z0-9_]*")
                list(APPEND violations "${module_file}: ${line}")
            endif()
        endforeach()
    endforeach()
endforeach()

if(violations)
    string(REPLACE ";" "\n" joined_violations "${violations}")
    message(FATAL_ERROR "GUI toolkit tokens found in GUI-independent modules:\n${joined_violations}")
endif()

message(STATUS "module_boundary_check: no GUI toolkit includes or type tokens in foundation, application or data")
