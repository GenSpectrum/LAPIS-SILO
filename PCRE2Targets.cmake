# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/PCRE2-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${pcre2_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${PCRE2_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET pcre2::pcre2)
    add_library(pcre2::pcre2 INTERFACE IMPORTED)
    message(${PCRE2_MESSAGE_MODE} "Conan: Target declared 'pcre2::pcre2'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/PCRE2-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()