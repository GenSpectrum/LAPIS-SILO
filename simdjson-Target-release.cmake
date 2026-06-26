# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(simdjson_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(simdjson_FRAMEWORKS_FOUND_RELEASE "${simdjson_FRAMEWORKS_RELEASE}" "${simdjson_FRAMEWORK_DIRS_RELEASE}")

set(simdjson_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET simdjson_DEPS_TARGET)
    add_library(simdjson_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET simdjson_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${simdjson_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${simdjson_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### simdjson_DEPS_TARGET to all of them
conan_package_library_targets("${simdjson_LIBS_RELEASE}"    # libraries
                              "${simdjson_LIB_DIRS_RELEASE}" # package_libdir
                              "${simdjson_BIN_DIRS_RELEASE}" # package_bindir
                              "${simdjson_LIBRARY_TYPE_RELEASE}"
                              "${simdjson_IS_HOST_WINDOWS_RELEASE}"
                              simdjson_DEPS_TARGET
                              simdjson_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "simdjson"    # package_name
                              "${simdjson_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${simdjson_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET simdjson::simdjson
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${simdjson_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${simdjson_LIBRARIES_TARGETS}>
                 )

    if("${simdjson_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET simdjson::simdjson
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     simdjson_DEPS_TARGET)
    endif()

    set_property(TARGET simdjson::simdjson
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${simdjson_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET simdjson::simdjson
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${simdjson_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET simdjson::simdjson
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${simdjson_LIB_DIRS_RELEASE}>)
    set_property(TARGET simdjson::simdjson
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${simdjson_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET simdjson::simdjson
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${simdjson_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(simdjson_LIBRARIES_RELEASE simdjson::simdjson)
