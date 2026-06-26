########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(FastFloat_FIND_QUIETLY)
    set(FastFloat_MESSAGE_MODE VERBOSE)
else()
    set(FastFloat_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FastFloatTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${fast_float_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(FastFloat_VERSION_STRING "8.1.0")
set(FastFloat_INCLUDE_DIRS ${fast_float_INCLUDE_DIRS_RELEASE} )
set(FastFloat_INCLUDE_DIR ${fast_float_INCLUDE_DIRS_RELEASE} )
set(FastFloat_LIBRARIES ${fast_float_LIBRARIES_RELEASE} )
set(FastFloat_DEFINITIONS ${fast_float_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${fast_float_BUILD_MODULES_PATHS_RELEASE} )
    message(${FastFloat_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


