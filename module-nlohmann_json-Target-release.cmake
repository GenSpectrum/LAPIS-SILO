# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(nlohmann_json_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(nlohmann_json_FRAMEWORKS_FOUND_RELEASE "${nlohmann_json_FRAMEWORKS_RELEASE}" "${nlohmann_json_FRAMEWORK_DIRS_RELEASE}")

set(nlohmann_json_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET nlohmann_json_DEPS_TARGET)
    add_library(nlohmann_json_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET nlohmann_json_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${nlohmann_json_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${nlohmann_json_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### nlohmann_json_DEPS_TARGET to all of them
conan_package_library_targets("${nlohmann_json_LIBS_RELEASE}"    # libraries
                              "${nlohmann_json_LIB_DIRS_RELEASE}" # package_libdir
                              "${nlohmann_json_BIN_DIRS_RELEASE}" # package_bindir
                              "${nlohmann_json_LIBRARY_TYPE_RELEASE}"
                              "${nlohmann_json_IS_HOST_WINDOWS_RELEASE}"
                              nlohmann_json_DEPS_TARGET
                              nlohmann_json_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "nlohmann_json"    # package_name
                              "${nlohmann_json_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${nlohmann_json_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET nlohmann_json::nlohmann_json
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${nlohmann_json_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${nlohmann_json_LIBRARIES_TARGETS}>
                 )

    if("${nlohmann_json_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET nlohmann_json::nlohmann_json
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     nlohmann_json_DEPS_TARGET)
    endif()

    set_property(TARGET nlohmann_json::nlohmann_json
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${nlohmann_json_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET nlohmann_json::nlohmann_json
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${nlohmann_json_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET nlohmann_json::nlohmann_json
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${nlohmann_json_LIB_DIRS_RELEASE}>)
    set_property(TARGET nlohmann_json::nlohmann_json
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${nlohmann_json_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET nlohmann_json::nlohmann_json
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${nlohmann_json_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(nlohmann_json_LIBRARIES_RELEASE nlohmann_json::nlohmann_json)
