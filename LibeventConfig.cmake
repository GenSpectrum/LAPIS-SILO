########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Libevent_FIND_QUIETLY)
    set(Libevent_MESSAGE_MODE VERBOSE)
else()
    set(Libevent_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/LibeventTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libevent_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Libevent_VERSION_STRING "2.1.12")
set(Libevent_INCLUDE_DIRS ${libevent_INCLUDE_DIRS_RELEASE} )
set(Libevent_INCLUDE_DIR ${libevent_INCLUDE_DIRS_RELEASE} )
set(Libevent_LIBRARIES ${libevent_LIBRARIES_RELEASE} )
set(Libevent_DEFINITIONS ${libevent_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${libevent_BUILD_MODULES_PATHS_RELEASE} )
    message(${Libevent_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


