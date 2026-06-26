# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(zstd_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(zstd_FRAMEWORKS_FOUND_RELEASE "${zstd_FRAMEWORKS_RELEASE}" "${zstd_FRAMEWORK_DIRS_RELEASE}")

set(zstd_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET zstd_DEPS_TARGET)
    add_library(zstd_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET zstd_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${zstd_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${zstd_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### zstd_DEPS_TARGET to all of them
conan_package_library_targets("${zstd_LIBS_RELEASE}"    # libraries
                              "${zstd_LIB_DIRS_RELEASE}" # package_libdir
                              "${zstd_BIN_DIRS_RELEASE}" # package_bindir
                              "${zstd_LIBRARY_TYPE_RELEASE}"
                              "${zstd_IS_HOST_WINDOWS_RELEASE}"
                              zstd_DEPS_TARGET
                              zstd_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "zstd"    # package_name
                              "${zstd_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${zstd_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT zstd::libzstd_static #############

        set(zstd_zstd_libzstd_static_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(zstd_zstd_libzstd_static_FRAMEWORKS_FOUND_RELEASE "${zstd_zstd_libzstd_static_FRAMEWORKS_RELEASE}" "${zstd_zstd_libzstd_static_FRAMEWORK_DIRS_RELEASE}")

        set(zstd_zstd_libzstd_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET zstd_zstd_libzstd_static_DEPS_TARGET)
            add_library(zstd_zstd_libzstd_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET zstd_zstd_libzstd_static_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'zstd_zstd_libzstd_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${zstd_zstd_libzstd_static_LIBS_RELEASE}"
                              "${zstd_zstd_libzstd_static_LIB_DIRS_RELEASE}"
                              "${zstd_zstd_libzstd_static_BIN_DIRS_RELEASE}" # package_bindir
                              "${zstd_zstd_libzstd_static_LIBRARY_TYPE_RELEASE}"
                              "${zstd_zstd_libzstd_static_IS_HOST_WINDOWS_RELEASE}"
                              zstd_zstd_libzstd_static_DEPS_TARGET
                              zstd_zstd_libzstd_static_LIBRARIES_TARGETS
                              "_RELEASE"
                              "zstd_zstd_libzstd_static"
                              "${zstd_zstd_libzstd_static_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET zstd::libzstd_static
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_LIBRARIES_TARGETS}>
                     )

        if("${zstd_zstd_libzstd_static_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET zstd::libzstd_static
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         zstd_zstd_libzstd_static_DEPS_TARGET)
        endif()

        set_property(TARGET zstd::libzstd_static APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET zstd::libzstd_static APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET zstd::libzstd_static APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_LIB_DIRS_RELEASE}>)
        set_property(TARGET zstd::libzstd_static APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET zstd::libzstd_static APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${zstd_zstd_libzstd_static_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET zstd::libzstd_static APPEND PROPERTY INTERFACE_LINK_LIBRARIES zstd::libzstd_static)

########## For the modules (FindXXX)
set(zstd_LIBRARIES_RELEASE zstd::libzstd_static)
