########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(simdjson_FIND_QUIETLY)
    set(simdjson_MESSAGE_MODE VERBOSE)
else()
    set(simdjson_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module-simdjsonTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${simdjson_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(simdjson_VERSION_STRING "4.2.4")
set(simdjson_INCLUDE_DIRS ${simdjson_INCLUDE_DIRS_RELEASE} )
set(simdjson_INCLUDE_DIR ${simdjson_INCLUDE_DIRS_RELEASE} )
set(simdjson_LIBRARIES ${simdjson_LIBRARIES_RELEASE} )
set(simdjson_DEFINITIONS ${simdjson_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${simdjson_BUILD_MODULES_PATHS_RELEASE} )
    message(${simdjson_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


include(FindPackageHandleStandardArgs)
set(simdjson_FOUND 1)
set(simdjson_VERSION "4.2.4")

find_package_handle_standard_args(simdjson
                                  REQUIRED_VARS simdjson_VERSION
                                  VERSION_VAR simdjson_VERSION)
mark_as_advanced(simdjson_FOUND simdjson_VERSION)

set(simdjson_FOUND 1)
set(simdjson_VERSION "4.2.4")
mark_as_advanced(simdjson_FOUND simdjson_VERSION)

