# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/module-simdutf-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${simdutf_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${simdutf_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET simdutf::simdutf)
    add_library(simdutf::simdutf INTERFACE IMPORTED)
    message(${simdutf_MESSAGE_MODE} "Conan: Target declared 'simdutf::simdutf'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/module-simdutf-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()