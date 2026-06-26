# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/Arrow-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${arrow_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${Arrow_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET arrow::arrow)
    add_library(arrow::arrow INTERFACE IMPORTED)
    message(${Arrow_MESSAGE_MODE} "Conan: Target declared 'arrow::arrow'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/Arrow-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()