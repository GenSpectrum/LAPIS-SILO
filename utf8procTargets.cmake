# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/utf8proc-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${utf8proc_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${utf8proc_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET utf8proc::utf8proc)
    add_library(utf8proc::utf8proc INTERFACE IMPORTED)
    message(${utf8proc_MESSAGE_MODE} "Conan: Target declared 'utf8proc::utf8proc'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/utf8proc-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()