# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/FastFloat-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${fast_float_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${FastFloat_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET FastFloat::fast_float)
    add_library(FastFloat::fast_float INTERFACE IMPORTED)
    message(${FastFloat_MESSAGE_MODE} "Conan: Target declared 'FastFloat::fast_float'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/FastFloat-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()