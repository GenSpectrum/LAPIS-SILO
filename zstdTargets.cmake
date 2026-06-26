# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/zstd-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${zstd_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${zstd_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET zstd::libzstd_static)
    add_library(zstd::libzstd_static INTERFACE IMPORTED)
    message(${zstd_MESSAGE_MODE} "Conan: Target declared 'zstd::libzstd_static'")
endif()
if(NOT TARGET zstd::libzstd)
    add_library(zstd::libzstd INTERFACE IMPORTED)
    set_property(TARGET zstd::libzstd PROPERTY INTERFACE_LINK_LIBRARIES zstd::libzstd_static)
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/zstd-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()