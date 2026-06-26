# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/simdjson-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${simdjson_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${simdjson_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET simdjson::simdjson)
    add_library(simdjson::simdjson INTERFACE IMPORTED)
    message(${simdjson_MESSAGE_MODE} "Conan: Target declared 'simdjson::simdjson'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/simdjson-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()