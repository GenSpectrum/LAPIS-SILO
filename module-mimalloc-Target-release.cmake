# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(mimalloc_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(mimalloc_FRAMEWORKS_FOUND_RELEASE "${mimalloc_FRAMEWORKS_RELEASE}" "${mimalloc_FRAMEWORK_DIRS_RELEASE}")

set(mimalloc_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET mimalloc_DEPS_TARGET)
    add_library(mimalloc_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET mimalloc_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${mimalloc_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${mimalloc_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### mimalloc_DEPS_TARGET to all of them
conan_package_library_targets("${mimalloc_LIBS_RELEASE}"    # libraries
                              "${mimalloc_LIB_DIRS_RELEASE}" # package_libdir
                              "${mimalloc_BIN_DIRS_RELEASE}" # package_bindir
                              "${mimalloc_LIBRARY_TYPE_RELEASE}"
                              "${mimalloc_IS_HOST_WINDOWS_RELEASE}"
                              mimalloc_DEPS_TARGET
                              mimalloc_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "mimalloc"    # package_name
                              "${mimalloc_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${mimalloc_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET mimalloc-static
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${mimalloc_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${mimalloc_LIBRARIES_TARGETS}>
                 )

    if("${mimalloc_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET mimalloc-static
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     mimalloc_DEPS_TARGET)
    endif()

    set_property(TARGET mimalloc-static
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${mimalloc_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET mimalloc-static
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${mimalloc_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET mimalloc-static
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${mimalloc_LIB_DIRS_RELEASE}>)
    set_property(TARGET mimalloc-static
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${mimalloc_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET mimalloc-static
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${mimalloc_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(mimalloc_LIBRARIES_RELEASE mimalloc-static)
