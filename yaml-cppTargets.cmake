# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/yaml-cpp-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${yaml-cpp_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${yaml-cpp_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET yaml-cpp::yaml-cpp)
    add_library(yaml-cpp::yaml-cpp INTERFACE IMPORTED)
    message(${yaml-cpp_MESSAGE_MODE} "Conan: Target declared 'yaml-cpp::yaml-cpp'")
endif()
if(NOT TARGET yaml-cpp)
    add_library(yaml-cpp INTERFACE IMPORTED)
    set_property(TARGET yaml-cpp PROPERTY INTERFACE_LINK_LIBRARIES yaml-cpp::yaml-cpp)
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/yaml-cpp-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()