########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Poco_FIND_QUIETLY)
    set(Poco_MESSAGE_MODE VERBOSE)
else()
    set(Poco_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/PocoTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${poco_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Poco_VERSION_STRING "1.15.2")
set(Poco_INCLUDE_DIRS ${poco_INCLUDE_DIRS_RELEASE} )
set(Poco_INCLUDE_DIR ${poco_INCLUDE_DIRS_RELEASE} )
set(Poco_LIBRARIES ${poco_LIBRARIES_RELEASE} )
set(Poco_DEFINITIONS ${poco_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${poco_BUILD_MODULES_PATHS_RELEASE} )
    message(${Poco_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


