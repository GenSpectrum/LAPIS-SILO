# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(simdutf_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(simdutf_FRAMEWORKS_FOUND_RELEASE "${simdutf_FRAMEWORKS_RELEASE}" "${simdutf_FRAMEWORK_DIRS_RELEASE}")

set(simdutf_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET simdutf_DEPS_TARGET)
    add_library(simdutf_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET simdutf_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${simdutf_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${simdutf_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### simdutf_DEPS_TARGET to all of them
conan_package_library_targets("${simdutf_LIBS_RELEASE}"    # libraries
                              "${simdutf_LIB_DIRS_RELEASE}" # package_libdir
                              "${simdutf_BIN_DIRS_RELEASE}" # package_bindir
                              "${simdutf_LIBRARY_TYPE_RELEASE}"
                              "${simdutf_IS_HOST_WINDOWS_RELEASE}"
                              simdutf_DEPS_TARGET
                              simdutf_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "simdutf"    # package_name
                              "${simdutf_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${simdutf_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET simdutf::simdutf
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${simdutf_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${simdutf_LIBRARIES_TARGETS}>
                 )

    if("${simdutf_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET simdutf::simdutf
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     simdutf_DEPS_TARGET)
    endif()

    set_property(TARGET simdutf::simdutf
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${simdutf_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET simdutf::simdutf
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${simdutf_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET simdutf::simdutf
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${simdutf_LIB_DIRS_RELEASE}>)
    set_property(TARGET simdutf::simdutf
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${simdutf_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET simdutf::simdutf
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${simdutf_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(simdutf_LIBRARIES_RELEASE simdutf::simdutf)
