########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(utf8proc_FIND_QUIETLY)
    set(utf8proc_MESSAGE_MODE VERBOSE)
else()
    set(utf8proc_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/utf8procTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${utf8proc_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(utf8proc_VERSION_STRING "2.9.0")
set(utf8proc_INCLUDE_DIRS ${utf8proc_INCLUDE_DIRS_RELEASE} )
set(utf8proc_INCLUDE_DIR ${utf8proc_INCLUDE_DIRS_RELEASE} )
set(utf8proc_LIBRARIES ${utf8proc_LIBRARIES_RELEASE} )
set(utf8proc_DEFINITIONS ${utf8proc_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${utf8proc_BUILD_MODULES_PATHS_RELEASE} )
    message(${utf8proc_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


