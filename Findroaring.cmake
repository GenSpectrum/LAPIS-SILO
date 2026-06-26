########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(roaring_FIND_QUIETLY)
    set(roaring_MESSAGE_MODE VERBOSE)
else()
    set(roaring_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module-roaringTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${roaring_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(roaring_VERSION_STRING "4.5.0")
set(roaring_INCLUDE_DIRS ${roaring_INCLUDE_DIRS_RELEASE} )
set(roaring_INCLUDE_DIR ${roaring_INCLUDE_DIRS_RELEASE} )
set(roaring_LIBRARIES ${roaring_LIBRARIES_RELEASE} )
set(roaring_DEFINITIONS ${roaring_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${roaring_BUILD_MODULES_PATHS_RELEASE} )
    message(${roaring_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


include(FindPackageHandleStandardArgs)
set(roaring_FOUND 1)
set(roaring_VERSION "4.5.0")

find_package_handle_standard_args(roaring
                                  REQUIRED_VARS roaring_VERSION
                                  VERSION_VAR roaring_VERSION)
mark_as_advanced(roaring_FOUND roaring_VERSION)

set(roaring_FOUND 1)
set(roaring_VERSION "4.5.0")
mark_as_advanced(roaring_FOUND roaring_VERSION)

