# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(roaring_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(roaring_FRAMEWORKS_FOUND_RELEASE "${roaring_FRAMEWORKS_RELEASE}" "${roaring_FRAMEWORK_DIRS_RELEASE}")

set(roaring_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET roaring_DEPS_TARGET)
    add_library(roaring_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET roaring_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${roaring_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${roaring_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### roaring_DEPS_TARGET to all of them
conan_package_library_targets("${roaring_LIBS_RELEASE}"    # libraries
                              "${roaring_LIB_DIRS_RELEASE}" # package_libdir
                              "${roaring_BIN_DIRS_RELEASE}" # package_bindir
                              "${roaring_LIBRARY_TYPE_RELEASE}"
                              "${roaring_IS_HOST_WINDOWS_RELEASE}"
                              roaring_DEPS_TARGET
                              roaring_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "roaring"    # package_name
                              "${roaring_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${roaring_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET roaring::roaring
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${roaring_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${roaring_LIBRARIES_TARGETS}>
                 )

    if("${roaring_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET roaring::roaring
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     roaring_DEPS_TARGET)
    endif()

    set_property(TARGET roaring::roaring
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${roaring_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET roaring::roaring
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${roaring_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET roaring::roaring
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${roaring_LIB_DIRS_RELEASE}>)
    set_property(TARGET roaring::roaring
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${roaring_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET roaring::roaring
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${roaring_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(roaring_LIBRARIES_RELEASE roaring::roaring)
