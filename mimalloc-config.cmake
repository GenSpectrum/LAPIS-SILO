########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(mimalloc_FIND_QUIETLY)
    set(mimalloc_MESSAGE_MODE VERBOSE)
else()
    set(mimalloc_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/mimallocTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${mimalloc_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(mimalloc_VERSION_STRING "3.3.2")
set(mimalloc_INCLUDE_DIRS ${mimalloc_INCLUDE_DIRS_RELEASE} )
set(mimalloc_INCLUDE_DIR ${mimalloc_INCLUDE_DIRS_RELEASE} )
set(mimalloc_LIBRARIES ${mimalloc_LIBRARIES_RELEASE} )
set(mimalloc_DEFINITIONS ${mimalloc_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${mimalloc_BUILD_MODULES_PATHS_RELEASE} )
    message(${mimalloc_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


