########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(simdutf_FIND_QUIETLY)
    set(simdutf_MESSAGE_MODE VERBOSE)
else()
    set(simdutf_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module-simdutfTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${simdutf_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(simdutf_VERSION_STRING "8.0.0")
set(simdutf_INCLUDE_DIRS ${simdutf_INCLUDE_DIRS_RELEASE} )
set(simdutf_INCLUDE_DIR ${simdutf_INCLUDE_DIRS_RELEASE} )
set(simdutf_LIBRARIES ${simdutf_LIBRARIES_RELEASE} )
set(simdutf_DEFINITIONS ${simdutf_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${simdutf_BUILD_MODULES_PATHS_RELEASE} )
    message(${simdutf_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


include(FindPackageHandleStandardArgs)
set(simdutf_FOUND 1)
set(simdutf_VERSION "8.0.0")

find_package_handle_standard_args(simdutf
                                  REQUIRED_VARS simdutf_VERSION
                                  VERSION_VAR simdutf_VERSION)
mark_as_advanced(simdutf_FOUND simdutf_VERSION)

set(simdutf_FOUND 1)
set(simdutf_VERSION "8.0.0")
mark_as_advanced(simdutf_FOUND simdutf_VERSION)

